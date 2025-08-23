#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_random.h"
#include "bootloader_random.h"

// ---------------------- Config & Globals ----------------------
static const char *TAG = "RPC_Light_MB";

static const uint32_t MESSAGE_BUFFER_SIZE_BYTES = 256;

static const int MINIMUM_RANDOM_NUMBER_RANGE = 0;
static const int MAXIMUM_RANDOM_NUMBER_RANGE = 9;

static const uint32_t MESSAGE_MAX_LENGTH_BYTES = 10;

static TaskHandle_t client1 = NULL;
static TaskHandle_t client2 = NULL;
static TaskHandle_t client3 = NULL;

MessageBufferHandle_t messageBuffer_request  = NULL;
MessageBufferHandle_t messageBuffer_response = NULL;

typedef enum request_types
{
    ADD,
    MULTIPLY,
    SUBTRACT,
    DIVIDE,
    REQUEST_TYPES_COUNT
} request_types_t;

static int add(int nr1, int nr2) { return nr1 + nr2; }
static int multiply(int nr1, int nr2) { return nr1 * nr2; }
static int subtract(int nr1, int nr2) { return nr1 - nr2; }
static int divide(int nr1, int nr2) { return nr2 ? nr1 / nr2 : 0; }

static uint32_t getRandomNumberInRange(const uint32_t MIN, const uint32_t MAX)
{
    uint32_t number;
    bootloader_random_enable();
    number = esp_random();
    bootloader_random_disable();
    return MIN + (number % (MAX - MIN + 1));
}

// ---------------------------------------------------------------------------
// On-wire frames (≤ 10 bytes total):
// Request:  [id_lo,id_hi][op][a_lo,a_hi][b_lo,b_hi] => 2 + 1 + 2 + 2 = 7 bytes
// Response: [id_lo,id_hi][op][res_lo,res_hi][status] => 2 + 1 + 2 + 1 = 6 bytes
// id: 16-bit; op: 8-bit; a,b,res: int16_t; status: 0=OK, 1=ERR
// ---------------------------------------------------------------------------
typedef struct __attribute__((packed)) {
    uint16_t id;
    uint8_t  op;
    int16_t  a;
    int16_t  b;
} wire_req_t;

typedef struct __attribute__((packed)) {
    uint16_t id;
    uint8_t  op;
    int16_t  result;
    uint8_t  status;
} wire_resp_t;

// Per-client response queues (avoid one client consuming another's response)
static QueueHandle_t qClient1 = NULL;
static QueueHandle_t qClient2 = NULL;
static QueueHandle_t qClient3 = NULL;

// Serialize/deserialize helpers (safe locally; single MCU, little-endian)
static inline size_t encode_request(uint8_t *buf, const wire_req_t *r)
{
    if (!buf || !r) return 0;
    buf[0] = (uint8_t)(r->id & 0xFF);
    buf[1] = (uint8_t)((r->id >> 8) & 0xFF);
    buf[2] = r->op;
    buf[3] = (uint8_t)(r->a & 0xFF);
    buf[4] = (uint8_t)((r->a >> 8) & 0xFF);
    buf[5] = (uint8_t)(r->b & 0xFF);
    buf[6] = (uint8_t)((r->b >> 8) & 0xFF);
    return 7;
}

static inline size_t encode_response(uint8_t *buf, const wire_resp_t *r)
{
    if (!buf || !r) return 0;
    buf[0] = (uint8_t)(r->id & 0xFF);
    buf[1] = (uint8_t)((r->id >> 8) & 0xFF);
    buf[2] = r->op;
    buf[3] = (uint8_t)(r->result & 0xFF);
    buf[4] = (uint8_t)((r->result >> 8) & 0xFF);
    buf[5] = r->status;
    return 6;
}

static inline int decode_request(const uint8_t *buf, size_t len, wire_req_t *out)
{
    if (!buf || !out || len < 7) return -1;
    out->id = (uint16_t)(buf[0] | (buf[1] << 8));
    out->op = buf[2];
    out->a  = (int16_t)(buf[3] | (buf[4] << 8));
    out->b  = (int16_t)(buf[5] | (buf[6] << 8));
    return 0;
}

static inline int decode_response(const uint8_t *buf, size_t len, wire_resp_t *out)
{
    if (!buf || !out || len < 6) return -1;
    out->id     = (uint16_t)(buf[0] | (buf[1] << 8));
    out->op     = buf[2];
    out->result = (int16_t)(buf[3] | (buf[4] << 8));
    out->status = buf[5];
    return 0;
}

// ID -> client queue mapping (distinct prefixes)
static QueueHandle_t queue_for_id(uint16_t id)
{
    // 1xxx -> client1, 2xxx -> client2, 3xxx -> client3
    if (id >= 1000 && id < 2000) return qClient1;
    if (id >= 2000 && id < 3000) return qClient2;
    if (id >= 3000 && id < 4000) return qClient3;
    return NULL;
}

// ------------------------------ Server --------------------------------------
static void server_task(void *arg)
{
    uint8_t buf[MESSAGE_MAX_LENGTH_BYTES];
    for (;;)
    {
        // Wait for request
        size_t n = xMessageBufferReceive(messageBuffer_request, buf, sizeof(buf), portMAX_DELAY);
        if (n == 0) continue;

        wire_req_t req;
        if (decode_request(buf, n, &req) != 0) {
            ESP_LOGW(TAG, "Server: bad request len=%u", (unsigned)n);
            continue;
        }

        // Execute
        int res = 0;
        uint8_t status = 0;
        switch ((request_types_t)req.op) {
            case ADD:      res = add(req.a, req.b); break;
            case MULTIPLY: res = multiply(req.a, req.b); break;
            case SUBTRACT: res = subtract(req.a, req.b); break;
            case DIVIDE:   res = divide(req.a, req.b); break;
            default:       status = 1; break;
        }

        wire_resp_t resp = (wire_resp_t){
            .id = req.id,
            .op = req.op,
            .result = (int16_t)res,
            .status = status
        };

        // Encode and send to response buffer
        uint8_t out[MESSAGE_MAX_LENGTH_BYTES];
        size_t out_len = encode_response(out, &resp);

        // Blocking for simplicity; you can add a timeout if desired
        xMessageBufferSend(messageBuffer_response, out, out_len, portMAX_DELAY);
    }
}

// --------------------------- Response Router --------------------------------
// Single consumer of messageBuffer_response; redistributes to client queues.
static void response_router_task(void *arg)
{
    uint8_t buf[MESSAGE_MAX_LENGTH_BYTES];
    for (;;)
    {
        size_t n = xMessageBufferReceive(messageBuffer_response, buf, sizeof(buf), portMAX_DELAY);
        if (n == 0) continue;

        wire_resp_t r;
        if (decode_response(buf, n, &r) != 0) {
            ESP_LOGW(TAG, "Router: bad resp len=%u", (unsigned)n);
            continue;
        }

        QueueHandle_t q = queue_for_id(r.id);
        if (!q) {
            ESP_LOGW(TAG, "Router: unknown id=%u", r.id);
            continue;
        }
        // Forward full response to the intended client
        xQueueSend(q, &r, portMAX_DELAY);
    }
}

// -------------------------------- Clients -----------------------------------
// Generic client: takes an ID prefix (1000/2000/3000) and uses its queue.
static void client_task_generic(void *arg)
{
    const uint16_t id_base = (uint32_t)arg; // 1000 / 2000 / 3000
    QueueHandle_t myQ = queue_for_id(id_base);
    configASSERT(myQ != NULL);

    uint16_t seq = 1;

    for (;;)
    {
        // Create random op and operands
        request_types_t op = (request_types_t)getRandomNumberInRange(0, REQUEST_TYPES_COUNT - 1);
        int16_t a = (int16_t)getRandomNumberInRange(MINIMUM_RANDOM_NUMBER_RANGE, MAXIMUM_RANDOM_NUMBER_RANGE);
        int16_t b = (int16_t)getRandomNumberInRange(MINIMUM_RANDOM_NUMBER_RANGE, MAXIMUM_RANDOM_NUMBER_RANGE);

        uint16_t id = id_base + seq++; // e.g., 1001, 1002, ...

        wire_req_t req = (wire_req_t){
            .id = id,
            .op = (uint8_t)op,
            .a = a,
            .b = b
        };

        // Serialize and send
        uint8_t out[MESSAGE_MAX_LENGTH_BYTES];
        size_t len = encode_request(out, &req);
        xMessageBufferSend(messageBuffer_request, out, len, portMAX_DELAY);

        // Wait for own response with timeout (1s)
        wire_resp_t resp;
        if (xQueueReceive(myQ, &resp, pdMS_TO_TICKS(1500)) == pdTRUE)
        {
            if (resp.id == id) {
                const char *op_s =
                    (resp.op == ADD)      ? "ADD" :
                    (resp.op == MULTIPLY) ? "MUL" :
                    (resp.op == SUBTRACT) ? "SUB" :
                    (resp.op == DIVIDE)   ? "DIV" : "?";

                if (resp.status == 0) {
                    ESP_LOGI(TAG, "Client %u got: id=%u op=%s a=%d b=%d => res=%d",
                             (unsigned)(id_base/1000), id, op_s, a, b, (int)resp.result);
                } else {
                    ESP_LOGW(TAG, "Client %u error: id=%u op=%s (invalid)",
                             (unsigned)(id_base/1000), id, op_s);
                }
            } else {
                // Rare: mismatch (e.g., out-of-order older reply). We just log here.
                ESP_LOGW(TAG, "Client %u mismatched id: got=%u expected=%u",
                         (unsigned)(id_base/1000), resp.id, id);
            }
        }
        else
        {
            ESP_LOGW(TAG, "Client %u timeout waiting response (id=%u)",
                     (unsigned)(id_base/1000), id);
        }

        // Random pause between requests (100..800 ms)
        TickType_t wait_ms = (TickType_t)getRandomNumberInRange(500, 1500);
        vTaskDelay(pdMS_TO_TICKS(wait_ms));
    }
}

// -------------------------------- app_main ----------------------------------
void app_main(void)
{
    // Message buffers
    messageBuffer_request  = xMessageBufferCreate(MESSAGE_BUFFER_SIZE_BYTES);
    messageBuffer_response = xMessageBufferCreate(MESSAGE_BUFFER_SIZE_BYTES);
    configASSERT(messageBuffer_request && messageBuffer_response);

    // Per-client response queues (demultiplexed by router)
    qClient1 = xQueueCreate(8, sizeof(wire_resp_t));
    qClient2 = xQueueCreate(8, sizeof(wire_resp_t));
    qClient3 = xQueueCreate(8, sizeof(wire_resp_t));
    configASSERT(qClient1 && qClient2 && qClient3);

    // Tasks
    xTaskCreatePinnedToCore(server_task,          "server",  4096, NULL, 5,  NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(response_router_task, "router",  4096, NULL, 6,  NULL, tskNO_AFFINITY);

    xTaskCreatePinnedToCore(client_task_generic,  "client1", 4096, (void*)1000, 4, &client1, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(client_task_generic,  "client2", 4096, (void*)2000, 4, &client2, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(client_task_generic,  "client3", 4096, (void*)3000, 4, &client3, tskNO_AFFINITY);

    ESP_LOGI(TAG, "RPC light started (3 clients, MB req/resp, router dispatch).");
}

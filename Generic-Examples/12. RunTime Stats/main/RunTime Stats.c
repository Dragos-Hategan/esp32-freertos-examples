#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_timer.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"

static const char *TAG = "RT_STATS_WIFI";

/* ---- Config Wi‑Fi ---- */
#ifndef CONFIG_EXAMPLE_WIFI_SSID
#define CONFIG_EXAMPLE_WIFI_SSID     "SSID_UL_TAU"
#endif
#ifndef CONFIG_EXAMPLE_WIFI_PASS
#define CONFIG_EXAMPLE_WIFI_PASS     "PAROLA_TA"
#endif
#define WIFI_MAX_RETRY 5

/* ---- Sync pentru conectare ---- */
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static int s_retry_num = 0;

/* ---- Task-uri demo ---- */
static void cpu_hog_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        uint64_t t0 = esp_timer_get_time();
        while ((esp_timer_get_time() - t0) < 5000) {/* busy ~5ms */}
        vTaskDelayUntil(&last, 1);          // pauză fixă de 1 tick
    }
}

static void io_like_task(void *arg)
{
    for (;;) vTaskDelay(pdMS_TO_TICKS(20));
}

static void print_runtime_stats_task(void *arg)
{
    const size_t BUFSZ = 4096;
    char *buf = malloc(BUFSZ);
    configASSERT(buf);

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        buf[0] = '\0';
        vTaskGetRunTimeStats(buf);
        ESP_LOGI(TAG, "\nTask                 Abs Time      %% CPU\n--------------------------------------------\n%s", buf);
    }
}

/* ---- Trafic mic TCP ca să “miște” stack-ul ---- */
static void tiny_http_traffic_task(void *arg)
{
    /* rulează doar după conectare la Wi‑Fi */
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

    for (;;) {
        struct addrinfo hints = { .ai_socktype = SOCK_STREAM };
        struct addrinfo *res = NULL;
        int sock = -1;

        if (getaddrinfo("example.com", "80", &hints, &res) == 0 && res) {
            sock = socket(res->ai_family, res->ai_socktype, 0);
            if (sock >= 0 && connect(sock, res->ai_addr, res->ai_addrlen) == 0) {
                const char *req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
                send(sock, req, strlen(req), 0);
                char tmp[256];
                (void)recv(sock, tmp, sizeof(tmp), 0); // nu ne interesează conținutul
            }
        }
        if (sock >= 0) close(sock);
        if (res) freeaddrinfo(res);

        vTaskDelay(pdMS_TO_TICKS(2000)); // trafic ușor la 2s
    }
}

/* ---- Event handler Wi‑Fi/IP ---- */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAX_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGW(TAG, "retry to connect to the AP...");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "got IP");
    }
}

/* ---- Init Wi‑Fi STA ---- */
static void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
            ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
            IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = { 0 };
    strncpy((char*)wifi_config.sta.ssid,     CONFIG_EXAMPLE_WIFI_SSID,     sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, CONFIG_EXAMPLE_WIFI_PASS,     sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/* ---- app_main ---- */
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    xTaskCreatePinnedToCore(cpu_hog_task, "cpu_hog", 2048, NULL, 5, NULL, 1);
    xTaskCreatePinnedToCore(io_like_task,  "io_wait", 2048, NULL, 4, NULL, tskNO_AFFINITY);
    xTaskCreatePinnedToCore(print_runtime_stats_task, "rt_stats", 4096, NULL, 3, NULL, tskNO_AFFINITY);

    /* pornește Wi‑Fi; după IP se lansează traficul */
    wifi_init_sta();
    xTaskCreatePinnedToCore(tiny_http_traffic_task, "net_traffic", 4096, NULL, 4, NULL, tskNO_AFFINITY);

    ESP_LOGI(TAG, "Run‑time stats + Wi‑Fi ready. Urmărește procentele în log.");
}

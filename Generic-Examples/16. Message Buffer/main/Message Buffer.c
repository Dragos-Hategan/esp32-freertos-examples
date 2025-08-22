#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/message_buffer.h"

#include "esp_random.h"
#include "bootloader_random.h"

static const uint32_t MESSAGE_BUFFER_SIZE_BYTES = 2048;

static const uint32_t MESSAGE_MIN_LENGTH_BYTES = 10;
static const uint32_t MESSAGE_MAX_LENGTH_BYTES = 200;

MessageBufferHandle_t messageBufferHandle = NULL;

static uint32_t getRandomNumberInRange(const uint32_t MIN, const uint32_t MAX)
{
    uint32_t number;

    bootloader_random_enable();
    number = esp_random();
    bootloader_random_disable();

    return MIN + (number % (MAX - MIN + 1));
}

static void build_message(uint8_t *tx_buffer, int buf_len)
{
    const uint8_t MIN_CHAR = 'A';
    const uint8_t MAX_CHAR = 'z';

    for (int index = 0; index < buf_len; index++)
    {
        tx_buffer[index] = (uint8_t)getRandomNumberInRange(MIN_CHAR, MAX_CHAR);
    }
}

static uint8_t get_crc8_07(const uint8_t *data, size_t len) {
    uint8_t crc = 0x00;          // init
    const uint8_t poly = 0x07;   // x^8 + x^2 + x + 1
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ poly) : (uint8_t)(crc << 1);
        }
    }
    return crc;                  // xorout 0x00
}

static void task1(void *arg)
{
    uint8_t frame[MESSAGE_MAX_LENGTH_BYTES + 1];
    uint8_t tx_buffer[MESSAGE_MAX_LENGTH_BYTES];
    size_t buf_len = 0;
    uint8_t crc = 0;

    while (1)
    {
        buf_len = getRandomNumberInRange(MESSAGE_MIN_LENGTH_BYTES, MESSAGE_MAX_LENGTH_BYTES);
        build_message(tx_buffer, buf_len);
        crc = get_crc8_07(tx_buffer, buf_len);

        memcpy(frame, tx_buffer, buf_len);
        frame[buf_len] = crc;
        size_t sent = xMessageBufferSend(messageBufferHandle, frame, buf_len + 1, pdMS_TO_TICKS(1000));

        if (sent != buf_len + 1)
        {
            printf("Send failed (sent=%zu)\n", sent);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

static void task2(void *arg)
{
    uint8_t rx_buffer[MESSAGE_MAX_LENGTH_BYTES + 1];
    size_t bytes_received = 0;
    uint8_t received_crc = 0;
    uint8_t crc = 0;
    
    while (1)
    {
        bytes_received = xMessageBufferReceive(messageBufferHandle, rx_buffer, sizeof(rx_buffer), portMAX_DELAY);
        if (bytes_received < 1) { printf("Empty/invalid frame\n"); continue; }

        received_crc = rx_buffer[bytes_received - 1];
        crc = get_crc8_07(rx_buffer, bytes_received - 1);
        if (crc != received_crc)
        {
            printf("\n\nCRC mismatch!");
            continue;
        }

        printf("\n\nreceived crc = 0x%" PRIX8 ".\ncalculated crc = 0x%" PRIX8 "\nbytes_received (message + 1 CRC byte) = %zu.\ndata = %.*s\n", received_crc, crc, bytes_received, bytes_received - 1, (char*)rx_buffer);
    }
}

void app_main(void)
{
    messageBufferHandle = xMessageBufferCreate(MESSAGE_BUFFER_SIZE_BYTES);

    if (messageBufferHandle != NULL)
    {
        xTaskCreate(task1, "Task 1", 1024 * 5, NULL, 1, NULL);
        xTaskCreate(task2, "Task 2", 1024 * 5, NULL, 1, NULL);
    }
    else
    {
        printf("Buffer Creation Failed.\n");
    }
}

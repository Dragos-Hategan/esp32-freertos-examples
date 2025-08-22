#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"

#include "esp_random.h"
#include "bootloader_random.h"
#include "esp_timer.h"

static const uint32_t MIN_MESSAGE_BYTES_SIZE = 64;
static const uint32_t MAX_MESSAGE_BYTES_SIZE = 256;

static const uint32_t MIN_INTERVAL_TIME_MS = 250;
static const uint32_t MAX_INTERVAL_TIME_MS = 7500;

static const uint32_t READ_CHUNK_SIZE_BYTES = 128;

TaskHandle_t producerTaskHandle = NULL;
TaskHandle_t consumerTaskHandle = NULL;

static int64_t send_time = 0;
static int64_t receive_time = 0;

static uint32_t getRandomNumberInRange(const uint32_t MIN, const uint32_t MAX)
{
    uint32_t number;

    bootloader_random_enable();
    number = esp_random();
    bootloader_random_disable();

    return MIN + (number % (MAX - MIN + 1));
}

static void producerTask(void *param)
{
    StreamBufferHandle_t streamBufferHandle = (StreamBufferHandle_t) param;
    uint32_t messageLen = 0; 
    uint32_t delay = 0;

    while (1)
    {
        messageLen = getRandomNumberInRange(MIN_MESSAGE_BYTES_SIZE, MAX_MESSAGE_BYTES_SIZE);
        char tx_buffer[MAX_MESSAGE_BYTES_SIZE + 1];
        for (uint32_t index = 0; index < messageLen; index++)
        {
            tx_buffer[index] = 'G';
        }
        tx_buffer[messageLen] = '\0';

        delay = getRandomNumberInRange(MIN_INTERVAL_TIME_MS, MAX_INTERVAL_TIME_MS);

        printf("\n\n************* SENDER *************\n");
        printf("Sending in %" PRIu32 " ms. Message length = %" PRIu32 "\n", delay, messageLen);
        vTaskDelay(pdMS_TO_TICKS(delay));

        size_t bytes_sent = xStreamBufferSend(streamBufferHandle, tx_buffer, messageLen, portMAX_DELAY);
        send_time = esp_timer_get_time();

        printf("%zu bytes were sent.\n", bytes_sent);
        printf("************* SENDER *************\n");

        xTaskNotifyGive(consumerTaskHandle);
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

static void consumerTask(void *param)
{
    StreamBufferHandle_t streamBufferHandle = (StreamBufferHandle_t) param;
    char rx_buffer[READ_CHUNK_SIZE_BYTES];
    size_t bytes_received = 0;

    while (1)
    {
        memset(rx_buffer, 0, sizeof(rx_buffer));
        bytes_received = xStreamBufferReceive(
            streamBufferHandle,
            rx_buffer,
            sizeof(rx_buffer),
            portMAX_DELAY);
            
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        printf("\n------------ RECEIVER ------------\n");
        
        int message_iterations = 1;
        while (bytes_received)
            {
                printf("Chunk %d of data.\n", message_iterations);
                printf("Bytes received: %zu\n", bytes_received);
                printf("Message received:\n%.*s\n", (int)bytes_received, rx_buffer);

                memset(rx_buffer, 0, sizeof(rx_buffer));
                bytes_received = xStreamBufferReceive(
                streamBufferHandle,
                rx_buffer,
                sizeof(rx_buffer),
                0);

                message_iterations++;
            }
        
        receive_time = esp_timer_get_time();
        // This is not the real buffer latency, this time also includes printfs and scheduling.
        printf("Latency per message = %" PRId64 "ms.\n", (receive_time - send_time) / 1000);

        printf("------------ RECEIVER ------------\n");

        xTaskNotifyGive(producerTaskHandle);
    }
}

void app_main(void)
{
    StreamBufferHandle_t streamBufferHandle = NULL;
    streamBufferHandle = xStreamBufferCreate(512, READ_CHUNK_SIZE_BYTES);

    if (streamBufferHandle != NULL)
    {
        xTaskCreatePinnedToCore(producerTask, "Producer", 1024 * 2, (void *)streamBufferHandle, 1, &producerTaskHandle, 0);
        xTaskCreatePinnedToCore(consumerTask, "Consumer", 1024 * 2, (void *)streamBufferHandle, 1, &consumerTaskHandle, 1);
    }

    while(1)
    {
        vTaskDelay(1);
    }
}

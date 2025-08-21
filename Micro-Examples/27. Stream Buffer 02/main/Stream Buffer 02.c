#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "freertos/stream_buffer.h"

StreamBufferHandle_t streamBufferHandle = NULL;

static void task1(void *arg)
{
    char tx_buffer[50];
    int buf_len = 0;
    int nr = 0;
    int bytes_sent = 0;

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(3000));

        nr++;
        buf_len = sprintf(tx_buffer, "Hello, I am Dragos, iteration %d\n", nr);
        bytes_sent = xStreamBufferSend(
            streamBufferHandle,
            tx_buffer,
            buf_len,
            portMAX_DELAY);

        printf("------------\nSend: buf_len = %d, bytes_sent = %d\n", buf_len, bytes_sent);
    }
}

static void task2(void *arg)
{
    char rx_buffer[50];
    int bytes_received = 0;

    while (1)
    {
        memset(rx_buffer, 0, sizeof(rx_buffer));
        bytes_received = xStreamBufferReceive(
            streamBufferHandle,
            rx_buffer,
            sizeof(rx_buffer),
            portMAX_DELAY);
        
        printf("Received: bytes_received = %d, data = %s", bytes_received, rx_buffer);

        //vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void task3(void *arg)
{
    while (1)
    {
        int buff_space = xStreamBufferSpacesAvailable(streamBufferHandle);
        printf("Buffer space is %d\n", buff_space);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void app_main(void)
{
    streamBufferHandle = xStreamBufferCreate(256, 100);

    if (streamBufferHandle != NULL)
    {
        xTaskCreate(task1, "Task 1", 1024 * 5, NULL, 1, NULL);
        xTaskCreate(task2, "Task 2", 1024 * 5, NULL, 1, NULL);
        xTaskCreate(task3, "Task 3", 1024 * 5, NULL, 1, NULL);
    }
    else
    {
        printf("Buffer Creation Failed.\n");
    }
}

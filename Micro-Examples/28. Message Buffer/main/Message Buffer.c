#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "freertos/message_buffer.h"

MessageBufferHandle_t messageBufferHandle = NULL;

static void task1(void *arg)
{
    char tx_buffer[50];
    int buf_len = 0;
    int bytes_sent = 0;

    for (int i = 0; i < 3; i++)
    {
        buf_len = sprintf(tx_buffer, "Hello, I am Dragos, iteration %d\n", i + 1);

        bytes_sent = xMessageBufferSend(
            messageBufferHandle,
            tx_buffer,
            buf_len,
            portMAX_DELAY);
    
        printf("------------\nSend: buf_len = %d, bytes_sent = %d\n", buf_len, bytes_sent);
    }
    vTaskDelete(NULL);
}

static void task2(void *arg)
{
    char rx_buffer[200];
    int bytes_received = 0;
    
    vTaskDelay(pdMS_TO_TICKS(3000));

    while (1)
    {
        memset(rx_buffer, 0, sizeof(rx_buffer));
        bytes_received = xMessageBufferReceive(
            messageBufferHandle,
            rx_buffer,
            sizeof(rx_buffer),
            portMAX_DELAY);

        printf("Received: bytes_received = %d, data = %s", bytes_received, rx_buffer);
    }
}

void app_main(void)
{
    messageBufferHandle = xMessageBufferCreate(1000);

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

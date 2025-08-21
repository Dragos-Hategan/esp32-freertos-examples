#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void writeTask(void *pvParameters){
    QueueHandle_t mailBoxHandle = (QueueHandle_t)pvParameters;
    BaseType_t xStatus;

    int i = 1;
    
    while (true){
        xStatus = xQueueOverwrite(mailBoxHandle, &i);
        if (xStatus != pdPASS) {
            printf("sendTask1: Failed to send data to the queue!\n");
        }else{
            printf("sendTask1: Data written to the mailbox!\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        i++;
        if (i == 6){
            i = 1;
        }
    }
}

void readTask1(void *pvParameters){
    QueueHandle_t mailBoxHandle = (QueueHandle_t)pvParameters;
    BaseType_t xStatus;

    int iReceived = 0;
    
    while (true){
        xStatus = xQueuePeek(mailBoxHandle, &iReceived, portMAX_DELAY);
        if (xStatus != pdPASS) {
            printf("readTask1: Failed to read mailbox!\n");
        }else{
            printf("readTask1: Data read from the mailbox is %d!\n", iReceived);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void readTask2(void *pvParameters){
    QueueHandle_t mailBoxHandle = (QueueHandle_t)pvParameters;
    BaseType_t xStatus;

    int iReceived = 0;
    
    while (true){
        xStatus = xQueuePeek(mailBoxHandle, &iReceived, portMAX_DELAY);
        if (xStatus != pdPASS) {
            printf("readTask2: Failed to read mailbox!\n");
        }else{
            printf("readTask2: Data read from the mailbox is %d!\n", iReceived);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void readTask3(void *pvParameters){
    QueueHandle_t mailBoxHandle = (QueueHandle_t)pvParameters;
    BaseType_t xStatus;

    int iReceived = 0;
    
    while (true){
        xStatus = xQueuePeek(mailBoxHandle, &iReceived, portMAX_DELAY);
        if (xStatus != pdPASS) {
            printf("readTask3: Failed to read mailbox!\n");
        }else{
            printf("readTask3: Data read from the mailbox is %d!\n", iReceived);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    QueueHandle_t mailBoxHandle = xQueueCreate(1, sizeof(int));

    if (mailBoxHandle != NULL){
        printf("Mailbox created successfully!\n");
        xTaskCreate(writeTask, "writeTask", 1024 * 5, (void *)mailBoxHandle, 1, NULL);
        xTaskCreate(readTask1, "readTask1", 1024 * 5, (void *)mailBoxHandle, 1, NULL);
        xTaskCreate(readTask2, "readTask2", 1024 * 5, (void *)mailBoxHandle, 1, NULL);
        xTaskCreate(readTask3, "readTask3", 1024 * 5, (void *)mailBoxHandle, 1, NULL);
    }

}

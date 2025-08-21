#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

void sendToBackTask(void *pvParam)
{
    QueueHandle_t queueHandle = (QueueHandle_t)pvParam;
    BaseType_t xStatus;
    int i = 1;

    while (true)
    {
        xStatus = xQueueSendToBack(queueHandle, &i, 0);
        if (xStatus == pdPASS) {
            printf("Sent item: %d\n", i);
        }else{
            printf("Failed to send item %d\n", i);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        i++;
        if (i == 6){
            i = 1;
        }
    }
}

void sendToFrontTask(void *pvParam)
{
    QueueHandle_t queueHandle = (QueueHandle_t)pvParam;
    BaseType_t xStatus;
    int i = 1;

    while (true)
    {
        xStatus = xQueueSendToFront(queueHandle, &i, 0);
        if (xStatus == pdPASS) {
            printf("Sent item: %d\n", i);
        }else{
            printf("Failed to send item %d\n", i);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        i++;
        if (i == 6){
            i = 1;
        }
    }
}

void recvTask(void *pvParam)
{
    QueueHandle_t queue = (QueueHandle_t)pvParam;
    BaseType_t xStatus;
    int j = 0;

    while(1) {
        UBaseType_t iDataItem = uxQueueMessagesWaiting(queue);

        if (iDataItem == 5){
            for (int item = 0; item < 5; item++) {
                xStatus = xQueueReceive(queue, &j, 0);
                if (xStatus == pdPASS) {
                    printf("Received item: %d\n", j);
                } else {
                    printf("Failed to receive item %d\n", j);
                }
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    QueueHandle_t queueHandle = xQueueCreate(5, sizeof(int));
    //xTaskCreate(sendToBackTask, "sendToBackTask", 2048, (void *) queueHandle, 1, NULL);
    xTaskCreate(sendToFrontTask, "sendToFrontTask", 2048, (void *) queueHandle, 1, NULL);
    xTaskCreate(recvTask, "RecvTask", 2048, (void *) queueHandle, 1, NULL);
}

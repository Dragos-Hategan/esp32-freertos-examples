#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void sendTask(void *arg)
{
    printf("Send Task - started at tick %lu\n", xTaskGetTickCount());
    QueueHandle_t queueHandle = (QueueHandle_t)arg;
    BaseType_t queueSendStatus;
    int count = 0;

    while(1){
        count++;
        queueSendStatus = xQueueSend(queueHandle, &count, 0);
        if (queueSendStatus != pdTRUE){
            printf("Send Task - Failed to send integer %d to queue at tick %lu\n", count, xTaskGetTickCount());
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void receiveTask(void *arg)
{
    printf("Receive Task - started at tick %lu\n", xTaskGetTickCount());
    QueueHandle_t queueHandle = (QueueHandle_t)arg;
    BaseType_t queueReceiveStatus;
    int receivedInteger = 0;

    while(1){
        queueReceiveStatus = xQueueReceive(queueHandle, &receivedInteger, portMAX_DELAY);
        if (queueReceiveStatus == pdTRUE) {
            printf("Receive Task - received integer: %d at tick %lu\n", receivedInteger, xTaskGetTickCount());
        }else{
            printf("Receive Task - Failed to receive integer at tick %lu\n", xTaskGetTickCount());
        }
    }
}

void app_main(void)
{
    QueueHandle_t queueHandle = NULL;
    queueHandle = xQueueCreate(5, sizeof(int));

    if (queueHandle != NULL){   
        printf("Queue created successfully, creating tasks.\n");
        xTaskCreatePinnedToCore(sendTask, "Send Task", 1024 * 2, (void *)queueHandle, 1, NULL, 1);
        xTaskCreatePinnedToCore(receiveTask, "Receive Task", 1024 * 2, (void *)queueHandle, 1, NULL, 1);
    }else{
        printf("Failed to create queue.\n");
    }
}

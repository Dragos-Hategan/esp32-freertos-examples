#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef struct A_STRUCT{
    char id;
    char data;
} xStruct;

void sendTask1(void *pvParameters){
    QueueHandle_t queueHandle = (QueueHandle_t)pvParameters;
    BaseType_t xStatus;

    int i = 1;
    
    while (true){
        xStatus = xQueueSendToBack(queueHandle, &i, 0);
        if (xStatus != pdPASS) {
            printf("sendTask1: Failed to send data to the queue!\n");
        }else{
            printf("sendTask1: Data sent to the queue!\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void sendTask2(void *pvParameters){
    QueueHandle_t queueHandle = (QueueHandle_t)pvParameters;
    BaseType_t xStatus;

    int i = 2;
    
    while (true){
        xStatus = xQueueSendToBack(queueHandle, &i, 0);
        if (xStatus != pdPASS) {
            printf("sendTask2: Failed to send data to the queue!\n");
        }else{
            printf("sendTask2: Data sent to the queue!\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void receiveTask(void *pvParameters){
    QueueSetHandle_t queueSetHandle = (QueueSetHandle_t)pvParameters;
    BaseType_t xStatus;
    
    int iReceived;

    while (true){
        QueueSetMemberHandle_t queueSetMemberHandle = xQueueSelectFromSet(queueSetHandle, portMAX_DELAY);

        xStatus = xQueueReceive(queueSetMemberHandle, &iReceived, portMAX_DELAY);
        if (xStatus != pdPASS){
            printf("receiveTask: Failed to receive data from the queue!\n");
        }else{
            printf("receiveTask: Received data: %d\n", iReceived);
        }
    }
}


void app_main(void)
{
    QueueHandle_t queueHandle1 = xQueueCreate(5, sizeof(int));
    QueueHandle_t queueHandle2 = xQueueCreate(5, sizeof(int));
    QueueSetHandle_t queueSetHandle = xQueueCreateSet(10);
    xQueueAddToSet(queueHandle1, queueSetHandle);
    xQueueAddToSet(queueHandle2, queueSetHandle);

    if ((queueHandle1 != NULL) && (queueHandle2 != NULL) && (queueSetHandle != NULL)) {
        printf("The queue created succesfully!\n");
        xTaskCreate(sendTask1, "sendTask1", 1024 * 5, (void *)queueHandle1, 1, NULL);
        xTaskCreate(sendTask2, "sendTask2", 1024 * 5, (void *)queueHandle2, 1, NULL);
        xTaskCreate(receiveTask, "receiveTask", 1024 * 5, (void *)queueSetHandle, 2, NULL);
    }else{
        printf("The queue can't be created!\n");
    }
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

typedef struct A_STRUCT{
    char id;
    char data;
} xStruct;

void sendTask(void *pvParameters){
    QueueHandle_t queueHandle = (QueueHandle_t)pvParameters;
    BaseType_t xStatus;
    
    xStruct xUSB = {1, 5};
    
    while (true){
        xStatus = xQueueSendToBack(queueHandle, &xUSB, 0);
        if (xStatus != pdPASS) {
            printf("myTask!1: Failed to send data to the queue!\n");
        }else{
            printf("myTask!1: Data sent to the queue!\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        xUSB.id++;
        if (xUSB.id == 6){
            xUSB.id = 1;
        }
    }
}

void receiveTask(void *pvParameters){
    QueueHandle_t queueHandle = (QueueHandle_t)pvParameters;
    BaseType_t xStatus;
    
    xStruct xUSB = {0, 0};

    while (true){
        xStatus = xQueueReceive(queueHandle, &xUSB, 0);
        if (xStatus != pdPASS){
            printf("myTask!2: Failed to receive data from the queue!\n");
        }else{
            printf("myTask!2: Received ID=%d, data=%d!\n", xUSB.id, xUSB.data);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void app_main(void)
{
    QueueHandle_t queueHandle = xQueueCreate(5, sizeof(xStruct));
    if (queueHandle != NULL) {
        printf("The queue created succesfully!\n");
        xTaskCreate(sendTask, "sendTask", 1024 * 5, (void *)queueHandle, 1, NULL);
        xTaskCreate(receiveTask, "receiveTask", 1024 * 5, (void *)queueHandle, 1, NULL);
    }else{
        printf("The queue can't be created!\n");
    }
}

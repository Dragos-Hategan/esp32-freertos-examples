#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

void myTask1(void *pvParameters){
    QueueHandle_t queueHandle = (QueueHandle_t)pvParameters;
    BaseType_t xStatus;
    int i = 0;
    
    while (true){
        UBaseType_t spacesAvailableInQueue = uxQueueSpacesAvailable(queueHandle);
        printf("myTask!1: Queue spaces available: %d\n", spacesAvailableInQueue);

        xStatus = xQueueSendToBack(queueHandle, &i, 0);
        if (xStatus != pdPASS) {
            printf("myTask!1: Failed to send data to the queue!\n");
        }else{
            printf("myTask!1: Data %d sent to the queue!\n", i);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        i++;
        if (i == 6){
            i = 0;
        }
    }
}

void myTask2(void *pvParameters){
    QueueHandle_t queueHandle = (QueueHandle_t)pvParameters;

    while (true){
        UBaseType_t messagesWaiting = uxQueueMessagesWaiting(queueHandle);
        printf("myTask!2: Messages Waiting In Queue: %d\n", messagesWaiting);
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void app_main(void)
{
    QueueHandle_t queueHandle = xQueueCreate(5, sizeof(int));
    if (queueHandle != NULL) {
        printf("The queue created succesfully!\n");
        xTaskCreate(myTask1, "sendTask", 1024 * 5, (void *)queueHandle, 0, NULL);
        xTaskCreate(myTask2, "receiveTask", 1024 * 5, (void *)queueHandle, 0, NULL);
    }else{
        printf("The queue can't be created!\n");
    }
}

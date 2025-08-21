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
    
    char *strToSend;
    const int STR_MAX_LENGTH = 75;
    uint8_t stringNumber = 0;
    
    while (true){
        strToSend = (char *)malloc(STR_MAX_LENGTH);
        stringNumber++;
        snprintf(strToSend, STR_MAX_LENGTH, "Salutare! Acesta este un mesaj trimis pe queue;)! String number = %d", stringNumber);
        xStatus = xQueueSendToBack(queueHandle, &strToSend, 0);

        if (xStatus != pdPASS) {
            printf("myTask!1: Failed to send data to the queue!\n");
        }else{
            printf("myTask!1: Data sent to the queue!\n");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        if (stringNumber == 10){
            printf("myTask!1: String number exceeded! Reinitializing!\n\n");
            stringNumber = 0;
        }
    }
}

void receiveTask(void *pvParameters){
    QueueHandle_t queueHandle = (QueueHandle_t)pvParameters;
    BaseType_t xStatus;
    
    char *strReceived;

    while (true){
        xStatus = xQueueReceive(queueHandle, &strReceived, 0);
        if (xStatus != pdPASS){
            printf("myTask!2: Failed to receive data from the queue!\n");
        }else{
            printf("myTask!2: Received string: %s\n", strReceived);
            printf("myTask!2: Freeing memory for the received string!\n");
            free(strReceived);
            strReceived = NULL;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


void app_main(void)
{
    QueueHandle_t queueHandle = xQueueCreate(5, sizeof(char *));
    if (queueHandle != NULL) {
        printf("The queue created succesfully!\n");
        xTaskCreate(sendTask, "sendTask", 1024 * 5, (void *)queueHandle, 1, NULL);
        xTaskCreate(receiveTask, "receiveTask", 1024 * 5, (void *)queueHandle, 1, NULL);
    }else{
        printf("The queue can't be created!\n");
    }
}

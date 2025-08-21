#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

SemaphoreHandle_t semaphoreHandle = NULL;

void carInTask(void *arg)
{
    int emptySpaces = 0;
    int result;

    while (true){
        emptySpaces = uxSemaphoreGetCount(semaphoreHandle);
        printf("There are %d empty spaces availabe.\n", emptySpaces);

        result = xSemaphoreTake(semaphoreHandle, portMAX_DELAY);
        if (result == pdTRUE){
            printf("A car is entering the parking lot.\n");
        }else{
            printf("No empty spaces available in the parking lot.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }   
}

void carOutTask(void *arg)
{
    while (true){
        vTaskDelay(pdMS_TO_TICKS(3000));

        if (xSemaphoreGive(semaphoreHandle) == pdTRUE){
            printf("A car is leaving the parking lot.\n");
        }
    }   
}

void app_main(void)
{
    // Create a counting semaphore with a maximum count of 5 and an initial count of 5
    // This means there are 5 empty spaces in the parking lot initially.
    semaphoreHandle = xSemaphoreCreateCounting(5, 5); 
    
    if (semaphoreHandle != NULL){
        xTaskCreate(carInTask, "carInTask", 1024 * 5, NULL, 1, NULL);
        xTaskCreate(carOutTask, "carOutTask", 1024 * 5, NULL, 1, NULL);
    }

}

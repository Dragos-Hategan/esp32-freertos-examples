#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

SemaphoreHandle_t semaphoreHandle = NULL;

void myTask(void *arg)
{
    while (true){
        xSemaphoreTake(semaphoreHandle, portMAX_DELAY);
        for (int i = 0; i < 10; i++) {
            printf("Task %s is running, iteration %d\n", pcTaskGetName(NULL), i);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        xSemaphoreGive(semaphoreHandle);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }   
}

void app_main(void)
{
    semaphoreHandle = xSemaphoreCreateBinary();
    xSemaphoreGive(semaphoreHandle);

    if (semaphoreHandle != NULL){
        xTaskCreate(myTask, "myTask1", 1024 * 5, NULL, 1, NULL);
        xTaskCreate(myTask, "myTask2", 1024 * 5, NULL, 1, NULL);
    }

}

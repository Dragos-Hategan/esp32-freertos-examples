#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

SemaphoreHandle_t mutexHandle = NULL; 

static void common_task(void)
{
    const char * TASK_NAME = pcTaskGetName(NULL);
    
    while (true){
        printf("Entering task %s\n", TASK_NAME);

        xSemaphoreTakeRecursive(mutexHandle, portMAX_DELAY);   
        printf("Resource A taken from task %s\n", TASK_NAME);
        for(int i = 0; i < 10; i++){
            printf("Task %s, for resource A, i=%d\n", TASK_NAME, i);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        xSemaphoreTakeRecursive(mutexHandle, portMAX_DELAY);   
        printf("Resource B taken from task %s\n", TASK_NAME);
        for(int i = 0; i < 10; i++){
            printf("Task %s, for resource B, i=%d\n", TASK_NAME, i);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }   
        printf("Task %s give resource B\n", TASK_NAME);     
        xSemaphoreGiveRecursive(mutexHandle);
        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("Task %s give resource A\n", TASK_NAME);     
        xSemaphoreGiveRecursive(mutexHandle);
        vTaskDelay(pdMS_TO_TICKS(3000));        
    }   
}

static void task1(void *arg)
{
    common_task();
}

static void task2(void *arg)
{
    vTaskDelay(pdMS_TO_TICKS(1000));
    common_task();
}

void app_main(void)
{
    mutexHandle = xSemaphoreCreateRecursiveMutex();
    if (mutexHandle != NULL){
        xTaskCreate(task1, "task1", 1024 * 2, NULL, 1, NULL);
        xTaskCreate(task2, "task2", 1024 * 2, NULL, 3, NULL);
    }
}

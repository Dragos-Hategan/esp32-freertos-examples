#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

SemaphoreHandle_t mutexHandle = NULL; 

static void common_task(void)
{
    int priority = 0;
    priority = uxTaskPriorityGet(NULL);
    const char * TASK_NAME = pcTaskGetName(NULL);
    printf("Entering task %s, priority is %d\n", TASK_NAME, priority);
    
    while (true){
        xSemaphoreTake(mutexHandle, portMAX_DELAY);
        priority = uxTaskPriorityGet(NULL);
        printf("Mutex taken, priority of task %s is %d\n", TASK_NAME, priority);        
    
        for(int i = 0; i < 10; i++){
            priority = uxTaskPriorityGet(NULL);
            printf("Task %s, priority = %d, i=%d\n", TASK_NAME, priority, i);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    
        xSemaphoreGive(mutexHandle);
        priority = uxTaskPriorityGet(NULL);
        printf("Mutex given, priority of task %s is %d\n", TASK_NAME, priority);   
        vTaskDelay(pdMS_TO_TICKS(1000));
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
    mutexHandle = xSemaphoreCreateMutex();
    if (mutexHandle != NULL){
        xTaskCreate(task1, "task1", 1024 * 2, NULL, 1, NULL);
        xTaskCreate(task2, "task2", 1024 * 2, NULL, 3, NULL);
    }
}

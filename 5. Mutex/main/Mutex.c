#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

SemaphoreHandle_t messageMutex = NULL;

static void print_task(void *arg)
{
    const char * TASK_NAME = pcTaskGetName(NULL);
    printf("%s started\n", TASK_NAME);
    BaseType_t mutex_status = pdFALSE;

    while (1){
        mutex_status = 0;
        mutex_status = xSemaphoreTake(messageMutex, portMAX_DELAY);
        if (mutex_status == pdTRUE){
            printf("Task '%s' has taken the mutex\n", TASK_NAME);
            for (int index = 0; index < 5; index++){
                printf("%d iterations left in %s\n", 5 - index, TASK_NAME);
                vTaskDelay(pdMS_TO_TICKS(750));
            }
            printf("\n");
            xSemaphoreGive(messageMutex);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void app_main(void)
{
    messageMutex = xSemaphoreCreateMutex();
    xSemaphoreGive(messageMutex);

    if (messageMutex != NULL){
        xTaskCreate(print_task, "Task 1", 1024 * 2, NULL, 1, NULL);
        xTaskCreate(print_task, "Task 2", 1024 * 2, NULL, 6, NULL);
    }   

}

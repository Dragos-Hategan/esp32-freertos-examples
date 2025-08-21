#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

TaskHandle_t task0Handle = NULL;
TaskHandle_t task1Handle = NULL;

static void myTask0(void *arg)
{
    const char *TASK_NAME = pcTaskGetName(NULL);
    while (1)
    {
        printf("\n--------------------------------------\n");
        printf("%s wait for notification!\n", TASK_NAME);

        vTaskDelay(pdMS_TO_TICKS(1000));

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        printf("%s got the notification -> synced!\n", TASK_NAME);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

static void myTask1(void *arg)
{
    const char *TASK_NAME = pcTaskGetName(NULL);
    while (1)
    {
        printf("\n--------------------------------------\n");
        printf("%s wait for notification!\n", TASK_NAME);

        vTaskDelay(pdMS_TO_TICKS(1000));

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        printf("%s got the notification -> synced!\n", TASK_NAME);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

static void myTask2(void *arg)
{
    const char *TASK_NAME = pcTaskGetName(NULL);
    while (1)
    {
        printf("\n--------------------------------------\n");
        printf("%s begins!\n", TASK_NAME);

        vTaskDelay(pdMS_TO_TICKS(5000));

        printf("%s notifies %s\n", TASK_NAME, pcTaskGetName(task0Handle));
        xTaskNotifyGive(task0Handle);

        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("%s notifies %s\n", TASK_NAME, pcTaskGetName(task1Handle));
        xTaskNotifyGive(task1Handle);

        printf("%s notify done: synced!\n", TASK_NAME);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void app_main(void)
{
    xTaskCreate(myTask0, "My Task 0", 1024 * 2, NULL, 1, &task0Handle);
    xTaskCreate(myTask1, "My Task 1", 1024 * 2, NULL, 1, &task1Handle);
    xTaskCreate(myTask2, "My Task 2", 1024 * 2, NULL, 1, NULL);
}

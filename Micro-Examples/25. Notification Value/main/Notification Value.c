#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;

static void myTask1(void *arg)
{
    const char *TASK_NAME = pcTaskGetName(NULL);
    uint32_t ulNotifiedValue;
    while (1)
    {
        printf("\n--------------------------------------\n");
        printf("%s wait for notification!\n", TASK_NAME);

        vTaskDelay(pdMS_TO_TICKS(1000));

        xTaskNotifyWait(0x00, ULONG_MAX, &ulNotifiedValue, portMAX_DELAY);

        if ((ulNotifiedValue & 0x01) != 0)
        {
            printf("%s process BIT_0 event!\n", TASK_NAME);
        }

        if ((ulNotifiedValue & 0x02) != 0)
        {
            printf("%s process BIT_1 event!\n", TASK_NAME);
        }
        
        if ((ulNotifiedValue & 0x04) != 0)
        {
            printf("%s process BIT_2 event!\n", TASK_NAME);
        }        

        printf("%s finished the event processing!\n", TASK_NAME);
        vTaskDelay(pdMS_TO_TICKS(1000));
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

        printf("%s notifies %s with BIT_0\n", TASK_NAME, pcTaskGetName(task1Handle));
        xTaskNotify(task1Handle, 0x01, eSetValueWithOverwrite);

        vTaskDelay(pdMS_TO_TICKS(2500));

        printf("%s notifies %s with BIT_1\n", TASK_NAME, pcTaskGetName(task1Handle));
        xTaskNotify(task1Handle, 0x02, eSetValueWithOverwrite);

        vTaskDelay(pdMS_TO_TICKS(2500));

        printf("%s notifies %s with BIT_2\n", TASK_NAME, pcTaskGetName(task1Handle));
        xTaskNotify(task1Handle, 0x04, eSetValueWithOverwrite);        

        printf("%s notify done!\n", TASK_NAME);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    xTaskCreate(myTask1, "My Task 1", 1024 * 2, NULL, 1, &task1Handle);
    xTaskCreate(myTask2, "My Task 2", 1024 * 2, NULL, 1, &task2Handle);
}

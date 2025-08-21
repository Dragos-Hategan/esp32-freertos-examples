#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#ifndef BIT_0
#define BIT_0 (1 << 0)
#endif

#ifndef BIT_4
#define BIT_4 (1 << 4)
#endif

EventGroupHandle_t eventGroupHandle = NULL;

static void myTask1(void *ard)
{
    const char *TASK_NAME = pcTaskGetName(NULL);
    while (1)
    {
        printf("\n--------------------------------------\n");
        printf("%s begins to check\n", TASK_NAME);

        xEventGroupWaitBits(
            eventGroupHandle,
            BIT_0 | BIT_4,
            pdTRUE,
            pdTRUE,
            portMAX_DELAY);

        printf("In %s, event group is set by BIT_0 and BIT_4, bits are cleared!\n", TASK_NAME);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void myTask2(void *ard)
{
    const char *TASK_NAME = pcTaskGetName(NULL);
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (1)
    {
        printf("\n--------------------------------------\n");

        printf("%s sets BIT_0\n", TASK_NAME);
        xEventGroupSetBits(
            eventGroupHandle,
            BIT_0);

        vTaskDelay(pdMS_TO_TICKS(5000));

        printf("%s sets BIT_4\n", TASK_NAME);
        xEventGroupSetBits(
            eventGroupHandle,
            BIT_4);

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    eventGroupHandle = xEventGroupCreate();
    if (eventGroupHandle != NULL)
    {
        xTaskCreate(myTask1, "My Task 1", 1024 * 2, NULL, 1, NULL);
        xTaskCreate(myTask2, "My Task 2", 1024 * 2, NULL, 1, NULL);
    }
}

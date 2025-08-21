#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#ifndef BIT_0
#define BIT_0 (1 << 0)
#endif

#ifndef BIT_1
#define BIT_1 (1 << 1)
#endif

#ifndef BIT_2
#define BIT_2 (1 << 2)
#endif

#ifndef ALL_SYNC_BITS
#define ALL_SYNC_BITS (BIT_0 | BIT_1 | BIT_2)
#endif

EventGroupHandle_t eventGroupHandle = NULL;

static void myTask0(void *arg)
{
    const char *TASK_NAME = pcTaskGetName(NULL);
    while (1)
    {
        printf("\n--------------------------------------\n");
        printf("%s begins!\n", TASK_NAME);

        vTaskDelay(pdMS_TO_TICKS(1000));

        printf("%s sets BIT_0!\n", TASK_NAME);

        xEventGroupSync(
            eventGroupHandle,
            BIT_0,
            ALL_SYNC_BITS,
            portMAX_DELAY);

        printf("%s sync!\n", TASK_NAME);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

static void myTask1(void *arg)
{
    const char *TASK_NAME = pcTaskGetName(NULL);
    while (1)
    {
        printf("\n--------------------------------------\n");
        printf("%s begins!\n", TASK_NAME);

        vTaskDelay(pdMS_TO_TICKS(3000));

        printf("%s sets BIT_1!\n", TASK_NAME);

        xEventGroupSync(
            eventGroupHandle,
            BIT_1,
            ALL_SYNC_BITS,
            portMAX_DELAY);

        printf("%s sync!\n", TASK_NAME);
        vTaskDelay(pdMS_TO_TICKS(5000));
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

        printf("%s sets BIT_2!\n", TASK_NAME);

        xEventGroupSync(
            eventGroupHandle,
            BIT_2,
            ALL_SYNC_BITS,
            portMAX_DELAY);

        printf("%s sync!\n", TASK_NAME);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    eventGroupHandle = xEventGroupCreate();
    if (eventGroupHandle != NULL)
    {
        xTaskCreate(myTask0, "My Task 0", 1024 * 2, NULL, 1, NULL);
        xTaskCreate(myTask1, "My Task 1", 1024 * 2, NULL, 1, NULL);
        xTaskCreate(myTask2, "My Task 2", 1024 * 2, NULL, 1, NULL);
    }
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

static void myTask1(void *arg)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    const char *TASK_NAME = pcTaskGetName(NULL);
    printf(
        "Entered %s at tick %lu.\nFunction 'vTaskDelay' will be used for delays.\nDelaying 100 ticks...\n",
        TASK_NAME,
        lastWakeTime);

    vTaskDelay(100);

    while (1)
    {
        lastWakeTime = xTaskGetTickCount();
        printf("Tick after the delay in %s is %lu.\nGoing for 10 50 tick delays.\n", TASK_NAME, lastWakeTime);

        for (int i = 0; i < 10; i++){
            vTaskDelay(50);
            printf("%s, iteration #%d, tick %lu.\n", TASK_NAME, i + 1, xTaskGetTickCount());
        }

        printf("Got out of the loop at tick %lu.\n", xTaskGetTickCount());
    }
}

static void myTask2(void *arg)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    const char *TASK_NAME = pcTaskGetName(NULL);
    printf("Entered %s at tick %lu.\nFunction 'vTaskDelayUntil' will be used for delays.\nDelaying 100 ticks...\n", TASK_NAME, lastWakeTime);

    vTaskDelayUntil(&lastWakeTime, 100);

    while (1)
    {
        lastWakeTime = xTaskGetTickCount();
        printf("Tick after the delay in %s is %lu.\nGoing for 10 50 tick delays.\n", TASK_NAME, lastWakeTime);

        for (int i = 0; i < 10; i++){
            vTaskDelayUntil(&lastWakeTime, 50);
            printf("%s, iteration #%d, tick %lu.\n", TASK_NAME, i + 1, lastWakeTime);
        }

        printf("Got out of the loop at tick %lu.\n", xTaskGetTickCount());
    }
}

void app_main(void)
{
    xTaskCreate(myTask1, "vTaskDelay Task", 1024 * 2, NULL, 1, NULL);
    xTaskCreate(myTask2, "vTaskDelayUntil Task", 1024 * 2, NULL, 1, NULL);
}

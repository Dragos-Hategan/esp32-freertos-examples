#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static void task1(void *arg)
{
    int count = 0;
    printf("Task1 - started at tick %ld\n", xTaskGetTickCount());
    while(1){
        count++;
        printf("Task1 - periodic printf no. %d, at tick %ld\n", count, xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void task2(void *arg)
{
    int count = 0;
    printf("Task2 - started at tick %ld\n", xTaskGetTickCount());
    while(1){
        count++;
        printf("Task2 - periodic printf no. %d, at tick %ld\n", count, xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

}

void app_main(void)
{
    xTaskCreatePinnedToCore(task1, "Task1", 1024 * 2, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(task2, "Task2", 1024 * 2, NULL, 1, NULL, 1);
}

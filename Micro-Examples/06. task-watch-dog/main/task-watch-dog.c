#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

static void myTask1(void *params){
    while (true) {
        //vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void myTask2(void *params){
    esp_task_wdt_add(NULL);

    while (true) {
        printf("myTask2!\n");
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    xTaskCreate(myTask1, "myTask1", 1024, NULL, 1, NULL);
    xTaskCreate(myTask2, "myTask2", 1024, NULL, 1, NULL);
}

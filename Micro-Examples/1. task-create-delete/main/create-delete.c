#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "spi_flash_mmap.h"

void myTask(void *pvParam){
    while (true){
        // get minimum number of bytes remaining from the beginning of the task
        printf("Hello Dragos! Printing from the 'myTask' function\nWatermark=%ul\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        printf("Task myTask done 1\n");

        vTaskDelete(NULL);

        printf("Task myTask done 2\n");
    }
    printf("Task myTask done 3\n");
}

void app_main(void)
{
    TaskHandle_t myHandle = NULL;
    xTaskCreate(myTask, "myTask1", 1024, NULL, 1, &myHandle);

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    if (myHandle != NULL){
       // vTaskDelete(myHandle);
    }

    printf("Task app_main done\n");
}

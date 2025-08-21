#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t taskHandle_myTask2 = NULL;

void myTask1(void *pvParam)
{
    while (true){
        while (taskHandle_myTask2 == NULL) vTaskDelay(pdMS_TO_TICKS(1));
        
        vTaskSuspend(taskHandle_myTask2); 
        
        printf("myTask1 - collect_data_01\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);

        printf("myTask1 - collect_data_02\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        
        printf("myTask1 - collect_data_03\n");
        vTaskResume(taskHandle_myTask2);
        vTaskDelay(1000 / portTICK_PERIOD_MS);    
    }
}

void myTask2(void *pvParam)
{
    while (true){
        printf("myTask2 - send_all_data_data\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    xTaskCreate(myTask1, "myTask1", 1024, NULL, 1, NULL);
    xTaskCreate(myTask2, "myTask2", 1024, NULL, 1, &taskHandle_myTask2);
}

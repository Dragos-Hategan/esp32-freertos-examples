#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void myTask1(void *params){
    printf("WM=%u bytes\n", uxTaskGetStackHighWaterMark(NULL)*sizeof(StackType_t));
    int a = 1;
    int b = 2;
    //int c = 3;
    printf("WM=%u bytes\n", uxTaskGetStackHighWaterMark(NULL)*sizeof(StackType_t));
    while (true) {
        printf("first: myTask1!\n");
        printf("second printf: myTask1!\n");
        //printf("third printf: myTask1! %d, %d, %d\n", a, b, c);
        printf("third printf: myTask1! %d, %d\n", a, b);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("WM=%u bytes\n", uxTaskGetStackHighWaterMark(NULL)*sizeof(StackType_t));
    }
}

void app_main(void)
{
    TaskHandle_t myTask1Handle = NULL;
    xTaskCreate(myTask1, "myTask1", 1024, NULL, 1, &myTask1Handle);

    UBaseType_t iFreeStackNum = 0;
    while (true){
        iFreeStackNum = uxTaskGetStackHighWaterMark(myTask1Handle);
        printf("iFreeStackNum = %d\n", iFreeStackNum);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// we need to enable the use of stats formatting functions
// in sdkconfig to use vTaskList:
// configUSE_TRACE_FACILITY=y
// configUSE_STATS_FORMATTING_FUNCTIONS=y

void app_main(void)
{
    static char pcWriteBuffer[1024] = {0};

    while (1){
        vTaskList((char *) &pcWriteBuffer);
        printf("Name    State   Priority    Stack   NUM\n");
        printf("------------------------------------------------\n");
        printf("%s\n", pcWriteBuffer);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
        printf("------------------------------------------------\n");
    }
}

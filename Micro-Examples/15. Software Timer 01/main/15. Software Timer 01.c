#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

void timerCallback(TimerHandle_t xTimer)
{
    printf("Timer callback executed!\n");
}

void app_main(void)
{
    TimerHandle_t timerHandle = NULL;
    timerHandle = xTimerCreate("myTimer", pdMS_TO_TICKS(1000), pdTRUE, (void *) 0, timerCallback);

    if (timerHandle != NULL){
        xTimerStart(timerHandle, 0);
        vTaskDelay(pdMS_TO_TICKS(5000));
        xTimerStop(timerHandle, 0);
    }
}

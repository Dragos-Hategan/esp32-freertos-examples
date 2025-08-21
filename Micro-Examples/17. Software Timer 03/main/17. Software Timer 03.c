#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

void timerCallback(TimerHandle_t xTimer)
{
    const char *strTimerName;
    strTimerName = pcTimerGetName(xTimer);
    uint32_t timerID;
    timerID = (uint32_t) pvTimerGetTimerID(xTimer);

    printf("Timmer Triggered - Name = '%s', ID = %lu\n", strTimerName, timerID);
}

void app_main(void)
{
    TimerHandle_t timerHandle1 = NULL;
    TimerHandle_t timerHandle2 = NULL;
    timerHandle1 = xTimerCreate("myTimer1", pdMS_TO_TICKS(1000), pdTRUE, (void *) 0, timerCallback);
    timerHandle2 = xTimerCreate("myTimer2", pdMS_TO_TICKS(2000), pdTRUE, (void *) 1, timerCallback);

    if (timerHandle1 != NULL && timerHandle2 != NULL) {
        xTimerStart(timerHandle1, 0);
        xTimerStart(timerHandle2, 0);

        vTaskDelay(pdMS_TO_TICKS(5000));

        xTimerChangePeriod(timerHandle1, pdMS_TO_TICKS(500), 0);
        
        while (true){
            vTaskDelay(pdMS_TO_TICKS(1000));
            xTimerReset(timerHandle2, 0);
        }
    }
}

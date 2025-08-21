#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

TimerHandle_t timerHandle1;
TimerHandle_t timerHandle2;
TimerHandle_t timerHandle3;

const BaseType_t INITIAL_PERIOD_TIMER1_MS = 1000;
const BaseType_t MINIMUM_PERIOD_TIMER1_MS = INITIAL_PERIOD_TIMER1_MS / 3;

const BaseType_t INITIAL_PERIOD_TIMER2_MS = 2000;
const BaseType_t MINIMUM_PERIOD_TIMER2_MS = INITIAL_PERIOD_TIMER2_MS / 3;

const BaseType_t INITIAL_PERIOD_TIMER3_MS = 3000;
const BaseType_t MINIMUM_PERIOD_TIMER3_MS = INITIAL_PERIOD_TIMER3_MS / 3;

const int TIMER1_ID = 0;
const int TIMER2_ID = 1;
const int TIMER3_ID = 2;

static void timerCallback(TimerHandle_t timerHandle)
{
    printf("Timer %s, ID = %d expired. \n", pcTimerGetName(timerHandle), (int)pvTimerGetTimerID(timerHandle));
}

static void init_timers(void)
{
    timerHandle1 = NULL;
    timerHandle2 = NULL;
    timerHandle3 = NULL;
    
    timerHandle1 = xTimerCreate("myTimer1", pdMS_TO_TICKS(INITIAL_PERIOD_TIMER1_MS), pdTRUE, (void *) TIMER1_ID, timerCallback);
    timerHandle2 = xTimerCreate("myTimer2", pdMS_TO_TICKS(INITIAL_PERIOD_TIMER2_MS), pdTRUE, (void *) TIMER2_ID, timerCallback);    
    timerHandle3 = xTimerCreate("myTimer3", pdMS_TO_TICKS(INITIAL_PERIOD_TIMER3_MS), pdTRUE, (void *) TIMER3_ID, timerCallback);

}

static void myTask(void *arg){
    printf("Task %s started.\n", pcTaskGetName(NULL));
    while (true){
        printf("Making every timer 10%% faster until one of them reaches its minimum period.\n\n");

        while(xTimerGetPeriod(timerHandle1) * portTICK_PERIOD_MS > MINIMUM_PERIOD_TIMER1_MS &&
              xTimerGetPeriod(timerHandle2) * portTICK_PERIOD_MS > MINIMUM_PERIOD_TIMER2_MS &&
              xTimerGetPeriod(timerHandle3) * portTICK_PERIOD_MS > MINIMUM_PERIOD_TIMER3_MS)
        {
            printf("\nCurrent periods: Timer1 = %d ms, Timer2 = %d ms, Timer3 = %d ms\n",
                   (int)(xTimerGetPeriod(timerHandle1) * portTICK_PERIOD_MS),
                   (int)(xTimerGetPeriod(timerHandle2) * portTICK_PERIOD_MS),
                   (int)(xTimerGetPeriod(timerHandle3) * portTICK_PERIOD_MS));
            
            const TickType_t reduced_period1 = (xTimerGetPeriod(timerHandle1) * portTICK_PERIOD_MS) - (xTimerGetPeriod(timerHandle1) * portTICK_PERIOD_MS) * 0.1;
            const TickType_t reduced_period2 = (xTimerGetPeriod(timerHandle2) * portTICK_PERIOD_MS) - (xTimerGetPeriod(timerHandle2) * portTICK_PERIOD_MS) * 0.1;
            const TickType_t reduced_period3 = (xTimerGetPeriod(timerHandle3) * portTICK_PERIOD_MS) - (xTimerGetPeriod(timerHandle3) * portTICK_PERIOD_MS) * 0.1;

            xTimerChangePeriod(timerHandle1, pdMS_TO_TICKS(reduced_period1), 0);
            xTimerChangePeriod(timerHandle2, pdMS_TO_TICKS(reduced_period2), 0);
            xTimerChangePeriod(timerHandle3, pdMS_TO_TICKS(reduced_period3), 0);
            
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        printf("\n\nResetting all timers to their original values.\n\n");
        xTimerChangePeriod(timerHandle1, pdMS_TO_TICKS(INITIAL_PERIOD_TIMER1_MS), 0);
        xTimerChangePeriod(timerHandle2, pdMS_TO_TICKS(INITIAL_PERIOD_TIMER2_MS), 0);
        xTimerChangePeriod(timerHandle3, pdMS_TO_TICKS(INITIAL_PERIOD_TIMER3_MS), 0);
    }
}

void app_main(void)
{
    init_timers();

    if (timerHandle1 != NULL && timerHandle2 != NULL && timerHandle3 != NULL) {
        if (xTimerStart(timerHandle1, 0)){
            printf("Timer %s, ID = %d started successfully.\n", pcTimerGetName(timerHandle1), (int)pvTimerGetTimerID(timerHandle1));
        }else{
            printf("Failed to start Timer %s.\n", pcTimerGetName(timerHandle1));
        }
        if (xTimerStart(timerHandle2, 0)){
            printf("Timer %s, ID = %d started successfully.\n", pcTimerGetName(timerHandle2), (int)pvTimerGetTimerID(timerHandle2));
        }else{
            printf("Failed to start Timer %s.\n", pcTimerGetName(timerHandle2));
        }
        if (xTimerStart(timerHandle3, 0)){
            printf("Timer %s, ID = %d started successfully.\n", pcTimerGetName(timerHandle3), (int)pvTimerGetTimerID(timerHandle3));
        }else{
            printf("Failed to start Timer %s.\n", pcTimerGetName(timerHandle3));
        }
    }

    xTaskCreate(myTask, "Timers Play", 1024 * 2, NULL, 5, NULL);
}

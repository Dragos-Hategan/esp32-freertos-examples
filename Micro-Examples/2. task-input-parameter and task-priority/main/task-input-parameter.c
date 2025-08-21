#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "spi_flash_mmap.h"

int testNum = 1;
int testArray[] = {5, 6, 7, 8};

void myTask1(void *pvParam)
{
    printf("before pointer - myTaskInt = %u bytes\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    printf("before pointer - myTaskInt = %u words\n", uxTaskGetStackHighWaterMark(NULL));
    int *pInt;
    pInt = (int *) pvParam;
    volatile int a = 7, b = 10;      // volatile = nu optimizează
    volatile int c = a - b;

    uint8_t bigbuf[1024];            // alocare clară pe stivă
    bigbuf[0] = (uint8_t)c;
    printf("after pointer - myTaskInt = %u bytes\n\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    printf("after pointer - myTaskInt = %u words\n", uxTaskGetStackHighWaterMark(NULL));
    
    printf("before printf - myTaskInt = %u bytes\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    printf("Task 'myTaskInt' started with parameter: %d\n", *pInt);
    printf("after printf - myTaskInt = %u bytes\n\n\n\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));

    vTaskDelete(NULL);
}

void myTask2(void *pvParam)
{
    printf("before pointer - myTaskIntArr = %u bytes\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    int *pIntArr;
    pIntArr = (int *) pvParam;
    printf("after pointer - myTaskIntArr = %u bytes\n\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    
    printf("before printf - myTaskIntArr = %u bytes\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    printf("Task 'myTaskIntArr' started with parameters: %d, %d, %d, %d\n", *(pIntArr), *(pIntArr + 1), *(pIntArr + 2), *(pIntArr + 3));
    printf("after printf - myTaskIntArr = %u bytes\n\n\n\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));

    vTaskDelete(NULL);
}

typedef struct A_STRUCT
{
    int imember1;
    int imember2
} xStruct;

xStruct myStruct = {6, 9};

void myTask3(void *pvParam)
{
    printf("before pointer - myTaskStruct = %u bytes\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    xStruct *pStructTest;
    pStructTest = (xStruct *) pvParam;
    printf("after pointer - myTaskStruct = %u bytes\n\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    
    printf("before printf - myTaskStruct = %u bytes\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
    printf("Task 'myTaskStruct' started with parameters: %d, %d\n", pStructTest->imember1, pStructTest->imember2);
    printf("after printf - myTaskStruct = %u bytes\n\n\n\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));

    vTaskDelete(NULL);
}

static const char *pcTxt = "Salam!";

void myTask4(void *pvParam)
{
    char *pTxtInTask;
    pTxtInTask = (char *) pvParam;

    printf("Task 'myTaskString' started with parameters: %s\n", pTxtInTask);

    vTaskDelete(NULL);
}

static void creating_tasks(void)
{
    printf("before tasks - app_main = %u bytes\n\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));

    xTaskCreate(myTask1, "myTaskInt", 4096, (void *) &testNum, 5, NULL);
    xTaskCreate(myTask2, "myTaskIntArr", 4096, (void *) testArray, 4, NULL);
    xTaskCreate(myTask3, "myTaskStruct", 4096, (void *) &myStruct, 3, NULL);

    printf("after tasks - app_main = %u bytes\n\n", uxTaskGetStackHighWaterMark(NULL) * sizeof(StackType_t));
}

static void handler_priority_set(void)
{
    TaskHandle_t myTask4Handle = NULL;
    xTaskCreate(myTask4, "myTaskString", 4096, (void *) pcTxt, 24, &myTask4Handle);
    UBaseType_t myTask4Priority = uxTaskPriorityGet(myTask4Handle);
    printf("initial priority: %u\n", myTask4Priority);
    
    vTaskPrioritySet(myTask4Handle, 5);
    myTask4Priority = uxTaskPriorityGet(myTask4Handle);
    printf("after set priority: %u\n", myTask4Priority);

}

void myTask5(void *pvParam)
{
    while (true){
        printf("myTask4\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}

void myTask6(void *pvParam)
{
    while (true){
        printf("myTask5\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

}

void app_main(void)
{
    // creating_tasks();
    // handler_priority_set();

    vTaskSuspendAll(); // Suspend all tasks to ensure the following tasks are created without interference
    xTaskCreate(myTask5, "myTask5", 1024, NULL, 1, NULL);
    xTaskCreate(myTask6, "myTask6", 1024, NULL, 2, NULL);
    xTaskResumeAll(); // Resume all tasks

}

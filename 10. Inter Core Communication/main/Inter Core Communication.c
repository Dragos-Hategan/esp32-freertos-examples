#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_random.h"
#include "bootloader_random.h"

const uint32_t MINIMUM_RANGE_NUMBER = 10;
const uint32_t MAXIMUM_RANGE_NUMBER = 100;

static uint32_t get_random_number_in_range(const uint32_t MIN, const uint32_t MAX)
{
    bootloader_random_enable();
    uint32_t randomNumber = esp_random();   
    bootloader_random_disable();
    return MIN + (randomNumber % (MAX - MIN + 1)); 
}

static void send_number(QueueHandle_t queueHandle)
{
    uint32_t sentRandomNumber = 0;
    
    sentRandomNumber = get_random_number_in_range(MINIMUM_RANGE_NUMBER, MAXIMUM_RANGE_NUMBER);
    printf("Sending number %lu from task %s\n", sentRandomNumber, pcTaskGetName(NULL));
    xQueueSend(queueHandle, &sentRandomNumber, portMAX_DELAY);
}

static void receive_number(QueueHandle_t queueHandle)
{
    uint32_t receivedRandomNumber = 0;

    xQueueReceive(queueHandle, &receivedRandomNumber, portMAX_DELAY);
    printf("Received number %lu in task %s\n\n", receivedRandomNumber, pcTaskGetName(NULL));
}

static void core0_task(void *arg)
{
    QueueHandle_t queueHandle = (QueueHandle_t) arg;

    while (1)
    {
        send_number(queueHandle);
        vTaskDelay(pdMS_TO_TICKS(1000));
        receive_number(queueHandle);
    }
}

static void core1_task(void *arg)
{
    QueueHandle_t queueHandle = (QueueHandle_t) arg;
    
    while (1)
    {
        receive_number(queueHandle);
        send_number(queueHandle);
        vTaskDelay(pdMS_TO_TICKS(1000));        
    }
}

void app_main(void)
{
    QueueHandle_t queueHandle = NULL;
    queueHandle = xQueueCreate(1, sizeof(uint32_t));

    TaskHandle_t core0_task_handle = NULL;
    TaskHandle_t core1_task_handle = NULL;

    if (queueHandle != NULL)
    {   
        xTaskCreatePinnedToCore(core0_task, "Core 0 Task", 1024 * 2, queueHandle, 1, &core0_task_handle, 0);
        xTaskCreatePinnedToCore(core1_task, "Core 1 Task", 1024 * 2, queueHandle, 1, &core1_task_handle, 1);
    }
}

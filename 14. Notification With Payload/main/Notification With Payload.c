#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_random.h"
#include "bootloader_random.h"

TaskHandle_t producerHandle = NULL;
TaskHandle_t consumerHandle = NULL;

static const uint32_t ACK_MESSAGE_NUMBER = 0xA55B00B5; 

static void producer(void *arg)
{
    uint32_t counter = 1;
    const char *TASK_NAME = pcTaskGetName(NULL);
    uint32_t randomNumber;

    TickType_t last = xTaskGetTickCount();

    while (consumerHandle == NULL)
    {
        vTaskDelay(1);
    }

    while (1)
    { 
        bootloader_random_enable();
        randomNumber = esp_random();
        bootloader_random_disable();

        printf("Attempting to send number %lu from %s. Waiting for ACK message.\n", randomNumber, TASK_NAME);
        xTaskNotify(consumerHandle, randomNumber, eSetValueWithOverwrite);

        uint32_t received_ack_number;
        xTaskNotifyWait(0, 0xFFFFFFFF, &received_ack_number, portMAX_DELAY);

        if (received_ack_number == ACK_MESSAGE_NUMBER)
        {
            printf("ACK message received, iteration %lu finished.\n\n", counter);
            counter++;
        }else
        {
            printf("Wrong ACK message, aborting session.\n\n");
            vTaskDelete(consumerHandle);
            vTaskDelete(NULL);
        }

        vTaskDelayUntil(&last, pdMS_TO_TICKS(2500));
    }
}

static void consumer(void *arg)
{
    const char *TASK_NAME = pcTaskGetName(NULL);
    uint32_t receivedValue = 0;
    while (1)
    {
        xTaskNotifyWait(0, 0xFFFFFFFF, &receivedValue, portMAX_DELAY);
        printf("%s: received value is %lu, sending ACK message.\n", TASK_NAME, receivedValue);
        xTaskNotify(producerHandle, ACK_MESSAGE_NUMBER, eSetValueWithOverwrite);
    }
}

void app_main(void)
{
    xTaskCreate(consumer, "Consumer Task", 1024 * 2, NULL, 1, &consumerHandle);
    xTaskCreate(producer, "Producer Task", 1024 * 2, NULL, 1, &producerHandle);
}

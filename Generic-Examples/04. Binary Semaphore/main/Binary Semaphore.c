#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

SemaphoreHandle_t blinkSemaphore = NULL;

static void onboard_led_init(void)
{
    gpio_config_t onboard_led_gpio_cfg = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&onboard_led_gpio_cfg);
    gpio_set_level(GPIO_NUM_2, 0);
}

static void blink_onboard_led(void)
{
    gpio_set_level(GPIO_NUM_2, 1);
    vTaskDelay(pdMS_TO_TICKS(500));
    gpio_set_level(GPIO_NUM_2, 0);
    vTaskDelay(pdMS_TO_TICKS(500));
}

static void task1(void *arg)
{
    const char * TASK_NAME = pcTaskGetName(NULL);
    printf("%s started\n", TASK_NAME);
    BaseType_t semaphore_status;

    while (1){
        semaphore_status = 0;
        semaphore_status = xSemaphoreTake(blinkSemaphore, portMAX_DELAY);
        if (semaphore_status == pdTRUE){
            printf("Task '%s' has taken the semaphore\n", TASK_NAME);
            blink_onboard_led();
        }
        xSemaphoreGive(blinkSemaphore);
        vTaskDelay(pdMS_TO_TICKS(0));
    }
}

static void task2(void *arg)
{
    const uint16_t DELAY_MS = 2500;
    const char * TASK_NAME = pcTaskGetName(NULL);
    printf("%s started\n", TASK_NAME);
    BaseType_t semaphore_status;

    vTaskDelay(pdMS_TO_TICKS(10));

    while(1){
        semaphore_status = 0;
        semaphore_status = xSemaphoreTake(blinkSemaphore, portMAX_DELAY);
        if (semaphore_status == pdTRUE){
            printf("Task '%s' has taken the semaphore\n", TASK_NAME);
            printf("Holding the semaphore for %dms\n\n", DELAY_MS);
            vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
        }
        xSemaphoreGive(blinkSemaphore);
        vTaskDelay(pdMS_TO_TICKS(0));
    }
}

void app_main(void)
{
    blinkSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(blinkSemaphore);

    onboard_led_init();

    if (blinkSemaphore != NULL){
        xTaskCreate(task1, "Blink Onboard LED", 1024 * 2, NULL, 1, NULL);
        xTaskCreate(task2, "WAIT", 1024 * 2, NULL, 1, NULL);
    }   

}

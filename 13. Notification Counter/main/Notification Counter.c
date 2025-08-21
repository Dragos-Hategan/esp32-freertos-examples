#include <stdio.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RED_BUTTON_PIN GPIO_NUM_32
#define RED_BUTTON_BIT (1 << 0)

#define WHITE_BUTTON_PIN GPIO_NUM_33
#define WHITE_BUTTON_BIT (1 << 1)

#define DEBOUNCE_MS 30
static TaskHandle_t button_task_handle = NULL;

static const TickType_t PERIOD = pdMS_TO_TICKS(2500);

static void IRAM_ATTR button_isr(void *arg)
{
    static TickType_t lastTick = 0; 
    TickType_t now = xTaskGetTickCountFromISR();

    if ((now - lastTick) >= pdMS_TO_TICKS(40)) {   // 40 ms debounce
        BaseType_t xHPW = pdFALSE;
        vTaskNotifyGiveFromISR(button_task_handle, &xHPW); // or xTaskNotifyFromISR(buttonTaskHandle, 0, eIncrement, &xHPW);
        portYIELD_FROM_ISR(xHPW);
        lastTick = now;
    }
}

static void configure_buttons(void)
{
    gpio_config_t red_button_gpio_config = {
        .pin_bit_mask = (1ULL << RED_BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE        
    };
    ESP_ERROR_CHECK(gpio_config(&red_button_gpio_config));    

    gpio_config_t white_button_gpio_config = {
        .pin_bit_mask = (1ULL << WHITE_BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE        
    };
    ESP_ERROR_CHECK(gpio_config(&white_button_gpio_config));    

    // Installs the ISR once
    // For ISR in IRAM: use ESP_INTR_FLAG_IRAM.
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));

    // Attaches header for pin
    ESP_ERROR_CHECK(gpio_isr_handler_add(RED_BUTTON_PIN, button_isr, NULL));
    ESP_ERROR_CHECK(gpio_isr_handler_add(WHITE_BUTTON_PIN, button_isr, NULL));
}


static void button_task(void *arg)
{
    TickType_t last = xTaskGetTickCount();

    for (;;){
        vTaskDelayUntil(&last, PERIOD);

        uint32_t counter_presses = ulTaskNotifyTake(pdTRUE, 0);

        printf("In the last %lums, there were %lu button presses.\n\n", ((PERIOD * 1000) / configTICK_RATE_HZ), counter_presses);
    }
}

void app_main(void)
{
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, &button_task_handle);
    configure_buttons();
}

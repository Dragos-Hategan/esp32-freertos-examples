#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define BUTTON_PIN GPIO_NUM_32
#define DEBOUNCE_MS 30
static TaskHandle_t s_button_task = NULL;

static void IRAM_ATTR button_isr(void *arg)
{
    BaseType_t hp = pdFALSE;
    vTaskNotifyGiveFromISR(s_button_task, &hp);
    if (hp){
        portYIELD_FROM_ISR();
    }
}

static void button_task(void *arg)
{
    const TickType_t DELAY = pdMS_TO_TICKS(DEBOUNCE_MS);
    for (;;) {
        // waits for a “give”; multiple gives before the “take” do not compound
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // simple debounde: confirmes the level after a small delay
        if (gpio_get_level(BUTTON_PIN) == 0) {
            vTaskDelay(DELAY);
            if (gpio_get_level(BUTTON_PIN) == 0) {
                printf("Button PRESSED\n");
                // waits for the freeing so that multiple prints are avoided
                while (gpio_get_level(BUTTON_PIN) == 0) {
                    vTaskDelay(pdMS_TO_TICKS(5));
                }
            }
        }
    }
}

static void configure_button(void)
{
    gpio_config_t button_gpio_config = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE        
    };
    ESP_ERROR_CHECK(gpio_config(&button_gpio_config));    

    // Installs the ISR once
    // For ISR in IRAM: use ESP_INTR_FLAG_IRAM.
    ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_IRAM));

    // Attaches header for pin
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_PIN, button_isr, (void *)BUTTON_PIN));
}

void app_main(void)
{
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, &s_button_task);
    configure_button();
}

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
static TaskHandle_t s_button_task = NULL;

static void IRAM_ATTR button_isr(void *arg)
{
    uint32_t bit = ((gpio_num_t)arg == RED_BUTTON_PIN) ? RED_BUTTON_BIT : WHITE_BUTTON_BIT;
    BaseType_t hp = pdFALSE;
    xTaskNotifyFromISR(s_button_task, bit, eSetBits, &hp);
    if (hp){
        portYIELD_FROM_ISR();
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
    ESP_ERROR_CHECK(gpio_isr_handler_add(RED_BUTTON_PIN, button_isr, (void *)RED_BUTTON_PIN));
    ESP_ERROR_CHECK(gpio_isr_handler_add(WHITE_BUTTON_PIN, button_isr, (void *)WHITE_BUTTON_PIN));
}

static void button_task(void *arg)
{
    TickType_t last_red = 0;
    TickType_t last_white = 0;

    for (;;){
        uint32_t bits = 0;
        xTaskNotifyWait(0, UINT32_MAX, &bits, portMAX_DELAY);

        TickType_t now = xTaskGetTickCount();

        if ((bits & RED_BUTTON_BIT) && (now - last_red > DEBOUNCE_MS)) {
            last_red = now;
            if (gpio_get_level(RED_BUTTON_PIN) == 0) printf("Red Button PRESSED\n");
        }
        if ((bits & WHITE_BUTTON_BIT) && (now - last_white > DEBOUNCE_MS)) {
            last_white = now;
            if (gpio_get_level(WHITE_BUTTON_PIN) == 0) printf("White Button PRESSED\n");
        }

        if (gpio_get_level(RED_BUTTON_PIN) == 0 && gpio_get_level(WHITE_BUTTON_PIN) == 0) {
            printf("\nBoth Buttons PRESSED!\n\n");
            while (gpio_get_level(RED_BUTTON_PIN) == 0 && gpio_get_level(WHITE_BUTTON_PIN) == 0){
                vTaskDelay(5);
            }
        }  
    }
}

void app_main(void)
{
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, &s_button_task);
    configure_buttons();
}

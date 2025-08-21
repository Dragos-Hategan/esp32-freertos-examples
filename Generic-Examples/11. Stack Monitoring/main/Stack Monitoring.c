#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// --- Config for the demo ---
#define MONITOR_PERIOD_MS   1000

static TaskHandle_t s_overflow_task   = NULL;
static TaskHandle_t s_monitor_task    = NULL;
static const char *TAG = "STACKMON";

// Intentionally heavy stack user: recursive, with a sizeable local buffer.
// noinline to ensure separate stack frames.
static __attribute__((noinline)) void blow_the_stack(int depth)
{
    volatile uint8_t junk[512];
    for (int i = 0; i < (int)sizeof(junk); ++i) {
        ((volatile uint8_t*)junk)[i] = (uint8_t)i;
    }
    // Keep the compiler honest
    asm volatile ("" ::: "memory");

    if (depth > 0) {
        blow_the_stack(depth - 1);
    }
}

static void overflow_task(void *arg)
{
    // Give the monitor a moment to print a baseline
    vTaskDelay(pdMS_TO_TICKS(1500));

    // This will quickly exceed a small task stack.
    while (1) {
        blow_the_stack(10);
        vTaskDelay(pdMS_TO_TICKS(100)); // likely never reached once it overflows
    }
}

static void monitor_task(void *arg)
{
    while (1) {
        // In ESP-IDF, this reports BYTES of minimum free stack left since start.
        UBaseType_t hwm_self     = uxTaskGetStackHighWaterMark(NULL);
        UBaseType_t hwm_overflow = (s_overflow_task)
                                   ? uxTaskGetStackHighWaterMark(s_overflow_task)
                                   : 0;

        ESP_LOGI(TAG,
                 "High-water mark (bytes): monitor=%u, overflow_task=%u",
                 (unsigned)hwm_self, (unsigned)hwm_overflow);

        vTaskDelay(pdMS_TO_TICKS(MONITOR_PERIOD_MS));
    }
}

// Optional: override the default hook to print a clean message.
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // Use low-overhead printing; then abort.
    printf("\n*** Stack overflow detected in task '%s' ***\n", pcTaskName ? pcTaskName : "UNKNOWN");
    abort();
}

void app_main(void)
{
    // Monitor task: comfy stack so it won't overflow
    xTaskCreatePinnedToCore(monitor_task, "monitor", 1024 * 4/*bytes*/, NULL, 3, &s_monitor_task, tskNO_AFFINITY);

    // Overflow task: deliberately small stack to trip protection
    xTaskCreatePinnedToCore(overflow_task, "overflow", 1536/*bytes*/, NULL, 2, &s_overflow_task, tskNO_AFFINITY);
}

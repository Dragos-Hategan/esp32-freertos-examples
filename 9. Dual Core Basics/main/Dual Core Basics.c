#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_system.h"

#define RUN_WINDOW_US   (150000)   // 150 ms "work window" per round
#define ROUNDS          (3)        // how many measurement rounds
#define STACK_WORDS     (4096)     // task stack size (~16 KB)

// Handles for tasks, used to send start notifications
static TaskHandle_t hTask0 = NULL;
static TaskHandle_t hTask1 = NULL;

// Keep the ALU busy for a given time window (prevents aggressive optimization)
static inline void busy_loop_until(uint64_t until_us) {
    volatile uint32_t s = 0x12345678;
    while ((uint64_t)esp_timer_get_time() < until_us) {
        s ^= (s << 13);
        s ^= (s >> 17);
        s ^= (s << 5);
        asm volatile("" ::: "memory"); // small barrier
    }
    (void)s; // silence unused warning
}

// Wait until app_main gives the start signal
static inline void wait_for_start_signal(void) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

static void core_task(void *arg) {
    const int my_core = xPortGetCoreID();
    const char *name  = pcTaskGetName(NULL);

    // Hold until synchronized start
    wait_for_start_signal();

    for (int r = 1; r <= ROUNDS; r++) {
        uint64_t t0     = (uint64_t)esp_timer_get_time();
        uint64_t t_end  = t0 + RUN_WINDOW_US;

        busy_loop_until(t_end);

        uint64_t t1 = (uint64_t)esp_timer_get_time();
        printf("[%s][core %d] round %d: start=%" PRIu64 "us  end=%" PRIu64 "us  dur=%" PRIu64 "us\n",
               name, my_core, r, t0, t1, (t1 - t0));

        // Small breather so logs are easier to read
        vTaskDelay(pdMS_TO_TICKS(30));
    }

    printf("[%s][core %d] done.\n", name, my_core);
    vTaskDelete(NULL);
}

void app_main(void) {
    // Create two tasks, each pinned to a different core
    xTaskCreatePinnedToCore(core_task, "TaskCore0", STACK_WORDS, NULL, 3, &hTask0, 0);
    xTaskCreatePinnedToCore(core_task, "TaskCore1", STACK_WORDS, NULL, 3, &hTask1, 1);

    // Give them a moment to start and block waiting for the notification
    vTaskDelay(pdMS_TO_TICKS(100));

    // Synchronized start: notify both tasks nearly simultaneously
    xTaskNotifyGive(hTask0);
    xTaskNotifyGive(hTask1);

    // app_main can continue doing other work; here we just wait long enough
    vTaskDelay(pdMS_TO_TICKS((RUN_WINDOW_US / 1000 + 50) * ROUNDS + 500));
}

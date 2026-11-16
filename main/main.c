#include <stdatomic.h>

#include "bsp_battery.h"
#include "bsp_button.h"
#include "bsp_display.h"
#include "bsp_pins.h"
#include "pocket-co-content.h"
#include "key_guard.h"
#include "esp_sleep.h"
#include "pocket-co-state.h"
#include "pocket-co-ui.h"
#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "pocket_co";
static QueueHandle_t s_keys;
static atomic_bool s_input_ready;
static key_guard_t s_key_guard;

typedef struct {
    uint32_t at_ms;
    bool return_home;
} co_key_t;

static uint32_t now_ms(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

// The button driver's shared timer task only enqueues intent.
static void on_key(bsp_btn_t button, bsp_btn_ev_t event, void *user)
{
    (void)user;
    if (key_guard_consume(&s_key_guard, event == BSP_BTN_RELEASE, esp_timer_get_time())) return;
    if (!s_input_ready) return;
    if (button != BSP_BTN_OK) return;
    if (event != BSP_BTN_CLICK && event != BSP_BTN_DOUBLE && event != BSP_BTN_LONG) return;
    const co_key_t key = { .at_ms = now_ms(), .return_home = event == BSP_BTN_LONG };
    (void)xQueueSend(s_keys, &key, 0);
}

void app_main(void)
{
    ESP_LOGI(TAG, "Pocket co 1.0.3: offline answer book");
    if (bsp_display_init() != ESP_OK || !bsp_lvgl_init()) {
        ESP_LOGE(TAG, "Display initialization failed");
        return;
    }
    pocket_co_state_t state;
    pocket_co_state_init(&state, now_ms());
    if (!bsp_lvgl_lock(1000)) {
        ESP_LOGE(TAG, "UI lock failed at startup");
        return;
    }
    if (!pocket_co_fonts_valid()) {
        ESP_LOGE(TAG, "Incomplete Chinese font coverage");
        bsp_lvgl_unlock();
        return;
    }
    pocket_co_ui_create();
    pocket_co_ui_render(&state, now_ms());
    bsp_lvgl_unlock();
    bsp_display_backlight(75);

    if (esp_sleep_get_wakeup_causes() & (1u << ESP_SLEEP_WAKEUP_GPIO)) {
        key_guard_arm(&s_key_guard, esp_timer_get_time(), 10LL * 1000 * 1000);
    }
    // The first screen is visible while the optional gauge becomes ready.
    const bool battery_ready = bsp_battery_init() == ESP_OK;
    s_keys = xQueueCreate(8, sizeof(co_key_t));
    if (!s_keys || bsp_button_init(on_key, NULL) != ESP_OK) {
        ESP_LOGE(TAG, "Button initialization failed");
        if (bsp_lvgl_lock(1000)) {
            pocket_co_ui_input_error();
            bsp_lvgl_unlock();
        }
        return;
    }
    key_guard_sync_released(&s_key_guard, bsp_button_read_mv() >= BSP_BTN_MV_RELEASED_MIN);
    s_input_ready = true;
    ESP_LOGI(TAG, "Ready: %u answers; Wi-Fi/Bluetooth/audio disabled", (unsigned)pocket_co_answer_count);
    uint32_t battery_at = now_ms() - 30000u;
    bool render_needed = true;
    for (;;) {
        co_key_t key;
        bool has_key = xQueueReceive(s_keys, &key, pdMS_TO_TICKS(40)) == pdTRUE;
        uint32_t now = now_ms();
        // Reject stale queued clicks rather than replaying them after rendering stalls.
        if (has_key && (uint32_t)(now - key.at_ms) < 500u) {
            if (key.return_home) {
                pocket_co_return_home(&state, now);
                render_needed = true;
            } else {
                render_needed |= pocket_co_press(&state, now, esp_random(), pocket_co_answer_count);
            }
        }
        render_needed |= pocket_co_tick(&state, now);
        int battery = -2;
        if ((uint32_t)(now - battery_at) >= 30000u) {
            battery = battery_ready ? bsp_battery_soc() : -1;
            battery_at = now;
        }
        if (bsp_lvgl_lock(100)) {
            if (render_needed || !state.dimmed) {
                pocket_co_ui_render(&state, now);
                render_needed = false;
            }
            if (battery != -2) pocket_co_ui_battery(battery);
            bsp_lvgl_unlock();
        }
        bsp_display_backlight(state.dimmed ? 12 : 75);
    }
}

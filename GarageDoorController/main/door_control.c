#include "door_control.h"
#include "door_fsm.h"
#include "led_indicator.h"
#include "mqtt_manager.h"
#include "config.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char *TAG = "Door";

QueueHandle_t g_door_cmd_queue;

/* ── GPIO ────────────────────────────────────────────────────────────── */

static void gpio_init(void)
{
    gpio_config_t trig_cfg = {
        .pin_bit_mask = (1ULL << DOOR_RELAY_PIN),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&trig_cfg));
    gpio_set_level(DOOR_RELAY_PIN, 0);

    gpio_config_t sensor_cfg = {
        .pin_bit_mask = (1ULL << DOOR_OPEN_SENSOR_PIN) |
                        (1ULL << DOOR_CLOSED_SENSOR_PIN),
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&sensor_cfg));
}

static void relay_pulse(void)
{
    ESP_LOGI(TAG, "Trigger pulse — start");
    gpio_set_level(DOOR_RELAY_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(RELAY_PULSE_MS));
    gpio_set_level(DOOR_RELAY_PIN, 0);
    ESP_LOGI(TAG, "Trigger pulse — end");
}

static door_state_t sensors_read(void)
{
    if (gpio_get_level(DOOR_CLOSED_SENSOR_PIN) == 0) return DOOR_STATE_CLOSED;
    if (gpio_get_level(DOOR_OPEN_SENSOR_PIN)   == 0) return DOOR_STATE_OPEN;
    return DOOR_STATE_UNKNOWN;
}

/* ── Task ────────────────────────────────────────────────────────────── */

void door_control_task(void *pvParameters)
{
    gpio_init();

    g_door_cmd_queue = xQueueCreate(4, sizeof(door_command_t));
    configASSERT(g_door_cmd_queue != NULL);

    door_state_t state           = sensors_read();
    door_state_t last_published  = (door_state_t)-1;
    TickType_t   transition_tick = 0;

    ESP_LOGI(TAG, "Ready — initial state: %s", door_state_to_str(state));

    while (1) {
        /* ── 1. Process any pending command ─────────────────────────── */
        door_command_t cmd;
        if (xQueueReceive(g_door_cmd_queue, &cmd, 0) == pdTRUE) {
            door_fsm_result_t r = door_fsm_process_command(state, cmd);

            if (r.trigger) {
                state = r.next;
                transition_tick = (state == DOOR_STATE_OPENING ||
                                   state == DOOR_STATE_CLOSING)
                                  ? xTaskGetTickCount() : 0;
                relay_pulse();
            }
        }

        /* ── 2. Update state from magnetic switches ──────────────────── */
        door_state_t sensor = sensors_read();
        door_state_t after  = door_fsm_apply_sensor(state, sensor);
        if (after != state) {
            ESP_LOGI(TAG, "Sensor confirmed: %s", door_state_to_str(after));
            state           = after;
            transition_tick = 0;
        }

        /* ── 3. Travel timeout (OPENING / CLOSING only) ──────────────── */
        if (transition_tick != 0 &&
            (state == DOOR_STATE_OPENING || state == DOOR_STATE_CLOSING)) {
            TickType_t elapsed = xTaskGetTickCount() - transition_tick;
            if (elapsed > pdMS_TO_TICKS(DOOR_TRAVEL_TIMEOUT_MS)) {
                ESP_LOGW(TAG, "Travel timeout — marking unknown");
                state           = DOOR_STATE_UNKNOWN;
                transition_tick = 0;
            }
        }

        /* ── 4. Publish on state change ──────────────────────────────── */
        if (state != last_published) {
            ESP_LOGI(TAG, "State: %s → %s",
                     door_state_to_str(last_published), door_state_to_str(state));
            mqtt_publish_door_state(door_state_to_str(state));
            led_indicator_set_state(state);
            last_published = state;
        }

        vTaskDelay(pdMS_TO_TICKS(SENSOR_POLL_MS));
    }
}
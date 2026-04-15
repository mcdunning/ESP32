#include "led_indicator.h"
#include "config.h"

#include "driver/ledc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdint.h>
#include <stdbool.h>

static const char *TAG = "LED";

/* ── Types ───────────────────────────────────────────────────────────── */

typedef struct { uint8_t r, g, b; } rgb_t;

/* ── State ───────────────────────────────────────────────────────────── */

static volatile door_state_t s_state = DOOR_STATE_UNKNOWN;

/* ── Color table ─────────────────────────────────────────────────────── */

static const rgb_t STATE_COLORS[] = {
    [DOOR_STATE_UNKNOWN]         = {255, 80,  0},  /* amber  — blink */
    [DOOR_STATE_OPEN]            = {  0, 255, 0},  /* green  — solid */
    [DOOR_STATE_CLOSED]          = {255,   0, 0},  /* red    — solid */
    [DOOR_STATE_OPENING]         = {  0, 255, 0},  /* green  — blink */
    [DOOR_STATE_CLOSING]         = {255,   0, 0},  /* red    — blink */
    [DOOR_STATE_STOPPED_OPENING] = {255, 80,  0},  /* amber  — solid */
    [DOOR_STATE_STOPPED_CLOSING] = {255, 80,  0},  /* amber  — solid */
};

static bool state_should_blink(door_state_t s)
{
    return s == DOOR_STATE_OPENING ||
           s == DOOR_STATE_CLOSING ||
           s == DOOR_STATE_UNKNOWN;
}

/* ── LEDC helpers ────────────────────────────────────────────────────── */

static void set_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, r);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, g);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, b);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
}

/* ── Blink task ──────────────────────────────────────────────────────── */

static void led_task(void *pvParameters)
{
    bool phase = true;

    while (1) {
        door_state_t state = s_state;
        rgb_t col = STATE_COLORS[state];

        if (state_should_blink(state) && !phase) {
            set_rgb(0, 0, 0);
        } else {
            set_rgb(col.r, col.g, col.b);
        }

        phase = !phase;
        vTaskDelay(pdMS_TO_TICKS(LED_BLINK_PERIOD_MS));
    }
}

/* ── Public API ──────────────────────────────────────────────────────── */

void led_indicator_set_state(door_state_t state)
{
    s_state = state;
}

void led_indicator_init(void)
{
    /* Shared timer for all three channels */
    ledc_timer_config_t timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,   /* 0–255 per channel */
        .freq_hz         = 5000,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    /* One channel per colour */
    const ledc_channel_config_t channels[3] = {
        { .channel   = LEDC_CHANNEL_0,
          .gpio_num  = LED_RED_PIN,
          .speed_mode = LEDC_LOW_SPEED_MODE,
          .timer_sel  = LEDC_TIMER_0,
          .duty = 0, .hpoint = 0 },
        { .channel   = LEDC_CHANNEL_1,
          .gpio_num  = LED_GREEN_PIN,
          .speed_mode = LEDC_LOW_SPEED_MODE,
          .timer_sel  = LEDC_TIMER_0,
          .duty = 0, .hpoint = 0 },
        { .channel   = LEDC_CHANNEL_2,
          .gpio_num  = LED_BLUE_PIN,
          .speed_mode = LEDC_LOW_SPEED_MODE,
          .timer_sel  = LEDC_TIMER_0,
          .duty = 0, .hpoint = 0 },
    };
    for (int i = 0; i < 3; i++) {
        ESP_ERROR_CHECK(ledc_channel_config(&channels[i]));
    }

    xTaskCreate(led_task, "led_indicator", 2048, NULL, 3, NULL);

    ESP_LOGI(TAG, "Initialized — R=GPIO%d G=GPIO%d B=GPIO%d",
             LED_RED_PIN, LED_GREEN_PIN, LED_BLUE_PIN);
}
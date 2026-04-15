#include "door_control.h"
#include "led_indicator.h"
#include "mqtt_manager.h"
#include "wifi_manager.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

static const char *TAG = "Main";

void app_main(void)
{
    ESP_LOGI(TAG, "Garage Door Controller starting");

    /* ── System init (must be first) ─────────────────────────────────── */
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* ── LED indicator — init early so it shows state from boot ─────── */
    led_indicator_init();

    /* ── Network — blocks until IP is obtained ───────────────────────── */
    wifi_manager_init();

    /* ── MQTT — connects in background, reconnects automatically ─────── */
    mqtt_manager_init();

    /* ── Door control task ───────────────────────────────────────────── */
    xTaskCreate(door_control_task, "door_ctrl",
                /*stack=*/4096, NULL, /*priority=*/5, NULL);

    ESP_LOGI(TAG, "System ready");
}
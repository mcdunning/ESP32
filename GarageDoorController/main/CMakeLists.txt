idf_component_register(
    SRCS
        "main.c"
        "wifi_manager.c"
        "mqtt_manager.c"
        "door_control.c"
        "led_indicator.c"
    INCLUDE_DIRS "."
    REQUIRES
        esp_driver_gpio
        esp_driver_ledc
        nvs_flash
        esp_wifi
        esp_netif
        esp_event
        espressif__mqtt
)
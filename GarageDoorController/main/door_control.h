#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef enum {
    DOOR_STATE_UNKNOWN = 0,
    DOOR_STATE_OPEN,
    DOOR_STATE_CLOSED,
    DOOR_STATE_OPENING,
    DOOR_STATE_CLOSING,
    DOOR_STATE_STOPPED_OPENING,  /* mid-travel stop while opening */
    DOOR_STATE_STOPPED_CLOSING,  /* mid-travel stop while closing */
} door_state_t;

typedef enum {
    DOOR_CMD_OPEN,
    DOOR_CMD_CLOSE,
    DOOR_CMD_TOGGLE,
} door_command_t;

/**
 * @brief Command queue. Created inside door_control_task at startup.
 *        External code should check for NULL before posting.
 */
extern QueueHandle_t g_door_cmd_queue;

/** @brief Convert a door_state_t to its MQTT payload string. */
const char *door_state_to_str(door_state_t state);

/** @brief FreeRTOS task — pass to xTaskCreate. */
void door_control_task(void *pvParameters);
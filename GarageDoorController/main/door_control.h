#pragma once

#include "door_fsm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/**
 * @brief Command queue. Created inside door_control_task at startup.
 *        External code should check for NULL before posting.
 */
extern QueueHandle_t g_door_cmd_queue;

/** @brief FreeRTOS task — pass to xTaskCreate. */
void door_control_task(void *pvParameters);
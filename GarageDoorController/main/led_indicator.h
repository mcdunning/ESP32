#pragma once

#include "door_control.h"

/**
 * @brief Configure LEDC channels for the RGB LED and start the blink task.
 *        Call once from app_main after gpio/ledc drivers are ready.
 */
void led_indicator_init(void);

/**
 * @brief Update the LED to reflect a new door state.
 *        Safe to call from any task.
 *
 *  CLOSED  → solid red
 *  OPEN    → solid green
 *  CLOSING → blinking red
 *  OPENING → blinking green
 *  UNKNOWN → blinking amber
 */
void led_indicator_set_state(door_state_t state);
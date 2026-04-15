#pragma once

/**
 * @brief Initialize and start the MQTT client.
 *        On connect: publishes "online" to the availability topic and
 *        subscribes to the command topic.
 *        Reconnects automatically on disconnect (handled by the ESP MQTT stack).
 */
void mqtt_manager_init(void);

/**
 * @brief Publish the current door state string to MQTT_TOPIC_STATE.
 *        Safe to call while disconnected — the publish will be dropped
 *        and the state will be re-published on the next change.
 *
 * @param state_str  One of "open", "closed", "opening", "closing", "unknown"
 */
void mqtt_publish_door_state(const char *state_str);
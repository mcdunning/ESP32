#pragma once

/**
 * @brief Initialize WiFi in STA mode and block until the first IP address is
 *        obtained. Reconnects automatically on subsequent disconnections.
 *
 * Requires esp_netif_init() and esp_event_loop_create_default() to have been
 * called before this function.
 */
void wifi_manager_init(void);
#include "mqtt_manager.h"
#include "door_control.h"
#include "config.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include <string.h>

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t s_client;

/* ── Event handler ───────────────────────────────────────────────────── */

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                                int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    switch ((esp_mqtt_event_id_t)event_id) {

        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Connected to broker");
            /* Announce presence and subscribe to commands */
            esp_mqtt_client_publish(s_client, MQTT_TOPIC_AVAIL,
                                    "online", 0, /*qos=*/1, /*retain=*/true);
            esp_mqtt_client_subscribe(s_client, MQTT_TOPIC_CMD, /*qos=*/1);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Disconnected from broker");
            break;

        case MQTT_EVENT_DATA: {
            /* Safely copy topic and payload — event strings are not NUL-terminated */
            char payload[32] = {0};
            int plen = event->data_len < (int)(sizeof(payload) - 1)
                       ? event->data_len : (int)(sizeof(payload) - 1);
            strncpy(payload, event->data, plen);

            ESP_LOGI(TAG, "Command received: \"%s\"", payload);

            door_command_t cmd;
            if (strcasecmp(payload, "OPEN")   == 0) {
                cmd = DOOR_CMD_OPEN;
            } else if (strcasecmp(payload, "CLOSE")  == 0) {
                cmd = DOOR_CMD_CLOSE;
            } else if (strcasecmp(payload, "TOGGLE") == 0) {
                cmd = DOOR_CMD_TOGGLE;
            } else {
                ESP_LOGW(TAG, "Unknown command: \"%s\" — ignored", payload);
                break;
            }

            /* Post to door task; drop silently if queue is full */
            if (g_door_cmd_queue != NULL) {
                if (xQueueSend(g_door_cmd_queue, &cmd, 0) != pdTRUE) {
                    ESP_LOGW(TAG, "Command queue full — dropped");
                }
            }
            break;
        }

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error");
            break;

        default:
            break;
    }
}

/* ── Public API ──────────────────────────────────────────────────────── */

void mqtt_manager_init(void)
{
    esp_mqtt_client_config_t cfg = {
        .broker.address.uri      = MQTT_BROKER_URI,
        .credentials.client_id   = MQTT_CLIENT_ID,
        .session.last_will = {
            .topic  = MQTT_TOPIC_AVAIL,
            .msg    = "offline",
            .qos    = 1,
            .retain = true,
        },
    };

    /* Conditionally set credentials only when non-empty */
    if (MQTT_USERNAME[0] != '\0') {
        cfg.credentials.username              = MQTT_USERNAME;
        cfg.credentials.authentication.password = MQTT_PASSWORD;
    }

    s_client = esp_mqtt_client_init(&cfg);
    configASSERT(s_client != NULL);

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(
        s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(s_client));

    ESP_LOGI(TAG, "Client started — broker: %s", MQTT_BROKER_URI);
}

void mqtt_publish_door_state(const char *state_str)
{
    if (s_client == NULL) return;

    int msg_id = esp_mqtt_client_publish(s_client, MQTT_TOPIC_STATE,
                                          state_str, 0, /*qos=*/1, /*retain=*/true);
    if (msg_id < 0) {
        ESP_LOGW(TAG, "Publish failed (not connected)");
    } else {
        ESP_LOGI(TAG, "Published \"%s\" → %s", state_str, MQTT_TOPIC_STATE);
    }
}
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

#define WIFI_SSID "CLARO_2G6A2A6C"
#define WIFI_PASS "33326479"
#define LED 2

static const char *TAG = "MQTT_EXAMPLE";

#include "cJSON.h"

void LEDBlink(uint32_t times, uint32_t onDelay, uint32_t offDelay)
{
    printf("times: %ld\tonDelay: %ld\toffDelay: %ld\n", times, onDelay, offDelay);
    for (size_t i = 0; i < times; i++)
    {
        gpio_set_level(LED, 1);
        vTaskDelay(pdMS_TO_TICKS(onDelay));
        gpio_set_level(LED, 0);
        vTaskDelay(pdMS_TO_TICKS(offDelay));
    }
    
}

static void wifi_init(void)
{
    esp_netif_init(); // initialize library for TCP/IP and WiFi
    esp_event_loop_create_default(); // default even loop to process events like connected, disconnected...
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // macro for default configs for wifi
    esp_wifi_init(&cfg);

    esp_wifi_set_mode(WIFI_MODE_STA); // STAtion mode (ESP is now the client)

    wifi_config_t wifi_config = {0}; // wifi "access" configs
    strncpy((char*)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));

    esp_wifi_set_config(WIFI_IF_STA, &wifi_config); // set the configs
    esp_wifi_start();
    esp_wifi_connect();

    ESP_LOGI(TAG, "Connecting to Wi-Fi %s...", WIFI_SSID);

    while (1)
    {
        esp_netif_ip_info_t ip_info;

        // gets the station interface handle
        esp_netif_t* sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

        // reads current IP from the interface
        if (esp_netif_get_ip_info(sta_netif, &ip_info) == ESP_OK) {
            if (ip_info.ip.addr != 0) { // compares is the IP is non zero (success)
                ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&ip_info.ip));
                LEDBlink(2, 200, 200);
                break;
            }
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void LEDControll(cJSON *root)
{
    uint32_t times = cJSON_GetObjectItem(root, "times")->valueint;
    uint32_t onDelay = cJSON_GetObjectItem(root, "on_delay")->valueint;
    uint32_t offDelay = cJSON_GetObjectItem(root, "off_delay")->valueint;

    LEDBlink(times, onDelay, offDelay);
}

static void mqtt_event_handler(void *handler_args,
                               esp_event_base_t base,
                               int32_t event_id,
                               void *event_data)
{
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;

    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Connected");
            esp_mqtt_client_subscribe(event->client, "blumenau/aronimo/esp32", 0);
            LEDBlink(2, 200, 200);
            break;

        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT DATA received on topic %.*s: %.*s",
                     event->topic_len, event->topic,
                     event->data_len, event->data);

            char msg[128];
            if (event->data_len >= sizeof(msg)) {
                ESP_LOGE(TAG, "MQTT message too long");
            }
            memcpy(msg, event->data, event->data_len);
            msg[event->data_len] = '\0';

            printf("JSON: %s\n", msg);

            cJSON *root = cJSON_Parse(msg);
            if (root == NULL) {
                ESP_LOGE(TAG, "Invalid JSON");
                return;
            }

            cJSON *cmd_item = cJSON_GetObjectItem(root, "cmd");
            if (!cJSON_IsString(cmd_item)) {
                ESP_LOGE(TAG, "cmd is missing or not a string");
                cJSON_Delete(root);
                return;
            }
            const char *cmd = cmd_item->valuestring;

            if (strcmp(cmd, "LED_CTRL") == 0) {
                LEDControll(root);
            }
            break;

            cJSON_Delete(root);

        default:
            break;
    }
}

static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtt://test.mosquitto.org:1883",
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void app_main(void)
{
    nvs_flash_init();
    gpio_reset_pin(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);

    wifi_init();

    // Wait a few seconds for Wi-Fi to connect
    vTaskDelay(pdMS_TO_TICKS(5000));

    mqtt_app_start();
}

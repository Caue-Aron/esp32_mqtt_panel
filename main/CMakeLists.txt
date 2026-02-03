idf_component_register(
    SRCS "esp32_mqtt_panel.c" "cJSON.c"
    INCLUDE_DIRS "."
    REQUIRES 
        esp_wifi       # for esp_wifi.h
        esp_netif      # for esp_netif.h
        nvs_flash      # for nvs_flash.h
        esp_event      # for event loop
        esp_driver_gpio         # for driver/gpio.h
        esp-mqtt       # for mqtt_client.h (must be in components/esp-mqtt)
)

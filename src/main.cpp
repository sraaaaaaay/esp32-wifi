#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include <string.h>
#include <config.h>

#define WIFI_SSID *CFG_SSID
#define WIFI_PASSWORD *CFG_PASSWORD
#define WIFI_CHECK_TAG "WIFI_CHECK"

static const char *TAG = "wifi_station";

/*
 * Initializes wifi in station mode & connects
 */
void wifi_init_sta()
{
    // Before anything, create default config & initialize wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Specify SSID/pass for network
    wifi_config_t my_cfg = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .bssid_set = false,
        }};

    // Set the mode, update config & connect
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &my_cfg);
    esp_wifi_start();
    esp_wifi_connect();
}

/*
 * Check that the ESP32 is still connected to wifi
 */
void check_wifi_status()
{
    wifi_ap_record_t ap_info;

    if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK)
    {
        ESP_LOGI(WIFI_CHECK_TAG, "Connected to Wi-Fi network: %s", ap_info.ssid);
    }
    else
    {
        ESP_LOGI(WIFI_CHECK_TAG, "Not connected to any Wi-Fi network");
    }
}

/* 
 * Entry point - initialize flash memory, check for errors, then go on to wifi init
 * Periodic check for connectedness
 */
extern "C" void app_main()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init_sta();
    ESP_LOGI(TAG, "Started wifi...");

    while (true)
    {
        check_wifi_status();
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

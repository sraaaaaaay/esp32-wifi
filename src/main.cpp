#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include <string.h>
#include <config.h>

#define WIFI_STA_TAG "WIFI_STATION"

int add (int x, int y);
void wifi_init_sta();
void check_wifi_status();

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

    // Connect to wifi in station mode
    // wifi_init_sta();

    ESP_LOGI(WIFI_STA_TAG, "Started wifi...");

    while (true)
    {
        // check_wifi_status();
        ESP_LOGI(WIFI_STA_TAG, "ESP32 waiting...");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    } 
}

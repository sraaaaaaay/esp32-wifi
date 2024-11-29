#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include <string.h>
#include <config.h>
#include <esp_http_server.h>
#include "nvs_flash.h"

httpd_handle_t start_webserver(void);
void wifi_init_sta(const char *ssid, const char *password);
void check_wifi_status();

extern TaskHandle_t wifi_task_handle;
extern TaskHandle_t server_task_handle;
httpd_handle_t server = NULL;

/*
 * Entry point - initialize flash memory, check for errors, then go on to wifi init
 * Periodic check for connectedness
 */
extern "C" void app_main()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI("WIFI STATION", "ESP_WIFI_MODE_STA");
    wifi_init_sta(CFG_SSID, CFG_PASSWORD);
}

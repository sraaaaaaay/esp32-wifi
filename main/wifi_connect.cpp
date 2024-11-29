#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <cstring>

#define WIFI_CHECK_TAG "WIFI_CHECK"
#define WIFI_STA_TAG "WIFI_STATION"
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAILED_BIT BIT1

static EventGroupHandle_t wifi_event_group;
static int retries = 0;

static void event_handler(void *handler_args, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (retries < 3)
        {
            esp_wifi_connect();
            retries++;
            ESP_LOGI(WIFI_STA_TAG, "Retrying AP connection...");
        }
        else
        {
            xEventGroupSetBits(wifi_event_group, WIFI_FAILED_BIT);
        }
        ESP_LOGI(WIFI_STA_TAG, "Failed to reconnect");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIFI_STA_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        retries = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/*
 * Initializes wifi in station mode & connects
 */
void wifi_init_sta(const char *ssid, const char *password)
{
    wifi_event_group = xEventGroupCreate();

    esp_err_t ret = nvs_flash_init();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler, NULL,
                                                        &instance_any_id));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler, NULL,
                                                        &instance_got_ip));

    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    // {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }
    // ESP_ERROR_CHECK(ret);

    // Before anything, create default config & initialize wifi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Specify SSID/pass for network
    wifi_config_t my_cfg = {
        .sta = {
            .ssid = "",
            .password = "",
            .bssid_set = false,
            //.threshold.authmode = WIFI_AUTH_WPA2_PSK,

        }};

    strncpy((char *)my_cfg.sta.ssid, ssid, sizeof(my_cfg.sta.ssid));
    strncpy((char *)my_cfg.sta.password, password, sizeof(my_cfg.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &my_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAILED_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(WIFI_STA_TAG, "connected to ap SSID:%s password:%s",
                 my_cfg.sta.ssid, my_cfg.sta.password);
    }
    else if (bits & WIFI_FAILED_BIT)
    {
        ESP_LOGI(WIFI_STA_TAG, "Failed to connect to SSID:%s, password:%s",
                 my_cfg.sta.ssid, my_cfg.sta.password);
    }
    else
    {
        ESP_LOGE(WIFI_STA_TAG, "UNEXPECTED EVENT");
    }

    // Set the mode, update config & connect
    ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(wifi_event_group);
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
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_netif.h"

TaskHandle_t server_task_handle = NULL;

/*
 * Handler function for GET request
 */
esp_err_t get_handler(httpd_req_t *req)
{
    const char* resp = "<h1>URI GET Response</h1>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/*
 * Handler function for POST request
 */
esp_err_t post_handler(httpd_req_t *req)
{
    const char* resp = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri = "/uri",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL,
};

httpd_uri_t uri_post = {
    .uri = "/uri",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL,
};

httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI("SERVER", "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
        return server;
    }

    ESP_LOGI("WEB SERVER", "Error starting server");
    return server;
}

void stop_server(httpd_handle_t server)
{
    if (server)
    {
        httpd_stop(server);
    }
}
#include <stdio.h>
#include <string.h>

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "freertos/message_buffer.h"
#include "cJSON.h"

#include "wifi.h"
#include "http_client.h"

SemaphoreHandle_t wificonnectedSemaphore;

MessageBufferHandle_t buffer_http;

void RealizaHTTPRequest(void *params)
{
    char str[600];
    size_t bytes_recebidos;
    cJSON *json;
    char *print_str;
    cJSON *temp;
    cJSON *main;
    cJSON *UTC;

    xSemaphoreTake(wificonnectedSemaphore, portMAX_DELAY);

    while (1)
    {
        ESP_LOGI("Task", "Realiza HTTP Request");
        http_request();

        bytes_recebidos = xMessageBufferReceive(buffer_http, str, sizeof(str), portMAX_DELAY);
        ESP_LOGW("TASK", "%s", str);

        json = cJSON_Parse(str);
        print_str = cJSON_Print(json);
        ESP_LOGI("TASK", "%s", print_str);

        main = cJSON_GetObjectItemCaseSensitive(json, "main");
        print_str = cJSON_Print(main);
        ESP_LOGE("TASK", "main: %s", print_str);

        temp = cJSON_GetObjectItemCaseSensitive(main, "temp");
        print_str = cJSON_Print(temp);
        ESP_LOGE("TASK", "temp: %s", print_str);

        if (cJSON_IsNumber(temp))
        {
            ESP_LOGE("TASK", "temp  = %f", temp->valuedouble);
        }

        UTC = cJSON_GetObjectItemCaseSensitive(main, "dt");
        anos = UTC/31557600


        vTaskDelay(pdMS_TO_TICKS(30000));
    }
}

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wificonnectedSemaphore = xSemaphoreCreateBinary();

    buffer_http = xMessageBufferCreate(600);

    wifi_start();

    xTaskCreate(RealizaHTTPRequest, "Processa HTTP", 4096, NULL, 2, NULL);
}

#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "freertos/message_buffer.h"

#include "wifi.h"
#include "mqtt.h"

SemaphoreHandle_t wificonnectedSemaphore;
SemaphoreHandle_t mqttconnectedSemaphore;

int cont = 0;

 
void wifiConnected(void *params)
{

    while (1)
    {
        if (xSemaphoreTake(wificonnectedSemaphore, portMAX_DELAY))
        {
            mqtt_start();
        }
    }
}


void task_media()
{
    float variavel = 0,media = 0;
while(1){
    if (cont <10)
    { 
    variavel = float(event->data) + variavel;
    }
    else
    {media = variavel/cont;
    ESP_LOGI("ENVIO", "%f",media);
    variavel = 0;
    media = 0;
    cont = 0;
    }
}

}



void comunicacao_broker(void *params)
{
    char msg[50];

    xSemaphoreTake(mqttconnectedSemaphore, portMAX_DELAY);

    mqtt_sbscribe("sensores/temperatura/BancadaP");

    while (1)
    {
        float temp = 20.0 + (float)(rand() % 10);
        sprintf(msg, "temp = %f", temp);
        ESP_LOGI("ENVIO", "%s",msg);
        mqtt_publish("sensores/temperatura/BancadaE", msg);

        vTaskDelay(pdMS_TO_TICKS(10000));
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
    mqttconnectedSemaphore = xSemaphoreCreateBinary();

    wifi_start();

    xTaskCreate(wifiConnected, "Conexao MQTT", 4096, NULL, 2, NULL);
    xTaskCreate(comunicacao_broker, "Comunicacao com o Broker", 4096, NULL, 2, NULL);
    xTaskCreate(task_media, "Media Temp", 4096, NULL, 2, NULL);
}

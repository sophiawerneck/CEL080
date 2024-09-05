#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include <freertos/queue.h>
#include <dht11.h>
#include <freertos/message_buffer.h>
#include "wifi.h"
#include "mqtt.h"

struct dht11_reading data;

int N = 10;

SemaphoreHandle_t wificonnectedSemaphore;
SemaphoreHandle_t mqttconnectedSemaphore;

QueueHandle_t myqueueTemp, myqueueUmid, myqueueMedTemp, myqueueMedUmid;

void vTaskLer(void* pvparameters);
void vTaskMedTemp(void* pvparameters);
void vTaskMedUmid(void* pvparameters);
void vTaskMQTT(void* pvparameters);

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
/*
void comunicacao_broker(void *params)
{
    char msg[50];

    xSemaphoreTake(mqttconnectedSemaphore, portMAX_DELAY);
    mqtt_sbscribe("CEL080B/Sensores/TamanhoMedia");
    while (1)
    {
        float temp = 20.0 + (float)(rand() % 10);
        sprintf(msg, "temp = %f", temp);
        ESP_LOGI("ENVIO", "%s", msg);
        mqtt_publish("sensores/temperatura/BancadaH", msg); // Trocar a letra da bancada para a letra da sua bancada

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
*/

void vTaskMQTT(void* pvparameters)
{
    int MedTemp;
    int MedUmid;

    char msgTemp[50];
    char msgUmid[50];

    xSemaphoreTake(mqttconnectedSemaphore, portMAX_DELAY);
    mqtt_sbscribe("CEL080B/Sensores/TamanhoMedia");

    ESP_LOGI("DISPLAY","Task MQTT inicializando");
    while(1)
    {
        xQueueReceive(myqueueMedTemp, &MedTemp , portMAX_DELAY);
        xQueueReceive(myqueueMedUmid, &MedUmid , portMAX_DELAY);

        sprintf(msgTemp, "MediaTemp = %d", MedTemp);
        sprintf(msgUmid, "MediaUmid = %d", MedUmid);

        mqtt_publish("CEL080B/Sensores/BancadaH/Temperatura", &MedTemp);
        mqtt_publish("CEL080B/Sensores/BancadaH/Umidade", &MedUmid);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void)
{
    DHT11_init(DHT11_PIN);

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

    myqueueTemp = xQueueCreate(10, sizeof(int)); 
    myqueueUmid = xQueueCreate(10, sizeof(int));
    myqueueMedTemp = xQueueCreate(1, sizeof(int));
    myqueueMedUmid = xQueueCreate(1, sizeof(int));

    xTaskCreate(wifiConnected, "Conexao MQTT", 4096, NULL, 2, NULL);
    //xTaskCreate(comunicacao_broker, "Comunicacao com o Broker", 4096, NULL, 2, NULL);

    xTaskCreate(vTaskLer, "Leitura", 2048, NULL, 3 , NULL);
    xTaskCreate(vTaskMedTemp, "Media_Temp", 2048, NULL, 2 , NULL);
    xTaskCreate(vTaskMedUmid, "Media_Umid", 2048, NULL, 2 , NULL);
    xTaskCreate(vTaskMQTT, "MQTT", 2048, NULL, 2 , NULL);
}

void vTaskLer(void* pvparameters)
{
    ESP_LOGI("LEITURA","Task leitura inicializando");

    while (1)
    {
        data = DHT11_read();
        xQueueSendToBack(myqueueTemp, &data.temperature, portMAX_DELAY);
        xQueueSendToBack(myqueueUmid, &data.humidity, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void vTaskMedTemp(void* pvparameters)
{
    ESP_LOGI("MEDIA_TEMP","Task Media_Temp inicializando");
    int temperatura;
    int media1;
    int soma = 0;
    int cont = 0;

    while(1)
    {   
        xQueueReceive(myqueueTemp, &temperatura, portMAX_DELAY);
        cont++;
        soma = soma + temperatura;

        if(cont == N)
        {
            media1 = soma/N;
            xQueueSendToBack(myqueueMedTemp, &media1, portMAX_DELAY);
            ESP_LOGE("MEDTEMP", "%d", media1);
            soma = 0;
            cont = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void vTaskMedUmid(void* pvparameters)
{
    ESP_LOGI("MEDIA_UMID","Task Media_Umid inicializando");
    int umidade;
    int media2;
    int soma = 0;
    int cont = 0;

    while(1)
    {   
        xQueueReceive(myqueueUmid, &umidade, portMAX_DELAY);
        cont++;
        soma = soma + umidade;
        

        if(cont == N)
        {
            media2 = soma/N;
            xQueueSendToBack(myqueueMedUmid, &media2, portMAX_DELAY);
            ESP_LOGE("MEDUMID", "%d", media2);
            soma = 0;
            cont = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}



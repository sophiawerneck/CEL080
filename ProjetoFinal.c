#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <freertos/stream_buffer.h>
#include <freertos/message_buffer.h>
#include <freertos/event_groups.h>
#include <string.h>
#include <esp_log.h>
#include <dht11.h>
#include "driver/adc.h"
#include "wifi.h"
#include "mqtt.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"


#define EV_TEMP (1 << 0)
#define EV_UMID (1 << 1)
#define EV_LUM (1 << 2)
#define EV_ALARM_TEMP (1 << 3)
#define EV_ALARM_UMID (1 << 4)
#define EV_ALARM_LUM (1 << 5)

#define MOISTURE_SENSOR_ADC_CHANNEL ADC1_CHANNEL_5 //D12
#define LDR_ADC_CHANNEL ADC1_CHANNEL_4 //D13
#define RELE_IN1_GPIO GPIO_NUM_25  //D25
#define BUZZER_GPIO GPIO_NUM_23   //D23

extern int N;
int N = 0;

struct dht11_reading data;

MessageBufferHandle_t buffer_1, buffer_2, buffer_3, buffer_4, buffer_5, buffer_6;
EventGroupHandle_t ev_group ;

SemaphoreHandle_t wificonnectedSemaphore;
SemaphoreHandle_t mqttconnectedSemaphore;

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

void vTaskLer(void* pvparameters);
void vTaskMedTemp(void* pvparameters);
void vTaskMedUmid(void* pvparameters);
void vTaskMedLum(void* pvparameters);
void vTaskDisplay(void* pvparameters);
void vTaskAlarme(void* pvparameters);

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

    esp_rom_gpio_pad_select_gpio(RELE_IN1_GPIO);
    gpio_set_direction(RELE_IN1_GPIO, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(BUZZER_GPIO);
    gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);

    buffer_1 = xMessageBufferCreate(10);
    buffer_2 = xMessageBufferCreate(10);
    buffer_3 = xMessageBufferCreate(10);
    buffer_4 = xMessageBufferCreate(16);
    buffer_5 = xMessageBufferCreate(16);
    buffer_6 = xMessageBufferCreate(16);

    ev_group = xEventGroupCreate();

    xTaskCreate(wifiConnected, "Conexao MQTT", 4096, NULL, 2, NULL);

    xTaskCreate(vTaskLer, "Leitura", 2048, NULL, 3 , NULL);
    xTaskCreate(vTaskMedTemp, "Media_Temp", 2048, NULL, 2 , NULL);
    xTaskCreate(vTaskMedUmid, "Media_Umid", 2048, NULL, 2 , NULL);
    xTaskCreate(vTaskMedLum, "Media_Lum", 2048, NULL, 2 , NULL);
    xTaskCreate(vTaskDisplay, "Display", 4096, NULL, 1 , NULL);
    xTaskCreate(vTaskAlarme, "Alarme", 2048, NULL, 1 , NULL);
}


void vTaskLer(void* pvparameters)
{
    xSemaphoreTake(mqttconnectedSemaphore,portMAX_DELAY);
    
    ESP_LOGI("LEITURA","Task leitura inicializando");

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(MOISTURE_SENSOR_ADC_CHANNEL, ADC_ATTEN_DB_11); //d12

    adc1_config_channel_atten(LDR_ADC_CHANNEL, ADC_ATTEN_DB_11); //d13

    while (1)
    {
        //Ler temperatura
        data = DHT11_read();
        xMessageBufferSend(buffer_1, &data.temperature, sizeof(data.temperature), portMAX_DELAY);

        //Ler umidade
        int valor_umidade = adc1_get_raw(MOISTURE_SENSOR_ADC_CHANNEL);
        ESP_LOGE("LEITURA", "Valor do sensor de umidade: %d", valor_umidade);
        xMessageBufferSend(buffer_2, &valor_umidade, sizeof(valor_umidade), portMAX_DELAY);

        //Ler luminosidade
        int valor_lum = adc1_get_raw(LDR_ADC_CHANNEL);
        //ESP_LOGW("LEITURA", "Valor do sensor de luminosidade: %d", valor_lum);
        xMessageBufferSend(buffer_3, &valor_lum, sizeof(valor_lum), portMAX_DELAY);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTaskMedTemp(void* pvparameters)
{
    ESP_LOGI("MEDIA_TEMP","Task Media_Temp inicializando");
    int temperatura;
    float media1;
    int soma = 0;
    int cont = 0;

    while(1)
    {   
        xMessageBufferReceive(buffer_1, &temperatura, sizeof(temperatura),portMAX_DELAY);
        //ESP_LOGE("LERTEMP", "%d",temperatura);
        cont++;
        soma = soma + temperatura;
        if(cont == 10)
        {
            media1 = soma/10;
            xMessageBufferSend(buffer_4, &media1, sizeof(media1),portMAX_DELAY);
            xEventGroupSetBits(ev_group,EV_TEMP);
            soma = 0;
            cont = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void vTaskMedUmid(void* pvparameters)
{
    ESP_LOGI("MEDIA_UMID","Task Media_Umid inicializando");

    int umidade;
    float media2, tensao2;
    int soma = 0;
    int cont = 0;

    while(1)
    {   
        xMessageBufferReceive(buffer_2, &umidade, sizeof(umidade),portMAX_DELAY);
        if(umidade != 0)
        {
            cont++;
            soma += umidade;
        }
        if(cont == 10)
        {
            media2 = soma/10;
            tensao2 = (media2/4095)*3.3*100; 
            xMessageBufferSend(buffer_5, &tensao2, sizeof(tensao2),portMAX_DELAY);
            xEventGroupSetBits(ev_group,EV_UMID);
            soma = 0;
            cont = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void vTaskMedLum(void* pvparameters)
{
    ESP_LOGI("MEDIA_LUM","Task Media_Lum inicializando");


    int luminosidade;
    float media3, tensao3;
    int soma = 0;
    int cont = 0;

    while(1)
    {   
        xMessageBufferReceive(buffer_3, &luminosidade, sizeof(luminosidade),portMAX_DELAY);
        if(luminosidade != 0)
        {
            cont++;
            soma += luminosidade;
        }
        if(cont == 10)
        {
            media3 = soma/10;
            tensao3 = (media3/4095)*3.3*100;
            xMessageBufferSend(buffer_6, &tensao3, sizeof(tensao3),portMAX_DELAY);
            xEventGroupSetBits(ev_group,EV_LUM);
            soma = 0;
            cont = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTaskDisplay(void* pvparameters)
{
    ESP_LOGI("DISPLAY","Task Display inicializando");
    EventBits_t bits;
    float temp;
    float umid;
    float lum;
    char msgTemp[50];
    char msgUmid[50];
    char msgLum[50];
    float temp_ref = 30.00;
    float umid_ref = 40;
    float lum_ref = 50.00;


    while(1)
    {
        bits = xEventGroupWaitBits(ev_group, (EV_TEMP | EV_UMID | EV_LUM),pdTRUE, pdTRUE,portMAX_DELAY);

        xMessageBufferReceive(buffer_4, &temp, sizeof(temp), portMAX_DELAY);
        xMessageBufferReceive(buffer_5, &umid, sizeof(umid), portMAX_DELAY);
        xMessageBufferReceive(buffer_6, &lum, sizeof(lum), portMAX_DELAY);
        
        if(bits == (EV_TEMP | EV_UMID | EV_LUM)) 
        {
            ESP_LOGI("DISPLAY", "Média Temperatura: %f , Média Umidade: %f %%, Média Luminosidade: %f %%",temp, umid, lum);
            sprintf(msgTemp, "%.1f", temp);
            sprintf(msgUmid, "%.1f", umid);
            sprintf(msgLum, "%.1f", lum);
            mqtt_publish("Sensores/Temperatura", msgTemp);
            mqtt_publish("Sensores/Umidade", msgUmid);
            mqtt_publish("Sensores/Luminosidade", msgLum);

        }
        if (temp >= temp_ref)
        {
            xEventGroupSetBits(ev_group, EV_ALARM_TEMP); 
        }
        if (umid <= umid_ref)
        {
            xEventGroupSetBits(ev_group, EV_ALARM_UMID); 
        } 
        if (lum >= lum_ref)
        {
            xEventGroupSetBits(ev_group, EV_ALARM_LUM); 
        } 
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTaskAlarme(void* pvparameters)
{
    EventBits_t bits2;
    char rele[2];
    while(1)
    {
        bits2 = xEventGroupWaitBits(ev_group, (EV_ALARM_TEMP | EV_ALARM_UMID | EV_ALARM_LUM), pdTRUE, pdFALSE, portMAX_DELAY);
        
        if(bits2 & EV_ALARM_TEMP)
        {
            ESP_LOGE("ALARME TEMP", "EXCEDEU A TEMPERATURA");
            
        }
        
        if(bits2 & EV_ALARM_UMID)
        {
            ESP_LOGE("ALARME UMID", "UMIDADE ABAIXO DO LIMITE");
            gpio_set_level(RELE_IN1_GPIO, 1);
            sprintf(rele, "1");
            mqtt_publish("Sensores/Rele", rele);
            gpio_set_level(BUZZER_GPIO, 1); // Ativar buzzer
            vTaskDelay(pdMS_TO_TICKS(1000)); // Manter o buzzer ligado por 1 segundo
            gpio_set_level(BUZZER_GPIO, 0); // Desativar buzzer
            ESP_LOGI("RELE", "ATIVADO");
        }
        
        else if(bits2 & EV_ALARM_LUM)
        {
            ESP_LOGE("ALARME LUM", "EXCEDEU A LUMINOSIDADE");
            
        }
        else
        {
            gpio_set_level(RELE_IN1_GPIO, 0);
            sprintf(rele, "0");
            mqtt_publish("Sensores/Rele", rele);
            ESP_LOGI("RELE", "DESATIVADO");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}



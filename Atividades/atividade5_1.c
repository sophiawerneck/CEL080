#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <freertos/stream_buffer.h>
#include <freertos/message_buffer.h>
#include <freertos/event_groups.h>
#include <string.h>
#include <esp_log.h>
#include <dht11.h>

#define EV_TEMP (1 << 0)
#define EV_UMID (1 << 1)
#define EV_ALARM_TEMP (1 << 2)
#define EV_ALARM_UMID (1 << 3)

struct dht11_reading data;

MessageBufferHandle_t buffer_1, buffer_2, buffer_3, buffer_4;
EventGroupHandle_t ev_group ;

void vTaskLer(void* pvparameters);
void vTaskMedTemp(void* pvparameters);
void vTaskMedUmid(void* pvparameters);
void vTaskDisplay(void* pvparameters);
void vTaskAlarme(void* pvparameters);

void app_main(void)
{
    DHT11_init(DHT11_PIN);

    buffer_1 = xMessageBufferCreate(10);
    buffer_2 = xMessageBufferCreate(10);
    buffer_3 = xMessageBufferCreate(16);
    buffer_4 = xMessageBufferCreate(16);

    ev_group = xEventGroupCreate();

    xTaskCreate(vTaskLer, "Leitura", 2048, NULL, 3 , NULL);
    xTaskCreate(vTaskMedTemp, "Media_Temp", 2048, NULL, 2 , NULL);
    xTaskCreate(vTaskMedUmid, "Media_Umid", 2048, NULL, 2 , NULL);
    xTaskCreate(vTaskDisplay, "Display", 2048, NULL, 1 , NULL);
    xTaskCreate(vTaskAlarme, "Alarme", 2048, NULL, 1 , NULL);
}


void vTaskLer(void* pvparameters)
{
    ESP_LOGI("LEITURA","Task leitura inicializando");

    while (1)
    {
        data = DHT11_read();
        xMessageBufferSend(buffer_1, &data.temperature, sizeof(data.temperature), portMAX_DELAY);
        //ESP_LOGW("LERTEMP", "%d",data.temperature);
        xMessageBufferSend(buffer_2, &data.humidity, sizeof(data.humidity), portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(200));
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
            xMessageBufferSend(buffer_3, &media1, sizeof(media1),portMAX_DELAY);
            xEventGroupSetBits(ev_group,EV_TEMP);
            soma = 0;
            cont = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


void vTaskMedUmid(void* pvparameters)
{
    ESP_LOGI("MEDIA_UMID","Task Media_Umid inicializando");
    int umidade;
    float media2;
    int soma = 0;
    int cont = 0;

    while(1)
    {   
        xMessageBufferReceive(buffer_2, &umidade, sizeof(umidade),portMAX_DELAY);
        cont++;
        soma = soma + umidade;
        if(cont == 10)
        {
            media2 = soma/10;
            xMessageBufferSend(buffer_4, &media2, sizeof(media2),portMAX_DELAY);
            xEventGroupSetBits(ev_group,EV_UMID);
            soma = 0;
            cont = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTaskDisplay(void* pvparameters)
{
    ESP_LOGI("DISPLAY","Task Display inicializando");
    EventBits_t bits;
    float temp;
    float umid;
    float temp_ref = 27.00;
    float umid_ref = 55.00;

    while(1)
    {
        bits = xEventGroupWaitBits(ev_group, (EV_TEMP | EV_UMID),pdTRUE, pdTRUE,portMAX_DELAY);

        xMessageBufferReceive(buffer_3, &temp, sizeof(temp), portMAX_DELAY);
        xMessageBufferReceive(buffer_4, &umid, sizeof(umid), portMAX_DELAY);
        
        if(bits == (EV_TEMP | EV_UMID)) 
        {
            ESP_LOGI("DISPLAY", "Média Temperatura: %f, Média Umidade: %f",temp, umid);
        }
        if (temp >= temp_ref)
        {
            xEventGroupSetBits(ev_group, EV_ALARM_TEMP); 
        }
        if (umid >= umid_ref)
        {
            xEventGroupSetBits(ev_group, EV_ALARM_UMID); 
        } 
        //vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTaskAlarme(void* pvparameters)
{
    EventBits_t bits2;
    while(1)
    {
        bits2 = xEventGroupWaitBits(ev_group, (EV_ALARM_TEMP | EV_ALARM_UMID), pdTRUE, pdFALSE, portMAX_DELAY);
        if(bits2 & EV_ALARM_TEMP)
        {
            ESP_LOGE("ALARME TEMP", "EXCEDEU A TEMPERATURA");
        }
        if(bits2 & EV_ALARM_UMID)
        {
            ESP_LOGE("ALARME UMID", "EXCEDEU A UMIDADE");
        }
    }
}



#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <freertos/stream_buffer.h>
#include <freertos/message_buffer.h>
#include <freertos/event_groups.h>
#include <string.h>
#include <esp_log.h>
#include <dht11.h>

#define TEMP_MAX 50
#define UMID_MAX 80

struct dht11_reading data;

MessageBufferHandle_t buffer_1, buffer_2, buffer_3, buffer_4;
//buffer_5, buffer_6;

void vTaskLer(void* pvparameters);
void vTaskMedTemp(void* pvparameters);
void vTaskMedUmid(void* pvparameters);
void vTaskDisplay(void* pvparameters);
//void vTaskAlarme(void* pvparameters);

void app_main(void)
{
    DHT11_init(DHT11_PIN);

    buffer_1 = xMessageBufferCreate(40);
    buffer_2 = xMessageBufferCreate(40);
    buffer_3 = xMessageBufferCreate(100);
    buffer_4 = xMessageBufferCreate(100);
    //buffer_5 = xMessageBufferCreate(4);
    //buffer_6 = xMessageBufferCreate(4);

    xTaskCreate(vTaskLer, "Leitura", 2048, NULL, 3 , NULL);
    xTaskCreate(vTaskMedTemp, "Media_Temp", 2048, NULL, 2 , NULL);
    xTaskCreate(vTaskMedUmid, "Media_Umid", 2048, NULL, 2 , NULL);
    xTaskCreate(vTaskDisplay, "Display", 2048, NULL, 1 , NULL);
    //xTaskCreate(vTaskAlarme, "Alarme", 2048, NULL, 1 , NULL);


}

void vTaskLer(void* pvparameters)
{
    ESP_LOGI("LEITURA","Task leitura inicializando");

    while (1)
    {
        data = DHT11_read();
        xMessageBufferSend(buffer_1, &data.temperature, sizeof(data.temperature), portMAX_DELAY);
        xMessageBufferSend(buffer_2, &data.humidity, sizeof(data.humidity), portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTaskMedTemp(void* pvparameters)
{
    ESP_LOGI("MEDIA_TEMP","Task Media_Temp inicializando");
    int temperatura;
    int media1;
    int soma = 0;
    int cont = 0;
    int numMsgTemp = 0;
    //size_t bytes_recebidos_1;

    while(1)
    {   
        xMessageBufferReceive(buffer_1, &temperatura, sizeof(temperatura), portMAX_DELAY);
        numMsgTemp++;
        while(numMsgTemp <= 10)
        {
            soma = soma + temperatura;
            cont++;
            if(cont == 10)
            {
                media1 = soma/10;
                soma = 0;
                cont = 0;
                xMessageBufferSend(buffer_3, &media1, sizeof(media1), portMAX_DELAY);
                ESP_LOGW("MEDIA_TEMP", "Media: %d",media1);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void vTaskMedUmid(void* pvparameters)
{
    ESP_LOGI("MEDIA_UMID","Task Media_Umid inicializando");
    int umidade;
    int media2;
    int soma = 0;
    int cont = 0;
    size_t bytes_recebidos_2;

    while(1)
    {   
        bytes_recebidos_2 = xMessageBufferReceive(buffer_2, &umidade, sizeof(umidade), portMAX_DELAY);
        while(bytes_recebidos_2 <= 10)
        {
            soma = soma + umidade;
            cont++;
            if(cont == 10)
            {
                media2 = soma/10;
                soma = 0;
                cont = 0;
                xMessageBufferSend(buffer_4, &media2, sizeof(media2), portMAX_DELAY);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTaskDisplay(void* pvparameters)
{
    ESP_LOGI("DISPLAY","Task Display inicializando");
    int temp;
    int umid;

    while(1)
    {
        xMessageBufferReceive(buffer_3, &temp, sizeof(temp), portMAX_DELAY);
        xMessageBufferReceive(buffer_4, &umid, sizeof(umid), portMAX_DELAY);
        ESP_LOGI("DISPLAY", "Média Temperatura: %d, Média Umidade: %d",temp, umid);
        /*
        if((temp>TEMP_MAX)||(umid)>UMID_MAX)
        {
            xMessageBufferSend(buffer_5, &temp, sizeof(temp), portMAX_DELAY);
            xMessageBufferSend(buffer_6, &umid, sizeof(umid), portMAX_DELAY);
        }
        */
        vTaskDelay(pdMS_TO_TICKS(2000));

    }
}
/*
void vTaskAlarme(void* pvparameters)
{
    int temp_alarme;
    int umid_alarme;
    while(1)
    {
        xMessageBufferReceive(buffer_5, &temp_alarme, sizeof(temp_alarme), portMAX_DELAY);
        xMessageBufferReceive(buffer_6, &umid_alarme, sizeof(umid_alarme), portMAX_DELAY);
        ESP_LOGE("ALARME", "EXCEDEU A TEMPERATURA OU UMIDADE MÁXIMAS");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
*/

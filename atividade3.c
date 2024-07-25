#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <dht11.h>


struct dht11_reading data;

struct medias
{
    float media;
};

QueueHandle_t myqueueTemp, myqueueUmid, myqueueMedTemp, myqueueMedUmid;

void vTaskLer(void* pvparameters);
void vTaskMedTemp(void* pvparameters);
void vTaskMedUmid(void* pvparameters);
void vTaskDisplay(void* pvparameters);

void app_main(void)
{
    DHT11_init(DHT11_PIN);

    myqueueTemp = xQueueCreate(10, sizeof(float)); 
    myqueueUmid = xQueueCreate(10, sizeof(float));
    myqueueMedTemp = xQueueCreate(1, sizeof(float));
    myqueueMedUmid = xQueueCreate(1, sizeof(float)); 

    xTaskCreatePinnedToCore(vTaskLer, "Leitura", 2048, NULL, 3 , NULL, 0);
    xTaskCreatePinnedToCore(vTaskMedTemp, "Media_Temp", 2048, NULL, 2 , NULL, 0);
    xTaskCreatePinnedToCore(vTaskMedUmid, "Media_Umid", 2048, NULL, 2 , NULL, 0);
    xTaskCreatePinnedToCore(vTaskDisplay, "Display", 2048, NULL, 1 , NULL, 0);
}

void vTaskLer(void* pvparameters)
{
    ESP_LOGI("LEITURA","Task leitura inicializando");

    while (1)
    {
        data = DHT11_read();
        xQueueSendToBack(myqueueTemp, &data.temperature, portMAX_DELAY);
        xQueueSendToBack(myqueueUmid, &data.humidity, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void vTaskMedTemp(void* pvparameters)
{
    ESP_LOGI("MEDIA_TEMP","Task Media_Temp inicializando");
    struct dht11_reading temp;
    struct medias media1;
    int itensLidos = 0;
    float soma;

    while(1)
    {   
        while(itensLidos<10)
        {
            if(xQueueReceive(myqueueTemp, &temp.temperature, portMAX_DELAY)==pdPASS)
            {
                soma += temp.temperature;
            }
        }
        media1.media = soma/10;
        xQueueSendToBack(myqueueMedTemp, &media1, portMAX_DELAY);
    }
}

void vTaskMedUmid(void* pvparameters)
{
    ESP_LOGI("MEDIA_UMID","Task Media_Umid inicializando");
    struct dht11_reading umid;
    int itensLidos = 0;
    float soma;
    struct medias media2;

    while(1)
    {   
        while(itensLidos<10)
        {
            if(xQueueReceive(myqueueUmid, &umid.humidity, portMAX_DELAY)==pdPASS)
            {
                soma += umid.humidity;
            }
        }
        media2.media = soma/10;
        xQueueSendToBack(myqueueMedUmid, &media2, portMAX_DELAY);
    }
}

void vTaskDisplay(void* pvparameters)
{
    ESP_LOGI("DISPLAY","Task Display inicializando");
    while(1)
    {
        xQueueReceive(myqueueMedTemp, & , portMAX_DELAY)
        xQueueReceive(myqueueMedUmid, & , portMAX_DELAY)
        ESP_LOGI("DISPLAY", "Temperature: %d, Humidity: %d", data.temperature, data.humidity);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}


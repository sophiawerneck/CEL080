#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <dht11.h>

struct dht11_reading data;

QueueHandle_t myqueueTemp, myqueueUmid, myqueueMedTemp, myqueueMedUmid;

void vTaskLer(void* pvparameters);
void vTaskMedTemp(void* pvparameters);
void vTaskMedUmid(void* pvparameters);
void vTaskDisplay(void* pvparameters);

void app_main(void)
{
    DHT11_init(DHT11_PIN);

    myqueueTemp = xQueueCreate(10, sizeof(int)); 
    myqueueUmid = xQueueCreate(10, sizeof(int));
    myqueueMedTemp = xQueueCreate(1, sizeof(int));
    myqueueMedUmid = xQueueCreate(1, sizeof(int)); 

    xTaskCreatePinnedToCore(vTaskLer, "Leitura", 2048, NULL, 1 , NULL, 0); // Verificar prioridades
    xTaskCreatePinnedToCore(vTaskMedTemp, "Media_Temp", 2048, NULL, 1 , NULL, 0);
    xTaskCreatePinnedToCore(vTaskMedUmid, "Media_Umid", 2048, NULL, 1 , NULL, 0);
    xTaskCreatePinnedToCore(vTaskDisplay, "Display", 2048, NULL, 1 , NULL, 0);
}

void vTaskLer(void* pvparameters)
{

    ESP_LOGI("LEITURA","Task leitura inicializando");

    uint16_t temperatura, umidade;
    uint16_t cont = 0;
    while (1)
    {
        data = DHT11_read();
        temperatura = data.temperature;
        umidade = data.humidity;
        cont++;

        xQueueSendToBack(myqueueTemp, &data.temperature, portMAX_DELAY);
        xQueueSendToBack(myqueueUmid, &data.humidity, portMAX_DELAY);

        if(cont==10)
        {
            cont = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vTaskMedTemp(void* pvparameters)
{
    ESP_LOGI("MEDIA_TEMP","Task Media_Temp inicializando");
    uint16_t temperatura;
    uint16_t media1;
    uint16_t soma = 0;
    uint16_t cont = 0;

    while(1)
    {   
        xQueueReceive(myqueueTemp, &temperatura, portMAX_DELAY);
        cont++;
        soma = soma + temperatura;
        if(cont == 10)
        {
            media1 = soma/10;
            soma = 0;
            cont = 0;
            xQueueSendToBack(myqueueMedTemp, &media1, portMAX_DELAY);
        }
        
    }
}

void vTaskMedUmid(void* pvparameters)
{
    ESP_LOGI("MEDIA_UMID","Task Media_Umid inicializando");
    uint16_t umidade;
    uint16_t media2;
    uint16_t soma = 0;
    uint16_t cont = 0;

    while(1)
    {   
        xQueueReceive(myqueueUmid, &umidade, portMAX_DELAY);
        cont++;
        soma = soma + umidade;
        if(cont == 10)
        {
            media2 = soma/10;
            soma = 0;
            cont = 0;
            xQueueSendToBack(myqueueMedUmid, &media2, portMAX_DELAY);
        }
        
    }
}

void vTaskDisplay(void* pvparameters)
{
    uint16_t MedTemp;
    uint16_t MedUmid;

    ESP_LOGI("DISPLAY","Task Display inicializando");
    while(1)
    {
        xQueueReceive(myqueueMedTemp, &MedTemp , portMAX_DELAY);
        xQueueReceive(myqueueMedUmid, &MedUmid , portMAX_DELAY);
        ESP_LOGI("DISPLAY", "Média Temperatura: %d, Média Umidade: %d", MedTemp, MedUmid);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

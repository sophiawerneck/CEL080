#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <dht11.h>
#include <freertos/queue.h>

#define GPIO_BT 0

void vTaskLeitura(void* pvparameters);
void vTaskMediaTemp(void* pvparameters);
void vTaskMediaUmid(void* pvparameters);
void vTaskDisplay(void* pvparameters);

struct dht11_reading data;
QueueHandle_t queueTemp;
QueueHandle_t queueUmid;
QueueHandle_t queueMediaTemp;
QueueHandle_t queueMediaUmid;

struct mystruct
{
    int temp;
    int umid;
    int cont;
};

void app_main(void)
{
    DHT11_init(DHT11_PIN);
    
    gpio_reset_pin(GPIO_BT);
    gpio_set_direction(GPIO_BT,GPIO_MODE_INPUT);

    // Criação da fila
    queueTemp = xQueueCreate(5, sizeof(struct mystruct));
    queueUmid = xQueueCreate(5, sizeof(struct mystruct));
    queueMediaTemp = xQueueCreate(5, sizeof(int));
    queueMediaUmid = xQueueCreate(5, sizeof(int));

    xTaskCreatePinnedToCore(vTaskLeitura, "Leitura", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskMediaTemp, "MediaTemp", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(vTaskMediaUmid, "MediaUmid", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(vTaskDisplay, "Display", 2048, NULL, 2, NULL, 0);
 
    
}

void vTaskLeitura(void* pvparameters)
{   
    struct mystruct st_Dados;

    ESP_LOGI("Leitura", "Task inicializando");
    while(1)
    {
        data = DHT11_read();
        st_Dados.temp = data.temperature;
        st_Dados.umid = data.humidity;
        st_Dados.cont ++;

        xQueueSendToBack(queueTemp,&st_Dados,portMAX_DELAY);
        xQueueSendToBack(queueUmid,&st_Dados,portMAX_DELAY);
        ESP_LOGI("Leitura", "Cont. CNT = %d",st_Dados.cont);

        if(st_Dados.cont == 10)
        {
            st_Dados.cont = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTaskMediaTemp(void* pvparameters)
{   
    struct mystruct st_Dados;
    int cont = 0;
    int soma = 0;
    int media = 0;

    ESP_LOGI("MediaTemp", "Task inicializando");
    while(1)
    {
        
        xQueueReceive(queueTemp, &st_Dados, portMAX_DELAY);
        soma = soma +st_Dados.temp;
        cont ++;

        if(cont == 10){
            media = soma / cont;
            soma = 0;
            cont = 0;

            xQueueSendToBack(queueMediaTemp,&media,portMAX_DELAY);
        }
      
    }
}

void vTaskMediaUmid(void* pvparameters)
{   
    struct mystruct st_Dados;
    int contU = 0;
    int somaU = 0;
    int mediaU = 0;

    ESP_LOGI("MediaUmid", "Task inicializando");
    while(1)
    {
        if(uxQueueMessagesWaiting(queueUmid) == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        xQueueReceive(queueUmid, &st_Dados, portMAX_DELAY);
        somaU = somaU + st_Dados.umid;
        contU++;

        if(contU == 10){
            mediaU = somaU / contU;
            somaU = 0;
            contU = 0;

            xQueueSendToBack(queueMediaUmid,&mediaU,portMAX_DELAY);
        }
    }
}

void vTaskDisplay(void* pvparameters)
{   
    int mediaTemp;
    int mediaUmid;

    ESP_LOGI("Display", "Task inicializando");
    while(1)
    {
        
        xQueueReceive(queueMediaTemp, &mediaTemp, portMAX_DELAY);
        ESP_LOGI("Display", "Media Temperatura = %d", mediaTemp);

        xQueueReceive(queueMediaUmid, &mediaUmid, portMAX_DELAY);
        ESP_LOGI("Display", "Media Umidade = %d", mediaUmid);
    }
}

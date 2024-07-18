#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <dht11.h>

#define GPIO_BT 0

struct dht11_reading data;

SemaphoreHandle_t sem_sinc1;
SemaphoreHandle_t sem_sinc2; // semaforo de sincronismo
SemaphoreHandle_t sem_mutex;

void vTaskBT(void* pvparameters);
void vTaskSensor(void* pvparameters);
void vTaskDisplay(void* pvparameters);

void app_main(void)
{
    DHT11_init(DHT11_PIN);

    gpio_reset_pin(GPIO_BT); 
    gpio_set_direction(GPIO_BT,GPIO_MODE_INPUT);

    sem_sinc1 = xSemaphoreCreateBinary();
    sem_sinc2 = xSemaphoreCreateBinary();
    sem_mutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(vTaskBT, "Botão", 2048, NULL, 3 , NULL, 0);
    xTaskCreatePinnedToCore(vTaskSensor, "Sensor", 2048, NULL, 2 , NULL, 0);
    xTaskCreatePinnedToCore(vTaskDisplay, "Display", 2048, NULL, 1 , NULL, 0);
}

void vTaskBT(void* pvparameters)
{
    ESP_LOGI("BOTAO","Task botao inicializando");

    while (1)
    {
        if(gpio_get_level(GPIO_BT) == 0)
        {
            xSemaphoreGive(sem_sinc2);
            ESP_LOGI("BOTAO","Botão pressionado");
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTaskSensor(void* pvparameters)
{
    ESP_LOGI("SENSOR","Task sensor inicializando");

    while (1)
    {
        xSemaphoreTake(sem_sinc2,portMAX_DELAY);
        xSemaphoreTake(sem_mutex, portMAX_DELAY);
            data = DHT11_read();
        xSemaphoreGive(sem_mutex);
        xSemaphoreGive(sem_sinc1);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTaskDisplay(void* pvparameters)
{
    ESP_LOGI("DISPLAY","Task display inicializando");

    while (1)
    {
        xSemaphoreTake(sem_sinc1,portMAX_DELAY);
        xSemaphoreTake(sem_mutex, portMAX_DELAY);
            ESP_LOGI("DISPLAY", "Temperature: %d, Humidity: %d", data.temperature, data.humidity);
        xSemaphoreGive(sem_mutex);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

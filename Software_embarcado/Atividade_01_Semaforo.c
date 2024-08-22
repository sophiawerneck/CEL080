#include <stdio.h>
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
#include<esp_log.h>
#include<freertos/semphr.h>
#include<driver/gpio.h>

//duas takss signal e wait cria o semaforo e cira o esquema 
//do slide 12 a signal tem maior prioridade
SemaphoreHandle_t bin_semaphore;
SemaphoreHandle_t mutex;
#define BT_GPIO 0
#define LED_GPIO 2
int cont = 0;
//cria os objetos e depois as tasks

void taskwait(void)
{
    BaseType_t status;
    ESP_LOGI("wait","task inicializando");
    uint8_t cnt = 0;
    while(1)
    { 

        //ESP_LOGI("wait","tenta adquirir o semaforo");
        status = xSemaphoreTake(bin_semaphore,500);
        if(status == pdTRUE)
        { 
        ESP_LOGI("wait","ACIONANDO O LED");
        xSemaphoreTake(mutex, portMAX_DELAY);
        gpio_set_level(LED_GPIO,cnt%2);//smp divide e pega o resto da divisao
        ESP_LOGI("wait"," botao pressionado %d",cont);
        xSemaphoreGive(mutex);
        }
        else
        {
         ESP_LOGE("wait"," tempo expirado");   
        }
    }
}
void tasksignal(void)
{
    
ESP_LOGI("signal","task inicializando");
    while(1)
    {
        vTaskDelay(10);
        if(gpio_get_level(BT_GPIO) == 0)
        {
            
            ESP_LOGI("signal"," botao pressionado");
            xSemaphoreTake(mutex, portMAX_DELAY);
            xSemaphoreGive(bin_semaphore);
            cont = cont +1;
            xSemaphoreGive(mutex);
        }
        
        
        


    }
}


void app_main(void)
{
    gpio_reset_pin(BT_GPIO);
    gpio_set_direction(BT_GPIO,GPIO_MODE_INPUT);
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO,GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO,0);
    mutex = xSemaphoreCreateMutex();
vSemaphoreCreateBinary(bin_semaphore);
xSemaphoreTake(bin_semaphore,0); // o take igual 0 apenas pega o semaforo para torna-lo indisponivel
xTaskCreate(taskwait,"Task wait",2048,NULL,3,NULL);
xTaskCreate(tasksignal,"Task signal",2048,NULL,2,NULL);
}

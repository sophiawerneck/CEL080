#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define BT_GPIO 0           // botao de boot gpio0  (depois de bootar ele vira um botao normal)
#define LD_GPIO 2           // led gpio2

SemaphoreHandle_t sem_sinc; // semaforo de sincronismo


void vTask_BT(void* pvparameters);
void vTask_LED(void* pvparameters);


void app_main(void)
{
    gpio_reset_pin(BT_GPIO);                          // deixa io na config default
    gpio_set_direction( BT_GPIO, GPIO_MODE_INPUT);    // primeiro argumento é o numero, segundo arg é se é input ou output

    gpio_reset_pin(LD_GPIO);
    gpio_set_direction( LD_GPIO, GPIO_MODE_OUTPUT);

    sem_sinc = xSemaphoreCreateBinary();              // semaforo binario
    if(sem_sinc != NULL)                              // teste pra ver se o semáforo foi criado corretamente
    {
        xTaskCreate(vTask_BT,"Task BT",2048,NULL,1,NULL);
        xTaskCreate(vTask_LED,"Task LED",2048,NULL,1,NULL);
    }

}

void vTask_LED(void* pvparameters)
{
    uint8_t cnt = 0;
    BaseType_t status;

    ESP_LOGI("LED","Task LED inicializando");

    while (1)
    {
        //status = xSemaphoreTake(sem_sinc,pdMS_TO_TICKS(5000));     // tenta adiquirir o semaforo, arg1: semaforo, arg2: opção de bloqueio ; retorna bool
        status = xSemaphoreTake(sem_sinc,portMAX_DELAY);             // portmaxdelay deixa no delay ate receber o semaforo, presisona o boot desbloqueia a task do led
        if(status == pdTRUE)
        {
            ESP_LOGI("LED","Task LED executando");
            gpio_set_level(LD_GPIO,(cnt++)%2);                     // primeiro argumento é o io, segundo agr é o nivel lógico
                                                                   // cnt++ incremente e %2 pega o módulo da divisão
        }
        else
        {
            ESP_LOGE("LED","Delay expirou");
        }
    }
    
}

void vTask_BT(void* pvparameters)
{
    uint8_t cnt = 0;
    
    ESP_LOGI("BOT","Task botao inicializando");

    while (1)
    {
        if(gpio_get_level(BT_GPIO) == 0)
        {
            ESP_LOGI("BOT","Botão pressionado %d vezes",++cnt);
            xSemaphoreGive(sem_sinc);                               // libera condição pro semaforo
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
}

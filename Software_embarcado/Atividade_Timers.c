#include <stdio.h>
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
#include<esp_log.h>
#include<freertos/queue.h>
#include<driver/gpio.h>
#include "freertos/stream_buffer.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"


#define BT_GPIO 0
#define LED_GPIO 2

uint8_t cnt = 0;

TimerHandle_t timer1, timer2;


void task_botao(void* parameters)
{
    while(1)
    {
        if(gpio_get_level(BT_GPIO)==0)
        {
            ESP_LOGI("Botão","Botão Pressionado");
            xTimerStart(timer2,0);
        }
        vTaskDelay(pdTICKS_TO_MS(2));
    }
}

void timer1_cb(TimerHandle_t timer_handle)
{

ESP_LOGI("TIMER","Estouro do Timer");
cnt++;
gpio_set_level(LED_GPIO, (cnt%2));

}

void timer2_cb(TimerHandle_t timer_handle)
{

ESP_LOGI("TIMER","Estouro do Timer 2");


}

void app_main(void)
{
gpio_reset_pin(BT_GPIO); // reseta o gpio
gpio_set_direction(BT_GPIO,GPIO_MODE_INPUT); // define o botão como entrada

gpio_reset_pin(LED_GPIO); // reseta o gpio
gpio_set_direction(LED_GPIO,GPIO_MODE_OUTPUT); // define o led como saida

timer1 = xTimerCreate("Timer 1", pdTICKS_TO_MS(20), pdTRUE, 0, timer1_cb);
timer2 = xTimerCreate("Timer 2", pdTICKS_TO_MS(10), pdFALSE,0,  timer2_cb);
ESP_LOGI("MAIN","Inicialização do TIMER");
xTimerStart(timer1,0);

xTaskCreate(task_botao,"task Botão",4096,NULL,1,NULL);
}

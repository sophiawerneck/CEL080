#include <stdio.h>
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
#include<esp_log.h>
#include<freertos/queue.h>
#include <driver/gpio.h>
#include "freertos/stream_buffer.h"
#include "freertos/event_groups.h"
#include "freertos/timers.h"


#define BT_GPIO 0

uint8_t flag = 0;
int cont = 0;
int contM = 0;

TaskHandle_t ISR = NULL;
TimerHandle_t timer2, timer1;

char dados[100];
int contvect = 0;




void IRAM_ATTR button_isr_handler(void)
{

xTaskResumeFromISR(ISR);

}




void timer1_cb(TimerHandle_t timer_handle)
{

    if((gpio_get_level(BT_GPIO))==0)
    {
        dados[contvect] = '-';
        contvect++;
        xTimerStart(timer2,0);
        contM = cont;
    }
    else
    {
        dados[contvect] = '.';
        contvect++;
        contM = cont;
        xTimerStart(timer2,0);
    }


}

void timer2_cb(TimerHandle_t timer_handle)
{

    if(cont == contM)
    {
    dados[contvect]=' ';
    contvect = contvect + 1;

    ESP_LOGI("Fecha Transmissão", "%s",dados);

    cont = 0;
    contM = 0;
    }


}

void task_botao(void)
{
	while(1)
	{
    	
        flag = 1;
        cont = cont + 1;
        vTaskSuspend(NULL); // A task roda só uma vez %%%%%%%% Voltar para antes na função se der erro %%%%%%%%%%%%
		
	}
}

void task_libera(void)
{
	while(1)
	{
        if(flag == 1)
        {
    	    xTimerStart(timer1,0);
        }	
        flag == 0;
	}


}


void app_main(void)
{

gpio_reset_pin(BT_GPIO); // reseta o gpio
gpio_set_direction(BT_GPIO,GPIO_MODE_INPUT); // define o botão como entrada
gpio_set_intr_type(BT_GPIO, GPIO_INTR_NEGEDGE); // define o interrupt na borda negativa do botão
gpio_install_isr_service(ESP_INTR_FLAG_SHARED); // instala o ISR com handler (permite interrupções do GPIO por pino) e instala o ISR com configuração padrão
gpio_isr_handler_add(BT_GPIO, button_isr_handler, NULL); // adiciona o handler do botão

ESP_LOGI("MAIN","Inicialização do TIMERS");
timer1 = xTimerCreate("Timer 1", pdMS_TO_TICKS(200), pdFALSE,0,  timer1_cb);
timer2 = xTimerCreate("Timer 2", pdMS_TO_TICKS(3000), pdFALSE,0,  timer2_cb);
ESP_LOGI("MAIN","Inicialização da task INTERRUPT");

xTaskCreate(task_botao, "task botão",4096,NULL, 3, &ISR); 

ESP_LOGI("MAIN","Inicialização da task libera");

xTaskCreate(task_libera, "task libera",4096,NULL,2, NULL);



}

#include <stdio.h>
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
#include<esp_log.h>
#include<freertos/queue.h>
#include<driver/gpio.h>
#include "freertos/stream_buffer.h"
#include <dht11.h>
#include "freertos/event_groups.h"

#define BT_GPIO 0
TaskHandle_t ISR = NULL;
int limite = 20;

static const char *TAG = "MyApp";
struct dht11_reading sensor;
StreamBufferHandle_t buffer_temp;
StreamBufferHandle_t buffer_hum;
StreamBufferHandle_t buffer_temp_disp;
StreamBufferHandle_t buffer_hum_disp;
#define bit_humidade (1 << 0)
#define bit_temperatura (1 << 1)

EventGroupHandle_t event_g;


void IRAM_ATTR button_isr_handler(void)
{

xTaskResumeFromISR(ISR);

}

void task_botao(void)
{
	while(1)
	{
		vTaskSuspend(NULL); // A task roda só uma vez
		
		limite = limite + 1;
		
		ESP_LOGI(TAG,"\nLimite = %d",limite );
	}


}

void task_alarme(void)
{
 int data_temp;
 size_t bytes_recebidos_temp_disp;
	while(1)
	{
		
        //ESP_LOGI(TAG,"\nTemperatura ATUAL %d°C !!!!!!",sensor.temperature);
		if(sensor.temperature>limite)
		{
			ESP_LOGI(TAG,"\nTemperatura ULTRAPASSOU O LIMITE de %d°C !!!!!!",limite );
		}
        vTaskDelay(100);
	}



}


void task_med(void)
{
  BaseType_t status_temp; 
  BaseType_t  status_hum;
  size_t bytes_env_temp;
  size_t bytes_env_hum;
  uint8_t cntS = 0;
while(1)
{ 
    sensor  = DHT11_read();
    bytes_env_temp = xStreamBufferSend(buffer_temp, &sensor.temperature, sizeof(sensor.temperature), portMAX_DELAY);
    bytes_env_hum = xStreamBufferSend(buffer_hum, &sensor.humidity, sizeof(sensor.humidity), portMAX_DELAY);

//ESP_LOGI(TAG,"\n TEMPERATURA agora :   %d",sensor.temperature);
//ESP_LOGI(TAG,"\nHUMIDADE agora:   %d",sensor.humidity);
    
    vTaskDelay(100);
}}


void task_media_temp(void)
{
EventBits_t bit_temp;
int cont =0;
int data_temp;
size_t bytes_recebidos_temp;
size_t bytes_env_temp_disp;


int vetTemp =0 ;

while(1)
{ 

bytes_recebidos_temp = xStreamBufferReceive(buffer_temp, &data_temp, sizeof(data_temp), portMAX_DELAY);
vetTemp = data_temp + vetTemp;


    if (cont ==9)
{
    int media_temp = vetTemp/10;
    
    bytes_env_temp_disp = xStreamBufferSend(buffer_temp_disp, &media_temp, sizeof(media_temp), portMAX_DELAY);
    vetTemp =0;
    cont = 0;
    bit_temp = xEventGroupSetBits(event_g, bit_temperatura);
  
}
else
{
    cont++;
}       
}}


void taks_media_hum(void)
{
EventBits_t bit_hum;
int cont =0;
int data_hum;
size_t bytes_recebidos_hum;
size_t bytes_env_hum_disp;

int vethum =0;

while(1)
{ 

bytes_recebidos_hum = xStreamBufferReceive(buffer_hum, &data_hum, sizeof(data_hum), portMAX_DELAY);
vethum = data_hum + vethum;


if (cont ==9)
{
    int media_hum = vethum/10;
    
    bytes_env_hum_disp = xStreamBufferSend(buffer_hum_disp, &media_hum, sizeof(media_hum), portMAX_DELAY);
    vethum = 0;
    cont = 0;
    bit_hum = xEventGroupSetBits(event_g, bit_humidade);
   
}
else
{

    cont++;
}       
}}


void display(void)
{
    uint16_t data_hum_display;
    uint16_t data_temp_display;
    size_t bytes_recebidos_hum_disp;
    size_t bytes_recebidos_temp_disp;
    int data_hum;
    int data_temp;
    EventBits_t bits;
while(1)
{ 

bits = xEventGroupWaitBits(event_g, bit_humidade | bit_temperatura, pdTRUE, pdTRUE, portMAX_DELAY);
bytes_recebidos_hum_disp = xStreamBufferReceive(buffer_hum_disp, &data_hum, sizeof(data_hum), portMAX_DELAY);
bytes_recebidos_temp_disp = xStreamBufferReceive(buffer_temp_disp, &data_temp, sizeof(data_temp), portMAX_DELAY);

ESP_LOGI(TAG,"\nTEMPERATURA MEDIA:   %d",data_temp);
ESP_LOGI(TAG,"\nHUMIDADE MEDIA:   %d",data_hum);
}}


void app_main(void)
{
gpio_reset_pin(BT_GPIO); // reseta o gpio
gpio_set_direction(BT_GPIO,GPIO_MODE_INPUT); // define o botão como entrada
gpio_set_intr_type(BT_GPIO, GPIO_INTR_NEGEDGE); // define o interrupt na borda negativa do botão
gpio_install_isr_service(ESP_INTR_FLAG_SHARED); // instala o ISR com handler (permite interrupções do GPIO por pino) e instala o ISR com configuração padrão
gpio_isr_handler_add(BT_GPIO, button_isr_handler, NULL); // adiciona o handler do botão


DHT11_init(DHT11_PIN);
buffer_temp = xStreamBufferCreate(10, 1);
buffer_hum = xStreamBufferCreate(10, 1);
buffer_temp_disp = xStreamBufferCreate(10, 1);
buffer_hum_disp = xStreamBufferCreate(10, 1);
event_g = xEventGroupCreate();
xTaskCreate(task_botao, "task botão",4096,NULL,2, &ISR); // cria a task botao
xTaskCreate(task_alarme, "task alarme",4096,NULL,3, NULL); // cria a task alarme
xTaskCreate(taks_media_hum," taks_media_hum",4096,NULL,2,NULL);
xTaskCreate(task_media_temp,"task_media_temp ",4096,NULL,2,NULL);
xTaskCreate(task_med,"task_med",4096,NULL,2,NULL);
xTaskCreate(display,"Task do display",4096,NULL,2,NULL);

}

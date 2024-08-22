#include <stdio.h>
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
#include<esp_log.h>
#include<freertos/semphr.h>
#include<driver/gpio.h>

static const char *TAG = "MyApp";
#define BT_GPIO 0
//---------------------------DEFINIÇÕES-----------------------------------------
int vect[10];
int inf  = 0;
int cont_read = 0;
int cont_write = 0;
int soma = 0;
int reading = 0;
void taskwrite(void);
void taskread(void);
SemaphoreHandle_t mutex;

//---------------------------READ-----------------------------------------
int my_read_function()
{
inf = vect[cont_read];
cont_read++;
ESP_LOGI(TAG,"Vetor[%d]: %d",cont_read-1,vect[cont_read-1]);
return inf;
}


void taskwrite(void)
{

int writing = 0;
while(1)
{ 
    


  xSemaphoreTake(mutex, portMAX_DELAY);
  if(cont_read <10 && cont_write == 10)
  {

        xSemaphoreGive(mutex);
       
        ESP_LOGI(TAG,"TASK WRITE BLOQUEADA");
  }
  else
  { 
     if(gpio_get_level(BT_GPIO) == 0)
        {
            writing = writing + 1;
            vect[cont_write] = writing;
            cont_write = cont_write + 1;
           
           ESP_LOGI(TAG,"Vetor[%d]: %d",cont_write-1,vect[cont_write-1]);
        }
    if(cont_read == 10 && cont_write == 10)
    {
       
        cont_write = 0;
        cont_read = 0;
    }
   
    vTaskDelay(10);
 
    xSemaphoreGive(mutex);
}}

}


void taskread(void)
{
while(1)
{ 
    
xSemaphoreTake(mutex, portMAX_DELAY);
   if(cont_read == cont_write)
   {
       // ESP_LOGI(TAG,"TASK READ BLOQUEADA");
        xSemaphoreGive(mutex);
        
   }
   else
   { 
        reading = my_read_function();
        vTaskDelay(10);
        xSemaphoreGive(mutex);
        ESP_LOGI(TAG,"TASK READ RODANDO");
   }
}
}

void app_main(void)
{

 gpio_reset_pin(BT_GPIO);
 gpio_set_direction(BT_GPIO,GPIO_MODE_INPUT);
mutex = xSemaphoreCreateMutex();
xTaskCreate(taskwrite,"Task write",4096,NULL,2,NULL);
xTaskCreate(taskread,"Task read",4096,NULL,2,NULL);
}

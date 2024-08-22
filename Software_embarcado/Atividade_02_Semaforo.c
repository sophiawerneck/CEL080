#include <stdio.h>
#include<freertos/FreeRTOS.h>
#include<freertos/task.h>
#include<esp_log.h>
#include<freertos/semphr.h>
#include<driver/gpio.h>

static const char *TAG = "MyApp";

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

//--------------------------- WRITE -----------------------------------------
int my_write_function(int write1)
{
    vect[cont_write] = write1;
    cont_write++;
    ESP_LOGI(TAG,"Vetor[%d]: %d",cont_write-1,vect[cont_write-1]);
    return 0;
}

// ----------------------------- TASKS --------------------------

void taskwrite(void)
{
int writing = 1;
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
    if(cont_read == 10 && cont_write == 10)
    {
        for(int i=0; i<10; i++)
        {
            printf("Vetor [%d]: %d\n",i,vect[i]);
        }
        cont_write = 0;
        cont_read = 0;
    }
    my_write_function(writing);
    vTaskDelay(pdMS_TO_TICKS(1000));
    writing++;
    ESP_LOGI(TAG,"TASK WRITE RODANDO");
    xSemaphoreGive(mutex);
}}}

void taskread(void)
{
while(1)
{ 
xSemaphoreTake(mutex, portMAX_DELAY);
   if(cont_read == cont_write)
   {

        xSemaphoreGive(mutex);
        ESP_LOGI(TAG,"TASK READ BLOQUEADA");
   }
   else
   { 
        reading = my_read_function();
        vTaskDelay(pdMS_TO_TICKS(1000));
        xSemaphoreGive(mutex);
        ESP_LOGI(TAG,"TASK READ RODANDO");
   }
}
}

void app_main(void)
{
mutex = xSemaphoreCreateMutex();
xTaskCreate(taskwrite,"Task write",4096,NULL,2,NULL);
xTaskCreate(taskread,"Task read",4096,NULL,2,NULL);
}
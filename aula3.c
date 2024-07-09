#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

void vTask(void* pvparameters);


void app_main(void)
{
    xTaskCreatePinnedToCore(vTask, "Task 1", 2048, 1000, 1, NULL, 0);

    xTaskCreatePinnedToCore(vTask, "Task 2", 2048, 5000, 1, NULL, 1);
}


void vTask(void* pvparameters)   //ponteiro pra void aponta pra qqer tipo de variavel
{   

    BaseType_t core;

    uint16_t ms_delay = (uint16_t)pvparameters;   //recupera o valor ponteiro pra void pra uint16

    char* name = pcTaskGetName(NULL);
    //char* tag = pcTaskGetName(NULL);

    ESP_LOGI("TASK","%s inicializando",name);
    while (1)
    {
        core = xPortGetCoreID();
        
        ESP_LOGI("TASK","%s executando no Core %d",name,core);

        vTaskDelay(pdMS_TO_TICKS(ms_delay));
    }
    
}

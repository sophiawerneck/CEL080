#include <stdio.h>
#include <esp_log.h> //Recursos de log

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void vTask1(void *pvparameters);
void vTask2(void *pvparameters);
void vTaskN(void *pvparameters);

TaskHandle_t t1Handle, t2Handle; //handle: manipulador da tarefa

void app_main(void)
{
    xTaskCreatePinnedToCore(vTask1,"Task 1", 4096, NULL,2, &t1Handle, 0); //4º argumento: permite que passe para task na hora da sua criação parâmetros (é constante)
    xTaskCreatePinnedToCore(vTask2,"Task 2", 4096,NULL,2, &t1Handle, 1);
    xTaskCreatePinnedToCore(vTaskN,"Task 3", 4096, "3",2, NULL, 0);
    xTaskCreatePinnedToCore(vTaskN,"Task 4", 4096, "4",2, NULL, 1);
}

void vTask1(void *pvparameters)
{
    BaseType_t coreID;
    uint16_t task_prio;

    uint16_t cnt = 0;
    ESP_LOGI("TASK1","Task 1 Inicializando..."); //nível de informacao I //ou LOGE ou LOGW
    while(1)
    {
        coreID = xPortGetCoreID();

        task_prio = uxTaskPriorityGet(t2Handle);

        ESP_LOGI("TASK1","Task 1 Executando no Core %d ", coreID);

        ESP_LOGE("Task 1","Task 2 Prio = %d", task_prio);

        vTaskDelay(pdMS_TO_TICKS(1000));
        
        if(cnt < 10)
        {
            cnt++;
        }

        if(cnt == 10)
        {
            ESP_LOGE("Task 1","Aumentando a prioridade da Task 2");
            vTaskPrioritySet(t2Handle, 5);   //função que muda a prioridade da task
            cnt = 11;
        }
    }
}

void vTask2(void *pvparameters)
{
    BaseType_t coreID;
    ESP_LOGI("TASK2","Task 2 Inicializando..."); //nível de informacao I
    while(1)
    {
        coreID = xPortGetCoreID();
        ESP_LOGI("TASK2","Task 2 Executando no Core %d ", coreID);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vTaskN(void *pvparameters) //mesmo
{
    BaseType_t coreID;
    char *number;
    number = (char*)pvparameters;

    ESP_LOGW("TASKN","Task %s Inicializando...", number); //nível de informacao I
    while(1)
    {
        coreID = xPortGetCoreID();
        ESP_LOGW("TASKN","Task %s Executando no Core %d ", number,coreID);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


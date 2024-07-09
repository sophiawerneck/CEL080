#include <stdio.h>
#include <esp_log.h> 

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void vTask1(void *pvparameters);
void vTask2(void *pvparameters);
void vTask3(void *pvparameters);
void vTask4(void *pvparameters);

TaskHandle_t t1Handle, t2Handle, t3Handle, t4Handle;

void app_main(void)
{
    xTaskCreatePinnedToCore(vTask1,"Task 1", 4096, "1",1, &t1Handle, 0);
    xTaskCreatePinnedToCore(vTask2,"Task 2", 4096, "2",1, &t2Handle, 0);
    xTaskCreatePinnedToCore(vTask3,"Task 3", 4096, "3",1, &t3Handle, 1);
    xTaskCreatePinnedToCore(vTask4,"Task 4", 4096, "4",1, &t4Handle, 1);

}

void vTask1(void *pvparameters) 
{
    BaseType_t coreID;
    UBaseType_t stack;
    uint16_t i,j,lim = 10,cnt=0;
    char *number;
    number = (char*)pvparameters;

    while(1)
    {
        coreID = xPortGetCoreID();
        ESP_LOGI("TASK1","Task %s Executando no Core %d ", number,coreID);
        stack = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI("TASK1", "Task %s tem %d de stack livre ", number,stack);

        if(cnt < 20)
        {
            cnt++;
        }
        if(cnt == 20)
        {
             vTaskDelete(t2Handle);
             ESP_LOGE("TASK1","Deletando Task2");
            cnt = 21;
        }

        for(i=0; i<lim; i++)
        {
            for(j=0; j<50000; j++)
            {

            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void vTask3(void *pvparameters) 
{
    BaseType_t coreID;
    UBaseType_t stack;
    uint16_t i,j,lim = 10,cnt=0;
    char *number;
    number = (char*)pvparameters;

    while(1)
    {
        coreID = xPortGetCoreID();
        ESP_LOGI("TASK3","Task %s Executando no Core %d ", number,coreID);
        stack = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI("TASK3", "Task %s tem %d de stack livre ", number,stack);

        if(cnt < 20)
        {
            cnt++;
        }
        if(cnt == 20)
        {
            vTaskDelete(t4Handle);
            ESP_LOGE("TASK3","Deletando Task4");
            cnt = 21;
        }

        for(i=0; i<lim; i++)
        {
            for(j=0; j<50000; j++)
            {

            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void vTask2(void *pvparameters) 
{
    BaseType_t coreID;
    UBaseType_t stack;
    uint16_t i,j,lim = 10;
    char *number;
    number = (char*)pvparameters;

    while(1)
    {
        coreID = xPortGetCoreID();
        ESP_LOGI("TASK2","Task %s Executando no Core %d ", number,coreID);
        stack = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI("TASK2", "Task %s tem %d de stack livre ", number,stack);

        for(i=0; i<lim; i++)
        {
            for(j=0; j<50000; j++)
            {

            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void vTask4(void *pvparameters) 
{
    BaseType_t coreID;
    UBaseType_t stack;
    uint16_t i,j,lim = 10;
    char *number;
    number = (char*)pvparameters;

    while(1)
    {
        coreID = xPortGetCoreID();
        ESP_LOGI("TASK4","Task %s Executando no Core %d ", number,coreID);
        stack = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI("TASK4", "Task %s tem %d de stack livre ", number,stack);

        for(i=0; i<lim; i++)
        {
            for(j=0; j<50000; j++)
            {

            }
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


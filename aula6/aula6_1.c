#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>
#include <freertos/queue.h>
#include <esp_log.h>

QueueHandle_t queue1, queue2;

void vTask1(void *pvparameters);
void vTask2(void *pvparameters);
void vTask3(void *pvparameters);

void app_main(void)
{
    queue1 = xQueueCreate(10, sizeof(uint16_t));
    queue2 = xQueueCreate(10, sizeof(uint16_t));

    xTaskCreate(vTask1, "Task1", 2048, NULL, 1, NULL);
    xTaskCreate(vTask2, "Task2", 2048, NULL, 1, NULL);
    xTaskCreate(vTask3, "Task3", 2048, NULL, 1, NULL);
}

void vTask1(void *pvparameters) // Enviando números pares para a fila 
{
    uint16_t cnt = 0;
    portBASE_TYPE status;
    while(1)
    {
        status = xQueueSendToBack(queue1, &cnt, 0);
        if(status == pdTRUE)
        {
            ESP_LOGI("TASK1", "Mensagem CNT = %d enviada corretamente.", cnt);
        }
        else
        {
            ESP_LOGE("TASK1", "Mensagem CNT = %d perdida.", cnt);
        }
        cnt += 2;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vTask2(void *pvparameters) // Enviando números ímpares para a fila 
{
    uint16_t cnt = 1; // Para mandar só dados ímpares
    portBASE_TYPE status;
    while(1)
    {
        status = xQueueSendToBack(queue2, &cnt, 0);
        if(status == pdTRUE)
        {
            ESP_LOGI("TASK2", "Mensagem CNT = %d enviada corretamente.", cnt);
        }
        else
        {
            ESP_LOGE("TASK2", "Mensagem CNT = %d perdida.", cnt);
        }
        cnt += 2;
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void vTask3(void *pvparameters)  
{
    uint16_t msg; 
    
    while(1)
    {
       xQueueReceive(queue1, &msg, portMAX_DELAY);
       ESP_LOGW("TASK3", "Mensagem MSG = %d recebida da Queue 1", msg);

       xQueueReceive(queue2, &msg, portMAX_DELAY); // Fica bloqueado por muito tempo aqui e não vê as mensagens da fila 1, e as mensagens são perdidas
       ESP_LOGW("TASK3", "Mensagem MSG = %d recebida da Queue 2", msg);

    }
}

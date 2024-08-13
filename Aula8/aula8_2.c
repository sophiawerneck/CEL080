#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/message_buffer.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define BT_IO 0
MessageBufferHandle_t buffer;

void vTask1 (void *pvparameters);
void vTask2 (void *pvparameters);

void app_main(void)
{
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO, GPIO_MODE_INPUT);

    buffer = xMessageBufferCreate(100); //100 = numero de bytes

    xTaskCreate(vTask1, "Task 1", 2048, NULL, 2, NULL);
    xTaskCreate(vTask2, "Task 2", 2048, NULL, 1, NULL);
}

void vTask1 (void *pvparameters)
{
    int cnt = 0;
    size_t bytes_enviados;
    char msg[50];
    while(1)
    {
        if(gpio_get_level(BT_IO)==0)
        {
            cnt++;
            sprintf(msg,"Valor do CNT = %d.",cnt);
            bytes_enviados = xMessageBufferSend(buffer, &msg, strlen(msg), portMAX_DELAY); //strlen(msg): calcula o numero max da string
            ESP_LOGI("TASK1", "%d bytes enviados. MSG = %s ", bytes_enviados, msg);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTask2 (void *pvparameters)
{
    char data[100]; //posso enviar mensagens ate 100 caracteres pq meu buffer tem ate 100 bytes
    size_t bytes_recebidos; //numeros de bytes lidos 
    while(1)
    {
        bytes_recebidos = xMessageBufferReceive(buffer, data, sizeof(data), portMAX_DELAY); //sizeof(data): capacidade máxima que eu posso receber
        ESP_LOGI("TASK2", "%d bytes recebidos. DATA = %s", bytes_recebidos, data);
    }
}

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/stream_buffer.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define BT_IO 0
StreamBufferHandle_t buffer;

void vTask1 (void *pvparameters);
void vTask2 (void *pvparameters);

void app_main(void)
{
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO, GPIO_MODE_INPUT);

    buffer = xStreamBufferCreate(25, 5);// 5 = numero de bytes no trigger level
    xTaskCreate(vTask1, "Task 1", 2048, NULL, 2, NULL);
    xTaskCreate(vTask2, "Task 2", 2048, NULL, 1, NULL);
}

void vTask1 (void *pvparameters)
{
    uint8_t cnt = 0;
    size_t bytes_enviados;
    while(1)
    {
        if(gpio_get_level(BT_IO)==0)
        {
            cnt++;
            bytes_enviados = xStreamBufferSend(buffer, &cnt, sizeof(cnt), portMAX_DELAY);
            ESP_LOGI("TASK1", "%d bytes enviados. CNT = %d ", bytes_enviados, cnt);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTask2 (void *pvparameters)
{
    uint8_t data;
    size_t bytes_recebidos; //numeros de bytes lidos 
    while(1)
    {
        bytes_recebidos = xStreamBufferReceive(buffer, &data, sizeof(data), portMAX_DELAY);
        ESP_LOGI("TASK2", "%d bytes recebidos. DATA = %d", bytes_recebidos, data);
    }
}

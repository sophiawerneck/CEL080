#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/queue.h>

#define BT_IO 0

void vTaskBT(void* parameters);
void vTaskCodigo (void* parameters);

void timer1_cb (TimerHandle_t xtimer);

TimerHandle_t timer1;
QueueHandle_t myqueue1;

void app_main(void)
{
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO, GPIO_MODE_INPUT);


    timer1 = xTimerCreate("Timer 1", pdMS_TO_TICKS(300), pdTRUE, (void *)0, timer1_cb);
    myqueue1 = xQueueCreate(10, sizeof(char));

    xTaskCreate(vTaskBT, "Task BT", 4096, NULL, 1, NULL);
    xTaskCreate(vTaskCodigo, "Task Codigo", 4096, NULL, 1, NULL);
}

void vTaskBT(void* parameters)
{
    while(1)
    {
        if(gpio_get_level(BT_IO)==0)
        {
            ESP_LOGI("BT", "BOTAO PRESSIONADO");
            xTimerStart(timer1, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
void timer1_cb (TimerHandle_t xtimer)
{
    while(1)
    {
        if (xtimer > pdMS_TO_TICKS(300))
        {
            ESP_LOGI("TIMER1", "_");
            xQueueSendToBack(myqueue1, "_", portMAX_DELAY);
        }
        else
        {
            ESP_LOGI("TIMER1", ".");
            xQueueSendToBack(myqueue1, ".", portMAX_DELAY);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTaskCodigo (void* parameters)
{
    while(1)
    {

    }
}





    
/*
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO, GPIO_MODE_INPUT);
    gpio_set_intr_type(BT_IO, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BT_IO, gpio_isr_handler, NULL);
*/



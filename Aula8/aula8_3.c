#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define BT_IO 0
//facilitar o acesso do bit a bit do registrador
#define EV_BT (1<<0)
#define EV_VAL (1<<1)


EventGroupHandle_t ev_group;

void vTask1 (void *pvparameters);
void vTask2 (void *pvparameters);
void vTask3 (void *pvparameters);

void app_main(void)
{
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO, GPIO_MODE_INPUT);

    ev_group = xEventGroupCreate();

    xTaskCreate(vTask1, "Task 1", 2048, NULL, 2, NULL);
    xTaskCreate(vTask2, "Task 2", 2048, NULL, 1, NULL);
    xTaskCreate(vTask3, "Task 3", 2048, NULL, 1, NULL);
}

void vTask1 (void *pvparameters)
{
    while(1)
    {
        if(gpio_get_level(BT_IO)==0)
        {
           ESP_LOGI("TASK1", "Botão pressionado.");
           xEventGroupSetBits(ev_group, EV_VAL);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTask2 (void *pvparameters)
{
    uint16_t valor = 0;
    while(1)
    {
       valor += rand() % 10;
       ESP_LOGI("TASK2", "Valor = %d", valor);
       if (valor > 100)
       {    
            xEventGroupSetBits(ev_group, EV_VAL);
            valor = 0;
       }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vTask3 (void *pvparameters)
{
    EventBits_t bits;
    while(1)
    {
       bits = xEventGroupWaitBits(ev_group, (EV_BT|EV_VAL), pdTRUE, pdTRUE, portMAX_DELAY); //param2: quais bits eu tenho que esperar / param4: se true->
       //os dois eventos devem ocorrer , se false-> ou um ou outro evento ocorre para liberar
       if(bits==EV_BT)
       {
        ESP_LOGI("TASK3","EV_BT!"); //bits == 01
       }
       if(bits==EV_VAL)
       {
        ESP_LOGI("TASK3","EV_VAL!"); //bits == 10
       }
       if(bits==(EV_VAL|EV_BT)) // bits == 11
       {
        ESP_LOGI("TASK3","Ocorreram os dois eventos!");
       }
    }
}

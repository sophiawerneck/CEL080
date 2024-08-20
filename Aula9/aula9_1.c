#include <stdio.h>
#include <freertos/freeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <esp_log.h>
#include <driver/gpio.h>

TimerHandle_t timer1, timer2;

void timer_cb(TimerHandle_t xtimer)
{
    uint16_t cnt;
    if (xtimer == timer1)
    {
        cnt = (uint16_t)pvTimerGetTimerID(timer1);
        cnt++;
        ESP_LOGI("TIMER1","Evento do Timer 1. CNT1 = %d ",cnt);
        vTimerSetTimerID(timer1, (void*)cnt);
    }
    else
    {
        cnt = (uint16_t)pvTimerGetTimerID(timer2);
        cnt++;
        ESP_LOGI("TIMER2","Evento do Timer 2. CNT2 = %d ",cnt);
        vTimerSetTimerID(timer2, (void*)cnt);
    }    
}

void app_main(void)
{
    timer1 = xTimerCreate("Timer1", pdMS_TO_TICKS(1000), pdTRUE, (void*) 0, timer_cb); //disparo único:pdFALSE; periodico:pdTRUE
    timer2 = xTimerCreate("Timer2", pdMS_TO_TICKS(3000), pdTRUE, (void*) 0, timer_cb);

    xTimerStart(timer1, 0);
    xTimerStart(timer2, 0);
}

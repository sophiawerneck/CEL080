#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <ultrasonic.h>

#define MAXDISTANCE_CM 500

#define TRIGGER_GPIO 18
#define BT_IO 0
#define ECHO_GPIO 5

ultrasonic_sensor_t sensor;
QueueHandle_t queue;

void task_leitura (void *pvparameters);
void task_alarme (void *pvparameters);
void task_troca(void *pvparameters);

float mindistance_cm = 10;

SemaphoreHandle_t sem_sinc;
SemaphoreHandle_t sem_mutex;

static void IRAM_ATTR gpio_isr_handler(void* args) 
{
    BaseType_t HPTW = pdFALSE;

    xSemaphoreGiveFromISR(sem_sinc,&HPTW);
    
    if(HPTW == pdTRUE)
    {
        portYIELD_FROM_ISR(); 
    }

}

void app_main(void)
{
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO, GPIO_MODE_INPUT);
    gpio_set_intr_type(BT_IO, GPIO_INTR_NEGEDGE);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BT_IO, gpio_isr_handler, NULL);

    sensor.trigger_pin = TRIGGER_GPIO;
    sensor.echo_pin = ECHO_GPIO;
    ultrasonic_init(&sensor);

    queue = xQueueCreate(10 ,(sizeof(float)));
    sem_sinc = xSemaphoreCreateBinary();
    sem_mutex = xSemaphoreCreateMutex();

    xTaskCreate(task_leitura, "task_leitura", 4096, NULL, 1, NULL);
    xTaskCreate(task_alarme, "task_alarme", 4096, NULL, 1, NULL);
    xTaskCreate(task_troca, "task_troca", 4096, NULL, 1, NULL);
}

void task_leitura (void *pvparameters)
{
    float distancia = 0;
    while(1)
    {
        xSemaphoreTake(sem_mutex, portMAX_DELAY);
        ultrasonic_measure(&sensor, MAXDISTANCE_CM, &distancia);
        distancia *= 100;
        xSemaphoreGive(sem_mutex);
        xSemaphoreTake(sem_mutex, portMAX_DELAY);
        ESP_LOGI("LEITURA", "%f", distancia);
        xSemaphoreGive(sem_mutex);
        xQueueSendToBack(queue, &distancia, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void task_alarme (void *pvparameters)
{
    float distancia = 0;
    while(1)
    {
        xQueueReceive(queue,&distancia,portMAX_DELAY);
        if(distancia < mindistance_cm)
        {
            xSemaphoreTake(sem_mutex, portMAX_DELAY);
            ESP_LOGW("ALARME","VALOR MÍNIMO ATINGIDO!");
            xSemaphoreGive(sem_mutex);
        }
    }
}

void task_troca(void *pvparameters)
{
    while(1)
    {
        xSemaphoreTake(sem_sinc, portMAX_DELAY);
        if (mindistance_cm < 100)
        {
            mindistance_cm = mindistance_cm + 10;
        }
        xSemaphoreTake(sem_mutex, portMAX_DELAY);
        ESP_LOGE("IMPRIME","Valor ajustado: %f", mindistance_cm);
        xSemaphoreGive(sem_mutex);
    }
}


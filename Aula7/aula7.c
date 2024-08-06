#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <driver/gpio.h>
#include <esp_log.h>

#define BT_IO 0

SemaphoreHandle_t sem_sinc;

static void IRAM_ATTR gpio_isr_handler(void* args) //definir o handler: função que eu quero executar quando o evento acontecer. Dados são armazenados na ram
{
    BaseType_t HPTW = pdFALSE;

    xSemaphoreGiveFromISR(sem_sinc,&HPTW);

    if(HPTW == pdTRUE)
    {
        portYIELD_FROM_ISR(); // Para forçar a troca de contexto
    }

}

void vTask1(void* pvparameters);

void app_main(void)
{
    // Configuração Periféricos
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO,GPIO_MODE_INPUT);

    gpio_set_intr_type(BT_IO,GPIO_INTR_POSEDGE); //GPIO_INTR_LOW_LEVEL: enquanto tiver com o botão pressionado vai gerar varios eventos 
    // GPIO_INTR_POSEDGE: quando soltar o botão | GPIO_INTR_NEGEDGE: quando pressionar o botão
    gpio_intr_enable(BT_IO);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BT_IO, gpio_isr_handler, NULL); // Para instalar o handler para tratar a interrupção

    // Criação Objetos
    sem_sinc = xSemaphoreCreateBinary();

    //Criação Tasks

    xTaskCreate(vTask1,"Task 1", 2048, NULL, 1, NULL);
}

void vTask1(void* pvparameters)
{
    ESP_LOGI("TASK1","Task 1 inicializando!");

    while (1)
    {
        ESP_LOGI("TASK1","Aguardando evento");
        xSemaphoreTake(sem_sinc,portMAX_DELAY);
        ESP_LOGI("TASK1","Botão pressionado");        
    }   
}

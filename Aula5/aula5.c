#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <driver/gpio.h>
#include <esp_log.h>

#define BTIO 0

struct mystruct{
    uint16_t cnt;
    TickType_t ticks;
};

QueueHandle_t myqueue;

void vTaskBT(void* parameters);
void vTaskDISP(void* parameters);

void app_main(void)
{
    // Configuração do IO
    gpio_reset_pin(BTIO);
    gpio_set_direction(BTIO, GPIO_MODE_INPUT);

    // Criação da Fila
    myqueue = xQueueCreate(5, sizeof(struct mystruct)); // 5 mensagens e tamanho da variável

    // Criação das Tasks
    xTaskCreate(vTaskBT, "BOTAO", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskDISP, "DISP", 2048, NULL, 1, NULL);
}

void vTaskBT(void* parameters)
{
    uint16_t cnt_BT = 0; // Variável local
    struct mystruct st_BT;
    st_BT.cnt = 0;

    ESP_LOGI("BT","Task BT inicializando!");
    while(1)
    {
        if(gpio_get_level(BTIO) == 0)
        {
            st_BT.cnt++;
            st_BT.ticks = xTaskGetTickCount(); // Função que retorna o tempo em que clicou no botão

            ESP_LOGI("BT", "Botão pressionado. CNT = %d", st_BT.cnt); // Copiado da task remetente para fila
            xQueueSendToBack(myqueue, &st_BT, portMAX_DELAY); // Fila, endereço e opção de bloqueio
            ESP_LOGI("BT", "Mensagem enviada!"); 
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTaskDISP(void* parameters)
{
    //uint16_t cnt_DISP; // Variável local
    struct mystruct st_DISP;
    ESP_LOGI("DISP", "Task DISP inicializando!");
    while(1)
    {
        if(uxQueueMessagesWaiting(myqueue) == 0) // Para poder encher a fila
        {
            vTaskDelay(pdMS_TO_TICKS(10000));
        }

        xQueueReceive(myqueue, &st_DISP, portMAX_DELAY); // Copia da fila para o endereço de memória que armazena o cnt_DISP, espera uma mensagem da fila
        ESP_LOGE("DISP","Dado recebido, CNT = %d. Botão pressionado em %d ms", st_DISP.cnt, (int)pdTICKS_TO_MS(st_DISP.ticks));
    }
}

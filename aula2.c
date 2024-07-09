#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void vTask1(void *pvparameters);
void vTask2(void *pvparameters);

void app_main(void)
{
    xTaskCreate(vTask1, "Task 1", 4096, NULL, 1, NULL);
    xTaskCreate(vTask2, "Task 2", 4096, NULL, 2, NULL);
}

void vTask1(void *pvparameters)
{
    uint16_t ii, jj;
    //Inicialização (execulta só uma vez)
    printf("Task 1 inicializando ... \n");

    while(1)
    {
        //Loop
        printf("Task 1 executando no core %d \n", xPortGetCoreID());

        for(ii = 0; ii<100;ii++)
        {
            for(jj = 0; jj<10000; jj++)
            {
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void vTask2(void *pvparameters)
{
    uint16_t ii, jj;
    //Inicialização (execulta só uma vez)
    printf("Task 2 inicializando ... \n");

    while(1)
    {
        //Loop
        printf("Task 2 executando no core %d \n", xPortGetCoreID());

        for(ii = 0; ii<100;ii++)
        {
            for(jj = 0; jj<10000; jj++)
            {
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

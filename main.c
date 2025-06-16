#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/i2c.h"

#include "queue.h"

// Definição dos pinos
#define LED_RED_PIN 13
#define LED_GREEN_PIN 11
#define LED_BLUE_PIN 12
#define BUZZER_PIN 21
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Handles das tarefas
TaskHandle_t xLedTaskHandle = NULL;
TaskHandle_t xBuzzerTaskHandle = NULL;

// Tarefa do LED RGB
void vLedTask(void *pvParameters) {
    volatile uint8_t current_color = 0; // 0=red, 1=green, 2=blue
    
    gpio_init(LED_RED_PIN);
    gpio_init(LED_GREEN_PIN);
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
    
    while(1) {
        // Desliga todos os LEDs
        gpio_put(LED_RED_PIN, 0);
        gpio_put(LED_GREEN_PIN, 0);
        gpio_put(LED_BLUE_PIN, 0);
        
        // Acende o LED atual
        switch(current_color) {
            case 0:
                gpio_put(LED_RED_PIN, 1);
                break;
            case 1:
                gpio_put(LED_GREEN_PIN, 1);
                break;
            case 2:
                gpio_put(LED_BLUE_PIN, 1);
                break;
        }
        
        // Avança para a próxima cor
        current_color = (current_color + 1) % 3;
        
        // Espera 500ms
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Tarefa do Buzzer
void vBuzzerTask(void *pvParameters) {
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    
    while(1) {
        // Emite um beep
        gpio_put(BUZZER_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_put(BUZZER_PIN, 0);
        
        // Espera 1 segundo
        vTaskDelay(pdMS_TO_TICKS(900));
    }
}

// Tarefa dos botões
void vButtonTask(void *pvParameters) {
    volatile bool led_is_paused = false;
    volatile bool buzzer_is_paused = false;
    
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);
    
    while(1) {
        bool button_a = !gpio_get(BUTTON_A_PIN); // Lógica invertida por causa do pull-up
        bool button_b = !gpio_get(BUTTON_B_PIN);
        
        // Verifica borda de descida do botão A
        if (button_a ) {
            if (!led_is_paused) {
              // desliga todas as leds
              gpio_put(LED_RED_PIN, 0);
              gpio_put(LED_GREEN_PIN, 0);
              gpio_put(LED_BLUE_PIN, 0);
              vTaskSuspend(xLedTaskHandle);
              led_is_paused = true;
              vTaskDelay(pdMS_TO_TICKS(150)); 
            }

        } else if(led_is_paused) {
               
              vTaskResume(xLedTaskHandle);
              led_is_paused = false;
              vTaskDelay(pdMS_TO_TICKS(150)); 

        }
        
        
        // Verifica borda de descida do botão B
        if (button_b ) {
          if (!buzzer_is_paused) {
            gpio_put(BUZZER_PIN, 0);
            vTaskSuspend(xBuzzerTaskHandle);
            buzzer_is_paused = true;
            vTaskDelay(pdMS_TO_TICKS(150)); 

          }
        } else if (buzzer_is_paused) {
          vTaskResume(xBuzzerTaskHandle);
          buzzer_is_paused = false;
          vTaskDelay(pdMS_TO_TICKS(150)); 

        }
        
        
        
        // Espera 100ms para debounce
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main() {
    stdio_init_all();
    
    // Cria as tarefas
    xTaskCreate(vLedTask, "LED Task", 256, NULL, 1, &xLedTaskHandle);
    xTaskCreate(vBuzzerTask, "Buzzer Task", 256, NULL, 1, &xBuzzerTaskHandle);
    xTaskCreate(vButtonTask, "Button Task", 256, NULL, 2, NULL); // Prioridade maior para garantir resposta rápida
    
    // Inicia o escalonador
    vTaskStartScheduler();
    
    // Nunca deve chegar aqui
    while(1);
    return 0;
}
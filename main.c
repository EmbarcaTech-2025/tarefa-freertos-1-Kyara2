#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/i2c.h"

void led_task(void *params) {
  const uint LED_PIN = 13;

  // set gpios
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  while (true) {
    gpio_put(LED_PIN, 1);
    vTaskDelay(100);
    // vTaskDelay(pdMS_TO_TICS(500));

    gpio_put(LED_PIN, 0);
    vTaskDelay(100);
    // vTaskDelay(pdMS_TO_TICKS(500));
  }
}

int main() {
  stdio_init_all();

  xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
  vTaskStartScheduler();

  while(1){
    printf("I really love RTOS!\n");
    sleep_ms(500);
  };
}
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

#define RED_LED_PIN     13
#define GREEN_LED_PIN   11
#define BUTTON_PIN       5

SemaphoreHandle_t yellow_override_semaphore;

// Função para configurar os GPIOs
void setup_leds() {
    gpio_init(RED_LED_PIN);
    gpio_set_dir(RED_LED_PIN, GPIO_OUT);
    gpio_put(RED_LED_PIN, 0);

    gpio_init(GREEN_LED_PIN);
    gpio_set_dir(GREEN_LED_PIN, GPIO_OUT);
    gpio_put(GREEN_LED_PIN, 0);
}

// Função para desligar todos os LEDs
void all_off() {
    gpio_put(RED_LED_PIN, 0);
    gpio_put(GREEN_LED_PIN, 0);
}

// Função que executa uma etapa com verificação do botão
bool led_stage_with_interrupt(uint red, uint green, int delay_ms) {
    gpio_put(RED_LED_PIN, red);
    gpio_put(GREEN_LED_PIN, green);

    const int check_interval = 50;
    int waited = 0;

    while (waited < delay_ms) {
        if (xSemaphoreTake(yellow_override_semaphore, 0) == pdTRUE) {
            // Botão pressionado: faz override
            printf("⚠️ Botão pressionado! Interrompendo ciclo\n");
            gpio_put(RED_LED_PIN, 1);
            gpio_put(GREEN_LED_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(3000));
            all_off();
            return true; // sinaliza que houve override
        }
        vTaskDelay(pdMS_TO_TICKS(check_interval));
        waited += check_interval;
    }

    return false; // ciclo normal
}

// Tarefa principal do semáforo
void traffic_light_task(void *params) {
    while (1) {
        printf("Vermelho por 5s\n");
        if (led_stage_with_interrupt(1, 0, 5000)) continue;

        printf("Verde por 5s\n");
        if (led_stage_with_interrupt(0, 1, 5000)) continue;

        printf("Amarelo (vermelho+verde) por 3s\n");
        if (led_stage_with_interrupt(1, 1, 3000)) continue;
    }
}

// Callback da interrupção do botão
void gpio_callback(uint gpio, uint32_t events) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (gpio == BUTTON_PIN && (events & GPIO_IRQ_EDGE_FALL)) {
        xSemaphoreGiveFromISR(yellow_override_semaphore, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

int main() {
    stdio_init_all();
    setup_leds();

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    // Cria semáforo para sinalizar interrupção do botão
    yellow_override_semaphore = xSemaphoreCreateBinary();
    if (yellow_override_semaphore == NULL) {
        printf("Erro: semáforo não pôde ser criado\n");
        while (1);
    }

    // Registra callback de interrupção do botão
    gpio_set_irq_enabled_with_callback(BUTTON_PIN, GPIO_IRQ_EDGE_FALL, true, gpio_callback);

    // Cria a tarefa principal
    xTaskCreate(traffic_light_task, "Traffic Light", 512, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
}

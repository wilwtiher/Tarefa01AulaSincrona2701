#include <stdio.h>
#include "pico/stdlib.h"


// Configurações dos pinos 
const uint led_pin = 13;    //Red=13, Blue=12, Green=11
const uint botao_pinA = 5;        // Botão A = 5, Botão B = 6 , BotãoJoy = 22
const uint botao_pinB = 6;        // Botão A = 5, Botão B = 6 , BotãoJoy = 22
// Prototipo da função de interrupção
static void gpio_irq_handler(uint gpio, uint32_t events);

int main()
{
    gpio_init(led_pin);                 // Inicializa o pino do LED
    gpio_set_dir(led_pin, GPIO_OUT);    // Configura o pino como saída
    gpio_init(botao_pinA);                    // Inicializa o botão
    gpio_set_dir(botao_pinA, GPIO_IN);        // Configura o pino como entrada
    gpio_pull_up(botao_pinA);                 // Habilita o pull-up interno
    gpio_init(botao_pinB);                    // Inicializa o botão
    gpio_set_dir(botao_pinB, GPIO_IN);        // Configura o pino como entrada
    gpio_pull_up(botao_pinB);                 // Habilita o pull-up interno
    // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(botao_pinA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_pinB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {
        gpio_put(led_pin, true);
        sleep_ms(100);
        gpio_put(led_pin, false);
        sleep_ms(100);
    }
}
void gpio_irq_handler(uint gpio, uint32_t events)
{
    bool estado_atual = gpio_get(led_pin); // Obtém o estado atual
    gpio_put(led_pin, !estado_atual);      // Alterna o estado
}

#include <stdio.h>
#include "pico/stdlib.h"


// Configurações dos pinos 
const uint led_pin = 13;    //Red=13, Blue=12, Green=11
const uint botao_pinA = 5;        // Botão A = 5, Botão B = 6 , BotãoJoy = 22
const uint botao_pinB = 6;        // Botão A = 5, Botão B = 6 , BotãoJoy = 22

// Variáveis globais
static volatile uint a = 1;
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)

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
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    printf("A = %d\n", a);
    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > 200000) // 200 ms de debouncing
    {
        last_time = current_time; // Atualiza o tempo do último evento
        printf("Mudanca de Estado do Led. A = %d\n", a);
        gpio_put(led_pin, !gpio_get(led_pin)); // Alterna o estado
        a++;                                   // incrementa a variavel de verificação
    }
}

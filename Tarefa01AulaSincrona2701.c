#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define IS_RGBW false
#define NUM_PIXELS 25
#define WS2812_PIN 7

// Variável global para armazenar a cor (Entre 0 e 255 para intensidade)
uint8_t led_r = 20; // Intensidade do vermelho
uint8_t led_g = 20; // Intensidade do verde
uint8_t led_b = 20; // Intensidade do azul

// Configurações dos pinos 
const uint led_pin = 13;    //Red=13, Blue=12, Green=11
const uint botao_pinA = 5;        // Botão A = 5, Botão B = 6 , BotãoJoy = 22
const uint botao_pinB = 6;        // Botão A = 5, Botão B = 6 , BotãoJoy = 22

// Variáveis globais
static volatile uint a = 1;
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)
static volatile int contador = 0;


// Buffer para armazenar quais LEDs serao ligados na matriz de acordo com a variavel contador
bool led_buffer[10][NUM_PIXELS] = {
    { // 0 invertido verticalmente:
      // Nova ordem: linha4, linha3, linha2, linha1, linha0
        0, 1, 1, 1, 0,   // antiga linha4
        1, 0, 0, 0, 1,   // antiga linha3
        1, 0, 0, 0, 1,   // antiga linha2
        1, 0, 0, 0, 1,   // antiga linha1
        0, 1, 1, 1, 0    // antiga linha0
    },
    { // 1 invertido verticalmente:
        0, 1, 1, 1, 0,   // antiga linha4
        0, 0, 1, 0, 0,   // antiga linha3
        0, 0, 1, 0, 0,   // antiga linha2
        0, 1, 1, 0, 0,   // antiga linha1
        0, 0, 1, 0, 0    // antiga linha0
    },
    { // 2 invertido verticalmente:
        1, 1, 1, 1, 1,   // antiga linha4
        1, 0, 0, 0, 0,   // antiga linha3
        0, 1, 1, 1, 0,   // antiga linha2
        0, 0, 0, 0, 1,   // antiga linha1
        1, 1, 1, 1, 1    // antiga linha0
    },
    { // 3 invertido verticalmente:
        1, 1, 1, 1, 1,   // antiga linha4
        0, 0, 0, 0, 1,   // antiga linha3
        1, 1, 1, 0, 0,   // antiga linha2
        0, 0, 0, 0, 1,   // antiga linha1
        1, 1, 1, 1, 1    // antiga linha0
    },
    { // 4 invertido verticalmente:
        0, 0, 1, 0, 0,   // antiga linha4
        1, 1, 1, 1, 1,   // antiga linha3
        0, 0, 1, 0, 1,   // antiga linha2
        0, 1, 1, 0, 0,   // antiga linha1
        0, 0, 1, 0, 0    // antiga linha0
    },
    { // 5 invertido verticalmente:
        0, 0, 1, 1, 1,   // antiga linha4
        0, 0, 0, 1, 0,   // antiga linha3
        0, 0, 1, 1, 1,   // antiga linha2
        1, 0, 0, 0, 0,   // antiga linha1
        1, 1, 1, 1, 1    // antiga linha0
    },
    { // 6 invertido verticalmente:
        0, 0, 1, 1, 0,   // antiga linha4
        1, 0, 0, 1, 0,   // antiga linha3
        0, 0, 1, 1, 1,   // antiga linha2
        1, 0, 0, 0, 0,   // antiga linha1
        0, 1, 1, 1, 0    // antiga linha0
    },
    { // 7 invertido verticalmente:
        0, 0, 0, 0, 1,   // antiga linha4
        0, 1, 0, 0, 0,   // antiga linha3
        0, 0, 1, 0, 0,   // antiga linha2
        0, 0, 0, 1, 0,   // antiga linha1
        1, 1, 1, 1, 1    // antiga linha0
    },
    { // 8 invertido verticalmente:
        0, 1, 1, 1, 0,   // antiga linha4
        1, 0, 0, 0, 1,   // antiga linha3
        0, 1, 1, 1, 0,   // antiga linha2
        1, 0, 0, 0, 1,   // antiga linha1
        0, 1, 1, 1, 0    // antiga linha0
    },
    { // 9 invertido verticalmente:
        0, 1, 1, 1, 0,   // antiga linha4
        0, 0, 0, 0, 1,   // antiga linha3
        1, 1, 1, 1, 0,   // antiga linha2
        1, 0, 0, 0, 1,   // antiga linha1
        0, 1, 1, 1, 0    // antiga linha0
    }
};

static inline void put_pixel(uint32_t pixel_grb)
{
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}


void set_one_led(uint8_t r, uint8_t g, uint8_t b)
{
    // Define a cor com base nos parâmetros fornecidos
    uint32_t color = urgb_u32(r, g, b);

    // Define todos os LEDs com a cor especificada
    for (int i = 0; i < NUM_PIXELS; i++)
    {
        if (led_buffer[contador][i])
        {
            put_pixel(color); // Liga o LED com um no buffer
        }
        else
        {
            put_pixel(0);  // Desliga os LEDs com zero no buffer
        }
    }
}


// Prototipo da função de interrupção
static void gpio_irq_handler(uint gpio, uint32_t events);
//static void gpio_irq_handlerB(uint gpio, uint32_t events);

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

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Configuração da interrupção com callback
    gpio_set_irq_enabled_with_callback(botao_pinA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
//    gpio_set_irq_enabled_with_callback(botao_pinB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handlerB);

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
    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > 200000) // 200 ms de debouncing
    {
        printf("Contador = %d\n", contador);
        last_time = current_time; // Atualiza o tempo do último evento
        contador++;
        if(contador > 9){
            contador = 9;
        }else{
            set_one_led(led_r, led_g, led_b);
        }
        a++;                                   // incrementa a variavel de verificação
    }
}
/*
void gpio_irq_handlerB(uint gpio, uint32_t events)
{
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > 200000) // 200 ms de debouncing
    {
        last_time = current_time; // Atualiza o tempo do último evento
        contador--;
        if(contador < 0){
            contador = 0;
        }else{
            set_one_led(led_r, led_g, led_b);
        }
        a++;                                   // incrementa a variavel de verificação
    }
}
*/
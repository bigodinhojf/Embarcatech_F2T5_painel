// -- Inclusão de bibliotecas
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"

// -- Definição de constantes
// GPIO
#define button_A 5 // Bitão A GPIO 5
#define button_B 6 // Botão B GPIO 6
#define joystick_PB 22 // Botão do joystick GPIO 22
#define buzzer_A 21 // Buzzer A GPIO 21
#define buzzer_B 10 // Buzzer B GPIO 10
#define LED_Green 11 // LED Verde GPIO 11
#define LED_Blue 12 // LED Azul GPIO 12
#define LED_Red 13 // LED Vermelho GPIO 13

// Display I2C
#define display_i2c_port i2c1 // Define a porta I2C
#define display_i2c_sda 14 // Define o pino SDA na GPIO 14
#define display_i2c_scl 15 // Define o pino SCL na GPIO 15
#define display_i2c_endereco 0x3C // Define o endereço do I2C
ssd1306_t ssd; // Inicializa a estrutura do display

SemaphoreHandle_t xDisplayMutex; // Declaração para o mutex
SemaphoreHandle_t xUsuariosSem; // Declração para o semaforo de contagem do botoão A
SemaphoreHandle_t xResetSem; // Declração para o semaforo de contagem do botoão B

#define MAX_USERS 10 // Quantidade máxima de usuários

// -- Variáveis globais
static volatile uint32_t last_time = 0; // Armazena o tempo do último clique dos botões
volatile uint16_t usuarios = 0; // Armazena a quantidade de usuários atual

// Interrupção do botão do Joystick
void gpio_callback(uint gpio, uint32_t events){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xResetSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void led_color(){
    if(usuarios == 0){
        gpio_put(LED_Green, false);
        gpio_put(LED_Blue, true);
        gpio_put(LED_Red, false);
    }else if(usuarios == MAX_USERS){
        gpio_put(LED_Green, false);
        gpio_put(LED_Blue, false);
        gpio_put(LED_Red, true);
    }else if(usuarios == (MAX_USERS - 1)){
        gpio_put(LED_Green, true);
        gpio_put(LED_Blue, false);
        gpio_put(LED_Red, true);
    }else{
        gpio_put(LED_Green, true);
        gpio_put(LED_Blue, false);
        gpio_put(LED_Red, false);
    }
}

void displayOLED(){
    if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY)==pdTRUE) {
            


        xSemaphoreGive(xDisplayMutex);
    }
}

void vTaskEntrada(void *params){
    while(true){
        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY)==pdTRUE) {
            


            xSemaphoreGive(xDisplayMutex);
        }
    }
}

void vTaskSaida(void *params){
    while(true){
        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY)==pdTRUE) {
            


            xSemaphoreGive(xDisplayMutex);
        }
    }
}

void vTaskReset(void *params){
    while(true){
        if (xSemaphoreTake(xDisplayMutex, portMAX_DELAY)==pdTRUE) {
            


            xSemaphoreGive(xDisplayMutex);
        }
    }
}

// Função de interrupção dos botões
void gpio_irq_handler(uint gpio, uint32_t events){
    //Debouncing
    uint32_t current_time = to_us_since_boot(get_absolute_time()); // Pega o tempo atual e transforma em us
    if((current_time - last_time) > 200000){
        last_time = current_time; // Atualização de tempo do último clique
        if(gpio == joystick_PB){
            gpio_callback(gpio, events);
        }
    }
}

int main()
{
    stdio_init_all();

    // Inicialização dos GPIO
    gpio_init(LED_Green); // Inicia a GPIO 11 do LED Verde
    gpio_set_dir(LED_Green, GPIO_OUT); // Define a direção da GPIO 11 do LED Verde como saída
    gpio_put(LED_Green, false); // Estado inicial do LED apagado
    gpio_init(LED_Blue); // Inicia a GPIO 12 do LED Azul
    gpio_set_dir(LED_Blue, GPIO_OUT); // Define a direção da GPIO 12 do LED Azul como saída
    gpio_put(LED_Blue, false); // Estado inicial do LED apagado
    gpio_init(LED_Red); // Inicia a GPIO 13 do LED Vermelho
    gpio_set_dir(LED_Red, GPIO_OUT); // Define a direção da GPIO 13 do LED Vermelho como saída
    gpio_put(LED_Red, false); // Estado inicial do LED apagado

    // Inicialização do Display I2C
    i2c_init(display_i2c_port, 400 * 1000); // Inicializa o I2C usando 400kHz
    gpio_set_function(display_i2c_sda, GPIO_FUNC_I2C); // Define o pino SDA (GPIO 14) na função I2C
    gpio_set_function(display_i2c_scl, GPIO_FUNC_I2C); // Define o pino SCL (GPIO 15) na função I2C
    gpio_pull_up(display_i2c_sda); // Ativa o resistor de pull up para o pino SDA (GPIO 14)
    gpio_pull_up(display_i2c_scl); // Ativa o resistor de pull up para o pino SCL (GPIO 15)
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, display_i2c_endereco, display_i2c_port); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display
    ssd1306_fill(&ssd, false); // Limpa o display
    
    ssd1306_rect(&ssd, 0, 0, 127, 63, true, false); // Borda principal
    ssd1306_line(&ssd, 1, 12, 126, 12, true); // Desenha uma linha horizontal
    ssd1306_line(&ssd, 1, 24, 126, 24, true); // Desenha uma linha horizontal
    ssd1306_draw_string(&ssd, "EMB Controle", 20, 3); // Desenha uma string
    ssd1306_draw_string(&ssd, "Escritorio", 22, 15); // Desenha uma string

    ssd1306_send_data(&ssd); // Atualiza o display

    // Interrupção do botão
    gpio_set_irq_enabled_with_callback(joystick_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Cria o mutex do display
    xDisplayMutex = xSemaphoreCreateMutex();
    xResetSem = xSemaphoreCreateCounting(MAX_USERS, MAX_USERS);
    xUsuariosSem = xSemaphoreCreateBinary();

    // Cria as tarefas
    xTaskCreate(vTaskEntrada, "Botão A", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "Botão B", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "Botão do joystick", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);

    // Inicia o agendador
    vTaskStartScheduler();
    panic_unsupported();
}

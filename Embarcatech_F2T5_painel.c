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
char str_usuarios[2]; // String para o display OLED
volatile int buzzer_play = 0; // Auxilia no beep do buzzer 


// Função para definir a cor do LED RGB
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


// Função para exibir informações no Display OLED (protegida com Mutex)
void displayOLED(const char* msg){
    if(xSemaphoreTake(xDisplayMutex, portMAX_DELAY) == pdTRUE) {
        
        sprintf(str_usuarios, "%d", usuarios); // Converte int em string
        ssd1306_draw_string(&ssd, "  ", 90, 32); // Apaga parte variável
        
        if(usuarios > 9){
            ssd1306_draw_string(&ssd, str_usuarios, 90, 32); // Desenha uma string
        }else{
            ssd1306_draw_string(&ssd, str_usuarios, 94, 32); // Desenha uma string   
        }
        
        ssd1306_draw_string(&ssd, "               ", 3, 45); // Apaga parte variável
        ssd1306_draw_string(&ssd, msg, 3, 45); // Desenha uma string 
        ssd1306_send_data(&ssd); // Atualiza o display
        
        xSemaphoreGive(xDisplayMutex);
    }
}

// Tarefa para a entrada de usuários (Botão A)
void vTaskEntrada(void *params){
    while(true){
        if(gpio_get(button_A) == 0){
            if(xSemaphoreTake(xUsuariosSem, 0) == pdTRUE){
                usuarios++;
                led_color();
                displayOLED("Mais 1 usuario");
            }else{
                displayOLED(" Espaco cheio!");
                buzzer_play = 1;
            }
            vTaskDelay(pdMS_TO_TICKS(200)); // Debounce
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Tarefa para a saída de usuários (Botão B)
void vTaskSaida(void *params){
    while(true){
        if(gpio_get(button_B) == 0 && usuarios > 0){
            usuarios--;
            xSemaphoreGive(xUsuariosSem);
            led_color();
            displayOLED("Menos 1 usuario");
            vTaskDelay(pdMS_TO_TICKS(200)); // Debounce
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Interrupção do botão do Joystick
void gpio_callback(uint gpio, uint32_t events){
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xResetSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Tarefa para o reset de usuários (Botão do Joystick)
void vTaskReset(void *params){
    while(true){
        if(xSemaphoreTake(xResetSem, portMAX_DELAY) == pdTRUE) {
            while(usuarios > 0){
                xSemaphoreGive(xUsuariosSem);
                usuarios--;
            }
            led_color();
            displayOLED("  Reset feito");
            buzzer_play = 2;
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

// Tarefa para emissão de sinal sonoro do buzzer
void vBuzzerTask(){
    gpio_set_function(buzzer_A, GPIO_FUNC_PWM); // Define a função da porta GPIO como PWM
    gpio_set_function(buzzer_B, GPIO_FUNC_PWM); // Define a função da porta GPIO como PWM

    uint freq = 1000; // Frequência do buzzer
    uint clock_div = 4; // Divisor do clock
    uint wrap = (125000000 / (clock_div * freq)) - 1; // Define o valor do wrap para frequência escolhida

    uint slice_A = pwm_gpio_to_slice_num(buzzer_A); // Define o slice do buzzer A
    uint slice_B = pwm_gpio_to_slice_num(buzzer_B); // Define o slice do buzzer B

    pwm_set_clkdiv(slice_A, clock_div); // Define o divisor do clock para o buzzer A
    pwm_set_clkdiv(slice_B, clock_div); // Define o divisor do clock para o buzzer B
    pwm_set_wrap(slice_A, wrap); // Define o valor do wrap para o buzzer A
    pwm_set_wrap(slice_B, wrap); // Define o valor do wrap para o buzzer B
    pwm_set_chan_level(slice_A, pwm_gpio_to_channel(buzzer_A), wrap / 40); // Duty cycle para definir o Volume do buzzer A
    pwm_set_chan_level(slice_B, pwm_gpio_to_channel(buzzer_B), wrap / 40); // Duty cycle para definir o volume do buzzer B

    while(true){
        if(buzzer_play == 1){ // Beep curto
            pwm_set_enabled(slice_A, true);
            pwm_set_enabled(slice_B, true);
            vTaskDelay(pdMS_TO_TICKS(200));
            pwm_set_enabled(slice_A, false);
            pwm_set_enabled(slice_B, false);
            buzzer_play = 0;
        }else if(buzzer_play == 2){ // Beep duplo
            pwm_set_enabled(slice_A, true);
            pwm_set_enabled(slice_B, true);
            vTaskDelay(pdMS_TO_TICKS(200));
            pwm_set_enabled(slice_A, false);
            pwm_set_enabled(slice_B, false);
            vTaskDelay(pdMS_TO_TICKS(200));
            pwm_set_enabled(slice_A, true);
            pwm_set_enabled(slice_B, true);
            vTaskDelay(pdMS_TO_TICKS(200));
            pwm_set_enabled(slice_A, false);
            pwm_set_enabled(slice_B, false);
            buzzer_play = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// Função principal
int main(){
    // Inicialização do monitor serial
    stdio_init_all();

    // Inicialização dos GPIO
    gpio_init(button_A); // Inicia a GPIO 5 do botão A
    gpio_set_dir(button_A, GPIO_IN); // Define a direção da GPIO 5 do botão A como entrada
    gpio_pull_up(button_A); // Habilita o resistor de pull up da GPIO 5 do botão A
    
    gpio_init(button_B); // Inicia a GPIO 6 do botão B
    gpio_set_dir(button_B, GPIO_IN); // Define a direção da GPIO 6 do botão B como entrada
    gpio_pull_up(button_B); // Habilita o resistor de pull up da GPIO 6 do botão B

    gpio_init(joystick_PB); // Inicia a GPIO 22 do botão do Joystick
    gpio_set_dir(joystick_PB, GPIO_IN); // Define a direção da GPIO 22 do botão do Joystick como entrada
    gpio_pull_up(joystick_PB); // Habilita o resistor de pull up da GPIO 22 do botão do Joystick

    gpio_init(LED_Green); // Inicia a GPIO 11 do LED Verde
    gpio_set_dir(LED_Green, GPIO_OUT); // Define a direção da GPIO 11 do LED Verde como saída
    gpio_put(LED_Green, false); // Estado inicial do LED apagado

    gpio_init(LED_Blue); // Inicia a GPIO 12 do LED Azul
    gpio_set_dir(LED_Blue, GPIO_OUT); // Define a direção da GPIO 12 do LED Azul como saída
    gpio_put(LED_Blue, false); // Estado inicial do LED apagado

    gpio_init(LED_Red); // Inicia a GPIO 13 do LED Vermelho
    gpio_set_dir(LED_Red, GPIO_OUT); // Define a direção da GPIO 13 do LED Vermelho como saída
    gpio_put(LED_Red, false); // Estado inicial do LED aceso

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
    ssd1306_draw_string(&ssd, "EMB Controle", 16, 3); // Desenha uma string
    ssd1306_draw_string(&ssd, "Escritorio", 22, 15); // Desenha uma string
    ssd1306_draw_string(&ssd, "Usuarios:", 18, 32); // Desenha uma string
    
    ssd1306_send_data(&ssd); // Atualiza o display

    // Interrupção do botão
    gpio_set_irq_enabled_with_callback(joystick_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    xDisplayMutex = xSemaphoreCreateMutex(); // Cria o mutex
    xUsuariosSem = xSemaphoreCreateCounting(MAX_USERS, MAX_USERS); // Cria o semáforo de contagem
    xResetSem = xSemaphoreCreateBinary(); // Cria o semáforo binário

    // Cria as tarefas
    xTaskCreate(vTaskEntrada, "Botão A", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskSaida, "Botão B", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vTaskReset, "Botão do joystick", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);
    xTaskCreate(vBuzzerTask, "Buzzer", configMINIMAL_STACK_SIZE + 128, NULL, 1, NULL);

    displayOLED("");
    led_color();

    // Inicia o agendador
    vTaskStartScheduler();
    panic_unsupported();
}

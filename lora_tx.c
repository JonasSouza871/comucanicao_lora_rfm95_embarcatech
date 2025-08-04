#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
#include "rfm95_lora.h"

// Definições do display
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define ENDERECO_DISP 0x3C
#define DISP_W 128
#define DISP_H 64
ssd1306_t ssd;

void setup_display() {
    i2c_init(I2C_PORT_DISP, 400 * 1000);
    gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_DISP);
    gpio_pull_up(I2C_SCL_DISP);

    // Chamada correta com 6 argumentos
    ssd1306_init(&ssd, DISP_W, DISP_H, false, ENDERECO_DISP, I2C_PORT_DISP);
    ssd1306_config(&ssd); // Adicionado para configurar os registradores do display

    // Usando as funções corretas da sua biblioteca
    ssd1306_fill(&ssd, false); // Equivalente a clear()
    ssd1306_draw_string(&ssd, "TX LoRa", 10, 10, false);
    ssd1306_send_data(&ssd); // Equivalente a show()
}

int main() {
    stdio_init_all();
    sleep_ms(2000); 
    printf("Iniciando Transmissor LoRa (TX)...\n");

    setup_display();

    if (!lora_init()) {
        printf("Falha na comunicacao com o RFM95. Travando. ❌\n");
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, "RFM95 FALHOU!", 10, 20, false);
        ssd1306_send_data(&ssd);
        while(1);
    }
    printf("Comunicacao com RFM95 OK! ✅\n");

    lora_set_power(17);

    int counter = 0;
    char message_buffer[50];

    while (1) {
        snprintf(message_buffer, sizeof(message_buffer), "Ola #%d", counter);
        
        lora_send_packet((uint8_t*)message_buffer, strlen(message_buffer));
        
        printf("Pacote enviado: '%s'\n", message_buffer);
        
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, "Pacote Enviado:", 5, 10, false);
        ssd1306_draw_string(&ssd, message_buffer, 5, 30, false);
        ssd1306_send_data(&ssd);
        
        counter++;
        sleep_ms(5000);
    }

    return 0;
}

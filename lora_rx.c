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
    ssd1306_draw_string(&ssd, "RX LoRa", 10, 10, false);
    ssd1306_draw_string(&ssd, "Aguardando...", 10, 30, false);
    ssd1306_send_data(&ssd); // Equivalente a show()
}

int main() {
    stdio_init_all();
    sleep_ms(2000); 
    printf("Iniciando Receptor LoRa (RX)...\n");

    setup_display();

    if (!lora_init()) {
        printf("Falha na comunicacao com o RFM95. Travando. ❌\n");
        ssd1306_fill(&ssd, false);
        ssd1306_draw_string(&ssd, "RFM95 FALHOU!", 10, 20, false);
        ssd1306_send_data(&ssd);
        while(1);
    }
    printf("Comunicacao com RFM95 OK! ✅\n");
    printf("Aguardando pacotes...\n");

    uint8_t buffer[256];

    while (1) {
        int packet_size = lora_receive_packet(buffer, sizeof(buffer));
        
        if (packet_size > 0) {
            buffer[packet_size] = '\0'; 
            
            int rssi = lora_packet_rssi();
            float snr = lora_packet_snr();

            printf("--------------------------------\n");
            printf("Pacote Recebido!\n");
            printf("  Mensagem: '%s'\n", buffer);
            printf("  Bytes: %d\n", packet_size);
            printf("  RSSI: %d dBm\n", rssi);
            printf("  SNR: %.2f dB\n", snr);
            printf("--------------------------------\n");

            char display_line[32];
            ssd1306_fill(&ssd, false);
            ssd1306_draw_string(&ssd, (char*)buffer, 5, 10, false);
            
            snprintf(display_line, sizeof(display_line), "RSSI:%d SNR:%.1f", rssi, snr);
            ssd1306_draw_string(&ssd, display_line, 5, 30, false);
            ssd1306_send_data(&ssd);
        }
        
        sleep_ms(10);
    }

    return 0;
}

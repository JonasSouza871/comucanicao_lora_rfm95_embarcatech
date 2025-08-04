#ifndef RFM95_LORA_H
#define RFM95_LORA_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdbool.h>


// Definições de Pinos e SPI

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_RST  20

// Frequência de operação (915 MHz para o Brasil)
#define LORA_FREQUENCY_HZ 915E6


// Funções Públicas da Biblioteca

// Inicializa o hardware SPI e o módulo RFM95
// Retorna true se a comunicação foi bem-sucedida, false caso contrário
bool lora_init();

// Coloca o módulo em modo de espera (standby) - baixo consumo, pronto para TX/RX
void lora_idle();

// Coloca o módulo em modo de hibernação (sleep) - menor consumo de energia
void lora_sleep();

// Configura a frequência de operação do rádio em Hertz (ex: 915000000)
void lora_set_frequency(long frequency);

// Configura a potência de transmissão em dBm (entre 2 e 17 para PA_BOOST)
void lora_set_power(uint8_t power);

// Envia um pacote de dados
// buffer: ponteiro para os dados, size: número de bytes
void lora_send_packet(const uint8_t* buffer, uint8_t size);

// Tenta receber um pacote (modo não-bloqueante) - deve ser chamada em loop
// buffer: buffer de destino, max_size: tamanho máximo do buffer
// Retorna: número de bytes recebidos ou 0 se nenhum pacote foi recebido
int lora_receive_packet(uint8_t* buffer, int max_size);

// Obtém o RSSI do último pacote recebido em dBm
int lora_packet_rssi();

// Obtém o SNR do último pacote recebido em dB
float lora_packet_snr();

#endif // RFM95_LORA_H

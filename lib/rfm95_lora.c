#include "rfm95_lora.h"
#include <string.h>

// Definições dos Registradores LoRa (privado)
#define REG_FIFO                  0x00
#define REG_OP_MODE               0x01
#define REG_FRF_MSB               0x06
#define REG_FRF_MID               0x07
#define REG_FRF_LSB               0x08
#define REG_PA_CONFIG             0x09
#define REG_LNA                   0x0C
#define REG_FIFO_ADDR_PTR         0x0D
#define REG_FIFO_TX_BASE_ADDR     0x0E
#define REG_FIFO_RX_BASE_ADDR     0x0F
#define REG_FIFO_RX_CURRENT_ADDR  0x10
#define REG_IRQ_FLAGS             0x12
#define REG_RX_NB_BYTES           0x13
#define REG_PKT_SNR_VALUE         0x19
#define REG_PKT_RSSI_VALUE        0x1A
#define REG_MODEM_CONFIG_1        0x1D
#define REG_MODEM_CONFIG_2        0x1E
#define REG_PREAMBLE_MSB          0x20
#define REG_PREAMBLE_LSB          0x21
#define REG_PAYLOAD_LENGTH        0x22
#define REG_MAX_PAYLOAD_LENGTH    0x23
#define REG_DIO_MAPPING_1         0x40
#define REG_VERSION               0x42
#define REG_PA_DAC                0x4D

// Modos de Operação
#define MODE_LORA                 0x80
#define MODE_SLEEP                0x00
#define MODE_STDBY                0x01
#define MODE_TX                   0x03
#define MODE_RX_CONTINUOUS        0x05
#define MODE_RX_SINGLE            0x06

// Máscaras de interrupção
#define IRQ_RX_DONE_MASK          0x40
#define IRQ_TX_DONE_MASK          0x08
#define IRQ_PAYLOAD_CRC_ERROR_MASK 0x20

// Frequência do cristal do módulo (Hz)
#define RF_CRYSTAL_FREQ_HZ        32000000

// ============================================================================
// Funções Privadas
// ============================================================================

/* Reinicia o módulo forçando o pino RST */
static void rmf95_reset() {
    gpio_put(PIN_RST, 0);
    sleep_ms(10);
    gpio_put(PIN_RST, 1);
    sleep_ms(10);
}

/* Leitura de um registrador (1 byte) via SPI */
static uint8_t rmf95_read_reg(uint8_t reg) {
    uint8_t tx[] = { reg & 0x7F, 0x00 };   // bit 7=0 → leitura
    uint8_t rx[2];
    gpio_put(PIN_CS, 0);
    spi_write_read_blocking(SPI_PORT, tx, rx, 2);
    gpio_put(PIN_CS, 1);
    return rx[1];
}

/* Escrita de um registrador (1 byte) via SPI */
static void rmf95_write_reg(uint8_t reg, uint8_t value) {
    uint8_t tx[] = { reg | 0x80, value };  // bit 7=1 → escrita
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, tx, 2);
    gpio_put(PIN_CS, 1);
}

/* Lê um bloco de dados a partir da FIFO */
static void rmf95_read_fifo(uint8_t* buffer, uint8_t length) {
    uint8_t addr = REG_FIFO & 0x7F;
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &addr, 1);
    spi_read_blocking(SPI_PORT, 0, buffer, length);
    gpio_put(PIN_CS, 1);
}

/* Escreve um bloco de dados na FIFO */
static void rmf95_write_fifo(const uint8_t* buffer, uint8_t length) {
    uint8_t addr = REG_FIFO | 0x80;
    gpio_put(PIN_CS, 0);
    spi_write_blocking(SPI_PORT, &addr, 1);
    spi_write_blocking(SPI_PORT, buffer, length);
    gpio_put(PIN_CS, 1);
}

// ============================================================================
// Implementação das Funções Públicas
// ============================================================================

bool lora_init() {
    /* --- Configuração básica do barramento SPI --- */
    spi_init(SPI_PORT, 1 * 1000 * 1000);          // 1 MHz
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    /* --- Pinos de controle CS e RST --- */
    gpio_init(PIN_CS);   gpio_set_dir(PIN_CS, GPIO_OUT);   gpio_put(PIN_CS, 1);
    gpio_init(PIN_RST);  gpio_set_dir(PIN_RST, GPIO_OUT);

    /* --- Reset do módulo e verificação da versão --- */
    rmf95_reset();
    if (rmf95_read_reg(REG_VERSION) != 0x12) {     // 0x12 é a versão esperada
        return false;
    }

    /* --- Entra em modo sleep para configurar com segurança --- */
    lora_sleep();

    /* --- Frequência de operação --- */
    lora_set_frequency(LORA_FREQUENCY_HZ);

    /* --- Ponteiros base da FIFO (TX e RX no início) --- */
    rmf95_write_reg(REG_FIFO_TX_BASE_ADDR, 0x00);
    rmf95_write_reg(REG_FIFO_RX_BASE_ADDR, 0x00);

    /* --- LNA: ativa ganho máximo para melhor sensibilidade --- */
    rmf95_write_reg(REG_LNA, rmf95_read_reg(REG_LNA) | 0x03);

    /* --- Configuração do modem
         BW 125 kHz, CR 4/5, CRC on, SF7 --- */
    rmf95_write_reg(REG_MODEM_CONFIG_1, 0x72);
    rmf95_write_reg(REG_MODEM_CONFIG_2, 0x74);

    /* --- Preâmbulo de 8 símbolos --- */
    rmf95_write_reg(REG_PREAMBLE_MSB, 0x00);
    rmf95_write_reg(REG_PREAMBLE_LSB, 0x08);

    /* --- Volta para standby, pronto para TX/RX --- */
    lora_idle();
    return true;
}

/* Converte frequência em Hz para os três registradores FRF */
void lora_set_frequency(long frequency) {
    uint64_t frf = ((uint64_t)frequency << 19) / RF_CRYSTAL_FREQ_HZ;
    rmf95_write_reg(REG_FRF_MSB, (uint8_t)(frf >> 16));
    rmf95_write_reg(REG_FRF_MID, (uint8_t)(frf >> 8));
    rmf95_write_reg(REG_FRF_LSB, (uint8_t) frf);
}

/* Define a potência de transmissão (2 dBm–17 dBm) usando PA_BOOST */
void lora_set_power(uint8_t power) {
    if (power > 17) power = 17;
    if (power < 2)  power = 2;
    rmf95_write_reg(REG_PA_CONFIG, 0x80 | (power - 2));   // 0x80 → PA_BOOST
}

void lora_sleep() {
    rmf95_write_reg(REG_OP_MODE, MODE_LORA | MODE_SLEEP);
}

void lora_idle() {
    rmf95_write_reg(REG_OP_MODE, MODE_LORA | MODE_STDBY);
}

/* Envia um pacote: grava FIFO, aciona modo TX e espera IRQ_TX_DONE */
void lora_send_packet(const uint8_t* buffer, uint8_t size) {
    lora_idle();
    rmf95_write_reg(REG_FIFO_ADDR_PTR, 0);
    rmf95_write_fifo(buffer, size);
    rmf95_write_reg(REG_PAYLOAD_LENGTH, size);

    rmf95_write_reg(REG_OP_MODE, MODE_LORA | MODE_TX);

    while (!(rmf95_read_reg(REG_IRQ_FLAGS) & IRQ_TX_DONE_MASK)) {
        sleep_ms(1);                                    // espera TX terminar
    }
    rmf95_write_reg(REG_IRQ_FLAGS, IRQ_TX_DONE_MASK);   // limpa flag
    lora_idle();
}

/* Recebe pacote em modo contínuo; retorna tamanho ou 0 se nada recebido */
int lora_receive_packet(uint8_t* buffer, int max_size) {
    rmf95_write_reg(REG_OP_MODE, MODE_LORA | MODE_RX_CONTINUOUS);

    uint8_t irq = rmf95_read_reg(REG_IRQ_FLAGS);
    if (irq & IRQ_RX_DONE_MASK) {
        rmf95_write_reg(REG_IRQ_FLAGS, IRQ_RX_DONE_MASK); // limpa flag

        if (irq & IRQ_PAYLOAD_CRC_ERROR_MASK) {
            return 0;                                     // CRC inválido
        }

        uint8_t len = rmf95_read_reg(REG_RX_NB_BYTES);
        if (len > max_size) len = max_size;

        uint8_t fifo_addr = rmf95_read_reg(REG_FIFO_RX_CURRENT_ADDR);
        rmf95_write_reg(REG_FIFO_ADDR_PTR, fifo_addr);
        rmf95_read_fifo(buffer, len);
        return len;
    }
    return 0;   // nada recebido
}

/* RSSI absoluto: (-157 dBm para 915 MHz) + valor lido */
int lora_packet_rssi() {
    return (rmf95_read_reg(REG_PKT_RSSI_VALUE) - 157);
}

/* SNR em dB (valor fracionário; cada unidade = 0,25 dB) */
float lora_packet_snr() {
    return ((int8_t)rmf95_read_reg(REG_PKT_SNR_VALUE)) * 0.25f;
}

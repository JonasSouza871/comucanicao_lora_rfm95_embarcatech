# 🚀 Pico LoRa Communicator – RFM95

<div align="center">

![Linguagem](https://img.shields.io/badge/Linguagem-C%2FC%2B%2B-blue?style=for-the-badge)
![Hardware](https://img.shields.io/badge/Hardware-Raspberry%20Pi%20Pico-E01244?style=for-the-badge)
![Tecnologia](https://img.shields.io/badge/Tecnologia-LoRa-E43A47?style=for-the-badge)
![Licença](https://img.shields.io/badge/Licen%C3%A7a-MIT-yellow?style=for-the-badge)

</div>

Uma biblioteca e exemplos para comunicação LoRa com o módulo RFM95 no Raspberry Pi Pico, incluindo integração com display OLED.

---

### 📝 Descrição Breve

Este projeto fornece uma biblioteca C/C++ para controlar o rádio transceptor LoRa RFM95 e dois exemplos práticos (Transmissor e Receptor) para o Raspberry Pi Pico. O objetivo é facilitar a implementação de comunicação sem fio de longo alcance. Os exemplos demonstram como enviar e receber pacotes, exibindo o status da operação e os dados recebidos (incluindo RSSI e SNR) em um display OLED SSD1306, criando uma solução de comunicação completa e de fácil depuração.

---

### ✨ Funcionalidades Principais

-   **✅ Biblioteca LoRa RFM95:** Funções encapsuladas para inicializar, configurar frequência/potência, enviar e receber pacotes de dados de forma simplificada.
-   **✅ Exemplo de Transmissor (TX):** Envia uma mensagem com um contador que se incrementa a cada 5 segundos, exibindo o pacote enviado em um display OLED local.
-   **✅ Exemplo de Receptor (RX):** Fica em modo de escuta contínua. Ao receber um pacote, exibe a mensagem, o RSSI (Indicador de Força do Sinal Recebido) e o SNR (Relação Sinal-Ruído) no display OLED e no terminal serial.
-   **✅ Integração com Display OLED:** Ambos os exemplos utilizam um display SSD1306 para fornecer feedback visual em tempo real, tornando o sistema autônomo e fácil de monitorar.
-   **✅ Configuração para 915 MHz:** A biblioteca está pré-configurada para operar na faixa de frequência de 915 MHz.


### ⚙ Hardware Necessário (para 1 par TX/RX)

| Componente | Quant. | Observações |
| :--- | :---: | :--- |
| Raspberry Pi Pico | 2 | Um para o transmissor e um para o receptor. |
| Módulo LoRa RFM95 | 2 | O coração da comunicação sem fio. |
| Display OLED 128x64 | 2 | Para a interface visual (I2C, SSD1306). |
| Antena para 915 MHz | 2 | Essencial para o alcance do sinal. |
| Protoboard e Jumpers | - | Para montagem dos dois circuitos. |

---

### 🔌 Conexões e Configuração

A pinagem é a mesma para os módulos Transmissor e Receptor.

**Módulo RFM95 (SPI):**
-   `MISO` -> `GPIO 16`
-   `CS` -> `GPIO 17`
-   `SCK` -> `GPIO 18`
-   `MOSI` -> `GPIO 19`
-   `RST` -> `GPIO 20`

**Display OLED (I2C):**
-   `SDA` -> `GPIO 14`
-   `SCL` -> `GPIO 15`

> **⚠ Importante:** Garanta um `GND` comum entre o Pico, o RFM95 e o display. Lembre-se de soldar uma antena apropriada ao pino `ANT` do módulo RFM95.

---

### 🚀 Começando

#### Pré-requisitos de Software

-   **SDK:** Raspberry Pi Pico SDK
-   **Linguagem:** C/C++
-   **IDE Recomendada:** VS Code com a extensão "CMake Tools"
-   **Toolchain:** ARM GNU Toolchain
-   **Build System:** CMake

#### Configuração e Compilação

O `CMakeLists.txt` está configurado para gerar dois executáveis: `lora_tx` e `lora_rx`.

```bash
# 1. Clone o repositório do projeto
git clone [https://github.com/JonasSouza871/Comunica-o_lora_m-dulo_RFM95W-.git](https://github.com/JonasSouza871/Comunica-o_lora_m-dulo_RFM95W-.git)
cd Comunica-o_lora_m-dulo_RFM95W-

# 2. Configure o ambiente de build com CMake
# (Certifique-se de que o PICO_SDK_PATH está definido como variável de ambiente)
mkdir build
cd build
cmake ..

# 3. Compile os projetos (use -j para acelerar)
# Você pode compilar tudo de uma vez
make -j$(nproc)

# Ou compilar apenas um alvo específico
# make lora_tx
# make lora_rx

# 4. Carregue o firmware em cada Pico
# Conecte o primeiro Pico segurando BOOTSEL e copie o 'lora_tx.uf2'.
# Conecte o segundo Pico segurando BOOTSEL e copie o 'lora_rx.uf2'.
cp lora_tx.uf2 /media/user/RPI-RP2
cp lora_rx.uf2 /media/user/RPI-RP2
```

---

### 📁 Estrutura do Projeto

```
.
├── build/              # Diretório de compilação (gerado)
├── lib/                # Bibliotecas de hardware e de terceiros
│   ├── font.h
│   ├── rfm95_lora.c
│   ├── rfm95_lora.h
│   ├── ssd1306.c
│   └── ssd1306.h
├── .gitignore
├── CMakeLists.txt      # Script de build principal do CMake
├── lora_rx.c           # Código fonte do Receptor
├── lora_tx.c           # Código fonte do Transmissor
└── README.md
```

---

### 🐛 Solução de Problemas

-   **"RFM95 FALHOU!" no display:**
    -   Indica falha na comunicação SPI. Verifique todas as conexões SPI (MISO, MOSI, SCK, CS) e o pino de Reset (RST).
    -   Certifique-se de que o módulo RFM95 está sendo alimentado corretamente com 3.3V.
-   **Nenhum pacote é recebido:**
    -   Verifique se as antenas estão conectadas corretamente em ambos os módulos.
    -   Confirme que a frequência definida em `lora_set_frequency()` é a mesma para o TX e o RX.
    -   Aproxime os módulos para um teste inicial.
-   **Display OLED não mostra nada:**
    -   Verifique as conexões I2C (SDA e SCL) e a alimentação do display.
    -   Confirme se o endereço I2C do display (`0x3C`) está correto.

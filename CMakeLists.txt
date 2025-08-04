# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# Versão mínima do CMake necessária
cmake_minimum_required(VERSION 3.13)

# O arquivo pico_sdk_import.cmake deve estar no mesmo diretório ou em um conhecido.
include(pico_sdk_import.cmake)

# Define o nome do projeto e as linguagens
project(lora_communication_project C CXX ASM)

# Define os padrões das linguagens C e C++
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializa o SDK
pico_sdk_init()

# Adiciona o diretório 'lib' ao caminho de busca por arquivos de cabeçalho (.h)
include_directories(lib)

# --- DEFINIÇÃO DO EXECUTÁVEL ---
# Para alternar entre o transmissor (TX) e o receptor (RX),
# comente uma linha e descomente a outra. Apenas uma pode estar ativa.

set(EXECUTABLE_NAME lora_rx)  # para utilizar o Rx
#set(EXECUTABLE_NAME lora_tx) #Para utilizar o codigo tx


# Lista dos arquivos .c da sua biblioteca que precisam ser compilados.
set(LIB_SOURCES
    lib/rfm95_lora.c
    lib/ssd1306.c
)

# Adiciona o executável ao projeto.
add_executable(${EXECUTABLE_NAME}
    ${EXECUTABLE_NAME}.c
    ${LIB_SOURCES}
)

# Adiciona as bibliotecas do Pico SDK que seu projeto utiliza.
target_link_libraries(${EXECUTABLE_NAME}
    pico_stdlib
    hardware_spi
    hardware_i2c
)

# Cria os arquivos .uf2, .hex, etc., para gravação no microcontrolador.
pico_add_extra_outputs(${EXECUTABLE_NAME})

# Habilita a saída de log via USB ou UART, dependendo da configuração do SDK.
pico_enable_stdio_usb(${EXECUTABLE_NAME} 1)
pico_enable_stdio_uart(${EXECUTABLE_NAME} 0)

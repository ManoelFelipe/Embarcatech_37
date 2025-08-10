# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Atividade_05 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()


# ---------------------------------------------
# Fontes do executável
# ---------------------------------------------
# Add executable. Default name is the project name, version 0.1
# Aqui listamos todos os arquivos .c: 
#   - o principal (src/Atividade_05.c)  
#   - o driver do SSD1306 em I²C   (lib/ssd1306_i2c.c)
add_executable(Atividade_05
    src/Atividade_05.c
    lib/ssd1306_i2c.c
)

# Define nome e versão do binário gerado
pico_set_program_name(Atividade_05 "Atividade_05.elf")
pico_set_program_version(Atividade_05 "0.1")
#pico_set_program_name(Atividade_05 "src/Atividade_05.c")



# ---------------------------------------------
# Configuração de saída de debug / stdio
# ---------------------------------------------
# Modify the below lines to enable/disable output over UART/USB
# Desabilita UART, habilita USB para printf() via USB-CDC
pico_enable_stdio_uart(Atividade_05 0)
pico_enable_stdio_usb(Atividade_05 1)

# ---------------------------------------------
# Diretórios de include
# ---------------------------------------------
# Add the standard include files to the build
# Adiciona caminhos para que #include "ssd1306.h" etc funcione
target_include_directories(Atividade_05 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}       # raiz do projeto
    ${CMAKE_CURRENT_LIST_DIR}/lib   # headers em lib/
    ${CMAKE_CURRENT_LIST_DIR}/src   # headers em src/
)

# ---------------------------------------------
# Bibliotecas do Pico e hardware
# ---------------------------------------------
# Linka as libs internas do SDK e módulos de hardware
target_link_libraries(Atividade_05
    pico_stdlib         # stdio, timer, GPIO abstractions
    hardware_adc        # driver ADC
    hardware_dma        # driver DMA
    hardware_i2c        # driver I2C
    # — removido “ssd1306_i2c” aqui pois já compilamos o .c diretamente
)

# ---------------------------------------------
# Geração dos binários adicionais (.uf2, .hex, .bin)
# ---------------------------------------------
pico_add_extra_outputs(Atividade_05)


##
# CMakeLists.txt - Projeto Atividade_09
# Descrição: Este arquivo controla o processo de build com CMake. Comentários foram adicionados
#            em português e seguem estilo Doxygen quando aplicável.
#            (Gerado em 25/05/2025)
##

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

project(Atividade_09 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Atividade_09 
                src/Atividade_09.c
                src/setup.c 
                src/irq_handlers.c 
                src/tarefa1_temp.c 
                src/tarefa2_display.c
                src/tarefa3_tendencia.c
                src/tarefa4_controla_neopixel.c
                src/testes_cores.c
                lib/ssd1306/display_utils.c
                lib/ssd1306/big_string_drawer.c
                lib/ssd1306/ssd1306_i2c.c
                lib/ssd1306/font_big_logo_data.c              
                lib/LabNeoPixel/neopixel_driver.c
                lib/LabNeoPixel/efeitos.c
                )

pico_set_program_name(Atividade_09 "Atividade_09")
pico_set_program_version(Atividade_09 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Atividade_09 0)
pico_enable_stdio_usb(Atividade_09 1)

# Add the standard library to the build
target_link_libraries(Atividade_09
        pico_stdlib
        hardware_adc
        hardware_dma
        hardware_irq
        hardware_watchdog
        hardware_i2c
        hardware_pio
        )

# Add the standard include files to the build
target_include_directories(Atividade_09 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lib/LabNeoPixel
        ${CMAKE_CURRENT_LIST_DIR}/lib/ssd1306
        ${CMAKE_CURRENT_LIST_DIR}/lib
        ${CMAKE_CURRENT_LIST_DIR}/src
)

# ——— PIO header (ws2818b.pio) -------------------------------
pico_generate_pio_header(Atividade_09
    ${CMAKE_CURRENT_LIST_DIR}/lib/LabNeoPixel/ws2818b.pio
)

# ——— UF2 / bin / map ----------------------------------------
pico_add_extra_outputs(Atividade_09)

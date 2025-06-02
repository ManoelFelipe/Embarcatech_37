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

project(Atividade_10 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Atividade_10 
            src/Atividade_10.c 
            src/funcao_atividade_.c 
            lib/neopixel/funcoes_neopixel.c
        )

pico_set_program_name(Atividade_10 "Atividade_10")
pico_set_program_version(Atividade_10 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Atividade_10 0)
pico_enable_stdio_usb(Atividade_10 1)

# Add the standard library to the build
target_link_libraries(Atividade_10
        pico_stdlib
        hardware_adc 
        hardware_gpio 
        hardware_timer 
        pico_multicore 
        hardware_pio 
        hardware_clocks
    )

# Add the standard include files to the build
target_include_directories(Atividade_10 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/lib
        ${CMAKE_CURRENT_LIST_DIR}/lib/neopixel
        ${CMAKE_CURRENT_LIST_DIR}/src

)

pico_add_extra_outputs(Atividade_10)

pico_generate_pio_header(Atividade_10 
            ${CMAKE_CURRENT_LIST_DIR}/lib/neopixel/ws2818b.pio
        )
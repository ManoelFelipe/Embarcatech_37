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

project(Atividade_02 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(Atividade_02 
            src/Atividade_02.c
            src/temperature.c
            src/web_server.c
            src/wifi_ap.c
            src/led_control.c
            src/debug.c
            libs/dhcpserver/dhcpserver.c
            libs/dnsserver/dnsserver.c 
        )

pico_set_program_name(Atividade_02 "Atividade_02")
pico_set_program_version(Atividade_02 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Atividade_02 0)
pico_enable_stdio_usb(Atividade_02 01)

# Add the standard library to the build
target_link_libraries(Atividade_02
        pico_stdlib
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdio_usb
        hardware_gpio
        hardware_adc
        hardware_i2c
    )

# Add the standard include files to the build
target_include_directories(Atividade_02 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        #${CMAKE_CURRENT_LIST_DIR}/..
        #${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/libs
        ${CMAKE_CURRENT_LIST_DIR}/libs/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/libs/dnsserver
)

pico_configure_ip4_address(Atividade_02 PRIVATE
        CYW43_DEFAULT_IP_AP_ADDRESS 192.168.4.1
        )

pico_add_extra_outputs(Atividade_02)


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

project(Atividade_08 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(Atividade_08_background
        src/Atividade_08.c
        src/oled_display.c
        src/alarm_control.c
        src/network_manager.c
        dhcpserver/dhcpserver.c
        dnsserver/dnsserver.c
        lib/ssd1306/ssd1306_i2c.c
        )

#pico_set_program_name(Atividade_08_background "Atividade_08")
#pico_set_program_version(Atividade_08_background "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Atividade_08_background 0)
pico_enable_stdio_usb(Atividade_08_background 1)

# Add the standard include files to the build
target_include_directories(Atividade_08_background PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        #${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        ${CMAKE_CURRENT_LIST_DIR}/lib/
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
        ${CMAKE_CURRENT_LIST_DIR}/src/
        ${CMAKE_CURRENT_LIST_DIR}/lib/ssd1306
        )

# Add any user requested libraries
target_link_libraries(Atividade_08_background
        pico_cyw43_arch_lwip_threadsafe_background
        pico_stdlib
        hardware_gpio
        hardware_i2c
        )

        # You can change the address below to change the address of the access point
pico_configure_ip4_address(Atividade_08_background PRIVATE
        CYW43_DEFAULT_IP_AP_ADDRESS 192.168.4.1
        )

pico_add_extra_outputs(Atividade_08_background)



add_executable(Atividade_08_poll
        src/Atividade_08.c
        src/oled_display.c
        src/alarm_control.c
        src/network_manager.c
        dhcpserver/dhcpserver.c
        dnsserver/dnsserver.c
        lib/ssd1306/ssd1306_i2c.c
        )

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Atividade_08_poll 0)
pico_enable_stdio_usb(Atividade_08_poll 1)

target_include_directories(Atividade_08_poll PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        #${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        ${CMAKE_CURRENT_LIST_DIR}/lib/
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
        ${CMAKE_CURRENT_LIST_DIR}/src/
        ${CMAKE_CURRENT_LIST_DIR}/lib/ssd1306
        )

target_link_libraries(Atividade_08_poll
        pico_cyw43_arch_lwip_poll
        pico_stdlib
        hardware_gpio
        hardware_i2c
        )

# You can change the address below to change the address of the access point
pico_configure_ip4_address(Atividade_08_poll PRIVATE
        CYW43_DEFAULT_IP_AP_ADDRESS 192.168.4.1
        )

pico_add_extra_outputs(Atividade_08_poll)
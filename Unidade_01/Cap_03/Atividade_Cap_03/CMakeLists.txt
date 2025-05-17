# --------------------------------------------------------
# CMake project for Raspberry Pi Pico W – Atividade_Cap_03
# --------------------------------------------------------
cmake_minimum_required(VERSION 3.13)

# Linguagens
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

# Placa‑alvo
set(PICO_BOARD pico_w CACHE STRING "Board type for Pico SDK")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
project(Atividade_Cap_03 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# ------------------------------------------------------------------
# Executável principal
# ------------------------------------------------------------------
add_executable(Atividade_Cap_03
    Atividade_Cap_03.c
    ws2812.c
)

# Gera cabeçalho do PIO (depois que o target existe!)
pico_generate_pio_header(Atividade_Cap_03 ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)

# Metadados
pico_set_program_name    (Atividade_Cap_03 "Atividade_Cap_03")
pico_set_program_version (Atividade_Cap_03 "0.1")

# I/O padrão – USB CDC somente
pico_enable_stdio_uart(Atividade_Cap_03 0)
pico_enable_stdio_usb (Atividade_Cap_03 1)

# Bibliotecas necessárias
target_link_libraries(Atividade_Cap_03 PUBLIC
    pico_stdlib
    hardware_adc
    hardware_timer
    hardware_sync
    hardware_pio
    hardware_dma        # (futuro – não usado diretamente)
)

# Inclui diretório atual para headers
target_include_directories(Atividade_Cap_03 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Saídas extras - UF2/ELF/BIN
pico_add_extra_outputs(Atividade_Cap_03)

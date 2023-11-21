cmake_minimum_required(VERSION 3.27)

include(pico_sdk_import.cmake)

project(zxpicojoym)

pico_sdk_init()

add_executable(zxpicojoym
    zxpicojoym.c
)

pico_generate_pio_header(zxpicojoym ${CMAKE_CURRENT_LIST_DIR}/zxpicojoym.pio)
pico_generate_pio_header(zxpicojoym ${CMAKE_CURRENT_LIST_DIR}/zxpicojoym_k.pio)
pico_generate_pio_header(zxpicojoym ${CMAKE_CURRENT_LIST_DIR}/zxpicojoypress.pio)

target_link_libraries(zxpicojoym pico_stdlib hardware_i2c hardware_pio hardware_dma hardware_flash)

pico_add_extra_outputs(zxpicojoym)

pico_enable_stdio_usb(zxpicojoym 1) 
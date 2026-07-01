cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(led_chaser C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(led_chaser
    main.c
)

target_link_libraries(led_chaser
    pico_stdlib
    hardware_pwm
)

pico_enable_stdio_usb(led_chaser 1)
pico_enable_stdio_uart(led_chaser 0)

pico_add_extra_outputs(led_chaser)

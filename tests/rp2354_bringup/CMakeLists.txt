cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(rp2354_bringup C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(rp2354_bringup
    main.c
)

target_link_libraries(rp2354_bringup
    pico_stdlib
)

pico_enable_stdio_usb(rp2354_bringup 1)
pico_enable_stdio_uart(rp2354_bringup 0)

pico_add_extra_outputs(rp2354_bringup)

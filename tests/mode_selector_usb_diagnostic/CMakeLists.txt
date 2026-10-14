cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(mode_selector_usb_diagnostic C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(mode_selector_usb_diagnostic
    main.c
)

target_include_directories(mode_selector_usb_diagnostic PRIVATE
    ../../firmware/src
)

target_link_libraries(mode_selector_usb_diagnostic
    pico_stdlib
)

pico_enable_stdio_usb(mode_selector_usb_diagnostic 1)
pico_enable_stdio_uart(mode_selector_usb_diagnostic 0)

pico_add_extra_outputs(mode_selector_usb_diagnostic)

set(DIAGNOSTIC_DIST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist")
add_custom_command(TARGET mode_selector_usb_diagnostic POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${DIAGNOSTIC_DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE_DIR:mode_selector_usb_diagnostic>/mode_selector_usb_diagnostic.uf2"
            "${DIAGNOSTIC_DIST_DIR}/mode_selector_usb_diagnostic.uf2"
    COMMENT "Copying UF2 to ${DIAGNOSTIC_DIST_DIR}/mode_selector_usb_diagnostic.uf2"
)

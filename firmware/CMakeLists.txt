cmake_minimum_required(VERSION 3.13)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(composer_midicv C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(composer_midicv
    src/main.c
    src/app/app.c
    src/config/mode_config.c
    src/debug/debug_log.c
    src/debug/debug_shell.c
    src/hardware/dac_mcp4822.c
    src/hardware/gate_output.c
    src/hardware/led.c
    src/hardware/mode_selector.c
    src/hardware/mode_selector_hw.c
    src/midi/midi_parser.c
    src/midi/midi_router.c
    src/midi/midi_stream_parser.c
    src/midi/trs_midi.c
    src/midi/usb_midi.c
    src/modes/mode_cc_convert.c
    src/modes/mode_clocked_chord_arp.c
    src/modes/mode_clocked_note.c
    src/modes/mode_drum4_velocity.c
    src/modes/mode_drum8_trigger.c
    src/modes/mode_dual_mono.c
    src/modes/mode_mono_perf.c
    src/modes/mode_mono_random_clock.c
    src/modes/mode_placeholder_test.c
    src/modes/mode_poly4.c
    src/modes/mode_stub.c
    src/music/calibration.c
    src/music/note_stack.c
    src/music/pitch_cv.c
    src/music/voice_allocator.c
    src/ui/led_chaser.c
    src/ui/led_feedback.c
    src/ui/mode_switch_indicator.c
    src/usb/usb_descriptors.c
)

target_include_directories(composer_midicv PRIVATE
    src
    src/usb
)

target_link_libraries(composer_midicv
    pico_stdlib
    tinyusb_device
    tinyusb_board
    hardware_flash
    hardware_adc
    hardware_pio
    hardware_pwm
    hardware_uart
    hardware_sync
)

pico_enable_stdio_usb(composer_midicv 0)
pico_enable_stdio_uart(composer_midicv 0)

pico_add_extra_outputs(composer_midicv)

set(COMPOSER_MIDICV_DIST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dist")
add_custom_command(TARGET composer_midicv POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${COMPOSER_MIDICV_DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE_DIR:composer_midicv>/composer_midicv.uf2"
            "${COMPOSER_MIDICV_DIST_DIR}/composer_midicv.uf2"
    COMMENT "Copying UF2 to ${COMPOSER_MIDICV_DIST_DIR}/composer_midicv.uf2"
)

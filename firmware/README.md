# Composer MIDICV Firmware

C firmware for the **v1.0 RP2354A four-CV / four-Gate module**, using Pico SDK 2.2.0 and TinyUSB. The developer has verified USB MIDI, TRS MIDI, all CV/Gate channels, all six panel modes and voltage calibration on assembled v1.0 hardware.

See the [project README](../README.md) for features and hardware files, and the [中文使用手册](../docs/USER_MANUAL.md) for connections, mode tables, flashing, calibration and troubleshooting.

## Build

Requirements: CMake 3.22+, Ninja, Python 3, and an Arm GNU embedded toolchain with newlib / `nosys.specs` available on `PATH`. The local toolchain is 14.2.Rel1. Run from the **project root**:

```sh
git submodule update --init --recursive third_party/pico-sdk
export PICO_SDK_PATH="$PWD/third_party/pico-sdk"
cmake -S firmware -B firmware/build-arm -G Ninja \
  -DPICO_PLATFORM=rp2350-arm-s -DPICO_BOARD=pico2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build firmware/build-arm --parallel
```

For Windows PowerShell commands, see the root README. The `pico2` SDK configuration targets the RP2350 ARM platform; board pin assignments come from `src/hardware/board_pins.h`.

Outputs: `firmware/build-arm/composer_midicv.uf2` and the automatic copy at `firmware/dist/composer_midicv.uf2`.

## Flash

Provide module power and connect a USB data cable. Hold **BOOTSEL**, press and release **RUN**, then release BOOTSEL. Copy `composer_midicv.uf2` to the mounted RP2350 boot drive. The module reboots as a composite **USB MIDI + USB CDC** device.

## Current configuration

The two panel switches select six modes: **midi1, midi2, midi3, sequencer1, sequencer2, sequencer3**. MIDI channels select voices within the selected mode. Default profiles are defined in `src/config/mode_config.c`; complete output routing is documented in the user manual.

| Function | GPIO |
| --- | --- |
| MCP4822 SCK / MOSI | 0 / 1 |
| MCP4822 U8 / U9 chip select | 2 / 3 |
| Gate 1–4 | 4 / 5 / 6 / 7 |
| TRS MIDI UART0 RX | 17 |
| Mode selector SW3 / SW4A / SW4B | 20 / 21 / 22 |
| RGB LED R / G / B, active-low | 25 / 24 / 23 |
| USB VBUS sense | 15 |

CV1/2 map to U8 VOUTA/B, and CV3/4 to U9 VOUTA/B. The firmware targets 0–8000 mV at the module outputs and converts calibrated millivolts to 12-bit DAC codes. The nominal analog full scale used in this conversion is 8190 mV.

USB CDC commands include `status`, `mute`, `test_gate`, `test_cv`, `cal show/reset/raw/set/save` and LED tests. Follow the manual's calibration sequence before saving new values.

## Current scope

Pitch bend is fixed at approximately ±2 semitones. Multi-voice modes use fixed MIDI channel groups rather than automatic chord allocation. Sequencer modes follow external MIDI Clock and transport; Song Position Pointer is not handled. Older bring-up programs and unassigned mode engines are not the six-mode v1.0 application.

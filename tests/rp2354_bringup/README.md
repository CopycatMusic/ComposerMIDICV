# RP2354 Bring-Up Firmware

This is a minimal Pico SDK hardware test program for the RP2354 4CV/4Gate
board.

It tests:

- RGB/status LED on GPIO0, GPIO1, GPIO2.
- Gate outputs on GPIO22, GPIO23, GPIO24, GPIO25.
- MCP4822 DAC outputs through bit-banged SPI:
  - SCK: GPIO26
  - MOSI/SDI: GPIO19
  - U5 CS: GPIO20
  - U6 CS: GPIO21

Expected outputs:

- J5 to J8 go high one at a time.
- J1 to J4 cycle through voltage steps.
- LED changes color to show the current stage.

Build:

```sh
cd "/Users/geng/Audio Projects/Composer MIDICV/tests/rp2354_bringup"

PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/toolchains/arm-gnu-toolchain-14.2.rel1-pkg/Payload/bin:$PATH" \
PICO_SDK_PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/pico-sdk" \
cmake -S . -B build-arm -G Ninja -DPICO_PLATFORM=rp2350

PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/toolchains/arm-gnu-toolchain-14.2.rel1-pkg/Payload/bin:$PATH" \
cmake --build build-arm
```

Program by USB BOOTSEL:

```sh
cp build-arm/rp2354_bringup.uf2 /Volumes/RP2350/
```

If your LED is not connected to GPIO0, GPIO1, GPIO2, change the `PIN_LED_*`
defines in `main.c`.

If the LED is common cathode instead of common anode, set `LED_ACTIVE_LOW` to
`0` in `main.c`.

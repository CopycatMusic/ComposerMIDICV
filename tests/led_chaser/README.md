# LED Chaser Test

Standalone RGB LED test for the Composer MIDICV RP2354 board.

It uses PWM on:

```text
R: GPIO0
G: GPIO1
B: GPIO2
```

The LED is assumed to be active-low. The firmware slowly cycles through a
continuous RGB color wheel. One full cycle takes about 30 seconds.

## Build

```sh
cd "/Users/geng/Audio Projects/Composer MIDICV/tests/led_chaser"

PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/toolchains/arm-gnu-toolchain-14.2.rel1-pkg/Payload/bin:$PATH" \
PICO_SDK_PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/pico-sdk" \
cmake -S . -B build-arm -G Ninja -DPICO_PLATFORM=rp2350-arm-s -DPICO_BOARD=pico2

PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/toolchains/arm-gnu-toolchain-14.2.rel1-pkg/Payload/bin:$PATH" \
PICO_SDK_PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/pico-sdk" \
cmake --build build-arm
```

## Flash

Hold `BOOTSEL` while plugging in USB, then copy:

```text
build-arm/led_chaser.uf2
```

to the mounted RP2350 drive.

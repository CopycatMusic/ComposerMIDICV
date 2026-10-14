# Mode Selector + USB Diagnostic

This is a standalone hardware diagnostic firmware for the current Composer
MIDICV board. It does not run the MIDI-CV mode engines.

## What It Tests

- USB device mount state.
- Mode selector raw GPIO state:
  - `SW3` on GPIO20
  - `SW4A` on GPIO21
  - `SW4B` on GPIO22
- Firmware decode of the six panel mode positions.

All selector inputs are configured as low-active GPIO inputs with internal
pull-ups, matching the main firmware.

## USB Serial Output

Open the USB CDC serial port after flashing. The firmware prints one line every
250 ms:

```text
usb=mounted raw=0x02 sw3=0 sw4a=1 sw4b=0 pos=1 mode=2
```

- `sw*=1` means that switch contact is active/closed to ground.
- `pos` is zero-based and should be `0..5`.
- `mode` is one-based and should be `1..6`.
- `pos=invalid` means `SW4A` and `SW4B` are both active at the same time.

Expected decode table:

| Mode | Position | SW3 | SW4A | SW4B |
|---:|---:|---:|---:|---:|
| 1 | 0 | 0 | 1 | 0 |
| 2 | 1 | 0 | 0 | 0 |
| 3 | 2 | 0 | 0 | 1 |
| 4 | 3 | 1 | 1 | 0 |
| 5 | 4 | 1 | 0 | 0 |
| 6 | 5 | 1 | 0 | 1 |

## LED And Gate Indicators

The RGB LED color indicates decoded position:

- Mode 1: red
- Mode 2: green
- Mode 3: blue
- Mode 4: yellow
- Mode 5: cyan
- Mode 6: magenta
- Invalid: white blink

Gate outputs show raw/debug state:

- Gate1: `SW3` active
- Gate2: `SW4A` active
- Gate3: `SW4B` active
- Gate4: decoded position valid

## Build

```sh
PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/toolchains/arm-gnu-toolchain-14.2.rel1-pkg/Payload/bin:$PATH" \
PICO_SDK_PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/pico-sdk" \
cmake -S tests/mode_selector_usb_diagnostic \
      -B tests/mode_selector_usb_diagnostic/build-arm-bundled \
      -G Ninja \
      -DPICO_PLATFORM=rp2350-arm-s \
      -DPICO_BOARD=pico2

PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/toolchains/arm-gnu-toolchain-14.2.rel1-pkg/Payload/bin:$PATH" \
PICO_SDK_PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/pico-sdk" \
cmake --build tests/mode_selector_usb_diagnostic/build-arm-bundled
```

The UF2 is copied to:

```text
tests/mode_selector_usb_diagnostic/dist/mode_selector_usb_diagnostic.uf2
```

# Composer MIDICV Firmware

First functional firmware for the RP2354A 4 CV + 4 Gate USB MIDI module.

## Build

From this directory:

```sh
PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/toolchains/arm-gnu-toolchain-14.2.rel1-pkg/Payload/bin:$PATH" \
PICO_SDK_PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/pico-sdk" \
cmake -S . -B build-arm -G Ninja -DPICO_PLATFORM=rp2350-arm-s -DPICO_BOARD=pico2

PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/toolchains/arm-gnu-toolchain-14.2.rel1-pkg/Payload/bin:$PATH" \
PICO_SDK_PATH="/Users/geng/Audio Projects/Composer MIDICV/third_party/pico-sdk" \
cmake --build build-arm
```

The generated UF2 is:

```text
build-arm/composer_midicv.uf2
```

## Flash

Hold `BOOTSEL` while plugging USB in, then copy the UF2 to the mounted RP2350 drive.

The firmware enumerates as a composite USB device:

- USB MIDI: receives MIDI input.
- USB CDC serial: debug and calibration shell.

## Hardware Mapping

```text
LED R/G/B: GPIO0/GPIO1/GPIO2, active-low PWM
MCP4822 SCK: GPIO26
MCP4822 SDI/MOSI: GPIO19
MCP4822 U5 CS: GPIO20
MCP4822 U6 CS: GPIO21
Gate 1-4: GPIO22/GPIO23/GPIO24/GPIO25
```

CV channel mapping:

```text
CV1 = U5 VOUTA
CV2 = U5 VOUTB
CV3 = U6 VOUTA
CV4 = U6 VOUTB
```

The current software range is `0..4095` DAC counts, treated as `0..4095 mV` at the MCP4822 output before any analog scaling after the DAC.

LED behavior:

```text
Idle/no note: low-brightness white
Note active: pitch-class color, brightness follows velocity
Multiple notes: latest active note chooses the color
Note release: fades back to the previous active note, or idle white if none
```

## MIDI Mode Selection

The first MIDI channel of an incoming channel message selects the mode:

```text
Channel 1: mono performance
Channel 2: mono random + audio square waves
Channel 3: dual mono
Channel 4: 4 voice polyphonic
Channel 5: 4 drum gates with velocity CV
Channel 6: 8 drum triggers
Channel 7: CC to CV/Gate
Channel 8-16: ignored
```

Implemented modes:

- Channel 1 mono performance:
  - CV1 pitch, 1 V/oct from MIDI note 36 as 0 V, with pitch bend.
  - CV2 note velocity.
  - CV3 CC1.
  - CV4 CC11.
  - Gate1 and Gate2 are held gates.
  - Gate3 and Gate4 are 10 ms note-on triggers.
- Channel 2 mono random + audio square waves:
  - CV1 pitch, with pitch bend.
  - CV2 note velocity.
  - CV3 and CV4 random sample-and-hold values on each note-on.
  - Gate1 is the held mono gate.
  - Gate2 is a 10 ms note-on trigger.
  - Gate3 is a 0-5 V audio-rate square wave at the active MIDI note.
  - Gate4 is a 0-5 V audio-rate square wave one octave above Gate3.
- Channel 3 dual mono:
  - The first two active notes are assigned to Voice A and Voice B.
  - Voice A: CV1 pitch, CV2 CC1 modulation, Gate1 held gate, Gate2 trigger.
  - Voice B: CV3 pitch, CV4 CC1 modulation, Gate3 held gate, Gate4 trigger.
  - This differs from the older hardware-switch spec that routed MIDI CH1/CH2,
    because this firmware uses MIDI CH1-7 for mode selection.
- Channel 4 polyphonic:
  - CV1-4 are per-voice pitch, with pitch bend applied to active voices.
  - Gate1-4 are per-voice gates.
- Channel 5 drum velocity:
  - MIDI notes C1/D1/E1/F#1, note numbers 24/26/28/30.
  - CV1-4 hold the latest velocity for each drum.
  - Gate1-4 emit 10 ms trigger pulses.
- Channel 6 eight drum triggers:
  - MIDI notes 24-31.
  - Notes 24-27 pulse CV1-4 as full-scale trigger outputs.
  - Notes 28-31 pulse Gate1-4.
- Channel 7 CC conversion:
  - CC1-4 map to CV1-4.
  - CC5-8 map to Gate1-4 with threshold `>= 64`.

## CDC Debug Shell

Open the CDC serial port with any terminal. Commands are line based:

```text
status
mute
test_gate <1-4>
test_cv <1-4> <mv>
cal show
cal reset
cal raw <cv> <code>
cal set <cv> <target_mv> <measured_mv>
cal save
```

Useful first tests before soldering jacks:

```text
test_gate 1
test_gate 2
test_gate 3
test_gate 4
test_cv 1 1000
test_cv 2 2000
test_cv 3 3000
test_cv 4 4095
```

`cal raw` bypasses calibration and writes the DAC code directly. This is the safest command for checking SPI/DAC behavior with a meter or oscilloscope:

```text
cal raw 1 0
cal raw 1 2048
cal raw 1 4095
```

## Current Limitations

- MIDI clock and transport messages are parsed but not musically used yet.
- Calibration is two-point linear correction per CV channel.
- The firmware has been compiled and inspected, but still needs hardware validation with DACs and output jacks installed.

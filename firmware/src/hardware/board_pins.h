#ifndef COMPOSER_MIDICV_HARDWARE_BOARD_PINS_H_
#define COMPOSER_MIDICV_HARDWARE_BOARD_PINS_H_

// ComposerMIDICV v0.3 KiCad: U3 common-anode RGB LED through R9/R8/R4.
#define PIN_LED_R 25
#define PIN_LED_G 24
#define PIN_LED_B 23
#define LED_ACTIVE_LOW 1

// GPIO0-7 drive U7 74AHCT244 inputs before reaching the DACs and gates.
#define PIN_DAC_SCK 0
#define PIN_DAC_MOSI 1
#define PIN_DAC_CS_U8 2
#define PIN_DAC_CS_U9 3

#define PIN_GATE_1 4
#define PIN_GATE_2 5
#define PIN_GATE_3 6
#define PIN_GATE_4 7

// SW3/SW4 are low-active mode selector inputs with firmware pull-ups.
#define PIN_MODE_SELECTOR_SW3 20
#define PIN_MODE_SELECTOR_SW4A 21
#define PIN_MODE_SELECTOR_SW4B 22

// TRS MIDI IN optocoupler output, routed to UART0 RX.
#define PIN_TRS_MIDI_RX 17

// Newer hardware routes divided/sensed USB VBUS to GPIO15.
#define PIN_USB_VBUS_SENSE 15

#endif  // COMPOSER_MIDICV_HARDWARE_BOARD_PINS_H_

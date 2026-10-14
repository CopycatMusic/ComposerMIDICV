#include "midi/trs_midi.h"

#include "hardware/board_pins.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "midi/midi_stream_parser.h"

#define TRS_MIDI_BAUD 31250u

static MidiStreamParser trs_parser;

void trs_midi_init(void) {
    midi_stream_parser_init(&trs_parser);
    uart_init(uart0, TRS_MIDI_BAUD);
    gpio_set_function(PIN_TRS_MIDI_RX, GPIO_FUNC_UART);
    uart_set_format(uart0, 8, 1, UART_PARITY_NONE);
    uart_set_hw_flow(uart0, false, false);
    uart_set_fifo_enabled(uart0, true);
}

bool trs_midi_read_event(bool usb_connected, MidiEvent* event) {
    while (uart_is_readable(uart0)) {
        const uint8_t byte = uart_getc(uart0);
        if (!midi_stream_parser_parse_byte(&trs_parser, byte, event)) {
            continue;
        }
        event->source = MIDI_SOURCE_TRS;
        event->usb_connected = usb_connected;
        return true;
    }
    return false;
}

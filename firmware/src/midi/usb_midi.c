#include "midi/usb_midi.h"

#include "midi/midi_parser.h"
#include "tusb.h"

bool usb_midi_read_event(MidiEvent* event) {
    if (!tud_midi_available()) {
        return false;
    }

    uint8_t packet[4];
    if (!tud_midi_packet_read(packet)) {
        return false;
    }

    return midi_parser_parse_usb_packet(packet, event);
}

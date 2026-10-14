#include "midi/midi_parser.h"

bool midi_parser_parse_usb_packet(const uint8_t packet[4], MidiEvent* event) {
    event->type = MIDI_EVENT_NONE;
    event->source = MIDI_SOURCE_UNKNOWN;
    event->channel = 0xff;
    event->data1 = 0;
    event->data2 = 0;
    event->pitch_bend = 0;
    event->realtime_byte = 0;
    event->usb_connected = false;

    const uint8_t cin = packet[0] & 0x0fu;
    const uint8_t status = packet[1];
    const uint8_t status_type = status & 0xf0u;

    if (status >= 0xf8u) {
        event->type = MIDI_EVENT_REALTIME;
        event->realtime_byte = status;
        return true;
    }

    if ((status & 0x80u) == 0) {
        event->type = MIDI_EVENT_UNKNOWN;
        return false;
    }

    event->channel = status & 0x0fu;
    event->data1 = packet[2];
    event->data2 = packet[3];

    switch (status_type) {
        case 0x80:
            event->type = MIDI_EVENT_NOTE_OFF;
            return true;

        case 0x90:
            if (event->data2 == 0) {
                event->type = MIDI_EVENT_NOTE_OFF;
            } else {
                event->type = MIDI_EVENT_NOTE_ON;
            }
            return true;

        case 0xb0:
            event->type = MIDI_EVENT_CONTROL_CHANGE;
            return true;

        case 0xc0:
            event->type = MIDI_EVENT_PROGRAM_CHANGE;
            return true;

        case 0xe0: {
            const uint16_t bend14 = ((uint16_t)packet[3] << 7) | packet[2];
            event->type = MIDI_EVENT_PITCH_BEND;
            event->pitch_bend = (int16_t)bend14 - 8192;
            return true;
        }

        default:
            event->type = MIDI_EVENT_UNKNOWN;
            return false;
    }
}

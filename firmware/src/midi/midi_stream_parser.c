#include "midi/midi_stream_parser.h"

static void clear_event(MidiEvent* event) {
    event->type = MIDI_EVENT_NONE;
    event->source = MIDI_SOURCE_UNKNOWN;
    event->channel = 0xffu;
    event->data1 = 0;
    event->data2 = 0;
    event->pitch_bend = 0;
    event->realtime_byte = 0;
    event->usb_connected = false;
}

static uint8_t channel_message_data_needed(uint8_t status) {
    switch (status & 0xf0u) {
        case 0x80u:
        case 0x90u:
        case 0xa0u:
        case 0xb0u:
        case 0xe0u:
            return 2;

        case 0xc0u:
        case 0xd0u:
            return 1;

        default:
            return 0;
    }
}

static bool emit_channel_message(uint8_t status,
                                 const uint8_t data[2],
                                 MidiEvent* event) {
    clear_event(event);
    event->channel = status & 0x0fu;
    event->data1 = data[0];
    event->data2 = data[1];

    switch (status & 0xf0u) {
        case 0x80u:
            event->type = MIDI_EVENT_NOTE_OFF;
            return true;

        case 0x90u:
            event->type = data[1] == 0 ? MIDI_EVENT_NOTE_OFF
                                       : MIDI_EVENT_NOTE_ON;
            return true;

        case 0xb0u:
            event->type = MIDI_EVENT_CONTROL_CHANGE;
            return true;

        case 0xc0u:
            event->type = MIDI_EVENT_PROGRAM_CHANGE;
            return true;

        case 0xe0u: {
            const uint16_t bend14 = ((uint16_t)data[1] << 7) | data[0];
            event->type = MIDI_EVENT_PITCH_BEND;
            event->pitch_bend = (int16_t)bend14 - 8192;
            return true;
        }

        default:
            event->type = MIDI_EVENT_UNKNOWN;
            return false;
    }
}

void midi_stream_parser_init(MidiStreamParser* parser) {
    parser->running_status = 0;
    parser->pending_status = 0;
    parser->data[0] = 0;
    parser->data[1] = 0;
    parser->data_count = 0;
    parser->data_needed = 0;
}

bool midi_stream_parser_parse_byte(MidiStreamParser* parser,
                                   uint8_t byte,
                                   MidiEvent* event) {
    clear_event(event);

    if (byte >= 0xf8u) {
        event->type = MIDI_EVENT_REALTIME;
        event->realtime_byte = byte;
        return true;
    }

    if (byte & 0x80u) {
        if (byte >= 0xf0u) {
            parser->running_status = 0;
            parser->pending_status = 0;
            parser->data_count = 0;
            parser->data_needed = 0;
            return false;
        }

        const uint8_t needed = channel_message_data_needed(byte);
        if (needed == 0) {
            parser->running_status = 0;
            parser->pending_status = 0;
            parser->data_count = 0;
            parser->data_needed = 0;
            return false;
        }

        parser->running_status = byte;
        parser->pending_status = byte;
        parser->data_count = 0;
        parser->data_needed = needed;
        return false;
    }

    if (parser->pending_status == 0) {
        if (parser->running_status == 0) {
            return false;
        }
        parser->pending_status = parser->running_status;
        parser->data_count = 0;
        parser->data_needed =
            channel_message_data_needed(parser->running_status);
    }

    if (parser->data_needed == 0 || parser->data_count >= 2) {
        return false;
    }

    parser->data[parser->data_count] = byte;
    ++parser->data_count;

    if (parser->data_count < parser->data_needed) {
        return false;
    }

    const uint8_t status = parser->pending_status;
    const bool emitted = emit_channel_message(status, parser->data, event);
    parser->pending_status = parser->running_status;
    parser->data_count = 0;
    parser->data[0] = 0;
    parser->data[1] = 0;
    parser->data_needed = channel_message_data_needed(parser->pending_status);
    return emitted;
}

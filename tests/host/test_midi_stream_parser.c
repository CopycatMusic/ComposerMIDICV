#include <assert.h>
#include <stdint.h>

#include "midi/midi_event.h"
#include "midi/midi_stream_parser.h"

static void test_note_on_and_note_off(void) {
    MidiStreamParser parser;
    MidiEvent event;
    midi_stream_parser_init(&parser);

    assert(!midi_stream_parser_parse_byte(&parser, 0x90, &event));
    assert(!midi_stream_parser_parse_byte(&parser, 60, &event));
    assert(midi_stream_parser_parse_byte(&parser, 100, &event));
    assert(event.type == MIDI_EVENT_NOTE_ON);
    assert(event.channel == 0);
    assert(event.data1 == 60);
    assert(event.data2 == 100);

    assert(!midi_stream_parser_parse_byte(&parser, 0x80, &event));
    assert(!midi_stream_parser_parse_byte(&parser, 60, &event));
    assert(midi_stream_parser_parse_byte(&parser, 0, &event));
    assert(event.type == MIDI_EVENT_NOTE_OFF);
    assert(event.data1 == 60);
}

static void test_running_status(void) {
    MidiStreamParser parser;
    MidiEvent event;
    midi_stream_parser_init(&parser);

    assert(!midi_stream_parser_parse_byte(&parser, 0x90, &event));
    assert(!midi_stream_parser_parse_byte(&parser, 60, &event));
    assert(midi_stream_parser_parse_byte(&parser, 100, &event));
    assert(event.type == MIDI_EVENT_NOTE_ON);
    assert(event.data1 == 60);

    assert(!midi_stream_parser_parse_byte(&parser, 64, &event));
    assert(midi_stream_parser_parse_byte(&parser, 0, &event));
    assert(event.type == MIDI_EVENT_NOTE_OFF);
    assert(event.data1 == 64);
}

static void test_realtime_does_not_clear_running_status(void) {
    MidiStreamParser parser;
    MidiEvent event;
    midi_stream_parser_init(&parser);

    assert(!midi_stream_parser_parse_byte(&parser, 0x90, &event));
    assert(!midi_stream_parser_parse_byte(&parser, 60, &event));
    assert(midi_stream_parser_parse_byte(&parser, 100, &event));

    assert(midi_stream_parser_parse_byte(&parser, 0xf8, &event));
    assert(event.type == MIDI_EVENT_REALTIME);
    assert(event.realtime_byte == 0xf8);

    assert(!midi_stream_parser_parse_byte(&parser, 67, &event));
    assert(midi_stream_parser_parse_byte(&parser, 100, &event));
    assert(event.type == MIDI_EVENT_NOTE_ON);
    assert(event.data1 == 67);
}

static void test_pitch_bend(void) {
    MidiStreamParser parser;
    MidiEvent event;
    midi_stream_parser_init(&parser);

    assert(!midi_stream_parser_parse_byte(&parser, 0xe0, &event));
    assert(!midi_stream_parser_parse_byte(&parser, 0x00, &event));
    assert(midi_stream_parser_parse_byte(&parser, 0x40, &event));
    assert(event.type == MIDI_EVENT_PITCH_BEND);
    assert(event.pitch_bend == 0);
}

int main(void) {
    test_note_on_and_note_off();
    test_running_status();
    test_realtime_does_not_clear_running_status();
    test_pitch_bend();
    return 0;
}

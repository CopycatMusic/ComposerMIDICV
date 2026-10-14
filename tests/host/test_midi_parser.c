#include <assert.h>
#include <stdint.h>

#include "midi/midi_event.h"
#include "midi/midi_parser.h"

static void test_realtime_cin_0f(void) {
    MidiEvent event;
    const uint8_t packet[4] = {0x0f, 0xfa, 0x00, 0x00};

    assert(midi_parser_parse_usb_packet(packet, &event));
    assert(event.type == MIDI_EVENT_REALTIME);
    assert(event.realtime_byte == 0xfa);
    assert(event.channel == 0xff);
}

static void test_realtime_status_wins_over_cin(void) {
    MidiEvent event;
    const uint8_t packet[4] = {0x05, 0xfa, 0x00, 0x00};

    assert(midi_parser_parse_usb_packet(packet, &event));
    assert(event.type == MIDI_EVENT_REALTIME);
    assert(event.realtime_byte == 0xfa);
    assert(event.channel == 0xff);
}

static void test_note_on_packet(void) {
    MidiEvent event;
    const uint8_t packet[4] = {0x09, 0x91, 0x3c, 0x64};

    assert(midi_parser_parse_usb_packet(packet, &event));
    assert(event.type == MIDI_EVENT_NOTE_ON);
    assert(event.channel == 1);
    assert(event.data1 == 60);
    assert(event.data2 == 100);
}

static void test_note_on_zero_velocity_is_note_off(void) {
    MidiEvent event;
    const uint8_t packet[4] = {0x09, 0x90, 0x3c, 0x00};

    assert(midi_parser_parse_usb_packet(packet, &event));
    assert(event.type == MIDI_EVENT_NOTE_OFF);
    assert(event.channel == 0);
    assert(event.data1 == 60);
}

static void test_control_change_packet(void) {
    MidiEvent event;
    const uint8_t packet[4] = {0x0b, 0xb2, 0x01, 0x7f};

    assert(midi_parser_parse_usb_packet(packet, &event));
    assert(event.type == MIDI_EVENT_CONTROL_CHANGE);
    assert(event.channel == 2);
    assert(event.data1 == 1);
    assert(event.data2 == 127);
}

static void test_pitch_bend_packet(void) {
    MidiEvent event;
    const uint8_t packet[4] = {0x0e, 0xe0, 0x00, 0x40};

    assert(midi_parser_parse_usb_packet(packet, &event));
    assert(event.type == MIDI_EVENT_PITCH_BEND);
    assert(event.channel == 0);
    assert(event.pitch_bend == 0);
}

static void test_non_realtime_single_byte_is_ignored(void) {
    MidiEvent event;
    const uint8_t packet[4] = {0x0f, 0xf6, 0x00, 0x00};

    assert(!midi_parser_parse_usb_packet(packet, &event));
}

int main(void) {
    test_realtime_cin_0f();
    test_realtime_status_wins_over_cin();
    test_note_on_packet();
    test_note_on_zero_velocity_is_note_off();
    test_control_change_packet();
    test_pitch_bend_packet();
    test_non_realtime_single_byte_is_ignored();
    return 0;
}

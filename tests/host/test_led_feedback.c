#include <assert.h>
#include <stdint.h>

#include "midi/midi_event.h"
#include "ui/led_feedback.h"

static MidiEvent note_on(uint8_t note, uint8_t velocity) {
    MidiEvent event = {
        .type = MIDI_EVENT_NOTE_ON,
        .channel = 0,
        .data1 = note,
        .data2 = velocity,
    };
    return event;
}

static MidiEvent note_off(uint8_t note) {
    MidiEvent event = {
        .type = MIDI_EVENT_NOTE_OFF,
        .channel = 0,
        .data1 = note,
    };
    return event;
}

static void tick_ms(LedFeedbackState* state, uint16_t count) {
    for (uint16_t i = 0; i < count; ++i) {
        led_feedback_tick_1ms(state);
    }
}

int main(void) {
    LedFeedbackState state;
    led_feedback_init(&state);

    LedRgb rgb = led_feedback_rgb(&state);
    assert(rgb.r == 77);
    assert(rgb.g == 77);
    assert(rgb.b == 77);

    MidiEvent event = note_on(60, 127);
    led_feedback_handle_midi(&state, &event);
    rgb = led_feedback_rgb(&state);
    assert(rgb.r == 255);
    assert(rgb.g == 0);
    assert(rgb.b == 0);

    event = note_on(64, 64);
    led_feedback_handle_midi(&state, &event);
    rgb = led_feedback_rgb(&state);
    assert(rgb.r > 32);
    assert(rgb.g > 80);
    assert(rgb.b == 0);
    assert(rgb.r < 255);
    assert(rgb.g < 255);

    event = note_off(64);
    led_feedback_handle_midi(&state, &event);
    rgb = led_feedback_rgb(&state);
    assert(rgb.r == 255);
    assert(rgb.g == 0);
    assert(rgb.b == 0);

    event = note_off(60);
    led_feedback_handle_midi(&state, &event);
    tick_ms(&state, 300);
    rgb = led_feedback_rgb(&state);
    assert(rgb.r == 77);
    assert(rgb.g == 77);
    assert(rgb.b == 77);

    return 0;
}

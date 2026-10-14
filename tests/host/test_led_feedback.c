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

static MidiEvent realtime(uint8_t byte) {
    MidiEvent event = {
        .type = MIDI_EVENT_REALTIME,
        .channel = 0xff,
        .realtime_byte = byte,
    };
    return event;
}

static void assert_rgb(LedRgb rgb, uint8_t r, uint8_t g, uint8_t b) {
    assert(rgb.r == r);
    assert(rgb.g == g);
    assert(rgb.b == b);
}

int main(void) {
    LedFeedbackState state;
    led_feedback_init(&state);

    LedRgb rgb = led_feedback_rgb(&state);
    assert_rgb(rgb, 0, 0, 0);

    led_feedback_set_mode(&state, MODE_ENGINE_MONO_MIDI);
    MidiEvent event = note_on(60, 127);
    led_feedback_handle_midi(&state, &event);
    rgb = led_feedback_rgb(&state);
    assert_rgb(rgb, 239, 59, 92);

    event = note_off(60);
    led_feedback_handle_midi(&state, &event);
    assert_rgb(led_feedback_rgb(&state), 0, 0, 0);

    event = note_on(35, 1);
    led_feedback_handle_midi(&state, &event);
    rgb = led_feedback_rgb(&state);
    assert_rgb(rgb, 239, 59, 92);

    for (uint16_t i = 0; i < 120; ++i) {
        led_feedback_tick_1ms(&state);
    }
    assert_rgb(led_feedback_rgb(&state), 0, 0, 0);
    for (uint16_t i = 0; i < 120; ++i) {
        led_feedback_tick_1ms(&state);
    }
    assert_rgb(led_feedback_rgb(&state), 239, 59, 92);
    for (uint16_t i = 0; i < 120; ++i) {
        led_feedback_tick_1ms(&state);
    }
    assert_rgb(led_feedback_rgb(&state), 0, 0, 0);
    event = note_off(35);
    led_feedback_handle_midi(&state, &event);
    assert_rgb(led_feedback_rgb(&state), 0, 0, 0);

    led_feedback_set_mode(&state, MODE_ENGINE_DUAL_MIDI);
    event = note_on(60, 1);
    event.channel = 0;
    led_feedback_handle_midi(&state, &event);
    assert_rgb(led_feedback_rgb(&state), 239, 59, 92);
    event = note_on(60, 127);
    event.channel = 8;
    led_feedback_handle_midi(&state, &event);
    assert_rgb(led_feedback_rgb(&state), 228, 255, 166);
    event = note_off(60);
    event.channel = 8;
    led_feedback_handle_midi(&state, &event);
    assert_rgb(led_feedback_rgb(&state), 239, 59, 92);

    event = note_off(60);
    event.channel = 0;
    led_feedback_handle_midi(&state, &event);
    assert_rgb(led_feedback_rgb(&state), 0, 0, 0);

    event = note_on(35, 127);
    event.channel = 8;
    led_feedback_handle_midi(&state, &event);
    assert_rgb(led_feedback_rgb(&state), 6, 214, 86);
    for (uint16_t i = 0; i < 120; ++i) {
        led_feedback_tick_1ms(&state);
    }
    assert_rgb(led_feedback_rgb(&state), 0, 0, 0);
    for (uint16_t i = 0; i < 120; ++i) {
        led_feedback_tick_1ms(&state);
    }
    assert_rgb(led_feedback_rgb(&state), 6, 214, 86);
    event = note_on(60, 127);
    event.channel = 8;
    led_feedback_handle_midi(&state, &event);
    assert_rgb(led_feedback_rgb(&state), 6, 214, 86);
    event = note_off(60);
    event.channel = 8;
    led_feedback_handle_midi(&state, &event);
    assert_rgb(led_feedback_rgb(&state), 0, 0, 0);

    led_feedback_set_mode(&state, MODE_ENGINE_POLY4_MIDI);
    for (uint8_t voice = 0; voice < 4; ++voice) {
        event = note_on((uint8_t)(60 + voice), 1);
        event.channel = (uint8_t)(voice * 4u);
        led_feedback_handle_midi(&state, &event);
    }
    assert_rgb(led_feedback_rgb(&state), 246, 255, 209);

    led_feedback_set_mode(&state, MODE_ENGINE_CLOCKED_NOTE);
    event = realtime(0xfau);
    led_feedback_handle_midi(&state, &event);
    assert_rgb(led_feedback_rgb(&state), 0, 0, 255);
    for (uint16_t i = 0; i < 239; ++i) {
        led_feedback_tick_1ms(&state);
    }
    assert_rgb(led_feedback_rgb(&state), 0, 0, 255);
    led_feedback_tick_1ms(&state);
    assert_rgb(led_feedback_rgb(&state), 0, 0, 0);

    return 0;
}

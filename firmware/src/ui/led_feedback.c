#include "ui/led_feedback.h"

enum {
    LED_IDLE_LEVEL = 77,
    LED_MIN_NOTE_LEVEL = 24,
    LED_FADE_STEP = 2,
};

static const LedRgb pitch_class_color[12] = {
    {255, 0, 0},    // C
    {255, 48, 0},   // C#
    {255, 128, 0},  // D
    {255, 220, 0},  // D#
    {150, 255, 0},  // E
    {0, 255, 0},    // F
    {0, 255, 128},  // F#
    {0, 220, 255},  // G
    {0, 64, 255},   // G#
    {96, 0, 255},   // A
    {180, 0, 255},  // A#
    {255, 0, 180},  // B
};

static LedRgb idle_rgb(void) {
    LedRgb rgb = {LED_IDLE_LEVEL, LED_IDLE_LEVEL, LED_IDLE_LEVEL};
    return rgb;
}

static uint8_t scale_channel(uint8_t channel, uint8_t velocity) {
    if (channel == 0 || velocity == 0) {
        return 0;
    }

    const uint16_t level =
        LED_MIN_NOTE_LEVEL +
        ((uint16_t)velocity * (255u - LED_MIN_NOTE_LEVEL)) / 127u;
    return (uint8_t)(((uint16_t)channel * level) / 255u);
}

static LedRgb note_rgb(uint8_t note, uint8_t velocity) {
    const LedRgb base = pitch_class_color[note % 12u];
    LedRgb rgb = {
        scale_channel(base.r, velocity),
        scale_channel(base.g, velocity),
        scale_channel(base.b, velocity),
    };
    return rgb;
}

static uint8_t move_channel(uint8_t current, uint8_t target) {
    if (current < target) {
        const uint16_t next = (uint16_t)current + LED_FADE_STEP;
        return next > target ? target : (uint8_t)next;
    }
    if (current > target) {
        return current - target > LED_FADE_STEP
                   ? (uint8_t)(current - LED_FADE_STEP)
                   : target;
    }
    return current;
}

static void update_from_active_note(LedFeedbackState* state) {
    uint8_t note = 0;
    uint8_t velocity = 0;
    if (note_stack_active(&state->notes, &note, &velocity)) {
        state->target = note_rgb(note, velocity);
        state->current = state->target;
    } else {
        state->target = idle_rgb();
    }
}

void led_feedback_init(LedFeedbackState* state) {
    note_stack_init(&state->notes);
    state->current = idle_rgb();
    state->target = state->current;
}

void led_feedback_handle_midi(LedFeedbackState* state, const MidiEvent* event) {
    switch (event->type) {
        case MIDI_EVENT_NOTE_ON:
            note_stack_note_on(&state->notes, event->data1, event->data2);
            update_from_active_note(state);
            break;

        case MIDI_EVENT_NOTE_OFF:
            note_stack_note_off(&state->notes, event->data1);
            update_from_active_note(state);
            break;

        default:
            break;
    }
}

void led_feedback_tick_1ms(LedFeedbackState* state) {
    state->current.r = move_channel(state->current.r, state->target.r);
    state->current.g = move_channel(state->current.g, state->target.g);
    state->current.b = move_channel(state->current.b, state->target.b);
}

LedRgb led_feedback_rgb(const LedFeedbackState* state) {
    return state->current;
}

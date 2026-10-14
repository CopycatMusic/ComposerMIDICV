#include "ui/led_feedback.h"

#include "music/pitch_cv.h"

enum {
    LED_TRANSPORT_FLASH_MS = 240,
    LED_RANGE_FLASH_SEGMENT_MS = 120,
    LED_FEEDBACK_NO_VOICE = 0xffu,
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

static const LedRgb poly_voice_color[4] = {
    {239, 59, 92},
    {6, 214, 86},
    {20, 48, 204},
    {255, 217, 59},
};

static const LedRgb dual_voice_color[2] = {
    {239, 59, 92},
    {6, 214, 86},
};

static LedRgb off_rgb(void) {
    LedRgb rgb = {0, 0, 0};
    return rgb;
}

static LedRgb note_rgb(uint8_t note) {
    return pitch_class_color[note % 12u];
}

static bool voice_active(const LedFeedbackState* state, uint8_t voice) {
    uint8_t note;
    uint8_t velocity;
    return voice < 4u &&
           note_stack_active(&state->notes[voice], &note, &velocity);
}

static LedRgb pitch_class_rgb(const LedFeedbackState* state) {
    uint8_t note = 0;
    uint8_t velocity = 0;
    if (note_stack_active(&state->notes[0], &note, &velocity)) {
        return note_rgb(note);
    }
    return off_rgb();
}

static LedRgb normalize_sum(uint16_t r, uint16_t g, uint16_t b) {
    uint16_t peak = r;
    if (g > peak) {
        peak = g;
    }
    if (b > peak) {
        peak = b;
    }
    if (peak <= 255u) {
        LedRgb rgb = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
        return rgb;
    }

    LedRgb rgb = {
        (uint8_t)((r * 255u) / peak),
        (uint8_t)((g * 255u) / peak),
        (uint8_t)((b * 255u) / peak),
    };
    return rgb;
}

static LedRgb mixed_voice_rgb(const LedFeedbackState* state,
                              const LedRgb* colors,
                              uint8_t voice_count) {
    uint16_t r = 0;
    uint16_t g = 0;
    uint16_t b = 0;
    for (uint8_t voice = 0; voice < voice_count; ++voice) {
        if (!voice_active(state, voice)) {
            continue;
        }
        r += colors[voice].r;
        g += colors[voice].g;
        b += colors[voice].b;
    }
    return normalize_sum(r, g, b);
}

static void update_current_color(LedFeedbackState* state) {
    switch (state->engine) {
        case MODE_ENGINE_DUAL_MIDI:
        case MODE_ENGINE_CLOCKED_NOTE:
            state->current = mixed_voice_rgb(state, dual_voice_color, 2);
            break;

        case MODE_ENGINE_POLY4_MIDI:
            state->current = mixed_voice_rgb(state, poly_voice_color, 4);
            break;

        case MODE_ENGINE_MONO_MIDI:
            state->current = mixed_voice_rgb(state, poly_voice_color, 1);
            break;

        default:
            state->current = pitch_class_rgb(state);
            break;
    }
}

static LedRgb range_error_rgb(const LedFeedbackState* state) {
    uint16_t r = 0;
    uint16_t g = 0;
    uint16_t b = 0;
    uint8_t voice_count = 1;
    const LedRgb* colors = poly_voice_color;

    if (state->engine == MODE_ENGINE_DUAL_MIDI ||
        state->engine == MODE_ENGINE_CLOCKED_NOTE) {
        voice_count = 2;
        colors = dual_voice_color;
    } else if (state->engine == MODE_ENGINE_POLY4_MIDI) {
        voice_count = 4;
    }

    for (uint8_t voice = 0; voice < voice_count; ++voice) {
        if (!state->range_error[voice]) {
            continue;
        }
        r += colors[voice].r;
        g += colors[voice].g;
        b += colors[voice].b;
    }
    return normalize_sum(r, g, b);
}

static bool range_error_active(const LedFeedbackState* state) {
    for (uint8_t voice = 0; voice < 4u; ++voice) {
        if (state->range_error[voice]) {
            return true;
        }
    }
    return false;
}

static uint8_t voice_for_event(const LedFeedbackState* state,
                               const MidiEvent* event) {
    if (state->engine == MODE_ENGINE_POLY4_MIDI) {
        return event->channel / 4u;
    }
    if (state->engine == MODE_ENGINE_DUAL_MIDI ||
        state->engine == MODE_ENGINE_CLOCKED_NOTE) {
        return event->channel < 8u ? 0u : 1u;
    }
    return 0;
}

static void select_fallback_voice(LedFeedbackState* state, uint8_t voice_count) {
    state->selected_voice = LED_FEEDBACK_NO_VOICE;
    for (uint8_t voice = 0; voice < voice_count; ++voice) {
        if (voice_active(state, voice)) {
            state->selected_voice = voice;
            return;
        }
    }
}

void led_feedback_init(LedFeedbackState* state) {
    led_feedback_set_mode(state, MODE_ENGINE_RESERVED);
}

void led_feedback_set_mode(LedFeedbackState* state, ModeEngineType engine) {
    for (uint8_t voice = 0; voice < 4u; ++voice) {
        note_stack_init(&state->notes[voice]);
    }
    state->engine = engine;
    state->selected_voice = LED_FEEDBACK_NO_VOICE;
    state->transport_flash_remaining_ms = 0;
    state->range_flash_phase_ms = 0;
    for (uint8_t voice = 0; voice < 4u; ++voice) {
        state->range_error[voice] = false;
    }
    state->current = off_rgb();
}

void led_feedback_handle_midi(LedFeedbackState* state, const MidiEvent* event) {
    switch (event->type) {
        case MIDI_EVENT_NOTE_ON: {
            const uint8_t voice = voice_for_event(state, event);
            if (voice < 4u) {
                if (!pitch_cv_note_is_in_output_range(event->data1)) {
                    state->range_error[voice] = true;
                    state->range_flash_phase_ms = 0;
                    update_current_color(state);
                    break;
                }
                state->range_error[voice] = false;
                note_stack_note_on(&state->notes[voice], event->data1,
                                   event->data2);
                state->selected_voice = voice;
            }
            update_current_color(state);
            break;
        }

        case MIDI_EVENT_NOTE_OFF: {
            const uint8_t voice = voice_for_event(state, event);
            if (voice < 4u) {
                state->range_error[voice] = false;
                note_stack_note_off(&state->notes[voice], event->data1);
                if (state->selected_voice == voice && !voice_active(state, voice)) {
                    select_fallback_voice(state, 4u);
                }
            }
            update_current_color(state);
            break;
        }

        case MIDI_EVENT_REALTIME:
            if (state->engine == MODE_ENGINE_CLOCKED_NOTE &&
                (event->realtime_byte == 0xfau ||
                 event->realtime_byte == 0xfbu ||
                 event->realtime_byte == 0xfcu)) {
                state->transport_flash_remaining_ms = LED_TRANSPORT_FLASH_MS;
            }
            break;

        default:
            break;
    }
}

void led_feedback_tick_1ms(LedFeedbackState* state) {
    if (state->transport_flash_remaining_ms > 0) {
        --state->transport_flash_remaining_ms;
    }
    if (range_error_active(state)) {
        ++state->range_flash_phase_ms;
        if (state->range_flash_phase_ms >= LED_RANGE_FLASH_SEGMENT_MS * 2u) {
            state->range_flash_phase_ms = 0;
        }
    } else {
        state->range_flash_phase_ms = 0;
    }
}

LedRgb led_feedback_rgb(const LedFeedbackState* state) {
    if (range_error_active(state)) {
        return state->range_flash_phase_ms < LED_RANGE_FLASH_SEGMENT_MS
                   ? range_error_rgb(state)
                   : off_rgb();
    }
    if (state->transport_flash_remaining_ms > 0) {
        const LedRgb blue = {0, 0, 255};
        return blue;
    }
    return state->current;
}

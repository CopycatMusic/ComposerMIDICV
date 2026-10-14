#include "modes/mode_mono_perf.h"

#include "board_config.h"
#include "music/pitch_cv.h"

static uint16_t random_cv(ModeMonoPerfState* state) {
    uint32_t x = state->random_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state->random_state = x == 0 ? 0x1f123bb5u : x;
    return (uint16_t)(state->random_state %
                      (COMPOSER_MIDICV_CV_MAX_MV + 1u));
}

static void apply_active_note(ModeMonoPerfState* state, OutputState* output) {
    uint8_t note = 0;
    uint8_t velocity = 0;
    if (note_stack_active(&state->notes, &note, &velocity)) {
        output->cv_mv[0] = pitch_cv_note_to_mv_bend(note, state->pitch_bend);
        output->cv_mv[1] = pitch_cv_7bit_to_mv(velocity);
        output->gate[0] = true;
        output->gate[1] = true;
    } else {
        output->gate[0] = false;
        output->gate[1] = false;
    }
    output->cv_mv[2] = pitch_cv_7bit_to_mv(state->cc1);
}

static void reset(void* mode_state, OutputState* output) {
    ModeMonoPerfState* state = (ModeMonoPerfState*)mode_state;
    note_stack_init(&state->notes);
    state->cc1 = 0;
    state->random_state = 0x31415927u;
    state->pitch_bend = 0;
    for (uint8_t i = 0; i < 4; ++i) {
        state->trigger_ms[i] = 0;
    }
    output_state_clear(output);
}

static void handle_midi(void* mode_state,
                        const MidiEvent* event,
                        OutputState* output) {
    ModeMonoPerfState* state = (ModeMonoPerfState*)mode_state;

    switch (event->type) {
        case MIDI_EVENT_NOTE_ON:
            note_stack_note_on(&state->notes, event->data1, event->data2);
            output->cv_mv[3] = random_cv(state);
            state->trigger_ms[2] = COMPOSER_MIDICV_TRIGGER_MS;
            state->trigger_ms[3] = COMPOSER_MIDICV_TRIGGER_MS;
            output->gate[2] = true;
            output->gate[3] = true;
            apply_active_note(state, output);
            break;

        case MIDI_EVENT_NOTE_OFF:
            note_stack_note_off(&state->notes, event->data1);
            apply_active_note(state, output);
            break;

        case MIDI_EVENT_CONTROL_CHANGE:
            if (event->data1 == 1) {
                state->cc1 = event->data2;
            }
            apply_active_note(state, output);
            break;

        case MIDI_EVENT_PITCH_BEND:
            state->pitch_bend = event->pitch_bend;
            apply_active_note(state, output);
            break;

        default:
            break;
    }
}

static void tick_1ms(void* mode_state, OutputState* output) {
    ModeMonoPerfState* state = (ModeMonoPerfState*)mode_state;
    for (uint8_t i = 2; i < 4; ++i) {
        if (state->trigger_ms[i] == 0) {
            continue;
        }
        --state->trigger_ms[i];
        if (state->trigger_ms[i] == 0) {
            output->gate[i] = false;
        }
    }
}

Mode mode_mono_perf_create(ModeMonoPerfState* state) {
    Mode mode = {
        .name = "midi1",
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

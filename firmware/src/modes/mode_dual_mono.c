#include "modes/mode_dual_mono.h"

#include "board_config.h"
#include "music/pitch_cv.h"

static void apply_voice(ModeDualMonoState* state,
                        OutputState* output,
                        uint8_t voice) {
    if (voice == 0) {
        output->cv_mv[0] = state->active[0]
                               ? pitch_cv_note_to_mv_bend(state->note[0],
                                                          state->pitch_bend)
                               : output->cv_mv[0];
        output->cv_mv[1] = pitch_cv_7bit_to_mv(state->cc1);
        output->gate[0] = state->active[0];
    } else {
        output->cv_mv[2] = state->active[1]
                               ? pitch_cv_note_to_mv_bend(state->note[1],
                                                          state->pitch_bend)
                               : output->cv_mv[2];
        output->cv_mv[3] = pitch_cv_7bit_to_mv(state->cc1);
        output->gate[2] = state->active[1];
    }
}

static void apply_all_voices(ModeDualMonoState* state, OutputState* output) {
    apply_voice(state, output, 0);
    apply_voice(state, output, 1);
}

static uint8_t allocate_voice(ModeDualMonoState* state, uint8_t note) {
    for (uint8_t i = 0; i < 2; ++i) {
        if (state->active[i] && state->note[i] == note) {
            return i;
        }
    }
    for (uint8_t i = 0; i < 2; ++i) {
        if (!state->active[i]) {
            return i;
        }
    }
    return 0;
}

static void reset(void* mode_state, OutputState* output) {
    ModeDualMonoState* state = (ModeDualMonoState*)mode_state;
    for (uint8_t i = 0; i < 2; ++i) {
        state->active[i] = false;
        state->note[i] = 0xff;
        state->trigger_ms[i] = 0;
    }
    state->cc1 = 0;
    state->pitch_bend = 0;
    output_state_clear(output);
}

static void handle_midi(void* mode_state,
                        const MidiEvent* event,
                        OutputState* output) {
    ModeDualMonoState* state = (ModeDualMonoState*)mode_state;

    switch (event->type) {
        case MIDI_EVENT_NOTE_ON: {
            const uint8_t voice = allocate_voice(state, event->data1);
            state->active[voice] = true;
            state->note[voice] = event->data1;
            state->trigger_ms[voice] = COMPOSER_MIDICV_TRIGGER_MS;
            output->gate[voice == 0 ? 1 : 3] = true;
            apply_voice(state, output, voice);
            break;
        }

        case MIDI_EVENT_NOTE_OFF:
            for (uint8_t i = 0; i < 2; ++i) {
                if (state->active[i] && state->note[i] == event->data1) {
                    state->active[i] = false;
                    state->note[i] = 0xff;
                    apply_voice(state, output, i);
                }
            }
            break;

        case MIDI_EVENT_CONTROL_CHANGE:
            if (event->data1 == 1) {
                state->cc1 = event->data2;
                output->cv_mv[1] = pitch_cv_7bit_to_mv(state->cc1);
                output->cv_mv[3] = pitch_cv_7bit_to_mv(state->cc1);
            }
            break;

        case MIDI_EVENT_PITCH_BEND:
            state->pitch_bend = event->pitch_bend;
            apply_all_voices(state, output);
            break;

        default:
            break;
    }
}

static void tick_1ms(void* mode_state, OutputState* output) {
    ModeDualMonoState* state = (ModeDualMonoState*)mode_state;
    for (uint8_t i = 0; i < 2; ++i) {
        if (state->trigger_ms[i] == 0) {
            continue;
        }
        --state->trigger_ms[i];
        if (state->trigger_ms[i] == 0) {
            output->gate[i == 0 ? 1 : 3] = false;
        }
    }
}

Mode mode_dual_mono_create(ModeDualMonoState* state) {
    Mode mode = {
        .name = "dual_mono",
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

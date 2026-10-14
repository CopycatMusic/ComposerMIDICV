#include "modes/mode_dual_mono.h"

#include "board_config.h"
#include "music/pitch_cv.h"

static void apply_voice(ModeDualMonoState* state,
                        OutputState* output,
                        uint8_t voice) {
    if (voice == 0) {
        output->cv_mv[0] = state->active[0]
                               ? pitch_cv_note_to_mv_bend(state->note[0],
                                                          state->pitch_bend[0])
                               : output->cv_mv[0];
        output->cv_mv[1] = pitch_cv_7bit_to_mv(state->velocity[0]);
        output->gate[0] = state->active[0];
    } else {
        output->cv_mv[2] = state->active[1]
                               ? pitch_cv_note_to_mv_bend(state->note[1],
                                                          state->pitch_bend[1])
                               : output->cv_mv[2];
        output->cv_mv[3] = pitch_cv_7bit_to_mv(state->velocity[1]);
        output->gate[2] = state->active[1];
    }
}

static uint8_t voice_for_channel(uint8_t channel) {
    return channel < 8u ? 0u : 1u;
}

static void reset(void* mode_state, OutputState* output) {
    ModeDualMonoState* state = (ModeDualMonoState*)mode_state;
    for (uint8_t i = 0; i < 2; ++i) {
        state->active[i] = false;
        state->note[i] = 0xff;
        state->velocity[i] = 0;
        state->pitch_bend[i] = 0;
        state->trigger_ms[i] = 0;
    }
    output_state_clear(output);
}

static void handle_midi(void* mode_state,
                        const MidiEvent* event,
                        OutputState* output) {
    ModeDualMonoState* state = (ModeDualMonoState*)mode_state;

    switch (event->type) {
        case MIDI_EVENT_NOTE_ON: {
            const uint8_t voice = voice_for_channel(event->channel);
            state->active[voice] = true;
            state->note[voice] = event->data1;
            state->velocity[voice] = event->data2;
            state->trigger_ms[voice] = COMPOSER_MIDICV_TRIGGER_MS;
            output->gate[voice == 0 ? 1 : 3] = true;
            apply_voice(state, output, voice);
            break;
        }

        case MIDI_EVENT_NOTE_OFF:
            {
                const uint8_t voice = voice_for_channel(event->channel);
                if (state->active[voice] &&
                    state->note[voice] == event->data1) {
                    state->active[voice] = false;
                    state->note[voice] = 0xff;
                    state->velocity[voice] = 0;
                    apply_voice(state, output, voice);
                }
            }
            break;

        case MIDI_EVENT_PITCH_BEND:
            {
                const uint8_t voice = voice_for_channel(event->channel);
                state->pitch_bend[voice] = event->pitch_bend;
                apply_voice(state, output, voice);
            }
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
        .name = "midi2",
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

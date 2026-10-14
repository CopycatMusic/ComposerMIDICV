#include "modes/mode_poly4.h"

#include "music/pitch_cv.h"

static uint8_t voice_for_channel(uint8_t channel) {
    return (uint8_t)(channel / 4u);
}

static void apply_voice(ModePoly4State* state,
                        OutputState* output,
                        uint8_t voice) {
    if (voice >= 4u || !state->active[voice]) {
        return;
    }

    output->cv_mv[voice] =
        pitch_cv_note_to_mv_bend(state->note[voice],
                                 state->pitch_bend[voice]);
}

static void reset(void* mode_state, OutputState* output) {
    ModePoly4State* state = (ModePoly4State*)mode_state;
    for (uint8_t i = 0; i < 4; ++i) {
        state->active[i] = false;
        state->note[i] = 0xff;
        state->pitch_bend[i] = 0;
    }
    output_state_clear(output);
}

static void handle_midi(void* mode_state,
                        const MidiEvent* event,
                        OutputState* output) {
    ModePoly4State* state = (ModePoly4State*)mode_state;

    switch (event->type) {
        case MIDI_EVENT_NOTE_ON: {
            const uint8_t voice = voice_for_channel(event->channel);
            state->active[voice] = true;
            state->note[voice] = event->data1;
            apply_voice(state, output, voice);
            output->gate[voice] = true;
            break;
        }

        case MIDI_EVENT_NOTE_OFF: {
            const uint8_t voice = voice_for_channel(event->channel);
            if (state->active[voice] && state->note[voice] == event->data1) {
                state->active[voice] = false;
                state->note[voice] = 0xff;
                output->gate[voice] = false;
            }
            break;
        }

        case MIDI_EVENT_PITCH_BEND: {
            const uint8_t voice = voice_for_channel(event->channel);
            state->pitch_bend[voice] = event->pitch_bend;
            apply_voice(state, output, voice);
            break;
        }

        default:
            break;
    }
}

static void tick_1ms(void* mode_state, OutputState* output) {
    (void)mode_state;
    (void)output;
}

Mode mode_poly4_create(ModePoly4State* state) {
    Mode mode = {
        .name = "midi3",
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

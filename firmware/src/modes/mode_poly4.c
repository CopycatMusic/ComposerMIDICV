#include "modes/mode_poly4.h"

#include "music/pitch_cv.h"

static void apply_active_voice_pitches(ModePoly4State* state,
                                       OutputState* output) {
    for (uint8_t voice = 0; voice < VOICE_ALLOCATOR_NUM_VOICES; ++voice) {
        if (!voice_allocator_is_active(&state->allocator, voice)) {
            continue;
        }
        output->cv_mv[voice] =
            pitch_cv_note_to_mv_bend(state->allocator.note[voice],
                                     state->pitch_bend);
    }
}

static void reset(void* mode_state, OutputState* output) {
    ModePoly4State* state = (ModePoly4State*)mode_state;
    voice_allocator_init(&state->allocator);
    state->pitch_bend = 0;
    output_state_clear(output);
}

static void handle_midi(void* mode_state,
                        const MidiEvent* event,
                        OutputState* output) {
    ModePoly4State* state = (ModePoly4State*)mode_state;

    switch (event->type) {
        case MIDI_EVENT_NOTE_ON: {
            const uint8_t voice = voice_allocator_note_on(&state->allocator,
                                                          event->data1);
            output->cv_mv[voice] =
                pitch_cv_note_to_mv_bend(event->data1, state->pitch_bend);
            output->gate[voice] = true;
            break;
        }

        case MIDI_EVENT_NOTE_OFF: {
            const uint8_t voice = voice_allocator_note_off(&state->allocator,
                                                           event->data1);
            if (voice < 4) {
                output->gate[voice] = false;
            }
            break;
        }

        case MIDI_EVENT_PITCH_BEND:
            state->pitch_bend = event->pitch_bend;
            apply_active_voice_pitches(state, output);
            break;

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
        .name = "poly4",
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

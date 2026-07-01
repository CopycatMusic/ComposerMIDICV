#include "modes/mode_drum4_velocity.h"

#include "board_config.h"
#include "music/pitch_cv.h"

static const uint8_t drum_notes[4] = {24, 26, 28, 30};

static int8_t drum_index(uint8_t note) {
    for (uint8_t i = 0; i < 4; ++i) {
        if (drum_notes[i] == note) {
            return (int8_t)i;
        }
    }
    return -1;
}

static void reset(void* mode_state, OutputState* output) {
    ModeDrum4VelocityState* state = (ModeDrum4VelocityState*)mode_state;
    for (uint8_t i = 0; i < 4; ++i) {
        state->trigger_ms[i] = 0;
    }
    output_state_clear(output);
}

static void handle_midi(void* mode_state,
                        const MidiEvent* event,
                        OutputState* output) {
    ModeDrum4VelocityState* state = (ModeDrum4VelocityState*)mode_state;
    if (event->type != MIDI_EVENT_NOTE_ON) {
        return;
    }

    const int8_t index = drum_index(event->data1);
    if (index < 0) {
        return;
    }

    output->cv_mv[index] = pitch_cv_7bit_to_mv(event->data2);
    output->gate[index] = true;
    state->trigger_ms[index] = COMPOSER_MIDICV_TRIGGER_MS;
}

static void tick_1ms(void* mode_state, OutputState* output) {
    ModeDrum4VelocityState* state = (ModeDrum4VelocityState*)mode_state;
    for (uint8_t i = 0; i < 4; ++i) {
        if (state->trigger_ms[i] == 0) {
            continue;
        }
        --state->trigger_ms[i];
        if (state->trigger_ms[i] == 0) {
            output->gate[i] = false;
        }
    }
}

Mode mode_drum4_velocity_create(ModeDrum4VelocityState* state) {
    Mode mode = {
        .name = "drum4_velocity",
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

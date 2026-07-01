#include "modes/mode_cc_convert.h"

#include "music/pitch_cv.h"

static void reset(void* mode_state, OutputState* output) {
    ModeCcConvertState* state = (ModeCcConvertState*)mode_state;
    for (uint8_t i = 0; i < 8; ++i) {
        state->last_cc[i] = 0;
    }
    output_state_clear(output);
}

static void handle_midi(void* mode_state,
                        const MidiEvent* event,
                        OutputState* output) {
    ModeCcConvertState* state = (ModeCcConvertState*)mode_state;
    if (event->type != MIDI_EVENT_CONTROL_CHANGE ||
        event->data1 < 1 ||
        event->data1 > 8) {
        return;
    }

    const uint8_t index = event->data1 - 1;
    state->last_cc[index] = event->data2;

    if (index < 4) {
        output->cv_mv[index] = pitch_cv_7bit_to_mv(event->data2);
    } else {
        output->gate[index - 4] = event->data2 >= 64;
    }
}

static void tick_1ms(void* mode_state, OutputState* output) {
    (void)mode_state;
    (void)output;
}

Mode mode_cc_convert_create(ModeCcConvertState* state) {
    Mode mode = {
        .name = "cc_convert",
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

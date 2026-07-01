#include "modes/mode_drum8_trigger.h"

#include "board_config.h"

static int8_t drum_index(uint8_t note) {
    if (note < 24 || note > 31) {
        return -1;
    }
    return (int8_t)(note - 24);
}

static void set_lane(OutputState* output, uint8_t lane, bool high) {
    if (lane < 4) {
        output->cv_mv[lane] = high ? COMPOSER_MIDICV_CV_MAX_MV : 0;
    } else {
        output->gate[lane - 4] = high;
    }
}

static void reset(void* mode_state, OutputState* output) {
    ModeDrum8TriggerState* state = (ModeDrum8TriggerState*)mode_state;
    for (uint8_t i = 0; i < 8; ++i) {
        state->trigger_ms[i] = 0;
    }
    output_state_clear(output);
}

static void handle_midi(void* mode_state,
                        const MidiEvent* event,
                        OutputState* output) {
    ModeDrum8TriggerState* state = (ModeDrum8TriggerState*)mode_state;
    if (event->type != MIDI_EVENT_NOTE_ON) {
        return;
    }

    const int8_t index = drum_index(event->data1);
    if (index < 0) {
        return;
    }

    set_lane(output, (uint8_t)index, true);
    state->trigger_ms[index] = COMPOSER_MIDICV_TRIGGER_MS;
}

static void tick_1ms(void* mode_state, OutputState* output) {
    ModeDrum8TriggerState* state = (ModeDrum8TriggerState*)mode_state;
    for (uint8_t i = 0; i < 8; ++i) {
        if (state->trigger_ms[i] == 0) {
            continue;
        }
        --state->trigger_ms[i];
        if (state->trigger_ms[i] == 0) {
            set_lane(output, i, false);
        }
    }
}

Mode mode_drum8_trigger_create(ModeDrum8TriggerState* state) {
    Mode mode = {
        .name = "drum8_trigger",
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

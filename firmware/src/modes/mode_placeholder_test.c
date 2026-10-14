#include "modes/mode_placeholder_test.h"

#include "app/output_state.h"
#include "board_config.h"

static void apply_placeholder_outputs(ModePlaceholderTestState* state,
                                      OutputState* output) {
    static const uint16_t cv_test_mv[] = {
        0u,
        2000u,
        4000u,
        6000u,
        8000u,
    };

    for (uint8_t i = 0; i < COMPOSER_MIDICV_NUM_CV_OUTPUTS; ++i) {
        output->cv_mv[i] =
            cv_test_mv[(uint8_t)((state->cv_step + i) % 5u)];
    }

    for (uint8_t i = 0; i < COMPOSER_MIDICV_NUM_GATE_OUTPUTS; ++i) {
        output->gate[i] = (i == state->active_gate);
        output->audio_hz[i] = 0;
    }
}

static void placeholder_reset(void* state, OutputState* output) {
    ModePlaceholderTestState* placeholder = (ModePlaceholderTestState*)state;
    placeholder->elapsed_ms = 0;
    placeholder->active_gate = 0;
    placeholder->cv_step = 0;
    apply_placeholder_outputs(placeholder, output);
}

static void placeholder_handle_midi(void* state,
                                    const MidiEvent* event,
                                    OutputState* output) {
    (void)event;
    apply_placeholder_outputs((ModePlaceholderTestState*)state, output);
}

static void placeholder_tick_1ms(void* state, OutputState* output) {
    ModePlaceholderTestState* placeholder = (ModePlaceholderTestState*)state;
    ++placeholder->elapsed_ms;
    if (placeholder->elapsed_ms >= MODE_PLACEHOLDER_TEST_STEP_MS) {
        placeholder->elapsed_ms = 0;
        placeholder->active_gate =
            (uint8_t)((placeholder->active_gate + 1u) %
                      COMPOSER_MIDICV_NUM_GATE_OUTPUTS);
        placeholder->cv_step = (uint8_t)((placeholder->cv_step + 1u) % 5u);
    }

    apply_placeholder_outputs(placeholder, output);
}

Mode mode_placeholder_test_create(ModePlaceholderTestState* state,
                                  const char* name) {
    state->name = name;
    state->elapsed_ms = 0;
    state->active_gate = 0;
    state->cv_step = 0;

    Mode mode = {
        .name = name,
        .state = state,
        .implemented = false,
        .reset = placeholder_reset,
        .handle_midi = placeholder_handle_midi,
        .tick_1ms = placeholder_tick_1ms,
    };
    return mode;
}

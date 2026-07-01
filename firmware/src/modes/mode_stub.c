#include "modes/mode_stub.h"

#include <stddef.h>

#include "app/output_state.h"

static void stub_reset(void* state, OutputState* output) {
    (void)state;
    output_state_clear(output);
}

static void stub_handle_midi(void* state,
                             const MidiEvent* event,
                             OutputState* output) {
    (void)state;
    (void)event;
    output_state_clear(output);
}

static void stub_tick_1ms(void* state, OutputState* output) {
    (void)state;
    (void)output;
}

Mode mode_stub_create(const char* name) {
    Mode mode = {
        .name = name,
        .state = NULL,
        .implemented = false,
        .reset = stub_reset,
        .handle_midi = stub_handle_midi,
        .tick_1ms = stub_tick_1ms,
    };
    return mode;
}

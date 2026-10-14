#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "app/output_state.h"
#include "midi/midi_event.h"
#include "midi/midi_router.h"

typedef struct {
    uint8_t id;
    uint8_t reset_count;
    uint8_t midi_count;
    uint8_t last_channel;
    uint8_t last_realtime;
} TestModeState;

static void reset_mode(void* state, OutputState* output) {
    TestModeState* test_state = (TestModeState*)state;
    ++test_state->reset_count;
    output_state_clear(output);
    output->cv_mv[0] = (uint16_t)(100u + test_state->id);
}

static void handle_midi(void* state,
                        const MidiEvent* event,
                        OutputState* output) {
    TestModeState* test_state = (TestModeState*)state;
    ++test_state->midi_count;
    test_state->last_channel = event->channel;
    test_state->last_realtime = event->realtime_byte;
    output->cv_mv[1] = (uint16_t)(200u + test_state->id);
}

static Mode test_mode_create(TestModeState* state, uint8_t id) {
    state->id = id;
    state->reset_count = 0;
    state->midi_count = 0;
    state->last_channel = 0xffu;
    state->last_realtime = 0;

    Mode mode = {
        .name = "test",
        .state = state,
        .implemented = true,
        .reset = reset_mode,
        .handle_midi = handle_midi,
        .tick_1ms = NULL,
    };
    return mode;
}

static MidiEvent note_on(uint8_t channel) {
    MidiEvent event = {
        .type = MIDI_EVENT_NOTE_ON,
        .channel = channel,
        .data1 = 60,
        .data2 = 100,
    };
    return event;
}

static MidiEvent realtime(uint8_t byte) {
    MidiEvent event = {
        .type = MIDI_EVENT_REALTIME,
        .channel = 0xffu,
        .realtime_byte = byte,
    };
    return event;
}

static void test_explicit_mode_selection_drives_router(void) {
    OutputState output;
    MidiRouter router;
    TestModeState mode0;
    TestModeState mode1;

    midi_router_init(&router, &output);
    midi_router_register_mode(&router, 0, test_mode_create(&mode0, 0));
    midi_router_register_mode(&router, 1, test_mode_create(&mode1, 1));

    assert(midi_router_active_mode_index(&router) == MIDI_ROUTER_NO_MODE);
    assert(midi_router_set_active_mode(&router, 1));
    assert(midi_router_active_mode_index(&router) == 1);
    assert(mode1.reset_count == 1);
    assert(output.cv_mv[0] == 101);

    MidiEvent event = note_on(0);
    assert(midi_router_handle_event(&router, &event));

    assert(midi_router_active_mode_index(&router) == 1);
    assert(mode0.midi_count == 0);
    assert(mode1.midi_count == 1);
    assert(mode1.last_channel == 0);
    assert(output.cv_mv[1] == 201);
}

static void test_channel_no_longer_selects_mode(void) {
    OutputState output;
    MidiRouter router;
    TestModeState mode0;
    TestModeState mode1;

    midi_router_init(&router, &output);
    midi_router_register_mode(&router, 0, test_mode_create(&mode0, 0));
    midi_router_register_mode(&router, 1, test_mode_create(&mode1, 1));
    assert(midi_router_set_active_mode(&router, 0));

    MidiEvent event = note_on(1);
    assert(midi_router_handle_event(&router, &event));

    assert(midi_router_active_mode_index(&router) == 0);
    assert(mode0.midi_count == 1);
    assert(mode0.last_channel == 1);
    assert(mode1.midi_count == 0);
}

static void test_invalid_explicit_mode_is_rejected(void) {
    OutputState output;
    MidiRouter router;
    TestModeState mode0;

    midi_router_init(&router, &output);
    midi_router_register_mode(&router, 0, test_mode_create(&mode0, 0));
    assert(midi_router_set_active_mode(&router, 0));

    assert(!midi_router_set_active_mode(&router, MIDI_ROUTER_NUM_MODES));
    assert(midi_router_active_mode_index(&router) == 0);
}

static void test_transport_running_is_restored_after_mode_switch(void) {
    OutputState output;
    MidiRouter router;
    TestModeState mode0;
    TestModeState mode1;

    midi_router_init(&router, &output);
    midi_router_register_mode(&router, 0, test_mode_create(&mode0, 0));
    midi_router_register_mode(&router, 1, test_mode_create(&mode1, 1));
    assert(midi_router_set_active_mode(&router, 0));

    MidiEvent event = realtime(0xfa);
    assert(midi_router_handle_event(&router, &event));
    assert(mode0.last_realtime == 0xfa);

    assert(midi_router_set_active_mode(&router, 1));
    assert(mode1.reset_count == 1);
    assert(mode1.midi_count == 1);
    assert(mode1.last_realtime == 0xfb);
}

static void test_transport_stop_prevents_restore_after_mode_switch(void) {
    OutputState output;
    MidiRouter router;
    TestModeState mode0;
    TestModeState mode1;

    midi_router_init(&router, &output);
    midi_router_register_mode(&router, 0, test_mode_create(&mode0, 0));
    midi_router_register_mode(&router, 1, test_mode_create(&mode1, 1));
    assert(midi_router_set_active_mode(&router, 0));

    MidiEvent event = realtime(0xfa);
    assert(midi_router_handle_event(&router, &event));
    event = realtime(0xfc);
    assert(midi_router_handle_event(&router, &event));

    assert(midi_router_set_active_mode(&router, 1));
    assert(mode1.reset_count == 1);
    assert(mode1.midi_count == 0);
}

int main(void) {
    test_explicit_mode_selection_drives_router();
    test_channel_no_longer_selects_mode();
    test_invalid_explicit_mode_is_rejected();
    test_transport_running_is_restored_after_mode_switch();
    test_transport_stop_prevents_restore_after_mode_switch();
    return 0;
}

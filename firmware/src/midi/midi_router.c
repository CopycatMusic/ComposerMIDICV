#include "midi/midi_router.h"

#include <stddef.h>

#include "app/output_state.h"
#include "modes/mode_stub.h"

void midi_router_init(MidiRouter* router, OutputState* output) {
    router->output = output;
    router->active_mode = MIDI_ROUTER_NO_MODE;
    for (uint8_t i = 0; i < MIDI_ROUTER_NUM_MODES; ++i) {
        router->modes[i] = mode_stub_create("stub");
    }
    output_state_clear(output);
}

void midi_router_register_mode(MidiRouter* router, uint8_t mode_index, Mode mode) {
    if (mode_index >= MIDI_ROUTER_NUM_MODES) {
        return;
    }
    router->modes[mode_index] = mode;
}

static bool switch_mode_if_needed(MidiRouter* router, uint8_t mode_index) {
    if (mode_index >= MIDI_ROUTER_NUM_MODES) {
        return false;
    }

    if (router->active_mode == mode_index) {
        return true;
    }

    router->active_mode = mode_index;
    output_state_clear(router->output);
    Mode* mode = &router->modes[router->active_mode];
    if (mode->reset) {
        mode->reset(mode->state, router->output);
    }
    return true;
}

bool midi_router_handle_event(MidiRouter* router, const MidiEvent* event) {
    if (!midi_event_has_channel(event)) {
        if (router->active_mode < MIDI_ROUTER_NUM_MODES) {
            Mode* mode = &router->modes[router->active_mode];
            if (mode->handle_midi) {
                mode->handle_midi(mode->state, event, router->output);
            }
        }
        return true;
    }

    if (event->channel >= MIDI_ROUTER_NUM_MODES) {
        return false;
    }

    if (!switch_mode_if_needed(router, event->channel)) {
        return false;
    }

    Mode* mode = &router->modes[router->active_mode];
    if (mode->handle_midi) {
        mode->handle_midi(mode->state, event, router->output);
    }
    return true;
}

void midi_router_tick_1ms(MidiRouter* router) {
    if (router->active_mode >= MIDI_ROUTER_NUM_MODES) {
        return;
    }

    Mode* mode = &router->modes[router->active_mode];
    if (mode->tick_1ms) {
        mode->tick_1ms(mode->state, router->output);
    }
}

const Mode* midi_router_active_mode(const MidiRouter* router) {
    if (router->active_mode >= MIDI_ROUTER_NUM_MODES) {
        return NULL;
    }
    return &router->modes[router->active_mode];
}

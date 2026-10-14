#include "midi/midi_router.h"

#include <stddef.h>

#include "app/output_state.h"
#include "modes/mode_stub.h"

#define MIDI_REALTIME_START 0xfau
#define MIDI_REALTIME_CONTINUE 0xfbu
#define MIDI_REALTIME_STOP 0xfcu

static void update_transport_state(MidiRouter* router,
                                   const MidiEvent* event) {
    if (event->type != MIDI_EVENT_REALTIME) {
        return;
    }

    switch (event->realtime_byte) {
        case MIDI_REALTIME_START:
        case MIDI_REALTIME_CONTINUE:
            router->transport_running = true;
            break;

        case MIDI_REALTIME_STOP:
            router->transport_running = false;
            break;

        default:
            break;
    }
}

static void restore_transport_to_active_mode(MidiRouter* router) {
    if (!router->transport_running ||
        router->active_mode >= MIDI_ROUTER_NUM_MODES) {
        return;
    }

    Mode* mode = &router->modes[router->active_mode];
    if (!mode->handle_midi) {
        return;
    }

    const MidiEvent event = {
        .type = MIDI_EVENT_REALTIME,
        .channel = 0xffu,
        .realtime_byte = MIDI_REALTIME_CONTINUE,
    };
    mode->handle_midi(mode->state, &event, router->output);
}

void midi_router_init(MidiRouter* router, OutputState* output) {
    router->output = output;
    router->active_mode = MIDI_ROUTER_NO_MODE;
    router->transport_running = false;
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

bool midi_router_set_active_mode(MidiRouter* router, uint8_t mode_index) {
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
    restore_transport_to_active_mode(router);
    return true;
}

uint8_t midi_router_active_mode_index(const MidiRouter* router) {
    return router->active_mode;
}

bool midi_router_handle_event(MidiRouter* router, const MidiEvent* event) {
    update_transport_state(router, event);

    if (router->active_mode >= MIDI_ROUTER_NUM_MODES) {
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

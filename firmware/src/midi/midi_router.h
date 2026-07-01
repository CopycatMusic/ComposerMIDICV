#ifndef COMPOSER_MIDICV_MIDI_MIDI_ROUTER_H_
#define COMPOSER_MIDICV_MIDI_MIDI_ROUTER_H_

#include <stdbool.h>
#include <stdint.h>

#include "app/output_state.h"
#include "midi/midi_event.h"
#include "modes/mode.h"

#define MIDI_ROUTER_NUM_MODES 7
#define MIDI_ROUTER_NO_MODE 0xffu

typedef struct {
    Mode modes[MIDI_ROUTER_NUM_MODES];
    uint8_t active_mode;
    OutputState* output;
} MidiRouter;

void midi_router_init(MidiRouter* router, OutputState* output);
void midi_router_register_mode(MidiRouter* router, uint8_t mode_index, Mode mode);
bool midi_router_handle_event(MidiRouter* router, const MidiEvent* event);
void midi_router_tick_1ms(MidiRouter* router);
const Mode* midi_router_active_mode(const MidiRouter* router);

#endif  // COMPOSER_MIDICV_MIDI_MIDI_ROUTER_H_

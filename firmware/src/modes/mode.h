#ifndef COMPOSER_MIDICV_MODES_MODE_H_
#define COMPOSER_MIDICV_MODES_MODE_H_

#include <stdbool.h>
#include <stdint.h>

#include "app/output_state.h"
#include "midi/midi_event.h"

typedef struct {
    const char* name;
    void* state;
    bool implemented;
    void (*reset)(void* state, OutputState* output);
    void (*handle_midi)(void* state, const MidiEvent* event, OutputState* output);
    void (*tick_1ms)(void* state, OutputState* output);
} Mode;

#endif  // COMPOSER_MIDICV_MODES_MODE_H_

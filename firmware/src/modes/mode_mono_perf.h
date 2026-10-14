#ifndef COMPOSER_MIDICV_MODES_MODE_MONO_PERF_H_
#define COMPOSER_MIDICV_MODES_MODE_MONO_PERF_H_

#include "modes/mode.h"
#include "music/note_stack.h"

typedef struct {
    NoteStack notes;
    uint8_t cc1;
    uint32_t random_state;
    int16_t pitch_bend;
    uint8_t trigger_ms[4];
} ModeMonoPerfState;

Mode mode_mono_perf_create(ModeMonoPerfState* state);

#endif  // COMPOSER_MIDICV_MODES_MODE_MONO_PERF_H_

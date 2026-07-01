#ifndef COMPOSER_MIDICV_MODES_MODE_MONO_RANDOM_CLOCK_H_
#define COMPOSER_MIDICV_MODES_MODE_MONO_RANDOM_CLOCK_H_

#include "modes/mode.h"
#include "music/note_stack.h"

typedef struct {
    NoteStack notes;
    int16_t pitch_bend;
    uint8_t trigger_ms;
    uint32_t random_state;
} ModeMonoRandomClockState;

Mode mode_mono_random_clock_create(ModeMonoRandomClockState* state);

#endif  // COMPOSER_MIDICV_MODES_MODE_MONO_RANDOM_CLOCK_H_

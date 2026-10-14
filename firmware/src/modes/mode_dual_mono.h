#ifndef COMPOSER_MIDICV_MODES_MODE_DUAL_MONO_H_
#define COMPOSER_MIDICV_MODES_MODE_DUAL_MONO_H_

#include <stdbool.h>
#include <stdint.h>

#include "modes/mode.h"

typedef struct {
    bool active[2];
    uint8_t note[2];
    uint8_t velocity[2];
    int16_t pitch_bend[2];
    uint8_t trigger_ms[2];
} ModeDualMonoState;

Mode mode_dual_mono_create(ModeDualMonoState* state);

#endif  // COMPOSER_MIDICV_MODES_MODE_DUAL_MONO_H_

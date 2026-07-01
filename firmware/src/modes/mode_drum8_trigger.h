#ifndef COMPOSER_MIDICV_MODES_MODE_DRUM8_TRIGGER_H_
#define COMPOSER_MIDICV_MODES_MODE_DRUM8_TRIGGER_H_

#include <stdint.h>

#include "modes/mode.h"

typedef struct {
    uint8_t trigger_ms[8];
} ModeDrum8TriggerState;

Mode mode_drum8_trigger_create(ModeDrum8TriggerState* state);

#endif  // COMPOSER_MIDICV_MODES_MODE_DRUM8_TRIGGER_H_

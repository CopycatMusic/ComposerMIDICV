#ifndef COMPOSER_MIDICV_MODES_MODE_DRUM4_VELOCITY_H_
#define COMPOSER_MIDICV_MODES_MODE_DRUM4_VELOCITY_H_

#include <stdint.h>

#include "modes/mode.h"

typedef struct {
    uint8_t trigger_ms[4];
} ModeDrum4VelocityState;

Mode mode_drum4_velocity_create(ModeDrum4VelocityState* state);

#endif  // COMPOSER_MIDICV_MODES_MODE_DRUM4_VELOCITY_H_

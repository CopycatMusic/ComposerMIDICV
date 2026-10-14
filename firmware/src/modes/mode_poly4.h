#ifndef COMPOSER_MIDICV_MODES_MODE_POLY4_H_
#define COMPOSER_MIDICV_MODES_MODE_POLY4_H_

#include <stdbool.h>
#include <stdint.h>

#include "modes/mode.h"

typedef struct {
    bool active[4];
    uint8_t note[4];
    int16_t pitch_bend[4];
} ModePoly4State;

Mode mode_poly4_create(ModePoly4State* state);

#endif  // COMPOSER_MIDICV_MODES_MODE_POLY4_H_

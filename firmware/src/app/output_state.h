#ifndef COMPOSER_MIDICV_APP_OUTPUT_STATE_H_
#define COMPOSER_MIDICV_APP_OUTPUT_STATE_H_

#include <stdbool.h>
#include <stdint.h>

#include "board_config.h"

typedef struct {
    uint16_t cv_mv[COMPOSER_MIDICV_NUM_CV_OUTPUTS];
    bool gate[COMPOSER_MIDICV_NUM_GATE_OUTPUTS];
    uint16_t audio_hz[COMPOSER_MIDICV_NUM_GATE_OUTPUTS];
} OutputState;

static inline void output_state_clear(OutputState* state) {
    for (uint8_t i = 0; i < COMPOSER_MIDICV_NUM_CV_OUTPUTS; ++i) {
        state->cv_mv[i] = 0;
    }
    for (uint8_t i = 0; i < COMPOSER_MIDICV_NUM_GATE_OUTPUTS; ++i) {
        state->gate[i] = false;
        state->audio_hz[i] = 0;
    }
}

#endif  // COMPOSER_MIDICV_APP_OUTPUT_STATE_H_

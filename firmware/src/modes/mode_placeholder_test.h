#ifndef COMPOSER_MIDICV_MODES_MODE_PLACEHOLDER_TEST_H_
#define COMPOSER_MIDICV_MODES_MODE_PLACEHOLDER_TEST_H_

#include <stdint.h>

#include "modes/mode.h"

#define MODE_PLACEHOLDER_TEST_STEP_MS 1500u

typedef struct {
    const char* name;
    uint16_t elapsed_ms;
    uint8_t active_gate;
    uint8_t cv_step;
} ModePlaceholderTestState;

Mode mode_placeholder_test_create(ModePlaceholderTestState* state,
                                  const char* name);

#endif  // COMPOSER_MIDICV_MODES_MODE_PLACEHOLDER_TEST_H_

#ifndef COMPOSER_MIDICV_MODES_MODE_CC_CONVERT_H_
#define COMPOSER_MIDICV_MODES_MODE_CC_CONVERT_H_

#include "modes/mode.h"

typedef struct {
    uint8_t last_cc[8];
} ModeCcConvertState;

Mode mode_cc_convert_create(ModeCcConvertState* state);

#endif  // COMPOSER_MIDICV_MODES_MODE_CC_CONVERT_H_

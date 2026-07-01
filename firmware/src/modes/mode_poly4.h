#ifndef COMPOSER_MIDICV_MODES_MODE_POLY4_H_
#define COMPOSER_MIDICV_MODES_MODE_POLY4_H_

#include "modes/mode.h"
#include "music/voice_allocator.h"

typedef struct {
    VoiceAllocator allocator;
    int16_t pitch_bend;
} ModePoly4State;

Mode mode_poly4_create(ModePoly4State* state);

#endif  // COMPOSER_MIDICV_MODES_MODE_POLY4_H_

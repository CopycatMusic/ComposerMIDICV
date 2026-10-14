#ifndef COMPOSER_MIDICV_MODES_MODE_CLOCKED_NOTE_H_
#define COMPOSER_MIDICV_MODES_MODE_CLOCKED_NOTE_H_

#include <stdbool.h>
#include <stdint.h>

#include "config/mode_config.h"
#include "modes/mode.h"

typedef struct {
    const char* name;
    uint8_t gate_trigger_division[2];
    bool active[2];
    uint8_t note[2];
    uint8_t velocity[2];
    int16_t pitch_bend[2];
    bool running;
    uint32_t clock_ticks;
    uint8_t gate_trigger_ms[3];
} ModeClockedNoteState;

Mode mode_clocked_note_create(ModeClockedNoteState* state);
Mode mode_clocked_note_create_from_config(ModeClockedNoteState* state,
                                          const ModeSlotConfig* config);

#endif  // COMPOSER_MIDICV_MODES_MODE_CLOCKED_NOTE_H_

#ifndef COMPOSER_MIDICV_MODES_MODE_CLOCKED_CHORD_ARP_H_
#define COMPOSER_MIDICV_MODES_MODE_CLOCKED_CHORD_ARP_H_

#include <stdbool.h>
#include <stdint.h>

#include "modes/mode.h"
#include "music/note_stack.h"

typedef struct {
    NoteStack notes;
    bool running;
    uint32_t clock_ticks;
    uint8_t arp_index_1ppqn;
    uint8_t arp_index_4ppqn;
    uint32_t random_state;
    uint8_t gate_trigger_ms[3];
} ModeClockedChordArpState;

Mode mode_clocked_chord_arp_create(ModeClockedChordArpState* state);

#endif  // COMPOSER_MIDICV_MODES_MODE_CLOCKED_CHORD_ARP_H_

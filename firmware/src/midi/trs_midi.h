#ifndef COMPOSER_MIDICV_MIDI_TRS_MIDI_H_
#define COMPOSER_MIDICV_MIDI_TRS_MIDI_H_

#include <stdbool.h>

#include "midi/midi_event.h"

void trs_midi_init(void);
bool trs_midi_read_event(bool usb_connected, MidiEvent* event);

#endif  // COMPOSER_MIDICV_MIDI_TRS_MIDI_H_

#ifndef COMPOSER_MIDICV_MIDI_USB_MIDI_H_
#define COMPOSER_MIDICV_MIDI_USB_MIDI_H_

#include <stdbool.h>

#include "midi/midi_event.h"

bool usb_midi_read_event(MidiEvent* event);

#endif  // COMPOSER_MIDICV_MIDI_USB_MIDI_H_

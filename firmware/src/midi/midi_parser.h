#ifndef COMPOSER_MIDICV_MIDI_MIDI_PARSER_H_
#define COMPOSER_MIDICV_MIDI_MIDI_PARSER_H_

#include <stdbool.h>
#include <stdint.h>

#include "midi/midi_event.h"

bool midi_parser_parse_usb_packet(const uint8_t packet[4], MidiEvent* event);

#endif  // COMPOSER_MIDICV_MIDI_MIDI_PARSER_H_

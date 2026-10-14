#ifndef COMPOSER_MIDICV_MIDI_MIDI_STREAM_PARSER_H_
#define COMPOSER_MIDICV_MIDI_MIDI_STREAM_PARSER_H_

#include <stdbool.h>
#include <stdint.h>

#include "midi/midi_event.h"

typedef struct {
    uint8_t running_status;
    uint8_t pending_status;
    uint8_t data[2];
    uint8_t data_count;
    uint8_t data_needed;
} MidiStreamParser;

void midi_stream_parser_init(MidiStreamParser* parser);
bool midi_stream_parser_parse_byte(MidiStreamParser* parser,
                                   uint8_t byte,
                                   MidiEvent* event);

#endif  // COMPOSER_MIDICV_MIDI_MIDI_STREAM_PARSER_H_

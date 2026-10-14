#ifndef COMPOSER_MIDICV_MIDI_MIDI_EVENT_H_
#define COMPOSER_MIDICV_MIDI_MIDI_EVENT_H_

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MIDI_EVENT_NONE = 0,
    MIDI_EVENT_NOTE_OFF,
    MIDI_EVENT_NOTE_ON,
    MIDI_EVENT_CONTROL_CHANGE,
    MIDI_EVENT_PITCH_BEND,
    MIDI_EVENT_PROGRAM_CHANGE,
    MIDI_EVENT_REALTIME,
    MIDI_EVENT_UNKNOWN,
} MidiEventType;

typedef enum {
    MIDI_SOURCE_UNKNOWN = 0,
    MIDI_SOURCE_USB,
    MIDI_SOURCE_TRS,
} MidiSource;

typedef struct {
    MidiEventType type;
    MidiSource source;
    uint8_t channel;  // 0-15 for channel voice messages, 0xff otherwise.
    uint8_t data1;
    uint8_t data2;
    int16_t pitch_bend;
    uint8_t realtime_byte;
    bool usb_connected;
} MidiEvent;

static inline bool midi_event_has_channel(const MidiEvent* event) {
    return event->channel < 16;
}

#endif  // COMPOSER_MIDICV_MIDI_MIDI_EVENT_H_

#ifndef COMPOSER_MIDICV_UI_LED_FEEDBACK_H_
#define COMPOSER_MIDICV_UI_LED_FEEDBACK_H_

#include <stdint.h>

#include "midi/midi_event.h"
#include "music/note_stack.h"

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} LedRgb;

typedef struct {
    NoteStack notes;
    LedRgb current;
    LedRgb target;
} LedFeedbackState;

void led_feedback_init(LedFeedbackState* state);
void led_feedback_handle_midi(LedFeedbackState* state, const MidiEvent* event);
void led_feedback_tick_1ms(LedFeedbackState* state);
LedRgb led_feedback_rgb(const LedFeedbackState* state);

#endif  // COMPOSER_MIDICV_UI_LED_FEEDBACK_H_

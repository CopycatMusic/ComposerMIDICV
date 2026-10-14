#ifndef COMPOSER_MIDICV_UI_LED_FEEDBACK_H_
#define COMPOSER_MIDICV_UI_LED_FEEDBACK_H_

#include <stdbool.h>
#include <stdint.h>

#include "config/mode_config.h"
#include "midi/midi_event.h"
#include "music/note_stack.h"

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} LedRgb;

typedef struct {
    NoteStack notes[4];
    ModeEngineType engine;
    uint8_t selected_voice;
    uint8_t transport_flash_remaining_ms;
    bool range_error[4];
    uint16_t range_flash_phase_ms;
    LedRgb current;
} LedFeedbackState;

void led_feedback_init(LedFeedbackState* state);
void led_feedback_set_mode(LedFeedbackState* state, ModeEngineType engine);
void led_feedback_handle_midi(LedFeedbackState* state, const MidiEvent* event);
void led_feedback_tick_1ms(LedFeedbackState* state);
LedRgb led_feedback_rgb(const LedFeedbackState* state);

#endif  // COMPOSER_MIDICV_UI_LED_FEEDBACK_H_

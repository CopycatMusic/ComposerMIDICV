#ifndef COMPOSER_MIDICV_UI_LED_CHASER_H_
#define COMPOSER_MIDICV_UI_LED_CHASER_H_

#include <stdint.h>

#include "ui/led_feedback.h"

#define LED_CHASER_STEP_DELAY_MS 20u
#define LED_CHASER_HUE_STEPS 1536u
#define LED_CHASER_BRIGHTNESS_PERIOD_MS 4000u
#define LED_CHASER_BRIGHTNESS_PHASE_OFFSET_MS 1000u
#define LED_CHASER_BRIGHTNESS_MIN 0u
#define LED_CHASER_BRIGHTNESS_MAX 255u

typedef struct {
    uint16_t hue;
    uint16_t brightness_phase_ms;
    uint8_t elapsed_ms;
} LedChaserState;

void led_chaser_init(LedChaserState* state);
void led_chaser_tick_1ms(LedChaserState* state);
LedRgb led_chaser_rgb(const LedChaserState* state);

#endif  // COMPOSER_MIDICV_UI_LED_CHASER_H_

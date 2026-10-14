#include "ui/led_chaser.h"

static uint8_t ramp_up(uint16_t phase) {
    return (uint8_t)phase;
}

static uint8_t ramp_down(uint16_t phase) {
    return (uint8_t)(255u - phase);
}

static LedRgb color_wheel(uint16_t hue) {
    hue %= LED_CHASER_HUE_STEPS;
    const uint16_t sector = hue / 256u;
    const uint16_t phase = hue % 256u;

    switch (sector) {
        case 0:
            return (LedRgb){255, ramp_up(phase), 0};
        case 1:
            return (LedRgb){ramp_down(phase), 255, 0};
        case 2:
            return (LedRgb){0, 255, ramp_up(phase)};
        case 3:
            return (LedRgb){0, ramp_down(phase), 255};
        case 4:
            return (LedRgb){ramp_up(phase), 0, 255};
        default:
            return (LedRgb){255, 0, ramp_down(phase)};
    }
}

static uint8_t brightness_for_phase(uint16_t phase_ms) {
    const uint16_t half_period = LED_CHASER_BRIGHTNESS_PERIOD_MS / 2u;
    const uint16_t range =
        LED_CHASER_BRIGHTNESS_MAX - LED_CHASER_BRIGHTNESS_MIN;
    const uint16_t position =
        phase_ms < half_period ? phase_ms :
                                 LED_CHASER_BRIGHTNESS_PERIOD_MS - phase_ms;
    return (uint8_t)(LED_CHASER_BRIGHTNESS_MIN +
                     ((uint32_t)range * position) / half_period);
}

static uint8_t scale_channel(uint8_t value, uint8_t brightness) {
    return (uint8_t)(((uint16_t)value * brightness + 127u) / 255u);
}

void led_chaser_init(LedChaserState* state) {
    state->hue = 0;
    state->brightness_phase_ms = LED_CHASER_BRIGHTNESS_PHASE_OFFSET_MS;
    state->elapsed_ms = 0;
}

void led_chaser_tick_1ms(LedChaserState* state) {
    state->brightness_phase_ms =
        (uint16_t)((state->brightness_phase_ms + 1u) %
                   LED_CHASER_BRIGHTNESS_PERIOD_MS);

    ++state->elapsed_ms;
    if (state->elapsed_ms < LED_CHASER_STEP_DELAY_MS) {
        return;
    }

    state->elapsed_ms = 0;
    state->hue = (uint16_t)((state->hue + 1u) % LED_CHASER_HUE_STEPS);
}

LedRgb led_chaser_rgb(const LedChaserState* state) {
    const LedRgb base = color_wheel(state->hue);
    const uint8_t brightness = brightness_for_phase(state->brightness_phase_ms);
    return (LedRgb){
        scale_channel(base.r, brightness),
        scale_channel(base.g, brightness),
        scale_channel(base.b, brightness),
    };
}

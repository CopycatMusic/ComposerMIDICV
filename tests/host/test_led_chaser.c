#include <assert.h>
#include <stdint.h>

#include "ui/led_chaser.h"

static void tick_ms(LedChaserState* state, uint16_t count) {
    for (uint16_t i = 0; i < count; ++i) {
        led_chaser_tick_1ms(state);
    }
}

static void test_starts_at_first_color_wheel_step(void) {
    LedChaserState state;
    led_chaser_init(&state);

    LedRgb rgb = led_chaser_rgb(&state);
    assert(rgb.r == 127);
    assert(rgb.g == 0);
    assert(rgb.b == 0);
}

static void test_advances_once_every_20ms(void) {
    LedChaserState state;
    led_chaser_init(&state);

    tick_ms(&state, 19);
    LedRgb rgb = led_chaser_rgb(&state);
    assert(rgb.r == 129);
    assert(rgb.g == 0);
    assert(rgb.b == 0);

    led_chaser_tick_1ms(&state);
    rgb = led_chaser_rgb(&state);
    assert(rgb.r == 130);
    assert(rgb.g == 1);
    assert(rgb.b == 0);
}

static void test_brightness_modulates_on_four_second_offset_cycle(void) {
    LedChaserState state;
    led_chaser_init(&state);

    tick_ms(&state, 1000);
    LedRgb rgb = led_chaser_rgb(&state);
    assert(rgb.r == 255);
    assert(rgb.g == 50);
    assert(rgb.b == 0);

    tick_ms(&state, 2000);
    rgb = led_chaser_rgb(&state);
    assert(rgb.r == 0);
    assert(rgb.g == 0);
    assert(rgb.b == 0);
}

static void test_wraps_after_full_color_wheel(void) {
    LedChaserState state;
    led_chaser_init(&state);

    tick_ms(&state, LED_CHASER_STEP_DELAY_MS * LED_CHASER_HUE_STEPS);
    LedRgb rgb = led_chaser_rgb(&state);
    assert(rgb.r == 35);
    assert(rgb.g == 0);
    assert(rgb.b == 0);
}

int main(void) {
    test_starts_at_first_color_wheel_step();
    test_advances_once_every_20ms();
    test_brightness_modulates_on_four_second_offset_cycle();
    test_wraps_after_full_color_wheel();
    return 0;
}

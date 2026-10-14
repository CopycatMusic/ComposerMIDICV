#include "hardware/led.h"

#ifndef COMPOSER_MIDICV_HOST_TEST
#include "hardware/board_pins.h"
#include "hardware/pwm.h"
#include "pico/stdlib.h"
#endif

#define LED_GAIN_R_NUMERATOR 24u
#define LED_GAIN_G_NUMERATOR 18u
#define LED_GAIN_B_NUMERATOR 22u
#define LED_GAIN_DENOMINATOR 100u

static uint8_t apply_gain(uint8_t value, uint8_t numerator) {
    return (uint8_t)(((uint16_t)value * numerator + 50u) /
                     LED_GAIN_DENOMINATOR);
}

void led_compensate_rgb(uint8_t* r, uint8_t* g, uint8_t* b) {
    *r = apply_gain(*r, LED_GAIN_R_NUMERATOR);
    *g = apply_gain(*g, LED_GAIN_G_NUMERATOR);
    *b = apply_gain(*b, LED_GAIN_B_NUMERATOR);
}

#ifndef COMPOSER_MIDICV_HOST_TEST
static void led_channel_put_u8(uint pin, uint8_t value) {
#if LED_ACTIVE_LOW
    value = 255u - value;
#endif
    pwm_set_gpio_level(pin, value);
}

void led_init(void) {
    const uint pins[3] = {PIN_LED_R, PIN_LED_G, PIN_LED_B};
    for (uint i = 0; i < 3; ++i) {
        gpio_set_function(pins[i], GPIO_FUNC_PWM);
        const uint slice = pwm_gpio_to_slice_num(pins[i]);
        pwm_config config = pwm_get_default_config();
        pwm_config_set_wrap(&config, 255);
        pwm_init(slice, &config, true);
    }
    led_off();
}

void led_set_rgb(bool r, bool g, bool b) {
    led_set_rgb_u8(r ? 255 : 0, g ? 255 : 0, b ? 255 : 0);
}

void led_set_rgb_u8(uint8_t r, uint8_t g, uint8_t b) {
    led_compensate_rgb(&r, &g, &b);
    led_channel_put_u8(PIN_LED_R, r);
    led_channel_put_u8(PIN_LED_G, g);
    led_channel_put_u8(PIN_LED_B, b);
}

void led_off(void) {
    led_set_rgb(false, false, false);
}

void led_set_ready(void) {
    led_set_rgb(false, true, false);
}

void led_set_error(void) {
    led_set_rgb(true, false, false);
}

void led_set_calibration(void) {
    led_set_rgb(true, true, false);
}

void led_flash_midi(void) {
    led_set_rgb(false, false, true);
}

void led_tick_1ms(void) {
}
#endif

#include "hardware/pwm.h"
#include "pico/stdlib.h"

#include <stdbool.h>
#include <stdint.h>

#define PIN_LED_R 0
#define PIN_LED_G 1
#define PIN_LED_B 2
#define LED_ACTIVE_LOW 1

#define PWM_WRAP 255u
#define STEP_DELAY_MS 20u
#define HUE_STEPS 1536u

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Rgb;

static uint8_t gamma_correct(uint8_t value) {
    return value;
}

static uint16_t led_level(uint8_t value) {
    const uint8_t corrected = gamma_correct(value);
#if LED_ACTIVE_LOW
    return PWM_WRAP - corrected;
#else
    return corrected;
#endif
}

static void set_pwm_level(uint pin, uint8_t value) {
    pwm_set_gpio_level(pin, led_level(value));
}

static void led_set_rgb(Rgb color) {
    set_pwm_level(PIN_LED_R, color.r);
    set_pwm_level(PIN_LED_G, color.g);
    set_pwm_level(PIN_LED_B, color.b);
}

static uint8_t ramp_up(uint16_t phase) {
    return (uint8_t)((phase * 255u) / 255u);
}

static uint8_t ramp_down(uint16_t phase) {
    return (uint8_t)(255u - ((phase * 255u) / 255u));
}

static Rgb color_wheel(uint16_t hue) {
    hue %= HUE_STEPS;
    const uint16_t sector = hue / 256u;
    const uint16_t phase = hue % 256u;

    switch (sector) {
        case 0:
            return (Rgb){255, ramp_up(phase), 0};
        case 1:
            return (Rgb){ramp_down(phase), 255, 0};
        case 2:
            return (Rgb){0, 255, ramp_up(phase)};
        case 3:
            return (Rgb){0, ramp_down(phase), 255};
        case 4:
            return (Rgb){ramp_up(phase), 0, 255};
        default:
            return (Rgb){255, 0, ramp_down(phase)};
    }
}

static void pwm_init_led_pin(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);

    const uint slice = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_WRAP);
    pwm_config_set_clkdiv(&config, 64.0f);
    pwm_init(slice, &config, true);

    set_pwm_level(pin, 0);
}

int main(void) {
    stdio_init_all();

    pwm_init_led_pin(PIN_LED_R);
    pwm_init_led_pin(PIN_LED_G);
    pwm_init_led_pin(PIN_LED_B);

    uint16_t hue = 0;
    while (true) {
        led_set_rgb(color_wheel(hue));
        hue = (uint16_t)((hue + 1u) % HUE_STEPS);
        sleep_ms(STEP_DELAY_MS);
    }
}

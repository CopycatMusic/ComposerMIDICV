#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "hardware/board_pins.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"

#define MODE_SELECTOR_INVALID_POSITION 0xffu

static void led_channel(uint pin, bool on) {
#if LED_ACTIVE_LOW
    gpio_put(pin, !on);
#else
    gpio_put(pin, on);
#endif
}

static void led_set(bool r, bool g, bool b) {
    led_channel(PIN_LED_R, r);
    led_channel(PIN_LED_G, g);
    led_channel(PIN_LED_B, b);
}

static void init_output_pin(uint pin, bool initial_high) {
    gpio_init(pin);
    gpio_set_function(pin, GPIO_FUNC_SIO);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, initial_high);
}

static void init_selector_pin(uint pin) {
    gpio_init(pin);
    gpio_set_function(pin, GPIO_FUNC_SIO);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

static void init_hardware(void) {
    init_output_pin(PIN_LED_R, LED_ACTIVE_LOW ? true : false);
    init_output_pin(PIN_LED_G, LED_ACTIVE_LOW ? true : false);
    init_output_pin(PIN_LED_B, LED_ACTIVE_LOW ? true : false);

    init_output_pin(PIN_GATE_1, false);
    init_output_pin(PIN_GATE_2, false);
    init_output_pin(PIN_GATE_3, false);
    init_output_pin(PIN_GATE_4, false);

    init_selector_pin(PIN_MODE_SELECTOR_SW3);
    init_selector_pin(PIN_MODE_SELECTOR_SW4A);
    init_selector_pin(PIN_MODE_SELECTOR_SW4B);
}

static uint8_t decode_position(bool sw3_active,
                               bool sw4a_active,
                               bool sw4b_active) {
    if (sw4a_active && sw4b_active) {
        return MODE_SELECTOR_INVALID_POSITION;
    }

    uint8_t sw4_position = 1u;
    if (sw4a_active) {
        sw4_position = 0u;
    } else if (sw4b_active) {
        sw4_position = 2u;
    }

    return (uint8_t)(sw4_position + (sw3_active ? 3u : 0u));
}

static uint8_t raw_switch_state(bool sw3_active,
                                bool sw4a_active,
                                bool sw4b_active) {
    uint8_t raw = 0;
    if (sw3_active) {
        raw |= 0x01u;
    }
    if (sw4a_active) {
        raw |= 0x02u;
    }
    if (sw4b_active) {
        raw |= 0x04u;
    }
    return raw;
}

static void apply_gate_indicators(bool sw3_active,
                                  bool sw4a_active,
                                  bool sw4b_active,
                                  bool position_valid) {
    gpio_put(PIN_GATE_1, sw3_active);
    gpio_put(PIN_GATE_2, sw4a_active);
    gpio_put(PIN_GATE_3, sw4b_active);
    gpio_put(PIN_GATE_4, position_valid);
}

static void apply_led_indicator(uint8_t position, bool blink_on) {
    switch (position) {
        case 0:
            led_set(true, false, false);
            break;
        case 1:
            led_set(false, true, false);
            break;
        case 2:
            led_set(false, false, true);
            break;
        case 3:
            led_set(true, true, false);
            break;
        case 4:
            led_set(false, true, true);
            break;
        case 5:
            led_set(true, false, true);
            break;
        default:
            led_set(blink_on, blink_on, blink_on);
            break;
    }
}

static void print_status(bool usb_mounted,
                         uint8_t raw,
                         bool sw3_active,
                         bool sw4a_active,
                         bool sw4b_active,
                         uint8_t position) {
    if (position == MODE_SELECTOR_INVALID_POSITION) {
        printf("usb=%s raw=0x%02x sw3=%u sw4a=%u sw4b=%u pos=invalid mode=invalid\r\n",
               usb_mounted ? "mounted" : "not-mounted",
               raw,
               sw3_active ? 1u : 0u,
               sw4a_active ? 1u : 0u,
               sw4b_active ? 1u : 0u);
    } else {
        printf("usb=%s raw=0x%02x sw3=%u sw4a=%u sw4b=%u pos=%u mode=%u\r\n",
               usb_mounted ? "mounted" : "not-mounted",
               raw,
               sw3_active ? 1u : 0u,
               sw4a_active ? 1u : 0u,
               sw4b_active ? 1u : 0u,
               position,
               (uint8_t)(position + 1u));
    }
}

int main(void) {
    stdio_init_all();
    init_hardware();

    uint32_t last_print_ms = 0;

    while (true) {
        const bool sw3_active = !gpio_get(PIN_MODE_SELECTOR_SW3);
        const bool sw4a_active = !gpio_get(PIN_MODE_SELECTOR_SW4A);
        const bool sw4b_active = !gpio_get(PIN_MODE_SELECTOR_SW4B);
        const uint8_t position =
            decode_position(sw3_active, sw4a_active, sw4b_active);
        const bool position_valid =
            position != MODE_SELECTOR_INVALID_POSITION;
        const uint8_t raw =
            raw_switch_state(sw3_active, sw4a_active, sw4b_active);

        const uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        const bool blink_on = ((now_ms / 250u) % 2u) == 0u;

        apply_gate_indicators(sw3_active,
                              sw4a_active,
                              sw4b_active,
                              position_valid);
        apply_led_indicator(position, blink_on);

        if ((uint32_t)(now_ms - last_print_ms) >= 250u) {
            last_print_ms = now_ms;
            print_status(stdio_usb_connected(),
                         raw,
                         sw3_active,
                         sw4a_active,
                         sw4b_active,
                         position);
        }

        sleep_ms(5);
    }
}

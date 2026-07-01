#include "pico/stdlib.h"

#include <stdbool.h>
#include <stdint.h>

#define PIN_LED_R 0
#define PIN_LED_G 1
#define PIN_LED_B 2
#define LED_ACTIVE_LOW 1

#define PIN_DAC_SCK 26
#define PIN_DAC_MOSI 19
#define PIN_DAC_CS_U5 20
#define PIN_DAC_CS_U6 21

#define PIN_GATE_1 22
#define PIN_GATE_2 23
#define PIN_GATE_3 24
#define PIN_GATE_4 25

static const uint gate_pins[] = {
    PIN_GATE_1,
    PIN_GATE_2,
    PIN_GATE_3,
    PIN_GATE_4,
};

static void gpio_out_init(uint pin, bool initial_high) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, initial_high);
}

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

static void all_gates_low(void) {
    for (uint i = 0; i < 4; ++i) {
        gpio_put(gate_pins[i], false);
    }
}

static void dac_clock_bit(bool bit_value) {
    gpio_put(PIN_DAC_MOSI, bit_value);
    sleep_us(1);
    gpio_put(PIN_DAC_SCK, true);
    sleep_us(1);
    gpio_put(PIN_DAC_SCK, false);
    sleep_us(1);
}

static void dac_write_raw(uint cs_pin, bool channel_b, uint16_t code) {
    if (code > 4095) {
        code = 4095;
    }

    // MCP4822: A/B, unbuffered Vref, 2x gain, active output, 12-bit code.
    uint16_t frame = (channel_b ? 0x8000 : 0x0000) | 0x1000 | (code & 0x0fff);

    gpio_put(PIN_DAC_SCK, false);
    gpio_put(cs_pin, false);
    sleep_us(1);

    for (int bit = 15; bit >= 0; --bit) {
        dac_clock_bit((frame >> bit) & 1u);
    }

    gpio_put(cs_pin, true);
    sleep_us(1);
}

static void dac_write_mv(uint cs_pin, bool channel_b, uint16_t millivolts) {
    // With MCP4822 2x gain, output is approximately 1mV per LSB.
    dac_write_raw(cs_pin, channel_b, millivolts);
}

static void set_cv_outputs(uint16_t cv1_mv, uint16_t cv2_mv,
                           uint16_t cv3_mv, uint16_t cv4_mv) {
    dac_write_mv(PIN_DAC_CS_U5, false, cv1_mv);
    dac_write_mv(PIN_DAC_CS_U5, true, cv2_mv);
    dac_write_mv(PIN_DAC_CS_U6, false, cv3_mv);
    dac_write_mv(PIN_DAC_CS_U6, true, cv4_mv);
}

static void init_board_pins(void) {
    gpio_out_init(PIN_LED_R, LED_ACTIVE_LOW ? true : false);
    gpio_out_init(PIN_LED_G, LED_ACTIVE_LOW ? true : false);
    gpio_out_init(PIN_LED_B, LED_ACTIVE_LOW ? true : false);

    gpio_out_init(PIN_DAC_SCK, false);
    gpio_out_init(PIN_DAC_MOSI, false);
    gpio_out_init(PIN_DAC_CS_U5, true);
    gpio_out_init(PIN_DAC_CS_U6, true);

    for (uint i = 0; i < 4; ++i) {
        gpio_out_init(gate_pins[i], false);
    }
}

static void led_self_test(void) {
    led_set(true, false, false);
    sleep_ms(250);
    led_set(false, true, false);
    sleep_ms(250);
    led_set(false, false, true);
    sleep_ms(250);
    led_set(true, true, true);
    sleep_ms(250);
    led_set(false, false, false);
}

static void gate_self_test(void) {
    led_set(false, true, false);

    for (uint i = 0; i < 4; ++i) {
        all_gates_low();
        gpio_put(gate_pins[i], true);
        sleep_ms(500);
    }

    all_gates_low();
}

static void cv_self_test(void) {
    led_set(false, false, true);

    set_cv_outputs(500, 1000, 2000, 3000);
    sleep_ms(1500);

    set_cv_outputs(0, 0, 0, 0);
    sleep_ms(500);

    set_cv_outputs(1000, 1000, 1000, 1000);
    sleep_ms(500);

    set_cv_outputs(2000, 2000, 2000, 2000);
    sleep_ms(500);

    set_cv_outputs(3000, 3000, 3000, 3000);
    sleep_ms(500);

    set_cv_outputs(4095, 4095, 4095, 4095);
    sleep_ms(500);
}

int main(void) {
    stdio_init_all();
    init_board_pins();
    sleep_ms(100);

    while (true) {
        led_self_test();
        gate_self_test();
        cv_self_test();

        led_set(true, true, false);
        all_gates_low();
        set_cv_outputs(0, 0, 0, 0);
        sleep_ms(1000);
    }
}

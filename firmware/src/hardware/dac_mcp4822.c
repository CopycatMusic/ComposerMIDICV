#include "hardware/dac_mcp4822.h"

#include "board_config.h"
#include "hardware/board_pins.h"
#include "pico/stdlib.h"

static uint16_t clamp_code(uint16_t code) {
    if (code > COMPOSER_MIDICV_DAC_MAX_CODE) {
        return COMPOSER_MIDICV_DAC_MAX_CODE;
    }
    return code;
}

static void clock_bit(bool bit_value) {
    gpio_put(PIN_DAC_MOSI, bit_value);
    sleep_us(1);
    gpio_put(PIN_DAC_SCK, true);
    sleep_us(1);
    gpio_put(PIN_DAC_SCK, false);
    sleep_us(1);
}

static void write_frame(uint cs_pin, bool channel_b, uint16_t code) {
    code = clamp_code(code);

    // MCP4822: A/B, unbuffered Vref, 2x gain, active output, 12-bit code.
    const uint16_t frame = (channel_b ? 0x8000u : 0x0000u) |
                           0x1000u |
                           (code & 0x0fffu);

    gpio_put(PIN_DAC_SCK, false);
    gpio_put(cs_pin, false);
    sleep_us(1);

    for (int bit = 15; bit >= 0; --bit) {
        clock_bit((frame >> bit) & 1u);
    }

    gpio_put(cs_pin, true);
    sleep_us(1);
}

void dac_mcp4822_init(void) {
    gpio_init(PIN_DAC_SCK);
    gpio_init(PIN_DAC_MOSI);
    gpio_init(PIN_DAC_CS_U8);
    gpio_init(PIN_DAC_CS_U9);

    gpio_set_dir(PIN_DAC_SCK, GPIO_OUT);
    gpio_set_dir(PIN_DAC_MOSI, GPIO_OUT);
    gpio_set_dir(PIN_DAC_CS_U8, GPIO_OUT);
    gpio_set_dir(PIN_DAC_CS_U9, GPIO_OUT);

    gpio_put(PIN_DAC_SCK, false);
    gpio_put(PIN_DAC_MOSI, false);
    gpio_put(PIN_DAC_CS_U8, true);
    gpio_put(PIN_DAC_CS_U9, true);

    dac_mcp4822_all_zero();
}

void dac_mcp4822_write_channel(uint8_t channel, uint16_t code) {
    switch (channel) {
        case 0:
            write_frame(PIN_DAC_CS_U8, false, code);
            break;
        case 1:
            write_frame(PIN_DAC_CS_U8, true, code);
            break;
        case 2:
            write_frame(PIN_DAC_CS_U9, false, code);
            break;
        case 3:
            write_frame(PIN_DAC_CS_U9, true, code);
            break;
        default:
            break;
    }
}

void dac_mcp4822_write_all(const uint16_t code[4]) {
    for (uint i = 0; i < 4; ++i) {
        dac_mcp4822_write_channel(i, code[i]);
    }
}

void dac_mcp4822_all_zero(void) {
    const uint16_t zero[4] = {0, 0, 0, 0};
    dac_mcp4822_write_all(zero);
}

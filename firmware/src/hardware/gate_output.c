#include "hardware/gate_output.h"

#include "hardware/board_pins.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "hardware/pio_instructions.h"
#include "pico/stdlib.h"

enum {
    AUDIO_PIO_CYCLES_PER_PERIOD = 64,
};

static const uint gate_pins[4] = {
    PIN_GATE_1,
    PIN_GATE_2,
    PIN_GATE_3,
    PIN_GATE_4,
};

static uint16_t square_program_instructions[2];

static const struct pio_program square_program = {
    .instructions = square_program_instructions,
    .length = 2,
    .origin = -1,
};

static bool audio_program_loaded;
static uint audio_program_offset;
static uint16_t active_audio_hz[4];

static void ensure_audio_program_loaded(void) {
    if (audio_program_loaded) {
        return;
    }
    square_program_instructions[0] =
        pio_encode_set(pio_pins, 1) | pio_encode_delay(31);
    square_program_instructions[1] =
        pio_encode_set(pio_pins, 0) | pio_encode_delay(31);
    audio_program_offset = pio_add_program(pio0, &square_program);
    audio_program_loaded = true;
}

static void gate_audio_disable(uint8_t index) {
    if (active_audio_hz[index] == 0) {
        return;
    }

    pio_sm_set_enabled(pio0, index, false);
    gpio_set_function(gate_pins[index], GPIO_FUNC_SIO);
    gpio_set_dir(gate_pins[index], GPIO_OUT);
    gpio_put(gate_pins[index], false);
    active_audio_hz[index] = 0;
}

static void gate_audio_set(uint8_t index, uint16_t hz) {
    if (active_audio_hz[index] == hz) {
        return;
    }

    ensure_audio_program_loaded();

    pio_sm_set_enabled(pio0, index, false);
    pio_gpio_init(pio0, gate_pins[index]);
    pio_sm_set_consecutive_pindirs(pio0, index, gate_pins[index], 1, true);

    pio_sm_config config = pio_get_default_sm_config();
    sm_config_set_set_pins(&config, gate_pins[index], 1);

    const uint32_t clk_hz = clock_get_hz(clk_sys);
    float divider = (float)clk_hz /
                    ((float)hz * (float)AUDIO_PIO_CYCLES_PER_PERIOD);
    if (divider < 1.0f) {
        divider = 1.0f;
    } else if (divider > 65535.0f) {
        divider = 65535.0f;
    }

    sm_config_set_clkdiv(&config, divider);
    pio_sm_init(pio0, index, audio_program_offset, &config);
    pio_sm_set_enabled(pio0, index, true);
    active_audio_hz[index] = hz;
}

void gate_output_init(void) {
    for (uint i = 0; i < 4; ++i) {
        gpio_init(gate_pins[i]);
        gpio_set_dir(gate_pins[i], GPIO_OUT);
        gpio_put(gate_pins[i], false);
        active_audio_hz[i] = 0;
    }
}

void gate_output_apply(const bool gate[4], const uint16_t audio_hz[4]) {
    for (uint i = 0; i < 4; ++i) {
        if (audio_hz[i] != 0) {
            gate_audio_set((uint8_t)i, audio_hz[i]);
            continue;
        }
        gate_audio_disable((uint8_t)i);
        gpio_put(gate_pins[i], gate[i]);
    }
}

void gate_output_set(uint8_t index, bool high) {
    if (index >= 4) {
        return;
    }
    gate_audio_disable(index);
    gpio_put(gate_pins[index], high);
}

void gate_output_all_low(void) {
    for (uint i = 0; i < 4; ++i) {
        gate_audio_disable((uint8_t)i);
        gpio_put(gate_pins[i], false);
    }
}

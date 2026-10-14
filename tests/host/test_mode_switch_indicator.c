#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "ui/mode_switch_indicator.h"

static void tick_ms(ModeSwitchIndicatorState* state, uint16_t count) {
    for (uint16_t i = 0; i < count; ++i) {
        mode_switch_indicator_tick_1ms(state);
    }
}

int main(void) {
    ModeSwitchIndicatorState state;
    mode_switch_indicator_init(&state);
    assert(!mode_switch_indicator_flashing(&state));

    mode_switch_indicator_trigger(&state);
    assert(mode_switch_indicator_flashing(&state));

    tick_ms(&state, MODE_SWITCH_INDICATOR_FLASH_MS - 1u);
    assert(mode_switch_indicator_flashing(&state));

    mode_switch_indicator_tick_1ms(&state);
    assert(!mode_switch_indicator_flashing(&state));

    mode_switch_indicator_trigger(&state);
    tick_ms(&state, MODE_SWITCH_INDICATOR_FLASH_MS / 2u);
    mode_switch_indicator_trigger(&state);
    tick_ms(&state, MODE_SWITCH_INDICATOR_FLASH_MS - 1u);
    assert(mode_switch_indicator_flashing(&state));

    return 0;
}

#include "ui/mode_switch_indicator.h"

void mode_switch_indicator_init(ModeSwitchIndicatorState* state) {
    state->flash_remaining_ms = 0;
}

void mode_switch_indicator_trigger(ModeSwitchIndicatorState* state) {
    state->flash_remaining_ms = MODE_SWITCH_INDICATOR_FLASH_MS;
}

void mode_switch_indicator_tick_1ms(ModeSwitchIndicatorState* state) {
    if (state->flash_remaining_ms > 0) {
        --state->flash_remaining_ms;
    }
}

bool mode_switch_indicator_flashing(const ModeSwitchIndicatorState* state) {
    return state->flash_remaining_ms > 0;
}

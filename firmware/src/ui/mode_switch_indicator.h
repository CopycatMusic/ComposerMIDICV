#ifndef COMPOSER_MIDICV_UI_MODE_SWITCH_INDICATOR_H_
#define COMPOSER_MIDICV_UI_MODE_SWITCH_INDICATOR_H_

#include <stdbool.h>
#include <stdint.h>

#define MODE_SWITCH_INDICATOR_FLASH_MS 240u

typedef struct {
    uint8_t flash_remaining_ms;
} ModeSwitchIndicatorState;

void mode_switch_indicator_init(ModeSwitchIndicatorState* state);
void mode_switch_indicator_trigger(ModeSwitchIndicatorState* state);
void mode_switch_indicator_tick_1ms(ModeSwitchIndicatorState* state);
bool mode_switch_indicator_flashing(const ModeSwitchIndicatorState* state);

#endif  // COMPOSER_MIDICV_UI_MODE_SWITCH_INDICATOR_H_

#ifndef COMPOSER_MIDICV_HARDWARE_MODE_SELECTOR_H_
#define COMPOSER_MIDICV_HARDWARE_MODE_SELECTOR_H_

#include <stdbool.h>
#include <stdint.h>

#define MODE_SELECTOR_NUM_POSITIONS 6u
#define MODE_SELECTOR_INVALID_POSITION 0xffu
#define MODE_SELECTOR_STABLE_SAMPLES 20u

#define MODE_SELECTOR_RAW_SW3_ACTIVE 0x01u
#define MODE_SELECTOR_RAW_SW4A_ACTIVE 0x02u
#define MODE_SELECTOR_RAW_SW4B_ACTIVE 0x04u

typedef struct {
    uint8_t stable_position;
    uint8_t candidate_position;
    uint8_t candidate_count;
    uint8_t raw_switch_state;
} ModeSelectorState;

void mode_selector_init(ModeSelectorState* selector);
uint8_t mode_selector_decode_switches(bool sw3_active,
                                      bool sw4a_active,
                                      bool sw4b_active);
void mode_selector_update_switches(ModeSelectorState* selector,
                                   bool sw3_active,
                                   bool sw4a_active,
                                   bool sw4b_active);
uint8_t mode_selector_position(const ModeSelectorState* selector);
uint8_t mode_selector_raw_switch_state(const ModeSelectorState* selector);

void mode_selector_hw_init(void);
void mode_selector_tick_1ms(ModeSelectorState* selector);

#endif  // COMPOSER_MIDICV_HARDWARE_MODE_SELECTOR_H_

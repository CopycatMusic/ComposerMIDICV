#include "hardware/mode_selector.h"

void mode_selector_init(ModeSelectorState* selector) {
    selector->stable_position = MODE_SELECTOR_INVALID_POSITION;
    selector->candidate_position = MODE_SELECTOR_INVALID_POSITION;
    selector->candidate_count = 0;
    selector->raw_switch_state = 0;
}

uint8_t mode_selector_decode_switches(bool sw3_active,
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

void mode_selector_update_switches(ModeSelectorState* selector,
                                   bool sw3_active,
                                   bool sw4a_active,
                                   bool sw4b_active) {
    selector->raw_switch_state = 0;
    if (sw3_active) {
        selector->raw_switch_state |= MODE_SELECTOR_RAW_SW3_ACTIVE;
    }
    if (sw4a_active) {
        selector->raw_switch_state |= MODE_SELECTOR_RAW_SW4A_ACTIVE;
    }
    if (sw4b_active) {
        selector->raw_switch_state |= MODE_SELECTOR_RAW_SW4B_ACTIVE;
    }

    const uint8_t position =
        mode_selector_decode_switches(sw3_active, sw4a_active, sw4b_active);

    if (position == selector->candidate_position) {
        if (selector->candidate_count < MODE_SELECTOR_STABLE_SAMPLES) {
            ++selector->candidate_count;
        }
    } else {
        selector->candidate_position = position;
        selector->candidate_count = 1;
    }

    if (selector->candidate_count >= MODE_SELECTOR_STABLE_SAMPLES) {
        selector->stable_position = selector->candidate_position;
    }
}

uint8_t mode_selector_position(const ModeSelectorState* selector) {
    return selector->stable_position;
}

uint8_t mode_selector_raw_switch_state(const ModeSelectorState* selector) {
    return selector->raw_switch_state;
}

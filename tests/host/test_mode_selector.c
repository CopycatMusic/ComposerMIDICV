#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "hardware/mode_selector.h"

static void update_switches(ModeSelectorState* selector,
                            bool sw3_active,
                            bool sw4a_active,
                            bool sw4b_active,
                            uint8_t count) {
    for (uint8_t i = 0; i < count; ++i) {
        mode_selector_update_switches(selector, sw3_active, sw4a_active,
                                      sw4b_active);
    }
}

static void test_switch_decode_has_six_positions(void) {
    assert(mode_selector_decode_switches(false, true, false) == 0);
    assert(mode_selector_decode_switches(false, false, false) == 1);
    assert(mode_selector_decode_switches(false, false, true) == 2);
    assert(mode_selector_decode_switches(true, true, false) == 3);
    assert(mode_selector_decode_switches(true, false, false) == 4);
    assert(mode_selector_decode_switches(true, false, true) == 5);
    assert(mode_selector_decode_switches(false, true, true) ==
           MODE_SELECTOR_INVALID_POSITION);
}

static void test_position_changes_only_after_stable_samples(void) {
    ModeSelectorState selector;
    mode_selector_init(&selector);

    update_switches(&selector, true, true, false,
                    MODE_SELECTOR_STABLE_SAMPLES - 1u);
    assert(mode_selector_position(&selector) == MODE_SELECTOR_INVALID_POSITION);

    mode_selector_update_switches(&selector, true, true, false);
    assert(mode_selector_position(&selector) == 3);
    assert(mode_selector_raw_switch_state(&selector) ==
           (MODE_SELECTOR_RAW_SW3_ACTIVE | MODE_SELECTOR_RAW_SW4A_ACTIVE));
}

static void test_noise_does_not_immediately_change_stable_position(void) {
    ModeSelectorState selector;
    mode_selector_init(&selector);

    update_switches(&selector, true, true, false,
                    MODE_SELECTOR_STABLE_SAMPLES);
    assert(mode_selector_position(&selector) == 3);

    mode_selector_update_switches(&selector, true, false, true);
    assert(mode_selector_position(&selector) == 3);

    update_switches(&selector, true, false, true,
                    MODE_SELECTOR_STABLE_SAMPLES);
    assert(mode_selector_position(&selector) == 5);
}

int main(void) {
    test_switch_decode_has_six_positions();
    test_position_changes_only_after_stable_samples();
    test_noise_does_not_immediately_change_stable_position();
    return 0;
}

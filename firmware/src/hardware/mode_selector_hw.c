#include "hardware/mode_selector.h"

#include "hardware/board_pins.h"
#include "hardware/gpio.h"

static void init_selector_pin(uint pin) {
    gpio_init(pin);
    gpio_set_function(pin, GPIO_FUNC_SIO);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

void mode_selector_hw_init(void) {
    init_selector_pin(PIN_MODE_SELECTOR_SW3);
    init_selector_pin(PIN_MODE_SELECTOR_SW4A);
    init_selector_pin(PIN_MODE_SELECTOR_SW4B);
}

void mode_selector_tick_1ms(ModeSelectorState* selector) {
    mode_selector_update_switches(selector,
                                  !gpio_get(PIN_MODE_SELECTOR_SW3),
                                  !gpio_get(PIN_MODE_SELECTOR_SW4A),
                                  !gpio_get(PIN_MODE_SELECTOR_SW4B));
}

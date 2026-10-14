#include <assert.h>
#include <stdint.h>

#include "hardware/led.h"

int main(void) {
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    led_compensate_rgb(&r, &g, &b);
    assert(r == 61);
    assert(g == 46);
    assert(b == 56);

    r = 100;
    g = 100;
    b = 100;
    led_compensate_rgb(&r, &g, &b);
    assert(r == 24);
    assert(g == 18);
    assert(b == 22);

    return 0;
}

#ifndef COMPOSER_MIDICV_HARDWARE_LED_H_
#define COMPOSER_MIDICV_HARDWARE_LED_H_

#include <stdbool.h>
#include <stdint.h>

void led_init(void);
void led_set_rgb(bool r, bool g, bool b);
void led_set_rgb_u8(uint8_t r, uint8_t g, uint8_t b);
void led_off(void);
void led_set_ready(void);
void led_set_error(void);
void led_set_calibration(void);
void led_flash_midi(void);
void led_tick_1ms(void);

#endif  // COMPOSER_MIDICV_HARDWARE_LED_H_

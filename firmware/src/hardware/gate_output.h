#ifndef COMPOSER_MIDICV_HARDWARE_GATE_OUTPUT_H_
#define COMPOSER_MIDICV_HARDWARE_GATE_OUTPUT_H_

#include <stdbool.h>
#include <stdint.h>

void gate_output_init(void);
void gate_output_apply(const bool gate[4], const uint16_t audio_hz[4]);
void gate_output_set(uint8_t index, bool high);
void gate_output_all_low(void);

#endif  // COMPOSER_MIDICV_HARDWARE_GATE_OUTPUT_H_

#ifndef COMPOSER_MIDICV_HARDWARE_DAC_MCP4822_H_
#define COMPOSER_MIDICV_HARDWARE_DAC_MCP4822_H_

#include <stdint.h>

void dac_mcp4822_init(void);
void dac_mcp4822_write_channel(uint8_t channel, uint16_t code);
void dac_mcp4822_write_all(const uint16_t code[4]);
void dac_mcp4822_all_zero(void);

#endif  // COMPOSER_MIDICV_HARDWARE_DAC_MCP4822_H_

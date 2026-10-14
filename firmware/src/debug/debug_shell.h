#ifndef COMPOSER_MIDICV_DEBUG_DEBUG_SHELL_H_
#define COMPOSER_MIDICV_DEBUG_DEBUG_SHELL_H_

#include <stdbool.h>
#include <stdint.h>

#include "app/output_state.h"
#include "hardware/mode_selector.h"
#include "midi/midi_router.h"
#include "music/calibration.h"
#include "ui/led_feedback.h"

#define DEBUG_SHELL_LED_CYCLE_DEFAULT_MS 1000u

typedef struct {
    char line[96];
    uint8_t length;
    uint8_t gate_pulse_ms[4];
    bool raw_override[4];
    uint16_t raw_code[4];
    bool led_override_enabled;
    bool led_cycle_enabled;
    uint8_t led_cycle_index;
    uint16_t led_cycle_interval_ms;
    uint16_t led_cycle_elapsed_ms;
    LedRgb led_override_rgb;
} DebugShell;

typedef struct {
    OutputState* output;
    CalibrationState* calibration;
    MidiRouter* router;
    ModeSelectorState* selector;
} DebugShellContext;

void debug_shell_init(DebugShell* shell);
void debug_shell_poll(DebugShell* shell, DebugShellContext* context);
void debug_shell_tick_1ms(DebugShell* shell, OutputState* output);
uint16_t debug_shell_output_code(const DebugShell* shell,
                                 const CalibrationState* calibration,
                                 uint8_t cv_index,
                                 uint16_t target_mv);
bool debug_shell_led_override_active(const DebugShell* shell);
LedRgb debug_shell_led_rgb(const DebugShell* shell);
void debug_shell_clear_raw_overrides(DebugShell* shell);

#endif  // COMPOSER_MIDICV_DEBUG_DEBUG_SHELL_H_

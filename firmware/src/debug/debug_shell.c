#include "debug/debug_shell.h"

#include <stdlib.h>
#include <string.h>

#include "debug/debug_log.h"
#include "tusb.h"

static const LedRgb debug_led_cycle_palette[] = {
    {239, 59, 92},
    {6, 214, 86},
    {20, 48, 204},
    {255, 217, 59},
    {255, 255, 255},
    {255, 0, 0},
};

static const uint8_t debug_led_cycle_palette_count =
    sizeof(debug_led_cycle_palette) / sizeof(debug_led_cycle_palette[0]);

static uint16_t clamp_output_mv(unsigned long value) {
    if (value > COMPOSER_MIDICV_CV_MAX_MV) {
        return COMPOSER_MIDICV_CV_MAX_MV;
    }
    return (uint16_t)value;
}

static uint16_t clamp_dac_code(unsigned long value) {
    if (value > COMPOSER_MIDICV_DAC_MAX_CODE) {
        return COMPOSER_MIDICV_DAC_MAX_CODE;
    }
    return (uint16_t)value;
}

static int parse_index_1_to_4(const char* token) {
    if (!token) {
        return -1;
    }
    const long value = strtol(token, NULL, 10);
    if (value < 1 || value > 4) {
        return -1;
    }
    return (int)value - 1;
}

static bool parse_u8_token(const char* token, uint8_t* value) {
    if (!token) {
        return false;
    }
    char* end = NULL;
    const unsigned long parsed = strtoul(token, &end, 10);
    if (*token == '\0' || *end != '\0' || parsed > 255ul) {
        return false;
    }
    *value = (uint8_t)parsed;
    return true;
}

static void command_status(DebugShellContext* context) {
    const Mode* mode = midi_router_active_mode(context->router);
    const uint8_t mode_index = midi_router_active_mode_index(context->router);
    const uint8_t selector_position =
        context->selector ? mode_selector_position(context->selector)
                          : MODE_SELECTOR_INVALID_POSITION;
    const uint8_t selector_raw =
        context->selector ? mode_selector_raw_switch_state(context->selector)
                          : 0u;
    debug_logf("status mode=%s index=%u selector=%u raw=0x%02x cv=%u,%u,%u,%u gate=%u,%u,%u,%u audio_hz=%u,%u,%u,%u cal=%s",
               mode ? mode->name : "none",
               mode_index,
               selector_position,
               selector_raw,
               context->output->cv_mv[0],
               context->output->cv_mv[1],
               context->output->cv_mv[2],
               context->output->cv_mv[3],
               context->output->gate[0] ? 1 : 0,
               context->output->gate[1] ? 1 : 0,
               context->output->gate[2] ? 1 : 0,
               context->output->gate[3] ? 1 : 0,
               context->output->audio_hz[0],
               context->output->audio_hz[1],
               context->output->audio_hz[2],
               context->output->audio_hz[3],
               context->calibration->loaded_from_flash ? "flash" : "default");
}

static void command_cal_show(DebugShellContext* context) {
    for (uint8_t i = 0; i < 4; ++i) {
        const CvCalibration* cv = &context->calibration->cv[i];
        debug_logf("cv%u p0=%u/%u p1=%u/%u",
                   i + 1,
                   cv->point[0].target_mv,
                   cv->point[0].measured_mv,
                   cv->point[1].target_mv,
                   cv->point[1].measured_mv);
    }
}

static void command_led(DebugShell* shell) {
    char* arg = strtok(NULL, " ");
    if (!arg) {
        debug_log("err usage: led auto|off|cycle [ms]|<r> <g> <b>");
        return;
    }

    if (strcmp(arg, "auto") == 0) {
        shell->led_override_enabled = false;
        shell->led_cycle_enabled = false;
        debug_log("ok led auto");
        return;
    }

    shell->led_override_enabled = true;

    if (strcmp(arg, "off") == 0) {
        shell->led_cycle_enabled = false;
        shell->led_override_rgb = (LedRgb){0, 0, 0};
        debug_log("ok led off");
        return;
    }

    if (strcmp(arg, "cycle") == 0) {
        char* interval_token = strtok(NULL, " ");
        uint16_t interval_ms = DEBUG_SHELL_LED_CYCLE_DEFAULT_MS;
        if (interval_token) {
            const unsigned long parsed = strtoul(interval_token, NULL, 10);
            if (parsed < 20ul || parsed > 60000ul) {
                debug_log("err usage: led cycle [20-60000ms]");
                return;
            }
            interval_ms = (uint16_t)parsed;
        }

        shell->led_cycle_enabled = true;
        shell->led_cycle_index = 0;
        shell->led_cycle_elapsed_ms = 0;
        shell->led_cycle_interval_ms = interval_ms;
        shell->led_override_rgb = debug_led_cycle_palette[0];
        debug_log("ok led cycle");
        return;
    }

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    if (!parse_u8_token(arg, &r) ||
        !parse_u8_token(strtok(NULL, " "), &g) ||
        !parse_u8_token(strtok(NULL, " "), &b)) {
        debug_log("err usage: led auto|off|cycle [ms]|<r> <g> <b>");
        return;
    }

    shell->led_cycle_enabled = false;
    shell->led_override_rgb = (LedRgb){r, g, b};
    debug_log("ok led");
}

static void execute_line(DebugShell* shell,
                         DebugShellContext* context,
                         char* line) {
    char* command = strtok(line, " ");
    if (!command) {
        return;
    }

    if (strcmp(command, "status") == 0) {
        command_status(context);
    } else if (strcmp(command, "led") == 0) {
        command_led(shell);
    } else if (strcmp(command, "mute") == 0) {
        output_state_clear(context->output);
        debug_shell_clear_raw_overrides(shell);
        debug_log("ok mute");
    } else if (strcmp(command, "test_gate") == 0) {
        const int index = parse_index_1_to_4(strtok(NULL, " "));
        if (index < 0) {
            debug_log("err usage: test_gate <1-4>");
            return;
        }
        context->output->gate[index] = true;
        shell->gate_pulse_ms[index] = 100;
        debug_log("ok test_gate");
    } else if (strcmp(command, "test_cv") == 0) {
        const int index = parse_index_1_to_4(strtok(NULL, " "));
        char* mv_token = strtok(NULL, " ");
        if (index < 0 || !mv_token) {
            debug_log("err usage: test_cv <1-4> <mv>");
            return;
        }
        context->output->cv_mv[index] =
            clamp_output_mv(strtoul(mv_token, NULL, 10));
        shell->raw_override[index] = false;
        debug_log("ok test_cv");
    } else if (strcmp(command, "cal") == 0) {
        char* sub = strtok(NULL, " ");
        if (!sub) {
            debug_log("err usage: cal show|reset|raw|set|save");
        } else if (strcmp(sub, "show") == 0) {
            command_cal_show(context);
        } else if (strcmp(sub, "reset") == 0) {
            calibration_reset_defaults(context->calibration);
            debug_shell_clear_raw_overrides(shell);
            debug_log("ok cal reset");
        } else if (strcmp(sub, "save") == 0) {
            calibration_save(context->calibration);
            debug_log("ok cal save");
        } else if (strcmp(sub, "raw") == 0) {
            const int index = parse_index_1_to_4(strtok(NULL, " "));
            char* code_token = strtok(NULL, " ");
            if (index < 0 || !code_token) {
                debug_log("err usage: cal raw <cv> <code>");
                return;
            }
            shell->raw_override[index] = true;
            shell->raw_code[index] =
                clamp_dac_code(strtoul(code_token, NULL, 10));
            debug_log("ok cal raw");
        } else if (strcmp(sub, "set") == 0) {
            const int index = parse_index_1_to_4(strtok(NULL, " "));
            char* target_token = strtok(NULL, " ");
            char* measured_token = strtok(NULL, " ");
            if (index < 0 || !target_token || !measured_token) {
                debug_log("err usage: cal set <cv> <target_mv> <measured_mv>");
                return;
            }
            calibration_set_measurement(
                context->calibration,
                (uint8_t)index,
                clamp_output_mv(strtoul(target_token, NULL, 10)),
                clamp_output_mv(strtoul(measured_token, NULL, 10)));
            debug_log("ok cal set");
        } else {
            debug_log("err usage: cal show|reset|raw|set|save");
        }
    } else {
        debug_log("err unknown command");
    }
}

void debug_shell_init(DebugShell* shell) {
    memset(shell, 0, sizeof(*shell));
    shell->led_cycle_interval_ms = DEBUG_SHELL_LED_CYCLE_DEFAULT_MS;
}

void debug_shell_poll(DebugShell* shell, DebugShellContext* context) {
    while (tud_cdc_available()) {
        char ch;
        if (tud_cdc_read(&ch, 1) != 1) {
            return;
        }

        if (ch == '\r' || ch == '\n') {
            if (shell->length > 0) {
                shell->line[shell->length] = '\0';
                execute_line(shell, context, shell->line);
                shell->length = 0;
            }
            tud_cdc_write_str("> ");
            tud_cdc_write_flush();
        } else if (shell->length + 1 < sizeof(shell->line)) {
            shell->line[shell->length++] = ch;
        }
    }
}

void debug_shell_tick_1ms(DebugShell* shell, OutputState* output) {
    if (shell->led_override_enabled && shell->led_cycle_enabled) {
        ++shell->led_cycle_elapsed_ms;
        if (shell->led_cycle_elapsed_ms >= shell->led_cycle_interval_ms) {
            shell->led_cycle_elapsed_ms = 0;
            shell->led_cycle_index =
                (uint8_t)((shell->led_cycle_index + 1u) %
                          debug_led_cycle_palette_count);
            shell->led_override_rgb =
                debug_led_cycle_palette[shell->led_cycle_index];
        }
    }

    for (uint8_t i = 0; i < 4; ++i) {
        if (shell->gate_pulse_ms[i] == 0) {
            continue;
        }
        --shell->gate_pulse_ms[i];
        if (shell->gate_pulse_ms[i] == 0) {
            output->gate[i] = false;
        }
    }
}

uint16_t debug_shell_output_code(const DebugShell* shell,
                                 const CalibrationState* calibration,
                                 uint8_t cv_index,
                                 uint16_t target_mv) {
    if (cv_index < 4 && shell->raw_override[cv_index]) {
        return shell->raw_code[cv_index];
    }
    return calibration_apply_mv(calibration, cv_index, target_mv);
}

bool debug_shell_led_override_active(const DebugShell* shell) {
    return shell->led_override_enabled;
}

LedRgb debug_shell_led_rgb(const DebugShell* shell) {
    return shell->led_override_rgb;
}

void debug_shell_clear_raw_overrides(DebugShell* shell) {
    for (uint8_t i = 0; i < 4; ++i) {
        shell->raw_override[i] = false;
        shell->raw_code[i] = 0;
    }
}

#include "debug/debug_shell.h"

#include <stdlib.h>
#include <string.h>

#include "debug/debug_log.h"
#include "tusb.h"

static uint16_t clamp_u16_4095(unsigned long value) {
    if (value > 4095ul) {
        return 4095;
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

static void command_status(DebugShellContext* context) {
    const Mode* mode = midi_router_active_mode(context->router);
    debug_logf("status mode=%s cv=%u,%u,%u,%u gate=%u,%u,%u,%u audio_hz=%u,%u,%u,%u cal=%s",
               mode ? mode->name : "none",
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

static void execute_line(DebugShell* shell,
                         DebugShellContext* context,
                         char* line) {
    char* command = strtok(line, " ");
    if (!command) {
        return;
    }

    if (strcmp(command, "status") == 0) {
        command_status(context);
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
        context->output->cv_mv[index] = clamp_u16_4095(strtoul(mv_token, NULL, 10));
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
            shell->raw_code[index] = clamp_u16_4095(strtoul(code_token, NULL, 10));
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
                clamp_u16_4095(strtoul(target_token, NULL, 10)),
                clamp_u16_4095(strtoul(measured_token, NULL, 10)));
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

void debug_shell_clear_raw_overrides(DebugShell* shell) {
    for (uint8_t i = 0; i < 4; ++i) {
        shell->raw_override[i] = false;
        shell->raw_code[i] = 0;
    }
}

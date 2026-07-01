#include "app/app.h"

#include "app/output_state.h"
#include "debug/debug_log.h"
#include "debug/debug_shell.h"
#include "hardware/dac_mcp4822.h"
#include "hardware/gate_output.h"
#include "hardware/led.h"
#include "midi/midi_event.h"
#include "midi/midi_router.h"
#include "midi/usb_midi.h"
#include "modes/mode_cc_convert.h"
#include "modes/mode_drum4_velocity.h"
#include "modes/mode_drum8_trigger.h"
#include "modes/mode_dual_mono.h"
#include "modes/mode_mono_perf.h"
#include "modes/mode_mono_random_clock.h"
#include "modes/mode_poly4.h"
#include "modes/mode_stub.h"
#include "music/calibration.h"
#include "ui/led_feedback.h"
#include "bsp/board_api.h"
#include "pico/stdlib.h"
#include "tusb.h"

static OutputState output_state;
static CalibrationState calibration_state;
static MidiRouter midi_router;
static DebugShell debug_shell;
static LedFeedbackState led_feedback_state;

static ModeMonoPerfState mode_mono_perf_state;
static ModeMonoRandomClockState mode_mono_random_clock_state;
static ModeDualMonoState mode_dual_mono_state;
static ModePoly4State mode_poly4_state;
static ModeDrum4VelocityState mode_drum4_velocity_state;
static ModeDrum8TriggerState mode_drum8_trigger_state;
static ModeCcConvertState mode_cc_convert_state;

static uint32_t last_tick_ms;

static void register_modes(void) {
    midi_router_register_mode(&midi_router, 0,
                              mode_mono_perf_create(&mode_mono_perf_state));
    midi_router_register_mode(&midi_router, 1,
                              mode_mono_random_clock_create(
                                  &mode_mono_random_clock_state));
    midi_router_register_mode(&midi_router, 2,
                              mode_dual_mono_create(&mode_dual_mono_state));
    midi_router_register_mode(&midi_router, 3,
                              mode_poly4_create(&mode_poly4_state));
    midi_router_register_mode(&midi_router, 4,
                              mode_drum4_velocity_create(
                                  &mode_drum4_velocity_state));
    midi_router_register_mode(&midi_router, 5,
                              mode_drum8_trigger_create(
                                  &mode_drum8_trigger_state));
    midi_router_register_mode(&midi_router, 6,
                              mode_cc_convert_create(&mode_cc_convert_state));
}

static void apply_outputs(void) {
    uint16_t dac_code[4];
    for (uint8_t i = 0; i < 4; ++i) {
        dac_code[i] = debug_shell_output_code(&debug_shell,
                                              &calibration_state,
                                              i,
                                              output_state.cv_mv[i]);
    }

    dac_mcp4822_write_all(dac_code);
    gate_output_apply(output_state.gate, output_state.audio_hz);
}

static void tick_1ms(void) {
    midi_router_tick_1ms(&midi_router);
    debug_shell_tick_1ms(&debug_shell, &output_state);
    led_feedback_tick_1ms(&led_feedback_state);
    const LedRgb led_rgb = led_feedback_rgb(&led_feedback_state);
    led_set_rgb_u8(led_rgb.r, led_rgb.g, led_rgb.b);
    led_tick_1ms();
    apply_outputs();
}

void app_init(void) {
    board_init();
    led_init();
    gate_output_init();
    dac_mcp4822_init();

    output_state_clear(&output_state);
    calibration_init(&calibration_state);
    midi_router_init(&midi_router, &output_state);
    register_modes();
    debug_shell_init(&debug_shell);
    led_feedback_init(&led_feedback_state);

    tusb_init();
    const LedRgb led_rgb = led_feedback_rgb(&led_feedback_state);
    led_set_rgb_u8(led_rgb.r, led_rgb.g, led_rgb.b);
    last_tick_ms = to_ms_since_boot(get_absolute_time());
}

void app_run(void) {
    while (true) {
        tud_task();

        MidiEvent event;
        while (usb_midi_read_event(&event)) {
            midi_router_handle_event(&midi_router, &event);
            led_feedback_handle_midi(&led_feedback_state, &event);
        }

        DebugShellContext shell_context = {
            .output = &output_state,
            .calibration = &calibration_state,
            .router = &midi_router,
        };
        debug_shell_poll(&debug_shell, &shell_context);

        const uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        while ((uint32_t)(now_ms - last_tick_ms) >= 1u) {
            ++last_tick_ms;
            tick_1ms();
        }

        tight_loop_contents();
    }
}

#include "app/app.h"

#include "app/output_state.h"
#include "config/mode_config.h"
#include "debug/debug_log.h"
#include "debug/debug_shell.h"
#include "hardware/dac_mcp4822.h"
#include "hardware/gate_output.h"
#include "hardware/led.h"
#include "hardware/mode_selector.h"
#include "midi/midi_event.h"
#include "midi/midi_router.h"
#include "midi/trs_midi.h"
#include "midi/usb_midi.h"
#include "modes/mode_clocked_chord_arp.h"
#include "modes/mode_clocked_note.h"
#include "modes/mode_dual_mono.h"
#include "modes/mode_mono_perf.h"
#include "modes/mode_placeholder_test.h"
#include "modes/mode_poly4.h"
#include "modes/mode_stub.h"
#include "music/calibration.h"
#include "ui/led_chaser.h"
#include "ui/led_feedback.h"
#include "ui/mode_switch_indicator.h"
#include "bsp/board_api.h"
#include "pico/stdlib.h"
#include "tusb.h"

static OutputState output_state;
static CalibrationState calibration_state;
static MidiRouter midi_router;
static DebugShell debug_shell;
static LedFeedbackState led_feedback_state;
static LedChaserState led_chaser_state;
static ModeSwitchIndicatorState mode_switch_indicator_state;
static ModeSelectorState mode_selector_state;
static bool led_chaser_active;

static ModeMonoPerfState mode_mono_perf_state;
static ModeDualMonoState mode_dual_mono_state;
static ModePoly4State mode_poly4_state;
static ModeClockedNoteState mode_clocked_note_state[3];
static ModeClockedChordArpState mode_clocked_chord_arp_state;
static ModePlaceholderTestState mode_placeholder_test_state[3];

static uint32_t last_tick_ms;

static void register_modes(void) {
    uint8_t clocked_note_count = 0;

    for (uint8_t slot = 0; slot < mode_config_num_slots(); ++slot) {
        const ModeSlotConfig* config = mode_config_default_slot(slot);
        switch (config->engine) {
            case MODE_ENGINE_MONO_MIDI:
                midi_router_register_mode(
                    &midi_router, slot,
                    mode_mono_perf_create(&mode_mono_perf_state));
                break;

            case MODE_ENGINE_DUAL_MIDI:
                midi_router_register_mode(
                    &midi_router, slot,
                    mode_dual_mono_create(&mode_dual_mono_state));
                break;

            case MODE_ENGINE_POLY4_MIDI:
                midi_router_register_mode(
                    &midi_router, slot,
                    mode_poly4_create(&mode_poly4_state));
                break;

            case MODE_ENGINE_CLOCKED_NOTE:
                if (clocked_note_count <
                    (sizeof(mode_clocked_note_state) /
                     sizeof(mode_clocked_note_state[0]))) {
                    midi_router_register_mode(
                        &midi_router, slot,
                        mode_clocked_note_create_from_config(
                            &mode_clocked_note_state[clocked_note_count],
                            config));
                    ++clocked_note_count;
                } else {
                    midi_router_register_mode(
                        &midi_router, slot, mode_stub_create(config->name));
                }
                break;

            case MODE_ENGINE_CLOCKED_CHORD_ARP:
                midi_router_register_mode(
                    &midi_router, slot,
                    mode_clocked_chord_arp_create(
                        &mode_clocked_chord_arp_state));
                break;

            case MODE_ENGINE_PLACEHOLDER_TEST:
                midi_router_register_mode(
                    &midi_router, slot,
                    mode_placeholder_test_create(
                        &mode_placeholder_test_state[slot - 3u],
                        config->name));
                break;

            case MODE_ENGINE_LED_CHASER:
            case MODE_ENGINE_RESERVED:
            default:
                midi_router_register_mode(&midi_router, slot,
                                          mode_stub_create(config->name));
                break;
        }
    }
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

static bool active_mode_uses_led_chaser(void) {
    const uint8_t mode_index = midi_router_active_mode_index(&midi_router);
    const ModeSlotConfig* config = mode_config_default_slot(mode_index);
    return config != NULL &&
           (config->engine == MODE_ENGINE_LED_CHASER ||
            config->engine == MODE_ENGINE_PLACEHOLDER_TEST);
}

static void apply_led_feedback(void) {
    if (mode_switch_indicator_flashing(&mode_switch_indicator_state)) {
        led_set_rgb_u8(128, 128, 128);
        mode_switch_indicator_tick_1ms(&mode_switch_indicator_state);
        return;
    }

    if (debug_shell_led_override_active(&debug_shell)) {
        const LedRgb led_rgb = debug_shell_led_rgb(&debug_shell);
        led_set_rgb_u8(led_rgb.r, led_rgb.g, led_rgb.b);
        return;
    }

    if (active_mode_uses_led_chaser()) {
        if (!led_chaser_active) {
            led_chaser_init(&led_chaser_state);
            led_chaser_active = true;
        }
        led_chaser_tick_1ms(&led_chaser_state);
        const LedRgb led_rgb = led_chaser_rgb(&led_chaser_state);
        led_set_rgb_u8(led_rgb.r, led_rgb.g, led_rgb.b);
        return;
    }

    led_chaser_active = false;
    led_feedback_tick_1ms(&led_feedback_state);
    const LedRgb led_rgb = led_feedback_rgb(&led_feedback_state);
    led_set_rgb_u8(led_rgb.r, led_rgb.g, led_rgb.b);
}

static void tick_1ms(void) {
    mode_selector_tick_1ms(&mode_selector_state);
    const uint8_t selector_position = mode_selector_position(&mode_selector_state);
    if (selector_position != MODE_SELECTOR_INVALID_POSITION) {
        const uint8_t previous_mode =
            midi_router_active_mode_index(&midi_router);
        if (midi_router_set_active_mode(&midi_router, selector_position) &&
            previous_mode != selector_position) {
            const ModeSlotConfig* config =
                mode_config_default_slot(selector_position);
            led_feedback_set_mode(&led_feedback_state,
                                  config ? config->engine
                                         : MODE_ENGINE_RESERVED);
            mode_switch_indicator_trigger(&mode_switch_indicator_state);
        }
    }

    midi_router_tick_1ms(&midi_router);
    debug_shell_tick_1ms(&debug_shell, &output_state);
    apply_led_feedback();
    led_tick_1ms();
    apply_outputs();
}

void app_init(void) {
    board_init();
    led_init();
    gate_output_init();
    dac_mcp4822_init();
    mode_selector_hw_init();
    trs_midi_init();

    output_state_clear(&output_state);
    calibration_init(&calibration_state);
    midi_router_init(&midi_router, &output_state);
    register_modes();
    midi_router_set_active_mode(&midi_router, 0);
    debug_shell_init(&debug_shell);
    led_feedback_init(&led_feedback_state);
    const ModeSlotConfig* initial_config = mode_config_default_slot(0);
    led_feedback_set_mode(&led_feedback_state,
                          initial_config ? initial_config->engine
                                         : MODE_ENGINE_RESERVED);
    led_chaser_init(&led_chaser_state);
    mode_switch_indicator_init(&mode_switch_indicator_state);
    mode_selector_init(&mode_selector_state);
    led_chaser_active = false;

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
        while (trs_midi_read_event(tud_mounted(), &event)) {
            midi_router_handle_event(&midi_router, &event);
            led_feedback_handle_midi(&led_feedback_state, &event);
        }

        DebugShellContext shell_context = {
            .output = &output_state,
            .calibration = &calibration_state,
            .router = &midi_router,
            .selector = &mode_selector_state,
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

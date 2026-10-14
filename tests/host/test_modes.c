#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "app/output_state.h"
#include "config/mode_config.h"
#include "midi/midi_event.h"
#include "midi/midi_router.h"
#include "modes/mode_clocked_chord_arp.h"
#include "modes/mode_clocked_note.h"
#include "modes/mode_stub.h"
#include "modes/mode_drum4_velocity.h"
#include "modes/mode_drum8_trigger.h"
#include "modes/mode_dual_mono.h"
#include "modes/mode_mono_perf.h"
#include "modes/mode_mono_random_clock.h"
#include "modes/mode_placeholder_test.h"
#include "modes/mode_poly4.h"

static MidiEvent note_on(uint8_t note, uint8_t velocity) {
    MidiEvent event = {
        .type = MIDI_EVENT_NOTE_ON,
        .channel = 0,
        .data1 = note,
        .data2 = velocity,
    };
    return event;
}

static MidiEvent note_on_channel(uint8_t channel,
                                 uint8_t note,
                                 uint8_t velocity) {
    MidiEvent event = note_on(note, velocity);
    event.channel = channel;
    return event;
}

static MidiEvent note_on_from(uint8_t note,
                              uint8_t velocity,
                              MidiSource source,
                              bool usb_connected) {
    MidiEvent event = note_on(note, velocity);
    event.source = source;
    event.usb_connected = usb_connected;
    return event;
}

static MidiEvent note_off(uint8_t note) {
    MidiEvent event = {
        .type = MIDI_EVENT_NOTE_OFF,
        .channel = 0,
        .data1 = note,
    };
    return event;
}

static MidiEvent note_off_channel(uint8_t channel, uint8_t note) {
    MidiEvent event = note_off(note);
    event.channel = channel;
    return event;
}

static MidiEvent cc(uint8_t controller, uint8_t value) {
    MidiEvent event = {
        .type = MIDI_EVENT_CONTROL_CHANGE,
        .channel = 0,
        .data1 = controller,
        .data2 = value,
    };
    return event;
}

static MidiEvent realtime(uint8_t byte) {
    MidiEvent event = {
        .type = MIDI_EVENT_REALTIME,
        .channel = 0xff,
        .realtime_byte = byte,
    };
    return event;
}

static MidiEvent pitch_bend(int16_t bend) {
    MidiEvent event = {
        .type = MIDI_EVENT_PITCH_BEND,
        .channel = 0,
        .pitch_bend = bend,
    };
    return event;
}

static MidiEvent pitch_bend_channel(uint8_t channel, int16_t bend) {
    MidiEvent event = pitch_bend(bend);
    event.channel = channel;
    return event;
}

static void tick_ms(const Mode* mode, uint16_t count, OutputState* output) {
    for (uint16_t i = 0; i < count; ++i) {
        mode->tick_1ms(mode->state, output);
    }
}

static void test_mode2_random_and_clock(void) {
    OutputState output;
    ModeMonoRandomClockState state;
    Mode mode = mode_mono_random_clock_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on(48, 100);
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[0] == 1000);
    assert(output.cv_mv[1] > 3000);
    assert(output.cv_mv[2] != 0);
    assert(output.cv_mv[3] != 0);
    assert(output.gate[0]);
    assert(output.gate[1]);
    assert(output.audio_hz[2] == 131);
    assert(output.audio_hz[3] == 262);

    tick_ms(&mode, 10, &output);
    assert(!output.gate[1]);

    event = realtime(0xfa);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.audio_hz[2] == 131);
    assert(output.audio_hz[3] == 262);

    event = pitch_bend(8191);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.audio_hz[2] == 147);
    assert(output.audio_hz[3] == 294);

    event = pitch_bend(-8192);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.audio_hz[2] == 117);
    assert(output.audio_hz[3] == 234);

    event = note_off(48);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.audio_hz[2] == 0);
    assert(output.audio_hz[3] == 0);
}

static void test_mode1_mono_midi_allocation(void) {
    OutputState output;
    ModeMonoPerfState state;
    Mode mode = mode_mono_perf_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = cc(1, 64);
    mode.handle_midi(mode.state, &event, &output);
    event = cc(11, 127);
    mode.handle_midi(mode.state, &event, &output);
    event = note_on(48, 100);
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[0] == 1000);
    assert(output.cv_mv[1] > 3000);
    assert(output.cv_mv[2] > 4000);
    assert(output.cv_mv[2] < 4100);
    assert(output.cv_mv[3] != 8000);
    assert(output.gate[0]);
    assert(output.gate[1]);
    assert(output.gate[2]);
    assert(output.gate[3]);

    tick_ms(&mode, 10, &output);
    assert(output.gate[0]);
    assert(output.gate[1]);
    assert(!output.gate[2]);
    assert(!output.gate[3]);
}

static void test_mode2_dual_midi_allocation(void) {
    OutputState output;
    ModeDualMonoState state;
    Mode mode = mode_dual_mono_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on_channel(0, 48, 100);
    mode.handle_midi(mode.state, &event, &output);
    event = note_on_channel(8, 60, 100);
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[0] == 1000);
    assert(output.cv_mv[1] > 3000);
    assert(output.cv_mv[2] == 2000);
    assert(output.cv_mv[3] > 3000);
    assert(output.gate[0]);
    assert(output.gate[1]);
    assert(output.gate[2]);
    assert(output.gate[3]);

    tick_ms(&mode, 10, &output);
    assert(output.gate[0]);
    assert(!output.gate[1]);
    assert(output.gate[2]);
    assert(!output.gate[3]);

    event = cc(1, 127);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.cv_mv[1] > 3000);
    assert(output.cv_mv[3] > 3000);

    event = note_off_channel(0, 48);
    mode.handle_midi(mode.state, &event, &output);
    assert(!output.gate[0]);
    assert(output.gate[2]);
}

static void test_mode2_dual_midi_routes_channel_ranges(void) {
    OutputState output;
    ModeDualMonoState state;
    Mode mode = mode_dual_mono_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on_channel(0, 48, 100);
    mode.handle_midi(mode.state, &event, &output);
    event = note_on_channel(7, 52, 100);
    mode.handle_midi(mode.state, &event, &output);
    event = note_on_channel(8, 60, 100);
    mode.handle_midi(mode.state, &event, &output);
    event = note_on_channel(15, 64, 100);
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[0] == 1333);
    assert(output.cv_mv[2] == 2333);
    assert(output.gate[0]);
    assert(output.gate[2]);

    event = note_off_channel(0, 48);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.gate[0]);
    assert(output.gate[2]);

    event = note_off_channel(7, 52);
    mode.handle_midi(mode.state, &event, &output);
    assert(!output.gate[0]);
    assert(output.gate[2]);
}

static void test_mode2_uses_merged_dual_allocation_with_sources_present(void) {
    OutputState output;
    ModeDualMonoState state;
    Mode mode = mode_dual_mono_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on_from(48, 100, MIDI_SOURCE_TRS, true);
    event.channel = 0;
    mode.handle_midi(mode.state, &event, &output);
    event = note_on_from(60, 100, MIDI_SOURCE_USB, true);
    event.channel = 8;
    mode.handle_midi(mode.state, &event, &output);
    event = note_on_from(64, 100, MIDI_SOURCE_TRS, true);
    event.channel = 7;
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[0] == 2333);
    assert(output.cv_mv[2] == 2000);
    assert(output.gate[0]);
    assert(output.gate[2]);
}

static void test_mode3_poly4_midi_allocation(void) {
    OutputState output;
    ModePoly4State state;
    Mode mode = mode_poly4_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on_channel(0, 48, 100);
    mode.handle_midi(mode.state, &event, &output);
    event = note_on_channel(4, 52, 100);
    mode.handle_midi(mode.state, &event, &output);
    event = note_on_channel(8, 55, 100);
    mode.handle_midi(mode.state, &event, &output);
    event = note_on_channel(12, 60, 100);
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[0] == 1000);
    assert(output.cv_mv[1] == 1333);
    assert(output.cv_mv[2] == 1583);
    assert(output.cv_mv[3] == 2000);
    assert(output.gate[0]);
    assert(output.gate[1]);
    assert(output.gate[2]);
    assert(output.gate[3]);
}

static void test_mode4_clocked_note_dual_pitch_and_transport(void) {
    OutputState output;
    ModeClockedNoteState state;
    Mode mode = mode_clocked_note_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on_channel(0, 60, 100);
    mode.handle_midi(mode.state, &event, &output);
    event = note_on_channel(8, 64, 80);
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[0] == 2000);
    assert(output.cv_mv[1] > 3000);
    assert(output.cv_mv[2] == 2333);
    assert(output.cv_mv[3] > 2500);
    assert(!output.gate[0]);

    event = realtime(0xfa);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.gate[0]);
    assert(output.gate[1]);
    tick_ms(&mode, 10, &output);
    assert(output.gate[0]);
    assert(!output.gate[1]);

    event = realtime(0xfc);
    mode.handle_midi(mode.state, &event, &output);
    assert(!output.gate[0]);
}

static void assert_clocked_note_clock_divisions(const ModeSlotConfig* config,
                                                uint8_t gate0_division,
                                                uint8_t gate1_division) {
    OutputState output;
    ModeClockedNoteState state;
    Mode mode = mode_clocked_note_create_from_config(&state, config);
    mode.reset(mode.state, &output);

    MidiEvent event = realtime(0xfa);
    mode.handle_midi(mode.state, &event, &output);
    tick_ms(&mode, 10, &output);

    for (uint8_t tick = 1; tick <= 24; ++tick) {
        event = realtime(0xf8);
        mode.handle_midi(mode.state, &event, &output);

        if ((tick % gate0_division) == 0u) {
            assert(output.gate[2]);
        } else {
            assert(!output.gate[2]);
        }
        if ((tick % gate1_division) == 0u) {
            assert(output.gate[3]);
        } else {
            assert(!output.gate[3]);
        }

        tick_ms(&mode, 10, &output);
        assert(!output.gate[2]);
        assert(!output.gate[3]);
    }

    event = realtime(0xfc);
    mode.handle_midi(mode.state, &event, &output);
    event = realtime(0xf8);
    mode.handle_midi(mode.state, &event, &output);
    assert(!output.gate[2]);
    assert(!output.gate[3]);
}

static void test_clocked_note_speed_profiles(void) {
    assert_clocked_note_clock_divisions(mode_config_default_slot(3), 24u, 3u);
    assert_clocked_note_clock_divisions(mode_config_default_slot(4), 12u, 2u);
    assert_clocked_note_clock_divisions(mode_config_default_slot(5), 6u, 1u);
}

static void test_mode4_run_restores_after_switching_back_while_playing(void) {
    OutputState output;
    MidiRouter router;
    ModeClockedNoteState clocked_note;

    midi_router_init(&router, &output);
    midi_router_register_mode(&router, 0, mode_stub_create("stub"));
    midi_router_register_mode(&router, 3,
                              mode_clocked_note_create(&clocked_note));

    assert(midi_router_set_active_mode(&router, 3));
    MidiEvent event = realtime(0xfa);
    assert(midi_router_handle_event(&router, &event));
    assert(output.gate[0]);

    assert(midi_router_set_active_mode(&router, 0));
    assert(!output.gate[0]);

    assert(midi_router_set_active_mode(&router, 3));
    assert(output.gate[0]);
    assert(!output.gate[1]);
}

static bool is_chord_pitch(uint16_t mv) {
    return mv == 1000 || mv == 1333 || mv == 1583;
}

static void add_test_chord(const Mode* mode, OutputState* output) {
    MidiEvent event = note_on(48, 100);
    mode->handle_midi(mode->state, &event, output);
    event = note_on(52, 100);
    mode->handle_midi(mode->state, &event, output);
    event = note_on(55, 100);
    mode->handle_midi(mode->state, &event, output);
}

static void test_mode5_clocked_chord_arp_divisions(void) {
    OutputState output;
    ModeClockedChordArpState state;
    Mode mode = mode_clocked_chord_arp_create(&state);
    mode.reset(mode.state, &output);

    add_test_chord(&mode, &output);

    MidiEvent event = realtime(0xfa);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.gate[0]);
    assert(output.gate[1]);
    tick_ms(&mode, 10, &output);
    assert(output.gate[0]);
    assert(!output.gate[1]);

    for (uint8_t tick = 1; tick <= 24; ++tick) {
        event = realtime(0xf8);
        mode.handle_midi(mode.state, &event, &output);

        if ((tick % 6u) == 0u) {
            assert(output.gate[3]);
            assert(is_chord_pitch(output.cv_mv[1]));
            assert(is_chord_pitch(output.cv_mv[3]));
        } else {
            assert(!output.gate[3]);
        }

        if ((tick % 24u) == 0u) {
            assert(output.gate[2]);
            assert(is_chord_pitch(output.cv_mv[0]));
            assert(is_chord_pitch(output.cv_mv[2]));
        } else {
            assert(!output.gate[2]);
        }

        tick_ms(&mode, 10, &output);
        assert(!output.gate[2]);
        assert(!output.gate[3]);
    }

    assert(output.cv_mv[0] == 1000);
    assert(output.cv_mv[1] == 1000);
}

static void test_mode5_clocked_chord_arp_clears_pitch_when_chord_released(void) {
    OutputState output;
    ModeClockedChordArpState state;
    Mode mode = mode_clocked_chord_arp_create(&state);
    mode.reset(mode.state, &output);

    add_test_chord(&mode, &output);
    MidiEvent event = realtime(0xfa);
    mode.handle_midi(mode.state, &event, &output);
    tick_ms(&mode, 10, &output);

    for (uint8_t tick = 0; tick < 24; ++tick) {
        event = realtime(0xf8);
        mode.handle_midi(mode.state, &event, &output);
        tick_ms(&mode, 10, &output);
    }
    assert(is_chord_pitch(output.cv_mv[0]));
    assert(is_chord_pitch(output.cv_mv[1]));
    assert(is_chord_pitch(output.cv_mv[2]));
    assert(is_chord_pitch(output.cv_mv[3]));

    event = note_off(48);
    mode.handle_midi(mode.state, &event, &output);
    event = note_off(52);
    mode.handle_midi(mode.state, &event, &output);
    event = note_off(55);
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[0] == 0);
    assert(output.cv_mv[1] == 0);
    assert(output.cv_mv[2] == 0);
    assert(output.cv_mv[3] == 0);
}

static void test_mode5_run_restores_after_switching_back_while_playing(void) {
    OutputState output;
    MidiRouter router;
    ModeClockedNoteState clocked_note;

    midi_router_init(&router, &output);
    midi_router_register_mode(&router, 0, mode_stub_create("stub"));
    midi_router_register_mode(&router, 4,
                              mode_clocked_note_create_from_config(
                                  &clocked_note,
                                  mode_config_default_slot(4)));

    assert(midi_router_set_active_mode(&router, 4));
    MidiEvent event = realtime(0xfa);
    assert(midi_router_handle_event(&router, &event));
    assert(output.gate[0]);

    assert(midi_router_set_active_mode(&router, 0));
    assert(!output.gate[0]);

    assert(midi_router_set_active_mode(&router, 4));
    assert(output.gate[0]);
    assert(!output.gate[1]);
}

static void test_mode5_drum_velocity(void) {
    OutputState output;
    ModeDrum4VelocityState state;
    Mode mode = mode_drum4_velocity_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on(26, 127);
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[1] == 8000);
    assert(output.gate[1]);
    tick_ms(&mode, 10, &output);
    assert(!output.gate[1]);
    assert(output.cv_mv[1] == 8000);
}

static void test_mode6_eight_triggers(void) {
    OutputState output;
    ModeDrum8TriggerState state;
    Mode mode = mode_drum8_trigger_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on(24, 100);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.cv_mv[0] == 8000);
    tick_ms(&mode, 10, &output);
    assert(output.cv_mv[0] == 0);

    event = note_on(31, 100);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.gate[3]);
    tick_ms(&mode, 10, &output);
    assert(!output.gate[3]);
}

static void test_placeholder_test_rotates_cv_voltage_steps_and_gate_chaser(void) {
    OutputState output;
    ModePlaceholderTestState state;
    Mode mode = mode_placeholder_test_create(&state, "placeholder");
    mode.reset(mode.state, &output);

    assert(output.cv_mv[0] == 0);
    assert(output.cv_mv[1] == 2000);
    assert(output.cv_mv[2] == 4000);
    assert(output.cv_mv[3] == 6000);
    assert(output.gate[0]);
    assert(!output.gate[1]);
    assert(!output.gate[2]);
    assert(!output.gate[3]);

    tick_ms(&mode, 1500, &output);
    assert(output.cv_mv[0] == 2000);
    assert(output.cv_mv[1] == 4000);
    assert(output.cv_mv[2] == 6000);
    assert(output.cv_mv[3] == 8000);
    assert(!output.gate[0]);
    assert(output.gate[1]);
    assert(!output.gate[2]);
    assert(!output.gate[3]);

    tick_ms(&mode, 1500, &output);
    assert(output.cv_mv[0] == 4000);
    assert(output.cv_mv[1] == 6000);
    assert(output.cv_mv[2] == 8000);
    assert(output.cv_mv[3] == 0);

    tick_ms(&mode, 3000, &output);
    assert(output.cv_mv[0] == 8000);
    assert(output.cv_mv[1] == 0);
    assert(output.cv_mv[2] == 2000);
    assert(output.cv_mv[3] == 4000);
    assert(output.gate[0]);
    assert(!output.gate[1]);
    assert(!output.gate[2]);
    assert(!output.gate[3]);
}

int main(void) {
    test_mode1_mono_midi_allocation();
    test_mode2_random_and_clock();
    test_mode2_dual_midi_allocation();
    test_mode2_dual_midi_routes_channel_ranges();
    test_mode2_uses_merged_dual_allocation_with_sources_present();
    test_mode3_poly4_midi_allocation();
    test_mode4_clocked_note_dual_pitch_and_transport();
    test_clocked_note_speed_profiles();
    test_mode4_run_restores_after_switching_back_while_playing();
    test_mode5_clocked_chord_arp_divisions();
    test_mode5_clocked_chord_arp_clears_pitch_when_chord_released();
    test_mode5_run_restores_after_switching_back_while_playing();
    test_mode5_drum_velocity();
    test_mode6_eight_triggers();
    test_placeholder_test_rotates_cv_voltage_steps_and_gate_chaser();
    return 0;
}

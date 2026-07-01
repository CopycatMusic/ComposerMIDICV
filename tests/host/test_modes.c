#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "app/output_state.h"
#include "midi/midi_event.h"
#include "modes/mode_drum4_velocity.h"
#include "modes/mode_drum8_trigger.h"
#include "modes/mode_dual_mono.h"
#include "modes/mode_mono_random_clock.h"

static MidiEvent note_on(uint8_t note, uint8_t velocity) {
    MidiEvent event = {
        .type = MIDI_EVENT_NOTE_ON,
        .channel = 0,
        .data1 = note,
        .data2 = velocity,
    };
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

static void tick_ms(const Mode* mode, uint8_t count, OutputState* output) {
    for (uint8_t i = 0; i < count; ++i) {
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

static void test_mode3_dual_mono(void) {
    OutputState output;
    ModeDualMonoState state;
    Mode mode = mode_dual_mono_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on(48, 100);
    mode.handle_midi(mode.state, &event, &output);
    event = note_on(60, 100);
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[0] == 1000);
    assert(output.cv_mv[2] == 2000);
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
    assert(output.cv_mv[1] == 4095);
    assert(output.cv_mv[3] == 4095);

    event = note_off(48);
    mode.handle_midi(mode.state, &event, &output);
    assert(!output.gate[0]);
    assert(output.gate[2]);
}

static void test_mode5_drum_velocity(void) {
    OutputState output;
    ModeDrum4VelocityState state;
    Mode mode = mode_drum4_velocity_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on(26, 127);
    mode.handle_midi(mode.state, &event, &output);

    assert(output.cv_mv[1] == 4095);
    assert(output.gate[1]);
    tick_ms(&mode, 10, &output);
    assert(!output.gate[1]);
    assert(output.cv_mv[1] == 4095);
}

static void test_mode6_eight_triggers(void) {
    OutputState output;
    ModeDrum8TriggerState state;
    Mode mode = mode_drum8_trigger_create(&state);
    mode.reset(mode.state, &output);

    MidiEvent event = note_on(24, 100);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.cv_mv[0] == 4095);
    tick_ms(&mode, 10, &output);
    assert(output.cv_mv[0] == 0);

    event = note_on(31, 100);
    mode.handle_midi(mode.state, &event, &output);
    assert(output.gate[3]);
    tick_ms(&mode, 10, &output);
    assert(!output.gate[3]);
}

int main(void) {
    test_mode2_random_and_clock();
    test_mode3_dual_mono();
    test_mode5_drum_velocity();
    test_mode6_eight_triggers();
    return 0;
}

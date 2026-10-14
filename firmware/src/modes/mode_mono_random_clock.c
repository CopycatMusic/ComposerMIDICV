#include "modes/mode_mono_random_clock.h"

#include "board_config.h"
#include "music/pitch_cv.h"

static uint16_t random_cv(ModeMonoRandomClockState* state) {
    uint32_t x = state->random_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state->random_state = x == 0 ? 0x1f123bb5u : x;
    return (uint16_t)(state->random_state %
                      (COMPOSER_MIDICV_CV_MAX_MV + 1u));
}

static void apply_active_note(ModeMonoRandomClockState* state,
                              OutputState* output) {
    uint8_t note = 0;
    uint8_t velocity = 0;
    if (note_stack_active(&state->notes, &note, &velocity)) {
        const uint16_t note_hz =
            pitch_cv_note_to_hz_bend(note, state->pitch_bend);
        const uint32_t octave_hz = (uint32_t)note_hz * 2u;
        output->cv_mv[0] = pitch_cv_note_to_mv_bend(note, state->pitch_bend);
        output->cv_mv[1] = pitch_cv_7bit_to_mv(velocity);
        output->audio_hz[2] = note_hz;
        output->audio_hz[3] =
            octave_hz > UINT16_MAX ? UINT16_MAX : (uint16_t)octave_hz;
        output->gate[0] = true;
    } else {
        output->audio_hz[2] = 0;
        output->audio_hz[3] = 0;
        output->gate[0] = false;
    }
}

static void reset(void* mode_state, OutputState* output) {
    ModeMonoRandomClockState* state = (ModeMonoRandomClockState*)mode_state;
    note_stack_init(&state->notes);
    state->pitch_bend = 0;
    state->trigger_ms = 0;
    state->random_state = 0x31415927u;
    output_state_clear(output);
}

static void handle_midi(void* mode_state,
                        const MidiEvent* event,
                        OutputState* output) {
    ModeMonoRandomClockState* state = (ModeMonoRandomClockState*)mode_state;

    switch (event->type) {
        case MIDI_EVENT_NOTE_ON:
            note_stack_note_on(&state->notes, event->data1, event->data2);
            output->cv_mv[2] = random_cv(state);
            output->cv_mv[3] = random_cv(state);
            output->gate[1] = true;
            state->trigger_ms = COMPOSER_MIDICV_TRIGGER_MS;
            apply_active_note(state, output);
            break;

        case MIDI_EVENT_NOTE_OFF:
            note_stack_note_off(&state->notes, event->data1);
            apply_active_note(state, output);
            break;

        case MIDI_EVENT_PITCH_BEND:
            state->pitch_bend = event->pitch_bend;
            apply_active_note(state, output);
            break;

        default:
            break;
    }
}

static void tick_1ms(void* mode_state, OutputState* output) {
    ModeMonoRandomClockState* state = (ModeMonoRandomClockState*)mode_state;
    if (state->trigger_ms == 0) {
        return;
    }
    --state->trigger_ms;
    if (state->trigger_ms == 0) {
        output->gate[1] = false;
    }
}

Mode mode_mono_random_clock_create(ModeMonoRandomClockState* state) {
    Mode mode = {
        .name = "mono_random_clock",
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

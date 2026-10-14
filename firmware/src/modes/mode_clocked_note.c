#include "modes/mode_clocked_note.h"

#include "board_config.h"
#include "music/pitch_cv.h"

#define MIDI_REALTIME_CLOCK 0xf8u
#define MIDI_REALTIME_START 0xfau
#define MIDI_REALTIME_CONTINUE 0xfbu
#define MIDI_REALTIME_STOP 0xfcu

enum {
    CV_PITCH_1 = 0,
    CV_VELOCITY_1 = 1,
    CV_PITCH_2 = 2,
    CV_VELOCITY_2 = 3,
    GATE_RUN = 0,
    GATE_RESET = 1,
    GATE_TRIGGER_0 = 2,
    GATE_TRIGGER_1 = 3,
    GATE_TRIGGER_RESET = 0,
    GATE_TRIGGER_GATE0 = 1,
    GATE_TRIGGER_GATE1 = 2,
};

static uint8_t ppqn_to_midi_clock_division(uint8_t ppqn) {
    if (ppqn == 0 || (24u % ppqn) != 0u) {
        return 24u;
    }
    return (uint8_t)(24u / ppqn);
}

static uint8_t output_function_to_ppqn(ModeOutputFunction output_function) {
    switch (output_function) {
        case MODE_OUTPUT_1PPQN_TRIGGER:
            return 1u;
        case MODE_OUTPUT_2PPQN_TRIGGER:
            return 2u;
        case MODE_OUTPUT_4PPQN_TRIGGER:
            return 4u;
        case MODE_OUTPUT_8PPQN_TRIGGER:
            return 8u;
        case MODE_OUTPUT_12PPQN_TRIGGER:
            return 12u;
        case MODE_OUTPUT_24PPQN_TRIGGER:
            return 24u;
        default:
            return 1u;
    }
}

static void configure_profile(ModeClockedNoteState* state,
                              const char* name,
                              uint8_t gate0_ppqn,
                              uint8_t gate1_ppqn) {
    state->name = name;
    state->gate_trigger_division[0] = ppqn_to_midi_clock_division(gate0_ppqn);
    state->gate_trigger_division[1] = ppqn_to_midi_clock_division(gate1_ppqn);
}

static void apply_voice(ModeClockedNoteState* state,
                        OutputState* output,
                        uint8_t voice) {
    if (voice == 0) {
        if (state->active[0]) {
            output->cv_mv[CV_PITCH_1] =
                pitch_cv_note_to_mv_bend(state->note[0],
                                         state->pitch_bend[0]);
        }
        output->cv_mv[CV_VELOCITY_1] =
            pitch_cv_7bit_to_mv(state->velocity[0]);
        return;
    }

    if (state->active[1]) {
        output->cv_mv[CV_PITCH_2] =
            pitch_cv_note_to_mv_bend(state->note[1], state->pitch_bend[1]);
    }
    output->cv_mv[CV_VELOCITY_2] = pitch_cv_7bit_to_mv(state->velocity[1]);
}

static uint8_t voice_for_channel(uint8_t channel) {
    return channel < 8u ? 0u : 1u;
}

static void pulse_gate(ModeClockedNoteState* state,
                       OutputState* output,
                       uint8_t gate_index,
                       uint8_t trigger_index) {
    state->gate_trigger_ms[trigger_index] = COMPOSER_MIDICV_TRIGGER_MS;
    output->gate[gate_index] = true;
}

static void handle_clock(ModeClockedNoteState* state, OutputState* output) {
    if (!state->running) {
        return;
    }

    ++state->clock_ticks;

    if ((state->clock_ticks % state->gate_trigger_division[0]) == 0u) {
        pulse_gate(state, output, GATE_TRIGGER_0, GATE_TRIGGER_GATE0);
    }
    if ((state->clock_ticks % state->gate_trigger_division[1]) == 0u) {
        pulse_gate(state, output, GATE_TRIGGER_1, GATE_TRIGGER_GATE1);
    }
}

static void reset(void* mode_state, OutputState* output) {
    ModeClockedNoteState* state = (ModeClockedNoteState*)mode_state;
    for (uint8_t i = 0; i < 2; ++i) {
        state->active[i] = false;
        state->note[i] = 0xff;
        state->velocity[i] = 0;
        state->pitch_bend[i] = 0;
    }
    state->running = false;
    state->clock_ticks = 0;
    for (uint8_t i = 0; i < 3; ++i) {
        state->gate_trigger_ms[i] = 0;
    }
    output_state_clear(output);
}

static void clear_triggers(ModeClockedNoteState* state, OutputState* output) {
    output->gate[GATE_RESET] = false;
    output->gate[GATE_TRIGGER_0] = false;
    output->gate[GATE_TRIGGER_1] = false;
    for (uint8_t i = 0; i < 3; ++i) {
        state->gate_trigger_ms[i] = 0;
    }
}

static void handle_realtime(ModeClockedNoteState* state,
                            const MidiEvent* event,
                            OutputState* output) {
    switch (event->realtime_byte) {
        case MIDI_REALTIME_CLOCK:
            handle_clock(state, output);
            break;

        case MIDI_REALTIME_START:
            state->running = true;
            state->clock_ticks = 0;
            output->gate[GATE_RUN] = true;
            pulse_gate(state, output, GATE_RESET, GATE_TRIGGER_RESET);
            break;

        case MIDI_REALTIME_CONTINUE:
            state->running = true;
            output->gate[GATE_RUN] = true;
            break;

        case MIDI_REALTIME_STOP:
            state->running = false;
            output->gate[GATE_RUN] = false;
            clear_triggers(state, output);
            break;

        default:
            break;
    }
}

static void handle_midi(void* mode_state,
                        const MidiEvent* event,
                        OutputState* output) {
    ModeClockedNoteState* state = (ModeClockedNoteState*)mode_state;

    switch (event->type) {
        case MIDI_EVENT_NOTE_ON: {
            const uint8_t voice = voice_for_channel(event->channel);
            state->active[voice] = true;
            state->note[voice] = event->data1;
            state->velocity[voice] = event->data2;
            apply_voice(state, output, voice);
            break;
        }

        case MIDI_EVENT_NOTE_OFF:
            {
                const uint8_t voice = voice_for_channel(event->channel);
                if (state->active[voice] &&
                    state->note[voice] == event->data1) {
                    state->active[voice] = false;
                    state->note[voice] = 0xff;
                    state->velocity[voice] = 0;
                    apply_voice(state, output, voice);
                }
            }
            break;

        case MIDI_EVENT_PITCH_BEND:
            {
                const uint8_t voice = voice_for_channel(event->channel);
                state->pitch_bend[voice] = event->pitch_bend;
                apply_voice(state, output, voice);
            }
            break;

        case MIDI_EVENT_REALTIME:
            handle_realtime(state, event, output);
            break;

        default:
            break;
    }
}

static void tick_1ms(void* mode_state, OutputState* output) {
    ModeClockedNoteState* state = (ModeClockedNoteState*)mode_state;

    for (uint8_t i = 0; i < 3; ++i) {
        if (state->gate_trigger_ms[i] > 0) {
            --state->gate_trigger_ms[i];
            if (state->gate_trigger_ms[i] == 0) {
                output->gate[i + GATE_RESET] = false;
            }
        }
    }
}

Mode mode_clocked_note_create(ModeClockedNoteState* state) {
    configure_profile(state, "sequencer1", 1u, 8u);

    Mode mode = {
        .name = state->name,
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

Mode mode_clocked_note_create_from_config(ModeClockedNoteState* state,
                                          const ModeSlotConfig* config) {
    configure_profile(state, config->name,
                      output_function_to_ppqn(config->gate[2]),
                      output_function_to_ppqn(config->gate[3]));

    Mode mode = {
        .name = state->name,
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

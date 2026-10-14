#include "modes/mode_clocked_chord_arp.h"

#include "board_config.h"
#include "music/pitch_cv.h"

#define MIDI_REALTIME_CLOCK 0xf8u
#define MIDI_REALTIME_START 0xfau
#define MIDI_REALTIME_CONTINUE 0xfbu
#define MIDI_REALTIME_STOP 0xfcu

enum {
    CV_1PPQN_ARP = 0,
    CV_4PPQN_ARP = 1,
    CV_1PPQN_RANDOM = 2,
    CV_4PPQN_RANDOM = 3,
    GATE_RUN = 0,
    GATE_RESET = 1,
    GATE_1PPQN_TRIGGER = 2,
    GATE_4PPQN_TRIGGER = 3,
    GATE_TRIGGER_RESET = 0,
    GATE_TRIGGER_1PPQN = 1,
    GATE_TRIGGER_4PPQN = 2,
};

static uint32_t next_random(ModeClockedChordArpState* state) {
    uint32_t x = state->random_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    state->random_state = x == 0 ? 0x45d9f3bu : x;
    return state->random_state;
}

static uint8_t chord_note_at(const ModeClockedChordArpState* state,
                             uint8_t index) {
    if (state->notes.count == 0) {
        return 0;
    }
    return state->notes.note[index % state->notes.count];
}

static void clear_pitch_outputs_if_empty(ModeClockedChordArpState* state,
                                         OutputState* output) {
    if (state->notes.count != 0) {
        return;
    }

    output->cv_mv[CV_1PPQN_ARP] = 0;
    output->cv_mv[CV_4PPQN_ARP] = 0;
    output->cv_mv[CV_1PPQN_RANDOM] = 0;
    output->cv_mv[CV_4PPQN_RANDOM] = 0;
}

static void update_arp_output(ModeClockedChordArpState* state,
                              OutputState* output,
                              uint8_t cv_index,
                              uint8_t* arp_index) {
    if (state->notes.count == 0) {
        output->cv_mv[cv_index] = 0;
        return;
    }

    const uint8_t note = chord_note_at(state, *arp_index);
    output->cv_mv[cv_index] = pitch_cv_note_to_mv(note);
    *arp_index = (uint8_t)((*arp_index + 1u) % state->notes.count);
}

static void update_random_output(ModeClockedChordArpState* state,
                                 OutputState* output,
                                 uint8_t cv_index) {
    if (state->notes.count == 0) {
        output->cv_mv[cv_index] = 0;
        return;
    }

    const uint8_t index = (uint8_t)(next_random(state) % state->notes.count);
    output->cv_mv[cv_index] = pitch_cv_note_to_mv(state->notes.note[index]);
}

static void pulse_gate(ModeClockedChordArpState* state,
                       OutputState* output,
                       uint8_t gate_index,
                       uint8_t trigger_index) {
    state->gate_trigger_ms[trigger_index] = COMPOSER_MIDICV_TRIGGER_MS;
    output->gate[gate_index] = true;
}

static void handle_4ppqn_step(ModeClockedChordArpState* state,
                              OutputState* output) {
    update_arp_output(state, output, CV_4PPQN_ARP, &state->arp_index_4ppqn);
    update_random_output(state, output, CV_4PPQN_RANDOM);
    pulse_gate(state, output, GATE_4PPQN_TRIGGER, GATE_TRIGGER_4PPQN);
}

static void handle_1ppqn_step(ModeClockedChordArpState* state,
                              OutputState* output) {
    update_arp_output(state, output, CV_1PPQN_ARP, &state->arp_index_1ppqn);
    update_random_output(state, output, CV_1PPQN_RANDOM);
    pulse_gate(state, output, GATE_1PPQN_TRIGGER, GATE_TRIGGER_1PPQN);
}

static void handle_clock(ModeClockedChordArpState* state, OutputState* output) {
    if (!state->running) {
        return;
    }

    ++state->clock_ticks;
    if ((state->clock_ticks % 6u) == 0u) {
        handle_4ppqn_step(state, output);
    }
    if ((state->clock_ticks % 24u) == 0u) {
        handle_1ppqn_step(state, output);
    }
}

static void clear_triggers(ModeClockedChordArpState* state,
                           OutputState* output) {
    output->gate[GATE_RESET] = false;
    output->gate[GATE_1PPQN_TRIGGER] = false;
    output->gate[GATE_4PPQN_TRIGGER] = false;
    for (uint8_t i = 0; i < 3; ++i) {
        state->gate_trigger_ms[i] = 0;
    }
}

static void reset(void* mode_state, OutputState* output) {
    ModeClockedChordArpState* state = (ModeClockedChordArpState*)mode_state;
    note_stack_init(&state->notes);
    state->running = false;
    state->clock_ticks = 0;
    state->arp_index_1ppqn = 0;
    state->arp_index_4ppqn = 0;
    state->random_state = 0x9e3779b9u;
    for (uint8_t i = 0; i < 3; ++i) {
        state->gate_trigger_ms[i] = 0;
    }
    output_state_clear(output);
}

static void handle_realtime(ModeClockedChordArpState* state,
                            const MidiEvent* event,
                            OutputState* output) {
    switch (event->realtime_byte) {
        case MIDI_REALTIME_CLOCK:
            handle_clock(state, output);
            break;

        case MIDI_REALTIME_START:
            state->running = true;
            state->clock_ticks = 0;
            state->arp_index_1ppqn = 0;
            state->arp_index_4ppqn = 0;
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
    ModeClockedChordArpState* state = (ModeClockedChordArpState*)mode_state;

    switch (event->type) {
        case MIDI_EVENT_NOTE_ON:
            note_stack_note_on(&state->notes, event->data1, event->data2);
            break;

        case MIDI_EVENT_NOTE_OFF:
            note_stack_note_off(&state->notes, event->data1);
            if (state->notes.count != 0) {
                state->arp_index_1ppqn %= state->notes.count;
                state->arp_index_4ppqn %= state->notes.count;
            }
            clear_pitch_outputs_if_empty(state, output);
            break;

        case MIDI_EVENT_REALTIME:
            handle_realtime(state, event, output);
            break;

        default:
            break;
    }
}

static void tick_1ms(void* mode_state, OutputState* output) {
    ModeClockedChordArpState* state = (ModeClockedChordArpState*)mode_state;

    for (uint8_t i = 0; i < 3; ++i) {
        if (state->gate_trigger_ms[i] > 0) {
            --state->gate_trigger_ms[i];
            if (state->gate_trigger_ms[i] == 0) {
                output->gate[i + GATE_RESET] = false;
            }
        }
    }
}

Mode mode_clocked_chord_arp_create(ModeClockedChordArpState* state) {
    Mode mode = {
        .name = "clocked_chord_arp",
        .state = state,
        .implemented = true,
        .reset = reset,
        .handle_midi = handle_midi,
        .tick_1ms = tick_1ms,
    };
    return mode;
}

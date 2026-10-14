#include "config/mode_config.h"

#include <stddef.h>

static const ModeSlotConfig default_slots[MODE_CONFIG_NUM_SLOTS] = {
    {
        .name = "midi1",
        .engine = MODE_ENGINE_MONO_MIDI,
        .cv = {
            MODE_OUTPUT_PITCH,
            MODE_OUTPUT_VELOCITY,
            MODE_OUTPUT_MODULATION_CC1,
            MODE_OUTPUT_RANDOM_SH,
        },
        .gate = {
            MODE_OUTPUT_GATE,
            MODE_OUTPUT_GATE,
            MODE_OUTPUT_TRIGGER,
            MODE_OUTPUT_TRIGGER,
        },
    },
    {
        .name = "midi2",
        .engine = MODE_ENGINE_DUAL_MIDI,
        .cv = {
            MODE_OUTPUT_PITCH_1,
            MODE_OUTPUT_VELOCITY_1,
            MODE_OUTPUT_PITCH_2,
            MODE_OUTPUT_VELOCITY_2,
        },
        .gate = {
            MODE_OUTPUT_GATE_1,
            MODE_OUTPUT_TRIGGER_1,
            MODE_OUTPUT_GATE_2,
            MODE_OUTPUT_TRIGGER_2,
        },
    },
    {
        .name = "midi3",
        .engine = MODE_ENGINE_POLY4_MIDI,
        .cv = {
            MODE_OUTPUT_PITCH_1,
            MODE_OUTPUT_PITCH_2,
            MODE_OUTPUT_PITCH_3,
            MODE_OUTPUT_PITCH_4,
        },
        .gate = {
            MODE_OUTPUT_GATE_1,
            MODE_OUTPUT_GATE_2,
            MODE_OUTPUT_GATE_3,
            MODE_OUTPUT_GATE_4,
        },
    },
    {
        .name = "sequencer1",
        .engine = MODE_ENGINE_CLOCKED_NOTE,
        .cv = {
            MODE_OUTPUT_PITCH_1,
            MODE_OUTPUT_VELOCITY_1,
            MODE_OUTPUT_PITCH_2,
            MODE_OUTPUT_VELOCITY_2,
        },
        .gate = {
            MODE_OUTPUT_RUN,
            MODE_OUTPUT_RESET,
            MODE_OUTPUT_1PPQN_TRIGGER,
            MODE_OUTPUT_8PPQN_TRIGGER,
        },
    },
    {
        .name = "sequencer2",
        .engine = MODE_ENGINE_CLOCKED_NOTE,
        .cv = {
            MODE_OUTPUT_PITCH_1,
            MODE_OUTPUT_VELOCITY_1,
            MODE_OUTPUT_PITCH_2,
            MODE_OUTPUT_VELOCITY_2,
        },
        .gate = {
            MODE_OUTPUT_RUN,
            MODE_OUTPUT_RESET,
            MODE_OUTPUT_2PPQN_TRIGGER,
            MODE_OUTPUT_12PPQN_TRIGGER,
        },
    },
    {
        .name = "sequencer3",
        .engine = MODE_ENGINE_CLOCKED_NOTE,
        .cv = {
            MODE_OUTPUT_PITCH_1,
            MODE_OUTPUT_VELOCITY_1,
            MODE_OUTPUT_PITCH_2,
            MODE_OUTPUT_VELOCITY_2,
        },
        .gate = {
            MODE_OUTPUT_RUN,
            MODE_OUTPUT_RESET,
            MODE_OUTPUT_4PPQN_TRIGGER,
            MODE_OUTPUT_24PPQN_TRIGGER,
        },
    },
    {.name = "reserved", .engine = MODE_ENGINE_RESERVED},
    {.name = "led_chaser", .engine = MODE_ENGINE_LED_CHASER},
};

uint8_t mode_config_num_slots(void) {
    return MODE_CONFIG_NUM_SLOTS;
}

const ModeSlotConfig* mode_config_default_slot(uint8_t slot) {
    if (slot >= MODE_CONFIG_NUM_SLOTS) {
        return NULL;
    }
    return &default_slots[slot];
}

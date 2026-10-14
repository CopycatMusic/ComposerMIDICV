#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "config/mode_config.h"

static void test_default_slots_match_mode_explain(void) {
    assert(mode_config_num_slots() == 8);

    const ModeSlotConfig* mode1 = mode_config_default_slot(0);
    assert(mode1 != NULL);
    assert(strcmp(mode1->name, "midi1") == 0);
    assert(mode1->engine == MODE_ENGINE_MONO_MIDI);
    assert(mode1->cv[0] == MODE_OUTPUT_PITCH);
    assert(mode1->cv[1] == MODE_OUTPUT_VELOCITY);
    assert(mode1->cv[2] == MODE_OUTPUT_MODULATION_CC1);
    assert(mode1->cv[3] == MODE_OUTPUT_RANDOM_SH);
    assert(mode1->gate[0] == MODE_OUTPUT_GATE);
    assert(mode1->gate[1] == MODE_OUTPUT_GATE);
    assert(mode1->gate[2] == MODE_OUTPUT_TRIGGER);
    assert(mode1->gate[3] == MODE_OUTPUT_TRIGGER);

    const ModeSlotConfig* mode2 = mode_config_default_slot(1);
    assert(mode2 != NULL);
    assert(strcmp(mode2->name, "midi2") == 0);
    assert(mode2->engine == MODE_ENGINE_DUAL_MIDI);
    assert(mode2->cv[0] == MODE_OUTPUT_PITCH_1);
    assert(mode2->cv[1] == MODE_OUTPUT_VELOCITY_1);
    assert(mode2->cv[2] == MODE_OUTPUT_PITCH_2);
    assert(mode2->cv[3] == MODE_OUTPUT_VELOCITY_2);
    assert(mode2->gate[0] == MODE_OUTPUT_GATE_1);
    assert(mode2->gate[1] == MODE_OUTPUT_TRIGGER_1);
    assert(mode2->gate[2] == MODE_OUTPUT_GATE_2);
    assert(mode2->gate[3] == MODE_OUTPUT_TRIGGER_2);

    const ModeSlotConfig* mode3 = mode_config_default_slot(2);
    assert(mode3 != NULL);
    assert(strcmp(mode3->name, "midi3") == 0);
    assert(mode3->engine == MODE_ENGINE_POLY4_MIDI);
    assert(mode3->cv[0] == MODE_OUTPUT_PITCH_1);
    assert(mode3->cv[1] == MODE_OUTPUT_PITCH_2);
    assert(mode3->cv[2] == MODE_OUTPUT_PITCH_3);
    assert(mode3->cv[3] == MODE_OUTPUT_PITCH_4);
    assert(mode3->gate[0] == MODE_OUTPUT_GATE_1);
    assert(mode3->gate[1] == MODE_OUTPUT_GATE_2);
    assert(mode3->gate[2] == MODE_OUTPUT_GATE_3);
    assert(mode3->gate[3] == MODE_OUTPUT_GATE_4);

    const ModeSlotConfig* mode4 = mode_config_default_slot(3);
    assert(mode4 != NULL);
    assert(strcmp(mode4->name, "sequencer1") == 0);
    assert(mode4->engine == MODE_ENGINE_CLOCKED_NOTE);
    assert(mode4->cv[0] == MODE_OUTPUT_PITCH_1);
    assert(mode4->cv[1] == MODE_OUTPUT_VELOCITY_1);
    assert(mode4->cv[2] == MODE_OUTPUT_PITCH_2);
    assert(mode4->cv[3] == MODE_OUTPUT_VELOCITY_2);
    assert(mode4->gate[0] == MODE_OUTPUT_RUN);
    assert(mode4->gate[1] == MODE_OUTPUT_RESET);
    assert(mode4->gate[2] == MODE_OUTPUT_1PPQN_TRIGGER);
    assert(mode4->gate[3] == MODE_OUTPUT_8PPQN_TRIGGER);
}

static void test_clocked_note_speed_slots(void) {
    const char* expected_names[] = {
        "sequencer2",
        "sequencer3",
    };

    for (uint8_t i = 4; i < 6; ++i) {
        const ModeSlotConfig* slot = mode_config_default_slot(i);
        assert(slot != NULL);
        assert(strcmp(slot->name, expected_names[i - 4u]) == 0);
    }

    const ModeSlotConfig* mode5 = mode_config_default_slot(4);
    assert(mode5->engine == MODE_ENGINE_CLOCKED_NOTE);
    assert(mode5->cv[0] == MODE_OUTPUT_PITCH_1);
    assert(mode5->cv[1] == MODE_OUTPUT_VELOCITY_1);
    assert(mode5->cv[2] == MODE_OUTPUT_PITCH_2);
    assert(mode5->cv[3] == MODE_OUTPUT_VELOCITY_2);
    assert(mode5->gate[0] == MODE_OUTPUT_RUN);
    assert(mode5->gate[1] == MODE_OUTPUT_RESET);
    assert(mode5->gate[2] == MODE_OUTPUT_2PPQN_TRIGGER);
    assert(mode5->gate[3] == MODE_OUTPUT_12PPQN_TRIGGER);

    const ModeSlotConfig* mode6 = mode_config_default_slot(5);
    assert(mode6->engine == MODE_ENGINE_CLOCKED_NOTE);
    assert(mode6->cv[0] == MODE_OUTPUT_PITCH_1);
    assert(mode6->cv[1] == MODE_OUTPUT_VELOCITY_1);
    assert(mode6->cv[2] == MODE_OUTPUT_PITCH_2);
    assert(mode6->cv[3] == MODE_OUTPUT_VELOCITY_2);
    assert(mode6->gate[0] == MODE_OUTPUT_RUN);
    assert(mode6->gate[1] == MODE_OUTPUT_RESET);
    assert(mode6->gate[2] == MODE_OUTPUT_4PPQN_TRIGGER);
    assert(mode6->gate[3] == MODE_OUTPUT_24PPQN_TRIGGER);

    const ModeSlotConfig* slot7 = mode_config_default_slot(6);
    assert(slot7 != NULL);
    assert(slot7->engine == MODE_ENGINE_RESERVED);
    assert(mode_config_default_slot(8) == NULL);
}

static void test_slot_8_is_led_chaser(void) {
    const ModeSlotConfig* mode8 = mode_config_default_slot(7);
    assert(mode8 != NULL);
    assert(mode8->engine == MODE_ENGINE_LED_CHASER);
    assert(mode8->name != NULL);
    for (uint8_t out = 0; out < 4; ++out) {
        assert(mode8->cv[out] == MODE_OUTPUT_NONE);
        assert(mode8->gate[out] == MODE_OUTPUT_NONE);
    }
}

int main(void) {
    test_default_slots_match_mode_explain();
    test_clocked_note_speed_slots();
    test_slot_8_is_led_chaser();
    return 0;
}

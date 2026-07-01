#include "music/note_stack.h"

void note_stack_init(NoteStack* stack) {
    stack->count = 0;
}

static void remove_note(NoteStack* stack, uint8_t note) {
    for (uint8_t i = 0; i < stack->count; ++i) {
        if (stack->note[i] != note) {
            continue;
        }
        for (uint8_t j = i; j + 1 < stack->count; ++j) {
            stack->note[j] = stack->note[j + 1];
            stack->velocity[j] = stack->velocity[j + 1];
        }
        --stack->count;
        return;
    }
}

void note_stack_note_on(NoteStack* stack, uint8_t note, uint8_t velocity) {
    remove_note(stack, note);

    if (stack->count == NOTE_STACK_CAPACITY) {
        for (uint8_t i = 0; i + 1 < NOTE_STACK_CAPACITY; ++i) {
            stack->note[i] = stack->note[i + 1];
            stack->velocity[i] = stack->velocity[i + 1];
        }
        stack->count = NOTE_STACK_CAPACITY - 1;
    }

    stack->note[stack->count] = note;
    stack->velocity[stack->count] = velocity;
    ++stack->count;
}

void note_stack_note_off(NoteStack* stack, uint8_t note) {
    remove_note(stack, note);
}

bool note_stack_active(const NoteStack* stack, uint8_t* note, uint8_t* velocity) {
    if (stack->count == 0) {
        return false;
    }

    const uint8_t index = stack->count - 1;
    *note = stack->note[index];
    *velocity = stack->velocity[index];
    return true;
}

bool note_stack_empty(const NoteStack* stack) {
    return stack->count == 0;
}

#ifndef COMPOSER_MIDICV_MUSIC_NOTE_STACK_H_
#define COMPOSER_MIDICV_MUSIC_NOTE_STACK_H_

#include <stdbool.h>
#include <stdint.h>

#define NOTE_STACK_CAPACITY 16

typedef struct {
    uint8_t note[NOTE_STACK_CAPACITY];
    uint8_t velocity[NOTE_STACK_CAPACITY];
    uint8_t count;
} NoteStack;

void note_stack_init(NoteStack* stack);
void note_stack_note_on(NoteStack* stack, uint8_t note, uint8_t velocity);
void note_stack_note_off(NoteStack* stack, uint8_t note);
bool note_stack_active(const NoteStack* stack, uint8_t* note, uint8_t* velocity);
bool note_stack_empty(const NoteStack* stack);

#endif  // COMPOSER_MIDICV_MUSIC_NOTE_STACK_H_

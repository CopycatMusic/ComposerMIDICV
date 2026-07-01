#ifndef COMPOSER_MIDICV_MUSIC_VOICE_ALLOCATOR_H_
#define COMPOSER_MIDICV_MUSIC_VOICE_ALLOCATOR_H_

#include <stdbool.h>
#include <stdint.h>

#define VOICE_ALLOCATOR_NUM_VOICES 4

typedef struct {
    bool active[VOICE_ALLOCATOR_NUM_VOICES];
    uint8_t note[VOICE_ALLOCATOR_NUM_VOICES];
    uint32_t age[VOICE_ALLOCATOR_NUM_VOICES];
    uint32_t clock;
} VoiceAllocator;

void voice_allocator_init(VoiceAllocator* allocator);
uint8_t voice_allocator_note_on(VoiceAllocator* allocator, uint8_t note);
uint8_t voice_allocator_note_off(VoiceAllocator* allocator, uint8_t note);
bool voice_allocator_is_active(const VoiceAllocator* allocator, uint8_t voice);

#endif  // COMPOSER_MIDICV_MUSIC_VOICE_ALLOCATOR_H_

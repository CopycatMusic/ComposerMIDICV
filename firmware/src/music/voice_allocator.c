#include "music/voice_allocator.h"

void voice_allocator_init(VoiceAllocator* allocator) {
    allocator->clock = 0;
    for (uint8_t i = 0; i < VOICE_ALLOCATOR_NUM_VOICES; ++i) {
        allocator->active[i] = false;
        allocator->note[i] = 0xff;
        allocator->age[i] = 0;
    }
}

uint8_t voice_allocator_note_on(VoiceAllocator* allocator, uint8_t note) {
    ++allocator->clock;

    for (uint8_t i = 0; i < VOICE_ALLOCATOR_NUM_VOICES; ++i) {
        if (allocator->active[i] && allocator->note[i] == note) {
            allocator->age[i] = allocator->clock;
            return i;
        }
    }

    for (uint8_t i = 0; i < VOICE_ALLOCATOR_NUM_VOICES; ++i) {
        if (!allocator->active[i]) {
            allocator->active[i] = true;
            allocator->note[i] = note;
            allocator->age[i] = allocator->clock;
            return i;
        }
    }

    uint8_t oldest = 0;
    for (uint8_t i = 1; i < VOICE_ALLOCATOR_NUM_VOICES; ++i) {
        if (allocator->age[i] < allocator->age[oldest]) {
            oldest = i;
        }
    }

    allocator->active[oldest] = true;
    allocator->note[oldest] = note;
    allocator->age[oldest] = allocator->clock;
    return oldest;
}

uint8_t voice_allocator_note_off(VoiceAllocator* allocator, uint8_t note) {
    for (uint8_t i = 0; i < VOICE_ALLOCATOR_NUM_VOICES; ++i) {
        if (allocator->active[i] && allocator->note[i] == note) {
            allocator->active[i] = false;
            allocator->note[i] = 0xff;
            return i;
        }
    }
    return 0xff;
}

bool voice_allocator_is_active(const VoiceAllocator* allocator, uint8_t voice) {
    if (voice >= VOICE_ALLOCATOR_NUM_VOICES) {
        return false;
    }
    return allocator->active[voice];
}

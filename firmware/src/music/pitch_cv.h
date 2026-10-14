#ifndef COMPOSER_MIDICV_MUSIC_PITCH_CV_H_
#define COMPOSER_MIDICV_MUSIC_PITCH_CV_H_

#include <stdbool.h>
#include <stdint.h>

bool pitch_cv_note_is_in_output_range(uint8_t midi_note);
uint16_t pitch_cv_note_to_mv(uint8_t midi_note);
uint16_t pitch_cv_note_to_mv_bend(uint8_t midi_note, int16_t pitch_bend);
uint16_t pitch_cv_7bit_to_mv(uint8_t value);
uint16_t pitch_cv_note_to_hz(uint8_t midi_note);
uint16_t pitch_cv_note_to_hz_bend(uint8_t midi_note, int16_t pitch_bend);

#endif  // COMPOSER_MIDICV_MUSIC_PITCH_CV_H_

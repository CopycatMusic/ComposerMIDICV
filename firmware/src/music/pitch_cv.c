#include "music/pitch_cv.h"

#include "board_config.h"

static uint16_t clamp_cv_mv_i32(int32_t value) {
    if (value < 0) {
        return 0;
    }
    if (value > (int32_t)COMPOSER_MIDICV_CV_MAX_MV) {
        return COMPOSER_MIDICV_CV_MAX_MV;
    }
    return (uint16_t)value;
}

uint16_t pitch_cv_note_to_mv(uint8_t midi_note) {
    const int32_t semitones_from_base = (int32_t)midi_note - 36;
    const int32_t mv = (semitones_from_base * 1000 + 6) / 12;
    return clamp_cv_mv_i32(mv);
}

uint16_t pitch_cv_note_to_mv_bend(uint8_t midi_note, int16_t pitch_bend) {
    const int32_t base_mv = pitch_cv_note_to_mv(midi_note);
    const int32_t bend_numerator = (int32_t)pitch_bend * 2000;
    const int32_t bend_denominator = 12 * 8192;
    const int32_t bend_mv =
        bend_numerator >= 0
            ? (bend_numerator + bend_denominator / 2) / bend_denominator
            : (bend_numerator - bend_denominator / 2) / bend_denominator;

    return clamp_cv_mv_i32(base_mv + bend_mv);
}

uint16_t pitch_cv_7bit_to_mv(uint8_t value) {
    if (value > 127) {
        value = 127;
    }
    return (uint16_t)(((uint32_t)value * COMPOSER_MIDICV_CV_MAX_MV) / 127u);
}

static uint32_t pitch_cv_note_to_hz_q16(uint8_t midi_note) {
    const uint32_t semitone_up_q16 = 69433u;
    const uint32_t semitone_down_q16 = 61858u;
    uint32_t hz_q16 = 440u << 16;

    if (midi_note >= 69u) {
        for (uint8_t i = 0; i < midi_note - 69u; ++i) {
            hz_q16 = (uint32_t)(((uint64_t)hz_q16 * semitone_up_q16 +
                                 (1u << 15)) >>
                                16);
        }
    } else {
        for (uint8_t i = 0; i < 69u - midi_note; ++i) {
            hz_q16 = (uint32_t)(((uint64_t)hz_q16 * semitone_down_q16 +
                                 (1u << 15)) >>
                                16);
        }
    }

    return hz_q16;
}

uint16_t pitch_cv_note_to_hz(uint8_t midi_note) {
    const uint32_t rounded_hz =
        (pitch_cv_note_to_hz_q16(midi_note) + (1u << 15)) >> 16;
    return rounded_hz > UINT16_MAX ? UINT16_MAX : (uint16_t)rounded_hz;
}

uint16_t pitch_cv_note_to_hz_bend(uint8_t midi_note, int16_t pitch_bend) {
    const int32_t bend_q8 =
        pitch_bend >= 0
            ? ((int32_t)pitch_bend * 512 + 4096) / 8192
            : ((int32_t)pitch_bend * 512 - 4096) / 8192;
    int32_t note_q8 = (int32_t)midi_note * 256 + bend_q8;
    if (note_q8 < 0) {
        note_q8 = 0;
    }

    uint8_t lower_note = (uint8_t)(note_q8 >> 8);
    if (lower_note >= UINT8_MAX) {
        return UINT16_MAX;
    }

    const uint8_t frac = (uint8_t)(note_q8 & 0xff);
    const uint32_t lower_hz_q16 = pitch_cv_note_to_hz_q16(lower_note);
    if (frac == 0) {
        const uint32_t rounded_hz = (lower_hz_q16 + (1u << 15)) >> 16;
        return rounded_hz > UINT16_MAX ? UINT16_MAX : (uint16_t)rounded_hz;
    }

    const uint32_t upper_hz_q16 = pitch_cv_note_to_hz_q16(lower_note + 1u);
    const uint32_t interp_hz_q16 =
        lower_hz_q16 +
        (uint32_t)(((uint64_t)(upper_hz_q16 - lower_hz_q16) * frac +
                    128u) >>
                   8);
    const uint32_t rounded_hz = (interp_hz_q16 + (1u << 15)) >> 16;
    return rounded_hz > UINT16_MAX ? UINT16_MAX : (uint16_t)rounded_hz;
}

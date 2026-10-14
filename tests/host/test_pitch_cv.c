#include <assert.h>
#include <stdint.h>

#include "music/pitch_cv.h"

int main(void) {
    assert(!pitch_cv_note_is_in_output_range(35));
    assert(pitch_cv_note_is_in_output_range(36));
    assert(pitch_cv_note_is_in_output_range(85));
    assert(pitch_cv_note_is_in_output_range(86));
    assert(pitch_cv_note_is_in_output_range(127));
    assert(pitch_cv_note_to_mv_bend(48, 0) == 1000);
    assert(pitch_cv_note_to_mv_bend(48, 8191) == 1167);
    assert(pitch_cv_note_to_mv_bend(48, -8192) == 833);
    assert(pitch_cv_note_to_mv_bend(36, -8192) == 0);
    assert(pitch_cv_note_to_hz_bend(48, 0) == 131);
    assert(pitch_cv_note_to_hz_bend(48, 8191) == 147);
    assert(pitch_cv_note_to_hz_bend(48, -8192) == 117);
    return 0;
}

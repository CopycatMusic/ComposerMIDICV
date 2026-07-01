#ifndef COMPOSER_MIDICV_MUSIC_CALIBRATION_H_
#define COMPOSER_MIDICV_MUSIC_CALIBRATION_H_

#include <stdbool.h>
#include <stdint.h>

#include "board_config.h"

typedef struct {
    uint16_t target_mv;
    uint16_t measured_mv;
} CalibrationPoint;

typedef struct {
    CalibrationPoint point[2];
} CvCalibration;

typedef struct {
    CvCalibration cv[COMPOSER_MIDICV_NUM_CV_OUTPUTS];
    bool loaded_from_flash;
} CalibrationState;

void calibration_init(CalibrationState* state);
void calibration_reset_defaults(CalibrationState* state);
uint16_t calibration_apply_mv(const CalibrationState* state,
                              uint8_t cv_index,
                              uint16_t target_mv);
bool calibration_set_measurement(CalibrationState* state,
                                 uint8_t cv_index,
                                 uint16_t target_mv,
                                 uint16_t measured_mv);
bool calibration_save(const CalibrationState* state);

#endif  // COMPOSER_MIDICV_MUSIC_CALIBRATION_H_

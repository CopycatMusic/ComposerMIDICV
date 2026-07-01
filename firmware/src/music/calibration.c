#include "music/calibration.h"

#include <string.h>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/platform.h"

#ifndef PICO_FLASH_SIZE_BYTES
#define PICO_FLASH_SIZE_BYTES (2 * 1024 * 1024)
#endif

#define CALIBRATION_MAGIC 0x434d4341u  // "CMCA"
#define CALIBRATION_VERSION 1u

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t size;
    CvCalibration cv[COMPOSER_MIDICV_NUM_CV_OUTPUTS];
    uint32_t crc32;
} CalibrationBlob;

static const uint32_t calibration_flash_offset =
    PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;

static uint16_t clamp_mv_u32(uint32_t value) {
    if (value > COMPOSER_MIDICV_CV_MAX_MV) {
        return COMPOSER_MIDICV_CV_MAX_MV;
    }
    return (uint16_t)value;
}

static uint32_t crc32_update(uint32_t crc, uint8_t byte) {
    crc ^= byte;
    for (uint8_t i = 0; i < 8; ++i) {
        const uint32_t mask = 0u - (crc & 1u);
        crc = (crc >> 1) ^ (0xedb88320u & mask);
    }
    return crc;
}

static uint32_t crc32_bytes(const void* data, uint32_t size) {
    const uint8_t* bytes = (const uint8_t*)data;
    uint32_t crc = 0xffffffffu;
    for (uint32_t i = 0; i < size; ++i) {
        crc = crc32_update(crc, bytes[i]);
    }
    return ~crc;
}

static uint32_t blob_crc(const CalibrationBlob* blob) {
    const uint32_t crc_offset = offsetof(CalibrationBlob, crc32);
    return crc32_bytes(blob, crc_offset);
}

void calibration_reset_defaults(CalibrationState* state) {
    for (uint8_t i = 0; i < COMPOSER_MIDICV_NUM_CV_OUTPUTS; ++i) {
        state->cv[i].point[0].target_mv = 0;
        state->cv[i].point[0].measured_mv = 0;
        state->cv[i].point[1].target_mv = COMPOSER_MIDICV_CV_MAX_MV;
        state->cv[i].point[1].measured_mv = COMPOSER_MIDICV_CV_MAX_MV;
    }
    state->loaded_from_flash = false;
}

void calibration_init(CalibrationState* state) {
    calibration_reset_defaults(state);

    const CalibrationBlob* blob =
        (const CalibrationBlob*)(XIP_BASE + calibration_flash_offset);

    if (blob->magic != CALIBRATION_MAGIC ||
        blob->version != CALIBRATION_VERSION ||
        blob->size != sizeof(CalibrationBlob)) {
        return;
    }

    if (blob->crc32 != blob_crc(blob)) {
        return;
    }

    memcpy(state->cv, blob->cv, sizeof(state->cv));
    state->loaded_from_flash = true;
}

uint16_t calibration_apply_mv(const CalibrationState* state,
                              uint8_t cv_index,
                              uint16_t target_mv) {
    if (cv_index >= COMPOSER_MIDICV_NUM_CV_OUTPUTS) {
        return 0;
    }

    const CalibrationPoint p0 = state->cv[cv_index].point[0];
    const CalibrationPoint p1 = state->cv[cv_index].point[1];
    const int32_t measured_delta = (int32_t)p1.measured_mv - p0.measured_mv;
    const int32_t target_delta = (int32_t)p1.target_mv - p0.target_mv;

    if (measured_delta == 0 || target_delta == 0) {
        return clamp_mv_u32(target_mv);
    }

    // Invert the measured response line so the requested voltage is corrected
    // into the DAC code that should produce it.
    const int32_t requested_delta = (int32_t)target_mv - p0.measured_mv;
    int32_t corrected = p0.target_mv +
                        (int32_t)(((int64_t)requested_delta * target_delta) /
                                  measured_delta);

    if (corrected < 0) {
        corrected = 0;
    }
    return clamp_mv_u32((uint32_t)corrected);
}

bool calibration_set_measurement(CalibrationState* state,
                                 uint8_t cv_index,
                                 uint16_t target_mv,
                                 uint16_t measured_mv) {
    if (cv_index >= COMPOSER_MIDICV_NUM_CV_OUTPUTS) {
        return false;
    }

    target_mv = clamp_mv_u32(target_mv);
    measured_mv = clamp_mv_u32(measured_mv);

    CvCalibration* cv = &state->cv[cv_index];
    const uint16_t midpoint = COMPOSER_MIDICV_CV_MAX_MV / 2u;
    const uint8_t slot = target_mv <= midpoint ? 0u : 1u;

    cv->point[slot].target_mv = target_mv;
    cv->point[slot].measured_mv = measured_mv;

    if (cv->point[0].target_mv > cv->point[1].target_mv) {
        const CalibrationPoint tmp = cv->point[0];
        cv->point[0] = cv->point[1];
        cv->point[1] = tmp;
    }

    state->loaded_from_flash = false;
    return true;
}

bool calibration_save(const CalibrationState* state) {
    CalibrationBlob blob;
    memset(&blob, 0xff, sizeof(blob));
    blob.magic = CALIBRATION_MAGIC;
    blob.version = CALIBRATION_VERSION;
    blob.size = sizeof(blob);
    memcpy(blob.cv, state->cv, sizeof(blob.cv));
    blob.crc32 = blob_crc(&blob);

    uint8_t page[FLASH_PAGE_SIZE];
    memset(page, 0xff, sizeof(page));
    memcpy(page, &blob, sizeof(blob));

    const uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(calibration_flash_offset, FLASH_SECTOR_SIZE);
    flash_range_program(calibration_flash_offset, page, FLASH_PAGE_SIZE);
    restore_interrupts(interrupts);

    return true;
}

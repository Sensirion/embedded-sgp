/*
 * Copyright (c) 2018, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef SGP_FEATURESET_H
#define SGP_FEATURESET_H

#include "sensirion_arch_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SGP_WORD_LEN 2
#define SGP_COMMAND_LEN SGP_WORD_LEN

/* maximal size of signal and profile names */
#define NAME_SIZE 32

#define PROFILE_NUMBER_IAQ_INIT 0
#define PROFILE_NUMBER_IAQ_MEASURE 1
#define PROFILE_NUMBER_IAQ_GET_BASELINE 2
#define PROFILE_NUMBER_IAQ_GET_TVOC_INCEPTIVE_BASELINE 22
#define PROFILE_NUMBER_IAQ_SET_BASELINE 3
extern const uint8_t PROFILE_NUMBER_MEASURE_RAW_SIGNALS;
extern const uint8_t PROFILE_NUMBER_MEASURE_RAW;
extern const uint8_t PROFILE_NUMBER_SET_ABSOLUTE_HUMIDITY;
extern const uint8_t PROFILE_NUMBER_IAQ_INIT0;
extern const uint8_t PROFILE_NUMBER_IAQ_INIT16;
extern const uint8_t PROFILE_NUMBER_IAQ_INIT64;
extern const uint8_t PROFILE_NUMBER_IAQ_INIT184;
extern const uint8_t PROFILE_NUMBER_IAQ_INIT_CONTINUOUS;
extern const uint8_t PROFILE_NUMBER_IAQ_SET_TVOC_BASELINE;
extern const uint8_t PROFILE_NUMBER_SET_POWER_MODE;

/**
 * Check if chip featureset is compatible with driver featureset:
 * Check product type mask     0xF000,
 * ignore reserved bits        0x0E00,
 * eng. bit not set            0x0100,
 * major version matches       0x00E0,
 * minor version greater equal 0x001F */
#define SGP_FS_COMPAT(chip_fs, drv_fs)                                         \
    ((/* major version equal, minor version upwards compatible */              \
      (((drv_fs)&0xF000) == ((chip_fs)&0xF000)) &&                             \
      (((chip_fs)&0x0100) == 0) &&                                             \
      (((drv_fs)&0x00E0) == ((chip_fs)&0x00E0)) &&                             \
      (((chip_fs)&0x001F) >= ((drv_fs)&0x001F))))

/** Check if the chip's featureset is newer than or equal to the required one */
#define SGP_REQUIRE_FS(chip_fs, major, minor)                                  \
    ((/* major version equal, minor version upwards compatible */              \
      ((((chip_fs)&0x00E0) >> 5) == (major)) &&                                \
      (((chip_fs)&0x001F) >= (minor))))

struct sgp_signal {
    uint16_t (*conversion_function)(uint16_t);
    char name[NAME_SIZE];
};

struct sgp_profile {
    /* expected duration of measurement, i.e., when to return for data */
    const uint32_t duration_us;
    /* signals */
    const struct sgp_signal **signals;
    const uint16_t number_of_signals;
    const uint16_t command;
    const uint8_t number;
    const char name[NAME_SIZE];
};

struct sgp_otp_featureset {
    const struct sgp_profile **profiles;
    uint16_t number_of_profiles;
    const uint16_t *supported_featureset_versions;
    uint16_t number_of_supported_featureset_versions;
};

struct sgp_otp_supported_featuresets {
    const struct sgp_otp_featureset **featuresets;
    uint16_t number_of_supported_featuresets;
};

extern const struct sgp_otp_supported_featuresets sgp_supported_featuresets;

#ifdef __cplusplus
}
#endif

#endif /* SGP_FEATURESET_H */

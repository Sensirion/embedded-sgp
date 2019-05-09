/*
 * Copyright (c) 2017, Sensirion AG
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

#include "sensirion_common.h"
#include "sgp30.h"
#include "sgp_featureset.h"
#include "sgp_git_version.h"
#include "sht.h"

#define T_LO (-20000)
#define T_HI 70000
static const uint32_t AH_LUT_100RH[] = {1078,  2364,  4849,  9383,   17243,
                                        30264, 50983, 82785, 130048, 198277};
static const uint32_t T_STEP = (T_HI - T_LO) / (ARRAY_SIZE(AH_LUT_100RH) - 1);

static void svm_compensate_rht(int32_t *temperature, int32_t *humidity) {
    *temperature = ((*temperature * 8225) >> 13) - 500;
    *humidity = (*humidity * 8397) >> 13;
}

/**
 * Convert relative humidity [%RH*1000] and temperature [mC] to
 * absolute humidity [mg/m^3]
 */
static uint32_t sensirion_calc_absolute_humidity(const int32_t *temperature,
                                                 const int32_t *humidity) {
    uint32_t t, i, rem, norm_humi, ret;

    if (*humidity == 0)
        return 0;

    norm_humi = ((uint32_t)*humidity * 82) >> 13;
    t = *temperature - T_LO;
    i = t / T_STEP;
    rem = t % T_STEP;

    if (i >= ARRAY_SIZE(AH_LUT_100RH) - 1) {
        ret = AH_LUT_100RH[ARRAY_SIZE(AH_LUT_100RH) - 1];

    } else if (rem == 0) {
        ret = AH_LUT_100RH[i];

    } else {
        ret = (AH_LUT_100RH[i] +
               ((AH_LUT_100RH[i + 1] - AH_LUT_100RH[i]) * rem / T_STEP));
    }
    return ret * norm_humi / 1000;
}

/**
 * svm_get_driver_version() - Return the driver version
 * Return:  Driver version string
 */
const char *svm_get_driver_version() {
    return SGP_DRV_VERSION_STR;
}

/**
 * svm_measure_iaq_blocking_read() - Measure IAQ concentrations tVOC, CO2-Eq.
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 * @co2_eq_ppm: The CO2-Equivalent ppm value will be written to this location
 *
 * The profile is executed synchronously.
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
int16_t svm_measure_iaq_blocking_read(uint16_t *tvoc_ppb, uint16_t *co2_eq_ppm,
                                      int32_t *temperature, int32_t *humidity) {
    uint32_t absolute_humidity;
    uint16_t sgp_feature_set;
    uint8_t sgp_product_type;
    int16_t err;

    err = sht_measure_blocking_read(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    sgp30_get_feature_set_version(&sgp_feature_set, &sgp_product_type);
    if (SGP_REQUIRE_FS(sgp_feature_set, 1, 0)) {
        absolute_humidity =
            sensirion_calc_absolute_humidity(temperature, humidity);
        sgp30_set_absolute_humidity(absolute_humidity);
    }

    svm_compensate_rht(temperature, humidity);

    err = sgp30_measure_iaq_blocking_read(tvoc_ppb, co2_eq_ppm);
    if (err != STATUS_OK)
        return err;

    return STATUS_OK;
}

/**
 * svm_measure_raw_blocking_read() - Measure raw signals
 *
 * The output values are written to the memory locations passed as parameters:
 * @ethanol_raw_signal: The ethanol signal
 * @h2_raw_signal:      The h2 signal
 * @temperature:        Temperature in [degree Celsius] multiplied by 1000
 * @humidity:           Relative humidity in [%RH (0..100)] multiplied by 1000
 *
 * The profile is executed synchronously.
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
int16_t svm_measure_raw_blocking_read(uint16_t *ethanol_raw_signal,
                                      uint16_t *h2_raw_signal,
                                      int32_t *temperature, int32_t *humidity) {
    uint32_t absolute_humidity;
    uint16_t sgp_feature_set;
    uint8_t sgp_product_type;
    int16_t err;

    err = sht_measure_blocking_read(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    sgp30_get_feature_set_version(&sgp_feature_set, &sgp_product_type);
    if (SGP_REQUIRE_FS(sgp_feature_set, 1, 0)) {
        absolute_humidity =
            sensirion_calc_absolute_humidity(temperature, humidity);
        sgp30_set_absolute_humidity(absolute_humidity);
    }

    err = sgp30_measure_raw_blocking_read(ethanol_raw_signal, h2_raw_signal);
    if (err != STATUS_OK)
        return err;

    return STATUS_OK;
}

/**
 * svm_probe() - check if an SVM30 module is available and initialize it
 *
 * This call aleady initializes the IAQ baselines (sgp30_iaq_init())
 *
 * Return:  STATUS_OK on success.
 */
int16_t svm_probe() {
    int16_t err;

    err = sht_probe();
    if (err != STATUS_OK)
        return err;

    return sgp30_probe();
}

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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sensirion_common.h"
#include "sgp30.h"
#include "sht.h"

#define SVM_DRV_VERSION_STR             "3.0.0"

#define T_LO (-20000)
#define T_HI 70000
static const u32 AH_LUT_100RH[] = {
    1078, 2364, 4849, 9383, 17243, 30264, 50983, 82785, 130048, 198277
};
static const u32 T_STEP = (T_HI - T_LO) / (ARRAY_SIZE(AH_LUT_100RH) - 1);

static void svm_compensate_rht(s32 *temperature, s32 *humidity) {
    *temperature = ((*temperature * 8225) >> 13) - 500;
    *humidity = (*humidity * 8397) >> 13;
}

/**
 * Convert relative humidity [%RH*1000] and temperature [mC] to
 * absolute humidity [mg/m^3]
 */
static u32 sensirion_calc_absolute_humidity(const s32 *temperature,
                                            const s32 *humidity) {
    u32 t, i, rem, norm_humi, ret;

    if (*humidity == 0)
        return 0;

    norm_humi = ((u32)*humidity * 82) >> 13;
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
const char *svm_get_driver_version()
{
    return SVM_DRV_VERSION_STR;
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
s16 svm_measure_iaq_blocking_read(u16 *tvoc_ppb, u16 *co2_eq_ppm,
                                  s32 *temperature, s32 *humidity) {
    u32 absolute_humidity;
    u16 sgp_feature_set;
    u8 sgp_product_type;
    s16 err;

    err = sht_measure_blocking_read(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    sgp_get_feature_set_version(&sgp_feature_set, &sgp_product_type);
    if (sgp_feature_set >= 0x20) {
        absolute_humidity = sensirion_calc_absolute_humidity(temperature, humidity);
        sgp_set_absolute_humidity(absolute_humidity);
    }

    svm_compensate_rht(temperature, humidity);

    err = sgp_measure_iaq_blocking_read(tvoc_ppb, co2_eq_ppm);
    if (err != STATUS_OK)
        return err;

    return STATUS_OK;
}

/**
 * svm_probe() - check if an SVM30 module is available and initialize it
 *
 * This call aleady initializes the IAQ baselines (sgp_iaq_init())
 *
 * Return:  STATUS_OK on success.
 */
s16 svm_probe() {
    s16 err;

    err = sht_probe();
    if (err != STATUS_OK)
        return err;

    return sgp_probe();
}

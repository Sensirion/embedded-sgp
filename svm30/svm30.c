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

#include "svm30.h"
#include "sensirion_common.h"
#include "sensirion_humidity_conversion.h"
#include "sgp30.h"
#include "sgp_git_version.h"
#include "shtc1.h"

static void svm_compensate_rht(int32_t* temperature, int32_t* humidity) {
    *temperature = ((*temperature * 8225) >> 13) - 500;
    *humidity = (*humidity * 8397) >> 13;
}

static int16_t svm_set_humidity(const int32_t* temperature,
                                const int32_t* humidity) {
    uint32_t absolute_humidity;

    absolute_humidity =
        sensirion_calc_absolute_humidity(*temperature, *humidity);

    if (absolute_humidity == 0)
        absolute_humidity = 1; /* avoid disabling humidity compensation */

    return sgp30_set_absolute_humidity(absolute_humidity);
}

const char* svm_get_driver_version() {
    return SGP_DRV_VERSION_STR;
}

int16_t svm_measure_iaq_blocking_read(uint16_t* tvoc_ppb, uint16_t* co2_eq_ppm,
                                      int32_t* temperature, int32_t* humidity) {
    int16_t err;

    err = shtc1_measure_blocking_read(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    err = svm_set_humidity(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    svm_compensate_rht(temperature, humidity);

    err = sgp30_measure_iaq_blocking_read(tvoc_ppb, co2_eq_ppm);
    if (err != STATUS_OK)
        return err;

    return STATUS_OK;
}

int16_t svm_measure_raw_blocking_read(uint16_t* ethanol_raw_signal,
                                      uint16_t* h2_raw_signal,
                                      int32_t* temperature, int32_t* humidity) {
    int16_t err;

    err = shtc1_measure_blocking_read(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    err = svm_set_humidity(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    err = sgp30_measure_raw_blocking_read(ethanol_raw_signal, h2_raw_signal);
    if (err != STATUS_OK)
        return err;

    return STATUS_OK;
}

int16_t svm_probe() {
    int16_t err;

    err = shtc1_probe();
    if (err != STATUS_OK)
        return err;

    return sgp30_probe();
}

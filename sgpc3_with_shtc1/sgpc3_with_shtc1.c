/*
 * Copyright (c) 2020, Sensirion AG
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
#include "sensirion_humidity_conversion.h"
#include "sgp_git_version.h"
#include "sgpc3.h"
#include "shtc1.h"

const char* sgpc3_with_shtc1_get_driver_version() {
    return SGP_DRV_VERSION_STR;
}

int16_t sgpc3_with_shtc1_measure_iaq_blocking_read(uint16_t* tvoc_ppb,
                                                   int32_t* temperature,
                                                   int32_t* humidity) {
    uint32_t absolute_humidity;
    uint16_t sgp_feature_set;
    uint8_t sgp_product_type;
    int16_t err;

    err = shtc1_measure_blocking_read(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    sgpc3_get_feature_set_version(&sgp_feature_set, &sgp_product_type);
    absolute_humidity =
        sensirion_calc_absolute_humidity(*temperature, *humidity);
    sgpc3_set_absolute_humidity(absolute_humidity);

    err = sgpc3_measure_tvoc_blocking_read(tvoc_ppb);
    if (err != STATUS_OK)
        return err;

    return STATUS_OK;
}

int16_t sgpc3_with_shtc1_measure_raw_blocking_read(uint16_t* ethanol_raw_signal,
                                                   int32_t* temperature,
                                                   int32_t* humidity) {
    uint32_t absolute_humidity;
    uint16_t sgp_feature_set;
    uint8_t sgp_product_type;
    int16_t err;

    err = shtc1_measure_blocking_read(temperature, humidity);
    if (err != STATUS_OK)
        return err;

    sgpc3_get_feature_set_version(&sgp_feature_set, &sgp_product_type);
    absolute_humidity =
        sensirion_calc_absolute_humidity(*temperature, *humidity);
    sgpc3_set_absolute_humidity(absolute_humidity);

    err = sgpc3_measure_raw_blocking_read(ethanol_raw_signal);
    if (err != STATUS_OK)
        return err;

    return STATUS_OK;
}

int16_t sgpc3_with_shtc1_probe() {
    int16_t err;

    err = shtc1_probe();
    if (err != STATUS_OK)
        return err;

    return sgpc3_probe();
}

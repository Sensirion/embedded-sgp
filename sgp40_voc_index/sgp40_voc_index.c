/*
 * Copyright (c) 2019, Sensirion AG
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

#include "sgp40_voc_index.h"
#include "sensirion_arch_config.h"
#include "sensirion_voc_algorithm.h"
#include "sgp40.h"
#include "shtc1.h"

#ifdef __cplusplus
extern "C" {
#endif

static VocAlgorithmParams voc_algorithm_params;

int16_t sensirion_init_sensors() {
    int16_t ret;

    sensirion_i2c_init();

    ret = shtc1_probe();
    if (ret)
        return SENSIRION_SHT_PROBE_FAILED;

    ret = sgp40_probe();
    if (ret)
        return SENSIRION_SGP_PROBE_FAILED;

    VocAlgorithm_init(&voc_algorithm_params);
    return 0;
}

int16_t sensirion_measure_voc_index(int32_t* voc_index) {
    return sensirion_measure_voc_index_with_rh_t(voc_index, NULL, NULL);
}

int16_t sensirion_measure_voc_index_with_rh_t(int32_t* voc_index,
                                              int32_t* relative_humidity,
                                              int32_t* temperature) {
    int32_t int_temperature, int_humidity;
    int16_t ret;
    uint16_t sraw;

    ret = shtc1_measure_blocking_read(&int_temperature, &int_humidity);
    if (ret)
        return SENSIRION_GET_RHT_SIGNAL_FAILED;

    if (temperature) {
        *temperature = int_temperature;
    }
    if (relative_humidity) {
        *relative_humidity = int_humidity;
    }

    ret = sgp40_measure_raw_with_rht_blocking_read(int_humidity,
                                                   int_temperature, &sraw);
    if (ret) {
        return SENSIRION_GET_SGP_SIGNAL_FAILED;
    }

    VocAlgorithm_process(&voc_algorithm_params, sraw, voc_index);
    return 0;
}

#ifdef __cplusplus
}
#endif

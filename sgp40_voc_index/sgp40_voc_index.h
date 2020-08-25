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

#ifndef SGP40_WRAPPER_H
#define SGP40_WRAPPER_H
#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SENSIRION_SHT_PROBE_FAILED (-11)
#define SENSIRION_GET_RHT_SIGNAL_FAILED (-12)

#define SENSIRION_SGP_PROBE_FAILED (-21)
#define SENSIRION_GET_SGP_SIGNAL_FAILED (-22)
#define SENSIRION_SET_RHT_SIGNAL_FAILED (-23)

/**
 * Initialize the SGP40, SHT and VOC algorithm.
 *
 * @return STATUS_OK on success, an error code otherwise
 */
int16_t sensirion_init_sensors();

/**
 * Measure the humidity-compensated VOC Index.
 *
 * The measurement triggers a humidity reading, sets the value on the SGP for
 * humidity compensation and runs the gas signal through the VOC algorithm for
 * the final result.
 *
 * This command works like sensirion_measure_voc_index_with_rh_t() but does not
 * return the measured ambient humidity and temperature used for compensation.
 *
 * @param voc_index     Pointer to buffer for measured voc_index. Range 0..500.
 * @return              STATUS_OK on success, an error code otherwise
 */
int16_t sensirion_measure_voc_index(int32_t* voc_index);

/**
 * Measure the humidity-compensated VOC Index and ambient temperature and
 * relative humidity.
 *
 * This command works like sensirion_measure_voc_index() but also returns the
 * measured ambient humidity and temperature used for compensation.
 *
 * @param voc_index         Pointer to buffer for measured VOC index. Range
 *                          0..500.
 * @param relative_humidity Pointer to buffer for relative humidity in milli %RH
 * @param temperature       Pointer to buffer for measured temperature in milli
 *                          degree Celsius.
 * @return                  STATUS_OK on success, an error code otherwise
 */
int16_t sensirion_measure_voc_index_with_rh_t(int32_t* voc_index,
                                              int32_t* relative_humidity,
                                              int32_t* temperature);

#ifdef __cplusplus
}
#endif

#endif /* SGP40_WRAPPER_H */

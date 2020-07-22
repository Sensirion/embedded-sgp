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

#ifndef SVM30_H
#define SVM30_H
#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sgp30.h"
#include "shtc1.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * svm_get_driver_version() - Return the driver version
 * Return:  Driver version string
 */
const char* svm_get_driver_version();

/**
 * svm_probe() - check if an SVM30 module is available and initialize it
 *
 * This call aleady initializes the IAQ baselines (sgp30_iaq_init())
 *
 * Return:  STATUS_OK on success.
 */
int16_t svm_probe(void);

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
int16_t svm_measure_iaq_blocking_read(uint16_t* tvoc_ppb, uint16_t* co2_eq_ppm,
                                      int32_t* temperature, int32_t* humidity);

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
int16_t svm_measure_raw_blocking_read(uint16_t* ethanol_raw_signal,
                                      uint16_t* h2_raw_signal,
                                      int32_t* temperature, int32_t* humidity);

#ifdef __cplusplus
}
#endif

#endif /* SVM30_H */

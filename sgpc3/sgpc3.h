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

#ifndef SGPC3_H
#define SGPC3_H
#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"

#define SGPC3_ERR_INSUFFICIENT_FEATURE_SET (-11)

#ifdef __cplusplus
extern "C" {
#endif

int16_t sgpc3_probe(void);
int16_t sgpc3_tvoc_init_preheat(void);
int16_t sgpc3_tvoc_init_no_preheat(void);
int16_t sgpc3_tvoc_init_64s_fs5(void);

const char *sgpc3_get_driver_version(void);
uint8_t sgpc3_get_configured_address(void);
int16_t sgpc3_get_feature_set_version(uint16_t *feature_set_version,
                                      uint8_t *product_type);
int16_t sgpc3_get_serial_id(uint64_t *serial_id);

int16_t sgpc3_get_tvoc_baseline(uint16_t *baseline);
int16_t sgpc3_set_tvoc_baseline(uint16_t baseline);
int16_t sgpc3_get_tvoc_inceptive_baseline(uint16_t *tvoc_inceptive_baseline);

int16_t sgpc3_measure_tvoc_blocking_read(uint16_t *tvoc_ppb);
int16_t sgpc3_measure_tvoc(void);
int16_t sgpc3_read_tvoc(uint16_t *tvoc_ppb);

int16_t sgpc3_measure_raw_blocking_read(uint16_t *ethanol_raw_signal);
int16_t sgpc3_measure_raw(void);
int16_t sgpc3_read_raw(uint16_t *ethanol_raw_signal);

int16_t sgpc3_measure_tvoc_and_raw_blocking_read(uint16_t *tvoc_ppb,
                                                 uint16_t *ethanol_raw_signal);
int16_t sgpc3_measure_tvoc_and_raw(void);
int16_t sgpc3_read_tvoc_and_raw(uint16_t *tvoc_ppb,
                                uint16_t *ethanol_raw_signal);

int16_t sgpc3_set_power_mode(uint16_t power_mode);
int16_t sgpc3_set_absolute_humidity(uint32_t absolute_humidity);
int16_t sgpc3_measure_test(uint16_t *test_result);

#ifdef __cplusplus
}
#endif

#endif /* SGPC3_H */

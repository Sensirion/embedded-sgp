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

#define SGPC3_ERR_UNSUPPORTED_FEATURE_SET (-11)
#define SGPC3_ERR_INVALID_PRODUCT_TYPE (-13)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * sgpc3_probe() - check if SGP sensor is available and initialize it
 *
 * This call aleady initializes the TVOC baselines
 * (sgpc3_tvoc_init_no_preheat())
 *
 * Return:  STATUS_OK on success,
 *          SGPC3_ERR_INVALID_PRODUT_TYPE if the sensor is not an SGPC3,
 *          SGPC3_ERR_UNSUPPORTED_FEATURE_SET if the sensor's feature set
 *                                            is unknown or outdated,
 *          An error code otherwise
 */
int16_t sgpc3_probe(void);

/**
 * sgpc3_tvoc_init_preheat() - reset the SGP's internal TVOC baselines and
 *                             run accelerated startup until the first
 *                             sgpc3_measure_tvoc()
 *
 * Return:  STATUS_OK on success.
 */
int16_t sgpc3_tvoc_init_preheat(void);

/**
 * sgpc3_tvoc_init_no_preheat() - reset the SGP's internal TVOC baselines
 * without accelerated startup time
 *
 * Return:  STATUS_OK on success.
 */
int16_t sgpc3_tvoc_init_no_preheat(void);

/**
 * sgpc3_tvoc_init_64s_fs5() - reset the SGP's internal TVOC baselines using
 * accelerated startup time of 64s.
 *
 * Note: legacy command for old SGPC3 fs<=5: initializes algo, for only TVOC,
 * with preheating for 64s. Does not work for the ULP mode! See
 * sgpc3_set_power_mode()
 *
 * Return:  STATUS_OK on success.
 */
int16_t sgpc3_tvoc_init_64s_fs5(void);

/**
 * sgpc3_get_driver_version() - Return the driver version
 * Return:  Driver version string
 */
const char* sgpc3_get_driver_version(void);

/**
 * sgpc3_get_configured_address() - returns the configured I2C address
 *
 * Return:      uint8_t I2C address
 */
uint8_t sgpc3_get_configured_address(void);

/**
 * sgpc3_get_feature_set_version() - Retrieve the sensor's feature set version
 * and product type
 *
 * @feature_set_version:    The feature set version
 * @product_type:           The product type: 0 for sgp30, 1: sgpc3
 *
 * Return:  STATUS_OK on success
 */
int16_t sgpc3_get_feature_set_version(uint16_t* feature_set_version,
                                      uint8_t* product_type);

/**
 * sgpc3_get_serial_id() - Retrieve the sensor's serial id
 *
 * @serial_id:    Output variable for the serial id
 *
 * Return:  STATUS_OK on success
 */
int16_t sgpc3_get_serial_id(uint64_t* serial_id);

/**
 * sgpc3_get_tvoc_baseline() - read out the baseline from the chip
 *
 * The TVOC baseline should be retrieved and persisted for a faster sensor
 * startup. See sgpc3_set_tvoc_baseline() for further documentation.
 *
 * A valid baseline value is only returned approx. 60min after a call to
 * sgpc3_tvoc_init_preheat() when it is not followed by a call to
 * sgpc3_set_tvoc_baseline() with a valid baseline.
 * This functions returns STATUS_FAIL if the baseline value is not valid.
 *
 * @baseline:   Pointer to raw uint16_t where to store the baseline
 *              If the method returns STATUS_FAIL, the baseline value must be
 *              discarded and must not be passed to sgpc3_set_tvoc_baseline().
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_get_tvoc_baseline(uint16_t* baseline);

/**
 * sgpc3_set_tvoc_baseline() - set the on-chip baseline
 * @baseline:   A raw uint16_t baseline
 *              This value must be unmodified from what was retrieved by a
 *              successful call to sgpc3_get_tvoc_baseline() with return value
 *              STATUS_OK. A persisted baseline should not be set if it is
 *              older than one week.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_set_tvoc_baseline(uint16_t baseline);

/**
 * sgpc3_get_tvoc_inceptive_baseline() - read the chip's tVOC inceptive baseline
 *
 * The inceptive baseline must only be used on the very first startup of the
 * sensor. It ensures that measured concentrations are consistent with the air
 * quality even before the first clean air event.
 * Note that the inceptive baseline is dependent on the currently set
 * power-mode.
 *
 * @tvoc_inceptive_baseline:
 *              Pointer to raw uint16_t where to store the inceptive baseline
 *              If the method returns STATUS_FAIL, the inceptive baseline value
 *              must be discarded and must not be passed to
 *              sgpc3_set_tvoc_baseline().
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_get_tvoc_inceptive_baseline(uint16_t* tvoc_inceptive_baseline);

/**
 * sgpc3_measure_tvoc_blocking_read() - Measure tVOC concentration
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 *
 * The profile is executed synchronously.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_tvoc_blocking_read(uint16_t* tvoc_ppb);

/**
 * sgpc3_measure_tvoc() - Measure tVOC values async
 *
 * The profile is executed asynchronously. Use sgpc3_read_tvoc to get the
 * values.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_tvoc(void);

/**
 * sgpc3_read_tvoc() - Read tVOC values async
 *
 * Read the tVOC values. This command can only be exectued after a measurement
 * has started with sgpc3_measure_tvoc and is finished.
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_read_tvoc(uint16_t* tvoc_ppb);

/**
 * sgpc3_measure_raw_blocking_read() - Measure raw signals
 * The profile is executed synchronously.
 *
 * @ethanol_raw_signal: Output variable for the ethanol raw signal
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_raw_blocking_read(uint16_t* ethanol_raw_signal);

/**
 * sgpc3_measure_raw() - Measure raw signals async
 *
 * The profile is executed asynchronously. Use sgpc3_read_raw to get
 * the signal values.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_raw(void);

/**
 * sgpc3_read_raw() - Read raw signals async
 * This command can only be exectued after a measurement has been started with
 * sgpc3_measure_raw and has finished.
 *
 * @ethanol_raw_signal: Output variable for ethanol signal.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_read_raw(uint16_t* ethanol_raw_signal);

/**
 * sgpc3_measure_tvoc_and_raw_blocking_read() - Measure tvoc and raw signals
 * The profile is executed synchronously.
 *
 * @tvoc_ppb:           The tVOC ppb value will be written to this location
 * @ethanol_raw_signal: Output variable for the ethanol raw signal
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_tvoc_and_raw_blocking_read(uint16_t* tvoc_ppb,
                                                 uint16_t* ethanol_raw_signal);

/**
 * sgpc3_measure_tvoc_and_raw() - Measure raw async
 *
 * The profile is executed asynchronously. Use sgpc3_read_tvoc_and_raw to get
 * the tvoc and raw signal values.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_tvoc_and_raw(void);

/**
 * sgpc3_read_tvoc_and_raw() - Read tvoc and raw signals async
 * This command can only be exectued after a measurement has been started with
 * sgpc3_measure_tvoc_and_raw and has finished.
 *
 * @tvoc_ppb:           The tVOC ppb value will be written to this location
 * @ethanol_raw_signal: Output variable for ethanol raw signal.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_read_tvoc_and_raw(uint16_t* tvoc_ppb,
                                uint16_t* ethanol_raw_signal);

/**
 * sgpc3_set_power_mode() - set the power mode
 *
 * The measurement interval for both TVOC and ethanol measurements changes
 * according to the power mode. See application notes for further
 * documentation.
 *
 * This functions returns STATUS_FAIL if power mode switching is not
 * available.
 *
 * @power_mode: Power mode to set:
 *              0: ULP (ultra low power mode 30s measurement interval)
 *              1: (Default) LP (low power mode 2s measurement interval)
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_set_power_mode(uint16_t power_mode);

/**
 * sgpc3_set_absolute_humidity() - set the absolute humidity for compensation
 *
 * The absolute humidity must be provided in mg/m^3 and the value must be
 * between 0 and 256000 mg/m^3.
 * If the absolute humidity is set to zero, humidity compensation is disabled.
 *
 * @absolute_humidity:      absolute humidity in mg/m^3
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_set_absolute_humidity(uint32_t absolute_humidity);

/**
 * sgpc3_measure_test() - Run the on-chip self-test
 *
 * This method is executed synchronously and blocks for the duration of the
 * measurement (~220ms)
 *
 * @test_result:    Allocated buffer to store the chip's error code.
 *                  test_result is SGPC3_CMD_MEASURE_TEST_OK on success or set
 *                  to zero (0) in the case of a communication error.
 *
 * Return: STATUS_OK on a successful self-test, an error code otherwise
 */
int16_t sgpc3_measure_test(uint16_t* test_result);

#ifdef __cplusplus
}
#endif

#endif /* SGPC3_H */

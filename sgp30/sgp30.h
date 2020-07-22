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

#ifndef SGP30_H
#define SGP30_H
#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"

#define SGP30_ERR_UNSUPPORTED_FEATURE_SET (-10)
#define SGP30_ERR_INVALID_PRODUCT_TYPE (-12)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * sgp30_probe() - check if SGP sensor is available and initialize it
 *
 * This call aleady initializes the IAQ baselines (sgp30_iaq_init())
 *
 * Return:  STATUS_OK on success,
 *          SGP30_ERR_INVALID_PRODUCT_TYPE if the sensor is not an SGP30,
 *          SGP30_ERR_UNSUPPORTED_FEATURE_SET if the sensor's feature set
 *                                            is unknown or outdated,
 *          An error code otherwise
 */
int16_t sgp30_probe(void);

/**
 * sgp30_iaq_init() - reset the SGP's internal IAQ baselines
 *
 * Return:  STATUS_OK on success.
 */
int16_t sgp30_iaq_init(void);

/**
 * sgp30_get_driver_version() - Return the driver version
 * Return:  Driver version string
 */
const char* sgp30_get_driver_version(void);

/**
 * sgp30_get_configured_address() - returns the configured I2C address
 *
 * Return:      uint8_t I2C address
 */
uint8_t sgp30_get_configured_address(void);

/**
 * sgp30_get_feature_set_version() - Retrieve the sensor's feature set version
 * and product type
 *
 * @feature_set_version:    The feature set version
 * @product_type:           The product type: 0 for sgp30, 1: sgpc3
 *
 * Return:  STATUS_OK on success
 */
int16_t sgp30_get_feature_set_version(uint16_t* feature_set_version,
                                      uint8_t* product_type);

/**
 * sgp30_get_serial_id() - Retrieve the sensor's serial id
 *
 * @serial_id:    Output variable for the serial id
 *
 * Return:  STATUS_OK on success
 */
int16_t sgp30_get_serial_id(uint64_t* serial_id);

/**
 * sgp30_get_iaq_baseline() - read out the baseline from the chip
 *
 * The IAQ baseline should be retrieved and persisted for a faster sensor
 * startup. See sgp30_set_iaq_baseline() for further documentation.
 *
 * A valid baseline value is only returned approx. 60min after a call to
 * sgp30_iaq_init() when it is not followed by a call to
 * sgp30_set_iaq_baseline() with a valid baseline. This functions returns
 * STATUS_FAIL if the baseline value is not valid.
 *
 * @baseline:   Pointer to raw uint32_t where to store the baseline
 *              If the method returns STATUS_FAIL, the baseline value must be
 *              discarded and must not be passed to sgp30_set_iaq_baseline().
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
int16_t sgp30_get_iaq_baseline(uint32_t* baseline);

/**
 * sgp30_set_iaq_baseline() - set the on-chip baseline
 * @baseline:   A raw uint32_t baseline
 *              This value must be unmodified from what was retrieved by a
 *              successful call to sgp30_get_iaq_baseline() with return value
 *              STATUS_OK. A persisted baseline should not be set if it is
 *              older than one week.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_set_iaq_baseline(uint32_t baseline);

/**
 * sgp30_get_tvoc_inceptive_baseline() - read the chip's tVOC inceptive baseline
 *
 * The inceptive baseline must only be used on the very first startup of the
 * sensor. It ensures that measured concentrations are consistent with the air
 * quality even before the first clean air event.
 *
 * @tvoc_inceptive_baseline:
 *              Pointer to raw uint16_t where to store the inceptive baseline
 *              If the method returns STATUS_FAIL, the inceptive baseline value
 *              must be discarded and must not be passed to
 *              sgp30_set_tvoc_baseline().
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_get_tvoc_inceptive_baseline(uint16_t* tvoc_inceptive_baseline);

/**
 * sgp30_set_tvoc_baseline() - set the on-chip tVOC baseline
 * @baseline:   A raw uint16_t tVOC baseline
 *              This value must be unmodified from what was retrieved by a
 *              successful call to sgp30_get_tvoc_inceptive_baseline() with
 * return value STATUS_OK.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_set_tvoc_baseline(uint16_t tvoc_baseline);

/**
 * sgp30_measure_iaq_blocking_read() - Measure IAQ concentrations tVOC, CO2-Eq.
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 * @co2_eq_ppm: The CO2-Equivalent ppm value will be written to this location
 *
 * The profile is executed synchronously.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_measure_iaq_blocking_read(uint16_t* tvoc_ppb,
                                        uint16_t* co2_eq_ppm);
/**
 * sgp30_measure_iaq() - Measure IAQ values async
 *
 * The profile is executed asynchronously. Use sgp30_read_iaq to get the values.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_measure_iaq(void);

/**
 * sgp30_read_iaq() - Read IAQ values async
 *
 * Read the IAQ values. This command can only be exectued after a measurement
 * has started with sgp30_measure_iaq and is finished.
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 * @co2_eq_ppm: The CO2-Equivalent ppm value will be written to this location
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_read_iaq(uint16_t* tvoc_ppb, uint16_t* co2_eq_ppm);

/**
 * sgp30_measure_tvoc_blocking_read() - Measure tVOC concentration
 *
 * The profile is executed synchronously.
 *
 * Return:  tVOC concentration in ppb. Negative if it fails.
 */
int16_t sgp30_measure_tvoc_blocking_read(uint16_t* tvoc_ppb);

/**
 * sgp30_measure_tvoc() - Measure tVOC concentration async
 *
 * The profile is executed asynchronously. Use sgp30_read_tvoc to get the
 * ppb value.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_measure_tvoc(void);

/**
 * sgp30_read_tvoc() - Read tVOC concentration async
 *
 * Read the tVOC value. This command can only be exectued after a measurement
 * has started with sgp30_measure_tvoc and is finished.
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_read_tvoc(uint16_t* tvoc_ppb);

/**
 * sgp30_measure_co2_eq_blocking_read() - Measure CO2-Equivalent concentration
 *
 * The profile is executed synchronously.
 *
 * Return:  CO2-Equivalent concentration in ppm. Negative if it fails.
 */
int16_t sgp30_measure_co2_eq_blocking_read(uint16_t* co2_eq_ppm);

/**
 * sgp30_measure_co2_eq() - Measure tVOC concentration async
 *
 * The profile is executed asynchronously. Use sgp30_read_co2_eq to get the
 * ppm value.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_measure_co2_eq(void);

/**
 * sgp30_read_co2_eq() - Read CO2-Equivalent concentration async
 *
 * Read the CO2-Equivalent value. This command can only be exectued after a
 * measurement was started with sgp30_measure_co2_eq() and is finished.
 *
 * @co2_eq_ppm: The CO2-Equivalent ppm value will be written to this location
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_read_co2_eq(uint16_t* co2_eq_ppm);

/**
 * sgp30_measure_raw_blocking_read() - Measure raw signals
 * The profile is executed synchronously.
 *
 * @ethanol_raw_signal: Output variable for the ethanol raw signal
 * @h2_raw_signal:      Output variable for the h2 raw signal
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_measure_raw_blocking_read(uint16_t* ethanol_raw_signal,
                                        uint16_t* h2_raw_signal);

/**
 * sgp30_measure_raw() - Measure raw signals async
 *
 * The profile is executed asynchronously. Use sgp30_read_raw to get the
 * signal values.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_measure_raw(void);

/**
 * sgp30_read_raw() - Read raw signals async
 * This command can only be exectued after a measurement started with
 * sgp30_measure_raw and has finished.
 *
 * @ethanol_raw_signal: Output variable for ethanol raw signal.
 * @h2_raw_signal: Output variable for h2 raw signal.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_read_raw(uint16_t* ethanol_raw_signal, uint16_t* h2_raw_signal);

/**
 * sgp30_measure_test() - Run the on-chip self-test
 *
 * This method is executed synchronously and blocks for the duration of the
 * measurement (~220ms)
 *
 * @test_result:    Allocated buffer to store the chip's error code.
 *                  test_result is SGP30_CMD_MEASURE_TEST_OK on success or set
 *                  to zero (0) in the case of a communication error.
 *
 * Return: STATUS_OK on a successful self-test, an error code otherwise
 */
int16_t sgp30_measure_test(uint16_t* test_result);

/**
 * sgp30_set_absolute_humidity() - set the absolute humidity for compensation
 *
 * The absolute humidity must be provided in mg/m^3 and the value must be
 * between 0 and 256000 mg/m^3.
 * If the absolute humidity is set to zero, humidity compensation is disabled.
 *
 * @absolute_humidity:      absolute humidity in mg/m^3
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgp30_set_absolute_humidity(uint32_t absolute_humidity);

#ifdef __cplusplus
}
#endif

#endif /* SGP30_H */

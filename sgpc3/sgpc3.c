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

#include "sgpc3.h"
#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"
#include "sgp_git_version.h"

static const uint8_t SGPC3_I2C_ADDRESS = 0x58;

/* command and constants for reading the serial ID */
#define SGPC3_CMD_GET_SERIAL_ID 0x3682
#define SGPC3_CMD_GET_SERIAL_ID_DURATION_US 500
#define SGPC3_CMD_GET_SERIAL_ID_WORDS 3

/* command and constants for reading the featureset version */
#define SGPC3_CMD_GET_FEATURESET 0x202f
#define SGPC3_CMD_GET_FEATURESET_DURATION_US 1000
#define SGPC3_CMD_GET_FEATURESET_WORDS 1

/* command and constants for on-chip self-test */
#define SGPC3_CMD_MEASURE_TEST 0x2032
#define SGPC3_CMD_MEASURE_TEST_DURATION_US 220000
#define SGPC3_CMD_MEASURE_TEST_WORDS 1
#define SGPC3_CMD_MEASURE_TEST_OK 0xd400

/* command and constants for IAQ init 0 */
#define SGPC3_CMD_IAQ_INIT_0 0x2024
#define SGPC3_CMD_IAQ_INIT_0_DURATION_US 10000

/* command and constants for IAQ init 64 */
#define SGPC3_CMD_IAQ_INIT_64 0x2003
#define SGPC3_CMD_IAQ_INIT_64_DURATION_US 10000

/* command and constants for IAQ init continuous */
#define SGPC3_CMD_IAQ_INIT_CON 0x20ae
#define SGPC3_CMD_IAQ_INIT_CON_DURATION_US 10000

/* command and constants for IAQ measure */
#define SGPC3_CMD_IAQ_MEASURE 0x2008
#define SGPC3_CMD_IAQ_MEASURE_DURATION_US 50000
#define SGPC3_CMD_IAQ_MEASURE_WORDS 1

/* command and constants for getting IAQ baseline */
#define SGPC3_CMD_GET_IAQ_BASELINE 0x2015
#define SGPC3_CMD_GET_IAQ_BASELINE_DURATION_US 10000
#define SGPC3_CMD_GET_IAQ_BASELINE_WORDS 1

/* command and constants for setting IAQ baseline */
#define SGPC3_CMD_SET_IAQ_BASELINE 0x201e
#define SGPC3_CMD_SET_IAQ_BASELINE_DURATION_US 10000

/* command and constants for getting IAQ inceptive baseline */
#define SGPC3_CMD_GET_IAQ_INCEPTIVE_BASELINE 0x20b3
#define SGPC3_CMD_GET_IAQ_INCEPTIVE_BASELINE_DURATION_US 10000
#define SGPC3_CMD_GET_IAQ_INCEPTIVE_BASELINE_WORDS 1

/* command and constants for raw measure */
#define SGPC3_CMD_RAW_MEASURE 0x204d
#define SGPC3_CMD_RAW_MEASURE_DURATION_US 50000
#define SGPC3_CMD_RAW_MEASURE_WORDS 1

/* command and constants for IAQ raw measure */
#define SGPC3_CMD_IAQ_RAW_MEASURE 0x2046
#define SGPC3_CMD_IAQ_RAW_MEASURE_DURATION_US 50000
#define SGPC3_CMD_IAQ_RAW_MEASURE_WORDS 2

/* command and constants for setting absolute humidity */
#define SGPC3_CMD_SET_ABSOLUTE_HUMIDITY 0x2061
#define SGPC3_CMD_SET_ABSOLUTE_HUMIDITY_DURATION_US 10000

/* command and constants for setting power mode */
#define SGPC3_CMD_SET_POWER_MODE 0x209f
#define SGPC3_CMD_SET_POWER_MODE_DURATION_US 10000

/**
 * sgpc3_check_featureset() - Check if the connected sensor has a certain FS
 *
 * @needed_fs: The featureset that is required
 *
 * Return: STATUS_OK if the sensor has the required FS,
 *         SGPC3_ERR_INSUFFICIENT_FEATURE_SET if the sensor does not
 *                                            have the required FS,
 *         an error code otherwise
 */
static int16_t sgpc3_check_featureset(uint16_t needed_fs) {
    int16_t ret;
    uint16_t fs_version;
    uint8_t product_type;

    ret = sgpc3_get_feature_set_version(&fs_version, &product_type);
    if (ret != STATUS_OK)
        return ret;

    if (fs_version < needed_fs)
        return SGPC3_ERR_INSUFFICIENT_FEATURE_SET;

    return STATUS_OK;
}

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
int16_t sgpc3_measure_test(uint16_t *test_result) {
    uint16_t measure_test_word_buf[SGPC3_CMD_MEASURE_TEST_WORDS];
    int16_t ret;

    *test_result = 0;

    ret = sensirion_i2c_delayed_read_cmd(
        SGPC3_I2C_ADDRESS, SGPC3_CMD_MEASURE_TEST,
        SGPC3_CMD_MEASURE_TEST_DURATION_US, measure_test_word_buf,
        SENSIRION_NUM_WORDS(measure_test_word_buf));
    if (ret != STATUS_OK)
        return ret;

    *test_result = *measure_test_word_buf;
    if (*test_result == SGPC3_CMD_MEASURE_TEST_OK)
        return STATUS_OK;

    return STATUS_FAIL;
}

/**
 * sgpc3_measure_tvoc() - Measure tVOC values async
 *
 * The profile is executed asynchronously. Use sgpc3_read_tvoc to get the
 * values.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_tvoc() {
    return sensirion_i2c_write_cmd(SGPC3_I2C_ADDRESS, SGPC3_CMD_IAQ_MEASURE);
}

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
int16_t sgpc3_read_tvoc(uint16_t *tvoc_ppb) {
    int16_t ret;
    uint16_t words[SGPC3_CMD_IAQ_MEASURE_WORDS];

    ret = sensirion_i2c_read_words(SGPC3_I2C_ADDRESS, words,
                                   SGPC3_CMD_IAQ_MEASURE_WORDS);

    *tvoc_ppb = words[0];

    return ret;
}

/**
 * sgpc3_measure_tvoc_blocking_read() - Measure tVOC concentration
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 *
 * The profile is executed synchronously.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_tvoc_blocking_read(uint16_t *tvoc_ppb) {
    int16_t ret;

    ret = sgpc3_measure_tvoc();
    if (ret != STATUS_OK)
        return ret;

    sensirion_sleep_usec(SGPC3_CMD_IAQ_MEASURE_DURATION_US);

    return sgpc3_read_tvoc(tvoc_ppb);
}

/**
 * sgpc3_measure_raw_blocking_read() - Measure raw signals
 * The profile is executed synchronously.
 *
 * @ethanol_raw_signal: Output variable for the ethanol raw signal
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_raw_blocking_read(uint16_t *ethanol_raw_signal) {
    int16_t ret;

    ret = sgpc3_measure_raw();
    if (ret != STATUS_OK)
        return STATUS_FAIL;

    sensirion_sleep_usec(SGPC3_CMD_RAW_MEASURE_DURATION_US);

    return sgpc3_read_raw(ethanol_raw_signal);
}

/**
 * sgpc3_measure_raw() - Measure raw signals async
 *
 * The profile is executed asynchronously. Use sgpc3_read_raw to get
 * the signal values.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_raw(void) {
    return sensirion_i2c_write_cmd(SGPC3_I2C_ADDRESS, SGPC3_CMD_RAW_MEASURE);
}

/**
 * sgpc3_read_raw() - Read raw signals async
 * This command can only be exectued after a measurement has been started with
 * sgpc3_measure_raw and has finished.
 *
 * @ethanol_raw_signal: Output variable for ethanol signal.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_read_raw(uint16_t *ethanol_raw_signal) {
    int16_t ret;
    uint16_t words[SGPC3_CMD_RAW_MEASURE_WORDS];

    ret = sensirion_i2c_read_words(SGPC3_I2C_ADDRESS, words,
                                   SGPC3_CMD_RAW_MEASURE_WORDS);

    *ethanol_raw_signal = words[0];

    return ret;
}

/**
 * sgpc3_measure_tvoc_and_raw_blocking_read() - Measure tvoc and raw signals
 * The profile is executed synchronously.
 *
 * @tvoc_ppb:           The tVOC ppb value will be written to this location
 * @ethanol_raw_signal: Output variable for the ethanol raw signal
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_tvoc_and_raw_blocking_read(uint16_t *tvoc_ppb,
                                                 uint16_t *ethanol_raw_signal) {
    int16_t ret;

    ret = sgpc3_measure_tvoc_and_raw();
    if (ret != STATUS_OK)
        return ret;

    sensirion_sleep_usec(SGPC3_CMD_IAQ_RAW_MEASURE_DURATION_US);

    return sgpc3_read_tvoc_and_raw(tvoc_ppb, ethanol_raw_signal);
}

/**
 * sgpc3_measure_tvoc_and_raw() - Measure raw async
 *
 * The profile is executed asynchronously. Use sgpc3_read_tvoc_and_raw to get
 * the tvoc and raw signal values.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_measure_tvoc_and_raw() {
    return sensirion_i2c_write_cmd(SGPC3_I2C_ADDRESS,
                                   SGPC3_CMD_IAQ_RAW_MEASURE);
}

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
int16_t sgpc3_read_tvoc_and_raw(uint16_t *tvoc_ppb,
                                uint16_t *ethanol_raw_signal) {
    int16_t ret;
    uint16_t words[SGPC3_CMD_IAQ_RAW_MEASURE_WORDS];

    ret = sensirion_i2c_read_words(SGPC3_I2C_ADDRESS, words,
                                   SGPC3_CMD_IAQ_RAW_MEASURE_WORDS);

    *tvoc_ppb = words[1];
    *ethanol_raw_signal = words[0];

    return ret;
}

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
int16_t sgpc3_get_tvoc_baseline(uint16_t *baseline) {
    int16_t ret;
    uint16_t words[SGPC3_CMD_GET_IAQ_BASELINE_WORDS];

    ret = sensirion_i2c_delayed_read_cmd(
        SGPC3_I2C_ADDRESS, SGPC3_CMD_GET_IAQ_BASELINE,
        SGPC3_CMD_GET_IAQ_BASELINE_DURATION_US, words,
        SGPC3_CMD_GET_IAQ_BASELINE_WORDS);

    if (ret != STATUS_OK)
        return ret;

    *baseline = words[0];

    if (*baseline)
        return STATUS_OK;
    return STATUS_FAIL;
}

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
int16_t sgpc3_set_tvoc_baseline(uint16_t baseline) {
    int16_t ret;

    if (!baseline)
        return STATUS_FAIL;

    ret = sensirion_i2c_write_cmd_with_args(
        SGPC3_I2C_ADDRESS, SGPC3_CMD_SET_IAQ_BASELINE, &baseline,
        SENSIRION_NUM_WORDS(baseline));

    sensirion_sleep_usec(SGPC3_CMD_SET_IAQ_BASELINE_DURATION_US);

    return ret;
}

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
int16_t sgpc3_get_tvoc_inceptive_baseline(uint16_t *tvoc_inceptive_baseline) {
    int16_t ret;

    ret = sgpc3_check_featureset(5);

    if (ret != STATUS_OK)
        return ret;

    return sensirion_i2c_delayed_read_cmd(
        SGPC3_I2C_ADDRESS, SGPC3_CMD_GET_IAQ_INCEPTIVE_BASELINE,
        SGPC3_CMD_GET_IAQ_INCEPTIVE_BASELINE_DURATION_US,
        tvoc_inceptive_baseline, SGPC3_CMD_GET_IAQ_INCEPTIVE_BASELINE_WORDS);
}

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
int16_t sgpc3_set_absolute_humidity(uint32_t absolute_humidity) {
    int16_t ret;
    uint16_t ah_scaled;

    ret = sgpc3_check_featureset(6);

    if (ret != STATUS_OK)
        return ret;

    if (absolute_humidity > 256000)
        return STATUS_FAIL;

    /* ah_scaled = (absolute_humidity / 1000) * 256 */
    ah_scaled = (uint16_t)((absolute_humidity * 16777) >> 16);

    ret = sensirion_i2c_write_cmd_with_args(
        SGPC3_I2C_ADDRESS, SGPC3_CMD_SET_ABSOLUTE_HUMIDITY, &ah_scaled,
        SENSIRION_NUM_WORDS(ah_scaled));

    sensirion_sleep_usec(SGPC3_CMD_SET_ABSOLUTE_HUMIDITY_DURATION_US);

    return ret;
}

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
int16_t sgpc3_set_power_mode(uint16_t power_mode) {
    int16_t ret;

    ret = sgpc3_check_featureset(6);

    if (ret != STATUS_OK)
        return ret;

    ret = sensirion_i2c_write_cmd_with_args(
        SGPC3_I2C_ADDRESS, SGPC3_CMD_SET_POWER_MODE, &power_mode,
        SENSIRION_NUM_WORDS(power_mode));

    sensirion_sleep_usec(SGPC3_CMD_SET_POWER_MODE_DURATION_US);

    return ret;
}

/**
 * sgpc3_get_driver_version() - Return the driver version
 * Return:  Driver version string
 */
const char *sgpc3_get_driver_version() {
    return SGP_DRV_VERSION_STR;
}

/**
 * sgpc3_get_configured_address() - returns the configured I2C address
 *
 * Return:      uint8_t I2C address
 */
uint8_t sgpc3_get_configured_address() {
    return SGPC3_I2C_ADDRESS;
}

/**
 * sgpc3_get_feature_set_version() - Retrieve the sensor's feature set version
 * and product type
 *
 * @feature_set_version:    The feature set version
 * @product_type:           The product type: 0 for sgp30, 1: sgpc3
 *
 * Return:  STATUS_OK on success
 */
int16_t sgpc3_get_feature_set_version(uint16_t *feature_set_version,
                                      uint8_t *product_type) {
    int16_t ret;
    uint16_t words[SGPC3_CMD_GET_FEATURESET_WORDS];

    ret = sensirion_i2c_delayed_read_cmd(SGPC3_I2C_ADDRESS,
                                         SGPC3_CMD_GET_FEATURESET,
                                         SGPC3_CMD_GET_FEATURESET_DURATION_US,
                                         words, SGPC3_CMD_GET_FEATURESET_WORDS);

    if (ret != STATUS_OK)
        return ret;

    *feature_set_version = words[0] & 0x00FF;
    *product_type = (uint8_t)((words[0] & 0xF000) >> 12);

    return STATUS_OK;
}

/**
 * sgpc3_get_serial_id() - Retrieve the sensor's serial id
 *
 * @serial_id:    Output variable for the serial id
 *
 * Return:  STATUS_OK on success
 */
int16_t sgpc3_get_serial_id(uint64_t *serial_id) {
    int16_t ret;
    uint16_t words[SGPC3_CMD_GET_SERIAL_ID_WORDS];

    ret = sensirion_i2c_delayed_read_cmd(SGPC3_I2C_ADDRESS,
                                         SGPC3_CMD_GET_SERIAL_ID,
                                         SGPC3_CMD_GET_SERIAL_ID_DURATION_US,
                                         words, SGPC3_CMD_GET_SERIAL_ID_WORDS);

    if (ret != STATUS_OK)
        return ret;

    *serial_id = (((uint64_t)words[0]) << 32) | (((uint64_t)words[1]) << 16) |
                 (((uint64_t)words[2]) << 0);

    return STATUS_OK;
}

/**
 * sgpc3_tvoc_init_preheat() - reset the SGP's internal TVOC baselines and
 *                             run accelerated startup until the first
 *                             sgpc3_measure_tvoc()
 *
 * Return:  STATUS_OK on success.
 */
int16_t sgpc3_tvoc_init_preheat() {
    int16_t ret;

    ret = sgpc3_check_featureset(6);

    if (ret != STATUS_OK)
        return ret;

    ret = sensirion_i2c_write_cmd(SGPC3_I2C_ADDRESS, SGPC3_CMD_IAQ_INIT_CON);
    sensirion_sleep_usec(SGPC3_CMD_IAQ_INIT_CON_DURATION_US);
    return ret;
}

/**
 * sgpc3_tvoc_init_no_preheat() - reset the SGP's internal TVOC baselines
 * without accelerated startup time
 *
 * Return:  STATUS_OK on success.
 */
int16_t sgpc3_tvoc_init_no_preheat() {
    int16_t ret;
    ret = sensirion_i2c_write_cmd(SGPC3_I2C_ADDRESS, SGPC3_CMD_IAQ_INIT_0);
    sensirion_sleep_usec(SGPC3_CMD_IAQ_INIT_0_DURATION_US);
    return ret;
}

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
int16_t sgpc3_tvoc_init_64s_fs5() {
    int16_t ret;
    ret = sensirion_i2c_write_cmd(SGPC3_I2C_ADDRESS, SGPC3_CMD_IAQ_INIT_64);
    sensirion_sleep_usec(SGPC3_CMD_IAQ_INIT_64_DURATION_US);
    return ret;
}

/**
 * sgpc3_probe() - check if SGP sensor is available and initialize it
 *
 * This call aleady initializes the TVOC baselines
 * (sgpc3_tvoc_init_no_preheat())
 *
 * Return:  STATUS_OK on success,
 *          SGPC3_ERR_INSUFFICIENT_FEATURE_SET if the sensor's feature set
 *                                             is unknown or outdated,
 *          An error code otherwise
 */
int16_t sgpc3_probe() {
    int16_t ret;

    ret = sgpc3_check_featureset(4);
    if (ret != STATUS_OK)
        return ret;

    return sgpc3_tvoc_init_no_preheat();
}

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

#include "sgp30.h"
#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"
#include "sgp_git_version.h"

#define SGP30_PRODUCT_TYPE 0
static const uint8_t SGP30_I2C_ADDRESS = 0x58;

/* command and constants for reading the serial ID */
#define SGP30_CMD_GET_SERIAL_ID 0x3682
#define SGP30_CMD_GET_SERIAL_ID_DURATION_US 500
#define SGP30_CMD_GET_SERIAL_ID_WORDS 3

/* command and constants for reading the featureset version */
#define SGP30_CMD_GET_FEATURESET 0x202f
#define SGP30_CMD_GET_FEATURESET_DURATION_US 10000
#define SGP30_CMD_GET_FEATURESET_WORDS 1

/* command and constants for on-chip self-test */
#define SGP30_CMD_MEASURE_TEST 0x2032
#define SGP30_CMD_MEASURE_TEST_DURATION_US 220000
#define SGP30_CMD_MEASURE_TEST_WORDS 1
#define SGP30_CMD_MEASURE_TEST_OK 0xd400

/* command and constants for IAQ init */
#define SGP30_CMD_IAQ_INIT 0x2003
#define SGP30_CMD_IAQ_INIT_DURATION_US 10000

/* command and constants for IAQ measure */
#define SGP30_CMD_IAQ_MEASURE 0x2008
#define SGP30_CMD_IAQ_MEASURE_DURATION_US 12000
#define SGP30_CMD_IAQ_MEASURE_WORDS 2

/* command and constants for getting IAQ baseline */
#define SGP30_CMD_GET_IAQ_BASELINE 0x2015
#define SGP30_CMD_GET_IAQ_BASELINE_DURATION_US 10000
#define SGP30_CMD_GET_IAQ_BASELINE_WORDS 2

/* command and constants for setting IAQ baseline */
#define SGP30_CMD_SET_IAQ_BASELINE 0x201e
#define SGP30_CMD_SET_IAQ_BASELINE_DURATION_US 10000

/* command and constants for raw measure */
#define SGP30_CMD_RAW_MEASURE 0x2050
#define SGP30_CMD_RAW_MEASURE_DURATION_US 25000
#define SGP30_CMD_RAW_MEASURE_WORDS 2

/* command and constants for setting absolute humidity */
#define SGP30_CMD_SET_ABSOLUTE_HUMIDITY 0x2061
#define SGP30_CMD_SET_ABSOLUTE_HUMIDITY_DURATION_US 10000

/* command and constants for getting TVOC inceptive baseline */
#define SGP30_CMD_GET_TVOC_INCEPTIVE_BASELINE 0x20b3
#define SGP30_CMD_GET_TVOC_INCEPTIVE_BASELINE_DURATION_US 10000
#define SGP30_CMD_GET_TVOC_INCEPTIVE_BASELINE_WORDS 1

/* command and constants for setting TVOC baseline */
#define SGP30_CMD_SET_TVOC_BASELINE 0x2077
#define SGP30_CMD_SET_TVOC_BASELINE_DURATION_US 10000

/**
 * sgp30_check_featureset() - Check if the connected sensor has a certain FS
 *
 * @needed_fs: The featureset that is required
 *
 * Return: STATUS_OK if the sensor has the required FS,
 *         SGP30_ERR_INVALID_PRODUCT_TYPE if the sensor is not an SGP30,
 *         SGP30_ERR_UNSUPPORTED_FEATURE_SET if the sensor does not
 *                                           have the required FS,
 *         an error code otherwise
 */
static int16_t sgp30_check_featureset(uint16_t needed_fs) {
    int16_t ret;
    uint16_t fs_version;
    uint8_t product_type;

    ret = sgp30_get_feature_set_version(&fs_version, &product_type);
    if (ret != STATUS_OK)
        return ret;

    if (product_type != SGP30_PRODUCT_TYPE)
        return SGP30_ERR_INVALID_PRODUCT_TYPE;

    if (fs_version < needed_fs)
        return SGP30_ERR_UNSUPPORTED_FEATURE_SET;

    return STATUS_OK;
}

int16_t sgp30_measure_test(uint16_t* test_result) {
    uint16_t measure_test_word_buf[SGP30_CMD_MEASURE_TEST_WORDS];
    int16_t ret;

    *test_result = 0;

    ret = sensirion_i2c_delayed_read_cmd(
        SGP30_I2C_ADDRESS, SGP30_CMD_MEASURE_TEST,
        SGP30_CMD_MEASURE_TEST_DURATION_US, measure_test_word_buf,
        SENSIRION_NUM_WORDS(measure_test_word_buf));
    if (ret != STATUS_OK)
        return ret;

    *test_result = *measure_test_word_buf;
    if (*test_result == SGP30_CMD_MEASURE_TEST_OK)
        return STATUS_OK;

    return STATUS_FAIL;
}

int16_t sgp30_measure_iaq() {
    return sensirion_i2c_write_cmd(SGP30_I2C_ADDRESS, SGP30_CMD_IAQ_MEASURE);
}

int16_t sgp30_read_iaq(uint16_t* tvoc_ppb, uint16_t* co2_eq_ppm) {
    int16_t ret;
    uint16_t words[SGP30_CMD_IAQ_MEASURE_WORDS];

    ret = sensirion_i2c_read_words(SGP30_I2C_ADDRESS, words,
                                   SGP30_CMD_IAQ_MEASURE_WORDS);

    *tvoc_ppb = words[1];
    *co2_eq_ppm = words[0];

    return ret;
}

int16_t sgp30_measure_iaq_blocking_read(uint16_t* tvoc_ppb,
                                        uint16_t* co2_eq_ppm) {
    int16_t ret;

    ret = sgp30_measure_iaq();
    if (ret != STATUS_OK)
        return ret;

    sensirion_sleep_usec(SGP30_CMD_IAQ_MEASURE_DURATION_US);

    return sgp30_read_iaq(tvoc_ppb, co2_eq_ppm);
}

int16_t sgp30_measure_tvoc() {
    return sgp30_measure_iaq();
}

int16_t sgp30_read_tvoc(uint16_t* tvoc_ppb) {
    uint16_t co2_eq_ppm;
    return sgp30_read_iaq(tvoc_ppb, &co2_eq_ppm);
}

int16_t sgp30_measure_tvoc_blocking_read(uint16_t* tvoc_ppb) {
    uint16_t co2_eq_ppm;
    return sgp30_measure_iaq_blocking_read(tvoc_ppb, &co2_eq_ppm);
}

int16_t sgp30_measure_co2_eq() {
    return sgp30_measure_iaq();
}

int16_t sgp30_read_co2_eq(uint16_t* co2_eq_ppm) {
    uint16_t tvoc_ppb;
    return sgp30_read_iaq(&tvoc_ppb, co2_eq_ppm);
}

int16_t sgp30_measure_co2_eq_blocking_read(uint16_t* co2_eq_ppm) {
    uint16_t tvoc_ppb;
    return sgp30_measure_iaq_blocking_read(&tvoc_ppb, co2_eq_ppm);
}

int16_t sgp30_measure_raw_blocking_read(uint16_t* ethanol_raw_signal,
                                        uint16_t* h2_raw_signal) {
    int16_t ret;

    ret = sgp30_measure_raw();
    if (ret != STATUS_OK)
        return ret;

    sensirion_sleep_usec(SGP30_CMD_RAW_MEASURE_DURATION_US);

    return sgp30_read_raw(ethanol_raw_signal, h2_raw_signal);
}

int16_t sgp30_measure_raw() {
    return sensirion_i2c_write_cmd(SGP30_I2C_ADDRESS, SGP30_CMD_RAW_MEASURE);
}

int16_t sgp30_read_raw(uint16_t* ethanol_raw_signal, uint16_t* h2_raw_signal) {
    int16_t ret;
    uint16_t words[SGP30_CMD_RAW_MEASURE_WORDS];

    ret = sensirion_i2c_read_words(SGP30_I2C_ADDRESS, words,
                                   SGP30_CMD_RAW_MEASURE_WORDS);

    *ethanol_raw_signal = words[1];
    *h2_raw_signal = words[0];

    return ret;
}

int16_t sgp30_get_iaq_baseline(uint32_t* baseline) {
    int16_t ret;
    uint16_t words[SGP30_CMD_GET_IAQ_BASELINE_WORDS];

    ret =
        sensirion_i2c_write_cmd(SGP30_I2C_ADDRESS, SGP30_CMD_GET_IAQ_BASELINE);

    if (ret != STATUS_OK)
        return ret;

    sensirion_sleep_usec(SGP30_CMD_GET_IAQ_BASELINE_DURATION_US);

    ret = sensirion_i2c_read_words(SGP30_I2C_ADDRESS, words,
                                   SGP30_CMD_GET_IAQ_BASELINE_WORDS);

    if (ret != STATUS_OK)
        return ret;

    *baseline = ((uint32_t)words[1] << 16) | ((uint32_t)words[0]);

    if (*baseline)
        return STATUS_OK;
    return STATUS_FAIL;
}

int16_t sgp30_set_iaq_baseline(uint32_t baseline) {
    int16_t ret;
    uint16_t words[2] = {(uint16_t)((baseline & 0xffff0000) >> 16),
                         (uint16_t)(baseline & 0x0000ffff)};

    if (!baseline)
        return STATUS_FAIL;

    ret = sensirion_i2c_write_cmd_with_args(SGP30_I2C_ADDRESS,
                                            SGP30_CMD_SET_IAQ_BASELINE, words,
                                            SENSIRION_NUM_WORDS(words));

    sensirion_sleep_usec(SGP30_CMD_SET_IAQ_BASELINE_DURATION_US);

    return ret;
}

int16_t sgp30_get_tvoc_inceptive_baseline(uint16_t* tvoc_inceptive_baseline) {
    int16_t ret;

    ret = sgp30_check_featureset(0x21);

    if (ret != STATUS_OK)
        return ret;

    ret = sensirion_i2c_write_cmd(SGP30_I2C_ADDRESS,
                                  SGP30_CMD_GET_TVOC_INCEPTIVE_BASELINE);

    if (ret != STATUS_OK)
        return ret;

    sensirion_sleep_usec(SGP30_CMD_GET_TVOC_INCEPTIVE_BASELINE_DURATION_US);

    return sensirion_i2c_read_words(
        SGP30_I2C_ADDRESS, tvoc_inceptive_baseline,
        SGP30_CMD_GET_TVOC_INCEPTIVE_BASELINE_WORDS);
}

int16_t sgp30_set_tvoc_baseline(uint16_t tvoc_baseline) {
    int16_t ret;

    ret = sgp30_check_featureset(0x21);

    if (ret != STATUS_OK)
        return ret;

    if (!tvoc_baseline)
        return STATUS_FAIL;

    ret = sensirion_i2c_write_cmd_with_args(
        SGP30_I2C_ADDRESS, SGP30_CMD_SET_TVOC_BASELINE, &tvoc_baseline,
        SENSIRION_NUM_WORDS(tvoc_baseline));

    sensirion_sleep_usec(SGP30_CMD_SET_TVOC_BASELINE_DURATION_US);

    return ret;
}

int16_t sgp30_set_absolute_humidity(uint32_t absolute_humidity) {
    int16_t ret;
    uint16_t ah_scaled;

    if (absolute_humidity > 256000)
        return STATUS_FAIL;

    /* ah_scaled = (absolute_humidity / 1000) * 256 */
    ah_scaled = (uint16_t)((absolute_humidity * 16777) >> 16);

    ret = sensirion_i2c_write_cmd_with_args(
        SGP30_I2C_ADDRESS, SGP30_CMD_SET_ABSOLUTE_HUMIDITY, &ah_scaled,
        SENSIRION_NUM_WORDS(ah_scaled));

    sensirion_sleep_usec(SGP30_CMD_SET_ABSOLUTE_HUMIDITY_DURATION_US);

    return ret;
}

const char* sgp30_get_driver_version() {
    return SGP_DRV_VERSION_STR;
}

uint8_t sgp30_get_configured_address() {
    return SGP30_I2C_ADDRESS;
}

int16_t sgp30_get_feature_set_version(uint16_t* feature_set_version,
                                      uint8_t* product_type) {
    int16_t ret;
    uint16_t words[SGP30_CMD_GET_FEATURESET_WORDS];

    ret = sensirion_i2c_delayed_read_cmd(SGP30_I2C_ADDRESS,
                                         SGP30_CMD_GET_FEATURESET,
                                         SGP30_CMD_GET_FEATURESET_DURATION_US,
                                         words, SGP30_CMD_GET_FEATURESET_WORDS);

    if (ret != STATUS_OK)
        return ret;

    *feature_set_version = words[0] & 0x00FF;
    *product_type = (uint8_t)((words[0] & 0xF000) >> 12);

    return STATUS_OK;
}

int16_t sgp30_get_serial_id(uint64_t* serial_id) {
    int16_t ret;
    uint16_t words[SGP30_CMD_GET_SERIAL_ID_WORDS];

    ret = sensirion_i2c_delayed_read_cmd(SGP30_I2C_ADDRESS,
                                         SGP30_CMD_GET_SERIAL_ID,
                                         SGP30_CMD_GET_SERIAL_ID_DURATION_US,
                                         words, SGP30_CMD_GET_SERIAL_ID_WORDS);

    if (ret != STATUS_OK)
        return ret;

    *serial_id = (((uint64_t)words[0]) << 32) | (((uint64_t)words[1]) << 16) |
                 (((uint64_t)words[2]) << 0);

    return STATUS_OK;
}

int16_t sgp30_iaq_init() {
    int16_t ret =
        sensirion_i2c_write_cmd(SGP30_I2C_ADDRESS, SGP30_CMD_IAQ_INIT);
    sensirion_sleep_usec(SGP30_CMD_IAQ_INIT_DURATION_US);
    return ret;
}

int16_t sgp30_probe() {
    int16_t ret = sgp30_check_featureset(0x20);

    if (ret != STATUS_OK)
        return ret;

    return sgp30_iaq_init();
}

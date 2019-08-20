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
#include "sgp_featureset.h"
#include "sgp_git_version.h"

#define SGPC3_RAM_WORDS 4
#define SGPC3_BUFFER_SIZE ((SGPC3_RAM_WORDS + 2) * (SGP_WORD_LEN + CRC8_LEN))
#define SGPC3_BUFFER_WORDS (SGPC3_BUFFER_SIZE / SGP_WORD_LEN)
#define SGPC3_MAX_PROFILE_RET_LEN 4 * (SGP_WORD_LEN + CRC8_LEN)
#define SGPC3_VALID_TVOC_BASELINE(b) ((b) != 0)

static const uint8_t SGPC3_I2C_ADDRESS = 0x58;

/* command and constants for reading the serial ID */
#define SGPC3_CMD_GET_SERIAL_ID_DURATION_US 500
#define SGPC3_CMD_GET_SERIAL_ID_WORDS 3
#define SGPC3_CMD_GET_SERIAL_ID 0x3682

/* command and constants for reading the featureset version */
#define SGPC3_CMD_GET_FEATURESET_DURATION_US 1000
#define SGPC3_CMD_GET_FEATURESET_WORDS 1
#define SGPC3_CMD_GET_FEATURESET 0x202f

/* command and constants for on-chip self-test */
#define SGPC3_CMD_MEASURE_TEST_DURATION_US 220000
#define SGPC3_CMD_MEASURE_TEST_WORDS 1
#define SGPC3_CMD_MEASURE_TEST_OK 0xd400
#define SGPC3_CMD_MEASURE_TEST 0x2032

static const struct sgp_otp_featureset sgpc3_features_unknown = {
    .profiles = NULL,
    .number_of_profiles = 0,
};

enum sgpc3_state_code { WAIT_STATE, MEASURING_PROFILE_STATE };

struct sgpc3_info {
    uint64_t serial_id;
    uint16_t feature_set_version;
};

static struct sgpc3_data {
    enum sgpc3_state_code current_state;
    struct sgpc3_info info;
    const struct sgp_otp_featureset *otp_features;
    union {
        uint16_t words[SGPC3_BUFFER_WORDS];
        uint64_t u64_value;
    } buffer;
} client_data;

/**
 * unpack_signals() - unpack signals which are stored in
 *                    client_data.buffer.words
 * @profile:    The profile
 */
static void unpack_signals(const struct sgp_profile *profile) {
    int16_t i, j;
    uint16_t data_words = profile->num_words;
    uint16_t word_buf[data_words];

    /* copy buffer */
    for (i = 0; i < data_words; i++)
        word_buf[i] = client_data.buffer.words[i];

    /* signals are in reverse order in the data buffer */
    for (i = profile->num_words - 1, j = 0; i >= 0; i -= 1, j += 1)
        client_data.buffer.words[j] = word_buf[i];
}

/**
 * read_measurement() - reads the result of a profile measurement
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
static int16_t read_measurement(const struct sgp_profile *profile) {
    int16_t ret;

    switch (client_data.current_state) {

        case MEASURING_PROFILE_STATE:
            ret = sensirion_i2c_read_words(SGPC3_I2C_ADDRESS,
                                           client_data.buffer.words,
                                           profile->num_words);

            if (ret)
                /* Measurement in progress */
                return STATUS_FAIL;

            unpack_signals(profile);
            client_data.current_state = WAIT_STATE;

            return STATUS_OK;

        default:
            /* No command issued */
            return STATUS_FAIL;
    }
}

/**
 * sgpc3_get_profile_by_number() - get a profile by its identifier number
 * @number      The number that identifies the profile
 *
 * Return:      A pointer to the profile or NULL if the profile does not exists
 */
static const struct sgp_profile *sgpc3_get_profile_by_number(uint16_t number) {
    uint8_t i;
    const struct sgp_profile *profile = NULL;

    for (i = 0; i < client_data.otp_features->number_of_profiles; i++) {
        profile = client_data.otp_features->profiles[i];
        if (number == profile->number)
            break;
    }

    if (i == client_data.otp_features->number_of_profiles) {
        return NULL;
    }

    return profile;
}

/**
 * sgpc3_run_profile_by_number() - run a profile by its identifier number
 * @number:     The number that identifies the profile
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
static int16_t sgpc3_run_profile_by_number(uint16_t number) {
    const struct sgp_profile *profile;
    int16_t ret;

    profile = sgpc3_get_profile_by_number(number);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = sensirion_i2c_write_cmd(SGPC3_I2C_ADDRESS, profile->command);
    if (ret != STATUS_OK)
        return ret;

    sensirion_sleep_usec(profile->duration_us);

    if (profile->num_words > 0) {
        client_data.current_state = MEASURING_PROFILE_STATE;
        return read_measurement(profile);
    }

    return STATUS_OK;
}

/**
 * sgpc3_detect_featureset_version() - extracts the featureset and initializes
 *                                   client_data.
 *
 * @featureset:  Pointer to the featureset bits
 *
 * Return:    STATUS_OK on success, STATUS_FAIL otherwise
 */
static int16_t sgpc3_detect_featureset_version(uint16_t *featureset) {
    int16_t i, j;
    uint16_t feature_set_version = *featureset;
    const struct sgp_otp_featureset *sgpc3_featureset;

    client_data.info.feature_set_version = feature_set_version;
    client_data.otp_features = &sgpc3_features_unknown;
    for (i = 0; i < sgp_supported_featuresets.number_of_supported_featuresets;
         ++i) {
        sgpc3_featureset = sgp_supported_featuresets.featuresets[i];
        for (j = 0;
             j < sgpc3_featureset->number_of_supported_featureset_versions;
             ++j) {
            if (SGP_FS_COMPAT(
                    feature_set_version,
                    sgpc3_featureset->supported_featureset_versions[j])) {
                client_data.otp_features = sgpc3_featureset;
                return STATUS_OK;
            }
        }
    }
    return STATUS_FAIL;
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
    const struct sgp_profile *profile;
    int16_t ret;

    profile = sgpc3_get_profile_by_number(PROFILE_NUMBER_IAQ_MEASURE);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = sensirion_i2c_write_cmd(SGPC3_I2C_ADDRESS, profile->command);
    if (ret != STATUS_OK)
        return ret;

    client_data.current_state = MEASURING_PROFILE_STATE;

    return STATUS_OK;
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
    const struct sgp_profile *profile;
    int16_t ret;

    profile = sgpc3_get_profile_by_number(PROFILE_NUMBER_IAQ_MEASURE);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = read_measurement(profile);
    if (ret != STATUS_OK)
        return ret;

    *tvoc_ppb = client_data.buffer.words[0];

    return STATUS_OK;
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

    ret = sgpc3_run_profile_by_number(PROFILE_NUMBER_IAQ_MEASURE);
    if (ret != STATUS_OK)
        return ret;

    *tvoc_ppb = client_data.buffer.words[0];

    return STATUS_OK;
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
    int16_t ret =
        sgpc3_run_profile_by_number(PROFILE_NUMBER_MEASURE_RAW_SIGNALS);
    if (ret != STATUS_OK)
        return ret;

    *ethanol_raw_signal = client_data.buffer.words[0];

    return STATUS_OK;
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
    const struct sgp_profile *profile;
    int16_t ret;

    profile = sgpc3_get_profile_by_number(PROFILE_NUMBER_MEASURE_RAW_SIGNALS);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = sensirion_i2c_write_cmd(SGPC3_I2C_ADDRESS, profile->command);
    if (ret != STATUS_OK)
        return ret;

    client_data.current_state = MEASURING_PROFILE_STATE;

    return STATUS_OK;
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
    const struct sgp_profile *profile;
    int16_t ret;

    profile = sgpc3_get_profile_by_number(PROFILE_NUMBER_MEASURE_RAW_SIGNALS);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = read_measurement(profile);
    if (ret != STATUS_OK)
        return ret;

    *ethanol_raw_signal = client_data.buffer.words[0];

    return STATUS_OK;
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

    ret = sgpc3_run_profile_by_number(PROFILE_NUMBER_MEASURE_RAW);
    if (ret != STATUS_OK)
        return ret;

    *tvoc_ppb = client_data.buffer.words[0];
    *ethanol_raw_signal = client_data.buffer.words[1];

    return STATUS_OK;
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
    const struct sgp_profile *profile;
    int16_t ret;

    profile = sgpc3_get_profile_by_number(PROFILE_NUMBER_MEASURE_RAW);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = sensirion_i2c_write_cmd(SGPC3_I2C_ADDRESS, profile->command);
    if (ret != STATUS_OK)
        return ret;

    client_data.current_state = MEASURING_PROFILE_STATE;

    return STATUS_OK;
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
    const struct sgp_profile *profile;
    int16_t ret;

    profile = sgpc3_get_profile_by_number(PROFILE_NUMBER_MEASURE_RAW);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = read_measurement(profile);
    if (ret != STATUS_OK)
        return ret;

    *tvoc_ppb = client_data.buffer.words[0];
    *ethanol_raw_signal = client_data.buffer.words[1];

    return STATUS_OK;
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
    int16_t ret = sgpc3_run_profile_by_number(PROFILE_NUMBER_IAQ_GET_BASELINE);
    if (ret != STATUS_OK)
        return ret;

    *baseline = client_data.buffer.words[0];

    if (!SGPC3_VALID_TVOC_BASELINE(*baseline))
        return STATUS_FAIL;

    return STATUS_OK;
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
    const struct sgp_profile *profile;

    if (!SGPC3_VALID_TVOC_BASELINE(baseline))
        return STATUS_FAIL;

    profile = sgpc3_get_profile_by_number(PROFILE_NUMBER_IAQ_SET_BASELINE);
    if (profile == NULL)
        return STATUS_FAIL;

    return sensirion_i2c_write_cmd_with_args(SGPC3_I2C_ADDRESS, profile->command,
                                             &baseline,
                                             SENSIRION_NUM_WORDS(baseline));
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

    ret = sgpc3_run_profile_by_number(
        PROFILE_NUMBER_IAQ_GET_TVOC_INCEPTIVE_BASELINE);
    if (ret != STATUS_OK)
        return ret;

    *tvoc_inceptive_baseline = client_data.buffer.words[0];

    return STATUS_OK;
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
    const struct sgp_profile *profile;
    uint16_t ah_scaled;

    if (!SGP_REQUIRE_FS(client_data.info.feature_set_version, 0, 6))
        return STATUS_FAIL; /* feature unavailable */

    profile = sgpc3_get_profile_by_number(PROFILE_NUMBER_SET_ABSOLUTE_HUMIDITY);
    if (profile == NULL)
        return STATUS_FAIL;

    if (absolute_humidity > 256000)
        return STATUS_FAIL;

    /* ah_scaled = (absolute_humidity / 1000) * 256 */
    ah_scaled = (uint16_t)((absolute_humidity * 16777) >> 16);

    return sensirion_i2c_write_cmd_with_args(SGPC3_I2C_ADDRESS, profile->command,
                                             &ah_scaled,
                                             SENSIRION_NUM_WORDS(ah_scaled));
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
    const struct sgp_profile *profile;

    if (!SGP_REQUIRE_FS(client_data.info.feature_set_version, 0, 6))
        return STATUS_FAIL; /* feature unavailable */

    profile = sgpc3_get_profile_by_number(PROFILE_NUMBER_SET_POWER_MODE);
    if (profile == NULL)
        return STATUS_FAIL;

    return sensirion_i2c_write_cmd_with_args(SGPC3_I2C_ADDRESS, profile->command,
                                             &power_mode,
                                             SENSIRION_NUM_WORDS(power_mode));
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
    *feature_set_version = client_data.info.feature_set_version & 0x00FF;
    *product_type =
        (uint8_t)((client_data.info.feature_set_version & 0xF000) >> 12);
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
    *serial_id = client_data.info.serial_id;
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
    return sgpc3_run_profile_by_number(PROFILE_NUMBER_IAQ_INIT_CONTINUOUS);
}

/**
 * sgpc3_tvoc_init_no_preheat() - reset the SGP's internal TVOC baselines
 * without accelerated startup time
 *
 * Return:  STATUS_OK on success.
 */
int16_t sgpc3_tvoc_init_no_preheat() {
    return sgpc3_run_profile_by_number(PROFILE_NUMBER_IAQ_INIT0);
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
    return sgpc3_run_profile_by_number(PROFILE_NUMBER_IAQ_INIT64);
}

/**
 * sgpc3_probe() - check if SGP sensor is available and initialize it
 *
 * This call aleady initializes the TVOC baselines
 * (sgpc3_tvoc_init_no_preheat())
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
int16_t sgpc3_probe() {
    int16_t err;
    uint64_t *serial_buf = &client_data.buffer.u64_value;

    *serial_buf = 0;
    client_data.current_state = WAIT_STATE;

    /* try to read the serial ID */
    err = sensirion_i2c_delayed_read_cmd(
        SGPC3_I2C_ADDRESS, SGPC3_CMD_GET_SERIAL_ID,
        SGPC3_CMD_GET_SERIAL_ID_DURATION_US, client_data.buffer.words,
        SGPC3_CMD_GET_SERIAL_ID_WORDS);
    if (err != STATUS_OK)
        return err;

    SENSIRION_WORDS_TO_BYTES(client_data.buffer.words,
                             SGPC3_CMD_GET_SERIAL_ID_WORDS);
    client_data.info.serial_id = be64_to_cpu(*serial_buf) >> 16;

    /* read the featureset version */
    err = sensirion_i2c_delayed_read_cmd(
        SGPC3_I2C_ADDRESS, SGPC3_CMD_GET_FEATURESET,
        SGPC3_CMD_GET_FEATURESET_DURATION_US, client_data.buffer.words,
        SGPC3_CMD_GET_FEATURESET_WORDS);
    if (err != STATUS_OK)
        return err;

    err = sgpc3_detect_featureset_version(client_data.buffer.words);
    if (err != STATUS_OK)
        return err;

    return sgpc3_tvoc_init_no_preheat();
}

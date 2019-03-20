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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sensirion_arch_config.h"
#include "sensirion_i2c.h"
#include "sensirion_common.h"
#include "sgp_featureset.h"
#include "sgp_git_version.h"
#include "sgp30.h"


#define SGP_RAM_WORDS                   4
#define SGP_BUFFER_SIZE                 ((SGP_RAM_WORDS + 2) * \
                                         (SGP_WORD_LEN + CRC8_LEN))
#define SGP_BUFFER_WORDS                (SGP_BUFFER_SIZE / SGP_WORD_LEN)
#define SGP_MAX_PROFILE_RET_LEN         4 * (SGP_WORD_LEN + CRC8_LEN)
#define SGP_VALID_IAQ_BASELINE(b)       ((b) != 0)

#ifdef SGP_ADDRESS
static const u8 SGP_I2C_ADDRESS = SGP_ADDRESS;
#else
static const u8 SGP_I2C_ADDRESS = 0x58;
#endif

/* command and constants for reading the serial ID */
#define SGP_CMD_GET_SERIAL_ID_DURATION_US   500
#define SGP_CMD_GET_SERIAL_ID_WORDS         3
static const u16 sgp30_cmd_get_serial_id = 0x3682;

/* command and constants for reading the featureset version */
#define SGP_CMD_GET_FEATURESET_DURATION_US  1000
#define SGP_CMD_GET_FEATURESET_WORDS        1
static const u16 sgp30_cmd_get_featureset = 0x202f;

/* command and constants for on-chip self-test */
#define SGP_CMD_MEASURE_TEST_DURATION_US    220000
#define SGP_CMD_MEASURE_TEST_WORDS          1
#define SGP_CMD_MEASURE_TEST_OK             0xd400
static const u16 sgp30_cmd_measure_test = 0x2032;

static const struct sgp_otp_featureset sgp30_features_unknown = {
    .profiles = NULL,
    .number_of_profiles = 0,
};

enum sgp30_state_code {
    WAIT_STATE,
    MEASURING_PROFILE_STATE
};

struct sgp30_info {
    u64 serial_id;
    u16 feature_set_version;
};

static struct sgp30_data {
    enum sgp30_state_code current_state;
    struct sgp30_info info;
    const struct sgp_otp_featureset *otp_features;
    union {
        u16 words[SGP_BUFFER_WORDS];
        u64 u64_value;
    } buffer;
} client_data;


/**
 * unpack_signals() - unpack signals which are stored in
 *                    client_data.buffer.words
 * @profile:    The profile
 */
static void unpack_signals(const struct sgp_profile *profile) {
    s16 i, j;
    const struct sgp_signal *signal;
    u16 data_words = profile->number_of_signals;
    u16 word_buf[data_words];
    u16 value;

    /* copy buffer */
    for (i = 0; i < data_words; i++)
        word_buf[i] = client_data.buffer.words[i];

    /* signals are in reverse order in the data buffer */
    for (i = profile->number_of_signals - 1, j = 0; i >= 0; i -= 1, j += 1) {
        signal = profile->signals[profile->number_of_signals - i - 1];
        value = word_buf[i];

        if (signal->conversion_function != NULL)
            client_data.buffer.words[j] = signal->conversion_function(value);
        else
            client_data.buffer.words[j] = value;
    }
}

/**
 * read_measurement() - reads the result of a profile measurement
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
static s16 read_measurement(const struct sgp_profile *profile) {
    s16 ret;

    switch (client_data.current_state) {

        case MEASURING_PROFILE_STATE:
            ret = sensirion_i2c_read_words(SGP_I2C_ADDRESS,
                                           client_data.buffer.words,
                                           profile->number_of_signals);

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
 * sgp30_run_profile() - run a profile and read write its return to client_data
 * @profile     A pointer to the profile
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
static s16 sgp30_run_profile(const struct sgp_profile *profile) {
    u32 duration_us = profile->duration_us + 5;
    s16 ret;

    ret = sensirion_i2c_write_cmd(SGP_I2C_ADDRESS, profile->command);
    if (ret != STATUS_OK)
        return ret;

    sensirion_sleep_usec(duration_us);

    if (profile->number_of_signals > 0) {
        client_data.current_state = MEASURING_PROFILE_STATE;
        return read_measurement(profile);
    }

    return STATUS_OK;
}


/**
 * sgp30_get_profile_by_number() - get a profile by its identifier number
 * @number      The number that identifies the profile
 *
 * Return:      A pointer to the profile or NULL if the profile does not exists
 */
static const struct sgp_profile *sgp30_get_profile_by_number(u16 number) {
    u8 i;
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
 * sgp30_run_profile_by_number() - run a profile by its identifier number
 * @number:     The number that identifies the profile
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
static s16 sgp30_run_profile_by_number(u16 number) {
    const struct sgp_profile *profile;

    profile = sgp30_get_profile_by_number(number);
    if (profile == NULL)
        return STATUS_FAIL;

    return sgp30_run_profile(profile);
}


/**
 * sgp30_detect_featureset_version() - extracts the featureset and initializes
 *                                   client_data.
 *
 * @featureset:  Pointer to the featureset bits
 *
 * Return:    STATUS_OK on success, STATUS_FAIL otherwise
 */
static s16 sgp30_detect_featureset_version(u16 *featureset) {
    s16 i, j;
    u16 feature_set_version = *featureset;
    const struct sgp_otp_featureset *sgp30_featureset;

    client_data.info.feature_set_version = feature_set_version;
    client_data.otp_features = &sgp30_features_unknown;
    for (i = 0; i < sgp_supported_featuresets.number_of_supported_featuresets; ++i) {
        sgp30_featureset = sgp_supported_featuresets.featuresets[i];
        for (j = 0; j < sgp30_featureset->number_of_supported_featureset_versions; ++j) {
            if (SGP_FS_COMPAT(feature_set_version,
                              sgp30_featureset->supported_featureset_versions[j])) {
                client_data.otp_features = sgp30_featureset;
                return STATUS_OK;
            }
        }
    }
    return STATUS_FAIL;
}


/**
 * sgp30_measure_test() - Run the on-chip self-test
 *
 * This method is executed synchronously and blocks for the duration of the
 * measurement (~220ms)
 *
 * @test_result:    Allocated buffer to store the chip's error code.
 *                  test_result is SGP_CMD_MEASURE_TEST_OK on success or set to
 *                  zero (0) in the case of a communication error.
 *
 * Return: STATUS_OK on a successful self-test, an error code otherwise
 */
s16 sgp30_measure_test(u16 *test_result) {
    u16 measure_test_word_buf[SGP_CMD_MEASURE_TEST_WORDS];
    s16 ret;

    *test_result = 0;

    ret = sensirion_i2c_delayed_read_cmd(SGP_I2C_ADDRESS, sgp30_cmd_measure_test,
                                         SGP_CMD_MEASURE_TEST_DURATION_US,
                                         measure_test_word_buf,
                                         SENSIRION_NUM_WORDS(measure_test_word_buf));
    if (ret != STATUS_OK)
        return ret;

    *test_result = *measure_test_word_buf;
    if (*test_result == SGP_CMD_MEASURE_TEST_OK)
        return STATUS_OK;

    return STATUS_FAIL;
}


/**
 * sgp30_measure_iaq() - Measure IAQ values async
 *
 * The profile is executed asynchronously. Use sgp30_read_iaq to get the values.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
s16 sgp30_measure_iaq() {
    const struct sgp_profile *profile;
    s16 ret;

    profile = sgp30_get_profile_by_number(PROFILE_NUMBER_IAQ_MEASURE);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = sensirion_i2c_write_cmd(SGP_I2C_ADDRESS, profile->command);
    if (ret != STATUS_OK)
        return ret;

    client_data.current_state = MEASURING_PROFILE_STATE;

    return STATUS_OK;
}


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
s16 sgp30_read_iaq(u16 *tvoc_ppb, u16 *co2_eq_ppm) {
    const struct sgp_profile *profile;
    s16 ret;

    profile = sgp30_get_profile_by_number(PROFILE_NUMBER_IAQ_MEASURE);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = read_measurement(profile);
    if (ret != STATUS_OK)
        return ret;

    *tvoc_ppb = client_data.buffer.words[0];
    *co2_eq_ppm = client_data.buffer.words[1];

    return STATUS_OK;
}


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
s16 sgp30_measure_iaq_blocking_read(u16 *tvoc_ppb, u16 *co2_eq_ppm) {
    s16 ret;

    ret = sgp30_run_profile_by_number(PROFILE_NUMBER_IAQ_MEASURE);
    if (ret != STATUS_OK)
        return ret;

    *tvoc_ppb = client_data.buffer.words[0];
    *co2_eq_ppm = client_data.buffer.words[1];

    return STATUS_OK;
}


/**
 * sgp30_measure_tvoc() - Measure tVOC concentration async
 *
 * The profile is executed asynchronously. Use sgp30_read_tvoc to get the
 * ppb value.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
s16 sgp30_measure_tvoc() {
    return sgp30_measure_iaq();
}


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
s16 sgp30_read_tvoc(u16 *tvoc_ppb) {
    u16 co2_eq_ppm;
    return sgp30_read_iaq(tvoc_ppb, &co2_eq_ppm);
}


/**
 * sgp30_measure_tvoc_blocking_read() - Measure tVOC concentration
 *
 * The profile is executed synchronously.
 *
 * Return:  tVOC concentration in ppb. Negative if it fails.
 */
s16 sgp30_measure_tvoc_blocking_read(u16 *tvoc_ppb) {
    u16 co2_eq_ppm;
    return sgp30_measure_iaq_blocking_read(tvoc_ppb, &co2_eq_ppm);
}


/**
 * sgp30_measure_co2_eq() - Measure tVOC concentration async
 *
 * The profile is executed asynchronously. Use sgp30_read_co2_eq to get the
 * ppm value.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
s16 sgp30_measure_co2_eq() {
    return sgp30_measure_iaq();
}


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
s16 sgp30_read_co2_eq(u16 *co2_eq_ppm) {
    u16 tvoc_ppb;
    return sgp30_read_iaq(&tvoc_ppb, co2_eq_ppm);
}


/**
 * sgp30_measure_co2_eq_blocking_read() - Measure CO2-Equivalent concentration
 *
 * The profile is executed synchronously.
 *
 * Return:  CO2-Equivalent concentration in ppm. Negative if it fails.
 */
s16 sgp30_measure_co2_eq_blocking_read(u16 *co2_eq_ppm) {
    u16 tvoc_ppb;
    return sgp30_measure_iaq_blocking_read(&tvoc_ppb, co2_eq_ppm);
}


/**
 * sgp30_measure_raw_blocking_read() - Measure raw signals
 * The profile is executed synchronously.
 *
 * @ethanol_raw_signal: Output variable for the ethanol raw signal
 * @h2_raw_signal:      Output variable for the h2 raw signal
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
s16 sgp30_measure_raw_blocking_read(u16 *ethanol_raw_signal, u16 *h2_raw_signal) {
    s16 ret;

    ret = sgp30_run_profile_by_number(PROFILE_NUMBER_MEASURE_RAW_SIGNALS);
    if (ret != STATUS_OK)
        return ret;

    *ethanol_raw_signal = client_data.buffer.words[0];
    *h2_raw_signal = client_data.buffer.words[1];

    return STATUS_OK;
}


/**
 * sgp30_measure_raw() - Measure raw signals async
 *
 * The profile is executed asynchronously. Use sgp30_read_raw to get the
 * signal values.
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
s16 sgp30_measure_raw() {
    const struct sgp_profile *profile;
    s16 ret;

    profile = sgp30_get_profile_by_number(PROFILE_NUMBER_MEASURE_RAW_SIGNALS);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = sensirion_i2c_write_cmd(SGP_I2C_ADDRESS, profile->command);
    if (ret != STATUS_OK)
        return ret;

    client_data.current_state = MEASURING_PROFILE_STATE;

    return STATUS_OK;
}


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
s16 sgp30_read_raw(u16 *ethanol_raw_signal, u16 *h2_raw_signal) {
    const struct sgp_profile *profile;
    s16 ret;

    profile = sgp30_get_profile_by_number(PROFILE_NUMBER_MEASURE_RAW_SIGNALS);
    if (profile == NULL)
        return STATUS_FAIL;

    ret = read_measurement(profile);
    if (ret != STATUS_OK)
        return ret;

    *ethanol_raw_signal = client_data.buffer.words[0];
    *h2_raw_signal = client_data.buffer.words[1];

    return STATUS_OK;
}

/**
 * sgp30_get_iaq_baseline() - read out the baseline from the chip
 *
 * The IAQ baseline should be retrieved and persisted for a faster sensor
 * startup. See sgp30_set_iaq_baseline() for further documentation.
 *
 * A valid baseline value is only returned approx. 60min after a call to
 * sgp30_iaq_init() when it is not followed by a call to sgp30_set_iaq_baseline()
 * with a valid baseline.
 * This functions returns STATUS_FAIL if the baseline value is not valid.
 *
 * @baseline:   Pointer to raw u32 where to store the baseline
 *              If the method returns STATUS_FAIL, the baseline value must be
 *              discarded and must not be passed to sgp30_set_iaq_baseline().
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp30_get_iaq_baseline(u32 *baseline) {
    s16 ret = sgp30_run_profile_by_number(PROFILE_NUMBER_IAQ_GET_BASELINE);
    if (ret != STATUS_OK)
        return ret;

    *baseline = (((u32)client_data.buffer.words[0]) << 16) |
                 (u32)client_data.buffer.words[1];

    if (!SGP_VALID_IAQ_BASELINE(*baseline))
        return STATUS_FAIL;

    return STATUS_OK;
}


/**
 * sgp30_set_iaq_baseline() - set the on-chip baseline
 * @baseline:   A raw u32 baseline
 *              This value must be unmodified from what was retrieved by a
 *              successful call to sgp30_get_iaq_baseline() with return value
 *              STATUS_OK. A persisted baseline should not be set if it is
 *              older than one week.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
s16 sgp30_set_iaq_baseline(u32 baseline) {
    const struct sgp_profile *profile;
    u16 words[SENSIRION_NUM_WORDS(baseline)] = {
        (u16)((baseline & 0xffff0000) >> 16),
        (u16)(baseline & 0x0000ffff)
    };

    if (!SGP_VALID_IAQ_BASELINE(baseline))
        return STATUS_FAIL;

    profile = sgp30_get_profile_by_number(PROFILE_NUMBER_IAQ_SET_BASELINE);
    if (profile == NULL)
        return STATUS_FAIL;

    return sensirion_i2c_write_cmd_with_args(SGP_I2C_ADDRESS, profile->command,
                                             words, SENSIRION_NUM_WORDS(words));
}


/**
 * sgp30_get_tvoc_inceptive_baseline() - read the chip's tVOC inceptive baseline
 *
 * The inceptive baseline must only be used on the very first startup of the
 * sensor. It ensures that measured concentrations are consistent with the air
 * quality even before the first clean air event.
 *
 * @tvoc_inceptive_baseline:
 *              Pointer to raw u16 where to store the inceptive baseline
 *              If the method returns STATUS_FAIL, the inceptive baseline value
 *              must be discarded and must not be passed to
 *              sgp30_set_tvoc_baseline().
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
s16 sgp30_get_tvoc_inceptive_baseline(u16 *tvoc_inceptive_baseline) {
    s16 ret;

    ret = sgp30_run_profile_by_number(PROFILE_NUMBER_IAQ_GET_TVOC_INCEPTIVE_BASELINE);
    if (ret != STATUS_OK)
        return ret;

    *tvoc_inceptive_baseline = client_data.buffer.words[0];

    return STATUS_OK;
}


/**
 * sgp30_set_tvoc_baseline() - set the on-chip tVOC baseline
 * @baseline:   A raw u16 tVOC baseline
 *              This value must be unmodified from what was retrieved by a
 *              successful call to sgp30_get_tvoc_inceptive_baseline() with return
 *              value STATUS_OK.
 *
 * Return:      STATUS_OK on success, an error code otherwise
 */
s16 sgp30_set_tvoc_baseline(u16 tvoc_baseline) {
    const struct sgp_profile *profile;

    if (!SGP_VALID_IAQ_BASELINE(tvoc_baseline))
        return STATUS_FAIL;

    profile = sgp30_get_profile_by_number(PROFILE_NUMBER_IAQ_SET_TVOC_BASELINE);
    if (profile == NULL)
        return STATUS_FAIL;

    return sensirion_i2c_write_cmd_with_args(SGP_I2C_ADDRESS, profile->command,
                                             &tvoc_baseline,
                                             SENSIRION_NUM_WORDS(tvoc_baseline));
}


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
s16 sgp30_set_absolute_humidity(u32 absolute_humidity) {
    u64 ah = absolute_humidity;
    const struct sgp_profile *profile;
    u16 ah_scaled;

    if (!SGP_REQUIRE_FS(client_data.info.feature_set_version, 1, 0))
        return STATUS_FAIL; /* feature unavailable */

    profile = sgp30_get_profile_by_number(PROFILE_NUMBER_SET_ABSOLUTE_HUMIDITY);
    if (profile == NULL)
        return STATUS_FAIL;

    if (absolute_humidity > 256000)
        return STATUS_FAIL;

    /* ah_scaled = (ah / 1000) * 256 */
    ah_scaled = (u16)((ah * 256 * 16777) >> 24);

    return sensirion_i2c_write_cmd_with_args(SGP_I2C_ADDRESS, profile->command,
                                             &ah_scaled,
                                             SENSIRION_NUM_WORDS(ah_scaled));
}


/**
 * sgp30_get_driver_version() - Return the driver version
 * Return:  Driver version string
 */
const char *sgp30_get_driver_version()
{
    return SGP_DRV_VERSION_STR;
}


/**
 * sgp30_get_configured_address() - returns the configured I2C address
 *
 * Return:      u8 I2C address
 */
u8 sgp30_get_configured_address() {
    return SGP_I2C_ADDRESS;
}


/**
 * sgp30_get_feature_set_version() - Retrieve the sensor's feature set version and
 *                                 product type
 *
 * @feature_set_version:    The feature set version
 * @product_type:           The product type: 0 for sgp30, 1: sgpc3
 *
 * Return:  STATUS_OK on success
 */
s16 sgp30_get_feature_set_version(u16 *feature_set_version, u8 *product_type) {
    *feature_set_version = client_data.info.feature_set_version & 0x00FF;
    *product_type = (u8)((client_data.info.feature_set_version & 0xF000) >> 12);
    return STATUS_OK;
}


/**
 * sgp30_get_serial_id() - Retrieve the sensor's serial id
 *
 * @serial_id:    Output variable for the serial id
 *
 * Return:  STATUS_OK on success
 */
s16 sgp30_get_serial_id(u64 *serial_id) {
    *serial_id = client_data.info.serial_id;
    return STATUS_OK;
}


/**
 * sgp30_iaq_init() - reset the SGP's internal IAQ baselines
 *
 * Return:  STATUS_OK on success.
 */
s16 sgp30_iaq_init() {
    return sgp30_run_profile_by_number(PROFILE_NUMBER_IAQ_INIT);
}


/**
 * sgp30_probe() - check if SGP sensor is available and initialize it
 *
 * This call aleady initializes the IAQ baselines (sgp30_iaq_init())
 *
 * Return:  STATUS_OK on success, an error code otherwise
 */
s16 sgp30_probe() {
    s16 err;
    u64 *serial_buf = &client_data.buffer.u64_value;

    *serial_buf = 0;
    client_data.current_state = WAIT_STATE;

    /* Initialize I2C */
    sensirion_i2c_init();

    /* try to read the serial ID */
    err = sensirion_i2c_delayed_read_cmd(SGP_I2C_ADDRESS,
                                         sgp30_cmd_get_serial_id,
                                         SGP_CMD_GET_SERIAL_ID_DURATION_US,
                                         client_data.buffer.words,
                                         SGP_CMD_GET_SERIAL_ID_WORDS);
    if (err != STATUS_OK)
        return err;

    SENSIRION_WORDS_TO_BYTES(client_data.buffer.words,
                             SGP_CMD_GET_SERIAL_ID_WORDS);
    client_data.info.serial_id = be64_to_cpu(*serial_buf) >> 16;

    /* read the featureset version */
    err = sensirion_i2c_delayed_read_cmd(SGP_I2C_ADDRESS,
                                         sgp30_cmd_get_featureset,
                                         SGP_CMD_GET_FEATURESET_DURATION_US,
                                         client_data.buffer.words,
                                         SGP_CMD_GET_FEATURESET_WORDS);
    if (err != STATUS_OK)
        return err;

    err = sgp30_detect_featureset_version(client_data.buffer.words);
    if (err != STATUS_OK)
        return err;

    return sgp30_iaq_init();
}

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
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "sensirion_i2c.h"
#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sgp_featureset.h"
#include "sgpc3.h"


#define SGP_DRV_VERSION_STR             "2.4.0"
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
#define SGP_CMD_GET_SERIAL_ID_WORDS         2
static const sgp_command sgp_cmd_get_serial_id = {
    .buf = {0x36, 0x82}
};

/* command and constants for reading the featureset version */
#define SGP_CMD_GET_FEATURESET_DURATION_US  1000
#define SGP_CMD_GET_FEATURESET_WORDS        1
static const sgp_command sgp_cmd_get_featureset = {
    .buf = {0x20, 0x2f}
};

/* command and constants for on-chip self-test */
#define SGP_CMD_MEASURE_TEST_DURATION_US    220000
#define SGP_CMD_MEASURE_TEST_WORDS          1
#define SGP_CMD_MEASURE_TEST_OK             0xd400
static const sgp_command sgp_cmd_measure_test = {
    .buf = {0x20, 0x32}
};

static const struct sgp_otp_featureset sgp_features_unknown = {
    .profiles = NULL,
    .number_of_profiles = 0,
};

enum sgp_state_code {
    WAIT_STATE,
    MEASURING_PROFILE_STATE
};

struct sgp_info {
    u64 serial_id;
    u16 feature_set_version;
};

static struct sgp_data {
    enum sgp_state_code current_state;
    struct sgp_info info;
    const struct sgp_otp_featureset *otp_features;
    u16 word_buf[SGP_BUFFER_WORDS];
} client_data;


/**
 * sgp_i2c_read_words() - read data words from SGP sensor
 * @data:       Allocated buffer to store the read data.
 *              The buffer may also have been modified on STATUS_FAIL return.
 * @data_words: Number of data words to read (without CRC bytes)
 *
 * Return:      STATUS_OK on success, STATUS_FAIL otherwise
 */
static s16 sgp_i2c_read_words(u16 *data, u16 data_words) {
    s16 ret;
    u16 i, j;
    u16 size = data_words * 3;
    u16 word_buf[SGP_MAX_PROFILE_RET_LEN / sizeof(u16)];
    u8 * const buf8 = (u8 *)word_buf;

    ret = sensirion_i2c_read(SGP_I2C_ADDRESS, buf8, size);

    if (ret != 0)
        return STATUS_FAIL;

    /* check the CRC for each word */
    for (i = 0, j = 0;
         i < size;
         i += SGP_WORD_LEN + CRC8_LEN, j += SGP_WORD_LEN) {

        if (sensirion_common_check_crc(&buf8[i], SGP_WORD_LEN,
                                       buf8[i + SGP_WORD_LEN]) == STATUS_FAIL) {
            return STATUS_FAIL;
        }
        ((u8 *)data)[j]     = buf8[i];
        ((u8 *)data)[j + 1] = buf8[i + 1];
    }

    return STATUS_OK;
}


/**
 * sgp_i2c_write() - writes to the SGP sensor
 * @command:     Command
 *
 * Return:      STATUS_OK on success.
 */
static s16 sgp_i2c_write(const sgp_command *command) {
    s8 ret;

    ret = sensirion_i2c_write(SGP_I2C_ADDRESS, command->buf, SGP_COMMAND_LEN);
    if (ret != 0)
        return STATUS_FAIL;

    return STATUS_OK;
}


/**
 * unpack_signals() - unpack signals which are stored in client_data.word_buf
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
        word_buf[i] = client_data.word_buf[i];

    /* signals are in reverse order in the data buffer */
    for (i = profile->number_of_signals - 1, j = 0; i >= 0; i -= 1, j += 1) {
        signal = profile->signals[profile->number_of_signals - i - 1];
        value = be16_to_cpu(word_buf[i]);

        if (signal->conversion_function != NULL)
            client_data.word_buf[j] = signal->conversion_function(value);
        else
            client_data.word_buf[j] = value;
    }
}

/**
 * read_measurement() - reads the result of a profile measurement
 *
 * Return:  Length of the written data to the buffer. Negative if it fails.
 */
static s16 read_measurement(const struct sgp_profile *profile) {

    s16 ret;

    switch (client_data.current_state) {

        case MEASURING_PROFILE_STATE:
            ret = sgp_i2c_read_words(client_data.word_buf,
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
 * sgp_i2c_read_words_from_cmd() - reads data words from the SGP sensor after a
 *                                 command has been issued
 * @cmd:        Command
 * @data_words: Allocated buffer to store the read data
 * @num_words:  Data words to read (without CRC bytes)
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
static s16 sgp_i2c_read_words_from_cmd(const sgp_command *cmd,
                                       u32 duration_us,
                                       u16 *data_words,
                                       u16 num_words) {

    if (sgp_i2c_write(cmd) == STATUS_FAIL)
        return STATUS_FAIL;

    /* the chip needs some time to write the data into the RAM */
    sensirion_sleep_usec(duration_us);
    return sgp_i2c_read_words(data_words, num_words);
}


/**
 * sgp_run_profile() - run a profile and read write its return to client_data
 * @profile     A pointer to the profile
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
static s16 sgp_run_profile(const struct sgp_profile *profile) {
    u32 duration_us = profile->duration_us + 5;

    if (sgp_i2c_write(&profile->command) == STATUS_FAIL)
        return STATUS_FAIL;

    sensirion_sleep_usec(duration_us);

    if (profile->number_of_signals > 0) {
        client_data.current_state = MEASURING_PROFILE_STATE;
        return read_measurement(profile);
    }

    return STATUS_OK;
}


/**
 * sgp_get_profile_by_number() - get a profile by its identifier number
 * @number      The number that identifies the profile
 *
 * Return:      A pointer to the profile or NULL if the profile does not exists
 */
static const struct sgp_profile *sgp_get_profile_by_number(u16 number) {
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
 * sgp_run_profile_by_number() - run a profile by its identifier number
 * @number:     The number that identifies the profile
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
static s16 sgp_run_profile_by_number(u16 number) {
    const struct sgp_profile *profile;

    profile = sgp_get_profile_by_number(number);
    if (profile == NULL)
        return STATUS_FAIL;

    if (sgp_run_profile(profile) == STATUS_FAIL)
        return STATUS_FAIL;

    return STATUS_OK;
}


/**
 * sgp_fill_cmd_send_buf() - create the i2c send buffer for a command and a set
 *                           of argument words.
 *                           The output buffer interleaves argument words with
 *                           their checksums.
 * @buf:        The generated buffer to send over i2c. Then buffer length must
 *              be at least SGP_COMMAND_LEN + num_args * (SGP_WORD_LEN +
 *              CRC8_LEN).
 * @cmd:        The sgp i2c command to send. It already includes a checksum.
 * @args:       The arguments to the command. Can be NULL if none.
 * @num_args:   The number of word arguments in args.
 * Return:      Bytes written to buf: SGP_COMMAND_LEN + num_args *
 *              (SGP_WORD_LEN + CRC8_LEN).
 */
static u16 sgp_fill_cmd_send_buf(u8 *buf, const sgp_command *cmd,
                                 const u16 *args, u8 num_args) {
    u16 word;
    u8 crc;
    u8 i;
    u8 idx = 0;

    buf[idx++] = cmd->buf[0];
    buf[idx++] = cmd->buf[1];

    for (i = 0; i < num_args; ++i) {
        word = be16_to_cpu(args[i]);
        crc = sensirion_common_generate_crc((u8 *)&word, SGP_WORD_LEN);

        buf[idx++] = (u8)((word & 0x00FF) >> 0);
        buf[idx++] = (u8)((word & 0xFF00) >> 8);
        buf[idx++] = crc;
    }
    return idx;
}


/**
 * sgp_detect_featureset_version() - extracts the featureset and initializes
 *                                   client_data.
 *
 * @featureset:  Pointer to the featureset bits
 *
 * Return:    STATUS_OK on success
 */
static s16 sgp_detect_featureset_version(u16 *featureset) {
    s16 i, j;
    s16 ret = STATUS_FAIL;
    u16 feature_set_version = be16_to_cpu(*featureset);
    const struct sgp_otp_featureset *sgp_featureset;

    client_data.info.feature_set_version = feature_set_version;
    client_data.otp_features = &sgp_features_unknown;
    for (i = 0; i < sgp_supported_featuresets.number_of_supported_featuresets; ++i) {
        sgp_featureset = sgp_supported_featuresets.featuresets[i];
        for (j = 0; j < sgp_featureset->number_of_supported_featureset_versions; ++j) {
            if (SGP_FS_COMPAT(feature_set_version,
                              sgp_featureset->supported_featureset_versions[j])) {
                client_data.otp_features = sgp_featureset;
                ret = STATUS_OK;
                break;
            }
        }
    }
    return ret;
}


/**
 * sgp_measure_test() - Run the on-chip self-test
 *
 * This method is executed synchronously and blocks for the duration of the
 * measurement (~220ms)
 *
 * @test_result:    Allocated buffer to store the chip's error code.
 *                  test_result is SGP_CMD_MEASURE_TEST_OK on success or set to
 *                  zero (0) in the case of a communication error.
 *
 * Return: STATUS_OK on a successful self-test, STATUS_FAIL otherwise.
 */
s16 sgp_measure_test(u16 *test_result) {
    u16 measure_test_word_buf[SGP_CMD_MEASURE_TEST_WORDS];

    *test_result = 0;

    if (sgp_i2c_write(&sgp_cmd_measure_test) != STATUS_OK)
        return STATUS_FAIL;

    sensirion_sleep_usec(SGP_CMD_MEASURE_TEST_DURATION_US);

    if (sgp_i2c_read_words(measure_test_word_buf,
                           SGP_CMD_MEASURE_TEST_WORDS) != STATUS_OK)
        return STATUS_FAIL;

    *test_result = be16_to_cpu(*measure_test_word_buf);
    if (*test_result == SGP_CMD_MEASURE_TEST_OK)
        return STATUS_OK;

    return STATUS_FAIL;
}


/**
 * sgp_measure_iaq() - Measure IAQ values async
 *
 * The profile is executed asynchronously. Use sgp_read_iaq to get the
 * values.
 *
 * Return:  STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_measure_iaq() {
    const struct sgp_profile *profile;

    profile = sgp_get_profile_by_number(PROFILE_NUMBER_IAQ_MEASURE);
    if (profile == NULL) {
        return STATUS_FAIL;
    }

    if (sgp_i2c_write(&(profile->command)) == STATUS_FAIL)
        return STATUS_FAIL;
    client_data.current_state = MEASURING_PROFILE_STATE;

    return STATUS_OK;
}


/**
 * sgp_read_iaq() - Read IAQ values async
 *
 * Read the IAQ values. This command can only be exectued after a measurement
 * has started with sgp_measure_iaq and is finished.
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_read_iaq(u16 *tvoc_ppb) {
    const struct sgp_profile *profile;

    profile = sgp_get_profile_by_number(PROFILE_NUMBER_IAQ_MEASURE);
    if (profile == NULL)
        return STATUS_FAIL;

    if (read_measurement(profile) == STATUS_FAIL)
        return STATUS_FAIL;

    *tvoc_ppb = client_data.word_buf[0];

    return STATUS_OK;
}


/**
 * sgp_measure_iaq_blocking_read() - Measure tVOC concentration
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 *
 * The profile is executed synchronously.
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_measure_iaq_blocking_read(u16 *tvoc_ppb) {
    if (sgp_run_profile_by_number(PROFILE_NUMBER_IAQ_MEASURE) == STATUS_FAIL)
        return STATUS_FAIL;

    *tvoc_ppb = client_data.word_buf[0];

    return STATUS_OK;
}


/**
 * sgp_measure_tvoc() - Measure tVOC concentration async
 *
 * The profile is executed asynchronously. Use sgp_read_tvoc to get the
 * ppb value.
 *
 * Return:  STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_measure_tvoc() {
    return sgp_measure_iaq();
}


/**
 * sgp_read_tvoc() - Read tVOC concentration async
 *
 * Read the tVOC value. This command can only be exectued after a measurement
 * has started with sgp_measure_tvoc and is finished.
 *
 * @tvoc_ppb:   The tVOC ppb value will be written to this location
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_read_tvoc(u16 *tvoc_ppb) {
    return sgp_read_iaq(tvoc_ppb);
}


/**
 * sgp_measure_tvoc_blocking_read() - Measure tVOC concentration
 *
 * The profile is executed synchronously.
 *
 * Return:  tVOC concentration in ppb. Negative if it fails.
 */
s16 sgp_measure_tvoc_blocking_read(u16 *tvoc_ppb) {
    return sgp_measure_iaq_blocking_read(tvoc_ppb);
}


/**
 * sgp_measure_signals_blocking_read() - Measure signals
 * The profile is executed synchronously.
 *
 * @scaled_ethanol_signal: Output variable for the ethanol signal
 *                         The signal is scaled by factor 512. Divide the scaled
 *                         value by 512 to get the real signal.
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_measure_signals_blocking_read(u16 *scaled_ethanol_signal) {

    if (sgp_run_profile_by_number(PROFILE_NUMBER_MEASURE_SIGNALS) == STATUS_FAIL)
        return STATUS_FAIL;

    *scaled_ethanol_signal = client_data.word_buf[0];

    return STATUS_OK;
}


/**
 * sgp_measure_signals() - Measure signals async
 *
 * The profile is executed asynchronously. Use sgp_read_signals to get
 * the signal values.
 *
 * Return:  STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_measure_signals(void) {
    const struct sgp_profile *profile;

    profile = sgp_get_profile_by_number(PROFILE_NUMBER_MEASURE_SIGNALS);
    if (profile == NULL) {
        return STATUS_FAIL;
    }

    if (sgp_i2c_write(&(profile->command)) == STATUS_FAIL)
        return STATUS_FAIL;
    client_data.current_state = MEASURING_PROFILE_STATE;

    return STATUS_OK;
}


/**
 * sgp_read_signals() - Read signals async
 * This command can only be exectued after a measurement has been started with
 * sgp_measure_signals and has finished.
 *
 * @scaled_ethanol_signal: Output variable for ethanol signal.
 *                         The signal is scaled by factor 512. Divide the scaled
 *                         value by 512 to get the real signal.
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_read_signals(u16 *scaled_ethanol_signal) {
    const struct sgp_profile *profile;

    profile = sgp_get_profile_by_number(PROFILE_NUMBER_MEASURE_SIGNALS);
    if (profile == NULL)
        return STATUS_FAIL;

    if (read_measurement(profile) == STATUS_FAIL)
        return STATUS_FAIL;

    *scaled_ethanol_signal = client_data.word_buf[0];

    return STATUS_OK;
}


/**
 * sgp_measure_raw_blocking_read() - Measure tvoc and signals
 * The profile is executed synchronously.
 *
 * @tvoc_ppb:              The tVOC ppb value will be written to this location
 * @scaled_ethanol_signal: Output variable for the ethanol signal
 *                         The signal is scaled by factor 512. Divide the scaled
 *                         value by 512 to get the real signal.
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_measure_raw_blocking_read(u16 *tvoc_ppb, u16 *scaled_ethanol_signal) {

    if (sgp_run_profile_by_number(PROFILE_NUMBER_MEASURE_RAW) == STATUS_FAIL)
        return STATUS_FAIL;

    *tvoc_ppb = client_data.word_buf[0];
    *scaled_ethanol_signal = client_data.word_buf[1];

    return STATUS_OK;
}


/**
 * sgp_measure_raw() - Measure raw async
 *
 * The profile is executed asynchronously. Use sgp_read_raw to get
 * the tvoc and signal values.
 *
 * Return:  STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_measure_raw(void) {
    const struct sgp_profile *profile;

    profile = sgp_get_profile_by_number(PROFILE_NUMBER_MEASURE_RAW);
    if (profile == NULL) {
        return STATUS_FAIL;
    }

    if (sgp_i2c_write(&(profile->command)) == STATUS_FAIL)
        return STATUS_FAIL;
    client_data.current_state = MEASURING_PROFILE_STATE;

    return STATUS_OK;
}


/**
 * sgp_read_raw() - Read tvoc and signals async
 * This command can only be exectued after a measurement has been started with
 * sgp_measure_raw and has finished.
 *
 * @tvoc_ppb:              The tVOC ppb value will be written to this location
 * @scaled_ethanol_signal: Output variable for ethanol signal.
 *                         The signal is scaled by factor 512. Divide the scaled
 *                         value by 512 to get the real signal.
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_read_raw(u16 *tvoc_ppb, u16 *scaled_ethanol_signal) {
    const struct sgp_profile *profile;

    profile = sgp_get_profile_by_number(PROFILE_NUMBER_MEASURE_RAW);
    if (profile == NULL)
        return STATUS_FAIL;

    if (read_measurement(profile) == STATUS_FAIL)
        return STATUS_FAIL;

    *tvoc_ppb = client_data.word_buf[0];
    *scaled_ethanol_signal = client_data.word_buf[1];

    return STATUS_OK;
}


/**
 * sgp_get_iaq_baseline() - read out the baseline from the chip
 *
 * The IAQ baseline should be retrieved and persisted for a faster sensor
 * startup. See sgp_set_iaq_baseline() for further documentation.
 *
 * A valid baseline value is only returned approx. 60min after a call to
 * sgp_iaq_init() when it is not followed by a call to sgp_set_iaq_baseline()
 * with a valid baseline.
 * This functions returns STATUS_FAIL if the baseline value is not valid.
 *
 * @baseline:   Pointer to raw u16 where to store the baseline
 *              If the method returns STATUS_FAIL, the baseline value must be
 *              discarded and must not be passed to sgp_set_iaq_baseline().
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_get_iaq_baseline(u16 *baseline) {
    s16 ret = sgp_run_profile_by_number(PROFILE_NUMBER_IAQ_GET_BASELINE);
    if (ret == STATUS_FAIL)
        return STATUS_FAIL;

    *baseline = client_data.word_buf[0];

    if (!SGP_VALID_IAQ_BASELINE(*baseline))
        return STATUS_FAIL;

    return STATUS_OK;
}


/**
 * sgp_set_iaq_baseline() - set the on-chip baseline
 * @baseline:   A raw u16 baseline
 *              This value must be unmodified from what was retrieved by a
 *              successful call to sgp_get_iaq_baseline() with return value
 *              STATUS_OK. A persisted baseline should not be set if it is
 *              older than one week.
 *
 * Return:      STATUS_OK on success, else STATUS_FAIL
 */
s16 sgp_set_iaq_baseline(u16 baseline) {
    const u16 BUF_SIZE = SGP_COMMAND_LEN + SGP_WORD_LEN + CRC8_LEN;
    u8 buf[BUF_SIZE];
    const struct sgp_profile *profile;

    if (!SGP_VALID_IAQ_BASELINE(baseline))
        return STATUS_FAIL;

    profile = sgp_get_profile_by_number(PROFILE_NUMBER_IAQ_SET_BASELINE);
    sgp_fill_cmd_send_buf(buf, &profile->command, &baseline,
                          sizeof(baseline) / SGP_WORD_LEN);

    if (sensirion_i2c_write(SGP_I2C_ADDRESS, buf, BUF_SIZE) != 0)
        return STATUS_FAIL;

    return STATUS_OK;
}


/**
 * sgp_get_driver_version() - Return the driver version
 * Return:  Driver version string
 */
const char *sgp_get_driver_version()
{
    return SGP_DRV_VERSION_STR;
}


/**
 * sgp_get_configured_address() - returns the configured I2C address
 *
 * Return:      u8 I2C address
 */
u8 sgp_get_configured_address() {
    return SGP_I2C_ADDRESS;
}


/**
 * sgp_get_feature_set_version() - Retrieve the sensor's feature set version and
 *                                 product type
 *
 * @feature_set_version:    The feature set version
 * @product_type:           The product type: 0 for sgp30, 1: sgpc3
 *
 * Return:  STATUS_OK on success
 */
s16 sgp_get_feature_set_version(u16 *feature_set_version, u8 *product_type) {
    *feature_set_version = client_data.info.feature_set_version & 0x00FF;
    *product_type = (u8)((client_data.info.feature_set_version & 0xC0000) >> 14);
    return STATUS_OK;
}


/**
 * sgp_get_serial_id() - Retrieve the sensor's serial id
 *
 * @serial_id:    Output variable for the serial id
 *
 * Return:  STATUS_OK on success
 */
s16 sgp_get_serial_id(u64 *serial_id) {
    *serial_id = client_data.info.serial_id;
    return STATUS_OK;
}



/**
 * sgp_iaq_init() - reset the SGP's internal IAQ baselines using the default
 *                  iaq init time of 64s
 *
 * Return:  STATUS_OK on success.
 */
s16 sgp_iaq_init() {
    return sgp_iaq_init64();
}

/**
 * sgp_iaq_init0() - reset the SGP's internal IAQ baselines using iaq init time
 *                   of 0s
 *
 * Return:  STATUS_OK on success.
 */
s16 sgp_iaq_init0() {
    return sgp_run_profile_by_number(PROFILE_NUMBER_IAQ_INIT0);
}

/**
 * sgp_iaq_init16() - reset the SGP's internal IAQ baselines using iaq init time
 *                    of 16s
 *
 * Return:  STATUS_OK on success.
 */
s16 sgp_iaq_init16() {
    return sgp_run_profile_by_number(PROFILE_NUMBER_IAQ_INIT16);
}

/**
 * sgp_iaq_init64() - reset the SGP's internal IAQ baselines using iaq init time
 *                    of 64s
 *
 * Return:  STATUS_OK on success.
 */
s16 sgp_iaq_init64() {
    return sgp_run_profile_by_number(PROFILE_NUMBER_IAQ_INIT64);
}

/**
 * sgp_iaq_init184() - reset the SGP's internal IAQ baselines using iaq init
 *                     time of 184s
 *
 * Return:  STATUS_OK on success.
 */
s16 sgp_iaq_init184() {
    return sgp_run_profile_by_number(PROFILE_NUMBER_IAQ_INIT184);
}


/**
 * sgp_probe() - check if SGP sensor is available and initialize it
 *
 * This call aleady initializes the IAQ baselines (sgp_iaq_init())
 *
 * Return:  STATUS_OK on success.
 */
s16 sgp_probe() {
    s16 err;
    const u64 *serial_buf = (const u64 *)client_data.word_buf;

    client_data.current_state = WAIT_STATE;

    /* Initialize I2C */
    sensirion_i2c_init();

    /* try to read the serial ID */
    err = sgp_i2c_read_words_from_cmd(&sgp_cmd_get_serial_id,
                                      SGP_CMD_GET_SERIAL_ID_DURATION_US,
                                      client_data.word_buf,
                                      SGP_CMD_GET_SERIAL_ID_WORDS);
    if (err == STATUS_FAIL)
        return err;

    client_data.info.serial_id = be64_to_cpu(*serial_buf) >> 16;

    /* read the featureset version */
    err = sgp_i2c_read_words_from_cmd(&sgp_cmd_get_featureset,
                                      SGP_CMD_GET_FEATURESET_DURATION_US,
                                      client_data.word_buf,
                                      SGP_CMD_GET_FEATURESET_WORDS);
    if (err == STATUS_FAIL)
        return STATUS_FAIL;

    err = sgp_detect_featureset_version(client_data.word_buf);
    if (err == STATUS_FAIL)
        return STATUS_FAIL;

    return sgp_iaq_init();
}

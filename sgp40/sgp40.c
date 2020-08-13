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

#include "sgp40.h"
#include "sgp_git_version.h"

static const uint8_t SGP40_I2C_ADDRESS = 0x59;

#define SGP40_CMD_MEASURE_RAW_WORDS 1
#define SGP40_CMD_MEASURE_RAW 0x260f

/* command and constants for reading the serial ID */
#define SGP40_CMD_GET_SERIAL_ID_DURATION_US 500
#define SGP40_CMD_GET_SERIAL_ID_WORDS 3
#define SGP40_CMD_GET_SERIAL_ID 0x3682

/* command and constants for reading the featureset version */
#define SGP40_CMD_GET_FEATURESET_DURATION_US 1000
#define SGP40_CMD_GET_FEATURESET_WORDS 1
#define SGP40_CMD_GET_FEATURESET 0x202f

int16_t sgp40_measure_raw_blocking_read(uint16_t* sraw) {
    int16_t ret;

    ret = sgp40_measure_raw();
    if (ret != STATUS_OK)
        return ret;
    sensirion_sleep_usec(SGP40_CMD_MEASURE_RAW_DURATION_US);
    return sgp40_read_raw(sraw);
}

void sgp40_convert_rht(int32_t humidity, int32_t temperature,
                       uint16_t* humidity_sensor_format,
                       uint16_t* temperature_sensor_format) {
    if (humidity < 0) {
        humidity = 0;
    } else if (humidity > 100000) {
        humidity = 100000;
    }
    if (temperature < -45000) {
        temperature = -45000;
    } else if (temperature > 129760) {
        temperature = 129760;
    }

    /* humidity_sensor_format = humidity / 100000 * 65535;
     * 65535 / 100000 = 0.65535 -> 0.65535 * 2^5 = 20.9712 / 2^10 ~= 671
     */
    *humidity_sensor_format = (uint16_t)((humidity * 671) >> 10);

    /* temperature_sensor_format[1] = (temperature + 45000) / 175000 * 65535;
     * 65535 / 175000 ~= 0.375 -> 0.375 * 2^3 = 2.996 ~= 3
     */
    *temperature_sensor_format = (uint16_t)(((temperature + 45000) * 3) >> 3);
}

int16_t sgp40_measure_raw_with_rht(int32_t humidity, int32_t temperature) {
    uint16_t args[2];
    sgp40_convert_rht(humidity, temperature, &args[0], &args[1]);
    return sensirion_i2c_write_cmd_with_args(
        SGP40_I2C_ADDRESS, SGP40_CMD_MEASURE_RAW, args, ARRAY_SIZE(args));
}

int16_t sgp40_measure_raw_with_rht_blocking_read(int32_t humidity,
                                                 int32_t temperature,
                                                 uint16_t* sraw) {
    int16_t error;
    error = sgp40_measure_raw_with_rht(humidity, temperature);
    if (error) {
        return error;
    }
    sensirion_sleep_usec(SGP40_CMD_MEASURE_RAW_DURATION_US);
    return sgp40_read_raw(sraw);
}

int16_t sgp40_measure_raw(void) {
    uint16_t args[2] = {SGP40_DEFAULT_HUMIDITY, SGP40_DEFAULT_TEMPERATURE};
    return sensirion_i2c_write_cmd_with_args(
        SGP40_I2C_ADDRESS, SGP40_CMD_MEASURE_RAW, args, ARRAY_SIZE(args));
}

int16_t sgp40_read_raw(uint16_t* sraw) {
    return sensirion_i2c_read_words(SGP40_I2C_ADDRESS, sraw,
                                    SENSIRION_NUM_WORDS(*sraw));
}

const char* sgp40_get_driver_version(void) {
    return SGP_DRV_VERSION_STR;
}

uint8_t sgp40_get_configured_address(void) {
    return SGP40_I2C_ADDRESS;
}

int16_t sgp40_get_serial_id(uint8_t* serial_id) {
    int16_t ret;

    ret = sensirion_i2c_write_cmd(SGP40_I2C_ADDRESS, SGP40_CMD_GET_SERIAL_ID);
    if (ret != STATUS_OK)
        return ret;
    sensirion_sleep_usec(SGP40_CMD_GET_SERIAL_ID_DURATION_US);
    return sensirion_i2c_read_words_as_bytes(SGP40_I2C_ADDRESS, serial_id,
                                             SGP40_CMD_GET_SERIAL_ID_WORDS);
}

int16_t sgp40_probe(void) {
    uint8_t serial[SGP40_SERIAL_ID_NUM_BYTES];

    return sgp40_get_serial_id(serial);
}

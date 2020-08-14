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

#ifndef SGP40_H
#define SGP40_H
#include "sensirion_arch_config.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SGP40_CMD_MEASURE_RAW_DURATION_US 100000
#define SGP40_DEFAULT_HUMIDITY 0x8000
#define SGP40_DEFAULT_TEMPERATURE 0x6666
#define SGP40_SERIAL_ID_NUM_BYTES 6

/**
 * sgp40_probe() - check if SGP sensor is available
 *
 * @return: STATUS_OK on success, an error code otherwise
 */
int16_t sgp40_probe(void);

/**
 * sgp40_get_driver_version() - Return the driver version
 *
 * @return: Driver version string
 */
const char* sgp40_get_driver_version(void);

/**
 * sgp40_get_configured_address() - returns the configured I2C address
 *
 * @return: uint8_t I2C address
 */
uint8_t sgp40_get_configured_address(void);

/**
 * sgp40_get_serial_id() - Retrieve the sensor's serial id
 *
 * To convert it to decimal use the following:
 *
 *     uint64_t serial_id_dec;
 *     uint8_t serial_id[SGP40_SERIAL_ID_NUM_BYTES];
 *     int16_t error;
 *     error = sgp40_get_serial_id(serial_id);
 *     if (error == STATUS_OK) {
 *         serial_id_dec = 0;
 *         for (int i = 0; i < SGP40_SERIAL_ID_NUM_BYTES; ++i) {
 *             serial_id_dec = (serial_id_dec << 8) | serial_id[i];
 *         }
 *     }
 *
 * @serial_id:  Output array for the serial id of type uint8_t
 *              and length SGP40_SERIAL_ID_NUM_BYTES
 *
 * @return: STATUS_OK on success
 */
int16_t sgp40_get_serial_id(uint8_t* serial_id);

/**
 * Convert humidity and temperature in the format used by the sensor.
 *
 * @param humidity                  Relative humidity in [percent relative
 *                                  humidity], multiplied by 1000.
 * @param temperature               Temperature in [degree Celsius], multiplied
 *                                  by 1000.
 * @param humidity_sensor_format    Output parameter, humidity in the format
 *                                  expected by the sensor.
 * @param temperature_sensor_format Output parameter, temperature in the format
 *                                  expected by the sensor.
 */
void sgp40_convert_rht(int32_t humidity, int32_t temperature,
                       uint16_t* humidity_sensor_format,
                       uint16_t* temperature_sensor_format);

/**
 * sgp40_measure_raw_blocking_read() - Measure raw signals
 * The profile is executed synchronously.
 *
 * @sraw: Output variable for the sraw signal
 *
 * @return: STATUS_OK on success, an error code otherwise
 */
int16_t sgp40_measure_raw_blocking_read(uint16_t* sraw);

/**
 * sgp40_measure_raw_with_rht() - Measure raw signals async with compensation.
 *
 * Start a measurement temperature and humidity compensation asynchronously.
 * Use sgp40_read_raw to get the signal value after waiting for the measurement
 * to finish.
 *
 *
 * @param humidity      Relative humidity in [percent relative humidity],
 *                      multiplied by 1000.
 * @param temperature   Temperature in [degree Celsius], multiplied by 1000,
 *
 * @return STATUS_OK on success, an error code otherwise
 */
int16_t sgp40_measure_raw_with_rht(int32_t humidity, int32_t temperature);

/**
 * sgp40_measure_raw_with_rht_blocking_read() - Measure raw signals
 * The profile is executed synchronously.
 *
 * @param humidity      Relative humidity in [percent relative humidity],
 *                      multiplied by 1000.
 * @param temperature   Temperature in [degree Celsius], multiplied by 1000,
 * @param sraw          Output variable for the sraw signal
 *
 * @return: STATUS_OK on success, an error code otherwise
 */
int16_t sgp40_measure_raw_with_rht_blocking_read(int32_t humidity,
                                                 int32_t temperature,
                                                 uint16_t* sraw);

/**
 * sgp40_measure_raw() - Measure raw signals async
 *
 * The profile is executed asynchronously. Use sgp40_read_raw to get the
 * signal values.
 *
 * @return: STATUS_OK on success, an error code otherwise
 */
int16_t sgp40_measure_raw(void);
/**
 * sgp40_read_raw() - Read raw signals async
 * This command can only be exectued after a measurement started with
 * sgp40_measure_raw and has finished.
 *
 * @sraw: Output variable for the sraw signal
 *
 * @return: STATUS_OK on success, an error code otherwise
 */
int16_t sgp40_read_raw(uint16_t* sraw);

#ifdef __cplusplus
}
#endif

#endif /* SGP40_H */

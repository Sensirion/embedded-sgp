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

#include <stdio.h>  // printf

/* TO USE CONSOLE OUTPUT (printf) YOU MAY NEED TO ADAPT THE
 * INCLUDES ABOVE OR DEFINE THEM ACCORDING TO YOUR PLATFORM.
 * #define printf(...)
 */

int main(void) {
    int16_t err;
    uint16_t sraw;
    uint16_t ix;

    const char* driver_version = sgp40_get_driver_version();
    if (driver_version) {
        printf("SGP40 driver version %s\n", driver_version);
    } else {
        printf("fatal: Getting driver version failed\n");
        return -1;
    }

    /* Initialize I2C bus */
    sensirion_i2c_init();

    /* Busy loop for initialization. The main loop does not work without
     * a sensor. */
    while (sgp40_probe() != STATUS_OK) {
        printf("SGP sensor probing failed\n");
        sensirion_sleep_usec(1000000);
    }
    printf("SGP sensor probing successful\n");

    uint8_t serial_id[SGP40_SERIAL_ID_NUM_BYTES];
    err = sgp40_get_serial_id(serial_id);
    if (err == STATUS_OK) {
        printf("SerialID: ");
        for (ix = 0; ix < SGP40_SERIAL_ID_NUM_BYTES - 1; ix++) {
            printf("%02X:", serial_id[ix]);
        }
        printf("%02X\n", serial_id[ix]);
    } else {
        printf("sgp40_get_serial_id failed!\n");
    }

    /* Run periodic measurements at defined intervals */
    while (1) {
        err = sgp40_measure_raw_blocking_read(&sraw);
        if (err == STATUS_OK) {
            printf("sraw: %u\n", sraw);
        } else {
            printf("error reading signal\n");
        }

        sensirion_sleep_usec(1000000);
    }

    return 0;
}

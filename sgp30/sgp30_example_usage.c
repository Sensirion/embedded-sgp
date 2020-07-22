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

#include <inttypes.h>  // PRIu64
#include <stdio.h>     // printf
#include <unistd.h>    // sleep

/* TO USE CONSOLE OUTPUT (printf) AND WAIT (sleep) YOU MAY NEED TO ADAPT THE
 * INCLUDES ABOVE OR DEFINE THEM ACCORDING TO YOUR PLATFORM.
 * #define printf(...)
 * #define sleep(...)
 */

int main(void) {
    uint16_t i = 0;
    int16_t err;
    uint16_t tvoc_ppb, co2_eq_ppm;
    uint32_t iaq_baseline;
    uint16_t ethanol_raw_signal, h2_raw_signal;

    const char* driver_version = sgp30_get_driver_version();
    if (driver_version) {
        printf("SGP30 driver version %s\n", driver_version);
    } else {
        printf("fatal: Getting driver version failed\n");
        return -1;
    }

    /* Initialize I2C bus */
    sensirion_i2c_init();

    /* Busy loop for initialization. The main loop does not work without
     * a sensor. */
    int16_t probe;
    while (1) {
        probe = sgp30_probe();

        if (probe == STATUS_OK)
            break;

        if (probe == SGP30_ERR_UNSUPPORTED_FEATURE_SET)
            printf(
                "Your sensor needs at least feature set version 1.0 (0x20)\n");

        printf("SGP sensor probing failed\n");
        sensirion_sleep_usec(1000000);
    }

    printf("SGP sensor probing successful\n");

    uint16_t feature_set_version;
    uint8_t product_type;
    err = sgp30_get_feature_set_version(&feature_set_version, &product_type);
    if (err == STATUS_OK) {
        printf("Feature set version: %u\n", feature_set_version);
        printf("Product type: %u\n", product_type);
    } else {
        printf("sgp30_get_feature_set_version failed!\n");
    }
    uint64_t serial_id;
    err = sgp30_get_serial_id(&serial_id);
    if (err == STATUS_OK) {
        printf("SerialID: %" PRIu64 "\n", serial_id);
    } else {
        printf("sgp30_get_serial_id failed!\n");
    }

    /* Read gas raw signals */
    err = sgp30_measure_raw_blocking_read(&ethanol_raw_signal, &h2_raw_signal);
    if (err == STATUS_OK) {
        /* Print ethanol raw signal and h2 raw signal */
        printf("Ethanol raw signal: %u\n", ethanol_raw_signal);
        printf("H2 raw signal: %u\n", h2_raw_signal);
    } else {
        printf("error reading raw signals\n");
    }

    /* Consider the two cases (A) and (B):
     * (A) If no baseline is available or the most recent baseline is more than
     *     one week old, it must discarded. A new baseline is found with
     *     sgp30_iaq_init() */
    err = sgp30_iaq_init();
    if (err == STATUS_OK) {
        printf("sgp30_iaq_init done\n");
    } else {
        printf("sgp30_iaq_init failed!\n");
    }
    /* (B) If a recent baseline is available, set it after sgp30_iaq_init() for
     * faster start-up */
    /* IMPLEMENT: retrieve iaq_baseline from presistent storage;
     * err = sgp30_set_iaq_baseline(iaq_baseline);
     */

    /* Run periodic IAQ measurements at defined intervals */
    while (1) {
        /*
         * IMPLEMENT: get absolute humidity to enable humidity compensation
         * uint32_t ah = get_absolute_humidity(); // absolute humidity in mg/m^3
         * sgp30_set_absolute_humidity(ah);
         */

        err = sgp30_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
        if (err == STATUS_OK) {
            printf("tVOC  Concentration: %dppb\n", tvoc_ppb);
            printf("CO2eq Concentration: %dppm\n", co2_eq_ppm);
        } else {
            printf("error reading IAQ values\n");
        }

        /* Persist the current baseline every hour */
        if (++i % 3600 == 3599) {
            err = sgp30_get_iaq_baseline(&iaq_baseline);
            if (err == STATUS_OK) {
                /* IMPLEMENT: store baseline to presistent storage */
            }
        }

        /* The IAQ measurement must be triggered exactly once per second (SGP30)
         * to get accurate values.
         */
        sleep(1);  // SGP30
    }
    return 0;
}

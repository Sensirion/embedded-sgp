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
    uint16_t tvoc_ppb;
    uint16_t tvoc_baseline;
    uint16_t ethanol_raw_signal;

    const char* driver_version = sgpc3_get_driver_version();
    if (driver_version) {
        printf("SGPC3 driver version %s\n", driver_version);
    } else {
        printf("fatal: Getting driver version failed\n");
        return -1;
    }

    /* Initialize I2C bus */
    sensirion_i2c_init();

    /* Busy loop for initialization. The main loop does not work without
     * a sensor. */
    while (sgpc3_probe() != STATUS_OK) {
        printf("SGP sensor probing failed\n");
        sleep(1);
    }
    printf("SGP sensor probing successful\n");

    uint16_t feature_set_version;
    uint8_t product_type;
    err = sgpc3_get_feature_set_version(&feature_set_version, &product_type);
    if (err == STATUS_OK) {
        printf("Feature set version: %u\n", feature_set_version);
        printf("Product type: %u\n", product_type);
    } else {
        printf("sgpc3_get_feature_set_version failed!\n");
    }
    uint64_t serial_id;
    err = sgpc3_get_serial_id(&serial_id);
    if (err == STATUS_OK) {
        printf("SerialID: %" PRIu64 "\n", serial_id);
    } else {
        printf("sgpc3_get_serial_id failed!\n");
    }

    /* Read raw signals.
     * Do not run measure_raw between tVOC measurements
     * without saving the baseline before the call and
     * restoring it after with sgpc3_get_tvoc_baseline /
     * sgpc3_set_tvoc_baseline. If a recent baseline is not available, reset it
     * using sgpc3_tvoc_init_preheat prior to running tVOC measurements. */
    err = sgpc3_measure_raw_blocking_read(&ethanol_raw_signal);

    if (err == STATUS_OK) {
        /* Print raw ethanol signal */
        printf("Ethanol raw signal: %u\n", ethanol_raw_signal);
    } else {
        printf("error reading raw signal\n");
    }

    /* Consider the two cases (A) and (B):
     * (A) If no baseline is available or the most recent baseline is more than
     *     one week old, it must discarded. A new baseline is found with
     *     sgpc3_tvoc_init_preheat() */
    if (feature_set_version >= 0x06) {
        err = sgpc3_tvoc_init_preheat();
        /* IMPLEMENT: sleep for the desired accelerated warm-up duration */
        sleep(64);
    } else {
        /* feature sets older than 0x06 do not support tvoc_init_preheat */
        err = sgpc3_tvoc_init_64s_fs5();
    }
    if (err == STATUS_OK) {
        printf("Init done\n");
    } else {
        printf("Init failed!\n");
    }

    /* (B) If a recent baseline is available, set it after
     *      sgpc3_tvoc_init_preheat() for faster start-up */
    /* IMPLEMENT: retrieve tvoc_baseline from presistent storage;
     * err = sgpc3_set_tvoc_baseline(tvoc_baseline);
     */

    /* Run periodic tVOC measurements at defined intervals */
    while (1) {
        err = sgpc3_measure_tvoc_and_raw_blocking_read(&tvoc_ppb,
                                                       &ethanol_raw_signal);
        if (err == STATUS_OK) {
            printf("tVOC  Concentration: %dppb\n", tvoc_ppb);
            printf("ethanol: %d\n", ethanol_raw_signal);
        } else {
            printf("error reading tVOC value\n");
        }

        /* Persist the current baseline every hour */
        if (++i % 1800 == 1799) {
            err = sgpc3_get_tvoc_baseline(&tvoc_baseline);
            if (err == STATUS_OK) {
                /* IMPLEMENT: store baseline to presistent storage */
            }
        }

        /* The tVOC measurement must be triggered exactly once every two seconds
         * to get accurate values and to respect the duty cycle/power budget.
         */
        sleep(2);
    }
    return 0;
}

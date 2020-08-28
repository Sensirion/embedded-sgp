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

#include "svm30.h"

#include <stdio.h>  // printf

/* TO USE CONSOLE OUTPUT (printf) YOU MAY NEED TO ADAPT THE
 * INCLUDE ABOVE OR DEFINE IT ACCORDING TO YOUR PLATFORM.
 * #define printf(...)
 */

int main(void) {
    uint16_t i = 0;
    int16_t err;
    uint16_t tvoc_ppb, co2_eq_ppm;
    uint32_t iaq_baseline;
    int32_t temperature, humidity;

    /* Initialize I2C */
    sensirion_i2c_init();

    /* Busy loop for initialization. The main loop does not work without
     * a sensor. */
    while (svm_probe() != STATUS_OK) {
        printf("SVM30 module probing failed\n");
    }
    printf("SVM30 module probing successful\n");

    /* Consider the two cases (A) and (B):
     * (A) If no baseline is available or the most recent baseline is more than
     *     one week old, it must discarded. A new baseline is found with
     *     sgp_iaq_init() */
    err = sgp30_iaq_init();
    /* (B) If a recent baseline is available, set it after sgp_iaq_init() for
     * faster start-up */
    /* IMPLEMENT: retrieve iaq_baseline from presistent storage;
     * err = sgp_set_iaq_baseline(iaq_baseline);
     */

    /* Run periodic IAQ measurements at defined intervals */
    while (1) {
        err = svm_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm,
                                            &temperature, &humidity);
        if (err == STATUS_OK) {
            printf("tVOC  Concentration: %dppb\n", tvoc_ppb);
            printf("CO2eq Concentration: %dppm\n", co2_eq_ppm);
            printf("Temperature: %0.3fC\n", temperature / 1000.0f);
            printf("Humidity: %0.3f%%RH\n", humidity / 1000.0f);

        } else {
            printf("error reading sensor\n");
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
        sensirion_sleep_usec(1000000);  // SVM30 / SGP30
    }
    return 0;
}

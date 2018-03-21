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

#include "sgpc3.h"

/* TO USE CONSOLE OUTPUT (printf) AND WAIT (sleep) PLEASE ADAPT THEM TO YOUR
 * PLATFORM.
 *
 * #include <stdio.h> // printf
 * #include <unistd.h> // sleep
 */


int main(void) {
    u16 i = 0;
    s16 err;
    u16 tvoc_ppb;
    u16 iaq_baseline;
    u16 scaled_ethanol_signal;

    /* Busy loop for initialization. The main loop does not work without
     * a sensor. */
    while (sgp_probe() != STATUS_OK) {
        /* printf("SGP sensor probing failed\n"); */
    }
    /* printf("SGP sensor probing successful\n"); */


    /* Read signals.
     * Do not run measure_signals between IAQ measurements
     * (iaq, tVOC) without saving the baseline before the call and
     * restoring it after with sgp_get_iaq_baseline / sgp_set_iaq_baseline.
     * If a recent baseline is not available, reset it using
     * sgp_iaq_init_continuous prior to running IAQ measurements.  */
    err = sgp_measure_signals_blocking_read(&scaled_ethanol_signal);

    if (err == STATUS_OK) {
        /* Print ethanol signal with floating point support */
        /* printf("Ethanol signal: %f\n", scaled_ethanol_signal / 512.0f); */

        /* Print ethanol signal without floating point support */
        /* printf("ethanol signal: %u.%09llu\n", scaled_ethanol_signal >> 9,
         *          ((scaled_ethanol_signal & 0x01ff) * (u64)1000000000) >> 9);
         */
    } else {
        /* printf("error reading signals\n"); */
    }

    /* Consider the two cases (A) and (B):
     * (A) If no baseline is available or the most recent baseline is more than
     *     one week old, it must discarded. A new baseline is found with
     *     sgp_iaq_init_continuous() */
    err = sgp_iaq_init_continuous();
    /* (B) If a recent baseline is available, set it after
     *      sgp_iaq_init_continuous() for faster start-up */
    /* IMPLEMENT: retrieve iaq_baseline from presistent storage;
     * err = sgp_set_iaq_baseline(iaq_baseline);
     */

    /* IMPLEMENT: sleep for the desired accelerated warm-up duration */
    /* sleep(64); */

    /* Run periodic IAQ measurements at defined intervals */
    while (1) {
        err = sgp_measure_iaq_blocking_read(&tvoc_ppb);
        if (err == STATUS_OK) {
            /* printf("tVOC  Concentration: %dppb\n", tvoc_ppb); */
        } else {
            /* printf("error reading IAQ values\n"); */
        }

        /* Persist the current baseline every hour */
        if (++i % 3600 == 3599) {
            err = sgp_get_iaq_baseline(&iaq_baseline);
            if (err == STATUS_OK) {
                /* IMPLEMENT: store baseline to presistent storage */
            }
        }

        /* The IAQ measurement must be triggered exactly once every two seconds
         * to get accurate values and to respect the duty cycle/power budget.
         */
        /* sleep(2); */
    }
    return 0;
}

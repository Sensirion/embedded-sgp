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

#include <stdio.h> // printf
#include <unistd.h> // sleep

/* TO USE CONSOLE OUTPUT (printf) AND WAIT (sleep) YOU MAY NEED TO ADAPT THE
 * INCLUDES ABOVE OR DEFINE THEM ACCORDING TO YOUR PLATFORM.
 * #define printf(...)
 * #define sleep(...)
 */

int main(void) {
    u16 i = 0;
    s16 err;
    u16 tvoc_ppb;
    u16 iaq_baseline;
    u16 ethanol_raw_signal;

    /* Busy loop for initialization. The main loop does not work without
     * a sensor. */
    while (sgpc3_probe() != STATUS_OK) {
        printf("SGP sensor probing failed\n");
        sleep(1);
    }
    printf("SGP sensor probing successful\n");

    /* Read raw signals.
     * Do not run measure_raw between tVOC measurements
     * without saving the baseline before the call and
     * restoring it after with sgpc3_get_iaq_baseline / sgpc3_set_iaq_baseline.
     * If a recent baseline is not available, reset it using
     * sgpc3_iaq_init_continuous prior to running tVOC measurements.  */
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
     *     sgpc3_iaq_init_continuous() */
    err = sgpc3_iaq_init_continuous();
    /* (B) If a recent baseline is available, set it after
     *      sgpc3_iaq_init_continuous() for faster start-up */
    /* IMPLEMENT: retrieve iaq_baseline from presistent storage;
     * err = sgpc3_set_iaq_baseline(iaq_baseline);
     */

    /* IMPLEMENT: sleep for the desired accelerated warm-up duration */
    sleep(64);

    /* Run periodic tVOC measurements at defined intervals */
    while (1) {
        err = sgpc3_measure_tvoc_blocking_read(&tvoc_ppb);
        if (err == STATUS_OK) {
            printf("tVOC  Concentration: %dppb\n", tvoc_ppb);
        } else {
            printf("error reading tVOC value\n");
        }

        /* Persist the current baseline every hour */
        if (++i % 1800 == 1799) {
            err = sgpc3_get_iaq_baseline(&iaq_baseline);
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

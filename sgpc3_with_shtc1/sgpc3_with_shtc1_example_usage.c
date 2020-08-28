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

#include "sgpc3_with_shtc1.h"

#include <stdio.h>  // printf

/* TO USE CONSOLE OUTPUT (printf) YOU MAY NEED TO ADAPT THE
 * INCLUDE ABOVE OR DEFINE IT ACCORDING TO YOUR PLATFORM.
 * #define printf(...)
 */

int main(void) {
    uint16_t i = 0;
    int16_t err;
    uint16_t tvoc_ppb;
    uint16_t tvoc_baseline;
    int32_t temperature, humidity;

    /* Initialize I2C bus */
    sensirion_i2c_init();

    /* Busy loop for initialization. The main loop does not work without
     * a sensor. */
    while (sgpc3_with_shtc1_probe() != STATUS_OK) {
        printf("Failed to detect SGPC3 and/or SHTC1\n");
        sensirion_sleep_usec(1000000);  // wait 1s before trying again
    }
    printf("SGPC3 and SHTC1 sensors detected\n");

    /* Consider the two cases (A) and (B):
     * (A) If no baseline is available or the most recent baseline is more than
     *     one week old, it must discarded. A new baseline is found with
     *     sgpc3_tvoc_init_no_preheat() which is already included in
     *     sgpc3_with_shtc1_probe()
     */
    /* (B) If a recent baseline is available, set it after
     * sgpc3_tvoc_init_no_preheat() for faster start-up behavior */
    /* IMPLEMENT: retrieve tvoc_baseline from presistent storage;
     * err = sgpc3_set_tvoc_baseline(tvoc_baseline);
     */

    /* Run periodic IAQ measurements at defined intervals */
    while (1) {
        err = sgpc3_with_shtc1_measure_iaq_blocking_read(
            &tvoc_ppb, &temperature, &humidity);
        if (err == STATUS_OK) {
            printf("tVOC  Concentration: %dppb\n", tvoc_ppb);
            printf("Temperature: %0.3fC\n", temperature / 1000.0f);
            printf("Humidity: %0.3f%%RH\n", humidity / 1000.0f);

        } else {
            printf("error reading sensor\n");
        }

        /* Persist the current baseline every hour */
        if (++i % 1800 == 1799) {
            err = sgpc3_get_tvoc_baseline(&tvoc_baseline);
            if (err == STATUS_OK) {
                /* IMPLEMENT: store baseline to presistent storage */
            }
        }

        /* The tVOC measurement must be triggered exactly once every two seconds
         * (SGPC3) to get accurate values.
         */
        sensirion_sleep_usec(2000000);  // 2s interval for SGPC3
    }
    return 0;
}

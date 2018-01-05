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
#include "sensirion_common.h"
#include "sgp_featureset.h"

#define PROFILE_NUMBER_SIGNALS 9
#define PROFILE_MEASURE_RAW 8
#define PROFILE_IAQ_INIT0 17
#define PROFILE_IAQ_INIT16 4
#define PROFILE_IAQ_INIT64 0
#define PROFILE_IAQ_INIT184 13

const u8 PROFILE_NUMBER_MEASURE_SIGNALS = PROFILE_NUMBER_SIGNALS;
const u8 PROFILE_NUMBER_MEASURE_RAW = PROFILE_MEASURE_RAW;
const u8 PROFILE_NUMBER_IAQ_INIT0 = PROFILE_IAQ_INIT0;
const u8 PROFILE_NUMBER_IAQ_INIT16 = PROFILE_IAQ_INIT16;
const u8 PROFILE_NUMBER_IAQ_INIT64 = PROFILE_IAQ_INIT64;
const u8 PROFILE_NUMBER_IAQ_INIT184 = PROFILE_IAQ_INIT184;

static const struct sgp_signal ETHANOL_SIGNAL = {
    .conversion_function = NULL,
    .name = "ethanol_signal",
};

static const struct sgp_signal TVOC_PPB = {
    .conversion_function = NULL,
    .name = "tVOC",
};

static const struct sgp_signal BASELINE_WORD1 = {
    .conversion_function = NULL,
    .name = "baseline1",
};

static const struct sgp_signal *SGP_PROFILE_IAQ_MEASURE_SIGNALS[] =
    { &TVOC_PPB };

static const struct sgp_signal *SGP_PROFILE_IAQ_GET_BASELINE_SIGNALS[] =
    { &BASELINE_WORD1 };

static const struct sgp_signal *SGP_PROFILE_MEASURE_SIGNALS_SIGNALS[] =
    { &ETHANOL_SIGNAL };

static const struct sgp_signal *SGP_PROFILE_MEASURE_RAW_SIGNALS[] =
    { &TVOC_PPB, &ETHANOL_SIGNAL };


static const struct sgp_profile SGP_PROFILE_IAQ_INIT64 = {
    .number            = PROFILE_IAQ_INIT64,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = { .buf = {0x20, 0x03} },
    .name              = "iaq_init64",
};

static const struct sgp_profile SGP_PROFILE_IAQ_INIT0 = {
    .number            = PROFILE_IAQ_INIT0,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = { .buf = {0x20, 0x89} },
    .name              = "iaq_init0",
};

static const struct sgp_profile SGP_PROFILE_IAQ_INIT16 = {
    .number            = PROFILE_IAQ_INIT16,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = { .buf = {0x20, 0x24} },
    .name              = "iaq_init16",
};

static const struct sgp_profile SGP_PROFILE_IAQ_INIT184 = {
    .number            = PROFILE_IAQ_INIT184,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = { .buf = {0x20, 0x6a} },
    .name              = "iaq_init184",
};

static const struct sgp_profile SGP_PROFILE_IAQ_MEASURE = {
    .number            = PROFILE_NUMBER_IAQ_MEASURE,
    .duration_us       = 50000,
    .signals           = SGP_PROFILE_IAQ_MEASURE_SIGNALS,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_IAQ_MEASURE_SIGNALS),
    .command           = { .buf = {0x20, 0x08} },
    .name              = "iaq_measure",
};

static const struct sgp_profile SGP_PROFILE_IAQ_GET_BASELINE = {
    .number            = PROFILE_NUMBER_IAQ_GET_BASELINE,
    .duration_us       = 10000,
    .signals           = SGP_PROFILE_IAQ_GET_BASELINE_SIGNALS,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_IAQ_GET_BASELINE_SIGNALS),
    .command           = { .buf = {0x20, 0x15} },
    .name              = "iaq_get_baseline",
};

static const struct sgp_profile SGP_PROFILE_IAQ_SET_BASELINE = {
    .number            = PROFILE_NUMBER_IAQ_SET_BASELINE,
    .duration_us       = 1000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = { .buf = {0x20, 0x1e} },
    .name              = "iaq_set_baseline",
};

static const struct sgp_profile SGP_PROFILE_MEASURE_SIGNALS = {
    .number            = PROFILE_NUMBER_SIGNALS,
    .duration_us       = 50000,
    .signals           = SGP_PROFILE_MEASURE_SIGNALS_SIGNALS,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_MEASURE_SIGNALS_SIGNALS),
    .command           = { .buf = {0x20, 0x4d} },
    .name              = "measure_signals_2s",
};

static const struct sgp_profile SGP_PROFILE_MEASURE_RAW = {
    .number            = PROFILE_MEASURE_RAW,
    .duration_us       = 50000,
    .signals           = SGP_PROFILE_MEASURE_RAW_SIGNALS,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_MEASURE_RAW_SIGNALS),
    .command           = { .buf = {0x20, 0x46} },
    .name              = "measure_raw_2s",
};


static const struct sgp_profile *sgp_profiles[] = {
    &SGP_PROFILE_IAQ_INIT0,
    &SGP_PROFILE_IAQ_INIT16,
    &SGP_PROFILE_IAQ_INIT64,
    &SGP_PROFILE_IAQ_INIT184,
    &SGP_PROFILE_IAQ_MEASURE,
    &SGP_PROFILE_IAQ_GET_BASELINE,
    &SGP_PROFILE_IAQ_SET_BASELINE,
    &SGP_PROFILE_MEASURE_SIGNALS,
    &SGP_PROFILE_MEASURE_RAW,
};

static const u16 supported_featureset_versions[] = { 0x1004 };

const struct sgp_otp_featureset sgp_featureset = {
    .profiles                                = sgp_profiles,
    .number_of_profiles                      = ARRAY_SIZE(sgp_profiles),
    .supported_featureset_versions           = (u16 *) supported_featureset_versions,
    .number_of_supported_featureset_versions = ARRAY_SIZE(supported_featureset_versions)
};

/**
 * Supported featuresets
 * List featuresets in descending order (new -> old) to use the most recent
 * compatible featureset config, as multiple ones may apply for the same major
 * version.
 */
const struct sgp_otp_featureset *featuresets[] = {
    &sgp_featureset,
};

const struct sgp_otp_supported_featuresets sgp_supported_featuresets = {
    .featuresets                     = featuresets,
    .number_of_supported_featuresets = ARRAY_SIZE(featuresets)
};

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
#include "sensirion_common.h"
#include "sgp_featureset.h"

#define PROFILE_NUMBER_RAW_SIGNALS 10
#define PROFILE_NUMBER_SET_AH 12
#define PROFILE_IAQ_SET_TVOC_BASELINE 14

const uint8_t PROFILE_NUMBER_IAQ_SET_TVOC_BASELINE =
    PROFILE_IAQ_SET_TVOC_BASELINE;
const uint8_t PROFILE_NUMBER_MEASURE_RAW_SIGNALS = PROFILE_NUMBER_RAW_SIGNALS;
const uint8_t PROFILE_NUMBER_SET_ABSOLUTE_HUMIDITY = PROFILE_NUMBER_SET_AH;

static const struct sgp_signal ETHANOL_SIGNAL_FS9 = {
    .conversion_function = NULL,
    .name = "ethanol_signal",
};

static const struct sgp_signal H2_SIGNAL_FS9 = {
    .conversion_function = NULL,
    .name = "h2_signal",
};

static const struct sgp_signal TVOC_PPB_FS9 = {
    .conversion_function = NULL,
    .name = "tVOC",
};

static const struct sgp_signal CO2_EQ_PPM = {
    .conversion_function = NULL,
    .name = "co2_eq",
};

static const struct sgp_signal BASELINE_WORD1 = {
    .conversion_function = NULL,
    .name = "baseline1",
};

static const struct sgp_signal BASELINE_WORD2 = {
    .conversion_function = NULL,
    .name = "baseline2",
};

static const struct sgp_signal *SGP_PROFILE_IAQ_MEASURE_SIGNALS9[] = {
    &TVOC_PPB_FS9, &CO2_EQ_PPM};

static const struct sgp_signal
    *SGP_PROFILE_IAQ_GET_TVOC_INCEPTIVE_BASELINE_SIGNALS[] = {&BASELINE_WORD1};

static const struct sgp_signal *SGP_PROFILE_IAQ_GET_BASELINE_SIGNALS[] = {
    &BASELINE_WORD1, &BASELINE_WORD2};

static const struct sgp_signal *SGP_PROFILE_MEASURE_SIGNALS_SIGNALS9[] = {
    &ETHANOL_SIGNAL_FS9, &H2_SIGNAL_FS9};

static const struct sgp_profile SGP_PROFILE_IAQ_INIT = {
    .number = PROFILE_NUMBER_IAQ_INIT,
    .duration_us = 10000,
    .signals = NULL,
    .number_of_signals = 0,
    .command = 0x2003,
    .name = "iaq_init",
};

static const struct sgp_profile SGP_PROFILE_IAQ_MEASURE9 = {
    .number = PROFILE_NUMBER_IAQ_MEASURE,
    .duration_us = 50000,
    .signals = SGP_PROFILE_IAQ_MEASURE_SIGNALS9,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_IAQ_MEASURE_SIGNALS9),
    .command = 0x2008,
    .name = "iaq_measure",
};

static const struct sgp_profile SGP_PROFILE_IAQ_GET_TVOC_INCEPTIVE_BASELINE = {
    .number = PROFILE_NUMBER_IAQ_GET_TVOC_INCEPTIVE_BASELINE,
    .duration_us = 10000,
    .signals = SGP_PROFILE_IAQ_GET_TVOC_INCEPTIVE_BASELINE_SIGNALS,
    .number_of_signals =
        ARRAY_SIZE(SGP_PROFILE_IAQ_GET_TVOC_INCEPTIVE_BASELINE_SIGNALS),
    .command = 0x20b3,
    .name = "iaq_get_tvoc_inceptive_baseline",
};

static const struct sgp_profile SGP_PROFILE_IAQ_SET_TVOC_BASELINE = {
    .number = PROFILE_IAQ_SET_TVOC_BASELINE,
    .duration_us = 10000,
    .signals = NULL,
    .number_of_signals = 0,
    .command = 0x2077,
    .name = "iaq_set_tvoc_baseline",
};

static const struct sgp_profile SGP_PROFILE_IAQ_GET_BASELINE = {
    .number = PROFILE_NUMBER_IAQ_GET_BASELINE,
    .duration_us = 10000,
    .signals = SGP_PROFILE_IAQ_GET_BASELINE_SIGNALS,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_IAQ_GET_BASELINE_SIGNALS),
    .command = 0x2015,
    .name = "iaq_get_baseline",
};

static const struct sgp_profile SGP_PROFILE_IAQ_SET_BASELINE = {
    .number = PROFILE_NUMBER_IAQ_SET_BASELINE,
    .duration_us = 10000,
    .signals = NULL,
    .number_of_signals = 0,
    .command = 0x201e,
    .name = "iaq_set_baseline",
};

static const struct sgp_profile SGP_PROFILE_MEASURE_SIGNALS9 = {
    .number = PROFILE_NUMBER_RAW_SIGNALS,
    .duration_us = 200000,
    .signals = SGP_PROFILE_MEASURE_SIGNALS_SIGNALS9,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_MEASURE_SIGNALS_SIGNALS9),
    .command = 0x2050,
    .name = "measure_signals",
};

static const struct sgp_profile SGP_PROFILE_SET_ABSOLUTE_HUMIDITY = {
    .number = PROFILE_NUMBER_SET_AH,
    .duration_us = 10000,
    .signals = NULL,
    .number_of_signals = 0,
    .command = 0x2061,
    .name = "set_absolute_humidity",
};

static const struct sgp_profile *sgp_profiles9[] = {
    &SGP_PROFILE_IAQ_INIT,         &SGP_PROFILE_IAQ_MEASURE9,
    &SGP_PROFILE_IAQ_GET_BASELINE, &SGP_PROFILE_IAQ_SET_BASELINE,
    &SGP_PROFILE_MEASURE_SIGNALS9,
};

static const struct sgp_profile *sgp_profiles32[] = {
    &SGP_PROFILE_IAQ_INIT,         &SGP_PROFILE_IAQ_MEASURE9,
    &SGP_PROFILE_IAQ_GET_BASELINE, &SGP_PROFILE_IAQ_SET_BASELINE,
    &SGP_PROFILE_MEASURE_SIGNALS9, &SGP_PROFILE_SET_ABSOLUTE_HUMIDITY,
};

static const struct sgp_profile *sgp_profiles33[] = {
    &SGP_PROFILE_IAQ_INIT,
    &SGP_PROFILE_IAQ_MEASURE9,
    &SGP_PROFILE_IAQ_GET_BASELINE,
    &SGP_PROFILE_IAQ_SET_BASELINE,
    &SGP_PROFILE_IAQ_GET_TVOC_INCEPTIVE_BASELINE,
    &SGP_PROFILE_IAQ_SET_TVOC_BASELINE,
    &SGP_PROFILE_MEASURE_SIGNALS9,
    &SGP_PROFILE_SET_ABSOLUTE_HUMIDITY,
};

static const uint16_t supported_featureset_versions_fs9[] = {9};
static const uint16_t supported_featureset_versions_fs32[] = {0x20};
static const uint16_t supported_featureset_versions_fs33[] = {0x21};

static const struct sgp_otp_featureset sgp_featureset9 = {
    .profiles = sgp_profiles9,
    .number_of_profiles = ARRAY_SIZE(sgp_profiles9),
    .supported_featureset_versions =
        (uint16_t *)supported_featureset_versions_fs9,
    .number_of_supported_featureset_versions =
        ARRAY_SIZE(supported_featureset_versions_fs9)};

static const struct sgp_otp_featureset sgp_featureset32 = {
    .profiles = sgp_profiles32,
    .number_of_profiles = ARRAY_SIZE(sgp_profiles32),
    .supported_featureset_versions =
        (uint16_t *)supported_featureset_versions_fs32,
    .number_of_supported_featureset_versions =
        ARRAY_SIZE(supported_featureset_versions_fs32)};

static const struct sgp_otp_featureset sgp_featureset33 = {
    .profiles = sgp_profiles33,
    .number_of_profiles = ARRAY_SIZE(sgp_profiles33),
    .supported_featureset_versions =
        (uint16_t *)supported_featureset_versions_fs33,
    .number_of_supported_featureset_versions =
        ARRAY_SIZE(supported_featureset_versions_fs33)};

/**
 * Supported featuresets
 * List featuresets in descending order (new -> old) to use the most recent
 * compatible featureset config, as multiple ones may apply for the same major
 * version.
 */
static const struct sgp_otp_featureset *featuresets[] = {
    &sgp_featureset33,
    &sgp_featureset32,
    &sgp_featureset9,
};

const struct sgp_otp_supported_featuresets sgp_supported_featuresets = {
    .featuresets = featuresets,
    .number_of_supported_featuresets = ARRAY_SIZE(featuresets)};

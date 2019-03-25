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
#include "sensirion_common.h"
#include "sgp_featureset.h"

#define PROFILE_NUMBER_RAW_SIGNALS 9
#define PROFILE_IAQ_MEASURE_RAW 8
#define PROFILE_SET_ABSOLUTE_HUMIDITY 12
#define PROFILE_SET_POWER_MODE 19
#define PROFILE_IAQ_INIT0 17
#define PROFILE_IAQ_INIT16 4
#define PROFILE_IAQ_INIT64 0
#define PROFILE_IAQ_INIT184 13
#define PROFILE_IAQ_INIT_CONTINUOUS 21

const u8 PROFILE_NUMBER_MEASURE_RAW_SIGNALS = PROFILE_NUMBER_RAW_SIGNALS;
const u8 PROFILE_NUMBER_MEASURE_RAW = PROFILE_IAQ_MEASURE_RAW;
const u8 PROFILE_NUMBER_IAQ_INIT0 = PROFILE_IAQ_INIT0;
const u8 PROFILE_NUMBER_IAQ_INIT16 = PROFILE_IAQ_INIT16;
const u8 PROFILE_NUMBER_IAQ_INIT64 = PROFILE_IAQ_INIT64;
const u8 PROFILE_NUMBER_IAQ_INIT184 = PROFILE_IAQ_INIT184;
const u8 PROFILE_NUMBER_IAQ_INIT_CONTINUOUS = PROFILE_IAQ_INIT_CONTINUOUS;
const u8 PROFILE_NUMBER_IAQ_SET_TVOC_BASELINE = PROFILE_NUMBER_IAQ_SET_BASELINE;
const u8 PROFILE_NUMBER_SET_ABSOLUTE_HUMIDITY = PROFILE_SET_ABSOLUTE_HUMIDITY;
const u8 PROFILE_NUMBER_SET_POWER_MODE = PROFILE_SET_POWER_MODE;

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

static const struct sgp_signal *SGP_PROFILE_MEASURE_ETOH_SIGNAL_SIGNALS[] =
    { &ETHANOL_SIGNAL };

static const struct sgp_signal *SGP_PROFILE_IAQ_MEASURE_RAW_SIGNALS[] =
    { &TVOC_PPB, &ETHANOL_SIGNAL };


static const struct sgp_profile SGP_PROFILE_IAQ_INIT64 = {
    .number            = PROFILE_IAQ_INIT64,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = 0x2003,
    .name              = "iaq_init64",
};

static const struct sgp_profile SGP_PROFILE_IAQ_INIT0 = {
    .number            = PROFILE_IAQ_INIT0,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = 0x2089,
    .name              = "iaq_init0",
};

static const struct sgp_profile SGP_PROFILE_IAQ_INIT16 = {
    .number            = PROFILE_IAQ_INIT16,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = 0x2024,
    .name              = "iaq_init16",
};

static const struct sgp_profile SGP_PROFILE_IAQ_INIT184 = {
    .number            = PROFILE_IAQ_INIT184,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = 0x206a,
    .name              = "iaq_init184",
};

static const struct sgp_profile SGP_PROFILE_IAQ_INIT_CONTINUOUS = {
    .number            = PROFILE_IAQ_INIT_CONTINUOUS,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = 0x20ae,
    .name              = "iaq_init_continuous",
};

static const struct sgp_profile SGP_PROFILE_IAQ_MEASURE = {
    .number            = PROFILE_NUMBER_IAQ_MEASURE,
    .duration_us       = 50000,
    .signals           = SGP_PROFILE_IAQ_MEASURE_SIGNALS,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_IAQ_MEASURE_SIGNALS),
    .command           = 0x2008,
    .name              = "iaq_measure",
};

static const struct sgp_profile SGP_PROFILE_IAQ_GET_BASELINE = {
    .number            = PROFILE_NUMBER_IAQ_GET_BASELINE,
    .duration_us       = 10000,
    .signals           = SGP_PROFILE_IAQ_GET_BASELINE_SIGNALS,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_IAQ_GET_BASELINE_SIGNALS),
    .command           = 0x2015,
    .name              = "iaq_get_baseline",
};

static const struct sgp_profile SGP_PROFILE_IAQ_SET_BASELINE = {
    .number            = PROFILE_NUMBER_IAQ_SET_BASELINE,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = 0x201e,
    .name              = "iaq_set_baseline",
};

static const struct sgp_profile SGP_PROFILE_IAQ_GET_TVOC_INCEPTIVE_BASELINE = {
    .number            = PROFILE_NUMBER_IAQ_GET_TVOC_INCEPTIVE_BASELINE,
    .duration_us       = 10000,
    .signals           = SGP_PROFILE_IAQ_GET_BASELINE_SIGNALS,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_IAQ_GET_BASELINE_SIGNALS),
    .command           = 0x20b3,
    .name              = "iaq_get_tvoc_inceptive_baseline",
};

static const struct sgp_profile SGP_PROFILE_MEASURE_ETOH_SIGNAL = {
    .number            = PROFILE_NUMBER_RAW_SIGNALS,
    .duration_us       = 50000,
    .signals           = SGP_PROFILE_MEASURE_ETOH_SIGNAL_SIGNALS,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_MEASURE_ETOH_SIGNAL_SIGNALS),
    .command           = 0x204d,
    .name              = "measure_etoh_signal",
};

static const struct sgp_profile SGP_PROFILE_IAQ_MEASURE_RAW = {
    .number            = PROFILE_IAQ_MEASURE_RAW,
    .duration_us       = 50000,
    .signals           = SGP_PROFILE_IAQ_MEASURE_RAW_SIGNALS,
    .number_of_signals = ARRAY_SIZE(SGP_PROFILE_IAQ_MEASURE_RAW_SIGNALS),
    .command           = 0x2046,
    .name              = "iaq_measure_raw",
};

static const struct sgp_profile SGP_PROFILE_SET_ABSOLUTE_HUMIDITY = {
    .number            = PROFILE_SET_ABSOLUTE_HUMIDITY,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = 0x2061,
    .name              = "set_absolute_humidity",
};

static const struct sgp_profile SGP_PROFILE_SET_POWER_MODE = {
    .number            = PROFILE_SET_POWER_MODE,
    .duration_us       = 10000,
    .signals           = NULL,
    .number_of_signals = 0,
    .command           = 0x209f,
    .name              = "set_power_mode",
};


static const struct sgp_profile *sgp_profiles_fs4[] = {
    &SGP_PROFILE_MEASURE_ETOH_SIGNAL,
    &SGP_PROFILE_IAQ_INIT0,
    &SGP_PROFILE_IAQ_INIT16,
    &SGP_PROFILE_IAQ_INIT64,
    &SGP_PROFILE_IAQ_INIT184,
    &SGP_PROFILE_IAQ_MEASURE,
    &SGP_PROFILE_IAQ_GET_BASELINE,
    &SGP_PROFILE_IAQ_SET_BASELINE,
    &SGP_PROFILE_IAQ_MEASURE_RAW,
};

static const struct sgp_profile *sgp_profiles_fs5[] = {
    &SGP_PROFILE_MEASURE_ETOH_SIGNAL,
    &SGP_PROFILE_IAQ_INIT0,
    &SGP_PROFILE_IAQ_INIT16,
    &SGP_PROFILE_IAQ_INIT64,
    &SGP_PROFILE_IAQ_INIT184,
    &SGP_PROFILE_IAQ_MEASURE,
    &SGP_PROFILE_IAQ_GET_BASELINE,
    &SGP_PROFILE_IAQ_SET_BASELINE,
    &SGP_PROFILE_IAQ_GET_TVOC_INCEPTIVE_BASELINE,
    &SGP_PROFILE_IAQ_MEASURE_RAW,
};

static const struct sgp_profile *sgp_profiles_fs6[] = {
    &SGP_PROFILE_MEASURE_ETOH_SIGNAL,
    &SGP_PROFILE_IAQ_INIT0,
    &SGP_PROFILE_IAQ_INIT16,
    &SGP_PROFILE_IAQ_INIT64,
    &SGP_PROFILE_IAQ_INIT184,
    &SGP_PROFILE_IAQ_INIT_CONTINUOUS,
    &SGP_PROFILE_IAQ_MEASURE,
    &SGP_PROFILE_IAQ_GET_BASELINE,
    &SGP_PROFILE_IAQ_SET_BASELINE,
    &SGP_PROFILE_IAQ_GET_TVOC_INCEPTIVE_BASELINE,
    &SGP_PROFILE_IAQ_MEASURE_RAW,
    &SGP_PROFILE_SET_ABSOLUTE_HUMIDITY,
    &SGP_PROFILE_SET_POWER_MODE,
};

static const u16 supported_featureset_versions_fs4[] = { 0x1004 };
static const u16 supported_featureset_versions_fs5[] = { 0x1005 };
static const u16 supported_featureset_versions_fs6[] = { 0x1006 };

const struct sgp_otp_featureset sgp_featureset4 = {
    .profiles                                = sgp_profiles_fs4,
    .number_of_profiles                      = ARRAY_SIZE(sgp_profiles_fs4),
    .supported_featureset_versions           = (u16 *) supported_featureset_versions_fs4,
    .number_of_supported_featureset_versions = ARRAY_SIZE(supported_featureset_versions_fs4)
};

const struct sgp_otp_featureset sgp_featureset5 = {
    .profiles                                = sgp_profiles_fs5,
    .number_of_profiles                      = ARRAY_SIZE(sgp_profiles_fs5),
    .supported_featureset_versions           = (u16 *) supported_featureset_versions_fs5,
    .number_of_supported_featureset_versions = ARRAY_SIZE(supported_featureset_versions_fs5)
};

const struct sgp_otp_featureset sgp_featureset6 = {
    .profiles                                = sgp_profiles_fs6,
    .number_of_profiles                      = ARRAY_SIZE(sgp_profiles_fs6),
    .supported_featureset_versions           = (u16 *) supported_featureset_versions_fs6,
    .number_of_supported_featureset_versions = ARRAY_SIZE(supported_featureset_versions_fs6)
};

/**
 * Supported featuresets
 * List featuresets in descending order (new -> old) to use the most recent
 * compatible featureset config, as multiple ones may apply for the same major
 * version.
 */
const struct sgp_otp_featureset *featuresets[] = {
    &sgp_featureset6,
    &sgp_featureset5,
    &sgp_featureset4,
};

const struct sgp_otp_supported_featuresets sgp_supported_featuresets = {
    .featuresets                     = featuresets,
    .number_of_supported_featuresets = ARRAY_SIZE(featuresets)
};

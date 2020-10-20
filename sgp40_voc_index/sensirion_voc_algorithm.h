/*
 * Copyright (c) 2020, Sensirion AG
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

#ifndef VOCALGORITHM_H_
#define VOCALGORITHM_H_



#include <stdint.h>

typedef int32_t fix16_t;

#define F16(x) ((fix16_t)(((x) >= 0) ? ((x) * 65536.0 + 0.5) : ((x) * 65536.0 - 0.5)))


#ifndef __cplusplus

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else

#ifndef bool
#define bool int
#define true 1
#define false 0
#endif // bool

#endif // __STDC_VERSION__

#endif // __cplusplus

// Should be set by the building toolchain
#ifndef LIBRARY_VERSION_NAME
#define LIBRARY_VERSION_NAME "custom build"
#endif

#define VocAlgorithm_SAMPLING_INTERVAL (1.)
#define VocAlgorithm_INITIAL_BLACKOUT (45.)
#define VocAlgorithm_VOC_INDEX_GAIN (230.)
#define VocAlgorithm_SRAW_STD_INITIAL (50.)
#define VocAlgorithm_SRAW_STD_BONUS (220.)
#define VocAlgorithm_TAU_MEAN_VARIANCE_HOURS (12.)
#define VocAlgorithm_TAU_INITIAL_MEAN (20.)
#define VocAlgorithm_INIT_DURATION_MEAN ((3600. * 0.75))
#define VocAlgorithm_INIT_TRANSITION_MEAN (0.01)
#define VocAlgorithm_TAU_INITIAL_VARIANCE (2500.)
#define VocAlgorithm_INIT_DURATION_VARIANCE ((3600. * 1.45))
#define VocAlgorithm_INIT_TRANSITION_VARIANCE (0.01)
#define VocAlgorithm_GATING_THRESHOLD (340.)
#define VocAlgorithm_GATING_THRESHOLD_INITIAL (510.)
#define VocAlgorithm_GATING_THRESHOLD_TRANSITION (0.09)
#define VocAlgorithm_GATING_MAX_DURATION_MINUTES ((60. * 3.))
#define VocAlgorithm_GATING_MAX_RATIO (0.3)
#define VocAlgorithm_SIGMOID_L (500.)
#define VocAlgorithm_SIGMOID_K (-0.0065)
#define VocAlgorithm_SIGMOID_X0 (213.)
#define VocAlgorithm_VOC_INDEX_OFFSET_DEFAULT (100.)
#define VocAlgorithm_LP_TAU_FAST (20.0)
#define VocAlgorithm_LP_TAU_SLOW (500.0)
#define VocAlgorithm_LP_ALPHA (-0.2)
#define VocAlgorithm_PERSISTENCE_UPTIME_GAMMA ((3. * 3600.))
#define VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__GAMMA_SCALING (64.)
#define VocAlgorithm_MEAN_VARIANCE_ESTIMATOR__FIX16_MAX (32767.)


typedef struct {
    fix16_t mVoc_Index_Offset;
    fix16_t mTau_Mean_Variance_Hours;
    fix16_t mGating_Max_Duration_Minutes;
    fix16_t mSraw_Std_Initial;
    fix16_t mUptime;
    fix16_t mSraw;
    fix16_t mVoc_Index;
    fix16_t m_Mean_Variance_Estimator__Gating_Max_Duration_Minutes;
    bool m_Mean_Variance_Estimator___Initialized;
    fix16_t m_Mean_Variance_Estimator___Mean;
    fix16_t m_Mean_Variance_Estimator___Sraw_Offset;
    fix16_t m_Mean_Variance_Estimator___Std;
    fix16_t m_Mean_Variance_Estimator___Gamma;
    fix16_t m_Mean_Variance_Estimator___Gamma_Initial_Mean;
    fix16_t m_Mean_Variance_Estimator___Gamma_Initial_Variance;
    fix16_t m_Mean_Variance_Estimator__Gamma_Mean;
    fix16_t m_Mean_Variance_Estimator__Gamma_Variance;
    fix16_t m_Mean_Variance_Estimator___Uptime_Gamma;
    fix16_t m_Mean_Variance_Estimator___Uptime_Gating;
    fix16_t m_Mean_Variance_Estimator___Gating_Duration_Minutes;
    fix16_t m_Mean_Variance_Estimator___Sigmoid__L;
    fix16_t m_Mean_Variance_Estimator___Sigmoid__K;
    fix16_t m_Mean_Variance_Estimator___Sigmoid__X0;
    fix16_t m_Mox_Model__Sraw_Std;
    fix16_t m_Mox_Model__Sraw_Mean;
    fix16_t m_Sigmoid_Scaled__Offset;
    fix16_t m_Adaptive_Lowpass__A1;
    fix16_t m_Adaptive_Lowpass__A2;
    bool m_Adaptive_Lowpass___Initialized;
    fix16_t m_Adaptive_Lowpass___X1;
    fix16_t m_Adaptive_Lowpass___X2;
    fix16_t m_Adaptive_Lowpass___X3;
} VocAlgorithmParams;

void VocAlgorithm_init(VocAlgorithmParams *params);
void VocAlgorithm_get_states(VocAlgorithmParams *params, int32_t *state0, int32_t *state1);
void VocAlgorithm_set_states(VocAlgorithmParams *params, int32_t state0, int32_t state1);
void VocAlgorithm_set_tuning_parameters(VocAlgorithmParams *params, int32_t voc_index_offset, int32_t learning_time_hours, int32_t gating_max_duration_minutes, int32_t std_initial);
void VocAlgorithm_process(VocAlgorithmParams *params, int32_t sraw, int32_t *voc_index);

#endif /* VOCALGORITHM_H_ */

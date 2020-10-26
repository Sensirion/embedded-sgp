#include "sensirion_test_setup.h"
#include "sgp40_voc_index.h"
#include <inttypes.h>
#include <stdio.h>

#define MIN_VALUE_ETHANOL 0
#define MAX_VALUE_ETHANOL 60000
#define MIN_VALUE_TEMPERATURE 0
#define MAX_VALUE_TEMPERATURE 100000
#define MIN_VALUE_HUMIDITY 0
#define MAX_VALUE_HUMIDITY 100000

TEST_GROUP (SGP40_VOC_INDEX_Tests) {
    void setup() {
        int16_t ret;
        sensirion_i2c_init();

        // Select MUX 1 channel 0 (SGP40 OTP15-eng, SHTC3) on ch-sta-p-rp0002
        ret = sensirion_i2c_mux_set_single_channel(0x72, 0);
        CHECK_ZERO_TEXT(ret, "sensirion_i2c_mux_set_single_channel");
    }

    void teardown() {
        int16_t ret = sensirion_i2c_general_call_reset();
        CHECK_ZERO_TEXT(ret, "sensirion_i2c_general_call_reset");
        sensirion_i2c_release();
    }
};

TEST (SGP40_VOC_INDEX_Tests, SGP40_Engine_Test) {
    int32_t voc_index;
    int32_t rh, t;
    int16_t ret;

    ret = sensirion_init_sensors();
    CHECK_ZERO_TEXT(ret, "sensirion_init_sensors");

    ret = sensirion_measure_voc_index(&voc_index);
    CHECK_ZERO_TEXT(ret, "sensirion_measure_voc_index");
    printf("VOC Index: %f\n", voc_index * 0.001f);

    ret = sensirion_measure_voc_index_with_rh_t(&voc_index, &rh, &t);
    printf("VOC Index: %i, RH: %f, T: %f\n", voc_index, rh * 0.001f,
           t * 0.001f);
    CHECK_ZERO_TEXT(ret, "sensirion_measure_voc_index_with_rh_t");
    CHECK_TRUE_TEXT(rh >= MIN_VALUE_HUMIDITY && rh <= MAX_VALUE_HUMIDITY,
                    "sgp40_measure_voc_index_with_rh humidity");
    CHECK_TRUE_TEXT(t >= MIN_VALUE_TEMPERATURE && t <= MAX_VALUE_TEMPERATURE,
                    "sgp40_measure_voc_index_with_rh temperature");
}

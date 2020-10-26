#include "sensirion_test_setup.h"
#include "svm30.h"
#include <stdio.h>

#define MIN_VALUE_TVOC 0
#define MAX_VALUE_TVOC 60000
#define MIN_VALUE_CO2 0
#define MAX_VALUE_CO2 60000
#define MIN_VALUE_ETHANOL 0
#define MAX_VALUE_ETHANOL 60000
#define MIN_VALUE_H2 0
#define MAX_VALUE_H2 60000
#define MIN_VALUE_TEMPERATURE 15000
#define MAX_VALUE_TEMPERATURE 35000
#define MIN_VALUE_HUMIDITY 20000
#define MAX_VALUE_HUMIDITY 80000

TEST_GROUP (SVM30_Tests) {
    void setup() {
        int16_t ret;
        sensirion_i2c_init();

        // Select channels 2 (SGP30 FS1.2) and 6 (SHTC1) to model an SVM30
        ret = sensirion_i2c_mux_set_single_channel(0x71, 2);
        CHECK_ZERO_TEXT(ret, "sensirion_i2c_mux_set_single_channel");

        ret = sensirion_i2c_mux_add_channel(0x71, 6);
        CHECK_ZERO_TEXT(ret, "sensirion_i2c_mux_add_channel");
    }

    void teardown() {
        int16_t ret = sensirion_i2c_general_call_reset();
        CHECK_ZERO_TEXT(ret, "sensirion_i2c_general_call_reset");
        sensirion_i2c_release();
    }
};

TEST (SVM30_Tests, SVM30_Test) {
    int16_t ret;
    uint16_t signal1, signal2;
    int32_t temp, humi;

    const char* version = svm_get_driver_version();
    printf("SVM Driver version: %s\n", version);

    ret = svm_probe();
    CHECK_ZERO_TEXT(ret, "svm_probe");

    ret = svm_measure_iaq_blocking_read(&signal1, &signal2, &temp, &humi);
    printf("Measured the following IAQ data:\n\
            \tTVOC: %u\n\
            \tCO2:  %u\n\
            \tTemp: %i\n\
            \tHumi: %i\n",
           signal1, signal2, temp, humi);
    CHECK_ZERO_TEXT(ret, "svm_measure_iaq_blocking_read");
    CHECK_TRUE_TEXT(signal1 >= MIN_VALUE_TVOC && signal1 <= MAX_VALUE_TVOC,
                    "svm_measure_iaq_blocking_read TVOC");
    CHECK_TRUE_TEXT(signal2 >= MIN_VALUE_CO2 && signal2 <= MAX_VALUE_CO2,
                    "svm_measure_iaq_blocking_read CO2");
    CHECK_TRUE_TEXT(temp >= MIN_VALUE_TEMPERATURE &&
                        temp <= MAX_VALUE_TEMPERATURE,
                    "svm_measure_iaq_blocking_read Temperature");
    CHECK_TRUE_TEXT(humi >= MIN_VALUE_HUMIDITY && humi <= MAX_VALUE_HUMIDITY,
                    "svm_measure_iaq_blocking_read Humidity");

    ret = svm_measure_raw_blocking_read(&signal1, &signal2, &temp, &humi);
    printf("Measured the following raw data:\n\
            \tEthanol: %u\n\
            \tH2:      %u\n\
            \tTemp:    %i\n\
            \tHumi:    %i\n",
           signal1, signal2, temp, humi);
    CHECK_ZERO_TEXT(ret, "svm_measure_raw_blocking_read");
    CHECK_TRUE_TEXT(signal1 >= MIN_VALUE_ETHANOL &&
                        signal1 <= MAX_VALUE_ETHANOL,
                    "svm_measure_raw_blocking_read Ethanol");
    CHECK_TRUE_TEXT(signal2 >= MIN_VALUE_H2 && signal2 <= MAX_VALUE_H2,
                    "svm_measure_raw_blocking_read H2");
    CHECK_TRUE_TEXT(temp >= MIN_VALUE_TEMPERATURE &&
                        temp <= MAX_VALUE_TEMPERATURE,
                    "svm_measure_raw_blocking_read Temperature");
    CHECK_TRUE_TEXT(humi >= MIN_VALUE_HUMIDITY && humi <= MAX_VALUE_HUMIDITY,
                    "svm_measure_raw_blocking_read Humidity");
}

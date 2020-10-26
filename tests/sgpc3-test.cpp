#include "sensirion_test_setup.h"
#include "sgpc3.h"
#include <inttypes.h>

#define SGPC3_PRODUCT_TYPE 1
#define SGPC3_MEASURE_TEST_RESULT 0xd400
#define MAX_VALUE_TVOC 60000
#define MIN_VALUE_ETHANOL 1000
#define MAX_VALUE_ETHANOL 60000
#define MAX_VALUE_HUMIDITY 256000
#define MEASUREMENT_TIME_USEC 50000
#define SGPC3_FS4_TVOC_BASELINE 8000

static void sgpc3_run_tests(uint16_t fs) {
    int16_t ret;
    uint16_t test_result;
    uint16_t tvoc_baseline_set;
    uint16_t tvoc_baseline_get;
    uint16_t tvoc_ppb;
    uint16_t ethanol;

    // Set baseline to inceptive baseline. Then get it and check if
    // it is indeed set to the inceptive baseline
    ret = sgpc3_get_tvoc_inceptive_baseline(&tvoc_baseline_set);
    if (fs >= 5) {
        CHECK_ZERO_TEXT(ret, "sgpc3_get_tvoc_inceptive_baseline");
    } else {
        CHECK_EQUAL_TEXT(SGPC3_ERR_UNSUPPORTED_FEATURE_SET, ret,
                         "sgpc3_get_tvoc_inceptive_baseline unsupported FS");
        tvoc_baseline_set = SGPC3_FS4_TVOC_BASELINE;
    }

    ret = sgpc3_set_tvoc_baseline(tvoc_baseline_set);
    CHECK_ZERO_TEXT(ret, "sgpc3_set_tvoc_baseline");
    ret = sgpc3_get_tvoc_baseline(&tvoc_baseline_get);
    CHECK_ZERO_TEXT(ret, "sgpc3_get_tvoc_baseline");
    CHECK_EQUAL_TEXT(tvoc_baseline_set, tvoc_baseline_get,
                     "sgpc3_get_tvoc_baseline mismatch");

    // check sensor for defects
    ret = sgpc3_measure_test(&test_result);
    CHECK_ZERO_TEXT(ret, "sgpc3_measure_test");
    CHECK_EQUAL_TEXT(SGPC3_MEASURE_TEST_RESULT, test_result,
                     "sgpc3_measure_test result");

    // tvoc measurements
    ret = sgpc3_measure_tvoc_blocking_read(&tvoc_ppb);
    CHECK_ZERO_TEXT(ret, "sgpc3_measure_tvoc_blocking_read");
    CHECK_TRUE_TEXT(tvoc_ppb <= MAX_VALUE_TVOC,
                    "sgpc3_measure_tvoc_blocking_read tvoc");

    ret = sgpc3_measure_tvoc();
    CHECK_ZERO_TEXT(ret, "sgpc3_measure_tvoc");
    sensirion_sleep_usec(MEASUREMENT_TIME_USEC);
    ret = sgpc3_read_tvoc(&tvoc_ppb);
    CHECK_ZERO_TEXT(ret, "sgpc3_read_tvoc");
    CHECK_TRUE_TEXT(tvoc_ppb <= MAX_VALUE_TVOC, "sgpc3_read_tvoc tvoc");

    // ethanol measurements
    ret = sgpc3_measure_raw_blocking_read(&ethanol);
    CHECK_ZERO_TEXT(ret, "sgpc3_measure_raw_blocking_read");
    CHECK_TRUE_TEXT(ethanol >= MIN_VALUE_ETHANOL &&
                        ethanol <= MAX_VALUE_ETHANOL,
                    "sgpc3_measure_raw_blocking_read ethanol");

    ret = sgpc3_measure_raw();
    CHECK_ZERO_TEXT(ret, "sgpc3_measure_raw");
    sensirion_sleep_usec(MEASUREMENT_TIME_USEC);
    ret = sgpc3_read_raw(&ethanol);
    CHECK_ZERO_TEXT(ret, "sgpc3_read_raw");
    CHECK_TRUE_TEXT(ethanol >= MIN_VALUE_ETHANOL &&
                        ethanol <= MAX_VALUE_ETHANOL,
                    "sgpc3_read_raw ethanol");

    // tvoc and ethanol measurements
    ret = sgpc3_measure_tvoc_and_raw_blocking_read(&tvoc_ppb, &ethanol);
    CHECK_ZERO_TEXT(ret, "sgpc3_measure_tvoc_and_raw_blocking_read");
    CHECK_TRUE_TEXT(tvoc_ppb <= MAX_VALUE_TVOC,
                    "sgpc3_measure_tvoc_and_raw_blocking_read tvoc");
    CHECK_TRUE_TEXT(ethanol >= MIN_VALUE_ETHANOL &&
                        ethanol <= MAX_VALUE_ETHANOL,
                    "sgpc3_measure_tvoc_and_raw_blocking_read ethanol");

    ret = sgpc3_measure_tvoc_and_raw();
    CHECK_ZERO_TEXT(ret, "sgpc3_measure_tvoc_and_raw");
    sensirion_sleep_usec(MEASUREMENT_TIME_USEC);
    ret = sgpc3_read_tvoc_and_raw(&tvoc_ppb, &ethanol);
    CHECK_ZERO_TEXT(ret, "sgpc3_read_tvoc_and_raw");
    CHECK_TRUE_TEXT(tvoc_ppb <= MAX_VALUE_TVOC, "sgpc3_read_tvoc_and_raw tvoc");
    CHECK_TRUE_TEXT(ethanol >= MIN_VALUE_ETHANOL &&
                        ethanol <= MAX_VALUE_ETHANOL,
                    "sgpc3_read_tvoc_and_raw ethanol");
}

static void sgpc3_test_all_humi(uint16_t fs) {
    int16_t ret;

    // Humidity compensation is only available from FS 6 onwards
    if (fs >= 6) {
        ret = sgpc3_set_absolute_humidity(0);
        CHECK_ZERO_TEXT(ret, "sgpc3_set_absolute_humidity minimum");
        sgpc3_run_tests(fs);

        ret = sgpc3_set_absolute_humidity(MAX_VALUE_HUMIDITY);
        CHECK_ZERO_TEXT(ret, "sgpc3_set_absolute_humidity maximum");
        sgpc3_run_tests(fs);

        ret = sgpc3_set_absolute_humidity(MAX_VALUE_HUMIDITY + 1);
        CHECK_EQUAL_TEXT(STATUS_FAIL, ret,
                         "sgpc3_set_absolute_humidity should fail");
    } else {
        ret = sgpc3_set_absolute_humidity(0);
        CHECK_EQUAL_TEXT(SGPC3_ERR_UNSUPPORTED_FEATURE_SET, ret,
                         "sgpc3_set_absolute_humidity unsupported FS");

        sgpc3_run_tests(fs);
    }
}

static void sgpc3_test_all_inits(uint16_t expected_feature_set) {
    uint8_t product_type;
    int16_t ret;
    uint16_t fs_version;
    uint64_t serial;

    // Probe sensor, check if it is here
    ret = sgpc3_probe();
    CHECK_ZERO_TEXT(ret, "sgpc3_probe");

    const char* version = sgpc3_get_driver_version();
    printf("sgpc3_get_driver_version: %s\n", version);

    uint8_t addr = sgpc3_get_configured_address();
    CHECK_EQUAL_TEXT(0x58, addr, "sgpc3_get_configured_address");

    ret = sgpc3_get_serial_id(&serial);
    CHECK_ZERO_TEXT(ret, "sgpc3_get_serial");
    printf("SGPC3 serial: %" PRIu64 "\n", serial);

    ret = sgpc3_get_feature_set_version(&fs_version, &product_type);
    CHECK_ZERO_TEXT(ret, "sgpc3_get_feature_set_version");
    printf("FS: 0x%02x, Type: 0x%02x\n", fs_version, product_type);
    CHECK_EQUAL_TEXT(expected_feature_set, fs_version,
                     "sgpc3_get_feature_set_version feature set");
    CHECK_EQUAL_TEXT(SGPC3_PRODUCT_TYPE, product_type,
                     "sgpc3_get_feature_set_version product type");

    if (fs_version >= 6) {
        // INIT with preheat (only FS >= 6)
        ret = sgpc3_tvoc_init_preheat();
        CHECK_ZERO_TEXT(ret, "sgpc3_tvoc_init_preheat");
        sgpc3_test_all_humi(fs_version);
    } else {
        ret = sgpc3_tvoc_init_preheat();
        CHECK_EQUAL_TEXT(SGPC3_ERR_UNSUPPORTED_FEATURE_SET, ret,
                         "sgpc3_tvoc_init_preheat unsupported FS");
    }

    // INIT without preheat
    ret = sgpc3_tvoc_init_no_preheat();
    CHECK_ZERO_TEXT(ret, "sgpc3_tvoc_init_no_preheat");
    sgpc3_test_all_humi(fs_version);

    // INIT with legacy function
    ret = sgpc3_tvoc_init_64s_fs5();
    CHECK_ZERO_TEXT(ret, "sgpc3_tvoc_init_64s_fs5");
    sgpc3_test_all_humi(fs_version);
}

static void test_teardown() {
    int16_t ret = sensirion_i2c_general_call_reset();
    CHECK_ZERO_TEXT(ret, "sensirion_i2c_general_call_reset");
    sensirion_i2c_release();
}

TEST_GROUP (SGPC3_FS4_Tests) {
    void setup() {
        sensirion_i2c_init();
        int16_t ret = sensirion_i2c_mux_set_single_channel(0x71, 3);
        CHECK_ZERO_TEXT(ret, "sensirion_i2c_mux_set_single_channel(0x71, 3)");
    }

    void teardown() {
        test_teardown();
    }
};

TEST_GROUP (SGPC3_FS5_Tests) {
    void setup() {
        sensirion_i2c_init();
        int16_t ret = sensirion_i2c_mux_set_single_channel(0x71, 4);
        CHECK_ZERO_TEXT(ret, "sensirion_i2c_mux_set_single_channel(0x71, 4)");
    }

    void teardown() {
        test_teardown();
    }
};

TEST_GROUP (SGPC3_FS6_Tests) {
    void setup() {
        sensirion_i2c_init();
        int16_t ret = sensirion_i2c_mux_set_single_channel(0x71, 5);
        CHECK_ZERO_TEXT(ret, "sensirion_i2c_mux_set_single_channel(0x71, 5)");
    }

    void teardown() {
        test_teardown();
    }
};

TEST (SGPC3_FS4_Tests, SGPC3Test_FS4) {
    sgpc3_test_all_inits(4);
    int16_t ret = sgpc3_set_power_mode(1);
    CHECK_EQUAL_TEXT(SGPC3_ERR_UNSUPPORTED_FEATURE_SET, ret,
                     "sgpc3_set_power_mode unsupported FS");
}

TEST (SGPC3_FS5_Tests, SGPC3Test_FS5) {
    sgpc3_test_all_inits(5);
    int16_t ret = sgpc3_set_power_mode(1);
    CHECK_EQUAL_TEXT(SGPC3_ERR_UNSUPPORTED_FEATURE_SET, ret,
                     "sgpc3_set_power_mode unsupported FS");
}

TEST (SGPC3_FS6_Tests, SGPC3Test_FS6) { sgpc3_test_all_inits(6); }

TEST (SGPC3_FS6_Tests, SGPC3Test_FS6_ULP) {
    int16_t ret = sgpc3_probe();
    CHECK_ZERO_TEXT(ret, "sgpc3_probe before setting ULP mode in SGPC3 FS6");
    ret = sgpc3_set_power_mode(1);
    CHECK_ZERO_TEXT(ret, "sgpc3_set_power_mode");
    sgpc3_test_all_inits(6);
}

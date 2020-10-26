#include "sensirion_test_setup.h"
#include "sgp30.h"
#include <inttypes.h>

#define SGP30_PRODUCT_TYPE 0
#define SGP30_IAQ_BASELINE 0x80018002
#define SGP30_TVOC_BASELINE 0x8000
#define SGP30_MEASURE_TEST_RESULT 0xd400
#define MIN_VALUE_TVOC 0
#define MAX_VALUE_TVOC 60000
#define MIN_VALUE_CO2 0
#define MAX_VALUE_CO2 60000
#define MIN_VALUE_ETHANOL 0
#define MAX_VALUE_ETHANOL 60000
#define MIN_VALUE_H2 0
#define MAX_VALUE_H2 60000
#define MAX_VALUE_HUMIDITY 256000
#define MEASUREMENT_DURATION_USEC 50000

static void sgp30_run_tests(uint16_t expected_feature_set) {
    uint8_t product_type;
    int16_t ret;
    uint16_t fs_version;
    uint16_t test_result;
    uint16_t tvoc_baseline;
    uint16_t tvoc_ppb;
    uint16_t co2_ppm;
    uint16_t ethanol_raw;
    uint16_t h2_raw;
    uint32_t iaq_baseline;
    uint64_t serial;

    const char* version = sgp30_get_driver_version();
    printf("sgp30_get_driver_version: %s\n", version);

    uint8_t addr = sgp30_get_configured_address();
    CHECK_EQUAL_TEXT(0x58, addr, "sgp30_get_configured_address");

    ret = sgp30_get_serial_id(&serial);
    CHECK_ZERO_TEXT(ret, "sgp30_get_serial_id");
    printf("SGP30 serial: %" PRIu64 "\n", serial);

    ret = sgp30_get_feature_set_version(&fs_version, &product_type);
    CHECK_ZERO_TEXT(ret, "sgp30_get_feature_set_version");
    printf("FS: 0x%02x, Type: 0x%02x\n", fs_version, product_type);
    CHECK_EQUAL_TEXT(expected_feature_set, fs_version,
                     "sgp30_get_feature_set_version feature set");
    CHECK_EQUAL_TEXT(SGP30_PRODUCT_TYPE, product_type,
                     "sgp30_get_feature_set_version product type");

    // Initialize IAQ
    ret = sgp30_iaq_init();
    CHECK_ZERO_TEXT(ret, "sgp30_iaq_init");

    // For FS > 0x20: Set tvoc baseline to inceptive baseline
    if (fs_version > 0x20) {
        ret = sgp30_get_tvoc_inceptive_baseline(&tvoc_baseline);
        CHECK_ZERO_TEXT(ret, "sgp30_get_tvoc_inceptive_baseline");

        // Hand-fused sensors may return 0 as inceptive baseline
        if (tvoc_baseline == 0) {
            tvoc_baseline = SGP30_TVOC_BASELINE;
        }

        ret = sgp30_set_tvoc_baseline(tvoc_baseline);
        CHECK_ZERO_TEXT(ret, "sgp30_set_tvoc_baseline");
    } else {
        ret = sgp30_get_tvoc_inceptive_baseline(&tvoc_baseline);
        CHECK_EQUAL_TEXT(SGP30_ERR_UNSUPPORTED_FEATURE_SET, ret,
                         "sgp30_get_tvoc_inceptive_baseline unsupported FS");

        ret = sgp30_set_tvoc_baseline(tvoc_baseline);
        CHECK_EQUAL_TEXT(SGP30_ERR_UNSUPPORTED_FEATURE_SET, ret,
                         "sgp30_set_tvoc_baseline unsupported FS");
    }

    // Set IAQ baseline to SGP30_IAQ_BASELINE. Then get it back and check
    // if it is indeed set to this value
    ret = sgp30_set_iaq_baseline(SGP30_IAQ_BASELINE);
    CHECK_ZERO_TEXT(ret, "sgp30_set_iaq_baseline");
    ret = sgp30_get_iaq_baseline(&iaq_baseline);
    CHECK_ZERO_TEXT(ret, "sgp30_get_iaq_baseline");
    CHECK_EQUAL_TEXT(SGP30_IAQ_BASELINE, iaq_baseline,
                     "sgp30_get_iaq_baseline mismatch");

    // check sensor for defects
    ret = sgp30_measure_test(&test_result);
    CHECK_ZERO_TEXT(ret, "sgp30_measure_test");
    CHECK_EQUAL_TEXT(SGP30_MEASURE_TEST_RESULT, test_result,
                     "sgp30_measure_test result");

    // iaq measurements (tvoc and co2)
    ret = sgp30_measure_iaq_blocking_read(&tvoc_ppb, &co2_ppm);
    CHECK_ZERO_TEXT(ret, "sgp30_measure_iaq_blocking_read");
    CHECK_TRUE_TEXT(tvoc_ppb >= MIN_VALUE_TVOC && tvoc_ppb <= MAX_VALUE_TVOC,
                    "sgp30_measure_iaq_blocking_read tvoc");
    CHECK_TRUE_TEXT(co2_ppm >= MIN_VALUE_CO2 && co2_ppm <= MAX_VALUE_CO2,
                    "sgp30_measure_iaq_blocking_read co2");

    ret = sgp30_measure_iaq();
    CHECK_ZERO_TEXT(ret, "sgp30_measure_iaq");
    sensirion_sleep_usec(MEASUREMENT_DURATION_USEC);
    ret = sgp30_read_iaq(&tvoc_ppb, &co2_ppm);
    CHECK_ZERO_TEXT(ret, "sgp30_read_iaq");
    CHECK_TRUE_TEXT(tvoc_ppb >= MIN_VALUE_TVOC && tvoc_ppb <= MAX_VALUE_TVOC,
                    "sgp30_read_iaq tvoc");
    CHECK_TRUE_TEXT(co2_ppm >= MIN_VALUE_CO2 && co2_ppm <= MAX_VALUE_CO2,
                    "sgp30_read_iaq co2");

    // tvoc measurements
    ret = sgp30_measure_tvoc_blocking_read(&tvoc_ppb);
    CHECK_ZERO_TEXT(ret, "sgp30_measure_tvoc_blocking_read");
    CHECK_TRUE_TEXT(tvoc_ppb >= MIN_VALUE_TVOC && tvoc_ppb <= MAX_VALUE_TVOC,
                    "sgp30_measure_tvoc_blocking_read tvoc");

    ret = sgp30_measure_tvoc();
    CHECK_ZERO_TEXT(ret, "sgp30_measure_tvoc");
    sensirion_sleep_usec(MEASUREMENT_DURATION_USEC);
    ret = sgp30_read_tvoc(&tvoc_ppb);
    CHECK_ZERO_TEXT(ret, "sgp30_read_tvoc");
    CHECK_TRUE_TEXT(tvoc_ppb >= MIN_VALUE_TVOC && tvoc_ppb <= MAX_VALUE_TVOC,
                    "spg30_read_tvoc tvoc");

    // co2 measurements
    ret = sgp30_measure_co2_eq_blocking_read(&co2_ppm);
    CHECK_ZERO_TEXT(ret, "sgp30_measure_co2_eq_blocking_read");
    CHECK_TRUE_TEXT(co2_ppm >= MIN_VALUE_CO2 && co2_ppm <= MAX_VALUE_CO2,
                    "sgp30_measure_co2_eq_blocking_read co2");

    ret = sgp30_measure_co2_eq();
    CHECK_ZERO_TEXT(ret, "sgp30_measure_co2_eq");
    sensirion_sleep_usec(MEASUREMENT_DURATION_USEC);
    ret = sgp30_read_co2_eq(&co2_ppm);
    CHECK_ZERO_TEXT(ret, "sgp30_read_co2_eq");
    CHECK_TRUE_TEXT(co2_ppm >= MIN_VALUE_CO2 && co2_ppm <= MAX_VALUE_CO2,
                    "sgp30_read_co2_eq co2");

    // raw measurements
    ret = sgp30_measure_raw_blocking_read(&ethanol_raw, &h2_raw);
    CHECK_ZERO_TEXT(ret, "sgp30_measure_raw_blocking_read");
    CHECK_TRUE_TEXT(ethanol_raw >= MIN_VALUE_ETHANOL &&
                        ethanol_raw <= MAX_VALUE_ETHANOL,
                    "sgp30_measure_raw_blocking_read ethanol");
    CHECK_TRUE_TEXT(h2_raw >= MIN_VALUE_H2 && h2_raw <= MAX_VALUE_H2,
                    "sgp30_measure_raw_blocking_read h2");

    ret = sgp30_measure_raw();
    CHECK_ZERO_TEXT(ret, "sgp30_measure_raw");
    sensirion_sleep_usec(MEASUREMENT_DURATION_USEC);
    ret = sgp30_read_raw(&ethanol_raw, &h2_raw);
    CHECK_ZERO_TEXT(ret, "sgp30_read_raw");
    CHECK_TRUE_TEXT(ethanol_raw >= MIN_VALUE_ETHANOL &&
                        ethanol_raw <= MAX_VALUE_ETHANOL,
                    "sgp30_read_raw ethanol");
    CHECK_TRUE_TEXT(h2_raw >= MIN_VALUE_H2 && h2_raw <= MAX_VALUE_H2,
                    "sgp30_read_raw h2");
}

static void test_setup() {
    int16_t ret = sgp30_probe();
    CHECK_ZERO_TEXT(ret, "sgp30_probe in test_setup");
}

static void test_teardown() {
    int16_t ret = sensirion_i2c_general_call_reset();
    CHECK_ZERO_TEXT(ret, "sensirion_i2c_general_call_reset");
    sensirion_i2c_release();
}

TEST_GROUP (SGP30_FS_0x20_Tests) {
    void setup() {
        sensirion_i2c_init();
        int16_t ret = sensirion_i2c_mux_set_single_channel(0x71, 0);
        CHECK_ZERO_TEXT(ret, "sensirion_mux_set_single_channel(0x71, 0)");
        test_setup();
    }

    void teardown() {
        test_teardown();
    }
};

TEST_GROUP (SGP30_FS_0x21_Tests) {
    void setup() {
        sensirion_i2c_init();
        int16_t ret = sensirion_i2c_mux_set_single_channel(0x71, 1);
        CHECK_ZERO_TEXT(ret, "sensirion_mux_set_single_channel(0x71, 1)");
        test_setup();
    }

    void teardown() {
        test_teardown();
    }
};

TEST_GROUP (SGP30_FS_0x22_Tests) {
    void setup() {
        sensirion_i2c_init();
        int16_t ret = sensirion_i2c_mux_set_single_channel(0x71, 2);
        CHECK_ZERO_TEXT(ret, "sensirion_mux_set_single_channel(0x71, 2)");
        test_setup();
    }

    void teardown() {
        test_teardown();
    }
};

TEST (SGP30_FS_0x20_Tests, SGP30_no_humi_compensation) {
    int16_t ret = sgp30_set_absolute_humidity(0);
    CHECK_ZERO_TEXT(ret, "sgp_set_absolute_humidity");
    sgp30_run_tests(0x20);
}

TEST (SGP30_FS_0x20_Tests, SGP30_with_humi_compensation) {
    int16_t ret = sgp30_set_absolute_humidity(MAX_VALUE_HUMIDITY);
    CHECK_ZERO_TEXT(ret, "sgp_set_absolute_humidity");
    sgp30_run_tests(0x20);
}

TEST (SGP30_FS_0x20_Tests, SGP30_too_much_humidity) {
    int16_t ret = sgp30_set_absolute_humidity(MAX_VALUE_HUMIDITY + 1);
    CHECK_EQUAL_TEXT(STATUS_FAIL, ret, "sgp_set_absolute_humidity should fail");
}

TEST (SGP30_FS_0x21_Tests, SGP30_no_humi_compensation) {
    int16_t ret = sgp30_set_absolute_humidity(0);
    CHECK_ZERO_TEXT(ret, "sgp_set_absolute_humidity");
    sgp30_run_tests(0x21);
}

TEST (SGP30_FS_0x21_Tests, SGP30_with_humi_compensation) {
    int16_t ret = sgp30_set_absolute_humidity(MAX_VALUE_HUMIDITY);
    CHECK_ZERO_TEXT(ret, "sgp_set_absolute_humidity");
    sgp30_run_tests(0x21);
}

TEST (SGP30_FS_0x21_Tests, SGP30_too_much_humidity) {
    int16_t ret = sgp30_set_absolute_humidity(MAX_VALUE_HUMIDITY + 1);
    CHECK_EQUAL_TEXT(STATUS_FAIL, ret, "sgp_set_absolute_humidity should fail");
}

TEST (SGP30_FS_0x22_Tests, SGP30_no_humi_compensation) {
    int16_t ret = sgp30_set_absolute_humidity(0);
    CHECK_ZERO_TEXT(ret, "sgp_set_absolute_humidity");
    sgp30_run_tests(0x22);
}

TEST (SGP30_FS_0x22_Tests, SGP30_with_humi_compensation) {
    int16_t ret = sgp30_set_absolute_humidity(MAX_VALUE_HUMIDITY);
    CHECK_ZERO_TEXT(ret, "sgp_set_absolute_humidity");
    sgp30_run_tests(0x22);
}

TEST (SGP30_FS_0x22_Tests, SGP30_too_much_humidity) {
    int16_t ret = sgp30_set_absolute_humidity(MAX_VALUE_HUMIDITY + 1);
    CHECK_EQUAL_TEXT(STATUS_FAIL, ret, "sgp_set_absolute_humidity should fail");
}

#include "sensirion_test_setup.h"
#include "sgp40.h"
#include <inttypes.h>
#include <stdio.h>

#define MIN_VALUE_ETHANOL 0
#define MAX_VALUE_ETHANOL 60000

TEST_GROUP (SGP40_Tests) {
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

TEST (SGP40_Tests, SGP40_Test) {
    uint8_t serial_id[SGP40_SERIAL_ID_NUM_BYTES];
    int16_t ret;
    uint16_t signal, ix;

    const char* version = sgp40_get_driver_version();
    printf("SGP40 Driver version: %s\n", version);

    ret = sgp40_probe();
    CHECK_ZERO_TEXT(ret, "sgp40_probe");

    ret = sgp40_get_serial_id(serial_id);
    CHECK_ZERO_TEXT(ret, "sgp40_get_serial_id");
    printf("SGP40 serial: ");
    for (ix = 0; ix < SGP40_SERIAL_ID_NUM_BYTES - 1; ix++) {
        printf("%02X:", serial_id[ix]);
    }
    printf("%02X\n", serial_id[ix]);

    uint64_t serial_id_dec = 0;
    for (int i = 0; i < SGP40_SERIAL_ID_NUM_BYTES; ++i) {
        serial_id_dec = (serial_id_dec << 8) | serial_id[i];
    }
    printf("SGP40 serial dec: %" PRIu64 "\n", serial_id_dec);

    ret = sgp40_measure_raw_blocking_read(&signal);
    printf("SGP Reading: %u\n", signal);
    CHECK_ZERO_TEXT(ret, "sgp40_measure_raw_blocking_read");
    CHECK_TRUE_TEXT(signal >= MIN_VALUE_ETHANOL && signal <= MAX_VALUE_ETHANOL,
                    "sgp40_measure_raw_blocking_read value");
}

TEST (SGP40_Tests, sgp40_convert_rht) {
    int32_t rh = 50000;
    int32_t t = 25000;
    uint16_t rh_sensor_format;
    uint16_t t_sensor_format;
    sgp40_convert_rht(rh, t, &rh_sensor_format, &t_sensor_format);
    CHECK_TEXT(rh_sensor_format >= (SGP40_DEFAULT_HUMIDITY - 40) &&
                   rh_sensor_format <= (SGP40_DEFAULT_HUMIDITY + 40),
               "sgp40_convert_rht wrong humidity");
    CHECK_TEXT(t_sensor_format >= (SGP40_DEFAULT_TEMPERATURE - 40) &&
                   t_sensor_format <= (SGP40_DEFAULT_TEMPERATURE + 40),
               "sgp40_convert_rht wrong temperature");
}

include ../embedded-common/test-config/base_config.inc
sgp_driver_dir := ${driver_dir}/embedded-sgp
include ${sgp_driver_dir}/svm30/default_config.inc
include ${sgp_driver_dir}/sgp30/default_config.inc
include ${sgp_driver_dir}/sgp40_voc_index/default_config.inc
include ${sgp_driver_dir}/sgp40/default_config.inc
include ${sgp_driver_dir}/sgpc3/default_config.inc

sgp30_test_binaries := sgp30-test-hw_i2c sgp30-test-sw_i2c
sgp40_test_binaries := sgp40-test-hw_i2c sgp40-test-sw_i2c
sgp40_voc_index_test_binaries := sgp40-voc-index-test-hw_i2c \
                                 sgp40-voc-index-test-sw_i2c \
                                 sensirion-voc-algorithm-test
sgpc3_test_binaries := sgpc3-test-hw_i2c sgpc3-test-sw_i2c
svm30_test_binaries := svm30-test-hw_i2c svm30-test-sw_i2c
sgp_test_binaries := ${sgp30_test_binaries} \
                     ${sgp40_test_binaries} \
                     ${sgp40_voc_index_test_binaries} \
                     ${sgpc3_test_binaries} \
                     ${svm30_test_binaries}

.PHONY: all clean prepare test

all: clean prepare test

prepare:
	cd ${sgp_driver_dir} && $(MAKE) prepare

sgp30-test-hw_i2c: CONFIG_I2C_TYPE := hw_i2c
sgp30-test-hw_i2c: sgp30-test.cpp ${sgp30_sources} ${hw_i2c_sources} ${sensirion_test_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

sgp30-test-sw_i2c: CONFIG_I2C_TYPE := sw_i2c
sgp30-test-sw_i2c: sgp30-test.cpp ${sgp30_sources} ${sw_i2c_sources} ${sensirion_test_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

sgp40-test-hw_i2c: CONFIG_I2C_TYPE := hw_i2c
sgp40-test-hw_i2c: sgp40-test.cpp ${sgp40_sources} ${hw_i2c_sources} ${sensirion_test_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

sgp40-test-sw_i2c: CONFIG_I2C_TYPE := sw_i2c
sgp40-test-sw_i2c: sgp40-test.cpp ${sgp40_sources} ${sw_i2c_sources} ${sensirion_test_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

sgp40-voc-index-test-hw_i2c: CONFIG_I2C_TYPE := hw_i2c
sgp40-voc-index-test-hw_i2c: sgp40-voc-index-test.cpp ${sgp40_voc_index_sources} ${hw_i2c_sources} ${sensirion_test_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

sgp40-voc-index-test-sw_i2c: CONFIG_I2C_TYPE := sw_i2c
sgp40-voc-index-test-sw_i2c: sgp40-voc-index-test.cpp ${sgp40_voc_index_sources} ${sw_i2c_sources} ${sensirion_test_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

sensirion-voc-algorithm-test: sensirion-voc-algorithm-test.cpp ${sgp40_voc_index_voc_algorithm_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

sgpc3-test-hw_i2c: CONFIG_I2C_TYPE := hw_i2c
sgpc3-test-hw_i2c: sgpc3-test.cpp ${sgpc3_sources} ${hw_i2c_sources} ${sensirion_test_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

sgpc3-test-sw_i2c: CONFIG_I2C_TYPE := sw_i2c
sgpc3-test-sw_i2c: sgpc3-test.cpp ${sgpc3_sources} ${sw_i2c_sources} ${sensirion_test_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

svm30-test-hw_i2c: svm30-test.cpp ${svm30_sources} ${hw_i2c_sources} ${sensirion_test_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

svm30-test-sw_i2c: CONFIG_I2C_TYPE := sw_i2c
svm30-test-sw_i2c: svm30-test.cpp ${svm30_sources} ${sw_i2c_sources} ${sensirion_test_sources}
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) ${sgp_test_binaries}

test: prepare ${sgp_test_binaries}
	set -ex; for test in ${sgp_test_binaries}; do echo $${test}; ./$${test}; echo; done;

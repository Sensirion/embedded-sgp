sht_source_dir := ../../embedded-sht/shtc1
sht_common_dir := ../../embedded-sht/sht-common
sht_sources := ${sht_source_dir}/shtc1.c ${sht_common_dir}/sht.h \
               ${sht_common_dir}/sht_common.c ${sht_common_dir}/sht_common.h \
               ${sht_common_dir}/git_version.c ${sht_common_dir}/git_version.h

sgp_source_dir := ../sgp30
sgp_common_dir := ../sgp-common
sgp_sources := ${sgp_source_dir}/sgp30_featureset.c \
               ${sgp_common_dir}/sgp_featureset.h \
               ${sgp_common_dir}/sgp_git_version.c ${sgp_common_dir}/sgp_git_version.h \
               ${sgp_source_dir}/sgp30.c ${sgp_source_dir}/sgp30.h

sensirion_common_dir := ../embedded-common
sensirion_common_sources := ${sensirion_common_dir}/sensirion_arch_config.h \
                            ${sensirion_common_dir}/sensirion_i2c.h \
                            ${sensirion_common_dir}/sensirion_common.c

sw_i2c_dir := ${sensirion_common_dir}/sw_i2c
hw_i2c_dir := ${sensirion_common_dir}/hw_i2c

CFLAGS := -Os -Wall -Wstrict-aliasing=1 -I. -I${sensirion_common_dir} -I${sgp_source_dir} \
          -I${sgp_common_dir} -I${sht_common_dir}

svm30_binaries := svm30_example_usage_hw_i2c svm30_example_usage_sw_i2c
svm_binaries += ${svm30_binaries}
svm_sources := svm30.c svm30.h

sw_objects := sensirion_sw_i2c.o sensirion_sw_i2c_implementation.o
hw_objects := sensirion_hw_i2c_implementation.o
all_objects := ${hw_objects} ${sw_objects}

.PHONY: all

all: ${svm_binaries}

sensirion_sw_i2c_implementation.o: ${sw_i2c_dir}/sensirion_sw_i2c_implementation.c
	$(CC) $(CFLAGS) -c -o $@ $^
sensirion_hw_i2c_implementation.o: ${hw_i2c_dir}/sensirion_hw_i2c_implementation.c
	$(CC) $(CFLAGS) -c -o $@ $^

sensirion_sw_i2c.o: ${sw_i2c_dir}/sensirion_sw_i2c.c
	$(CC) -I${sw_i2c_dir} $(CFLAGS) -c -o $@ $^

svm30_example_usage_hw_i2c: ${hw_objects} ${sensirion_common_sources} ${sgp_sources} ${sht_sources} ${svm_sources} svm30_example_usage.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

svm30_example_usage_sw_i2c: ${sw_objects} ${sensirion_common_sources} ${sgp_sources} ${sht_sources} ${svm_sources} svm30_example_usage.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS)

clean:
	$(RM) ${all_objects} ${svm_binaries}

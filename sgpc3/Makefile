sensirion_common_dir := ../embedded-common
sgp_common_dir := ../sgp-common
sw_i2c_dir := ${sensirion_common_dir}/sw_i2c
hw_i2c_dir := ${sensirion_common_dir}/hw_i2c

CFLAGS := -Wall -Werror -I. -I${sensirion_common_dir} -I${sgp_common_dir}

sensirion_common_objects := sensirion_common.o
sgpc3_binaries := sgpc3_example_usage_hw_i2c sgpc3_example_usage_sw_i2c
sgpc3_featureset := sgpc3_featureset.o
sgp_featuresets += ${sgpc3_featureset}
sgp_binaries += ${sgpc3_binaries}

sw_objects := sensirion_sw_i2c.o sensirion_sw_i2c_implementation.o
hw_objects := sensirion_hw_i2c_implementation.o
all_objects := ${sensirion_common_objects} ${hw_objects} ${sw_objects} ${sgp_featuresets} sgpc3.o

.PHONY: all

all: ${sgp_binaries}

sensirion_common.o: ${sensirion_common_dir}/sensirion_common.c
	$(CC) $(CFLAGS) -c -o $@ $^
sensirion_sw_i2c_implementation.o: ${sw_i2c_dir}/sensirion_sw_i2c_implementation.c
	$(CC) $(CFLAGS) -c -o $@ $^
sensirion_hw_i2c_implementation.o: ${hw_i2c_dir}/sensirion_hw_i2c_implementation.c
	$(CC) $(CFLAGS) -c -o $@ $^

sensirion_sw_i2c.o: ${sw_i2c_dir}/sensirion_sw_i2c.c
	$(CC) -I${sw_i2c_dir} $(CFLAGS) -c -o $@ $^

sgpc3.o: ${sgp_common_dir}/sgp_featureset.h ${sensirion_common_dir}/sensirion_i2c.h sgpc3.c sgpc3.h

sgpc3_example_usage_sw_i2c: ${sensirion_common_objects} ${sw_objects} ${sgp_featuresets} sgpc3.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS) sgpc3_example_usage.c

sgpc3_example_usage_hw_i2c: ${sensirion_common_objects} ${hw_objects} ${sgp_featuresets} sgpc3.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LOADLIBES) $(LDLIBS) sgpc3_example_usage.c

clean:
	$(RM) ${all_objects} ${sgp_binaries}

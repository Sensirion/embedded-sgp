# See user_config.inc for build customization
-include user_config.inc
include default_config.inc

.PHONY: all clean

all: sgpc3_with_shtc1_example_usage

sgpc3_with_shtc1_example_usage: clean
	$(CC) $(CFLAGS) -o $@ ${sgpc3_with_shtc1_sources} ${${CONFIG_I2C_TYPE}_sources} ${sgpc3_with_shtc1_dir}/sgpc3_with_shtc1_example_usage.c

clean:
	$(RM) sgpc3_with_shtc1_example_usage

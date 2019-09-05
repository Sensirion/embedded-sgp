# See user_config.inc for build customization
-include user_config.inc
include default_config.inc

.PHONY: all clean

all: sgp30_example_usage

sgp30_example_usage: clean
	$(CC) $(CFLAGS) -o $@ ${sgp30_sources} ${${CONFIG_I2C_TYPE}_sources} ${sgp30_dir}/sgp30_example_usage.c

clean:
	$(RM) sgp30_example_usage

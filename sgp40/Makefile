# See user_config.inc for build customizations
-include user_config.inc
include default_config.inc

.PHONY: all clean

all: sgp40_example_usage

sgp40_example_usage: clean
	$(CC) $(CFLAGS) -o $@ ${sgp40_sources} ${${CONFIG_I2C_TYPE}_sources} ${sgp40_dir}/sgp40_example_usage.c

clean:
	$(RM) sgp40_example_usage

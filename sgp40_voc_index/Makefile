# See user_config.inc for build customization
-include user_config.inc
include default_config.inc

.PHONY: all clean

all: sgp40_voc_index_example_usage

sgp40_voc_index_example_usage: clean
	$(CC) $(CFLAGS) -o $@ ${sgp40_voc_index_sources} ${${CONFIG_I2C_TYPE}_sources} ${sgp40_voc_index_dir}/sgp40_voc_index_example_usage.c ${LDFLAGS}

clean:
	$(RM) sgp40_voc_index_example_usage

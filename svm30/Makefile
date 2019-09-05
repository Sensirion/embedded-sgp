# See user_config.inc for build customization
-include user_config.inc
include default_config.inc

.PHONY: all clean

all: svm30_example_usage

svm30_example_usage: clean
	$(CC) $(CFLAGS) -o $@ ${svm30_sources} ${${CONFIG_I2C_TYPE}_sources} ${svm30_dir}/svm30_example_usage.c

clean:
	$(RM) svm30_example_usage

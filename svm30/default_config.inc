sgp_driver_dir ?= ..
sht_driver_dir ?= ${sgp_driver_dir}/embedded-sht
sensirion_common_dir ?= ${sgp_driver_dir}/embedded-common
sgp_common_dir ?= ${sgp_driver_dir}/sgp-common
sgp30_dir ?= ${sgp_driver_dir}/sgp30
svm30_dir ?= ${sgp_driver_dir}/svm30
sht_common_dir ?= ${sht_driver_dir}/sht-common
sht_utils_dir ?= ${sht_driver_dir}/utils
shtc1_dir ?= ${sht_driver_dir}/shtc1
CONFIG_I2C_TYPE ?= hw_i2c

sw_i2c_impl_src ?= ${sensirion_common_dir}/sw_i2c/sensirion_sw_i2c_implementation.c
hw_i2c_impl_src ?= ${sensirion_common_dir}/hw_i2c/sensirion_hw_i2c_implementation.c

CFLAGS ?= -Os -Wall -fstrict-aliasing -Wstrict-aliasing=1 -Wsign-conversion -fPIC
CFLAGS += -I${sensirion_common_dir} -I${sht_utils_dir} \
          -I${sgp_common_dir} -I${sgp30_dir} \
          -I${sht_common_dir} -I${shtc1_dir} -I${svm30_dir} \
          -I${sensirion_common_dir}/${CONFIG_I2C_TYPE}

sensirion_common_sources = ${sensirion_common_dir}/sensirion_arch_config.h \
                           ${sensirion_common_dir}/sensirion_i2c.h \
                           ${sensirion_common_dir}/sensirion_common.h \
                           ${sensirion_common_dir}/sensirion_common.c

sht_humidity_conversion_sources = \
    ${sht_utils_dir}/sensirion_humidity_conversion.h \
    ${sht_utils_dir}/sensirion_humidity_conversion.c

sgp_common_sources = ${sgp_common_dir}/sgp_git_version.h \
                     ${sgp_common_dir}/sgp_git_version.c \

sgp30_sources = ${sgp30_dir}/sgp30.h ${sgp30_dir}/sgp30.c

sht_common_sources = ${sht_common_dir}/sht_git_version.h \
                     ${sht_common_dir}/sht_git_version.c

shtc1_sources = ${shtc1_dir}/shtc1.h ${shtc1_dir}/shtc1.c

svm30_sources = ${sensirion_common_sources} \
                ${sht_humidity_conversion_sources} \
                ${sgp_common_sources} \
                ${sht_common_sources} \
                ${sgp30_sources} \
                ${shtc1_sources} \
                ${svm30_dir}/svm30.h ${svm30_dir}/svm30.c

hw_i2c_sources = ${hw_i2c_impl_src}
sw_i2c_sources = ${sensirion_common_dir}/sw_i2c/sensirion_sw_i2c_gpio.h \
                 ${sensirion_common_dir}/sw_i2c/sensirion_sw_i2c.c \
                 ${sw_i2c_impl_src}

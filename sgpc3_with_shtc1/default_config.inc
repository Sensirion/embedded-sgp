sgp_driver_dir ?= ..
sht_driver_dir ?= ${sgp_driver_dir}/embedded-sht
sensirion_common_dir ?= ${sgp_driver_dir}/embedded-common
sgp_common_dir ?= ${sgp_driver_dir}/sgp-common
sgpc3_dir ?= ${sgp_driver_dir}/sgpc3
sgpc3_with_shtc1_dir ?= ${sgp_driver_dir}/sgpc3_with_shtc1
sht_common_dir ?= ${sht_driver_dir}/sht-common
sht_utils_dir ?= ${sht_driver_dir}/utils
shtc1_dir ?= ${sht_driver_dir}/shtc1
CONFIG_I2C_TYPE ?= hw_i2c

sw_i2c_impl_src ?= ${sensirion_common_dir}/sw_i2c/sensirion_sw_i2c_implementation.c
hw_i2c_impl_src ?= ${sensirion_common_dir}/hw_i2c/sensirion_hw_i2c_implementation.c

CFLAGS ?= -Os -Wall -fstrict-aliasing -Wstrict-aliasing=1 -Wsign-conversion -fPIC
CFLAGS += -I${sensirion_common_dir} -I${sht_utils_dir} \
          -I${sgp_common_dir} -I${sgpc3_dir} \
          -I${sht_common_dir} -I${shtc1_dir} -I${sgpc3_with_shtc1_dir} \
          -I${sensirion_common_dir}/${CONFIG_I2C_TYPE}

sensirion_common_sources = ${sensirion_common_dir}/sensirion_arch_config.h \
                           ${sensirion_common_dir}/sensirion_i2c.h \
                           ${sensirion_common_dir}/sensirion_common.h \
                           ${sensirion_common_dir}/sensirion_common.c

sht_humidity_conversion_sources = \
    ${sht_utils_dir}/sensirion_humidity_conversion.h \
    ${sht_utils_dir}/sensirion_humidity_conversion.c

sgp_common_sources = ${sgp_common_dir}/sgp_git_version.h \
                     ${sgp_common_dir}/sgp_git_version.c

sgpc3_sources = ${sgpc3_dir}/sgpc3.h ${sgpc3_dir}/sgpc3.c

sht_common_sources = ${sht_common_dir}/sht_git_version.h \
                     ${sht_common_dir}/sht_git_version.c

shtc1_sources = ${shtc1_dir}/shtc1.h ${shtc1_dir}/shtc1.c

sgpc3_with_shtc1_sources = ${sensirion_common_sources} \
                           ${sht_humidity_conversion_sources} \
                           ${sgp_common_sources} \
                           ${sht_common_sources} \
                           ${sgpc3_sources} \
                           ${shtc1_sources} \
                           ${sgpc3_with_shtc1_dir}/sgpc3_with_shtc1.h \
                           ${sgpc3_with_shtc1_dir}/sgpc3_with_shtc1.c

hw_i2c_sources = ${hw_i2c_impl_src}
sw_i2c_sources = ${sensirion_common_dir}/sw_i2c/sensirion_sw_i2c_gpio.h \
                 ${sensirion_common_dir}/sw_i2c/sensirion_sw_i2c.c \
                 ${sw_i2c_impl_src}

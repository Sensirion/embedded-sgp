sgp_driver_dir ?= ..
sht_driver_dir ?= ${sgp_driver_dir}/embedded-sht
sensirion_common_dir ?= ${sgp_driver_dir}/embedded-common
sgp_common_dir ?= ${sgp_driver_dir}/sgp-common
sgp40_dir ?= ${sgp_driver_dir}/sgp40
sgp40_voc_index_dir ?= ${sgp_driver_dir}/sgp40_voc_index
sgp40_voc_index_voc_algorithm_dir ?= ${sgp40_voc_index_dir}
sht_common_dir ?= ${sht_driver_dir}/sht-common
shtc1_dir ?= ${sht_driver_dir}/shtc1
CONFIG_I2C_TYPE ?= hw_i2c

sw_i2c_impl_src ?= ${sensirion_common_dir}/sw_i2c/sensirion_sw_i2c_implementation.c
hw_i2c_impl_src ?= ${sensirion_common_dir}/hw_i2c/sensirion_hw_i2c_implementation.c

CFLAGS ?= -Os -Wall -fstrict-aliasing -Wstrict-aliasing=1 -fPIC
CFLAGS += -I${sensirion_common_dir} -I${sgp_common_dir} -I${sgp40_dir} \
          -I${sht_common_dir} -I${shtc1_dir} \
          -I${sgp40_voc_index_dir} \
          -I${sensirion_common_dir}/${CONFIG_I2C_TYPE}
LDFLAGS += -lm

sensirion_common_sources = ${sensirion_common_dir}/sensirion_arch_config.h \
                           ${sensirion_common_dir}/sensirion_i2c.h \
                           ${sensirion_common_dir}/sensirion_common.h \
                           ${sensirion_common_dir}/sensirion_common.c

sgp_common_sources = ${sgp_common_dir}/sgp_git_version.h \
                     ${sgp_common_dir}/sgp_git_version.c

sgp40_sources = ${sensirion_common_sources} \
                ${sgp_common_sources} \
                ${sgp40_dir}/sgp40.h ${sgp40_dir}/sgp40.c

sgp40_voc_index_voc_algorithm_sources = ${sgp40_voc_index_voc_algorithm_dir}/sensirion_voc_algorithm.h \
                                     ${sgp40_voc_index_voc_algorithm_dir}/sensirion_voc_algorithm.c

sgp40_voc_index_sources = ${sgp40_sources} \
                        ${sht_common_sources} \
                        ${shtc1_sources} \
                        ${sgp40_voc_index_voc_algorithm_sources} \
                        ${sgp40_voc_index_dir}/sgp40_voc_index.h \
                        ${sgp40_voc_index_dir}/sgp40_voc_index.c

sht_common_sources = ${sht_common_dir}/sht_git_version.h \
                     ${sht_common_dir}/sht_git_version.c

shtc1_sources = ${shtc1_dir}/shtc1.h ${shtc1_dir}/shtc1.c

hw_i2c_sources = ${hw_i2c_impl_src}
sw_i2c_sources = ${sensirion_common_dir}/sw_i2c/sensirion_sw_i2c_gpio.h \
                 ${sensirion_common_dir}/sw_i2c/sensirion_sw_i2c.c \
                 ${sw_i2c_impl_src}

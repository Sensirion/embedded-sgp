# embedded-sgp [![CircleCI](https://circleci.com/gh/Sensirion/embedded-sgp.svg?style=shield)](https://circleci.com/gh/Sensirion/embedded-sgp) [![GitHub license](https://img.shields.io/badge/license-BSD3-blue.svg)](https://raw.githubusercontent.com/Sensirion/embedded-sgp/master/LICENSE)
This repository contains the embedded driver sources for Sensirion's
SGP product line.

## Download the Latest Driver Release

**Download the latest ready-to-use driver bundle from the [releases
page](https://github.com/Sensirion/embedded-sgp/releases/)**

We strongly recommend to use the driver from the release bundle instead of
cloning the repository.

**Note:** If you use the SVM40 module (which contains an SGP40) please check
out https://github.com/Sensirion/embedded-svm40.

## Clone this repository
```
git clone --recursive https://github.com/Sensirion/embedded-sgp.git
```

## Repository content

* embedded-common - Submodule repository for common embedded driver stuff
* embedded-sht - Submodule repository for SHT drivers
* sgp40 - SPG40 driver
* sgp40\_voc\_index - Driver for a SGP40 and SHTC3 combo with a VOC algorithm.
* sgp30 - SGP30 driver
* sgpc3 - SGPC3 driver
* svm30 - Driver for the SVM30 module consisting of a SPG30 and an SHTC3 sensor.
* sgpc3\_with\_shtc1 - Driver for a SGPC3 and SHTC1 sensor combo.
* sgp-common - Common code for all SGP drivers.

## Collecting resources
```
make release
```
This will create a release folder
with the necessary driver files in it, including a Makefile. That way, you have
just ONE folder with all the sources ready to build your driver for your
platform.


## Files to adjust (from embedded-common)
You only need to touch the following files:

* `sensirion_arch_config.h` (architecture specifics, you need to specify the
integer sizes)

and depending on your i2c implementation one of the following:

* `sensirion_hw_i2c_implementation.c` (functions for hardware i2c
communication if your platform supports that)
* `sensirion_sw_i2c_implementation.c` (functions for software i2c
communication via GPIOs)


## Building the driver
1. Step into your desired directory (e.g.: release/sgp30)
2. Adjust sensirion\_arch\_config.h if you don't have the `<stdint.h>` header
   file available
3. Implement necessary functions in one of the `*_implementation.c` files
4. make

---

Please check the [embedded-common](https://github.com/Sensirion/embedded-common)
repository for further information and sample implementations.

---

# embedded-sgp [![CircleCI](https://circleci.com/gh/Sensirion/embedded-sgp.svg?style=shield)](https://circleci.com/gh/Sensirion/embedded-sgp) [![GitHub license](https://img.shields.io/badge/license-BSD3-blue.svg)](https://raw.githubusercontent.com/Sensirion/embedded-sgp/master/LICENSE)
This repository contains the embedded driver sources for Sensirion's
SGP product line.

## Clone this repository
```
git clone --recursive https://github.com/Sensirion/embedded-sgp.git
```

## Repository content
* embedded-common (submodule repository for common embedded driver stuff)
* sgp40 (SPG40 driver related)
* sgp30 (SGP30 driver related)
* sgpc3 (SGPC3 driver related)
* sgp-common (SGPxx related)

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


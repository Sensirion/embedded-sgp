# embedded-sgp
Embedded SGP Drivers

## Clone this repository
```
git clone --recursive https://github.com/Sensirion/embedded-sgp.git
```

## Repistory content
* embedded-common (submodule repository for common embedded driver stuff)
* sgp30 (sgp30 driver related)
* sgpc3 (sgpc3 driver related)
* sgp-common (SGPxx related)

## Collecting resources
```
make release
```
This will create a release folder with the necessary driver files in it, including a Makefile. That way, you have just ONE folder with all the sources ready to build your driver for your platform.


## Files to adjust (from embedded-common)
You only need to touch the following files:

* ```sensirion_arch_config.h``` (architecture specifics, you need to specify the integer sizes)

and depending on your i2c implementation one of the following:

* ```sensirion_hw_i2c_implementation.c``` (functions for hardware i2c communication if your platform supports that)
* ```sensirion_sw_i2c_implementation.c``` (functions for software i2c communication via GPIOs)


## Building the driver
1. Step into your desired directory (e.g.: release/sgp30)
2. Adjust sensirion_arch_config.h if you don't have the ```<stdint.h>``` header file available.
3. Implement necessary functions in the ```*_implementation.c``` files:
4. make


---

Please check the [embedded-common](https://github.com/Sensirion/embedded-common) repository for further information and sample implementations.

---




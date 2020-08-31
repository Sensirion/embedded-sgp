Getting Started on Raspberry Pi
===============================

This document explains how to set up the SGP40 to run on a Raspberry Pi. Since
the SGP40 runs at 3.3V, which matches the Raspberry Pi's logic level, no level
shifting is required on the I2C bus.

Setup Guide
-----------

Connecting the Sensor
~~~~~~~~~~~~~~~~~~~~~

Your sensor has the four different connectors: VCC, GND, SDA, SCL.
Use the following pins to connect your SGP40:

+-----------+---------------------+
| *SGP40*   | *Raspberry Pi*      |
+===========+=====================+
| VCC       | Pin 1 (3.3V source) |
+-----------+---------------------+
| GND       | Pin 6               |
+-----------+---------------------+
| SDA       | Pin 3               |
+-----------+---------------------+
| SCL       | Pin 5               |
+-----------+---------------------+


|RaspberryPi Pinout|

Raspberry Pi
~~~~~~~~~~~~

 - Install the Raspberry Pi OS on to your Raspberry Pi
    - For help with this, kindly refer to the Raspberry Pi website:
      https://projects.raspberrypi.org/en/projects/raspberry-pi-setting-up
 - Enable the I2C interface in the Raspberry Pi config
    - Open a terminal
    - Run ``sudo raspi-config``
    - Select "5 Interfacing Options Configure connections to peripherals"
    - Select "P5 I2C Enable/Disable automatic loading of I2C kernel module"
    - Select "Yes" when questioned "Would you like the ARM I2C interface to be enabled?"
    - The Raspberry Pi should respond with "The ARM I2C interface is enabled". Confirm with "Ok"
 - Download driver
    - Go to the Sensirion SGP Driver Release page (https://github.com/Sensirion/embedded-sgp/releases) and download the ``sgp40-VERSION.zip`` file.
    - Unzip the file into the directory where you want to install the driver (Run: ``unzip sgp40-VERSION.zip``).
 - Adapt the driver to work with Raspberry Pi
    - We use the ``linux_user_space`` implementation from ``hw_i2c`` (The
      Linux kernel provides an I2C controller, thus "hardware" I2C) to run the
      driver on the Raspberry Pi. To use the in the project provided sample
      implementation navigate to the
      ``./hw_i2c/sample-implementations/linux_user_space`` directory.
    - Copy the file ``sensirion_hw_i2c_implementation.c`` from there and
      replace the file named the same directly in ``./hw_i2c`` or just run this
      command in the root directory of your project:
      ``cp ./hw_i2c/sample-implementations/linux_user_space/sensirion_hw_i2c_implementation.c ./hw_i2c/``.
 - Compile the driver
    - Run ``make`` in the root directory of your project.

Output:

::

    rm -f sgp40_example_usage
    cc -Os -Wall -fstrict-aliasing -Wstrict-aliasing=1 -Wsign-conversion -fPIC -I. -I. -I. -I./hw_i2c -o sgp40_example_usage ./sensirion_arch_config.h ./sensirion_i2c.h ./sensirion_common.h ./sensirion_common.c ./sgp_git_version.h ./sgp_git_version.c ./sgp40.h ./sgp40.c ./hw_i2c/sensirion_hw_i2c_implementation.c ./sgp40_example_usage.c

-  **To test the setup run the example code which was just compiled with**
   ``./sgp40_example_usage``

Output:

::

    SGP40 driver version 6.0.0-32-gc792a99
    SGP sensor probing successful
    SerialID: 00:00:02:37:BF:3D
    sraw: 30403
    sraw: 30414
    sraw: 30407
    sraw: 30403
    sraw: 30390
    sraw: 30379
    sraw: 30370
    sraw: 30383
    ...

Troubleshooting
---------------

SGP sensor probing failed
~~~~~~~~~~~~~~~~~~~~~~~~~

- Ensure that you connected the sensor correctly: all cables are fully so all
  cables are fully plugged in and connected to the correct header.
- Ensure that I2C is enabled on the Raspberry Pi. For this redo the steps on
  "Enable the I2C interface in the Raspberry Pi config" in the guide above.
- Ensure that your user account has read and write access to the I2C device. If
  it only works with user root (``sudo ./sgp40_example_usage``), it's typically
  due to wrong permission settings.

.. |RaspberryPi Pinout| image:: ./images/GPIO-Pinout-Diagram-2.png

SGP40 Voc Index on Raspberry Pi
===============================

This document explains how to set the SGP40 up to run with a Raspberry
Pi. This became very easy with the new SGP40 since it can run with a
logic level of 3.3v which matches the one the Raspberry Pi has.

Getting started
---------------

Connecting the Sensor
~~~~~~~~~~~~~~~~~~~~~

| Your Sensor should have four different connectors: VCC, GND, SDA, SCL.
Please refere to other documentation of the Sensor to find out which
connector is which on your specific modelle.
| Use the following pins to connect your SGP40:

+-----------+------------------+
| *SGP40*   | *Raspberry Pi*   |
+===========+==================+
| VCC       | Pin 1            |
+-----------+------------------+
| GND       | Pin 6            |
+-----------+------------------+
| SDA       | Pin 3            |
+-----------+------------------+
| SCL       | Pin 5            |
+-----------+------------------+

|RaspberryPi Pinout|

Raspberry Pi
~~~~~~~~~~~~

-  **Install the newest Raspberry Pi OS on to your Raspberry Pi. If you
   don't know how to do that, there is a guide on the Raspberry Pi
   website**
   (https://projects.raspberrypi.org/en/projects/raspberry-pi-setting-up).
-  Enable the I2C interface in the Raspberry Pi config.
    -  Open a terminal.
    -  Run ``sudo raspi-config``.
    -  Select "5 Interfacing Options Configure connections to peripherals".
    -  Select "P5 I2C Enable/Disable automatic loading of I2C kernel module".
    -  Select "Yes" when questioned "Would you like the ARM I2C interface to be enabled?".
    -  The Raspberry Pi should respond with "The ARM I2C interface is enabled". Confirm with "Ok"
-  Download driver.
    -  Go to the Sensirion SGP Driver Release page (https://github.com/Sensirion/embedded-sgp/releases) and download the ``sgp40-voc-index-VERSION.zip`` file.
    -  Unzip the file into the directory where you want to install the driver (Run: ``unzip sgp40-voc-index-VERSION.zip`` ).
-  Adapt the driver to work with Raspberry Pi.
    -  We use the linux\_user\_space implementation of the hw\_i2c to run the driver on the Raspberry Pi. To use the in the project provided sample implementation navigate to the ``./hw_i2c/sample-implementations/linux_user_space`` direcory.
    -  Copy the file ``sensirion_hw_i2c_implementation.c`` from there and replace the file named the same directly in ``./hw_i2c`` or just run this command in the root directory of your project: ``cp ./hw_i2c/sample-implementations/linux_user_space/sensirion_hw_i2c_implementation.c ./hw_i2c/``.
-  Compile the driver.
    -  Run ``make`` in the root directory of your project.

Output:

::

    rm -f sgp40_voc_index_example_usage
    cc -Os -Wall -fstrict-aliasing -Wstrict-aliasing=1 -fPIC -I. -I. -I. -I. -I. -I. -I./hw_i2c -o sgp40_voc_index_example_usage ./sensirion_arch_config.h ./sensirion_i2c.h ./sensirion_common.h ./sensirion_common.c ./sgp_git_version.h ./sgp_git_version.c ./sht_git_version.h ./sht_git_version.c ./sgp40.h ./sgp40.c ./shtc1.h ./shtc1.c ./sensirion_voc_algorithm.h ./sensirion_voc_algorithm.c ./sgp40_voc_index.h ./sgp40_voc_index.c ./hw_i2c/sensirion_hw_i2c_implementation.c ./sgp40_voc_index_example_usage.c -lm

-  **To test the setup run the example code which was just compiled with**
   ``./sgp40_voc_index_example_usage`` **.**

Output:

::

    initialization successful
    VOC Index: 0
    Temperature: 39.128degC
    Relative Humidity: 29.695%RH
    VOC Index: 0
    Temperature: 39.086degC
    Relative Humidity: 29.687%RH
    VOC Index: 0
    Temperature: 39.106degC
    Relative Humidity: 29.647%RH
    VOC Index: 0
    Temperature: 39.076degC
    Relative Humidity: 29.714%RH
    VOC Index: 0
    Temperature: 39.068degC
    Relative Humidity: 29.615%RH
    VOC Index: 0
    Temperature: 39.082degC
    Relative Humidity: 29.674%RH
    ...

Troubleshooting
---------------

Initialization failed
~~~~~~~~~~~~~~~~~~~~~

-  Check if you connected the sensor correctly, so all cables are fully
   pluged in and connected to the correct header.
-  Check if you have enabled I2C on the Raspberry Pi. For this redo the
   steps on "Enable the I2C interface in the Raspberry Pi config" in the
   guide above.

.. |RaspberryPi Pinout| image:: ./images/GPIO-Pinout-Diagram-2.png

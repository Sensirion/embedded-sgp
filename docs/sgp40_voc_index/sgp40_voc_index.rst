This application note will provide you with an introduction to the SGP40 VOC
Index driver bundle.

Overview
========

This bundle combines Sensirion's SGP40 and SHTC3 sensors to provide a humidity
compensated sensor signal. It further embeds a VOC algorithm that
post-processes the raw sensor output into the resulting VOC Index.

Hardware setup
==============

The hardware setup uses a single I2C bus where both sensors are connected to
the micro controller unit (MCU). Connect the sensors according to the
datasheet. Remember to add pull-up resistors for SCL and SDA (10kOhm).

.. image:: ./images/hardware-setup.png
    :align: center
    :width: 50%


Getting Started
===============


Copy files to your project
--------------------------

#. Copy all SGP driver files (.c and .h) into your software project folder.
#. Make sure all files are added to your IDE.

.. note::
   Include new image

.. image:: copy_files.png


Adapt *sensirion_arch_config.h* for your platform
-------------------------------------------------

You may need to adapt *sensirion_arch_config.h* if your compiler doesn't
support C99 and thus does not provide `stdint.h` and `stdlib.h`.


Implement *sensirion_hw_i2c_implementation.c*
---------------------------------------------
To use your I2C hardware the file
*common/hw_i2c/sensirion_hw_i2c_implementation.c* needs to be completed. In it
you find the following functions where all parts marked with “// IMPLEMENT”
have to be replaced with code performing the necessary setup described here.

Alternatively you can use a sample implementation from
*common/hw_i2c/sample-implementations* and override it.

I2C functions to implement
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: sensirion_i2c_init
.. doxygenfunction:: sensirion_i2c_release
.. doxygenfunction:: sensirion_i2c_read
.. doxygenfunction:: sensirion_i2c_write
.. doxygenfunction:: sensirion_sleep_usec
.. doxygenfunction:: sensirion_i2c_select_bus


Measure IAQ (VOC index) and signal values
-----------------------------------------

The example *sgp40_voc_index_example_usage.c* gives a starting point on how to
use the sensors.

The SGP40 driver provides the following functions:

.. doxygenfunction:: sensirion_init_sensors
.. doxygenfunction:: sensirion_measure_voc_index
.. doxygenfunction:: sensirion_measure_voc_index_with_rh_t

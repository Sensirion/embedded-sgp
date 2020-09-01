Getting Started On a Custom Platform
====================================

Copy files to your project
--------------------------

 1. Copy all top level source files (.c and .h) into your software project folder.
 2. Copy *hw_i2c/sensirion_hw_i2c_implementation.c* to your project as well.
    You'll need to adapt it to your platform.
    See :ref:`I2C implementation`.
 3. Make sure all files are added to your IDE.

Adapt *sensirion_arch_config.h* for your platform
-------------------------------------------------

You may need to adapt *sensirion_arch_config.h* if your compiler doesn't
support C99 and thus does not provide `stdint.h` and `stdlib.h`.


.. _I2C implementation:

Implement *sensirion_hw_i2c_implementation.c*
---------------------------------------------
To use your I2C hardware the file *hw_i2c/sensirion_hw_i2c_implementation.c*
needs to be completed. In it you find the following functions where all parts
marked with “// IMPLEMENT” have to be replaced with code performing the
necessary setup described here.

Alternatively you can use a sample implementation from
*hw_i2c/sample-implementations/* and override it.

I2C functions to implement
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. doxygenfunction:: sensirion_i2c_init
.. doxygenfunction:: sensirion_i2c_release
.. doxygenfunction:: sensirion_i2c_read
.. doxygenfunction:: sensirion_i2c_write
.. doxygenfunction:: sensirion_sleep_usec
.. doxygenfunction:: sensirion_i2c_select_bus

Run *sgp40_voc_index_example_usage.c*
-------------------------------------

If your platform supports the ``printf`` function just run the examples and you
should see the following messages:

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
    ...

If your platform doesn't support ``printf`` remove the print statements and
observe the measurement values in your debugger.

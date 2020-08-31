Getting Started On a Custom Platform
====================================

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

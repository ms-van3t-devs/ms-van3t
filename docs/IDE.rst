=====================
Working with an IDE
=====================

Although not necessarily required, you can also configure an IDE in order to more comfortably work with ms-van3t.

Qt-Creator
===========

The suggested IDE, which has also been used for the development of ms-van3t, is *QtCreator*.

You can find all the instructions for setting up QtCreator with ns-3 (and the same applies to ms-van3t, as it is based on ns-3) on the `official ns-3 Wiki <https://www.nsnam.org/wiki/HOWTO_configure_QtCreator_with_ns-3>`_.

QtCreator can be installed on Debian/Ubuntu with:

.. code-block:: bash

   sudo apt install qtcreator

You need also to install the `libclang-common-8-dev` package (the command for Debian/Ubuntu is reported below):

.. code-block:: bash

   sudo apt install libclang-common-8-dev

Not installing `libclang-common-8-dev` may result in QtCreator wrongly highlighting several errors and not recognizing some types, when opening any source or header file, even if the code compiles correctly.

**Important**: if `libclang-common-8-dev` is not available, you can try installing a newer version. For example, on Ubuntu 22, we verified that `libclang-common-15-dev` works well too.

CLion
===========
If using ms-van3t in Windows with WSL refer to this guide --> https://gabrielcarvfer.github.io/NS3/installation/clion .
For Ubuntu, you may follow these steps:

1) After opening CLion, go to *File > Open..* and select the path/to/ms-van3t/ns-3-dev/ like in the picture.

   .. image:: CLion_1.png

2) Click on trust project.

   .. image:: CLion_2.png

3) Leave the default toolchain and click Next

   .. image:: CLion_3.png

4) Setup the CMake configuration as seen in the picture, you may change the build folder name if desired. Recommended Cmake Options (change debug for a different desired build type):

   .. code-block:: bash

      -G Ninja -DCMAKE_BUILD_TYPE=debug -Wall -DNS3_WARNINGS_AS_ERRORS=OFF -DNS3_EXAMPLES=ON 

   .. image:: CLion_4.png

5) Go to *File* and click on *Reload CMake Project*

6) Select the desired target, either libautomotive for building the entire module or a specific example target (e.g., v2i-areaSpeedAdvisor-80211p), and click build.

   .. image:: CLion_5.png

7) Before running a given target go to *Edit configurations*.

   .. image:: CLion_6.png

Enter the path/to/ms-van3t/ns-3-dev as *Working directory* and desired command-line arguments as *Program Arguments*. 

   .. image:: CLion_7.png

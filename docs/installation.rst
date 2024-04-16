============
Installation
============

.. contents:: Table of Contents
    :local:

Installing SUMO
===============

Install SUMO following the guide at `SUMO Downloads <https://sumo.dlr.de/wiki/Downloads>`_. 

For Linux systems, you can use these commands:

.. code-block:: bash

   sudo add-apt-repository ppa:sumo/stable  
   sudo apt update  
   sudo apt install sumo sumo-tools sumo-doc

Be careful: future updates of SUMO which are not ensured to work with this script (that are tested with any version from **v-1.6.0** to **v-1.18.0**).

You can test SUMO by opening a terminal and running "sumo-gui".

Possible Problems
-----------------

You may get the following error when running SUMO:

.. code-block:: bash

   "sumo-gui: symbol lookup error: /usr/lib/libgdal.so.26: undefined symbol: GEOSMakeValid_r"

To solve it, remove all the reference to GEOS inside /usr/local/lib/ (do NOT do it if you need the GEOS library):

.. code-block:: bash

   sudo rm /usr/local/lib/libgeos*

Cloning the Repository
======================

Clone this repository in your pc:

.. code-block:: bash

   git clone https://github.com/marcomali/ms-van3t

Installing Dependencies
=======================

Run, from this repository either:

.. code-block:: bash

   ./sandbox_builder.sh install-dependencies

If this is the first time you install ns-3 or ms-van3t on your system, or

.. code-block:: bash

   ./sandbox_builder.sh

If this is **not** the first time you install ns-3.

Configuring and Building ns3
============================

Configure `ns3` to build the framework with:

.. code-block:: bash

    <ns3-folder>./ns3 configure --build-profile=optimized --enable-examples --enable-tests --disable-python

The usage of the optimized profile allows to speed up the simulation time. This command should be launched from inside the `ns-3-dev` folder.

Important: If you are compiling ms-van3t on Ubuntu 22.04 LTS or later, you need to specify, when calling `./ns3 configure`, also the ``--disable-werror`` flag.

Build ns3:

.. code-block:: bash

   ./ns3 build

Important Notes
---------------

``src/automotive/`` contains all the application related files and all the source code implementing the ETSI ITS-G5 stack for vehicular communications. Inside `sumo_files_v2v_map` you can find the SUMO map and trace for the V2V sample application, while inside `sumo_files_v2i_map` you can find the SUMO map and trace for the V2I sample application. Similarly you can find the SUMO map and trace for the Traffic Manager sample application inside `sumo_files_v2i_TM_map` and the ones for the Emergency Vehicle Warning inside `sumo_files_v2i_EVW_map`

``src/traci/`` and `src/traci-applications/` contain instead all the logic to link ns-3 and SUMO. 

``src/cv2x/`` contains the model for C-V2X in transmission mode 4.

The user is also encouraged to use the ``sumo_files_v2v_map`` and ``sumo_files_v2i_map`` folders to save there the SUMO-related files for their own applications.

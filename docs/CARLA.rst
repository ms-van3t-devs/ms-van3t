========================
ms-van3t-CARLA extension
========================

In addition to SUMO and GPS traces ms-van3t supports the use of CARLA for mobility and sensor perception simulation. This extension leverages the `OpenCDA framework <https://github.com/ucla-mobility/OpenCDA>`_ to develop an LDM module and extend the gRPC adapter devised `here <https://github.com/veins/veins_carla>`_ to be able to extract not only localization information from CARLA but also perception information from the LDM module. The developed client module on ns-3 queries the information to use it for the mobility of each of the ns-3 simulated nodes and to update the LDM module with all perception data sent over the simulated vehicular network. 

System requirements
===================

We highly recommend running ms-van3t-CARLA on Ubuntu 20.04 (used for developing the framework) or Ubuntu 18.04, Ubuntu 22.04 is not officially supported by CARLA. If both CARLA and OpenCDA need to be installed we recommend at least 35GB of free space on your system. For smooth execution of simulation (especially if AI/ML models are leveraged for the perception simulation) we recommend using a GPU with at least 8GB of memory.
The version of CARLA supported by ms-van3t-CARLA is CARLA 0.9.12.

Installing ms-van3t-CARLA
=========================

To enable ms-van3t-CARLA, after following the steps detailed above to build the project, from inside the ``ns-3-dev`` folder execute the ``switch_ms-van3t-CARLA.sh`` script. 
The script will try to find the path of your CARLA and OpenCDA installation to be defined in the ``CARLA-OpenCDA.conf`` file. If the user already has an installation of either CARLA or OpenCDA they should specify the path to the installation together with the path to their Python environment in the following way: 

.. code-block:: bash

  CARLA_HOME=/path/to/CARLA_0.9.12

  OpenCDA_HOME=/path/to/OpenCDA

  Python_Interpreter=/path/to/anaconda3/envs/msvan3t_carla/bin/python3

In case this is the first time using either CARLA or OpenCDA the script will install them prompting for confirmation in each case. It is highly recommended to install OpenCDA with conda.
Once the script finishes its execution the user should build the project again with ``./ns3 build``.

ms-van3t-CARLA examples
=======================

Two examples leveraging CARLA are provided, showcasing how to use the extension with both IEEE 802.11p and NR-V2X as access technologies.
To run the provided examples: 

``./ns3 run "v2v-carla-80211p"`` or  ``./ns3 run "v2v-carla-nrv2x"`` 

 For further description of the modules provided in this extension please refer to our paper `here <https://www.eurecom.fr/publication/7556/download/comsys-publi-7556.pdf>`_.

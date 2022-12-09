.. _NrV2xExamples:

.. _cttcNrV2xDemoSimple:

cttc-nr-v2x-demo-simple.cc
==========================

This is a simplest example describing how to setup an NR sidelink out-of-coverage
simulation using the 3GPP channel model from TR 37.885. This example simulates a
simple topology consist of 2 UEs placed 20 m apart, where UE-1 transmits,
and UE-2 receives.

With the default configuration, the example will create a flow that will
go through a subband or bandwidth part. For that,
specifically, one band with a single CC, and one bandwidth part is used.

The example will print on-screen the average Packet Inter-Reception (PIR)
type 2 computed as defined in TR 37.885. The code to compute the PIR is embedded
in the example, i.e., it does not use `V2xKpi` class to compute PIR. Thus, it
serves as an example to show how one could compute the PIR without using
the V2X KPI management framework. On the other hand, it also demonstrates the 
use of V2X KPI management framework to save MAC and PHY layer
traces in a SQLITE database using ns-3 stats module. Moreover, since there
is only one transmitter in the scenario, sensing is by default not enabled.

.. _nrV2xWestToEastHighway:

nr-v2x-west-to-east-highway.cc
==============================

This example setups an NR Sidelink broadcast out-of-coverage simulation
using the 3GPP V2V highway channel model from TR 37.885. It simulates a
configurable highway topology consisting of type 2 vehicular UEs (see TR 37.885 sec 6.1.2)
in each lane, which travels from west to east. When it comes to the number of
transmitting and receiving vehicular UEs, it allows two configurations:

- To make all the vehicular UEs to transmit and receive during a simulation.

- To make a middle vehicular UE per lane the transmitter, and rest of the
  vehicular UEs the receivers. In this configuration, there could be only odd
  number of vehicles per lane.
 
Note, it does not limit the number of lanes or number of  vehicular UEs per
lane. With the default configuration, it uses one band with a single CC, and
one bandwidth part.

Moreover, this example fully utilizes the V2X KPI management framework and saves
RLC, MAC, PHY layer traces in a SQLITE database using ns-3 stats module. At the
end of the simulation, using the KPI management framework it computes the
following V2X KPIs:

- Average PIR delay for a fixed range of 200 m

- Average PRR for a fixed range of 200 m

- Throughput (by considering all the links)

- Simultaneous PSSCH Tx from MAC

- PSSCH TB corruption

These KPIs are written in the same database where traces are written. This example,
also provides an option to generate GNU plot scripts to plot initial positions
of the vehicles in the scenario, and also to generate a GIF to see the mobility
of the vehicles.

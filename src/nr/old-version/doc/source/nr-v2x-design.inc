Overview
========
The extension to support NR V2X functionalities involved modifications at
all layers of the protocol stack, from the ``NAS`` and down to the ``PHY`` layer.
One of the most important changes towards the implementation of NR V2X communications,
compared to the typical cellular communications available in 5G-LENA simulator,
is the introduction of sidelink, i.e., direct vehicle-to-vehicle communications.
For that, the bearer establishment and ``RRC`` layer have been fully updated
according to NR V2X ``RRC`` specification in TS 38.331. Also, ``MAC`` and ``PHY``
layers have been redesigned to implement NR V2X Mode 2 procedures using sensing-based
Semi Persistent Scheduling (SPS), as described in Section III-B of [cttc-nr-v2x]_,
according to TS 38.321 for the MAC layer, and TS 38.211 and TS 38.212 for the PHY layer. 
In the simulator, we focus on NR V2X Mode 2 for out-of-coverage
scenarios with broadcast communications and therefore, for the moment only blind
retransmissions (i.e., without sidelink HARQ feedback) are considered. In
Table :ref:`tab-nr-v2x-models`, we detail the features and functionalities that
are available in the developed NR V2X system-level simulator.
The features listed for our simulator are those included in the first release, which allow
the evaluation of a full NR V2X system with a subset of NR V2X features.


.. only:: latex

    .. raw:: latex
      
        \clearpage


.. _tab-nr-v2x-models:

.. table:: NR V2X models
   :class: longtable

   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   |                                |                                                  **NR V2X**                                                                            |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Frame structure                | TDD NR-compliant frame structure with slots and OFDM symbols                                                                           |
   |                                | of numerology-dependent length [TS38300]_:                                                                                             |
   |                                |                                                                                                                                        |
   |                                | - Frame: 10 ms, subframe: 1 ms.                                                                                                        |
   |                                | - Each subframe has :math:`2^{\mu}` slots (associated to :math:`15\times2^{\mu}` kHz SCS).                                             |
   |                                | - Numerologies :math:`{\mu}` = 0, 1, 2, 3, 4 are supported.                                                                            |
   |                                | - Each slot is composed of 14 OFDM symbols.                                                                                            |
   |                                |                                                                                                                                        |
   |                                | Support for multiple bandwidth parts [cttc-nr]_:                                                                                       |
   |                                |                                                                                                                                        |
   |                                | - More than one BWP can be configured for sidelink                                                                                     |
   |                                | - Each BWP can have pre-configured multiple sidelink resource pools, but only one pool can be active at time.                          |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Duplex mode                    | TDD                                                                                                                                    |
   |                                |                                                                                                                                        |
   |                                | - The TDD pattern is flexible in length and composition, and can include downlink-only slots, uplink-only slots, or flexible slots     |
   |                                |   (in which downlink and uplink transmissions can occur). An example of TDD pattern is [D F U U U].                                    |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Sidelink Resource pool         | - Sidelink transmission is only allowed in uplink-only slots, and whether an uplink slot is available for sidelink or not is specified |
   |                                |   through the SL bitmap. An example of the SL bitmap is [1 1 1 1 1 1 0 0 0], where 1 indicates that a slot is available for sidelink   |
   |                                |   and 0 that it is not.                                                                                                                |
   |                                | - Within the uplink slots available for sidelink, the symbols available for sidelink are RRC pre-configured and our default structure  |
   |                                |   is as follows: PSCCH can occupy 1 or 2 starting symbols, and depending on the PSCCH allocation, 2nd to 13th or 3rd to 13th symbols   |
   |                                |   are available for PSSCH, and the 14th symbol is left empty as a guard period.                                                        |
   |                                | - In the frequency domain, RRC pre-configures the subchannel size (in number of RBs), and as per this configured size, divides the     |
   |                                |   available bandwidth into a number of available subchannels.                                                                          |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Sidelink data/control channels | - PSSCH and PSCCH are multiplexed in time.                                                                                             |
   |                                | - PSSCH and PSCCH are sent and received quasi-omnidirectionally at the UEs.                                                            |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Error models                   | - NR PHY abstraction for PSSCH and PSCCH channels [cttc-phy-abstraction]_ including support for MCS Table1 and Table2 [TS38214]_.      |
   |                                | - MCS LDPC coding and block segmentation [TS38212]_.                                                                                   |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Modulation                     | OFDM                                                                                                                                   |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Channel Coding                 | LDPC                                                                                                                                   |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | MCS                            | QPSK, 16-QAM, 64-QAM, 256-QAM                                                                                                          |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | HARQ                           | NR PHY abstraction for HARQ includes support for HARQ-IR and HARQ-CC                                                                   |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Retransmissions                | Blind retransmissions, including up to a pre-configured number with retransmission combining.                                          |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Resource allocation            | Sensing-based and random resource selections are supported.                                                                            |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Link adaptation                | Fixed MCS                                                                                                                              |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Antenna models                 | 3GPP-compliant [TR38802]_:                                                                                                             |
   |                                |                                                                                                                                        |
   |                                | - Antenna arrays: 1 uniform planar array per UE, :math:`M{\times}N` antenna elements, no polarization.                                 |
   |                                | - Antenna elements: isotropical and  directional radiation are supported.                                                              |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+
   | Channel models                 | 3GPP-compliant [TR37885]_, supporting Urban grid and Highway scenarios, in both sub 6 GHz and mmWave bands.                            |
   +--------------------------------+----------------------------------------------------------------------------------------------------------------------------------------+

Architecture
============

UE architectures
================
Figure :ref:`fig-ue-sl-data-plane` and Figure :ref:`fig-ue-sl-control-plane` show
the NR module UE model architecture in the data plane and the control plane,
respectively. For NR V2X implementation, these architectures have not been
changed; however, it is noticeable that we have created sidelink specific
Service Access Point (SAP) provider and user interfaces. This choice is motivated
by the fact that having separate sidelink SAPs makes it easier to extend the
sidelink functionalities without changing the NR or LTE specific SAPs. 


.. _fig-ue-sl-data-plane:

.. figure:: figures/nr-sl-arch-ue-data-plane.*
   :align: center
   :scale: 30%

   NR Sidelink UE data plane architecture
   

.. _fig-ue-sl-control-plane:

.. figure:: figures/nr-sl-arch-ue-ctrl-plane.*
   :align: center
   :scale: 30%

   NR Sidelink UE control plane architecture


NAS
===
The establishment and management of sessions occur at the highest layer on the
control plane, the ``NAS``. The current functionalities of the ``NAS`` layer
involve establishment of ``EPS`` bearers, multiplexing uplink data packets coming
from the upper layers into the appropriate ``EPS`` bearer by using the ``TFT`` classifier.
A ``TFT`` defines the rules for mapping IP packets to the right bearer based on
IP addresses, ports, and type of service parameters.


For sidelink, the modifications are similar to the ones introduced in [nist-d2d]_.
Specifically, ``NAS`` now supports the activation of sidelink bearers, mapping of
IP packets to the sidelink bearers based only on the IP destination address of
the packets, and the transmission/reception of packets in ``NAS`` OFF state to
support out-of-coverage scenarios.


RRC
===
The ``RRC`` is the control plane protocol in charge of setting important parameters
for the session. The modifications in the ``RRC`` include the creation of the
sidelink bearers upon receiving a notification from ``NAS``, and the pre-configuration
of UEs in an out-of-coverage scenario. As mentioned earlier, the model currently
focuses on the broadcast communication, therefore, as per the standard, it supports
the creation of uni-directional sidelink radio bearers [TS38322]_.

Regarding the UEs' pre-configuration, the model implements all RRC Information Element (IE)
needed to configure a UE [TS38331]_. This configuration is of key importance to
perform sidelink communication when the gNB is absent. These IEs are mainly used
for two purposes. The first is to configure the UE's ``PHY`` layer parameters,
e.g., numerology, symbols per slot, bandwidth, and TDD pattern. The second is to
provide the sidelink resource pool(s) information to MAC and PHY layers. It is
also worth mentioning that the model allows the configuration of multiple BWPs for
sidelink, where for each BWP, more than one resource pool can be configured
through ``RRC``. We note that, in spite of supporting multiple resource pools
per BWP, only one pool can be active at one time [TS38321]_. Moreover, differently
from the standard, which uses separate pools for transmission and reception [TS38331]_,
our model uses the same active pool for both. 

Finally, for the sake of simplifying the implementation we have taken some
assumptions for the configuration of a TDD pattern, sidelink bitmap and the
resource reservation period in the simulator. In particular, one should
always respect the following conditions while configuring these parameters.

 #. The size of a sidelink bitmap should always be a multiple of the number of
    UL slots in a TDD pattern because if its size is not multiple of the number
    of UL slots, we can not construct the physical bitmap which could be applied
    repeatedly throughout a simulation.

 #. The resultant physical bitmap size should always be the multiple of resource
    reservation period used in the simulation. This is adopted to guarantee that
    once a slot is selected for sidelink communications, it is available for
    sidelink after the resource reservation period in ms and the sensing-based
    resource selection mechanism of neighboring UEs can operate adequately,
    which receive the resource reservation period of other UEs in SCI stage 1.

.. _sec-pdcp:

PDCP
====
The changes introduced in ``PDCP`` layer are in line with LTE sidelink [nist-d2d]_.
In particular, when it comes to sidelink, it is no longer possible to uniquely
identify a logical channel only based on its LCID. With sidelink communications,
UEs independently assign the LCIDs to logical channels for each
destination (i.e., Layer 2 group ID) to which they are transmitting. Thus, it is
impossible for UEs to identify the packets if multiple transmitting UEs select
the same LCID for the same group. To solve this, two more identifiers, i.e.,
source Layer-2 ID and destination Layer-2 ID, are included to identify the
transmitting UE [TS38300]_.



RLC
===
The ``RLC`` layer in the simulator already supports the so called Unacknowledged (UM),
which is the RLC mode used for sidelink broadcast communications. The only
modifications made to the ``RLC`` layer are identical to the ``PDCP`` layer.


MAC
===
The UE's ``MAC`` layer has been extensively modified to transmit and receive
sidelink transmissions. In the following, we explain these procedures in detail.

MAC transmitting procedure
##########################
In out-of-coverage scenarios, UEs are required to perform the autonomous resource
selection following Mode 2, which could be based on sensing-based or random
selection procedures, as [TS38321]_, respectively. The first significant
modification introduced in
this respect is the new ``MAC`` scheduler interface. This interface allows the
implementation of sidelink UE-specific schedulers, which could assign resources
following specific strategies, e.g., fixed MCS, adaptive MCS based on Channel
Sensing Information (CSI),
etc. The UE ``MAC`` layer is extended to provide all the information needed by
a scheduler to perform resource selection. For example, the information related
to all the LCs of destinations the UE is interested in transmitting to,
the total number of available subchannels, :math:`N_{\max\_\text{reserve}}`, the
maximum number of PSSCH transmissions, and most importantly, the ``RLC`` sidelink
Buffer Status Reports (BSRs) of each LC that indicate how much sidelink traffic
needs to be transmitted. The second important addition is the buffering of the
sensing data reported by the UE's ``PHY`` layer. This buffer behaves like a
sensing window at the time of a sensing-based resource selection. It contains
the sensing information for the interval [:math:`n-T_\text{0}, n-T_\text{proc,0}`),
where :math:`n` is the slot at which the resource selection is triggered, and
:math:`T_\text{0}` is configured by the ``RRC`` while :math:`T_\text{proc,0}`
is a ``MAC`` layer parameter. In what follows we will dive into the details of
UE's ``MAC`` layer operation to perform sensing-based resource selection.

At slot :math:`n`, when a resource selection is triggered for a destination, the
``MAC`` layer draws a random counter known as Sidelink Resource Reselection
Counter (SLRRC) based on the user configured Resource Reservation Period (PRSVP)
value, which is used to compute :math:`C_\text{resel}`. Then, for an active
pool configured by the ``RRC``, it computes the candidate resources
(i.e., available slots) for sidelink transmission based on the selection window
parameters, :math:`T_\text{1}`, :math:`T_\text{2,min}`, and :math:`T_\text{2}`.
Since the final resources must be selected based on sensing information, the MAC
follows the standard procedure described in Section III-B1 of [cttc-nr-v2x]_, to
filter out the resources from the total available ones, which could be occupied
by the other transmitting UEs. Once the filtered candidate resources' list is
ready, the ``MAC`` layer forwards it to the scheduler. Our model provides a
sample scheduler, which as per the standard [TS38331]_, first randomly selects a
number of slots, i.e., :math:`N_\text{selected}`, for sidelink transmissions.
The number of :math:`N_\text{selected}` slots depends on the number of slots that
are available in the filtered list and the maximum number of configured PSSCH
transmissions. If :math:`K` denotes the total number of available slots, and
:math:`N_\text{PSSCH,maxTx}` is the maximum number of PSSCH configured
transmissions, then:

.. math::

    N_{selected}=\begin{Bmatrix}
                   N_\text{PSSCH,maxTx} & \text{if}\ K \geq N_\text{PSSCH,maxTx}\\ 
                   K, & \text{otherwise}
                 \end{Bmatrix}


After randomly selecting the required number of slots, it then randomly the selects
the required number of contiguous subchannels computed using a fixed MCS
strategy. Note that, adaptive MCS strategy makes sense in unicast and
groupcast communications when the CSI from the receiving UE can be acquired
through the PSSCH. As for this initial implementation, we have focused on broadcast communications,
without PSFCH, the fixed MCS strategy is the adequate one. However, the
implemented scheduler interface is general enough to accommodate more sophisticated
schedulers. After selecting the :math:`N_\text{selected}` number of slots, the scheduler
computes the TB size using the fixed MCS by taking into account the
BSR of a LC, and the 5 bytes overhead of 2nd-stage-SCI, which needs
to be multiplexed with data. Finally, it prepares a sidelink allocation valid for
the first resource reservation period deciding also aspects like which slots from
the :math:`N_\text{selected}` have to carry the 1st-stage-SCI, the New Data
Indicator (NDI), and the Redundancy Version (RV) number of each slot. The UE
``MAC`` layer, upon receiving this allocation, creates the SPS grants based on the
configured value of PRSVP and the already drawn counters, i.e., SLRRC and
:math:`C_\text{resel}`. After using these grants for a number of transmissions
equal to the SLRRC, a resource reselection is triggered. That is, once SLRRC
reaches zero, the UE either keeps the previous selection or selects new resources
based on the pre-configured probability of resource keep. Finally, if
:math:`C_\text{resel}` reaches zero, the resource reselection is triggered,
independently of this probability. As already discussed, our model also supports
a random resource selection. The only difference between the two approaches is
that the random resource selection procedure does not filter the slots from the
available ones before giving the list to the scheduler, so that the sensing
information is not used.


Before forwarding the sidelink packets to the ``PHY`` layer, a check is
performed at the beginning of each slot to ensure the availability of a valid
grant for that slot. If there is, the ``MAC`` layer prepares two packet bursts,
one for the 1st-stage-SCI and the second for the 2nd-stage-SCI plus data, and
assigns a HARQ process ID to the data packet. It also saves this data packet
into a HARQ buffer if blind retransmissions are configured. We note that the
model allows to configure multiple (no limit for research purposes) sidelink/HARQ
processes to allow continuous flow of data. After this, both packet bursts are
forwarded to the lower layer. Upon receiving these packet bursts, the ``PHY``,
places them in a queue to be transmitted on the configured PSCCH and PSSCH symbols.


MAC receiving procedure
#######################

The UE's ``MAC`` layer, upon receiving the PSSCH packet burst from the ``PHY``,
first retrieves the 2nd-stage-SCI to read the source Layer-2 ID and the
destination Layer-2 ID of the received packet. As mentioned in section :ref:`sec-pdcp`,
these identifiers are used to map the received packet to its logical channel.
If a bearer for the received packet is already established, the data packet is
forwarded to the upper layers. Otherwise, the ``MAC`` asks the ``RRC`` to
establish the bearer for the reception. Once this is done, the packet is forwarded
to the upper layers.

PHY
===
Similarly to the ``MAC`` layer, the ``PHY`` functionality can also be divided
into transmitting and receiving procedures. In the following, we describe them
in detail.

PHY transmitting procedure
##########################
The 5G-LENA simulator accurately models (as per the standard) the
numerology-dependent slot and OFDM symbol granularity. The state-machine of the
``PHY`` layer is mainly determined by the definition of the concept of start slot
event and variable TTI. When the start slot event is triggered, the processing
follows a logical order that involves the MAC and then the scheduler, before
returning the control to the PHY. For sidelink, once the control gets back to the
``PHY``, the PHY checks if the ``MAC`` has provided an allocation for the current
slot. This allocation further consists of variable TTI allocations. The
variable TTI means that the number of allocated symbols to physical sidelink
channels (i.e., PSCCH and PSSCH) is variable, based on the sidelink configuration.
Upon finding the allocation for the slot, the ``PHY`` layer transmits PSCCH and
PSSCH PDU on their respective symbols whose duration depend on the configured
numerology and CP.


PHY receiving procedure
#######################
To receive the sidelink transmissions, one of the key enhancements of the ``PHY``
is the introduction of the ability to handle collisions/interference, also
introduced in [nist-d2d]_. The interference model available in the 5G-LENA
simulator was designed for a typical cellular communication. Its design assumes
that a UE is interested in transmitting or receiving only from its serving gNB,
and assumes no interference from the UEs served by the same gNB. Transmissions
from other gNBs/UEs are simply considered as interference. In case of sidelink,
especially in broadcast or groupcast, a UE is interested in transmitting to or
receiving from multiple surrounding UEs. In this context, UEs in out-of-coverage
scenarios or ``UE-selected'' mode can select the same (or overlapping) resources
because the allocation is uncoordinated. Therefore, to determine which packet
will be successfully decoded, the new implementation keeps track of the SINR
values for each sidelink transmission.

As described earlier, currently our model supports the transmission and reception
of timely multiplexed PSCCH and PSSCH. Thus, the ``PHY`` first receives signal(s)
(i.e., 1st-stage-SCI) transmitted over PSCCH. This signal is used for two purposes:
1) to measure the RSRP required for the sensing-based resource selection,
2) to retrieve the information about the possible PSSCH transmission and retransmissions.
The RSRP is computed using the 3 Resource Elements (REs) per RBs, carrying the
1st-stage-SCI, since the simulator does not explicitly include PSCCH DMRS. Moreover,
for the sensing-based resource selection, the ``PHY`` measures the RSRP of each
correctly decoded 1st-stage-SCI, from all the surrounding UEs. On the other hand,
after computing the RSRP, if it is from the transmitter of interest, it reads
the information encoded in the 1st-stage-SCI to receive the PSSCH transmission
and its possible retransmissions.


Concerning the error model used for the reception of PSCCH and PSSCH transmission,
we use the existing data plane error model in 5G-LENA [cttc-phy-abstraction]_,
since the MCS defined for PSSCH are the same as the ones defined for PDSCH/PUSCH.
Also, we adopt such an error model for the PSCCH, using MCS0.



Channel Models
==============
TR 37.885 [TR37885]_ defines the system-level evaluation methodology for 5G V2X use
cases, including the description and modeling of scenarios, deployment, mobility,
antenna, traffic, and channel models. For channel modeling, TR 37.885 extends the
geometry-based stochastic channel modeling framework introduced in TR 38.901 [TR38901]_
for typical cellular communications, by adding the possibility to model wireless
channel in vehicular environments and sidelink communications in which both the
transmitter and the receiver are in motion.

 
Two key scenarios are used for NR V2X evaluation:

  #. Urban grid, which targets urban environments with a grid of buildings and
     roads with four lanes (two in each direction) between the buildings, and
 
  #. Highway, which targets highway environments with a highway composed of a
     total of six lanes, considering three lanes in each opposite direction.

For each scenario, TR 37.885 specifies new channel condition models, propagation
models, and fast fading parameters capturing the characteristics of each environment. 

The developed ns-3 NR V2X module includes the channel and
antenna models for both V2X Urban grid and Highway scenarios, as defined
in the standard [v2x-channel-ns-3]_.


NR V2X KPI management framework
===============================
For the sake of computing KPIs for the NR V2X simulations, we have developed a
KPI Management framework, which is comprised on several APIs that can be
categorized as follows:

 #. The APIs responsible of listening to the specific ns-3 traces and store the
    information in a SQLITE database in the form of tables.
    
 #. The APIs responsible to read the SQLITE tables written by the category-1
    APIs, and compute V2X specific KPIs. These KPIs are then stored
    in the same database by creating new tables specific to each KPI. 
    
The table :ref:`tab-nr-v2x-kpis` lists all these APIs and their functionalities.

.. tabularcolumns:: |p{4.5cm}|c|p{8cm}|

.. _tab-nr-v2x-kpis:

.. table:: NR V2X KPI management framework APIs
   :class: longtable
  
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | Class Name               | Category | Functionalities                                                                   |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UeMacPscchTxOutputStats  |     1    | Listens and stores the                                                            |
   |                          |          | SlPscchScheduling trace of NrUeMac                                                |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UeMacPsschTxOutputStats  |     1    | Listens and stores the                                                            |
   |                          |          | SlPsschScheduling trace of NrUeMac                                                |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UePhyPscchRxOutputStats  |     1    | Listens and stores the                                                            |
   |                          |          | RxPscchTraceUe trace of NrSpectrumPhy                                             |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UePhyPsschRxOutputStats  |     1    | Listens and stores the                                                            |
   |                          |          | RxPsschTraceUe trace of NrSpectrumPhy                                             |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UeRlcRxOutputStats       |     1    | Listens and stores the                                                            |
   |                          |          | RxRlcPduWithTxRnti trace of NrUeMac                                               |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | UeToUePktTxRxOutputStats |     1    | Listen and store the application level                                            |
   |                          |          | traces of type TxWithSeqTsSize and RxWithSeqTsSize                                |
   |                          |          | into a database.                                                                  |
   +--------------------------+----------+-----------------------------------------------------------------------------------+
   | V2xKpi                   |     2    | This class reads the specific tables of a                                         |
   |                          |          | given DB written by category 1 APIs to compute                                    |
   |                          |          | the following V2X KPIs:                                                           |
   |                          |          |                                                                                   |
   |                          |          |  - Average Packet Inter-Reception (PIR) (range-based and not range based)         |
   |                          |          |                                                                                   |
   |                          |          |  - Average Average Packet Reception Ratio (PRR) (range-based and not range based) |
   |                          |          |                                                                                   |
   |                          |          |  - Throughput                                                                     |
   |                          |          |                                                                                   |
   |                          |          |  - Simultaneous PSSCH Transmissions                                               |
   |                          |          |                                                                                   |
   |                          |          |  - PSSCH TB RX stats                                                              |
   +--------------------------+----------+-----------------------------------------------------------------------------------+







.. [cttc-nr-v2x] Zoraze Ali, Sandra Lagen, Lorenza Giupponi and Richard Rouil, "3GPP NR V2X Mode 2: Overview, Models and System-Level Evaluation," in IEEE Access, vol. 9, pp. 89554-89579, 2021.

.. [cttc-nr] Natale Patriciello, Sandra Lagen, Biljana Bojovic, and Lorenza Giupponi, “An E2E simulatorfor 5G NR networks,” Simulation Modelling Practice and Theory, vol. 96, p. 101933, 2019.

.. [cttc-phy-abstraction] Sandra Lagen, K. Wanuga, H. Elkotby, S. Goyal, Natale Patriciello, and Lorenza Giupponi, “New Radio Physical Layer Abstraction for System-Level Simulations of 5G Networks,” in IEEE International Conference on Communications, June 2020.

.. [nist-d2d] Richard Rouil, F. J. Cintrón, A. B. Mosbah, and S. Gamboa, “Implementation and Validation of an LTE D2D Model for ns-3,” in Workshop on Ns-3, 2017.

.. [v2x-channel-ns-3] Tommaso Zugno, Matteo Drago, Sandra Lagen, Zoraze Ali, and Michele Zorzi, “Extending the ns-3  spatial  channel  model  for  vehicular  scenarios,”  inWorkshop  on  Ns-3,2021

.. [TR38802] 3GPP TR 38.802, Study on New Radio (NR) Access Technology; Physical Layer Aspects (Release 14), v14.2.0, Sep. 2017.

.. [TR37885] 3GPP TR 37.885, Study on Evaluation Methodology of New Vehicle-to-Everything  (V2X) Use Cases for LTE and NR (Rel. 15), V15.3.0, Jun.2019.

.. [TS38321] 3GPP TS 38.321, TSG RAN; NR; Medium Access Control (MAC) Protocol Specification, Release 15, v16.3.0, Jan. 2021.

.. [TS38322] 3GPP TS 38.322, TSG RAN; NR; Radio Link Control (RLC) protocol specification, Release 16, v16.2.0, Jan. 2021.

.. [TR38901] 3GPP TR 38.901, Study on Channel Model for Frequencies from 0.5 to 100 GHz (Rel. 15), V15.0.0, Jun. 2019.

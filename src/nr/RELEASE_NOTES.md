5G-LENA Release Notes                         {#releasenotes}
=====================

This file contains release notes for the NR module (most recent releases first).  

All of the ns-3 documentation is accessible from the ns-3 website:
http://www.nsnam.org including tutorials: http://www.nsnam.org/tutorials.html

Consult the file CHANGES.md for more detailed information about changed
API and behavior across releases.



Release NR-v2.2
----------------

Availability
------------
Available since June 03, 2022

Supported platforms
-------------------
The supported platforms are the same as for the NR-v2.1 release, except that 
the recommended ns-3 release is ns-3.36.1.

Important news
--------------
This module can be updated with the usual

```
$ git pull
```

command.

Remember to follow the instructions from the README.md file, i.e., to checkout 
the correct release branch of both, ns-3 and the NR module. E.g., the NR module 
Release 2.1 is compatible with the ns-3.36 release branch, while the NR module 
Release 2.2 is compatible with the ns-3.36.1 release branch.

The information about compatibility with the corresponding ns-3 release branch 
is stated in this (RELEASE_NOTES.md) document in the "Supported platforms" 
section for each NR release (starting from the NR Release 1.3).

New user-visible features (old first)
-------------------------

None.

Bugs fixed
----------

None.

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr



Release NR-v2.1
---------------

Availability
------------
Available since May 06, 2022

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note: not all features available on all platforms):
- g++-9 or later
- clang-8 or later
- (macOS only) Xcode 10.1 or later

This release has been tested on the following platforms:
- ArchLinux with g++-9 and 10 clang-8, 9, 10, and 11

Recommended ns-3 release: ns-3.36.

Important news
--------------
This module can be updated with the usual

```
$ git pull
```

command. We hope you will have fun and good times in using our module!

New user-visible features (old first)
-------------------------
- Added new distance-based 3GPP spectrum propagation loss model
- Added the Get function to obtain the pointer to PHY traces
- Added scenario with UE mobility in `HexagonalGridScenarioHelper` class
- Added option to set random antenna height in centrain percentage of UEs in 
`HexagonalGriScenarioHelper`
- Extended `HexagonalGridScenarioHelper` to allow installing the hexagonal scenario 
with the 4th and the 5th ring (needed for the wrap around calibration)
- Added new attribute to `NrMacSchedulerNs` to allow enabling or disabling HARQ ReTx
- `NrRadioEnvironmentMapHelper` is extended to provide the progress report at std::
cout, i.e., 1%, 10%, ..., 100%, and provides the estimation of the time left
- Added CQI column in RxPacketTraceUe
- Added SIR calculation and plot in `NrRadioEnvironmentMapHelper`
- Added CellScan algorithm based on azimuth and zenith in class called 
`CellScanBeamformingAzimuthZenith` in `ideal-beamforming-algorithm.h/cc`
- Added new trace for reporting DL SINR CTRL 
- Extended and improved RLC and PDCP traces to include simple traces per RX/TX 
side, and combined/merged end-to-end traces.



Bugs fixed
----------
- Fixed how to consider RLC overhead when updating the TX queues in MAC scheduler
- Fixed attachment for loop in `lena-lte-comparison` example 
- Fixed wrong assignation of NetDeviceContainer vector for REM in `lena-lte-comparison` 
example
- Fixed REM sector index bug in lena-lte-comparison.cc example
- Fix the buffer size calculation in `NrMacSchedulerLcg` to consider correctly the 
RLC overhead
- Improved formatting of the RLC/PDCP traces

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr



Release NR-v2.0
--------------

Availability
------------
Available since April 21, 2022

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note: not all features available on all platforms):
- g++-9 or later
- clang-8 or later
- (macOS only) Xcode 10.1 or later

This release has been tested on the following platforms:
- ArchLinux with g++-9 and 10 clang-8, 9, 10, and 11

Recommended ns-3 release: ns-3.36. (If ns-3.36 is not available yet, use ns-3 
master branch.)

Important news
--------------
This module can be updated with the usual

```
$ git pull
```

command. We hope you will have fun and good times in using our module!

New user-visible features (old first)
-------------------------
- The NR module now supports DP-MIMO spatial multiplexing. The NR DP-MIMO model 
currently supports MIMO with 2 streams for the downlink, however, in the future, 
the MIMO model will be extended to support more streams and the operation in the 
uplink. 
The current, NR DP-MIMO model exploits dual-polarized antennas and their 
orthogonality to send the two data streams, by exploiting polarization diversity. 
The model does not rely on abstraction, as is it case with LTE MIMO, 
and thus can more properly model the propagation differences between the two 
streams and account for rank adaptation. So, one of the important new features 
that enters with this release along the NR DP-MIMO is the rank adaptation. 
The major modifications to support the DP-MIMO feature are inside the PHY and 
MAC layers of the NR module.


Bugs fixed
----------


Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr


Release NR-v1.3
---------------

Availability
------------
Available since April 7, 2022.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note: not all features available on all platforms):
- g++-9 or later
- clang-8 or later
- (macOS only) Xcode 10.1 or later

This release has been tested on the following platforms:
- ArchLinux with g++-9 and 10 clang-8, 9, 10, and 11

Recommended ns-3 release: ns-3.36.

Important news
--------------
In past, the NR module releases were not bound to any particular ns-3 release 
because we were trying to keep in sync with the latest advancements in ns-3-dev 
(master). Since ns-3 has had many disruptive API changes since its release 3.35 
towards the upcoming release 3.36, and it also changed the build system from waf to 
cmake, many 5G-LENA users have reported issues related to the unsuccessful 
compilation of NR v1.2 with the latest ns-3 master. 

Hence, starting from this NR release we decided to bind each NR release to 
the specific ns-3 release by recommending the ns-3 release with which has been 
tested the specific NR release.

Additionally, due to many accumulated changes in the NR module, that were waiting 
for the next ns-3 release, we have decided to split all these upcoming changes into 
several incremental releases to facilitate the 5G-LENA users to upgrade 
their code gradually.

As a first step towards that goal, we have created this NR release v1.3 to 
provide to the 5G-LENA users a version of the NR module that is compatible with 
the upcoming ns.3-36. 

(This release has been tested with ns-3-dev master with commit 4a98f050, and is 
expected to be fully compatible with ns-3.36).

This module can be updated with the usual

```
$ git pull
```

command. 

We hope you will have fun and good times in using our module!

New user-visible features (old first)
-------------------------
- Switch to cmake build system
- Upgrade nr to spectrum changes that entered with the MR 686
- Upgrade nr to lte changes that entered with the MR 810

Bugs fixed
----------

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr



Release NR-v1.2
--------------

Availability
------------
Available since June 4, 2021.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note: not all features available on all platforms):
- g++-7 or later
- clang-8 or later
- (macOS only) Xcode 10.1 or later

This release has been tested on the following platforms:
- ArchLinux with g++-7, 8, 9, and 10 clang-8, 9, 10, and 11

Important news
--------------
This module can be updated with the usual

```
$ git pull
```

command. We hope you will have fun and good times in using our module!

New user-visible features (old first)
-------------------------
- File Transfer Protocol (FTP) model 1 traffic model is included.

Bugs fixed
----------
- The computation of the effective SINR for the error modeling under HARQ-IR uses
now an updated formula that accounts for pure IR with no repetition of coding bits.
- There were cases in which multiple UEs could be assigned the same SRS offset value,
because the generation of the possible SRS offset was including multiple 0 values.
Now, the generation of the available values for the SRS offsets has been updated
to not contain multiple 0 values.
- Realistic beamforming algorithm with trigger event configured as delay update
uses the actual channel at SRS reception moment for real BF update with delay.
- There have been reported cases where an assert in PHY was triggered due to the
fact that the Allocation Statistics were not in accordance with the real allocation.
This happened because the `SlotAllocInfo` structure, and in particular the
`m_numSymAlloc` field, was not updated accurately when a UE didn’t get a
DCI (i.e., when the TBS is less than 7 bytes). Scheduler, now correcly updates
the `m_numSymAlloc` field and the `usedSym` variable in `NrMacSchedulerNs3::DoScheduleDlData`
and `NrMacSchedulerNs3::DoScheduleUlData` when DCI is not created.

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr


Release NR-v1.1
--------------

Availability
------------
Available since March 2, 2021.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note:  not all features available on all platforms):

- g++-4.9 or later
- Apple LLVM version 7.0.2 or later
- clang-3.3 or later

In addition, Python 2.7 (Python 2 series) or Python 3.4-3.8 (Python 3 series)

This release has been tested on the following platforms:
- ArchLinux with g++-9.2.1, clang-9.0.1, and Python 3.8.1
- Ubuntu 18.04 (64 bit) with g++-8.3.0 and Python 2.7.12/3.5.2

Important news
--------------
This module can be updated with the usual

```
$ git pull
```

command. We hope you will have fun and good times in using our module!

New user-visible features (old first)
-------------------------
- The scheduler can selectively leave particular RBG empty. This feature is
  called notching, and is used when multiple gNBs are collaborating to avoid
  interferences over a spectrum part.
- Added SRS allocation, transmission, and reception. Added an SRS message that
  takes 4 symbols (in the default configuration) within some periodicity (default
  at 80 slots). SRS are dynamically scheduled by the gNB (with an interface and
  an example specialized scheduler for it, `NrMacSchedulerSrsDefault`), and its
  allocation is signaled to the UE through a DCI. This is used by the UE to
  transmit SRS.
- `RealisticBeamformingAlgorithm` class is added. It implements a
  beamforming algorithm that determines the beamforming vectors of the transmitter 
  and the receiver based on the SINR SRS. 
- Uplink power control functionality implemented through the `NrUePowerControl`
  class, supporting UL power control for PUSCH, PUCCH, and SRS.
- IPV6 is now supported. That is, the end-to-end connections between the UEs
  and the remote hosts can be IPv4 or IPv6.

Bugs fixed
----------
- BeamManager called the function with the name "ChangeToOmniTx" of 3gpp
  antenna. This was causing that the CTRL was not being passed through 3gpp
  spectrum propagation model, but only through the propagation loss model.
- `GridScenarioHelper` fixes to correctly place nodes.

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr


Release NR-v1.0
--------------

Availability
------------
Available since Sept. 16, 2020.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note:  not all features available on all platforms):

- g++-4.9 or later
- Apple LLVM version 7.0.2 or later
- clang-3.3 or later

In addition, Python 2.7 (Python 2 series) or Python 3.4-3.8 (Python 3 series)

This release has been tested on the following platforms:
- ArchLinux with g++-9.2.1, clang-9.0.1, and Python 3.8.1
- Ubuntu 18.04 (64 bit) with g++-8.3.0 and Python 2.7.12/3.5.2

Important news
--------------
The v1.0 can now be installed in the ns-3-dev repository, or any ns-3 version
starting from ns-3.31. This module can be updated with the usual

```
$ git pull
```

command. We hope you will have fun and good times in using our module!

New user-visible features (old first)
-------------------------

- Renamed all mmwave- classes, tests, examples, helpers, to nr-. Renamed all
  the classes by replacing the prefix `MmWave` with `Nr`.
- Renamed the `Enb` part in `Gnb` (e.g., `NrEnbPhy` -> `NrGnbPhy`)
- Processing delays N0, N1, N2 are introduced as attributes of NrEnbPhy,
  respectively for the DL DCI delay, DL HARQ feedback delay, and UL DCI delay.
  The values K0, K1, K2 (definition in the 3GPP standard) are then calculated
  and communicated to the UE in the DCI. The N2Delay parameter replaces the old
  UlSchedDelay parameter.
- Removed PhyMacCommon. Its attributes are now divided among different
  classes. Please check CHANGES.md for the list.
- Separated NrEesmErrorModel in four different classes: NrEesmIrT1, NrEesmIrT2,
  NrEesmCcT1, NrEesmCcT2. These classes encapsulate the properties (harq method,
  table) that were an attribute of NrEesmErrorModel.
- Added the LENA error model. To be used only in conjunction with a OFDMA
  scheduler, and without beams. This error model has the same performance and
  values as the error model used in the lte/ module. The reference file is
  `lena-error-model.h`.
- Added the attribute `RbOverhead` to the NrGnbPhy and NrUePhy to set the
  bandwidth overhead to keep in consideration when calculating the number of
  usable RB. By default, it is set to 0.04, while in the previous versions the
  effect was like a value set to 0.0 (0.0 means that there are no guard bands,
  and the entire bandwidth is usable).
- Starting with this release the simulator is using new ns-3-dev 3ggp
  channel, spectrum, propagation, channel condition and antenna models
  that are implemented in spectrum, propagation and antenna modules of
  ns-3-dev. To allow usage of this new channel and antenna models, we have
  introduced a new BeamManager class which is responsible configuration of
  beamforming vectors of antenna arrays. BeamManager class is also responsible
  of configuring quasi-omni beamforming vector for omni transmissions.
  Since real beamforming management methods are still not implemented
  in our module, there are available ideal beamforming methods: cell scan
  and direct path. User can configure ideal beamforming method by using
  attribute of IdealBeamformingHelper which is in charge of creating
  the corresponding beamforming algorithm and calling it with configured
  periodicity to generate beamforming vectors for pairs of gNBs and UEs.
  BeamManager class is then responsible to cache beamforming vectors for
  antenna. For example, at gNB BeamManager for each connected UE device
  there will be cached the beamforming vector that will be used for  
  communication with that UE. In the same way, the BeamManager at UE
  serves the same purpose, with the difference that it will be normally just one
  element in the map and that is toward its own gNB.
- Starting with this release the default behaviour will be to calculate
  interference for all the links, and will not be any more possible to exclude
  UE->UE and GNB->GNB interference calculations.
- NrHelper is now refactored to take into account Multi-Cell Configurations
  (i.e., different configuration for different cells).
- Introduced the TDD pattern: every gNb can use a different pattern, specified
  by a string that identify a sequence of slot types (e.g.,
  "DL|UL|UL|DL|DL|DL|").
- Introduced FDD operational mode for a Bandwidth Part.
- A new Component Carrier/Bandwidth Part helper has been added in file
  `cc-bwp-helper.h`. With this class, it is easy to divide the spectrum in
  different regions.
- Added the support for NR MAC sub-headers and subPDU.
- Added SHORT_BSR as MAC CE element, that goes with MAC data, and is evaluated
  by the error model upon delivery.
- A new NrRadioEnvironmentMapHelper has been implemented for the generation of
  DL and UL REM maps.

Bugs fixed
----------

- Removed legacy and invalid mmwave-* examples, that were inherited from mmWave codebase.
- The code performing LBT at UE side always assumed a DL CTRL symbol inside a slot.
  With TDD, that may not happen, and the code has been updated to not
  always assume a DL CTRL.
- TDMA Scheduler was assuming that there are always UE to schedule. That may not be true
  if these UEs have scheduled an HARQ retransmission (and hence unable to
  receive a new data DCI). Code updated to remove this assumption.
- gNb was scheduling first DL and then UL, so when it was calculating the symbols to be
  assigned to DL, it was not taking into account the UL symbols. Code has been updated to
  schedule first UL and then DL.
- N1 (used to schedule DL HARQ Feedback) could not be set to zero, because the way the
  calculation was carried out was resulting in negative delay. Calculation has been updated
  to take into account the case of N1=0.
- The Harq timer is now set to 0 every time a retransmission is scheduled and
  transmitted (before, it was just incremented)
- The BwpManagerGnb now redirects BSR and SR to the source bandwidth part, as
  the UE has already done the selection based on the configured QCI.
- LBT at the gNb side is now scheduled (and done) every time there is a DL CTRL
  to transmit, as in FDD configuration, other BWP can inject messages after
  the start of the slot.
- NrLteErrorModel contained a bug that prevented the calculation of the right
  error value.
- Scheduler contained a bug that was forcing, in some situation, a double SR
  scheduling, stopped by a FATAL_ERROR.

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr


Release NR-v0.4
--------------

Availability
------------
Available since Thu Feb 13 2020.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note:  not all features available on all platforms):

- g++-4.9 or later
- Apple LLVM version 7.0.2 or later
- clang-3.3 or later

In addition, Python 2.7 (Python 2 series) or Python 3.4-3.7 (Python 3 series)

This release has been tested on the following platforms:
- ArchLinux with g++-9.2.1, clang-9.0.1, and Python 3.8.1
- Ubuntu 18.04 (64 bit) with g++-8.3.0 and Python 2.7.12/3.5.2

Important news
--------------
- This release is aligned with CTTC's ns-3-dev commit id
217b410c lte-rlc: TM is now sending more than one packet per transmission opportunity
that is on top of the nsnam ns-3-dev master commit id
578c107eb internet: fix packet deduplication test .
To upgrade CTTC's ns-3-dev, please run the following (save any non-official
commit, as they will be deleted):

$ cd /path/to/cttc/ns-3-dev
$ git reset --hard HEAD~200
$ git pull

This module can be updated with the usual

$ git pull


New user-visible features (old first)
-------------------------------------

- (error model) new BLER-SINR tables for MCS table1 and table2
- (performance) Various performance improvements


Bugs fixed
----------
- (scheduler) Correctly schedule beams of users that got a HARQ retx space.

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr


Release NR-v0.3
--------------

Availability
------------
Available since Tue Aug 27 2019.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note:  not all features available on all platforms):

- g++-4.9 or later
- Apple LLVM version 7.0.2 or later
- clang-3.3 or later

In addition, Python 2.7 (Python 2 series) or Python 3.4-3.7 (Python 3 series)

This release has been tested on the following platforms:
- ArchLinux 2018.10.08 with g++-8.2.1 and Python 3.7.1
- Ubuntu 16.04 (64 bit) with g++-5.4.0 and Python 2.7.12/3.5.2

New user-visible features (old first)
-------------------------

- (error model) Added the NrEesmErrorModel class. It models the NR PHY
abstraction according to LDPC coding, block segmentation, and including
MCS/CQI table 1 and 2. The user can select the HARQ method (HarqCc or HarqIr)
as well as the MCS/CQI table of NR to be used (McsTable1 or McsTable2), through
new attributes. In this release, the BLER-SINR tables are not completed yet,
and so it is recommended not to use this error model.

- (3gpp channel model) 3gppChannelModel can now be used by any other module,
i.e. it is not any more mmwave specific spectrum propagation model. This means
that any subclass of NetDevice can be attached to a channel using this
SpectrumPropagationModel. An additional requirement is that the technology uses
AntennaModel that is implementing AntennaArrayBasicModel interface. This
feature is a basic prerequisite for simulating co-existence of different
technologies that are using the 3gpp channel model implementation.

- (3gpp channel model) The beamforming phase has been extracted from the model,
and it is now a duty of the NetDevice. The gNB phy has now a new attribute to
configure the periodicity of the beamforming. Please note that it is still ideal,
i.e., it does not require any simulated time to be performed.

- (3gpp channel model) gNB-gNB and UE-UE pathloss and channel computation can be
allowed (through a new attribute) for Indoor Hotspot scenarios.

- (spectrum phy) gNB-gNB and UE-UE interferences can be enabled (through a new
attribute).

- (RRC) Now all carriers are registered to RRC, to transmit system information
through all the bandwidth parts.

- (SCHED) The scheduler now is informed of RACH preamble messages, and reserve
some space in the DL CTRL symbol to send the RAR messages.

- Added traces that indicate the transmission or the reception of CTRL
messages. For instance, take a look to *EnbMacRxedCtrlMsgsTrace* or
*EnbMacTxedCtrlMsgsTrace* in the Gnb MAC file.

Bugs fixed
----------
- (scheduler) Fixed the use of a static MCS value in the schedulers
- (spectrum-phy) While looping the packets in the packet burst,
  to send the feedback, extract the RNTI for each loop. Before, it was
  asserting when the RNTI changed. The code does not depend on the
  RNTI previous values, so it should be safe.
- (phy) At the beginning, fill some slot (number configured by UL sched delay
  param) with UL CTRL symbol.

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr


Release NR-v0.2
---------------

Availability
------------
Available since Fri Feb 1 2019.

Supported platforms
-------------------
This release is intended to work on systems with the following minimal
requirements (Note:  not all features available on all platforms):

- g++-4.9 or later
- Apple LLVM version 7.0.2 or later
- clang-3.3 or later

In addition, Python 2.7 (Python 2 series) or Python 3.4-3.7 (Python 3 series)

This release has been tested on the following platforms:
- ArchLinux 2018.10.08 with g++-8.2.1 and Python 3.7.1
- Ubuntu 16.04 (64 bit) with g++-5.4.0 and Python 2.7.12/3.5.2

New user-visible features (old first)
-------------------------

- (mmwave) Removed any reference to Rlc Low Latency
- (mmwave) The code that was previously under the directory src/mmwave has been
  moved to src/nr.
- (nr) Aligned ComponentCarrierGnb, MmWaveEnbMac, MmWaveUePhy to ns-3-dev
- (nr) Aligned the BwpManager and the various helper/example to the bearer
  definitions in Rel. 15
- (nr) Removed unsupported MmWaveBeamforming and MmWaveChannelMatrix classes.
- (nr) Added a 3GPP-compliant antenna class.
- (nr) Added a 3GPP-compliant UL scheduling request feature.

Bugs fixed
----------

Known issues
------------
In general, known issues are tracked on the project tracker available
at https://gitlab.cttc.es/ns3-new-radio/nr

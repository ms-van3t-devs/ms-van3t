5G-LENA Changes       {#changes}
===============

NR module: API and model change history
---------------------------------------

<!-- This ChangeLog is updated in the reverse order with the most recent changes coming first.  Date format:  DD-MM-YYYY -->

ns-3 is an evolving system and there will be API or behavioral changes
from time to time.   Users who try to use scripts or models across
versions of ns-3 may encounter problems at compile time, run time, or
may see the simulation output change.

We have adopted the development policy that we are going to try to ease
the impact of these changes on users by documenting these changes in a
single place (this file), and not by providing a temporary or permanent
backward-compatibility software layer.

A related file is the RELEASE_NOTES.md file in the top level directory.
This file complements RELEASE_NOTES.md by focusing on API and behavioral
changes that users upgrading from one release to the next may encounter.
RELEASE_NOTES attempts to comprehensively list all of the changes
that were made.  There is generally some overlap in the information
contained in RELEASE_NOTES.md and this file.

The goal is that users who encounter a problem when trying to use older
code with newer code should be able to consult this file to find
guidance as to how to fix the problem.  For instance, if a method name
or signature has changed, it should be stated what the new replacement
name is.

Note that users who upgrade the simulator across versions, or who work
directly out of the development tree, may find that simulation output
changes even when the compilation doesn't break, such as when a
simulator default value is changed.  Therefore, it is good practice for
_anyone_ using code across multiple ns-3 releases to consult this file,
as well as the RELEASE_NOTES.md, to understand what has changed over time.

This file is a best-effort approach to solving this issue; we will do
our best but can guarantee that there will be things that fall through
the cracks, unfortunately.  If you, as a user, can suggest improvements
to this file based on your experience, please contribute a patch or drop
us a note on ns-developers mailing list.

---


## Changes from NR-v2.1 to v2.2

This release contains only the upgrade of the supported ns-3 release, 
i.e., upgrade from ns-3.36 to ns-3.36.1.

### New API:

None.

### Changes to existing API:

None.

### Changed behavior:

None.

---

## Changes from NR-v2.0 to v2.1

### New API:
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

### Changes to existing API:

### Changed behavior:
- Calculate CQI based on Average Spectral Efficiency in `nr-amc.cc`
- Stop assigning resources in UL TDMA if TB size >= buffSize in `nr-mac-scheduler-ue-info.cc`
- Changed how to consider RLC overhead when updating the TX queues in MAC scheduler
- Changed the buffer size calculation in `NrMacSchedulerLcg` to consider correctly the 
RLC overhead

---


## Changes from NR-v1.3 to v2.0

### New API:

* `NrUePhy` has a new function `GetSpectrumPhy` that returns the `NrSpectrumPhy` 
instance corresponding to the provided index. By default it returns 0th 
`NrSpectrumPhy` instance, since `NrUePhy` must have at least 1 `NrSpectrumPhy` 
installed. But, can have more instances, depending on the supported number 
of streams for DP-MIMO. Additionally, 
`NrUePhy` has new public functions related to rank adaptation: `SetFixedRankIndicator`, 
`GetFixedRankIndicator`, `UseFixedRankIndicator`, `SetRiSinrThreshold1`, `GetRiSinrThreshold1`, 
`SetRiSinrThreshold2`, and `GetRiSinrThreshold2`. Additionally, it has one new 
private function called `SelectRi` for selecting the RI that will be reported to 
gNB. `NrSpectrumPhy` has new function for configuring the inter-stream interference, 
called `SetInterStreamInterferenceRatio`.

* `BeamConfId` is a new class that is added to uniquely identify the beam 
configuration of a `NrUePhy` or `NrGnbPhy` supporting DP-MIMO, hence up to two streams.
Previously, OFDMA scheduler was assigning at the same varTti only users beloning 
to the same beam (identified by BeamId), because there was maximum 1 beam per 
`NrUePhy` or `NrGnbPhy` instance. Now, since with DP-MIMO we can have 2 beams per 
PHY instance, it is used `BeamConfId` in OFDMA scheduling to select the users 
that can be scheduled at the same varTti. 

* Added new `NrStatsCalculator` statistic base class, and its specialization class called 
 `NrMacSchedulingStats` class that provides statistics for DL and UL MAC scheduling, 
 and it also includes the stream ID. Added new `NrSchedulingCallbackInfo` structure 
 for holding the scheduling information. 
 
 * `NrGnbMac` has a new trace source called `UlScheduling`, while `DlScheduling` 
 trace source is extended to provide more detailed scheduling information.
 
 * Added new `ThreeGppChannelModelParam` class that inherits `ThreeGppChannelModel` 
 class and allows the parametrization of the correlation coefficient. This 
 class is added for research purposes to allow parametrizing the cross correlation 
 correlation parameter and thus study the impact of this parameter on 
 inter-stream interference among the signals of different polarizations, and 
 to compare it with the original 3GPP cross polarization correlation parameter 
 value and resulting interference.
 
 * Added new example called `cttc-nr-mimo-demo` that allows the configuration, 
 usage and testing of DP-MIMO feature in nr module.
 
 * Added API for new pathloss traces. Extended CTRL traces to include SRS Tx and 
 RX information. Added stream ID in DL/UL RX traces (`NrPhyRxTrace`).
 
 * `NrHelper` functions for enabling NR traces are now part of NrHelper public API. 
 These new public functions are: `EnableDlPhyTraces`, `EnableUlPhyTraces`, 
 `EnableGnbPacketCountTrace`, `EnableUePacketCountTrace`, `EnableTransportBlockTrace`, 
 `EnableGnbPhyCtrlMsgsTraces`, `EnableUePhyCtrlMsgsTraces`, `EnableGnbMacCtrlMsgsTraces`, 
 `EnableUeMacCtrlMsgsTraces`, `EnableRlcTraces`, `EnablePdcpTraces`, `GetPdcpStats`, 
 `EnableDlMacSchedTraces`, `EnableUlMacSchedTraces` and `EnablePathlossTraces`.
 

### Changes to existing API:

* `NrHelper`'s public API functions `InstallUeDevice`, `InstallGnbDevice` have one 
additional parameter that defines how many streams will support the device. 
The value of this new parameter is by default 1 so it does not impact the usual 
usage of these functions. `NrHelper`'s private API functions `CreateGnbPhy`, 
`CreateUePhy`, `InstallSingleUeDevice` and `InstallSingleGnbDevice` have now one 
additional parameter that allows to configure the number of streams (antenna 
arrays or subpartitions) supported by the corresponding PHY or device. 
Also, `NrHelper`'s private function `CreateUePhy` has now one less parameter, 
i.e., dlHarqCallback parameter is removed. These changes should not affect 
5G-LENA users who do not modify/inherit/extend `NrHelper`'s code.

* `NrGnbPhy` function `GenerateDataCqiReport` has one more paramater `streamId` to 
indicate for which stream is reported CQI. Private function `NrGnbPhy::SendDataChannels` 
has one more parameter that is the index of the `NrSpectrumPhy` of that `NrGnbPhy`
over which will be sent the data. Private function `SetSubChannels` has one more 
parameter that says the number of active streams.

* Changed parameter type of the `NrUePhy::CreateDlCqiFeedbackMessage` function.
Private `NrUePhy::SetSubChannelsForTransmission` function has one more parameter that 
indicates the number of active streams, i.e, the number of antenna arrays 
(subpartitions) over which will be split the total configured TX power.
`ReportCurrentCellRsrpSinr` extended to include the stream ID.
`NrUePhy::GetTxPowerSpectralDensity` has one more parameter that indicates the number 
of active streams (antenna subpartitions or subarrays) over which the total TX 
power will be split. 

* Callback functions for the HARQ feedback are moved from `NrSpectrumPhy` to `NrUePhy`.
`NrUePhy` now has the the following functions: `SetPhyDlHarqFeedbackCallback` and 
 `NotifyDlHarqFeedback`.

* Old struct that represents HARQ information, called `NrDlHarqProcessInfo` has 
been replaced with `HarqProcessInfoSingleStream` that is used for the same purpose. 
Now, new `NrDlHarqProcessInfo` contains a vector of `HarqProcessInfoSingleStream`, 
one instance per spectrum phy (per antenna araray or antenna subpartition).

* `DciInfoElementTdma` constructor is changed to support more streams, i.e., 
instead of scalar parameters for tbs, ndi and rv, now there are `std::vector` 
parameters for the same purpose. `HarqStatus` enumeration extended to support 
new `NONE` state, apart from previous `NACK` and `ACK`.

* `NrPhySap` function `SendMacPdu` now has one more parameter that indicates the 
index of the NrSpectrumPhy (stream) which will be used for the transmission. 
The default value of this parameter is 0, so this change should not affect 
regular usage of this function call. 

* APIs of `BeamformingHelperBase`, `RealisticBeamformingHelper` and 
`IdealBeamformingHelper` are changed to allow beamforming per stream (antenna array of 
`NrPhy`), and are also refactored to simplify function calls.

*`BeamManager` is installed per `NrSpectrumPhy` instance and not per `NrPhy` instance 
as before. Each `BeamManager` takes care of a single antenna array or antenna 
subpartition. 

* APIs of `RealisticBeamformingAlgorithm` and all `IdeaBeamformingAlgorithm` child 
classes have been updated accordingly.

### Changed behavior:

---

## Changes from NR-v1.2 to v1.3

### New API:

* `NrGnbNetDevice` now has function `GetCellIds` that returns the list of 
cell IDs belonging to that gNB. This function is added for the 
compatibility with the ns-3 LTE 810 MR.
* `NrUePhy` implements `DoGetCellId` and `DoGetDlEarfcn` for the API 
compatibility with the ns-3 LTE 810 MR.

### Changes to existing API:

* `NrSpectrumPhy` and `BeamManager` have a function called `GetAntenna` 
instead of `GetAntennaArray` that they had previously.
* `NrPhy`, `NrUePhy`, `NrGnbPhy`, do not have any more function called 
`GetAntennaArray`.

### Changed behavior:

---

## Changes from NR-v1.1 to v1.2

### New API:

* Added "file-transfer-application.cc/h" and "file-transfer-helper.cc/h" that
implement File Transfer Protocol (FTP) applications. In addition, 3GPP FTP model
1 helper has been added that configures FTP Model 1 traffic model. These FTP and
3GPP FTP M1 implementations could be used by other modules, but they still need
some development, so for the moment they will be included in the nr module.
* New `NrHarqTest` is included to test HARQ-IR and HARQ-CC combining methods (thanks
to Andrey Belogaev).


### Changes to existing API:

* `m_numSymAlloc` field of `SlotAllocInfo` structure and the `usedSym` variable
in `NrMacSchedulerNs3::DoScheduleDlData` and `NrMacSchedulerNs3::DoScheduleUlData`
are now correctly updated when a UE does not get a DCI (i.e., when the TBS is less than 7 bytes).
* PHY traces have been modified to allow post-processing. Moreover, a simulation
tag can be added in order to generate different file names when Simulation Campaigns
are performed.
* `cttc-fh-compression.cc` example has been extended with a possibility to use
FTP traffic model.
* Extended APIs to compute SRS SNR and enable real BF based on SRS SNR measurements
(previously real BF was performed based on SRS SINR measurements, now it can be done
based on either SRS SNR or SRS SINR). In particular, `NrInterference` has been
extended to compute SNR of received data, a new callback is included in `NrSpectrumPhy`
to compute the SNR of SRS, and real BF algorithm has been extended to use SNR report.
The new attribute `UseSnrSrs` of `RealisticBeamformingAlgorithm` denotes whether
the SRS measurement will be SNR or SINR (and defaults to true, i.e., SNR is used).


### Changed behavior:

* If HARQ-IR is used, the performance may change because its error modeling uses
a new formula for the computation of the effective SINR.
* The available SRS offsets are correctly generated so that none of the UEs get
the same SRS offset.
* If realistic beamforming algorithm with trigger event configured with delay update
is used, the performance may change because it now uses the actual channel at SRS
reception moment for real BF update with delay.
* By default, SRS Symbols is set to 1 symbol. In the previous release, the default
value was 4 symbols.

---

## Changes from NR-v1.0 to v1.1

### New API:

* Added attribute "SrsSymbols" in `NrMacSchedulerNs3`, to indicate how many
  symbols are available to the SRS message.
* Added attribute "EnableSrsInUlSlots" in `NrMacSchedulerNs3` to allow SRS only
  in F slots, or both in F and UL slots.
* Added attribute "EnableSrsInFSlots" in `NrMacSchedulerNs3` through which SRS
  can be disabled in F slots. If SRS is disabled in both UL and F slots, then
  SRS will be completely disabled.
* Added new beamforming algorithm called `RealisticBeamformingAlgorithm`
  which determines the beamforming vector of the transmitter and receiver based
  on the SINR of SRS.
* Added `RealisticBeamformingHelper` that needs to be used when
  `RealisticBeamformingAlgorithm` is being configured in order to
  schedule beamforming updates and to collect the information of SRS SINR
  reports that are necessary for the `RealisticBeamformingAlgorithm`
  execution.
* Added attribute `PowerAllocationType` to `NrGnbPhy` and `NrUePhy`, which allows
  configuring the power allocation type. Currently, two types of power allocation
  are supported: uniformly over all bandwidth (all RBs) or uniformly over active
  (used) RBs.
* Added `NrUePowerControl` class that implements NR UL PC power control
  functionality. Support UL PC power control for PUSCH, PUCCH and SRS, and can
  operate in two different modes: TS 36.213 and TS 38.213 modes. Uplink power
  control is disabled by default.
* Proportional Fair scheduler in UL direction is added.
* New examples: cttc-nr-notching.cc and cttc-realistic-beamforming.cc
* New tests: nr-test-notching.cc, nr-uplink-power-control-test.cc, and
  nr-realistic-beamforming-test.cc


### Changes to existing API:

* Added `NrHelper::AssignStreams` function that is a central function in NR module for
  assigning streams. The exceptions are:
  `HexagonalGridScenarioHelper` and `GridScenarioHelper` whose randomness is not
  controlled from NrHelper::AssignStreams function, but instead it needs to be called
  separately if one wants to assign streams for scenarios. Also, class `RealisticBeamformingAlgorithm`
  is not controlled from `NrHelper::AssignStreams`, but it should be from future upgrades.
  To allow assigning streams and control randomness in NR module all functions that have
  some random variables are extended with `AssignStreams` function. Classes that are extended
  include: `NrUeMac`, `NrSpectrumPhy`, `NrMacSchedulerSrsDefault` (also parent classes
  `NrMacSchedulerNs3` and `NrMacScheduler`), `RealisticBeamformingAlgorithm`, `NrHelper`,
  `GridScenarioHelper`, `HexagonalGridScenarioHelper`.
* Added functions InH_OfficeOpen_nLos, InH_OfficeOpen_Los, InH_OfficeMixed_nLoS,
  InH_OfficeMixed_LoS, InitUmaBuildings, InitUmiBuildings, InitV2VHighway,
  InitV2VUrban channel modeling in cc-bwp-helper.h/cc and nr-helper.cc
* Attribute `IdealBeamformingMethod` of `IdealBeamformingHelper` class
  is renamed to `BeamformingMethod`
* The scheduler now support the setting of the notched mask, through
  void SetDlNotchedRbgMask (const std::vector<uint8_t> &dlNotchedRbgsMask);
  for the DL and
  void SetUlNotchedRbgMask (const std::vector<uint8_t> &ulNotchedRbgsMask);
  for the UL
* Added != operator function to `BeamId` class.
* IPV6 is now supported. That is, the end-to-end connections between
  the UEs and the remote hosts can be IPv4 or IPv6. The classes,
  `NrNetDevice`, `NrGnbNetDevice`, and `NrUeNetDevice` are updated accordingly.
* Previously we had two overloaded functions `GetSpectrumModel` one was with
  the parameters: double bandwidth, double centerFrequency, uint8_t numerology
  and another with parameters: uint32_t numRbs, double centerFrequency,
  double subcarrierSpacing. These two functions are unified, and only the latter
  one is left.
* `nr-phy.h` does not contain any more default configurations related to the
  bandwidth. Default configurations is now set for UE by `DoSetInitialBandwidth`,
  and for gNB it should be set with UpdateConfig call.
* Initial bandwidth at UE depends on the numerology and it will be set to 6 RBs until
  configured through RRC.
* The example lte-lena-comparison.cc has been extended with more command line
  parameters.
* Some tests are renamed so that all tests start with prefix `nr-`.


### Changed behavior:

* If a notching mask is set, the scheduler will avoid to allocate the RBG in
  which the mask (a vector of integers) is set to zero.
* When PF scheduler is configured, UL is also PF (previously RR UL was considered)
* Newly added `NrHelper::AssignStreams` function may change default stream
  assignment that was being assigned previously in NR examples. All examples
  are updated to use `NrHelper::AssignStreams` to fix random streams in NR module.
* By default is configured power allocation over active RBs. Before this release, by
  default was uniform power allocation over all RBs. Power allocation type can be 
  configured by using `PowerAllocationType` attribute of `NrGnbPhy` and `NrUePhy`.
* Newly added SRS allocation, transmission and reception will occupy periodically
  4 symbols in a slot over a certain periodicity, in UL or F slots.
* If real beamforming is configured, then the beamforming update will be based
  on SRS SINR reports, instead of with a given periodicity (as it was before
  in the case of ideal beamforming).

---

## Changes from NR-v0.4 to v1.0

### New API:

* Added `ReportPowerSpectralDensity` trace source in NrUePhy.
* Added `SlotDataStats` and `SlotCtrlStats` trace source in NrGnbPhy.
* Added `RBDataStats` trace source in NrGnbPhy.
* Added `RxDataTrace` trace source in NrSpectrumPhy.
* Extended the GridScenarioHelper with the introduction of the
  HexagonalGridScenarioHelper class. This new class creates a hexagonal grid
  deployment consisting of up to 19 sites with 3 sectors of 120º each,
  resulting in 57 hexagonal cells. The location of sites and UEs can be
  represented with gnuplot.
* Added GetSpectrumModel to the SAP interface between PHY and MAC, so the CQI
  calculation can see that value.
* Added `Pattern` attribute to NrGnbPhy, to set the TDD Pattern.
* Added IdealBeamformingHelper class that is used to configure ideal beamforming
  algorithm and the periodicity of performing the ideal beamforming methods.
  Attribute `IdealBeamformingMethod` will be used to configure the beamforming
  algorithm, and `BeamformingPeriodicity` to configure the beamforming
  periodicity.
* Added IdealBeamformingAlgorithm class and its subclasses that are used to
  configure beamforming vectors for the pairs of devices, normally between gNB
  and UE, but is possible to use it in future for UE to UE.  
* Added BeamManager class at gNB and UE phy that is responsible for caching
  beamforming vectors to use when communicating with connected devices. It is
  also responsible for configuring quasi-omni beamforming vector for omni
  transmissions. This class should be used by gNB and UE to control its
  ThreeGppAntennaArrayModel instances.
* Added attribute `NumRefScPerRb` in NrAmc, to indicate the number of reference
  subcarriers per RB.


### Changes to existing API:

* Functions `NrGnbPhy::ReceiveUlHarqFeedback` and `NrUePhy::ReceiveLteDlHarqFeedback`
  are renamed to `NrGnbPhy::ReportUlHarqFeedback` and `NrUePhy::EnqueueDlHarqFeedback`, respectively.
* Removed attribute `NumberOfComponentCarriers` and `UseCa` from NrHelper.
* Removed `SetScheduler` method from NrHelper.
* NrUePhy includes now traces for the DL DCI and the corresponding DL HARQ Feedback
* Renamed MmWaveEnbNetDevice attribute `ComponentCarrierMap` into BandwidthPartMap
* Removed `CentreFrequency` attribute from MmWavePhyMacCommon: it is now Set
  by the CcBwpHelper while dividing the spectrum.
* Removed `N0Delay` and `N1Delay` attributes from MmWavePhyMacCommon: they are
  now in NrGnbPhy.
* Removed `TbDecodeLatency` attribute from MmWavePhyMacCommon: it is now
  an attribute of `NrGnbPhy` and `NrUePhy`.
* Moved `NumRbPerRbg` attribute from MmWavePhyMacCommon to NrGnbMac.
  This attribute is still needed by UE PHY, thus for the moment this attribute
  cannot be reconfigured once being set. Once that DCI bitmask is changed to
  work per the RB granularity, this attribute can be reconfigured.
* Removed `CellId` attribute from NrGnbNetDevice.
* Removed `CcId` attribute from MmWavePhyMacCommon.
* Removed `MacSchedType` attribute from MmWavePhyMacCommon.
* Removed `Bandwidth`, `Numerology`, `SymbolsPerSlot` attribute from
  MmWavePhyMacCommon. These attribute are now of NrGnbPhy, while the UEs get
  these values from RRC or from direct call by the helper at the attaching
  moment.
* Removed `NumHarqProcess` and `HarqDlTimeout` from MmWavePhyMacCommon.
  `NumHarqProcess` is now attribute of NrGnbnbMac and NrUeMac, while
  `HarqDlTimeout` is completely removed since the timeout is equivalent to the (maximum) number of harq processes.
* The number of DL and UL CTRL symbols can be configured now in
  NrMacSchedulerNs3 through the attributes `DlCtrlSymbols` and `UlCtrlSymbols`.
* Removed attribute `L1L2CtrlLatency`, and fixed it to 2 in NrPhy.
* Removed `EnableAllInterferences` attribute from MmWaveSpectrumPhy which was
  used to enable or disable interference calculations for all links.
* Replaced the message DCI_TDMA with UL_DCI and DL_DCI, to differentiate among
  the allocations.
* The EESM error model class has been separated, and the attributes HarqMethod
  and McsTable removed. Now, to configure different error model based on EESM,
  you have to specify directly the Harq method and the table name in the
  error model name: the new classes are NrEesmIrT1, NrEesmIrT2, NrEesmCcT1,
  NrEesmCcT2.
* The AMC inside the scheduler has been separated into an UL and a DL part.
  Methods in the helper have been added to help the user configuring that part.
  In particular, have a look at SetGnbDlAmcAttribute and SetGnbUlAmcAttribute.
* Removed attribute `Ber` in NrAmc, which is now set based on the error model
  in use.
* Renamed all mmwave- classes, tests, examples, helpers, to nr-.

### Changed behavior:

* K0, K1, K2 Delays are removed from the phy-mac common, instead they are
  implemented as parameters of the DCI. In the DCI message, the gNb reports the
  K{0,1,2} delay instead of the sfn number.
* UE receives DL data according to K0 and sends UL data according to K2
  (passed from the gNb in the DL and UL DCI, respectively).
* UE schedules the DL HARQ Feedback according to K1 delay (passed from the
  gNb to the UE in the DL DCI).
* Gnb first schedules UL and then DL.
* RACH Preamble is sent without applying any delay (i.e. L1l2CtrlLatency)

---

## Changes from NR-v0.3 to NR-v0.4

### New API:

### Changes to existing API:

* Replaced SINR-BLER curves for MCS Table1 and Table2 of the NR error model
  with more accurate (and complete) values.

### Changed behavior:


---

## Changes from NR-v0.2 to NR-v0.3

### New API:

* Added NrErrorModel as interface for any error model. It is used by NrAmc and
MmWaveSpectrumPhy.
* Added attribute *ErrorModelType* to the class NrAmc, which must be kept in
sync with the attribute *ErrorModelType* of MmWaveSpectrumPhy
* Added attribute *ErrorModelType* to the class MmWaveSpectrumPhy
* Added class NrEesmErrorModel to model the NR PHY abstraction according to LDPC
coding, block segmentation, and including MCS/CQI table 1 and 2. It has an attribute
to select the HARQ method (HarqCc or HarqIr) and another attribute to select the
MCS/CQI table of NR to be used (McsTable1 or McsTable2). In this release, the
BLER-SINR are not completed yet, and so it is recommended not to use this model.
* Added attributes AntennaNumDim1, AntennaNumDim2, AntennaArrayType to
MmWaveEnbPhy and MmWaveUePhy. And these two classes are responsible for an
antenna array installation. This means that there is an instance of
AntennaArrayBasicModel per MmWavePhy.
* MmWavePhy has new functions Set/Get functions for configuring and
obtaining AntennaNumDim1, AntennaNumDim2, AntennaArrayType attributes. The
configuration of these parameters is allowed through MmWavePhy object since
we would like to have access to these attributes through the Config system
and that we can specify different values for an antenna for MmWaveUePhy and
MmWaveEnbPhy. Function to set AntennaArrayType can be called only for the
initial configuration. If it is called after the antenna object is created the
program will be halted and the user will be informed of the error.
* AntennaArray3gppModel has new attribute RandomUeorientation. If set to true
the 3D antenna orientation will be generated randomly throwing a dice by
using a UniformRandomVariable.
* MmWave3gppChannel has a new attribute UpdateBeamformingVectorsIdeally which
determines if the update of the beamforming vectors will be ideal, i.e. if
the beamforming vectors will be adjusted every time that the channel is updated.
* AntennaArrayBasicModel has a new function GetBeamformingVectorUpdateTime
that returns the last time at which the beamforming vector toward a specified
device has been updated.
* EnbPhy now can perform beamforming, in a very initial form (every connected ue
will perform the beamforming, and it will take 0 simulated time). The periodicity
of this beamforming depends on the value of the new attribute *BeamformingPeriodicity*
of the class MmWaveEnbPhy
* MmWaveSpectrumPhy now has two more traces: TxCtrlTrace and TxDataTrace, to extract
information about how the channel is occupied
* Now MmWaveSpectrumPhy can enable interference for all links, also UE->UE and GNB->GNB,
configured through the attribute EnableAllInterferences.
* Now MmWave3gppChannel can enable generation of propagation losses for all links,
also UE->UE and GNB->GNB, configured through the attribute EnableAllChannels. This
feature, nowadays, applies *only* to Indoor Hotspot scenarios, for which the 3GPP pathloss
model is valid.
* Introduced Listen-Before-Talk after MAC. The interface of a channel access manager
is inside the file *nr-ch-access-manager.h*.

### Changes to existing API:

* Renamed MmWaveAmc into NrAmc
* Renamed MmWaveMiErrorModel in NrLteMiErrorModel, and adapted it to the new
NrErrorModel interface
* MmWave3ppChannelModel does not anymore have an attribute for
MmWavePhyMacCommon class and thus it does not have any more the corresponding
functions to Set/Get the corresponding instance, SetConfigurationParameters
and GetConfigurationParameters, respectively. It instead has a new attribute
that needs to be configured at the beginning of the simulation.
This attribute is Bandwidth. The two most important attributes of this class
are now CenterFrequency and Bandwidth.
* AntennaArrayBasicModel has now a pointer to a SpectrumModel.
* MmWaveSpectrumValueHelper.
* Common functions of MmWaveEnbPhy and MmWaveUePhy,
CreateTxPowerSpectralDensity and CreateNoisePowerSpectralDensity are moved to
MmWaveUePhy and renamed to GetNoisePowerSpectralDensity and
GetTxPowerSpectralDensity to distinguish them from the original functions that
belong to MmWaveSpectrumValueHelper.
* MmWaveSpectrumValueHelper does not depend any more on MmWavePhyMacCommon
class. Functions that had up to know this class as an input, now have the
minimum subset of parameters that are used in the corresponding function.
* MmWaveSpectrumValueHelper does not have anymore empty function for
CreateTxPowerSpectralDensity that had as an input a power map (powerTxMap).
* MmWave3gppChannel can now be used by any other module, it is not any more
only mmwave specific spectrum propagation model. This means that any subclass
of NetDevice can be attached to a channel using this SpectrumPropagationModel.
An additional requirement is that the technology uses AntennaModel that is
implementing AntennaArrayBasicModel interface. The
dependencies from mmwave module-specific classes are removed, e.g. dependency on
MmWaveEnbNetDevice, MmWaveUeNetDevice, MmWaveUePhy, MmWaveEnbPhy.
* Removed MmWaveMacSchedulerNs3 attribute McsDefaultDl/McsDefaultUl. Its
functionality is now taken by the attribute StartingMcsDl/StartingMcsUl
* Renamed MmWavePointToPointEpcHelper into NrPointToPointEpcHelper. All
attributes that before were in the form "ns3::MmWavePointToPointEpcHelper"
must now be referred to "ns3::PointToPointEpcHelper". In fact, NrPointToPointEpcHelper
is now inheriting from PointToPointEpcHelper.
* AntennaArrayModel has the beamforming vector that emulates omni reception and
transmission.
* Removed attribute *BeamformingEnabled* and *UpdateBeamformingVectorsIdeally*
from MmWave3gppChannel.
* Removed attribute *EnbComponentCarrierManager* from MmWaveHelper. The only allowed
CCManager is now BwpManagerGnb.
* Removed attribute *UeComponentCarrierManager* from MmWaveHelper. The only allowed
manager is now UeManagerGnb.
* Removed attribute *Bandwidth* from MmWave3gppChannel.

### Changed behavior:
* BeamSearchBeamforming and LongTermCovMatrixBeamforming functions of
3gppChannelModel can now be called from the outside of this class. This means
that the update of the beamforming vectors can be triggered at any time and
does not have to be related to the channel update event.
* TxPower default value for a UE passed from 30.0 dBm to 2.0 dBm
* TxPower default value for a gNb passed from 30.0 dBm to 2.0 dBm
* MmWave3gppChannel supports calculation of the beamforming gain when one of
devices is in omni mode. This is possible because AntennaArrayModel has new
beamforming vector that emulates omni reception and transmission.
* The Slot Indication is asked to the MAC at the beginning of the slot, before
doing everything else at PHY level.
* UE and ENB PHY does not rely on the order of the allocation to understand if
an allocation is DL/UL, DATA/CTRL. Also, we support now the possibility to start
an allocation at a symbol != 0.
* The scheduler is now informed of RACH messages, even if (for the moment) it
does nothing to allocate space for UL RRC message, it reserve some ideal
space in the DL CTRL link to send the RAR response.
* The UE will ignore the UL CTRL slot if there are no messages to send. Practically speaking,
the UE will not set power on the RBG, not generating interference.
* The GNB will not put power during the DL CTRL symbol if there aren't messages
to send to the UEs.
* The control message timings are implemented in such a way that the delay can
be adjusted in a flexible manner. In particular, when a control message is passed
from PHY to MAC or from MAC to PHY, the user can adjust the delay according to each
release specifications. As default we apply a delay of 1 ms, except of some cases
(e.g. RAR msg), where the delay is set to 2 ms.

---

## Changes from NR-v0.1 to NR-v0.2

### New API:

* A new AntennaArray3gppModel is introduced that inherits all the features of the AntennaArrayModel, but it considers 3GPP directional antenna elements instead of ISO antenna elements.
* 3gppChannelModel has a new attribute "speed" for configuring the speed. Previous and currently the default behaviour is that 3gppChannelModel calculates the relative speed between the transmitter and the receiver based on their positions. However, this parameter can be configured when the static scenario is being used but is desired to imitate small scale fading effects that would exist in a mobile scenario.
* New traces sources are added to the Interference class for collecting the SNR and RSSI values.

### Changes to existing API:

* The supported bearers QCI are only the ones defined in Rel. 15.
* The Ue NoiseFigure is not anymore hard-coded value, instead can be configured through the new MmWaveUePhy attribute called "NoiseFigure".
* MmWaveEnbDevice and MmWaveUeDevice have new attributes AntennaNumDim1 and AntennaNumDim2 that allow the configuration of a rectangular antenna. Previously only squared antenna was supported.
* GnbAntennaArrayModelType and UeAntennaArrayModelType attributes are added to MmWaveHelper that allow the configuration of the antenna type (ISO vs 3gppp) of gNb and UE, respectively.
* AntennaArrayModel has a new attribute AntennaOrientation that can be used to configure the antenna orientation which can be horizontal in XY plane, or vertical in ZY plane.
* The attributes _MmWaveEnbNetDevice::MmWaveEnbPhy_ and _MmWaveEnbNetDevice::MmWaveEnbMac_ have been removed as they are replaced by the component carrier map. GetPhy () and GetMac () for UE and gNB NetDevices are now deleted, instead it is necessary to use the version with the ccId.
* Times in _PhyMacCommon_ class are now expressed with ns3::Time.
Removed attribute _PhyMacCommon::WbCqiPeriod_ as it was unused.

### Changed behavior:
* The SR is now sent only for Component Carrier ID == 0. The Enb Mac will receive it and forward to the CC manager; then, the CCM will route it to the most appropriate MAC, so the assignation for the SR will be done on the appropriate CC.
* In any given slot, the scheduler will now ignore UEs with new data to transmit if it assigned resources for retransmitting an HARQ process.
*  Default value of the UpdatePeriod parameter of the MmWave3gppChannel is changed to be 1 ms. Note that this dramatically slows down the simulation execution since the channel matrix will be updated every 1 ms. Previously, the default parameter was 0, which means that unless configured differently the channel matrix was not being updated.
* Previously there was a single propagation loss model, no matter how many BWPs there are in the simulation. Now, there is a propagation loss model instance per BWP. Long story short. MmWaveHelper instantiates a channel instance per BWP. The channel of each BWP is configured by using its own instance of the MmWavePhyMacCommon channel parameters. Since different BWPs are on different frequencies each BWP shall have its own propagation loss model that is configured with the frequency of the corresponding BWP.
* The default value of the UpdatePeriod parameter of the MmWave3gppChannel is returned to 0 ms. This is because it is detected that there are many occasions when the update of the channel matrix is not needed in the simulation example or the test, hence when the update is enabled by default the execution time of these simulations unnecessarily is slowed down.
* The SINR traces in MmWaveSpectrumPhy are now printed based on the TB of the UE, and not the entire band as it was before.
* The scheduler now assigns the maximum amount of data that is possible to the various LCs. Before, if there was only one LC with only 3 bytes, the assignation would have been of 3 bytes, despite of the TBS. Now, all the TBS can be assigned to him. We also manage cases in which multiple LC or LCG are active, but (as before) only in a round-robin fashion.

---

## Changes from NR-v0.0 to NR-v0.1

We did not track changes since the beginning. However, the main features we added to the mmWave module (from which we forked) are the FDM of numerologies and a new and improved MAC scheduling layer.

### New API:

* ND

### Changes to existing API:

* ND

### Changed behavior:

* ND

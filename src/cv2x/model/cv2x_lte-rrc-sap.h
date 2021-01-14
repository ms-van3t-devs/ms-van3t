/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Nicola Baldo <nbaldo@cttc.es>
 *          Lluis Parcerisa <lparcerisa@cttc.cat>
 * Modified by: NIST (D2D)
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */


#ifndef CV2X_LTE_RRC_SAP_H
#define CV2X_LTE_RRC_SAP_H

#include <stdint.h>
#include <list>
#include <bitset>

#include <ns3/ptr.h>
#include <ns3/simulator.h>

namespace ns3 {

class cv2x_LteRlcSapUser;
class cv2x_LtePdcpSapUser;
class cv2x_LteRlcSapProvider;
class cv2x_LtePdcpSapProvider;
class Packet;

/**
 * \ingroup lte
 *
 * \brief Class holding definition common to all UE/eNodeB SAP Users/Providers.
 *
 * See 3GPP TS 36.331 for reference.
 *
 * Note that only those values that are (expected to be) used by the
 * ns-3 model are mentioned here. The naming of the variables that are
 * defined here is the same of 36.331, except for removal of "-" and
 * conversion to CamelCase or ALL_CAPS where needed in order to follow
 * the ns-3 coding style. Due to the 1-to-1 mapping with TS 36.331,
 * detailed doxygen documentation is omitted, so please refer to
 * 36.331 for the meaning of these data structures / fields.
 *
 */
class cv2x_LteRrcSap
{
public:
  virtual ~cv2x_LteRrcSap ();

  /// Constraint values
  static const uint8_t MaxReportCells = 8;

  // Information Elements
  /// PlmnIdentityInfo structure
  struct PlmnIdentityInfo
  {
    uint32_t plmnIdentity; ///< PLMN identity
  };

  /// CellAccessRelatedInfo structure
  struct CellAccessRelatedInfo
  {
    PlmnIdentityInfo plmnIdentityInfo; ///< PLMN identity info
    uint32_t cellIdentity; ///< cell identity
    bool csgIndication; ///< CSG indication
    uint32_t csgIdentity; ///< CSG identity
  };

  /// CellSelectionInfo structure
  struct CellSelectionInfo
  {
    int8_t qRxLevMin; ///< INTEGER (-70..-22), actual value = IE value * 2 [dBm].
    int8_t qQualMin; ///< INTEGER (-34..-3), actual value = IE value [dB].
  };

  /// FreqInfo structure
  struct FreqInfo
  {
    uint32_t ulCarrierFreq; ///< UL carrier frequency
    uint8_t ulBandwidth; ///< UL bandwidth
  };

  /// RlcConfig structure
  struct RlcConfig
  {
    /// the direction choice
    enum direction
    {
      AM,
      UM_BI_DIRECTIONAL,
      UM_UNI_DIRECTIONAL_UL,
      UM_UNI_DIRECTIONAL_DL
    } choice; ///< direction choice
  };

  /// LogicalChannelConfig structure
  struct LogicalChannelConfig
  {
    uint8_t priority; ///< priority
    uint16_t prioritizedBitRateKbps; ///< prioritized bit rate Kbps
    uint16_t bucketSizeDurationMs; ///< bucket size duration ms
    uint8_t logicalChannelGroup; ///< logical channel group
  };

  /// SoundingRsUlConfigCommon structure
  struct SoundingRsUlConfigCommon
  {
    /// the config action
    enum action
    {
      SETUP, RESET
    } type; ///< action type
    uint8_t srsBandwidthConfig; ///< SRS bandwidth config
    uint8_t srsSubframeConfig; ///< SRS subframe config
  };

  /// SoundingRsUlConfigDedicated structure
  struct SoundingRsUlConfigDedicated
  {
    /// the config action
    enum action
    {
      SETUP, RESET
    } type; ///< action type
    uint8_t srsBandwidth; ///< SRS bandwidth
    uint16_t srsConfigIndex; ///< SRS config index
  };

  /// AntennaInfoDedicated structure
  struct AntennaInfoDedicated
  {
    uint8_t transmissionMode; ///< transmission mode
  };

  /// PdschConfigCommon structure
  struct PdschConfigCommon
  {
    int8_t referenceSignalPower;  ///< INTEGER (-60..50),
    int8_t pb;                    ///< INTEGER (0..3),
  };

  /// PdschConfigDedicated structure
  struct PdschConfigDedicated
  {
    /**
     * P_A values, TS 36.331 6.3.2 PDSCH-Config
     * ENUMERATED { dB-6, dB-4dot77, dB-3, dB-1dot77, dB0, dB1, dB2, dB3 }
     */
    enum db
    {
      dB_6,
      dB_4dot77,
      dB_3,
      dB_1dot77,
      dB0,
      dB1,
      dB2,
      dB3
    };
    uint8_t pa; ///< P_A value
  };

  /**
   * Convert PDSCH config dedicated function
   *
   * \param pdschConfigDedicated PdschConfigDedicated
   * \returns double value
   */
  static double ConvertPdschConfigDedicated2Double (PdschConfigDedicated pdschConfigDedicated)
  {
    double pa = 0;
    switch (pdschConfigDedicated.pa)
      {
      case PdschConfigDedicated::dB_6:
        pa = -6;
        break;
      case PdschConfigDedicated::dB_4dot77:
        pa = -4.77;
        break;
      case PdschConfigDedicated::dB_3:
        pa = -3;
        break;
      case PdschConfigDedicated::dB_1dot77:
        pa = -1.77;
        break;
      case PdschConfigDedicated::dB0:
        pa = 0;
        break;
      case PdschConfigDedicated::dB1:
        pa = 1;
        break;
      case PdschConfigDedicated::dB2:
        pa = 2;
        break;
      case PdschConfigDedicated::dB3:
        pa = 3;
        break;
      default:
        break;
      }
    return pa;
  }

  /// PhysicalConfigDedicated structure
  struct PhysicalConfigDedicated
  {
    bool haveSoundingRsUlConfigDedicated; ///< have sounding RS UL config dedicated?
    SoundingRsUlConfigDedicated soundingRsUlConfigDedicated; ///< sounding RS UL config dedicated
    bool haveAntennaInfoDedicated; ///< have antenna info dedicated?
    AntennaInfoDedicated antennaInfo; ///< antenna info
    bool havePdschConfigDedicated; ///< have PDSCH config dedicated?
    PdschConfigDedicated pdschConfigDedicated; ///< PDSCH config dedicated
  };


  /// SrbToAddMod structure
  struct SrbToAddMod
  {
    uint8_t srbIdentity; ///< SB identity
    LogicalChannelConfig logicalChannelConfig; ///< logical channel config
  };

  /// DrbToAddMod structure
  struct DrbToAddMod
  {
    uint8_t epsBearerIdentity; ///< EPS bearer identity
    uint8_t drbIdentity; ///< DRB identity
    RlcConfig rlcConfig; ///< RLC config
    uint8_t logicalChannelIdentity; ///< logical channel identify
    LogicalChannelConfig logicalChannelConfig; ///< logical channel config
  };

  /// PreambleInfo structure
  struct PreambleInfo
  {
    uint8_t numberOfRaPreambles; ///< number of RA preambles
  };

  /// RaSupervisionInfo structure
  struct RaSupervisionInfo
  {
    uint8_t preambleTransMax; ///< preamble transmit maximum
    uint8_t raResponseWindowSize; ///< RA response window size
  };

  /// RachConfigCommon structure
  struct RachConfigCommon
  {
    PreambleInfo preambleInfo; ///< preamble info
    RaSupervisionInfo raSupervisionInfo; ///< RA supervision info
  };

  /// RadioResourceConfigCommon structure
  struct RadioResourceConfigCommon
  {
    RachConfigCommon rachConfigCommon; ///< RACH config common
  };

  /// RadioResourceConfigCommonSib structure
  struct RadioResourceConfigCommonSib
  {
    RachConfigCommon rachConfigCommon; ///< RACH config common
    PdschConfigCommon pdschConfigCommon; ///< PDSCH config common
  };

  /// RadioResourceConfigDedicated structure
  struct RadioResourceConfigDedicated
  {
    std::list<SrbToAddMod> srbToAddModList; ///< SRB to add mod list
    std::list<DrbToAddMod> drbToAddModList; ///< DRB to add mod list
    std::list<uint8_t> drbToReleaseList; ///< DRB to release list
    bool havePhysicalConfigDedicated; ///< have physical config dedicated?
    PhysicalConfigDedicated physicalConfigDedicated; ///< physical config dedicated
  };

  /// QuantityConfig structure
  struct QuantityConfig
  {
    uint8_t filterCoefficientRSRP; ///< filter coefficient RSRP
    uint8_t filterCoefficientRSRQ; ///< filter coefficient RSRQ
  };

  /// CellsToAddMod structure
  struct CellsToAddMod
  {
    uint8_t cellIndex; ///< cell index
    uint16_t physCellId; ///< Phy cell ID
    int8_t cellIndividualOffset; ///< cell individual offset
  };

  /// PhysCellIdRange structure
  struct PhysCellIdRange
  {
    uint16_t start; ///< starting cell ID
    bool haveRange; ///< has a range?
    uint16_t range; ///< the range
  };

  /// BlackCellsToAddMod structure
  struct BlackCellsToAddMod
  {
    uint8_t cellIndex; ///< cell index
    PhysCellIdRange physCellIdRange; ///< Phy cell ID range
  };

  /// MeasObjectEutra structure
  struct MeasObjectEutra
  {
    uint32_t carrierFreq; ///< carrier frequency
    uint8_t allowedMeasBandwidth; ///< allowed measure bandwidth
    bool presenceAntennaPort1; ///< antenna port 1 present?
    uint8_t neighCellConfig; ///< neighbor cell config
    int8_t offsetFreq; ///< offset frequency
    std::list<uint8_t> cellsToRemoveList; ///< cells to remove list
    std::list<CellsToAddMod> cellsToAddModList; ///< cells to add mod list
    std::list<uint8_t> blackCellsToRemoveList; ///< black cells to remove list
    std::list<BlackCellsToAddMod> blackCellsToAddModList; ///< black cells to add mod list
    bool haveCellForWhichToReportCGI; ///< have cell for which to report CGI?
    uint16_t cellForWhichToReportCGI; ///< cell for which to report CGI
  };

  /**
   * \brief Threshold for event evaluation.
   *
   * For RSRP-based threshold, the actual value is (value - 140) dBm. While for
   * RSRQ-based threshold, the actual value is (value - 40) / 2 dB. This is in
   * accordance with section 9.1.4 and 9.1.7 of 3GPP TS 36.133.
   *
   * \sa ns3::cv2x_EutranMeasurementMapping
   */
  struct ThresholdEutra
  {
    /// Threshold enumeration
    enum
    {
      THRESHOLD_RSRP, ///< RSRP is used for the threshold.
      THRESHOLD_RSRQ ///< RSRQ is used for the threshold.
    } choice;
    uint8_t range; ///< Value range used in RSRP/RSRQ threshold.
  };

  /// Specifies criteria for triggering of an E-UTRA measurement reporting event.
  struct ReportConfigEutra
  {
    /// Trigger enumeration
    enum
    {
      EVENT,      ///< event report
      PERIODICAL  ///< periodical report
    } triggerType; ///< trigger type

    /// Event enumeration
    enum
    {
      EVENT_A1, ///< Event A1: Serving becomes better than absolute threshold.
      EVENT_A2, ///< Event A2: Serving becomes worse than absolute threshold.
      EVENT_A3, ///< Event A3: Neighbour becomes amount of offset better than PCell.
      EVENT_A4, ///< Event A4: Neighbour becomes better than absolute threshold.
      EVENT_A5  ///< Event A5: PCell becomes worse than absolute `threshold1` AND Neighbour becomes better than another absolute `threshold2`.

    } eventId; ///< Choice of E-UTRA event triggered reporting criteria.

    ThresholdEutra threshold1; ///< Threshold for event A1, A2, A4, and A5.
    ThresholdEutra threshold2; ///< Threshold for event A5.

    /// Indicates whether or not the UE shall initiate the measurement reporting procedure when the leaving condition is met for a cell in `cellsTriggeredList`, as specified in 5.5.4.1 of 3GPP TS 36.331.
    bool reportOnLeave;

    /// Offset value for Event A3. An integer between -30 and 30. The actual value is (value * 0.5) dB.
    int8_t a3Offset;

    /// Parameter used within the entry and leave condition of an event triggered reporting condition. The actual value is (value * 0.5) dB.
    uint8_t hysteresis;

    /// Time during which specific criteria for the event needs to be met in order to trigger a measurement report.
    uint16_t timeToTrigger;

    /// the report purpose
    enum report
    {
      REPORT_STRONGEST_CELLS,
      REPORT_CGI
    } purpose; ///< purpose

    /// Trigger type enumeration
    enum
    {
      RSRP, ///< Reference Signal Received Power
      RSRQ ///< Reference Signal Received Quality
    } triggerQuantity; ///< The quantities used to evaluate the triggering condition for the event, see 3GPP TS 36.214.

    /// Report type enumeration
    enum
    {
      SAME_AS_TRIGGER_QUANTITY,
      BOTH ///< Both the RSRP and RSRQ quantities are to be included in the measurement report.
    } reportQuantity; ///< The quantities to be included in the measurement report, always assumed to be BOTH.

    /// Maximum number of cells, excluding the serving cell, to be included in the measurement report.
    uint8_t maxReportCells;

    /// Report interval enumeration
    enum
    {
      MS120,
      MS240,
      MS480,
      MS640,
      MS1024,
      MS2048,
      MS5120,
      MS10240,
      MIN1,
      MIN6,
      MIN12,
      MIN30,
      MIN60,
      SPARE3,
      SPARE2,
      SPARE1
    } reportInterval; ///< Indicates the interval between periodical reports.

    /// Number of measurement reports applicable, always assumed to be infinite.
    uint8_t reportAmount;

    /// Report config eutra function
    ReportConfigEutra ();

  }; // end of struct ReportConfigEutra

  /// MeasObjectToAddMod structure
  struct MeasObjectToAddMod
  {
    uint8_t measObjectId; ///< measure object ID
    MeasObjectEutra measObjectEutra; ///< measure object eutra
  };

  /// ReportConfigToAddMod structure
  struct ReportConfigToAddMod
  {
    uint8_t reportConfigId; ///< report config ID
    ReportConfigEutra reportConfigEutra; ///< report config eutra
  };

  /// MeasIdToAddMod structure
  struct MeasIdToAddMod
  {
    uint8_t measId; ///< measure ID
    uint8_t measObjectId; ///< measure object ID
    uint8_t reportConfigId; ///< report config ID
  };

  /// MeasGapConfig structure
  struct MeasGapConfig
  {
    /// the action type
    enum action
    {
      SETUP, RESET
    } type; ///< action type
    /// the gap offest
    enum gap
    {
      GP0, GP1
    } gapOffsetChoice; ///< gap offset
    uint8_t gapOffsetValue; ///< gap offset value
  };

  /// MobilityStateParameters structure
  struct MobilityStateParameters
  {
    uint8_t tEvaluation; ///< evaluation
    uint8_t tHystNormal; ///< hyst normal
    uint8_t nCellChangeMedium; ///< cell change medium
    uint8_t nCellChangeHigh; ///< cell change high
  };

  /// SpeedStateScaleFactors structure
  struct SpeedStateScaleFactors
  {
    // 25 = oDot25, 50 = oDot5, 75 = oDot75, 100 = lDot0
    uint8_t sfMedium; ///< scale factor medium
    uint8_t sfHigh; ///< scale factor high
  };

  /// SpeedStatePars structure
  struct SpeedStatePars
  {
    /// the action type
    enum action
    {
      SETUP,
      RESET
    } type; ///< action type
    MobilityStateParameters mobilityStateParameters; ///< mobility state parameters
    SpeedStateScaleFactors timeToTriggerSf; ///< time to trigger scale factors
  };

  /// MeasConfig structure
  struct MeasConfig
  {
    std::list<uint8_t> measObjectToRemoveList; ///< measure object to remove list
    std::list<MeasObjectToAddMod> measObjectToAddModList; ///< measure object to add mod list
    std::list<uint8_t> reportConfigToRemoveList; ///< report config to remove list
    std::list<ReportConfigToAddMod> reportConfigToAddModList; ///< report config to add mod list
    std::list<uint8_t> measIdToRemoveList; ///< measure ID to remove list
    std::list<MeasIdToAddMod> measIdToAddModList; ///< measure ID to add mod list
    bool haveQuantityConfig; ///< have quantity config?
    QuantityConfig quantityConfig; ///< quantity config
    bool haveMeasGapConfig; ///< have measure gap config?
    MeasGapConfig measGapConfig; ///< measure gap config
    bool haveSmeasure; ///< have S measure?
    uint8_t sMeasure; ///< S measure
    bool haveSpeedStatePars; ///< have speed state parameters?
    SpeedStatePars speedStatePars; ///< speed state parameters
  };

  /// CarrierFreqEutra structure
  struct CarrierFreqEutra
  {
    uint32_t dlCarrierFreq; ///< DL carrier frequency
    uint32_t ulCarrierFreq; ///< UL carrier frequency
  };

  /// CarrierBandwidthEutra structure
  struct CarrierBandwidthEutra
  {
    uint8_t dlBandwidth; ///< DL bandwidth
    uint8_t ulBandwidth; ///< UL bandwidth
  };

  /// RachConfigDedicated structure
  struct RachConfigDedicated
  {
    uint8_t raPreambleIndex; ///< RA preamble index
    uint8_t raPrachMaskIndex; ///< RA PRACH mask index
  };

  /// MobilityControlInfo structure
  struct MobilityControlInfo
  {
    uint16_t targetPhysCellId; ///< target Phy cell ID
    bool haveCarrierFreq; ///< have carrier frequency?
    CarrierFreqEutra carrierFreq; ///< carrier frequency
    bool haveCarrierBandwidth; ///< have carrier bandwidth?
    CarrierBandwidthEutra carrierBandwidth; ///< carrier bandwidth
    uint16_t newUeIdentity; ///< new UE identity
    RadioResourceConfigCommon radioResourceConfigCommon; ///< radio resource config common
    bool haveRachConfigDedicated; ///< Have RACH config dedicated?
    RachConfigDedicated rachConfigDedicated; ///< RACH config dedicated
  };

  /// ReestabUeIdentity structure
  struct ReestabUeIdentity
  {
    uint16_t cRnti; ///< RNTI
    uint16_t physCellId; ///< Phy cell ID
  };

  /// ReestablishmentCause enumeration
  enum ReestablishmentCause
  {
    RECONFIGURATION_FAILURE,
    HANDOVER_FAILURE,
    OTHER_FAILURE
  };

  /// MasterInformationBlock structure
  struct MasterInformationBlock
  {
    uint8_t dlBandwidth; ///< DL bandwidth
    uint8_t systemFrameNumber; ///< system frame number
  };

  /// SystemInformationBlockType1 structure
  struct SystemInformationBlockType1
  {
    CellAccessRelatedInfo cellAccessRelatedInfo; ///< cell access related info
    CellSelectionInfo cellSelectionInfo; ///< cell selection info
  };

  /// SystemInformationBlockType2 structure
  struct SystemInformationBlockType2
  {
    RadioResourceConfigCommonSib radioResourceConfigCommon; ///< radio resource config common
    FreqInfo freqInfo; ///< frequency info
  };

  /* NIST: Information Elements for SystemInformationBlockType18 */
#define MAXSL_DEST 16
#define MAXSL_TF_INDEXPAIR 64
#define MAXSL_TXPOOL 4
#define MAXSL_RXPOOL 16
#define MAXSL_DEST 16

#define MAXSL_DISC_POWERCLASS 3

#define MAX_PLMN 6

  /* CNI: Information Elements for V2X*/
#define MAXSL_V2X_RXPOOL 16
#define MAXSL_V2X_RXPOOLPRECONF 16
#define MAXSL_V2X_TXPOOL 8
#define MAXSL_V2X_TXPOOLPRECONF 8
#define MAXFREQ_V2X 8 
#define MAXFREQ_V2X_1 7
#define MAXCELLINTRA 16
#define MAXSL_PRIO 8
#define MAXPSSCH_TXCONFIG 16
#define MAXRESERVERATIONPERIOD 16
#define MAXSL_V2X_CBRCONFIG 4
#define MAXSL_V2X_TXCONFIG 64
#define MAXSL_V2X_TXCONFIG2 128
#define MAXCBR_LEVEL_1 15
#define MAXSL_V2X_CBRCONFIG2 8
#define MAXSL_V2X_CBRCONFIG2_1 7
#define MAXCBR_LEVEL 16
#define MAXSL_V2X_SYNCCONFIG 16
#define MAXSL_DISCCELLS 16
#define MAXEARFCN2 262143
#define MAXSL_POOLTOMEASURE 72
#define MAXLCG 4
  
  struct SlCpLen {
    enum {
      NORMAL,
      EXTENDED
    } cplen;
  };

  struct SlPeriodComm {
    enum
      {
        sf40,
        sf60,
        sf70,
        sf80,
        sf120,
        sf140,
        sf160,
        sf240,
        sf280,
        sf320
      } period;
  };


  static SlPeriodComm PeriodAsEnum (uint32_t p_length)
  {
    SlPeriodComm p;
    switch (p_length)
      {
      case 40:
        p.period = SlPeriodComm::sf40;
        break;
      case 60:
        p.period = SlPeriodComm::sf60;
        break;
      case 70:
        p.period = SlPeriodComm::sf70;
        break;
      case 80: 
        p.period = SlPeriodComm::sf80;
        break;
      case 120:
        p.period = SlPeriodComm::sf120;
        break;
      case 140:
        p.period = SlPeriodComm::sf140;
        break;
      case 160:
        p.period = SlPeriodComm::sf160;
        break;
      case 240:
        p.period = SlPeriodComm::sf240;
        break;
      case 280:
        p.period = SlPeriodComm::sf280;
        break;
      case 320:
        p.period = SlPeriodComm::sf320;
        break;
      default:
        NS_FATAL_ERROR("SL PERIOD LENGTH NOT SUPPORTED: "<< p_length);
      }

      return p;
  }
  static uint32_t PeriodAsInt (SlPeriodComm period)
  {
    uint32_t p = 0;
    switch (period.period)
      {
      case SlPeriodComm::sf40:
        p = 40;
        break;
      case SlPeriodComm::sf60:
        p = 60;
        break;
      case SlPeriodComm::sf70:
        p = 70;
        break;
      case SlPeriodComm::sf80:
        p = 80;
        break;
      case SlPeriodComm::sf120:
        p = 120;
        break;
      case SlPeriodComm::sf140:
        p = 140;
        break;
      case SlPeriodComm::sf160:
        p = 160;
        break;
      case SlPeriodComm::sf240:
        p = 240;
        break;
      case SlPeriodComm::sf280:
        p = 280;
        break;
      case SlPeriodComm::sf320:
        p = 320;
        break;        
      }

      return p;
  }

  //Offset of the pool of resource relative to SFN 0 of the cell or DFN 0 when out of coverage
  struct SlOffsetIndicator {
    uint16_t offset; //MAx is 319 for communication, 10239 for discovery
  };

  struct SubframeBitmapSl {
    std::bitset<40> bitmap; //40 bits for FDD
  };
  
  /**
   * SL-TF-ResourceConfig
   * The IE SL-TF-ResourceConfig specifies a set of time/ frequency resources used for sidelink.
   */
  struct SlTfResourceConfig {
    uint16_t prbNum;
    uint16_t prbStart;
    uint16_t prbEnd;
    SlOffsetIndicator offsetIndicator;
    SubframeBitmapSl subframeBitmap;

    friend bool operator==(const SlTfResourceConfig& lhs, const SlTfResourceConfig& rhs)
    {
      return lhs.prbNum == rhs.prbNum && lhs.prbStart == rhs.prbStart && lhs.prbEnd == rhs.prbEnd
        && lhs.offsetIndicator.offset == rhs.offsetIndicator.offset && lhs.subframeBitmap.bitmap == rhs.subframeBitmap.bitmap;
    }
  };

  struct SlHoppingConfigComm {
    uint8_t hoppingParameters; //valid range 0..504
    enum {
      ns1,
      ns2,
      ns4,
    } numSubbands;
    uint8_t rbOffset; //valid range 0..110

    friend bool operator==(const SlHoppingConfigComm& lhs, const SlHoppingConfigComm& rhs)
    {
      return lhs.numSubbands == rhs.numSubbands && lhs.rbOffset == rhs.rbOffset;
    }
  };

  struct SlTrptSubset {
    std::bitset<3> subset; //3 bits for FDD to indicate the set of k values
  };

  struct P0Sl {
  int16_t p0; //INTEGER (-126..31)
  };

  struct SlTxParameters {
    enum AlphaEnum {
      al0,
      al04,
      al05,
      al06,
      al07,
      al08,
      al09,
      al1
    };
    AlphaEnum alpha;
    int16_t p0; //valid range -126..31

    friend bool operator==(const SlTxParameters& lhs, const SlTxParameters& rhs)
    {
      return lhs.alpha == rhs.alpha && lhs.p0 == rhs.p0;
    }
  };

  static double AlphaAsDouble (SlTxParameters p) {
    double alpha = 0;
    switch (p.alpha) {
    case SlTxParameters::al0:
      alpha = 0;
      break;
    case SlTxParameters::al04:
      alpha = 0.4;
      break;
    case SlTxParameters::al05:
      alpha = 0.5;
      break;
    case SlTxParameters::al06:
      alpha = 0.6;
      break;
    case SlTxParameters::al07:
      alpha = 0.7;
      break;
    case SlTxParameters::al08:
      alpha = 0.8;
      break;
    case SlTxParameters::al09:
      alpha = 0.9;
      break;
    case SlTxParameters::al1:
      alpha = 1;
      break;
    }
    return alpha;
  };

  static SlTxParameters::AlphaEnum AlphaFromDouble (double p) 
  {
    SlTxParameters::AlphaEnum alpha; 
    switch ((int) p*10)
    {
      case 0:
        alpha = SlTxParameters::al0;
        break;
      case 4: 
        alpha = SlTxParameters::al04;
        break;
      case 5: 
        alpha = SlTxParameters::al05;
        break;
      case 6: 
        alpha = SlTxParameters::al06;
        break;
      case 7: 
        alpha = SlTxParameters::al07;
        break;                
      case 8: 
        alpha = SlTxParameters::al08;
        break;   
      case 9: 
        alpha = SlTxParameters::al09;
        break;
      case 10: 
        alpha = SlTxParameters::al1;
        break;
      default:
        NS_FATAL_ERROR ("UNSUPPORTED DATA TX ALPHA: " << p);
        break;    
    }
    return alpha; 
  };


  struct SlCommResourcePool {
    SlCpLen scCpLen;
    SlPeriodComm scPeriod;
    SlTfResourceConfig scTfResourceConfig;
    SlCpLen dataCpLen;
    SlHoppingConfigComm dataHoppingConfig;
    bool haveUeSelectedResourceConfig;
    struct 
      {
        SlTfResourceConfig dataTfResourceConfig;
        bool haveTrptSubset;
        SlTrptSubset trptSubset; //optional
      } ueSelectedResourceConfig;
    //rxParametersNCell not specified yet
    bool haveTxParameters; //mandatory present when included in commTxPoolNormalDedicated, commTxPoolNormalCommon or commTxPoolExceptional
    struct
      {
        SlTxParameters scTxParameters;
        SlTxParameters dataTxParameters;
      } txParameters;

    friend bool operator==(const SlCommResourcePool& lhs, const SlCommResourcePool& rhs)
      {
        bool equal = lhs.scCpLen.cplen == rhs.scCpLen.cplen
          && lhs.scPeriod.period == rhs.scPeriod.period
          && lhs.scTfResourceConfig == rhs.scTfResourceConfig
          && lhs.dataCpLen.cplen == rhs.dataCpLen.cplen
          && lhs.dataHoppingConfig == rhs.dataHoppingConfig
          && lhs.haveUeSelectedResourceConfig == rhs.haveUeSelectedResourceConfig;
        if (equal && lhs.haveUeSelectedResourceConfig) {
          equal = equal && lhs.ueSelectedResourceConfig.dataTfResourceConfig == rhs.ueSelectedResourceConfig.dataTfResourceConfig
            && lhs.ueSelectedResourceConfig.haveTrptSubset == rhs.ueSelectedResourceConfig.haveTrptSubset;
          if (equal && lhs.ueSelectedResourceConfig.haveTrptSubset) {
            equal = equal && lhs.ueSelectedResourceConfig.trptSubset.subset == rhs.ueSelectedResourceConfig.trptSubset.subset;
          }
        }
        equal = equal && lhs.haveTxParameters == rhs.haveTxParameters;
        if (equal && lhs.haveTxParameters) {
          equal = equal && lhs.txParameters.scTxParameters == rhs.txParameters.scTxParameters
            && lhs.txParameters.dataTxParameters == rhs.txParameters.dataTxParameters;
        }
        return equal;
      }
    
  };
  
  struct SlCommTxPoolList {
    uint8_t nbPools;
    SlCommResourcePool pools[MAXSL_TXPOOL];
  };

  struct SlCommRxPoolList {
    uint8_t nbPools;
    SlCommResourcePool pools[MAXSL_RXPOOL];
  };

  struct SlSyncConfigList {
    uint8_t nbConfig;
  };

  struct Sib18CommConfig { //SlCommConfig struct used in RRC Connection
    SlCommRxPoolList commRxPool;
    SlCommTxPoolList commTxPoolNormalCommon; //Optional (number of pools may be 0)
    SlCommTxPoolList commTxPoolExceptional;  //Optional (number of pools may be 0)
    SlSyncConfigList commSyncConfig;         //Optional (number of pools may be 0)
  };
  
  struct SystemInformationBlockType18
  {
    Sib18CommConfig commConfig;
  };

  struct SlPreconfigGeneral {
    uint16_t carrierFreq; //ulEarfcn
    uint16_t slBandwidth;  //nb RBs
  };
  
  /*
   * \brief Structure representing the Master Information Block Sidelink to be sent by SyncRefs
   */
  struct MasterInformationBlockSL
  {
    uint16_t  slBandwidth;  	///< Sidelink bandwidth[RBs]
    uint16_t  directFrameNo;	///< Frame number of the frame in which is sent this MIB-SL
    uint16_t  directSubframeNo;	///< Subframe number of the subframe in which is sent this MIB-SL
    bool    inCoverage;			///< Indicates if the SyncRef sending the MIB-SL is in coverage
    uint64_t  slssid;			///< SLSSID of the SyncRef sending the MIB-SL
    Time rxTimestamp;			///< Reception timestamp filled upon reception
    Time creationTimestamp;		///< Creation timestamp filled when created
    uint16_t rxOffset;			///< Reception offset
  };

  /*
   * \brief Structure representing the Sidelink synchronization preconfigured parameters
   * to be used in the out-of-coverage case
   */
  struct SlPreconfigSync
  {
    SlCpLen syncCPLen; 				///< Cyclic prefix length
    uint16_t syncOffsetIndicator1;  ///< First offset indicator for transmission of SLSSs. Valid values: 0 ... 39
    uint16_t syncOffsetIndicator2;  ///< Second offset indicator for transmission of SLSSs. Valid values: 0 ... 39
    int16_t syncTxThreshOoC;  		///< Threshold representing inner cell boundaries of a detected SyncRef. Unit [dBm]. Valid values: -110, -105, -100, and so on (i.e. in steps of 5), -60 (+inf not represented)
    uint16_t filterCoefficient; 	///< Filter coefficient for L3 filtering. Valid values (k): 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 15, 17, 19
    uint16_t syncRefMinHyst;    	///< Threshold representing how higher than the minimum required the SyncRef S-RSRP should be to consider it detected. Unit [dB] Valid values: 0, 3, 6, 9, 12
    uint16_t syncRefDiffHyst;   	///< Threshold representing how higher than the (currently) selected SyncRef S-RSRP the S-RSRP of a newly detected SyncRef should be to consider a change. Unit[dB] Valid values: 0, 3, 6, 9, 12 (+inf not represented)
  };

  struct SlPreconfigCommPool { //Same as SlCommResourcePool with rxParametersNCell absent
    SlCpLen scCpLen;
    SlPeriodComm scPeriod;
    SlTfResourceConfig scTfResourceConfig;
    SlTxParameters scTxParameters;
    SlCpLen dataCpLen;
    SlHoppingConfigComm dataHoppingConfig;
    SlTfResourceConfig dataTfResourceConfig;
    SlTrptSubset trptSubset; //optional
    SlTxParameters dataTxParameters;
  };
  
  struct SlPreconfigCommPoolList {
    uint8_t nbPools;
    SlPreconfigCommPool pools[MAXSL_TXPOOL];
  };

  ///// BEGIN V2X /////
  struct AdjacencyPscchPssch{
    bool adjacency;
  };
  
  static bool adjacencyAsBool(AdjacencyPscchPssch pAdjacency)
  {
    return pAdjacency.adjacency; 
  }

struct cv2x_StartRbSubchannel{
    uint16_t startRb; 
  };

  static uint16_t startRbSubchannelAsInt(cv2x_StartRbSubchannel pStart)
  {
    return pStart.startRb; 
  }

  struct StartRbPscchPool{
    uint16_t startRb; 
  };

  static uint16_t startRbPscchPoolAsInt(StartRbPscchPool pStart)
  {
    return pStart.startRb; 
  }



struct cv2x_NumSubchannel{
    enum{
      n1,
      n3,
      n5, 
      n8, 
      n10, 
      n15, 
      n20, 
      spare1
    } num; 
  };

  static int numSubchannelAsInt (cv2x_NumSubchannel pNum) {
    int num = 0;
    switch (pNum.num) {
    case cv2x_NumSubchannel::n1:
      num = 1;
      break;
    case cv2x_NumSubchannel::n3:
      num = 3;
      break;
    case cv2x_NumSubchannel::n5:
      num = 5;
      break;
    case cv2x_NumSubchannel::n8:
      num = 8;
      break;
    case cv2x_NumSubchannel::n10:
      num = 10;
      break;
    case cv2x_NumSubchannel::n15:
      num = 15;
      break;
    case cv2x_NumSubchannel::n20:
      num = 20;
      break;
    // not supported yet
    case cv2x_NumSubchannel::spare1:
      num = 0;
      break;
    }
    return num;
  };

  static cv2x_NumSubchannel cv2x_NumSubchannelFromInt (uint16_t p) 
  {
    cv2x_NumSubchannel numSubchannel; 
    switch (p)
    {
      case 1:
        numSubchannel.num = cv2x_NumSubchannel::n1;
        break;
      case 3: 
        numSubchannel.num = cv2x_NumSubchannel::n3;
        break;
      case 5: 
        numSubchannel.num = cv2x_NumSubchannel::n5;
        break;
      case 8: 
        numSubchannel.num = cv2x_NumSubchannel::n8;
        break;
      case 10: 
        numSubchannel.num = cv2x_NumSubchannel::n10;
        break;                
      case 15: 
        numSubchannel.num = cv2x_NumSubchannel::n15;
        break;   
      case 20: 
        numSubchannel.num = cv2x_NumSubchannel::n20;
        break;
      default:
        NS_FATAL_ERROR ("UNSUPPORTED SUBCHANNEL NUMBER: " << p);
        break;    
    }
    return numSubchannel; 
  };

  struct SizeSubchannel{
    enum{
      n4,
      n5, 
      n6, 
      n8, 
      n9, 
      n10, 
      n12, 
      n15, 
      n16, 
      n18, 
      n20, 
      n25, 
      n30,
      n48, 
      n50, 
      n72, 
      n75, 
      n96, 
      n100, 
      spare13, 
      spare12, 
      spare11,
      spare10, 
      spare9, 
      spare8, 
      spare7, 
      spare6, 
      spare5, 
      spare4,
      spare3, 
      spare2, 
      spare1
    }size; 
  };

  static int sizeSubchannelAsInt (SizeSubchannel pSize) {
    int size = 0;
    switch (pSize.size) {
    case SizeSubchannel::n4:
      size = 4;
      break;
    case SizeSubchannel::n5:
      size = 5;
      break;
    case SizeSubchannel::n6:
      size = 6;
      break;
    case SizeSubchannel::n8:
      size = 8;
      break;
    case SizeSubchannel::n9:
      size = 9;
      break;
    case SizeSubchannel::n10:
      size = 10;
      break;
    case SizeSubchannel::n12:
      size = 12;
      break;
    case SizeSubchannel::n15:
      size = 15;
      break;
    case SizeSubchannel::n16:
      size = 16;
      break;
    case SizeSubchannel::n18:
      size = 18;
      break;
    case SizeSubchannel::n20:
      size = 20;
      break;
    case SizeSubchannel::n25:
      size = 25;
      break;
    case SizeSubchannel::n30:
      size = 30;
      break;
    case SizeSubchannel::n48:
      size = 48;
      break;
    case SizeSubchannel::n50:
      size = 50;
      break;
    case SizeSubchannel::n72:
      size = 72;
      break;
    case SizeSubchannel::n75:
      size = 75;
      break;
    case SizeSubchannel::n96:
      size = 96;
      break;
    case SizeSubchannel::n100:
      size = 100;
      break;
    // not supported yet 
    case SizeSubchannel::spare13:
      size = 0;
      break;
    case SizeSubchannel::spare12:
      size = 0;
      break;
    case SizeSubchannel::spare11:
      size = 0;
      break;
    case SizeSubchannel::spare10:
      size = 0;
      break;
    case SizeSubchannel::spare9:
      size = 0;
      break;
    case SizeSubchannel::spare8:
      size = 0;
      break;
    case SizeSubchannel::spare7:
      size = 0;
      break;
    case SizeSubchannel::spare6:
      size = 0;
      break;
    case SizeSubchannel::spare5:
      size = 0;
      break;
    case SizeSubchannel::spare4:
      size = 0;
      break;
    case SizeSubchannel::spare3:
      size = 0;
      break;
    case SizeSubchannel::spare2:
      size = 0;
      break;
    case SizeSubchannel::spare1:
      size = 0;
      break;
    }
    return size;
  };

  static SizeSubchannel SizeSubchannelFromInt (uint16_t p) 
  {
    SizeSubchannel sizeSubchannel; 
    switch (p)
    {
      case 4:
        sizeSubchannel.size = SizeSubchannel::n4;
        break;
      case 5: 
        sizeSubchannel.size = SizeSubchannel::n5;
        break;
      case 6: 
        sizeSubchannel.size = SizeSubchannel::n6;
        break;
      case 8: 
        sizeSubchannel.size = SizeSubchannel::n8;
        break;
      case 9: 
        sizeSubchannel.size = SizeSubchannel::n9;
        break;                
      case 10: 
        sizeSubchannel.size = SizeSubchannel::n10;
        break;   
      case 12: 
        sizeSubchannel.size = SizeSubchannel::n12;
        break;    
      case 15:
        sizeSubchannel.size = SizeSubchannel::n15;
        break;
      case 16: 
        sizeSubchannel.size = SizeSubchannel::n16;
        break;
      case 18: 
        sizeSubchannel.size = SizeSubchannel::n18;
        break;
      case 20: 
        sizeSubchannel.size = SizeSubchannel::n20;
        break;
      case 25: 
        sizeSubchannel.size = SizeSubchannel::n25;
        break;                
      case 30: 
        sizeSubchannel.size = SizeSubchannel::n30;
        break;   
      case 48: 
        sizeSubchannel.size = SizeSubchannel::n48;
        break;      
      case 50:
        sizeSubchannel.size = SizeSubchannel::n50;
        break;
      case 72: 
        sizeSubchannel.size = SizeSubchannel::n72;
        break;
      case 75: 
        sizeSubchannel.size = SizeSubchannel::n75;
        break;
      case 96: 
        sizeSubchannel.size = SizeSubchannel::n96;
        break;
      case 100: 
        sizeSubchannel.size = SizeSubchannel::n100;
        break; 
      default:
        NS_FATAL_ERROR ("UNSUPPORTED SUBCHANNEL SIZE: " << p);                                     
    }
    return sizeSubchannel; 
  };

  struct SlV2xTxPoolReportIdentity{
    uint8_t ident; // INTEGER (0..maxSL-PoolToMeasure-r14)
  };

  struct ThreshSrssiCbr{
    uint8_t thresh; // INTEGER (0..45)
  }; 

  struct ZoneId{
    uint16_t id; // INTEGER (0..7)
  };

  struct RxParametersNCell{
    //TddConfig tddConfig; 
    uint8_t syncConfigIndex; // INTEGER (0..15)
  }; 

  struct ZoneLength{
    enum{
      m5, 
      m10,
      m20,
      m50,
      m100,
      m200,
      m500,
      spare1
    }length; 
  };

  static int ZoneLengthAsInt (ZoneLength pLength) {
    int length = 0;
    switch (pLength.length) {
    case ZoneLength::m5:
      length = 5;
      break;
    case ZoneLength::m10:
      length = 10;
      break;
    case ZoneLength::m20:
      length = 20;
      break;
    case ZoneLength::m50:
      length = 50;
      break;
    case ZoneLength::m100:
      length = 100;
      break;
    case ZoneLength::m200:
      length = 20;
      break;
    case ZoneLength::m500:
      length = 500;
      break;
    // not supported yet
    case ZoneLength::spare1:
      length = 0;
      break;
    }
    return length;
  };

  struct ZoneWidth{
    enum{
      m5, 
      m10,
      m20,
      m50,
      m100,
      m200,
      m500,
      spare1
    }width; 
  };

  static int ZoneWidthAsInt (ZoneWidth pWidth) {
    int width = 0;
    switch (pWidth.width) {
    case ZoneWidth::m5:
      width = 5;
      break;
    case ZoneWidth::m10:
      width = 10;
      break;
    case ZoneWidth::m20:
      width = 20;
      break;
    case ZoneWidth::m50:
      width = 50;
      break;
    case ZoneWidth::m100:
      width = 100;
      break;
    case ZoneWidth::m200:
      width = 20;
      break;
    case ZoneWidth::m500:
      width = 500;
      break;
    // not supported yet
    case ZoneWidth::spare1:
      width = 0;
      break;
    }
    return width;
  };


  struct ThresUeSpeed{
    enum{
      kmph60,
      kmph80,
      kmph100, 
      kmph120, 
      kmph140,
      kmph160, 
      kmph180,
      kmph200
    } speed; 
  };

  static int ThresUeSpeedAsInt (ThresUeSpeed thres){
    uint8_t speed = 0;
    switch (thres.speed){
    case ThresUeSpeed::kmph60:
      speed = 60; 
      break; 
    case ThresUeSpeed::kmph80:
      speed = 80; 
      break; 
    case ThresUeSpeed::kmph100:
      speed = 100; 
      break; 
    case ThresUeSpeed::kmph120:
      speed = 120; 
      break; 
    case ThresUeSpeed::kmph140:
      speed = 140; 
      break; 
    case ThresUeSpeed::kmph160:
      speed = 160; 
      break; 
    case ThresUeSpeed::kmph180:
      speed = 180; 
      break; 
    case ThresUeSpeed::kmph200:
      speed = 200; 
      break; 
    } 
    return speed;    
  }

  struct AllowedRetxNumberPssch{
    enum{
      n0,
      n1,
      both,
      spare1
    }num;
  };

  static int AllowedReTxNumAsInt (AllowedRetxNumberPssch pNum) {
    int num = 0;
    switch (pNum.num) {
    case AllowedRetxNumberPssch::n0:
      num = 0;
      break;
    case AllowedRetxNumberPssch::n1:
      num = 1;
      break;
    // not supported yet 
    case AllowedRetxNumberPssch::both:
      num = 0;
      break;
    case AllowedRetxNumberPssch::spare1:
      num = 0;
      break;
    }
    return num;
  };

  struct SlReselectAfter{
    enum{
      n1,
      n2,
      n3,
      n4,
      n5,
      n6,
      n7,
      n8,
      n9,
      spare7,
      spare6,
      spare5,
      spare4,
      spare3,
      spare2,
      spare1
    }num; 
  };

  static int ReselectAfterAsInt (SlReselectAfter pNum) {
    int num = 0;
    switch (pNum.num) {
    case SlReselectAfter::n1:
      num = 1;
      break;
    case SlReselectAfter::n2:
      num = 2;
      break;
    case SlReselectAfter::n3:
      num = 3;
      break;
    case SlReselectAfter::n4:
      num = 4;
      break;
    case SlReselectAfter::n5:
      num = 5;
      break;
    case SlReselectAfter::n6:
      num = 6;
      break;
    case SlReselectAfter::n7:
      num = 7;
      break;
    case SlReselectAfter::n8:
      num = 8;
      break;
      case SlReselectAfter::n9:
      num = 9;
      break;
    // not supported yet 
    case SlReselectAfter::spare7:
      num = 0;
      break;
    case SlReselectAfter::spare6:
      num = 0;
      break;
    case SlReselectAfter::spare5:
      num = 0;
      break;
    case SlReselectAfter::spare4:
      num = 0;
      break;
    case SlReselectAfter::spare3:
      num = 0;
      break;
    case SlReselectAfter::spare2:
      num = 0;
      break;
    case SlReselectAfter::spare1:
      num = 0;
      break;
    }
    return num;
  }

  struct ProbResourceKeep{
    enum{
      v0, 
      v0dot2, 
      v0dot4,
      v0dot6,
      v0dot8,
      spare3,
      spare2,
      spare1
    }prob; 
  };

  static double ProbResourceKeepAsInt (ProbResourceKeep pProb) {
    double prob = 0;
    switch (pProb.prob) {
    case ProbResourceKeep::v0:
      prob = 0;
      break;
    case ProbResourceKeep::v0dot2:
      prob = 0.2;
      break;
    case ProbResourceKeep::v0dot4:
      prob = 0.4;
      break;
    case ProbResourceKeep::v0dot6:
      prob = 0.6;
      break;
    case ProbResourceKeep::v0dot8:
      prob = 0.8;
      break;
    // not supported yet 
    case ProbResourceKeep::spare3:
      prob = 0;
      break;
    case ProbResourceKeep::spare2:
      prob = 0;
      break;
    case ProbResourceKeep::spare1:
      prob = 0;
      break;
    }
    return prob;
  }

  struct SlBandwidth{
    enum{
      n6,
      n15,
      n25,
      n50,
      n75,
      n100
    } bandwidth; 
  }; 

  static uint16_t SlBandwidthAsInt(SlBandwidth pBandwidth)
  {
    uint16_t bandwidth; 
    switch(pBandwidth.bandwidth){
    case SlBandwidth::n6:
      bandwidth = 6;
      break;
    case SlBandwidth::n15:
      bandwidth = 15;
      break;
    case SlBandwidth::n25:
      bandwidth = 25;
      break;
    case SlBandwidth::n50:
      bandwidth = 50;
      break;
    case SlBandwidth::n75:
      bandwidth = 75;
      break;
    case SlBandwidth::n100:
      bandwidth = 100;
      break;
    }
    return bandwidth; 
  }

  struct OffsetDFN{
    uint16_t offset; 
  };


  struct SyncRefMinHyst{
    enum {
      dB0,
      dB3,
      dB6,
      dB9,
      dB12
    } sync; 
  };

  struct SyncRefDiffHyst{
    enum {
      dB0,
      dB3,
      dB6,
      dB9,
      dB12,
      dBinf
    } sync; 
  };

  ///// BEGIN Sidelink information elements 3GPP 36.331, 6.3.8, V15.0.1 ///// 

  struct ARFCNValueEUTRA {
    uint32_t value; // INTEGER (0..maxEARFCN2)
  };

  /**
   * The IE SL-AnchorCarrierFreqList
   * The IE SL-AnchorCarrierFreqList-V2X specifies the SL V2X anchor frequencies i.e. frequencies that include intercarrier
   * resource configuration for V2X sidelink communication.
   */
  struct SlAnchorCarrierFreqListV2x{
    uint8_t nbList;
    ARFCNValueEUTRA list[MAXFREQ_V2X];
  };

  /**
   * SL-Priority
   * The IE SL-Priority indicates the one or more priorities of resource pool used for sidelink communication, or of a logical
   * channel group used in case of scheduled sidelink communication resources, see TS 36.321 [6].
   */
  struct SlPriority {
    uint8_t prio; // INTEGER (1..8)
  }; 

  struct SlPriorityList {
    uint8_t nbList; 
    SlPriority list[MAXSL_PRIO];
  };

  struct TxConfigIndex {
    uint8_t configIndex; // INTEGER (0..maxSL-V2X-TxConfig-1-r14)
  };
 
  struct TxConfigIndexList {
    uint8_t nbList;
    TxConfigIndex list[MAXCBR_LEVEL];
  };

  /**
   * SL-CBR-PPPP-TxConfigList
   * The IE SL-CBR-PPPP-TxConfigList indicates the mapping between PSSCH transmission parameter (such as MCS,
   * PRB number, retransmission number, CR limit) sets by using the indexes of the configurations provided in sl-CBRPSSCH-
   * TxConfigList, CBR ranges by an index to the entry of the CBR range configuration in cbr-
   * RangeCommonConfigList, and PPPP ranges. It also indicates the default PSSCH transmission parameters to be used
   * when CBR measurement results are not available.
   */
  struct SlPpppTxConfigIndex {
    SlPriority priorityThreshold; 
    uint8_t defaultTxConfigIndex; // INTEGER (0..maxCBR-Level-1-r14)
    uint8_t cbrConfigIndex; // INTEGER (0..maxSL-V2X-CBRConfig-1-r14),
    TxConfigIndexList txConfigIndexList; 
  };

  struct SlCbrPpppTxConfigList {
    SlPpppTxConfigIndex list[8]; 
  };

  struct SubframeBitmapSlV2x {
    std::bitset<20> bitmap; // 16/20/100 bits for FDD
  };

  /**
   * SL-P2X-ResourceSelectionConfig
   * The IE SL-P2X-ResourceSelectionConfig includes the configuration of resource selection for P2X related V2X sidelink
   * communication. E-UTRAN configures at least one resource selection mechanism.
   */
  struct SlP2xResourceSelectionConfig{
    enum{
      //true
    }partialSensing;
    enum{
      //true
    }randomSelection;
  };

  /**
   * SL-TypeTxSync
   * The IE SL-TypeTxSync indicates the synchronization reference type.
   */
  struct SlTypeTxSync {
    enum {
      gnss,
      enb,
      ue
    }type;
  };

  /**
   * SL-TxPower
   * The IE SL-TxPower is used to limit the UE's sidelink transmission power on a carrier frequency. The unit is dBm. Value
   * minusinfinity corresponds to –infinity.
   */
  struct SlTxPower {
      // Choice
      // minusinfinity; 
      int8_t txPower; // INTEGER (-41..31)
  };

  struct SlPsschTxParameters{
    uint8_t minMcsPssch; // INTEGER (0..31)
    uint8_t maxMcsPssch; // INTEGER (0..31)
    uint8_t minSubChannelNumberPssch; // INTEGER (1..20)
    uint8_t maxSubChannelNumberPssch; // INTEGER (1..20)
    AllowedRetxNumberPssch allowedRetxNumberPssch; 
    SlTxPower maxTxPower; 
  };

  struct SlPsschTxConfig {
    SlTypeTxSync typeTxSync; 
    ThresUeSpeed thresUeSpeed;
    SlPsschTxParameters parametersAboveThres;
    SlPsschTxParameters parametersBelowThres;
  };

  /**
   * SL-PSSCH-TxConfigList
   * The IE SL-PSSCH-TxConfigList indicates PSSCH transmission parameters. When lower layers select parameters from
   * the range indicated in IE SL-PSSCH-TxConfigList, the UE considers both configurations in IE SL-PSSCH-TxConfigList
   * and the CBR-dependent configurations represented in IE SL-CBR-PPPP-TxConfigList. Only one IE SL-PSSCHTxConfig
   * is provided per typeTxSync.
   */
  struct SlPsschTxConfigList {
    uint8_t nbList; 
    SlPsschTxConfig list[MAXPSSCH_TXCONFIG]; 
  };

  struct SlRestrictResourceReservationPeriod{
    enum{
      v0dot2,
      v0dot5, 
      v1,
      v2,
      v3,
      v4,
      v5,
      v6,
      v7,
      v8,
      v9,
      v10,
      spare4,
      spare3,
      spare2,
      spare1
    } period; 
  };

  static double RestrictResourceReservationPeriodAsInt (SlRestrictResourceReservationPeriod pPeriod) {
    double period = 0;
    switch (pPeriod.period) {
    case SlRestrictResourceReservationPeriod::v0dot2:
      period = 0.2;
      break;
    case SlRestrictResourceReservationPeriod::v0dot5:
      period = 0.5;
      break;
    case SlRestrictResourceReservationPeriod::v1:
      period = 1;
      break;
    case SlRestrictResourceReservationPeriod::v2:
      period = 2;
      break;
    case SlRestrictResourceReservationPeriod::v3:
      period = 3;
      break;
    case SlRestrictResourceReservationPeriod::v4:
      period = 4;
      break;
    case SlRestrictResourceReservationPeriod::v5:
      period = 5;
      break;
    case SlRestrictResourceReservationPeriod::v6:
      period = 6;
      break;
    case SlRestrictResourceReservationPeriod::v7:
      period = 7;
      break;
    case SlRestrictResourceReservationPeriod::v8:
      period = 8;
      break;
    case SlRestrictResourceReservationPeriod::v9:
      period = 9;
      break;
    case SlRestrictResourceReservationPeriod::v10:
      period = 10;
      break;
    // not supported yet
    case SlRestrictResourceReservationPeriod::spare4:
      period = 0;
      break;
    case SlRestrictResourceReservationPeriod::spare3:
      period = 0;
      break;
    case SlRestrictResourceReservationPeriod::spare2:
      period = 0;
      break;
    case SlRestrictResourceReservationPeriod::spare1:
      period = 0;
      break;
    }
    return period;
  };

  struct SlRestrictResourceReservationPeriodList {
    uint8_t nbPeriod; 
    SlRestrictResourceReservationPeriod list[MAXRESERVERATIONPERIOD];
  };


  struct SlCbr {
    uint8_t cbr; // INTEGER (0..100)
  };

  struct SlCbrPsschTxConfig {
    uint16_t crLimit; // INTEGER (0..10000)
    SlPsschTxParameters txParameters; 
  };

  struct SlCbrLevelsConfig {
    uint8_t nbConfigs;
    SlCbr configs[MAXCBR_LEVEL];
  };

  struct CbrRangeCommonConfigList {
    uint8_t nbList;
    SlCbrLevelsConfig list[MAXSL_V2X_CBRCONFIG];
  };

  struct SlCbrPsschTxConfigList {
    uint8_t nbList;
    SlCbrPsschTxConfig list[MAXSL_V2X_TXCONFIG];
  };

  /**
   * SL-CBR-CommonTxConfigList
   * The IE SL-CBR-CommonTxConfigList indicates the list of PSSCH transmission parameters (such as MCS, sub-channel
   * number, retransmission number, CR limit) in sl-CBR-PSSCH-TxConfigList, and the list of CBR ranges in 
   * cbr-RangeCommonConfigList, to configure congestion control to the UE for V2X sidelink communication.
   */
  struct SlCbrCommonTxConfigList {
    CbrRangeCommonConfigList cbrRangeCommonConfigList; 
    SlCbrPsschTxConfigList slCbrPsschTxConfigList; 
  };

  /**
   * SL-SyncAllowed
   * The IE SL-SyncAllowed indicates the allowed the synchronization references for a transmission resource pool for V2X
   * sidelink communication.
   */
  struct SlSyncAllowed{ 
    enum {
       // true
    }gnssSync;
    enum {
       // true
    }enbSync;
    enum {
       // true
    }ueSync;
  };

  /**
   * SL-CommResourcePool
   * The IE SL-CommResourcePool and SL-CommResourcePoolV2X specifies the configuration information for an
   * individual pool of resources for sidelink communication and V2X sidelink communication respectively. The IE covers
   * the configuration of both the sidelink control information and the data.
   */
  struct SlCommResourcePoolV2x{
    bool haveSlOffsetIndicator;
    SlOffsetIndicator slOffsetIndicator; 
    SubframeBitmapSl slSubframe; 
    AdjacencyPscchPssch adjacencyPscchPssch; 
    SizeSubchannel sizeSubchannel;
    cv2x_NumSubchannel numSubchannel;
    cv2x_StartRbSubchannel startRbSubchannel; 
    bool haveStartRbPscchPool; 
    StartRbPscchPool startRbPscchPool; 
    bool haveRxParametersNCell; 
    RxParametersNCell rxParametersNCell;
    bool haveSlTxParameters;
    SlTxParameters dataTxParameters; 
    bool haveZoneId; 
    ZoneId zoneId; 
    bool haveThreshSrssiCbr; 
    ThreshSrssiCbr threshSrssiCbr;
    bool havePoolReportId;
    SlV2xTxPoolReportIdentity poolReportId;
    bool haveCbrPsschTxConfigList; 
    SlCbrPsschTxConfigList cbrPsschTxConfigList; 
    bool haveResourceSelectionConfigP2x;
    SlP2xResourceSelectionConfig resourceSelectionConfigP2x; 
    bool haveSyncAllowed;
    SlSyncAllowed syncAllowed; 
    bool haveRestrictResourceReservationPeriod;
    SlRestrictResourceReservationPeriodList restrictResourceReservationPeriod;

    friend bool operator==(const SlCommResourcePoolV2x& lhs, const SlCommResourcePoolV2x& rhs)
    {
      bool equal = lhs.slSubframe.bitmap == rhs.slSubframe.bitmap
        && lhs.adjacencyPscchPssch.adjacency == rhs.adjacencyPscchPssch.adjacency
        && lhs.sizeSubchannel.size == rhs.sizeSubchannel.size 
        && lhs.numSubchannel.num == rhs.numSubchannel.num
        && lhs.startRbSubchannel.startRb == rhs.startRbSubchannel.startRb;
      return equal; 
      /*if (equal && lhs.haveStartRbPscchPool) {
        equal = lhs.startRbPscchPool.startRb == rhs.startRbPscchPool.startRb;
      }
      if (equal && lhs.haveRxParametersNCell) {
        equal = lhs.rxParametersNCell == rhs.rxParametersNCell;
      }
      if (equal && lhs.haveSlTxParameters) {
        equal = lhs.SlTxParameters== rhs.SlTxParameters;
      } 
      if (equal && lhs.haveZoneId) {
        equal = lhs.zoneId.id == rhs.zoneId.id; 
      }
      if (equal && lhs.haveThreshSrssiCbr) {
        equal = lhs.threshSrssiCbr.thresh == rhs.threshSrssiCbr.thresh;
      }
      if (equal && lhs.havePoolReportId) {
        equal = lhs.poolReportId.ident == rhs.poolReportId.ident;
      }
      if (equal && lhs.haveCbrPsschTxConfigList) {
        equal = lhs.cbrPsschTxConfigList == rhs.cbrPsschTxConfigList; 
      }
      if (equal && lhs.haveResourceSelectionConfigP2x) { 
        equal = lhs.resourceSelectionConfigP2x == rhs.resourceSelectionConfigP2x;
      }
      if (equal && lhs.haveSyncAllowed) {
        equal = lhs.syncAllowed == rhs.syncAllowed; 
      }
      if (equal && lhs.haveRestrictResourceReservationPeriod) {
        equal = lhs.restrictResourceReservationPeriod == rhs.restrictResourceReservationPeriod;
      }*/
    }

  };
  
  struct SlCommTxPoolListV2x {
    uint8_t nbPools;
    SlCommResourcePoolV2x pools[MAXSL_V2X_TXPOOL];
  };

  struct SlCommRxPoolListV2x {
    uint8_t nbPools;
    SlCommResourcePoolV2x pools[MAXSL_V2X_RXPOOL];
  };

  struct SlThresPsschRsrp {
    uint8_t thresPsschRsrp; // INTEGER (0..66)
  };

  /**
   * SL-ThresPSSCH-RSRP-List
   * IE SL-ThresPSSCH-RSRP-List indicates a threshold used for sensing based UE autonomous resource selection (see TS
   * 36.213 [23]). A resource is excluded if it is indicated or reserved by a decoded SCI and PSSCH RSRP in the associated
   * data resource is above the threshold defined by IE SL-ThresPSSCH-RSRP-List.
   */
  struct SlThresPsschRsrpList{
    uint8_t nbList;
    SlThresPsschRsrp list[64];
  }; 

  struct P2XSensingConfig{
    uint8_t minNumCandidateSF; 
    std::bitset<10> gapCandidateSensing; 
  };

  /**
   * SL-CommTxPoolSensingConfig
   * The IE SL-CommTxPoolSensingConfig specifies V2X sidelink communication configurations used for UE autonomous
   * resource selection.
   */
  struct SlCommTxPoolSensingConfig{
    SlPsschTxConfigList psschTxConfigList;
    SlThresPsschRsrpList thresPsschRsrpList; 
    SlRestrictResourceReservationPeriodList restrictResourceReservationPeriod; 
    ProbResourceKeep probResourceKeep; 
    P2XSensingConfig p2xSensingConfig;
    SlReselectAfter slReselectAfter;
  };

  /**
   * SL-OffsetIndicator
   * The IE SL-OffsetIndicator indicates the offset of the pool of resources relative to SFN 0 of the cell from which it was
   * obtained or, when out of coverage, relative to DFN 0.
   */
  struct SlOffsetIndicatorSync {
    uint8_t offset; // INTEGER (0..159)
  };


  struct SlSyncConfigNFreq {
    // TODO: Implement SL-SyncConfigNFreq-r13
  };

  struct SlSyncConfigListNFreqV2x {
    uint8_t nbList; 
    SlSyncConfigNFreq list[MAXSL_V2X_SYNCCONFIG];
  };

  /**
   * SL-SyncConfig
   * The IE SL-SyncConfig specifies the configuration information concerning reception of synchronisation signals from
   * neighbouring cells as well as concerning the transmission of synchronisation signals for sidelink communication and
   * sidelink discovery.
   */
  struct SlSyncConfig {
    SlCpLen syncCPLen; 
    SlOffsetIndicatorSync syncOffsetIndicator; 
    uint8_t slssid; // INTEGER (0..167) 
    SlTxParameters txParameters;
    // RxParametersNCell rxParametersNCell; not implemented yet
  };

  struct SlSyncConfigListV2x {
    uint8_t nbList; 
    SlSyncConfig list[MAXSL_V2X_SYNCCONFIG];
  };

  /**
   * SL-ZoneConfig
   * The IE SL-ZoneConfig indicates zone configurations used for V2X sidelink communication.
   */
  struct SlZoneConfig{
    ZoneLength zoneLength; 
    ZoneWidth zoneWidth; 
    uint8_t zoneIdLongiMod; // {1..4} Indicates the total number of zones that is configured with respect to longitude.
    uint8_t zoneIdLatiMod; // {1..4} Indicates the total number of zones that is configured with respect to latitude.
  };


  struct PhysCellId{
    uint16_t cellid; // INTEGER (0..503)
  };

  struct PhysCellIdList {
    uint8_t nbList; 
    PhysCellId list[MAXSL_DISCCELLS];
  };

  struct SlV2xInterFreqUeConfig {
    PhysCellIdList physCellIdList; 
    SlTypeTxSync typeTxSync;
    SlSyncConfigListNFreqV2x v2xCommRxPool;
    SlCommTxPoolListV2x v2xCommTxPoolNormal;
    SlCommTxPoolListV2x p2xCommTxPoolNormal; 
    SlCommResourcePoolV2x v2xCommTxPoolExceptional;
    SlCommTxPoolSensingConfig v2xResourceSelectionConfig;
    SlZoneConfig zoneConfig;
    OffsetDFN offsetDFN; 
  };

  /**
   * SL-V2X-UE-ConfigList
   * The IE SL-V2X-UE-ConfigList indicates inter-frequency resource configuration per-carrier or per-cell.
   */
  struct SlV2xUeConfigList {
    uint8_t nbConfigs;
    SlV2xInterFreqUeConfig configs[MAXCELLINTRA];
  };

  struct Pmax {
    int8_t max; // INTEGER (-30..33)
  };

  struct PlmnIdendityList{
    int nbPlmn;
    PlmnIdentityInfo plmnIdentityInfo[MAX_PLMN]; 
  };

  struct SlInterFreqInfoV2x {
    PlmnIdendityList plmnIdentityList;
    ARFCNValueEUTRA v2xCommCarrierFreq; 
    Pmax  slMaxTxPower;
    SlBandwidth slBandwidth; 
    SlCommResourcePoolV2x v2xSchedulingPool;
    SlV2xUeConfigList v2xUeConfigList; 
    // CHOICE additionalSpectrumEmission not implemented yet
  };

  /**
   * SL-InterFreqInfoListV2X
   * The IE SL-InterFreqInfoListV2X indicates synchronization and resource allocation configurations of the neighboring
   * frequency for V2X sidelink communication.
   */
  struct SlInterFreqInfoListV2x {
    uint8_t nbInfos;
    SlInterFreqInfoV2x infos[MAXFREQ_V2X_1];
  };

  /**
   * SL-TxPoolIdentity
   * The IE SL-TxPoolIdentity identifies an individual pool entry configured for sidelink transmission, used for
   * communication and discovery.
   */
  struct SlV2xTxPoolIdentity {
    uint8_t txPoolIdentity; // INTEGER (1..maxSL-V2X-TxPool-r14)
  };

  /**
   * SL-V2X-ConfigDedicated
   * The IE SL-V2X-ConfigDedicated specifies the dedicated configuration information for V2X sidelink communication.
   */

  struct SlTxPoolToReleaseListV2x {
    uint8_t nbPools; 
    SlV2xTxPoolIdentity pools[MAXSL_V2X_TXPOOL];
  };

  struct SlTxPoolToAddMod {
    SlV2xTxPoolIdentity poolIdentity; 
    SlCommResourcePoolV2x pool; 
  };

  struct SlTxPoolToAddModListV2x {
    uint8_t nbPools;
    SlTxPoolToAddMod pools[MAXSL_V2X_TXPOOL];
  };

  struct LogicalChGroupInfoList {
    SlPriorityList infos[MAXLCG];
  };

  struct PeriodicBsrTimer
  {
    enum {
      sf5,
      sf10,
      sf16,
      sf20,
      sf32,
      sf40,
      sf64,
      sf80,
      sf128,
      sf160,
      sf320,
      sf640,
      sf1280,
      sf2560,
      infinity
    } period;
  };

  struct RetxBsrTimer {
    enum {
      sf320,
      sf640,
      sf1280,
      sf2560,
      sf5120,
      sf10240
    } period;
  };

  struct SlMacMainConfigSl //MAC-MainConfigSL-r12
  {
    PeriodicBsrTimer periodicBsrTimer;
    RetxBsrTimer rtxBsrTimer;
  };

  struct SlCommConfigScheduledV2x { //Indicates the configuration for the case E-UTRAN schedules the transmission resources based on sidelink specific BSR from the UE.
    uint16_t crnti;
    SlMacMainConfigSl macMainConfig;
    SlCommResourcePoolV2x v2xSchedulingPool; // OPTIONAL
    bool haveMcs; 
    uint8_t mcs; //INTEGER(0..31) OPTIONAL 
    LogicalChGroupInfoList logicalChGroupInfoList; 
  };

  struct SlCommConfigUeSelectedV2x { //commTxPoolNormalDedicated: Indicates a pool of transmission resources the UE is allowed to use while in RRC_CONNECTED.
    bool havePoolToRelease;
    SlTxPoolToReleaseListV2x poolToRelease;
    bool havePoolToAdd;
    SlTxPoolToAddModListV2x poolToAddModList; 
  };

  struct SlCommTxResourcesSetupV2x { //for dedicated configuration
    enum {
      SCHEDULED,
      UE_SELECTED
    } setup; //indicates which type of resources is being allocated
    SlCommConfigScheduledV2x scheduled;
    SlCommConfigUeSelectedV2x ueSelected;
  };

  struct SlV2xConfigDedicated {
    enum {
      RELEASE, 
      SETUP
    } commTxResources; //indicates if it is allocating or releasing resources
    SlCommTxResourcesSetupV2x setup; //valid only if commTxResources = setup
  };

struct cv2x_SlCbrPreconfigTxConfigList {
    SlCbrLevelsConfig cbrRangeCommonConfigList[MAXSL_V2X_CBRCONFIG2];
    SlCbrPsschTxConfig SlCbrPsschTxConfigList[MAXSL_V2X_TXCONFIG2]; 
  };

  struct SlV2xSyncOffsetIndicators{
    uint8_t syncOffsetIndicator1;
    uint8_t syncOffsetIndicator2;
    uint8_t syncOffsetIndicator3;
  };

  struct SlPreconfigV2xSync{
    SlV2xSyncOffsetIndicators syncOffsetIndicators; 
    SlTxParameters syncTxParameters; 
    uint8_t RSRPRangeSl3R12; // INTEGER (0..11)
    uint16_t filterCoefficient; 
    SyncRefMinHyst syncRefMinHyst;
    SyncRefDiffHyst syncRefDiffHyst;
  };

  struct SlV2xPreconfigCommPool { 
    SlOffsetIndicator slOffsetIndicator; // Optional
    SubframeBitmapSlV2x slSubframeV2x; 
    AdjacencyPscchPssch adjacencyPscchPssch; 
    SizeSubchannel sizeSubchannel;
    cv2x_NumSubchannel numSubchannel;
    cv2x_StartRbSubchannel startRbSubchannel; 
    SlTxParameters dataTxParameters; 
    StartRbPscchPool startRbPscchPool; 
    ZoneId zoneId; // Optional
    ThreshSrssiCbr threshSrssiCbr; // Optional  
    SlCbrPpppTxConfigList cbrPsschTxConfigList; // Optional
    SlP2xResourceSelectionConfig resourceSelectionConfigP2x; // Optional 
    SlSyncAllowed syncAllowed; // Optional
    SlRestrictResourceReservationPeriodList restrictResourceReservationPeriod; // Optional 
  };

  struct SlPreconfigV2xRxPoolList{
    uint8_t nbPools;
    SlV2xPreconfigCommPool pools[MAXSL_V2X_RXPOOLPRECONF];
  };

  struct SlPreconfigV2xTxPoolList{
    uint8_t nbPools;
    SlV2xPreconfigCommPool pools[MAXSL_V2X_TXPOOLPRECONF];  
  };

  struct SlV2xPreconfigFreqInfo{
    SlPreconfigGeneral v2xCommPreconfigGeneral; 
    SlPreconfigV2xSync v2xCommPreconfigSync; //  Optional
    SlPreconfigV2xRxPoolList v2xCommRxPoolList;
    SlPreconfigV2xTxPoolList v2xCommTxPoolList;
    SlPreconfigV2xTxPoolList p2xCommTxPoolList; 
    SlCommTxPoolSensingConfig v2xResourceSelectionConfig; // Optional
    SlZoneConfig zoneConfig; // Optional
    enum {
      gnss,
      enb
    } syncPriority;
    SlPriority thresSlTxPrioritization; // Optional
    uint16_t offsetDFN; // Optional 
  };

  struct SlV2xPreconfigFreqList {
    uint8_t nbFreq;
    SlV2xPreconfigFreqInfo freq[MAXFREQ_V2X];
  }; 

  struct SlV2xPreconfiguration {
    SlV2xPreconfigFreqList v2xPreconfigFreqList;
    SlAnchorCarrierFreqListV2x anchorCarrierFreqList; // Optional 
    cv2x_SlCbrPreconfigTxConfigList cbrPreconfigList; // Optional
  };

 /**
  * 36.331 6.3.1 V15.0.1
  * The IE SystemInformationBlockType21 contains V2X sidelink communication configuration
 */
  struct SlV2xConfigCommon {
    SlCommRxPoolListV2x v2xCommRxPool; 
    SlCommTxPoolListV2x v2xCommTxPoolNormalCommon; 
    SlCommTxPoolListV2x p2xCommTxPoolNormalCommon;
    SlCommResourcePoolV2x v2xCommTxPoolExceptional; 
    SlSyncConfigListV2x v2xSyncConfig; 
    SlInterFreqInfoListV2x v2xResourceSelectionConfig; 
    SlZoneConfig zoneConfig; 
    SlTypeTxSync typeTxSync; 
    SlPriority thresSlTxPriorization;
    SlAnchorCarrierFreqListV2x anchorCarrierFreqList;
    uint16_t offsetDFN; 
    SlCbrCommonTxConfigList cbrCommonTxConfigList; 
  };

  struct SystemInformationBlockType21 {
    SlV2xConfigCommon slV2xConfigCommon;
    std::string lateNonCriticalExtension;    
  };
  ///// END V2X /////


   // Discovery period duration
  struct SlPeriodDisc {
    enum
      {
        rf32,
        rf64,
        rf128,
        rf256,
        rf512,
        rf1024
      } period;
  };

  static uint32_t DiscPeriodAsInt (SlPeriodDisc period)
  {
    uint32_t p = 0;
    switch (period.period)
      {
      case SlPeriodDisc::rf32:
        p = 320;
        break;
      case SlPeriodDisc::rf64:
        p = 640;
        break;
      case SlPeriodDisc::rf128:
        p = 1280;
        break;
      case SlPeriodDisc::rf256:
        p = 2560;
        break;
      case SlPeriodDisc::rf512:
        p = 5120;
        break;
      case SlPeriodDisc::rf1024:
        p = 10240;
        break;
      }

      return p;
  };

  // Probability of transmission for Discovery
  struct TxProbability {
    enum {
      p25,
      p50,
      p75,
      p100,
      pOptimal
    } probability;
  };

  static uint32_t TxProbabilityAsInt (TxProbability prob)
  {
    uint32_t p = 0;
    switch (prob.probability)
    {
      case TxProbability::p25:
        p = 25;
        break;
      case TxProbability::p50:
        p = 50;
        break;
      case TxProbability::p75:
        p = 75;
        break;
      case TxProbability::p100:
        p = 100;
        break;
      case TxProbability::pOptimal:
        p = 32;
        break;
    }
    return p;
  }

  static TxProbability TxProbabilityFromInt (uint32_t p)
  {
    TxProbability prob;
    switch (p)
    {
      case 25:
        prob.probability = TxProbability::p25;
        break;
      case 50:
        prob.probability = TxProbability::p50;
        break;
      case 75:
        prob.probability = TxProbability::p75;
        break;
      case 100:
        prob.probability = TxProbability::p100;
        break;
      case 32:
        prob.probability = TxProbability::pOptimal;
        break;
    }
    return prob;
  }


  struct SlPreconfigDiscPool { 
    SlCpLen cpLen;
    SlPeriodDisc discPeriod;
    int8_t numRetx; // 0..3
    int32_t numRepetition; // 1..50
    SlTfResourceConfig tfResourceConfig;
    struct
      {
        SlTxParameters txParametersGeneral;
        TxProbability txProbability; 
      } txParameters;

  };
  
  struct SlPreconfigDiscPoolList {
    uint8_t nbPools;
    SlPreconfigDiscPool pools[MAXSL_TXPOOL];
  };

  struct SlPreconfiguration
  {
    SlPreconfigGeneral preconfigGeneral;
    SlPreconfigSync preconfigSync; ///< Synchronization configuration
    SlPreconfigCommPoolList preconfigComm;
    SlPreconfigDiscPoolList preconfigDisc;
  };

  struct SlCommTxPoolToAddMod
  {
    uint8_t poolIdentity;
    SlCommResourcePool pool;
  };
  
  struct SlCommTxPoolToAddModList
  {
    uint8_t nbPools;
    SlCommTxPoolToAddMod pools[MAXSL_TXPOOL];
  };

  struct SlTxPoolToReleaseList {
    uint8_t nbPools;
    uint8_t poolIdentities[MAXSL_TXPOOL];
  };
  
  struct SlCommConfigScheduled { //Indicates the configuration for the case E-UTRAN schedules the transmission resources based on sidelink specific BSR from the UE.
    uint16_t crnti;
    SlMacMainConfigSl macMainConfig;
    SlCommResourcePool commTxConfig;
    bool haveMcs; //indicates if MCS is being set
    uint8_t mcs; //0..28
  };
  
  struct SlCommConfigUeSelected { //commTxPoolNormalDedicated: Indicates a pool of transmission resources the UE is allowed to use while in RRC_CONNECTED.
    bool havePoolToRelease;
    SlTxPoolToReleaseList poolToRelease;
    bool havePoolToAdd;
    SlCommTxPoolToAddModList poolToAddModList;
  };

  struct SlCommTxResourcesSetup {//for dedicated configuration
    enum {
      SCHEDULED,
      UE_SELECTED
    } setup; //indicates which type of resources is being allocated
    SlCommConfigScheduled scheduled;
    SlCommConfigUeSelected ueSelected;
  };
  
  struct SlCommConfig {//for dedicated configuration
    enum {
      RELEASE,
      SETUP
    } commTxResources; //indicates if it is allocating or releasing resources
    SlCommTxResourcesSetup setup; //valid only if commTxResources = setup
  };

  struct SlDestinationInfoList {
    int nbDestinations;
    uint32_t SlDestinationIdentity[MAXSL_DEST]; //each destination is 24 bit long.
  };
  
  struct SlCommTxResourceReq {
    uint32_t carrierFreq;
    SlDestinationInfoList slDestinationInfoList;
  };
  
  
  /* end SystemInformationBlockType18 */


 ///////////////* Start SystemInformationBlockType19 *///////////////

  //Discovery period and probability of transmission are defined above 
  //in order to set the preconfigured pools
 
  struct PoolSelection {
    enum {
      RSRPBASED,
      RANDOM
    } selection;
  };
  struct PoolSelectionRsrpBased {
    uint32_t threshLow; // 0..7
    uint32_t threshHigh; // 0..7
  }; // Value 0 corresponds to -infinity, value 1 to -110dBm, value 2 to -100dBm, and so on (i.e. in steps of 10dBm) until value 6, which corresponds to -60dBm, while value 7 corresponds to +infinity.

  static uint32_t RsrpValueDbm (uint32_t rsrp)
  {
    uint32_t r = 0;
    switch (rsrp)
    {
      case 1:
        r = -110;
        break;
      case 2:
        r = -100;
        break;
      case 3:
        r = -90;
        break;
      case 4:
        r = -80;
        break;
      case 5:
        r = -70;
        break;
      case 6:
        r = -60;
        break;
    }
    return r;
  }

  struct SubframeAssignement {
    enum {
      sa0,
      sa1,
      sa2,
      sa3,
      sa4,
      sa5,
      sa6
    } sa;
  };
  struct SpecialSubframePatterns {
    enum {
      ssp0,
      ssp1,
      ssp2,
      ssp3,
      ssp4,
      ssp5,
      ssp6,
      ssp7,
      ssp8
    } ssp;
  };

  struct SlDiscResourcePool {
    SlCpLen cpLen; // defined for communication.
    SlPeriodDisc discPeriod;
    int8_t numRetx; // 0..3
    int32_t numRepetition; // 1..50
    SlTfResourceConfig tfResourceConfig; 
    bool haveTxParameters;
    struct {
      SlTxParameters txParametersGeneral;
      struct {
        PoolSelection poolSelection;
        bool havePoolSelectionRsrpBased;
        PoolSelectionRsrpBased poolSelectionRsrpBased; 
        TxProbability txProbability; 
      } ueSelectedResourceConfig;
    } txParameters;
    bool haveRxParameters;
    struct {
      struct {
        SubframeAssignement subframeAssignement;
        SpecialSubframePatterns specialSubframePatterns;
      } tddConfig;
      uint32_t syncConfigIndex; // 0 .. 15
    } rxParameters;
  
    friend bool operator==(const SlDiscResourcePool& lhs, const SlDiscResourcePool& rhs)
    {
      bool equal = lhs.cpLen.cplen == rhs.cpLen.cplen
        && lhs.discPeriod.period == rhs.discPeriod.period
        && lhs.numRetx == rhs.numRetx
        && lhs.numRepetition == rhs.numRepetition
        && lhs.tfResourceConfig == rhs.tfResourceConfig
        && lhs.haveTxParameters == rhs.haveTxParameters;
      if (equal && lhs.haveTxParameters) {
        equal = equal && lhs.txParameters.txParametersGeneral == rhs.txParameters.txParametersGeneral
          && lhs.txParameters.ueSelectedResourceConfig.poolSelection.selection == rhs.txParameters.ueSelectedResourceConfig.poolSelection.selection;
      }
      equal = equal && lhs.txParameters.ueSelectedResourceConfig.havePoolSelectionRsrpBased == rhs.txParameters.ueSelectedResourceConfig.havePoolSelectionRsrpBased;
      if (equal && lhs.txParameters.ueSelectedResourceConfig.havePoolSelectionRsrpBased) {
        equal = equal && lhs.txParameters.ueSelectedResourceConfig.poolSelectionRsrpBased.threshHigh == rhs.txParameters.ueSelectedResourceConfig.poolSelectionRsrpBased.threshHigh
          && lhs.txParameters.ueSelectedResourceConfig.poolSelectionRsrpBased.threshLow == rhs.txParameters.ueSelectedResourceConfig.poolSelectionRsrpBased.threshLow
          && lhs.txParameters.ueSelectedResourceConfig.txProbability.probability == rhs.txParameters.ueSelectedResourceConfig.txProbability.probability;
      }
      equal = equal && lhs.haveRxParameters == rhs.haveRxParameters;
      if (equal && lhs.haveRxParameters) {
        equal = equal && lhs.rxParameters.tddConfig.subframeAssignement.sa == rhs.rxParameters.tddConfig.subframeAssignement.sa
          && lhs.rxParameters.tddConfig.specialSubframePatterns.ssp == rhs.rxParameters.tddConfig.specialSubframePatterns.ssp
          && lhs.rxParameters.syncConfigIndex == rhs.rxParameters.syncConfigIndex;
      }
      return equal;
    }
  };

  struct SlDiscTxPoolList {
    uint8_t nbPools;
    SlDiscResourcePool pools[MAXSL_TXPOOL];
  };

  struct SlDiscRxPoolList {
    uint8_t nbPools;
    SlDiscResourcePool pools[MAXSL_RXPOOL];
  };

  struct SlDiscTxPowerInfo {
    uint32_t discMaxTxPower; //-30..30
  };

  // The first entry in SLDiscTxPowerInfoList corresponds to UE range class "short", the second entry corresponds to "medium" and the third entry corresponds to "long"
  struct SlDiscTxPowerInfoList {
    uint8_t nbPowerInfo;
    SlDiscTxPowerInfo power[MAXSL_DISC_POWERCLASS];
  };

  struct Sib19DiscConfig { // SlDiscConfig struct used in RRC Connection
    SlDiscRxPoolList discRxPool;
    SlDiscTxPoolList discTxPoolCommon; // Optional 
    SlDiscTxPowerInfoList discTxPowerInfo;  // Optional 
    SlSyncConfigList discSyncConfig;         // Optional 
  };
  
  struct SlCarrierFreqInfoList
  {
    uint16_t carrierFreq;
    PlmnIdendityList plmnIdentityList;
  };

  struct SystemInformationBlockType19
  {
    Sib19DiscConfig discConfig;
    SlCarrierFreqInfoList discInterFreqList;
  };
  
  struct SlDiscTxPoolToAddMod
  {
    uint8_t poolIdentity;
    SlDiscResourcePool pool;
  };
  
  struct SlDiscTxPoolToAddModList
  {
    uint8_t nbPools;
    SlDiscTxPoolToAddMod pools[MAXSL_TXPOOL];
  };

  struct SlTfIndexPair {
    uint32_t discSfIndex; //1..200
    uint32_t discPrbIndex; //1..50
  };

  struct SlTfIndexPairList {
    uint8_t nbPair;
    SlTfIndexPair pair[MAXSL_TF_INDEXPAIR];
  };

  struct SlHoppingConfigDisc {
    uint32_t a; //1..200
    uint32_t b; //1..10
    enum {
      n1,
      n5
    } c;
  };

  struct SlDiscConfigScheduled { 
    SlDiscResourcePool discTxConfig;
    SlTfIndexPairList discTfIndexList;
    SlHoppingConfigDisc discHoppingConfigDisc;
  };

  struct SlDiscConfigUeSelected { 
    bool havePoolToRelease;
    SlTxPoolToReleaseList poolToRelease;
    bool havePoolToAdd;
    SlDiscTxPoolToAddModList poolToAddModList;
  };

  struct SlDiscTxResourcesSetup {
    enum {
      SCHEDULED,
      UE_SELECTED
    } setup;
    SlDiscConfigScheduled scheduled;
    SlDiscConfigUeSelected ueSelected;
  };
  
  struct SlDiscConfig {
    enum {
      RELEASE,
      SETUP
    } discTxResources; 
    SlDiscTxResourcesSetup setup;
  };

  ///////////////* End SystemInformationBlockType19 *///////////////



  /// SystemInformation structure
  struct SystemInformation
  {
    bool haveSib2; ///< have SIB2?
    SystemInformationBlockType2 sib2; ///< SIB2
    bool haveSib18; //NIST: add SIB 18
    SystemInformationBlockType18 sib18; //NIST: add SIB 18
    bool haveSib19; // add SIB 19
    SystemInformationBlockType19 sib19;
    //bool haveSib21; // CNI: add SIB 21
    //SystemInformationBlockType21 sib21; // CNI: add SIB 21
  };

  /// AsConfig structure
  struct AsConfig
  {
    MeasConfig sourceMeasConfig; ///< source measure config
    RadioResourceConfigDedicated sourceRadioResourceConfig; ///< source radio resource config
    uint16_t sourceUeIdentity; ///< source UE identity
    MasterInformationBlock sourceMasterInformationBlock; ///< source master information block
    SystemInformationBlockType1 sourceSystemInformationBlockType1; ///< source system information block type 1
    SystemInformationBlockType2 sourceSystemInformationBlockType2; ///< source system information block type 2
    uint32_t sourceDlCarrierFreq; ///< source DL carrier frequency
  };

  /// CgiInfo structure
  struct CgiInfo
  {
    uint32_t plmnIdentity; ///< PLMN identity
    uint32_t cellIdentity; ///< cell identity
    uint16_t trackingAreaCode; ///< tracking area code
    std::list<uint32_t> plmnIdentityList; ///< PLMN identity list
  };

  /// MeasResultEutra structure
  struct MeasResultEutra
  {
    uint16_t physCellId; ///< Phy cell ID
    bool haveCgiInfo; ///< have CGI info?
    CgiInfo cgiInfo; ///< CGI info
    bool haveRsrpResult; ///< have RSRP result
    uint8_t rsrpResult; ///< RSRP result
    bool haveRsrqResult; ///< have RSRQ result?
    uint8_t rsrqResult; ///< RSRQ result
  };

  /// MeasResultScell structure
  struct MeasResultScell
  {
    uint16_t servFreqId; ///< service frequency ID
    bool haveRsrpResult; ///< have RSRP result?
    uint8_t rsrpResult; ///< the RSRP result
    bool haveRsrqResult; ///< have RSRQ result?
    uint8_t rsrqResult; ///< the RSRQ result
  };

  /// MeasResultBestNeighCell structure
  struct MeasResultBestNeighCell
  {
    uint16_t servFreqId; ///< service frequency ID
    uint16_t physCellId; ///< physical cell ID
    bool haveRsrpResult; ///< have RSRP result?
    uint8_t rsrpResult; ///< the RSRP result
    bool haveRsrqResult; ///< have RSRQ result?
    uint8_t rsrqResult; ///< the RSRQ result
  };

  /// MeasResultServFreqList
  struct MeasResultServFreqList
  {
    bool haveMeasurementResultsServingSCells; ///< have measure results serving Scells
    std::list<MeasResultScell> measResultScell; ///< measure results Scells
    bool haveMeasurementResultsNeighCell; ///< always false since not implemented
    std::list<MeasResultBestNeighCell> measResultBestNeighCell; ///< measure result best neighbor cell
  };

  /// MeasResults structure
  struct MeasResults
  {
    uint8_t measId; ///< measure ID
    uint8_t rsrpResult; ///< RSRP result
    uint8_t rsrqResult; ///< RSRQ result
    bool haveMeasResultNeighCells; ///< have measure result neighbor cells
    std::list<MeasResultEutra> measResultListEutra; ///< measure result list eutra
    bool haveScellsMeas; ///< has SCells measure
    MeasResultServFreqList measScellResultList; ///< measure SCell result list
  };

  // Messages

  /// RrcConnectionRequest structure
  struct RrcConnectionRequest
  {
    uint64_t ueIdentity; ///< UE identity
  };

  /// RrcConnectionSetup structure
  struct RrcConnectionSetup
  {
    uint8_t rrcTransactionIdentifier; ///< RRC transaction identifier
    RadioResourceConfigDedicated radioResourceConfigDedicated; ///< radio resource config dedicated
  };

  /// RrcConnectionSetupCompleted structure
  struct RrcConnectionSetupCompleted
  {
    uint8_t rrcTransactionIdentifier; ///< RRC transaction identifier
  };


  /// CellIdentification structure
  struct CellIdentification
  {
    uint32_t physCellId; ///< physical cell ID
    uint32_t dlCarrierFreq; ///<  ARFCN - valueEUTRA
  };

  /// AntennaInfoCommon structure
  struct AntennaInfoCommon
  {
    uint16_t antennaPortsCount; ///< antenna ports count
  };

  /// UlPowerControlCommonSCell structure
  struct UlPowerControlCommonSCell
  {
    uint16_t alpha; ///< alpha value
  };
  
  /// PrachConfigSCell structure
  struct PrachConfigSCell
  {
    uint16_t index; ///< the index
  };

  /// NonUlConfiguration structure
  struct NonUlConfiguration
  {
    // 3GPP TS 36.311 v.11.10 R11 pag.220
    /// 1: Cell characteristics
    uint16_t dlBandwidth;
    /// 2: Physical configuration, general antennaInfoCommon-r10
    AntennaInfoCommon antennaInfoCommon;
    // 3: Physical configuration, control phich-Config-r10
    // Not Implemented
    /// 4: Physical configuration, physical channels pdsch-ConfigCommon-r10
    PdschConfigCommon pdschConfigCommon;
    // 5: tdd-Config-r10
    //Not Implemented
  };

  /// UlConfiguration structure
  struct UlConfiguration 
  { 
    FreqInfo ulFreqInfo; ///< UL frequency info
    UlPowerControlCommonSCell ulPowerControlCommonSCell; ///< 3GPP TS 36.331 v.11.10 R11 pag.223 
    SoundingRsUlConfigCommon soundingRsUlConfigCommon; ///< sounding RS UL config common
    PrachConfigSCell prachConfigSCell; ///< PRACH config SCell
    //PushConfigCommon pushConfigCommon; //NOT IMPLEMENTED!
  };

  /// AntennaInfoUl structure
  struct AntennaInfoUl
  {
    uint8_t transmissionMode; ///< transmission mode
  };

  /// PuschConfigDedicatedSCell structure
  struct PuschConfigDedicatedSCell
  {
    /// 3GPP TS 36.331 v.11.10 R11 page 216
    uint16_t nPuschIdentity;
  };

  /// UlPowerControlDedicatedSCell structure
  struct UlPowerControlDedicatedSCell
  {
    /// 3GPP TS 36.331 v.11.10 R11 page 234
    uint16_t pSrsOffset;
  };

  /// PhysicalConfigDedicatedSCell structure
  struct PhysicalConfigDedicatedSCell
  {
    // Non-Ul Configuration
    bool haveNonUlConfiguration; ///< have non UL configuration?
    bool haveAntennaInfoDedicated; ///< have antenna info dedicated?
    AntennaInfoDedicated antennaInfo; ///< antenna info dedicated
    bool crossCarrierSchedulingConfig; ///< currently implemented as boolean variable --> implementing crossCarrierScheduling is out of the scope of this GSoC proposal
    bool havePdschConfigDedicated; ///< have PDSCH config dedicated?
    PdschConfigDedicated pdschConfigDedicated; ///< PDSCH config dedicated

    // Ul Configuration
    bool haveUlConfiguration; ///< have UL configuration?
    bool haveAntennaInfoUlDedicated; ///< have antenna info UL dedicated?
    AntennaInfoDedicated antennaInfoUl; ///< antenna info UL
    PuschConfigDedicatedSCell pushConfigDedicatedSCell; ///< PUSCH config dedicated SCell
    UlPowerControlDedicatedSCell  ulPowerControlDedicatedSCell; ///< UL power control dedicated SCell
    bool haveSoundingRsUlConfigDedicated; ///< have sounding RS UL config dedicated?
    SoundingRsUlConfigDedicated soundingRsUlConfigDedicated; ///< sounding RS UL config dedicated
  };

  /// RadioResourceConfigCommonSCell
  struct RadioResourceConfigCommonSCell
  {
    bool haveNonUlConfiguration; ///< have non UL configuration?
    NonUlConfiguration nonUlConfiguration; ///< non UL configuration
    bool haveUlConfiguration; ///< have UL configuration
    UlConfiguration ulConfiguration; ///< UL configuration
  };

  /// RadioResourceConfigDedicatedSCell structure
  struct RadioResourceConfigDedicatedSCell
  {
    PhysicalConfigDedicatedSCell physicalConfigDedicatedSCell; ///< physical config dedicated SCell
  };

  /// SCellToAddMod structure
  struct SCellToAddMod
  {
    uint32_t sCellIndex; ///< SCell index
    CellIdentification cellIdentification; ///< cell identification
    RadioResourceConfigCommonSCell radioResourceConfigCommonSCell; ///< radio resource config common SCell
    bool haveRadioResourceConfigDedicatedSCell; ///< have radio resource config dedicated SCell?
    RadioResourceConfigDedicatedSCell radioResourceConfigDedicateSCell; ///< radio resource config dedicated SCell
  };

  /// NonCriticalExtensionConfiguration structure
  struct NonCriticalExtensionConfiguration
  {
    std::list<SCellToAddMod> sCellsToAddModList; ///< SCell to add mod list
    std::list<uint32_t> sCellToReleaseList; ///< SCell to release list
  };

  /// RrcConnectionReconfiguration structure
  struct RrcConnectionReconfiguration
  {
    uint8_t rrcTransactionIdentifier; ///< RRC transaction identifier
    bool haveMeasConfig; ///< have measure config
    MeasConfig measConfig; ///< measure config
    bool haveMobilityControlInfo; ///< have mobility control info
    MobilityControlInfo mobilityControlInfo; ///< mobility control info
    bool haveRadioResourceConfigDedicated; ///< have radio resource config dedicated
    RadioResourceConfigDedicated radioResourceConfigDedicated; ///< radio resource config dedicated
    bool haveNonCriticalExtension; ///< have critical extension?
    /// 3GPP TS 36.331 v.11.10 R11 Sec. 6.2.2 pag. 147 (also known as ETSI TS 136 331 v.11.10 Feb-2015)
    NonCriticalExtensionConfiguration nonCriticalExtension;
    bool haveSlCommConfig; //presence of dedicated sidelink communication configuration
    SlCommConfig slCommConfig;    
    bool haveSlDiscConfig; // sidelink discovery configuration
    SlDiscConfig slDiscConfig;
 };

  /// RrcConnectionReconfigurationCompleted structure
  struct RrcConnectionReconfigurationCompleted
  {
    uint8_t rrcTransactionIdentifier; ///< RRC transaction identifier
  };


  /// RrcConnectionReestablishmentRequest structure
  struct RrcConnectionReestablishmentRequest
  {
    ReestabUeIdentity ueIdentity; ///< UE identity
    ReestablishmentCause reestablishmentCause; ///< reestablishment cause
  };

  /// RrcConnectionReestablishment structure
  struct RrcConnectionReestablishment
  {
    uint8_t rrcTransactionIdentifier; ///< RRC transaction identifier
    RadioResourceConfigDedicated radioResourceConfigDedicated; ///< radio resource config dedicated
  };

  /// RrcConnectionReestablishmentComplete structure
  struct RrcConnectionReestablishmentComplete
  {
    uint8_t rrcTransactionIdentifier; ///< RRC transaction identifier
  };

  /// RrcConnectionReestablishmentReject structure
  struct RrcConnectionReestablishmentReject
  {
  };

  /// RrcConnectionRelease structure
  struct RrcConnectionRelease
  {
    uint8_t rrcTransactionIdentifier; ///< RRC transaction identifier
  };

  /// RrcConnectionReject structure
  struct RrcConnectionReject
  {
    uint8_t waitTime; ///< wait time
  };

  /// HandoverPreparationInfo structure
  struct HandoverPreparationInfo
  {
    AsConfig asConfig; ///< AS config
  };

  /// MeasurementReport structure
  struct MeasurementReport
  {
    MeasResults measResults; ///< measure results
  };

  struct SidelinkUeInformation {
    bool haveCommRxInterestedFreq;
    uint32_t commRxInterestedFreq; //max value=262143
    bool haveCommTxResourceReq;
    SlCommTxResourceReq slCommTxResourceReq;
    bool haveDiscRxInterest;
    bool discRxInterest;
    bool haveDiscTxResourceReq;
    uint8_t discTxResourceReq; //1..63, use 0 as invalid
  };

  struct SlV2xCommFreqList {
    uint16_t nbList; 
    uint8_t list[MAXFREQ_V2X];
  };

  struct SidelinkUeInformationV1430 {
    bool haveCommRxInterestedFreqList;
    SlV2xCommFreqList v2xCommRxInterestedFreqList;
    bool haveP2xCommTxType;
    enum {
      truee
    } p2xCommTxType;
    bool haveCommTxResourceReq; 
    SlV2xCommFreqList v2xCommTxResourceReq;
    bool havenonCriticalExtension;
    struct {

    } nonCriticalExtension; 
  };

};



/**
 * \brief Part of the RRC protocol. This Service Access Point (SAP) is used by
 *        the UE RRC to send messages to the eNB. Each method defined in this
 *        class corresponds to the transmission of a message that is defined in
 *        Section 6.2.2 of TS 36.331.
 */
class cv2x_LteUeRrcSapUser : public cv2x_LteRrcSap
{
public:
  /// SetupParameters structure
  struct SetupParameters
  {
    cv2x_LteRlcSapProvider* srb0SapProvider; ///< SRB0 SAP provider
    cv2x_LtePdcpSapProvider* srb1SapProvider; ///< SRB1 SAP provider
  };

  /**
   * \brief Setup function
   * \param params the setup parameters
   */
  virtual void Setup (SetupParameters params) = 0;

  /**
   * \brief Send an _RRCConnectionRequest message to the serving eNodeB
   *        during an RRC connection establishment procedure
   *        (Section 5.3.3 of TS 36.331).
   * \param msg the message
   */
  virtual void SendRrcConnectionRequest (RrcConnectionRequest msg) = 0;

  /**
   * \brief Send an _RRCConnectionSetupComplete_ message to the serving eNodeB
   *        during an RRC connection establishment procedure
   *        (Section 5.3.3 of TS 36.331).
   * \param msg the message
   */
  virtual void SendRrcConnectionSetupCompleted (RrcConnectionSetupCompleted msg) = 0;

  /**
   * \brief Send an _RRCConnectionReconfigurationComplete_ message to the serving eNodeB
   *        during an RRC connection reconfiguration procedure
   *        (Section 5.3.5 of TS 36.331).
   * \param msg the message
   */
  virtual void SendRrcConnectionReconfigurationCompleted (RrcConnectionReconfigurationCompleted msg) = 0;

  /**
   * \brief Send an _RRCConnectionReestablishmentRequest_ message to the serving eNodeB
   *        during an RRC connection re-establishment procedure
   *        (Section 5.3.7 of TS 36.331).
   * \param msg the message
   */
  virtual void SendRrcConnectionReestablishmentRequest (RrcConnectionReestablishmentRequest msg) = 0;

  /**
   * \brief Send an _RRCConnectionReestablishmentComplete_ message to the serving eNodeB
   *        during an RRC connection re-establishment procedure
   *        (Section 5.3.7 of TS 36.331).
   * \param msg the message
   */
  virtual void SendRrcConnectionReestablishmentComplete (RrcConnectionReestablishmentComplete msg) = 0;

  /**
   * \brief Send a _MeasurementReport_ message to the serving eNodeB
   *        during a measurement reporting procedure
   *        (Section 5.5.5 of TS 36.331).
   * \param msg the message
   */
  virtual void SendMeasurementReport (MeasurementReport msg) = 0;

  /**
   * \brief Send a _SidelinkUeInformation_ message to the serving eNodeB
   *        to indicate interest in sidelink transmission/reception
   * \param msg the message
   */
  virtual void SendSidelinkUeInformation (SidelinkUeInformation msg) = 0;

};


/**
 * \brief Part of the RRC protocol. This Service Access Point (SAP) is used to
 *        let the UE RRC receive a message from the eNB RRC. Each method defined
 *        in this class corresponds to the reception of a message that is
 *        defined in Section 6.2.2 of TS 36.331.
 */
class cv2x_LteUeRrcSapProvider : public cv2x_LteRrcSap
{
public:
  /// CompleteSetupParameters structure
  struct CompleteSetupParameters
  {
    cv2x_LteRlcSapUser* srb0SapUser; ///< SRB0 SAP user
    cv2x_LtePdcpSapUser* srb1SapUser; ///< SRB1 SAP user
  };

  /**
   * \brief Complete setup function
   * \param params the complete setup parameters
   */
  virtual void CompleteSetup (CompleteSetupParameters params) = 0;

  /**
   * \brief Receive a _SystemInformation_ message from the serving eNodeB
   *        during a system information acquisition procedure
   *        (Section 5.2.2 of TS 36.331).
   * \param msg the message
   */
  virtual void RecvSystemInformation (SystemInformation msg) = 0;

  /**
   * \brief Receive an _RRCConnectionSetup_ message from the serving eNodeB
   *        during an RRC connection establishment procedure
   *        (Section 5.3.3 of TS 36.331).
   * \param msg the message
   */
  virtual void RecvRrcConnectionSetup (RrcConnectionSetup msg) = 0;

  /**
   * \brief Receive an _RRCConnectionReconfiguration_ message from the serving eNodeB
   *        during an RRC connection reconfiguration procedure
   *        (Section 5.3.5 of TS 36.331).
   * \param msg the message
   */
  virtual void RecvRrcConnectionReconfiguration (RrcConnectionReconfiguration msg) = 0;

  /**
   * \brief Receive an _RRCConnectionReestablishment_ message from the serving eNodeB
   *        during an RRC connection re-establishment procedure
   *        (Section 5.3.7 of TS 36.331).
   * \param msg the message
   */
  virtual void RecvRrcConnectionReestablishment (RrcConnectionReestablishment msg) = 0;

  /**
   * \brief Receive an _RRCConnectionReestablishmentReject_ message from the serving eNodeB
   *        during an RRC connection re-establishment procedure
   *        (Section 5.3.7 of TS 36.331).
   * \param msg the message
   */
  virtual void RecvRrcConnectionReestablishmentReject (RrcConnectionReestablishmentReject msg) = 0;

  /**
   * \brief Receive an _RRCConnectionRelease_ message from the serving eNodeB
   *        during an RRC connection release procedure
   *        (Section 5.3.8 of TS 36.331).
   * \param msg the message
   */
  virtual void RecvRrcConnectionRelease (RrcConnectionRelease msg) = 0;

  /**
   * \brief Receive an _RRCConnectionReject_ message from the serving eNodeB
   *        during an RRC connection establishment procedure
   *        (Section 5.3.3 of TS 36.331).
   * \param msg the message
   */
  virtual void RecvRrcConnectionReject (RrcConnectionReject msg) = 0;

};


/**
 * \brief Part of the RRC protocol. This Service Access Point (SAP) is used by
 *        the eNB RRC to send messages to the UE RRC.  Each method defined in
 *        this class corresponds to the transmission of a message that is
 *        defined in Section 6.2.2 of TS 36.331.
 */
class cv2x_LteEnbRrcSapUser : public cv2x_LteRrcSap
{
public:
  /// SetupUeParameters structure
  struct SetupUeParameters
  {
    cv2x_LteRlcSapProvider* srb0SapProvider; ///< SRB0 SAP provider
    cv2x_LtePdcpSapProvider* srb1SapProvider; ///< SRB1 SAP provider
  };

  /**
   * \brief Setup UE function
   * \param rnti the RNTI
   * \param params the setup UE parameters
   */
  virtual void SetupUe (uint16_t rnti, SetupUeParameters params) = 0;
  /**
   * \brief Remove UE function
   * \param rnti the RNTI
   */
  virtual void RemoveUe (uint16_t rnti) = 0;

  /**
   * \brief Send a _SystemInformation_ message to all attached UEs
   *        during a system information acquisition procedure
   *        (Section 5.2.2 of TS 36.331).
   * \param cellId cell ID
   * \param msg the message
   */
  virtual void SendSystemInformation (uint16_t cellId, SystemInformation msg) = 0;

  /**
   * \brief Send an _RRCConnectionSetup_ message to a UE
   *        during an RRC connection establishment procedure
   *        (Section 5.3.3 of TS 36.331).
   * \param rnti the RNTI of the destination UE
   * \param msg the message
   */
  virtual void SendRrcConnectionSetup (uint16_t rnti, RrcConnectionSetup msg) = 0;

  /**
   * \brief Send an _RRCConnectionReconfiguration_ message to a UE
   *        during an RRC connection reconfiguration procedure
   *        (Section 5.3.5 of TS 36.331).
   * \param rnti the RNTI of the destination UE
   * \param msg the message
   */
  virtual void SendRrcConnectionReconfiguration (uint16_t rnti, RrcConnectionReconfiguration msg) = 0;

  /**
   * \brief Send an _RRCConnectionReestablishment_ message to a UE
   *        during an RRC connection re-establishment procedure
   *        (Section 5.3.7 of TS 36.331).
   * \param rnti the RNTI of the destination UE
   * \param msg the message
   */
  virtual void SendRrcConnectionReestablishment (uint16_t rnti, RrcConnectionReestablishment msg) = 0;

  /**
   * \brief Send an _RRCConnectionReestablishmentReject_ message to a UE
   *        during an RRC connection re-establishment procedure
   *        (Section 5.3.7 of TS 36.331).
   * \param rnti the RNTI of the destination UE
   * \param msg the message
   */
  virtual void SendRrcConnectionReestablishmentReject (uint16_t rnti, RrcConnectionReestablishmentReject msg) = 0;

  /**
   * \brief Send an _RRCConnectionRelease_ message to a UE
   *        during an RRC connection release procedure
   *        (Section 5.3.8 of TS 36.331).
   * \param rnti the RNTI of the destination UE
   * \param msg the message
   */
  virtual void SendRrcConnectionRelease (uint16_t rnti, RrcConnectionRelease msg) = 0;

  /**
   * \brief Send an _RRCConnectionReject_ message to a UE
   *        during an RRC connection establishment procedure
   *        (Section 5.3.3 of TS 36.331).
   * \param rnti the RNTI of the destination UE
   * \param msg the message
   */
  virtual void SendRrcConnectionReject (uint16_t rnti, RrcConnectionReject msg) = 0;

  /**
   * \brief Encode handover prepration information
   * \param msg HandoverPreparationInfo
   * \returns the packet
   */
  virtual Ptr<Packet> EncodeHandoverPreparationInformation (HandoverPreparationInfo msg) = 0;
  /**
   * \brief Decode handover prepration information
   * \param p the packet
   * \returns HandoverPreparationInfo
   */
  virtual HandoverPreparationInfo DecodeHandoverPreparationInformation (Ptr<Packet> p) = 0;
  /**
   * \brief Encode handover command
   * \param msg RrcConnectionReconfiguration
   * \returns the packet
   */
  virtual Ptr<Packet> EncodeHandoverCommand (RrcConnectionReconfiguration msg) = 0;
  /**
   * \brief Decode handover command
   * \param p the packet
   * \returns RrcConnectionReconfiguration
   */
  virtual RrcConnectionReconfiguration DecodeHandoverCommand (Ptr<Packet> p) = 0;

};


/**
 * \brief Part of the RRC protocol. This Service Access Point (SAP) is used to
 *        let the eNB RRC receive a message from a UE RRC.  Each method defined
 *        in this class corresponds to the reception of a message that is
 *        defined in Section 6.2.2 of TS 36.331.
 */
class cv2x_LteEnbRrcSapProvider : public cv2x_LteRrcSap
{
public:
  /// CompleteSetupUeParameters structure
  struct CompleteSetupUeParameters
  {
    cv2x_LteRlcSapUser* srb0SapUser; ///< SRB0 SAP user
    cv2x_LtePdcpSapUser* srb1SapUser; ///< SRB1 SAP user
  };

  /**
   * \brief Complete setup UE function
   * \param rnti the RNTI of UE which sent the message
   * \param params CompleteSetupUeParameters
   */
  virtual void CompleteSetupUe (uint16_t rnti, CompleteSetupUeParameters params) = 0;

  /**
   * \brief Receive an _RRCConnectionRequest_ message from a UE
   *        during an RRC connection establishment procedure
   *        (Section 5.3.3 of TS 36.331).
   * \param rnti the RNTI of UE which sent the message
   * \param msg the message
   */
  virtual void RecvRrcConnectionRequest (uint16_t rnti,
                                         RrcConnectionRequest msg) = 0;

  /**
   * \brief Receive an _RRCConnectionSetupComplete_ message from a UE
   *        during an RRC connection establishment procedure
   *        (Section 5.3.3 of TS 36.331).
   * \param rnti the RNTI of UE which sent the message
   * \param msg the message
   */
  virtual void RecvRrcConnectionSetupCompleted (uint16_t rnti,
                                                RrcConnectionSetupCompleted msg) = 0;

  /**
   * \brief Receive an _RRCConnectionReconfigurationComplete_ message from a UE
   *        during an RRC connection reconfiguration procedure
   *        (Section 5.3.5 of TS 36.331).
   * \param rnti the RNTI of UE which sent the message
   * \param msg the message
   */
  virtual void RecvRrcConnectionReconfigurationCompleted (uint16_t rnti,
                                                          RrcConnectionReconfigurationCompleted msg) = 0;

  /**
   * \brief Receive an _RRCConnectionReestablishmentRequest_ message from a UE
   *        during an RRC connection re-establishment procedure
   *        (Section 5.3.7 of TS 36.331).
   * \param rnti the RNTI of UE which sent the message
   * \param msg the message
   */
  virtual void RecvRrcConnectionReestablishmentRequest (uint16_t rnti,
                                                        RrcConnectionReestablishmentRequest msg) = 0;

  /**
   * \brief Receive an _RRCConnectionReestablishmentComplete_ message from a UE
   *        during an RRC connection re-establishment procedure
   *        (Section 5.3.7 of TS 36.331).
   * \param rnti the RNTI of UE which sent the message
   * \param msg the message
   */
  virtual void RecvRrcConnectionReestablishmentComplete (uint16_t rnti,
                                                         RrcConnectionReestablishmentComplete msg) = 0;

  /**
   * \brief Receive a _MeasurementReport_ message from a UE
   *        during a measurement reporting procedure
   *        (Section 5.5.5 of TS 36.331).
   * \param rnti the RNTI of UE which sent the message
   * \param msg the message
   */
  virtual void RecvMeasurementReport (uint16_t rnti, MeasurementReport msg) = 0;

  /**
   * \brief Receive a _SidelinkUeInformation_ message from a UE
   * \param rnti the RNTI of UE which sent the message
   * \param msg the message
   */
  virtual void RecvSidelinkUeInformation (uint16_t rnti, SidelinkUeInformation msg) = 0;

};






////////////////////////////////////
//   templates
////////////////////////////////////


/**
 * Template for the implementation of the cv2x_LteUeRrcSapUser as a member
 * of an owner class of type C to which all methods are forwarded
 *
 */
template <class C>
class cv2x_MemberLteUeRrcSapUser : public cv2x_LteUeRrcSapUser
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteUeRrcSapUser (C* owner);

  // inherited from cv2x_LteUeRrcSapUser
  virtual void Setup (SetupParameters params);
  virtual void SendRrcConnectionRequest (RrcConnectionRequest msg);
  virtual void SendRrcConnectionSetupCompleted (RrcConnectionSetupCompleted msg);
  virtual void SendRrcConnectionReconfigurationCompleted (RrcConnectionReconfigurationCompleted msg);
  virtual void SendRrcConnectionReestablishmentRequest (RrcConnectionReestablishmentRequest msg);
  virtual void SendRrcConnectionReestablishmentComplete (RrcConnectionReestablishmentComplete msg);
  virtual void SendMeasurementReport (MeasurementReport msg);
  virtual void SendSidelinkUeInformation (SidelinkUeInformation msg);

private:
  cv2x_MemberLteUeRrcSapUser ();
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteUeRrcSapUser<C>::cv2x_MemberLteUeRrcSapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
cv2x_MemberLteUeRrcSapUser<C>::cv2x_MemberLteUeRrcSapUser ()
{
}

template <class C>
void
cv2x_MemberLteUeRrcSapUser<C>::Setup (SetupParameters params)
{
  m_owner->DoSetup (params);
}

template <class C>
void
cv2x_MemberLteUeRrcSapUser<C>::SendRrcConnectionRequest (RrcConnectionRequest msg)
{
  m_owner->DoSendRrcConnectionRequest (msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapUser<C>::SendRrcConnectionSetupCompleted (RrcConnectionSetupCompleted msg)
{
  m_owner->DoSendRrcConnectionSetupCompleted (msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapUser<C>::SendRrcConnectionReconfigurationCompleted (RrcConnectionReconfigurationCompleted msg)
{
  m_owner->DoSendRrcConnectionReconfigurationCompleted (msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapUser<C>::SendRrcConnectionReestablishmentRequest (RrcConnectionReestablishmentRequest msg)
{
  m_owner->DoSendRrcConnectionReestablishmentRequest (msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapUser<C>::SendRrcConnectionReestablishmentComplete (RrcConnectionReestablishmentComplete msg)
{
  m_owner->DoSendRrcConnectionReestablishmentComplete (msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapUser<C>::SendMeasurementReport (MeasurementReport msg)
{
  m_owner->DoSendMeasurementReport (msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapUser<C>::SendSidelinkUeInformation (SidelinkUeInformation msg)
{
  m_owner->DoSendSidelinkUeInformation (msg);
}

/**
 * Template for the implementation of the cv2x_LteUeRrcSapProvider as a member
 * of an owner class of type C to which all methods are forwarded
 *
 */
template <class C>
class cv2x_MemberLteUeRrcSapProvider : public cv2x_LteUeRrcSapProvider
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteUeRrcSapProvider (C* owner);

  // methods inherited from cv2x_LteUeRrcSapProvider go here
  virtual void CompleteSetup (CompleteSetupParameters params);
  virtual void RecvSystemInformation (SystemInformation msg);
  virtual void RecvRrcConnectionSetup (RrcConnectionSetup msg);
  virtual void RecvRrcConnectionReconfiguration (RrcConnectionReconfiguration msg);
  virtual void RecvRrcConnectionReestablishment (RrcConnectionReestablishment msg);
  virtual void RecvRrcConnectionReestablishmentReject (RrcConnectionReestablishmentReject msg);
  virtual void RecvRrcConnectionRelease (RrcConnectionRelease msg);
  virtual void RecvRrcConnectionReject (RrcConnectionReject msg);

private:
  cv2x_MemberLteUeRrcSapProvider ();
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteUeRrcSapProvider<C>::cv2x_MemberLteUeRrcSapProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
cv2x_MemberLteUeRrcSapProvider<C>::cv2x_MemberLteUeRrcSapProvider ()
{
}

template <class C>
void
cv2x_MemberLteUeRrcSapProvider<C>::CompleteSetup (CompleteSetupParameters params)
{
  m_owner->DoCompleteSetup (params);
}

template <class C>
void
cv2x_MemberLteUeRrcSapProvider<C>::RecvSystemInformation (SystemInformation msg)
{
  Simulator::ScheduleNow (&C::DoRecvSystemInformation, m_owner, msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapProvider<C>::RecvRrcConnectionSetup (RrcConnectionSetup msg)
{
  Simulator::ScheduleNow (&C::DoRecvRrcConnectionSetup, m_owner, msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapProvider<C>::RecvRrcConnectionReconfiguration (RrcConnectionReconfiguration msg)
{
  Simulator::ScheduleNow (&C::DoRecvRrcConnectionReconfiguration, m_owner, msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapProvider<C>::RecvRrcConnectionReestablishment (RrcConnectionReestablishment msg)
{
  Simulator::ScheduleNow (&C::DoRecvRrcConnectionReestablishment, m_owner, msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapProvider<C>::RecvRrcConnectionReestablishmentReject (RrcConnectionReestablishmentReject msg)
{
  Simulator::ScheduleNow (&C::DoRecvRrcConnectionReestablishmentReject, m_owner, msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapProvider<C>::RecvRrcConnectionRelease (RrcConnectionRelease msg)
{
  Simulator::ScheduleNow (&C::DoRecvRrcConnectionRelease, m_owner, msg);
}

template <class C>
void
cv2x_MemberLteUeRrcSapProvider<C>::RecvRrcConnectionReject (RrcConnectionReject msg)
{
  Simulator::ScheduleNow (&C::DoRecvRrcConnectionReject, m_owner, msg);
}


/**
 * Template for the implementation of the cv2x_LteEnbRrcSapUser as a member
 * of an owner class of type C to which all methods are forwarded
 *
 */
template <class C>
class cv2x_MemberLteEnbRrcSapUser : public cv2x_LteEnbRrcSapUser
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteEnbRrcSapUser (C* owner);

  // inherited from cv2x_LteEnbRrcSapUser

  virtual void SetupUe (uint16_t rnti, SetupUeParameters params);
  virtual void RemoveUe (uint16_t rnti);
  virtual void SendSystemInformation (uint16_t cellId, SystemInformation msg);
  virtual void SendRrcConnectionSetup (uint16_t rnti, RrcConnectionSetup msg);
  virtual void SendRrcConnectionReconfiguration (uint16_t rnti, RrcConnectionReconfiguration msg);
  virtual void SendRrcConnectionReestablishment (uint16_t rnti, RrcConnectionReestablishment msg);
  virtual void SendRrcConnectionReestablishmentReject (uint16_t rnti, RrcConnectionReestablishmentReject msg);
  virtual void SendRrcConnectionRelease (uint16_t rnti, RrcConnectionRelease msg);
  virtual void SendRrcConnectionReject (uint16_t rnti, RrcConnectionReject msg);
  virtual Ptr<Packet> EncodeHandoverPreparationInformation (HandoverPreparationInfo msg);
  virtual HandoverPreparationInfo DecodeHandoverPreparationInformation (Ptr<Packet> p);
  virtual Ptr<Packet> EncodeHandoverCommand (RrcConnectionReconfiguration msg);
  virtual RrcConnectionReconfiguration DecodeHandoverCommand (Ptr<Packet> p);

private:
  cv2x_MemberLteEnbRrcSapUser ();
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteEnbRrcSapUser<C>::cv2x_MemberLteEnbRrcSapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
cv2x_MemberLteEnbRrcSapUser<C>::cv2x_MemberLteEnbRrcSapUser ()
{
}

template <class C>
void
cv2x_MemberLteEnbRrcSapUser<C>::SetupUe (uint16_t rnti, SetupUeParameters params)
{
  m_owner->DoSetupUe (rnti, params);
}

template <class C>
void
cv2x_MemberLteEnbRrcSapUser<C>::RemoveUe (uint16_t rnti)
{
  m_owner->DoRemoveUe (rnti);
}

template <class C>
void
cv2x_MemberLteEnbRrcSapUser<C>::SendSystemInformation (uint16_t cellId, SystemInformation msg)
{
  m_owner->DoSendSystemInformation (cellId, msg);
}

template <class C>
void
cv2x_MemberLteEnbRrcSapUser<C>::SendRrcConnectionSetup (uint16_t rnti, RrcConnectionSetup msg)
{
  m_owner->DoSendRrcConnectionSetup (rnti, msg);
}

template <class C>
void
cv2x_MemberLteEnbRrcSapUser<C>::SendRrcConnectionReconfiguration (uint16_t rnti, RrcConnectionReconfiguration msg)
{
  m_owner->DoSendRrcConnectionReconfiguration (rnti, msg);
}

template <class C>
void
cv2x_MemberLteEnbRrcSapUser<C>::SendRrcConnectionReestablishment (uint16_t rnti, RrcConnectionReestablishment msg)
{
  m_owner->DoSendRrcConnectionReestablishment (rnti, msg);
}

template <class C>
void
cv2x_MemberLteEnbRrcSapUser<C>::SendRrcConnectionReestablishmentReject (uint16_t rnti, RrcConnectionReestablishmentReject msg)
{
  m_owner->DoSendRrcConnectionReestablishmentReject (rnti, msg);
}

template <class C>
void
cv2x_MemberLteEnbRrcSapUser<C>::SendRrcConnectionRelease (uint16_t rnti, RrcConnectionRelease msg)
{
  m_owner->DoSendRrcConnectionRelease (rnti, msg);
}

template <class C>
void
cv2x_MemberLteEnbRrcSapUser<C>::SendRrcConnectionReject (uint16_t rnti, RrcConnectionReject msg)
{
  m_owner->DoSendRrcConnectionReject (rnti, msg);
}

template <class C>
Ptr<Packet>
cv2x_MemberLteEnbRrcSapUser<C>::EncodeHandoverPreparationInformation (HandoverPreparationInfo msg)
{
  return m_owner->DoEncodeHandoverPreparationInformation (msg);
}

template <class C>
cv2x_LteRrcSap::HandoverPreparationInfo
cv2x_MemberLteEnbRrcSapUser<C>::DecodeHandoverPreparationInformation (Ptr<Packet> p)
{
  return m_owner->DoDecodeHandoverPreparationInformation (p);
}


template <class C>
Ptr<Packet>
cv2x_MemberLteEnbRrcSapUser<C>::EncodeHandoverCommand (RrcConnectionReconfiguration msg)
{
  return m_owner->DoEncodeHandoverCommand (msg);
}

template <class C>
cv2x_LteRrcSap::RrcConnectionReconfiguration
cv2x_MemberLteEnbRrcSapUser<C>::DecodeHandoverCommand (Ptr<Packet> p)
{
  return m_owner->DoDecodeHandoverCommand (p);
}

/**
 * Template for the implementation of the cv2x_LteEnbRrcSapProvider as a member
 * of an owner class of type C to which all methods are forwarded
 *
 */
template <class C>
class cv2x_MemberLteEnbRrcSapProvider : public cv2x_LteEnbRrcSapProvider
{
public:
  /**
   * Constructor
   *
   * \param owner
   */
  cv2x_MemberLteEnbRrcSapProvider (C* owner);

  // methods inherited from cv2x_LteEnbRrcSapProvider go here

  virtual void CompleteSetupUe (uint16_t rnti, CompleteSetupUeParameters params);
  virtual void RecvRrcConnectionRequest (uint16_t rnti, RrcConnectionRequest msg);
  virtual void RecvRrcConnectionSetupCompleted (uint16_t rnti, RrcConnectionSetupCompleted msg);
  virtual void RecvRrcConnectionReconfigurationCompleted (uint16_t rnti, RrcConnectionReconfigurationCompleted msg);
  virtual void RecvRrcConnectionReestablishmentRequest (uint16_t rnti, RrcConnectionReestablishmentRequest msg);
  virtual void RecvRrcConnectionReestablishmentComplete (uint16_t rnti, RrcConnectionReestablishmentComplete msg);
  virtual void RecvMeasurementReport (uint16_t rnti, MeasurementReport msg);
  virtual void RecvSidelinkUeInformation (uint16_t rnti, SidelinkUeInformation msg);


private:
  cv2x_MemberLteEnbRrcSapProvider ();
  C* m_owner; ///< the owner class
  };

  template <class C>
  cv2x_MemberLteEnbRrcSapProvider<C>::cv2x_MemberLteEnbRrcSapProvider (C* owner)
    : m_owner (owner)
  {
  }

  template <class C>
  cv2x_MemberLteEnbRrcSapProvider<C>::cv2x_MemberLteEnbRrcSapProvider ()
  {
  }

  template <class C>
  void
  cv2x_MemberLteEnbRrcSapProvider<C>::CompleteSetupUe (uint16_t rnti, CompleteSetupUeParameters params)
  {
    m_owner->DoCompleteSetupUe (rnti, params);
  }

  template <class C>
  void
  cv2x_MemberLteEnbRrcSapProvider<C>::RecvRrcConnectionRequest (uint16_t rnti, RrcConnectionRequest msg)
  {
    Simulator::ScheduleNow (&C::DoRecvRrcConnectionRequest, m_owner, rnti, msg);
  }

  template <class C>
  void
  cv2x_MemberLteEnbRrcSapProvider<C>::RecvRrcConnectionSetupCompleted (uint16_t rnti, RrcConnectionSetupCompleted msg)
  {
    Simulator::ScheduleNow (&C::DoRecvRrcConnectionSetupCompleted, m_owner, rnti, msg);
  }

  template <class C>
  void
  cv2x_MemberLteEnbRrcSapProvider<C>::RecvRrcConnectionReconfigurationCompleted (uint16_t rnti, RrcConnectionReconfigurationCompleted msg)
  {
    Simulator::ScheduleNow (&C::DoRecvRrcConnectionReconfigurationCompleted, m_owner, rnti, msg);
  }

  template <class C>
  void
  cv2x_MemberLteEnbRrcSapProvider<C>::RecvRrcConnectionReestablishmentRequest (uint16_t rnti, RrcConnectionReestablishmentRequest msg)
  {
    Simulator::ScheduleNow (&C::DoRecvRrcConnectionReestablishmentRequest, m_owner, rnti, msg);
  }

  template <class C>
  void
  cv2x_MemberLteEnbRrcSapProvider<C>::RecvRrcConnectionReestablishmentComplete (uint16_t rnti, RrcConnectionReestablishmentComplete msg)
  {
    Simulator::ScheduleNow (&C::DoRecvRrcConnectionReestablishmentComplete, m_owner, rnti, msg);
  }

  template <class C>
  void
  cv2x_MemberLteEnbRrcSapProvider<C>::RecvMeasurementReport (uint16_t rnti, MeasurementReport msg)
  {
    Simulator::ScheduleNow (&C::DoRecvMeasurementReport, m_owner, rnti, msg);
  }

  template <class C>
  void
  cv2x_MemberLteEnbRrcSapProvider<C>::RecvSidelinkUeInformation (uint16_t rnti, SidelinkUeInformation msg)
  {
    Simulator::ScheduleNow (&C::DoRecvSidelinkUeInformation, m_owner, rnti, msg);
  }

} // namespace ns3

#endif // CV2X_LTE_RRC_SAP_H





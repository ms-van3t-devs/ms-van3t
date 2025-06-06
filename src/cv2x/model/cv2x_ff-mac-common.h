/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 * 
 * Modified by:
 *          Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *          Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#ifndef CV2X_FF_MAC_COMMON_H
#define CV2X_FF_MAC_COMMON_H

#include <ns3/simple-ref-count.h>
#include <ns3/ptr.h>
#include <vector>
#include <bitset>
#include <map>


/**
 * Constants. See section 4.4
 */
#define MAX_SCHED_CFG_LIST    10
#define MAX_LC_LIST           10

#define MAX_RACH_LIST         30
#define MAX_DL_INFO_LIST      30
#define MAX_BUILD_DATA_LIST   30
#define MAX_BUILD_RAR_LIST    10
#define MAX_BUILD_BC_LIST     3
#define MAX_UL_INFO_LIST      30
#define MAX_DCI_LIST          30
#define MAX_PHICH_LIST        30
#define MAX_TB_LIST           2
#define MAX_RLC_PDU_LIST      30
#define MAX_NR_LCG            4
#define MAX_MBSFN_CONFIG      5
#define MAX_SI_MSG_LIST       32
#define MAX_SI_MSG_SIZE       65535

#define MAX_CQI_LIST          30
#define MAX_UE_SELECTED_SB    6
#define MAX_HL_SB             25
#define MAX_SINR_RB_LIST      100
#define MAX_SR_LIST           30
#define MAX_MAC_CE_LIST       30

namespace ns3 {

/// Result_e enumeration
enum cv2x_Result_e
{
  cv2x_SUCCESS,
  cv2x_FAILURE
};

/// SetupRelease_e enumeration
enum cv2x_SetupRelease_e
{
  cv2x_setup,
  cv2x_release
};

/// CeBitmap_e
enum cv2x_CeBitmap_e
{
  cv2x_TA,
  cv2x_DRX,
  cv2x_CR
};

/// NormalExtended_e enumeration
enum cv2x_NormalExtended_e
{
  cv2x_normal,
  cv2x_extended
};


/**
 * \brief See section 4.3.1 dlDciListElement
 * \struct cv2x_DlDciListElement_s
 */
struct cv2x_DlDciListElement_s
{
  uint16_t  m_rnti; ///< RNTI
  uint32_t  m_rbBitmap; ///< rb bitmap
  uint8_t   m_rbShift; ///< rb shift
  uint8_t   m_resAlloc; ///< res allocate
  std::vector <uint16_t>  m_tbsSize; ///< tbs size
  std::vector <uint8_t>   m_mcs; ///< mcs
  std::vector <uint8_t>   m_ndi; ///< ndi
  std::vector <uint8_t>   m_rv; ///< rv
  uint8_t   m_cceIndex; ///< CCE index
  uint8_t   m_aggrLevel; ///< aggr level
  uint8_t   m_precodingInfo; ///< precoding info
  /// Format enumeration
  enum Format_e
  {
    ONE, ONE_A, ONE_B, ONE_C, ONE_D, TWO, TWO_A, TWO_B
  } m_format; ///< the format
  uint8_t   m_tpc; ///< TPC
  uint8_t   m_harqProcess; ///< HARQ process
  uint8_t   m_dai; ///< DAI
  /// Vrb Format enum
  enum VrbFormat_e
  {
    VRB_DISTRIBUTED,
    VRB_LOCALIZED
  } m_vrbFormat; ///< the format
  bool      m_tbSwap; ///< swap?
  bool      m_spsRelease; ///< release?
  bool      m_pdcchOrder; ///< cch order?
  uint8_t   m_preambleIndex; ///< preamble index
  uint8_t   m_prachMaskIndex; ///< RACH mask index
  /// Ngap enum
  enum Ngap_e
  {
    GAP1, GAP2
  } m_nGap; ///< the gap
  uint8_t   m_tbsIdx; ///< tbs index
  uint8_t   m_dlPowerOffset; ///< DL power offset
  uint8_t   m_pdcchPowerOffset; ///<  CCH power offset
};

/**
 * \brief See section 4.3.2 ulDciListElement
 * \struct cv2x_UlDciListElement_s
 */
struct cv2x_UlDciListElement_s
{
  uint16_t  m_rnti; ///< RNTI
  uint8_t   m_rbStart; ///< start
  uint8_t   m_rbLen; ///< length
  uint16_t  m_tbSize; ///< size
  uint8_t   m_mcs; ///< MCS
  uint8_t   m_ndi; ///< NDI
  uint8_t   m_cceIndex; ///< CCE index
  uint8_t   m_aggrLevel; ///< aggr level
  uint8_t   m_ueTxAntennaSelection; ///< UE antenna selection
  bool      m_hopping; ///< hopping?
  uint8_t   m_n2Dmrs; ///< n2 DMRS
  int8_t    m_tpc; ///< TPC
  bool      m_cqiRequest; ///< CQI request
  uint8_t   m_ulIndex; ///< UL index
  uint8_t   m_dai; ///< DAI
  uint8_t   m_freqHopping; ///< freq hopping
  int8_t    m_pdcchPowerOffset; ///< CCH power offset
};

/**
 * \brief See section 5.3.3.1.9 Rel 12.4
 */
struct cv2x_SlDciListElement_s
{
  uint16_t m_rnti;
  uint16_t m_resPscch;
  uint8_t m_tpc;
  uint8_t m_hopping;
  uint8_t m_rbStart; //models rb assignment
  uint8_t m_rbLen;   //models rb assignment
  uint8_t m_trp;
};

/**
 * \brief See section 5.4.3.1.1 Rel 12.4
 */
struct cv2x_SciListElement_s
{
  uint16_t  m_rnti;
  uint8_t   m_resPscch;       //added for modeling
  uint8_t   m_hopping;
  uint8_t   m_rbStart; //models rb assignment
  uint8_t   m_rbLen;   //models rb assignment
  uint16_t  m_tbSize;  //added for modeling
  uint8_t   m_trp;
  uint8_t   m_mcs;
  uint16_t  m_timing;
  uint8_t   m_groupDstId;
};
 
/**
 * \brief See 36.212 section 5.3.3.1.9A V15.0.1
 * DCI format 5A is used for scheduling of PSCCH, and also contains 
 * several SCI format 1 fields used for the scheduling of PSSCH
 */
struct cv2x_SlDciListElementV2x 
{
  uint8_t m_CIF;                      // carrier indicator 
  uint16_t m_firstSubchannelIdx;      // lowest index of the subchannel allocation to the inital transmission
  uint16_t m_RIV;                     // frequency resource location of initial transmission and retransmission 
  uint8_t m_SFgap;                    // time gap between initial transmission and retransmission
  uint8_t m_slIndex;                  // SL index (present only for cases with TDD operation with uplink-downlink configuration 0-6)
  uint8_t m_slSPSconfigIndex;         // SL SPS configuration index (present only when the format 5A CRC is scrambled with SL-SPS-V-RNTI)
  uint8_t m_indicator;                // Activation/release indication (present only when the format 5A CRC is scrambled with SL-SPS-V-RNTI)
}; 

/**
 * \brief See section 36.212 5.4.3.1.2 V15.0.1
 * SCI format 1 is used for scheduling of PSSCH 
 */
struct cv2x_SciListElementV2x  
{
  uint16_t m_rnti; 
  uint8_t m_prio;                     // priority - 3 bits
  uint16_t m_pRsvp;                    // resource reservation - 4 bits 
  uint16_t m_riv;                     // frequency resource location of initial transmission and retransmission - ceil(log2(N_subCH(N_subCH+1)/2)) bits
  uint8_t m_sfGap;                    // time gap between initial transmission and retransmission - 4 bits
  uint8_t m_mcs;                      // modulation and coding scheme - 5 bits
  uint8_t m_reTxIdx;                  // retransmission index - 1 bit
  
  uint16_t m_resPscch;           // added for modelling: resource where PSCCH occur (in subchannel)
  uint16_t m_tbSize;                  // added for modelling: transferblock size
};



struct cv2x_SlDiscMsg
{
  uint16_t  m_rnti; //added for modeling
  uint8_t   m_resPsdch; //added for modeling
  uint8_t m_msgType; 
  std::bitset <184> m_proSeAppCode;
  uint32_t m_mic;
  uint8_t m_utcBasedCounter;
};

/**
* \brief Base class for storing the values of vendor specific parameters
*/
struct cv2x_VendorSpecificValue : public SimpleRefCount<cv2x_VendorSpecificValue>
{ 
  virtual ~cv2x_VendorSpecificValue ();

};

/**
 * \brief See section 4.3.3 vendorSpecifiListElement
 * \struct cv2x_VendorSpecificListElement_s
 */
struct cv2x_VendorSpecificListElement_s
{
  uint32_t m_type; ///< type
  uint32_t m_length; ///< length
  Ptr<cv2x_VendorSpecificValue> m_value; ///< value
};

/**
 * \brief See section 4.3.4 logicalChannelConfigListElement
 * \struct cv2x_LogicalChannelConfigListElement_s
 */
struct cv2x_LogicalChannelConfigListElement_s
{
  uint8_t   m_logicalChannelIdentity; ///< logical channel indentity
  uint8_t   m_logicalChannelGroup; ///< logical channel group

  /// Direction enum
  enum Direction_e
  {
    DIR_UL,
    DIR_DL,
    DIR_BOTH
  } m_direction; ///< the direction

  /// QosBearerType enum
  enum QosBearerType_e
  {
    QBT_NON_GBR,
    QBT_GBR
  } m_qosBearerType; ///< the QOS bearer type

  uint8_t   m_qci; ///< QCI
  uint64_t  m_eRabMaximulBitrateUl; ///< ERAB maximum bit rate UL
  uint64_t  m_eRabMaximulBitrateDl; ///< ERAB mqximum bit rate DL
  uint64_t  m_eRabGuaranteedBitrateUl; ///< ERAB guaranteed bit rate UL
  uint64_t  m_eRabGuaranteedBitrateDl; ///< ERAB guaranteed bit rate DL
};

/**
 * \brief See section 4.3.6 rachListElement
 * \struct cv2x_RachListElement_s 
 */
struct cv2x_RachListElement_s
{
  uint16_t  m_rnti; ///< RNTI
  uint16_t  m_estimatedSize; ///< estimated size
};

/**
 * \brief See section 4.3.7 phichListElement
 * \struct cv2x_PhichListElement_s
 */
struct cv2x_PhichListElement_s
{
  uint16_t  m_rnti; ///< RNTI
  /// Phich enum
  enum Phich_e
  {
    ACK, NACK
  } m_phich; ///< the phich
};

/**
 * \brief See section 4.3.9 rlcPDU_ListElement
 */
struct cv2x_RlcPduListElement_s
{
  uint8_t   m_logicalChannelIdentity; ///< logical channel indentity
  uint16_t  m_size; ///< size
};

/**
 * \brief See section 4.3.8 builDataListElement
 * \struct cv2x_BuildDataListElement_s
 */
struct cv2x_BuildDataListElement_s
{
  uint16_t  m_rnti; ///< RNTI
  struct cv2x_DlDciListElement_s m_dci; ///< DCI
  std::vector <enum cv2x_CeBitmap_e> m_ceBitmap; ///< CE bitmap
  std::vector < std::vector <struct cv2x_RlcPduListElement_s> > m_rlcPduList; ///< RLC PDU list
};

/**
 * \brief Substitutive structure for specifying cv2x_BuildRarListElement_s::m_grant field
 */
struct cv2x_UlGrant_s
{
  uint16_t m_rnti; ///< RNTI
  uint8_t m_rbStart; ///< start
  uint8_t m_rbLen; ///< length
  uint16_t m_tbSize; ///< size
  uint8_t m_mcs; ///< MCS
  bool m_hopping; ///< hopping?
  int8_t m_tpc; ///< TPC 
  bool m_cqiRequest; ///< CQI request?
  bool m_ulDelay; ///< UL delay?
}; 

/**
 * \brief See section 4.3.10 buildRARListElement
 */
struct cv2x_BuildRarListElement_s
{
  uint16_t  m_rnti; ///< RNTI
  //uint32_t  m_grant; // Substituted with type cv2x_UlGrant_s
  cv2x_UlGrant_s m_grant; ///< grant
  struct cv2x_DlDciListElement_s m_dci; ///< DCI
};

/**
 * \brief See section 4.3.11 buildBroadcastListElement
 */
struct cv2x_BuildBroadcastListElement_s
{
  /// Type enum
  enum Type_e
  {
    BCCH, PCCH
  } m_type; ///< the type
  uint8_t m_index; ///< index
  struct cv2x_DlDciListElement_s m_dci; ///< DCI
};

/**
 * \brief See section 4.3.12 ulInfoListElement
 */
struct cv2x_UlInfoListElement_s
{
  uint16_t  m_rnti; ///< RNTI
  std::vector <uint16_t> m_ulReception; ///< UL reception
  /// Reception status enum
  enum ReceptionStatus_e 
  {
    Ok, NotOk, NotValid
  } m_receptionStatus; ///< the status
  uint8_t   m_tpc; ///< TPC
};

/**
 * \brief See section 4.3.13 srListElement
 */
struct cv2x_SrListElement_s
{
  uint16_t  m_rnti; ///< RNTI
};

/**
 * \brief See section 4.3.15 macCEValue
 */
struct cv2x_MacCeValue_u
{
  uint8_t   m_phr; ///< phr
  uint8_t   m_crnti; ///< NRTI
  std::vector <uint8_t> m_bufferStatus; ///< buffer status
  std::map <uint8_t, std::vector <uint8_t> > m_SlBufferStatus; ///< modified structure m_bufferStatus to handle UL different bearer scheduling
};

/**
 * \brief See section 4.3.14 macCEListElement
 */
struct cv2x_MacCeListElement_s
{
  uint16_t  m_rnti; ///< RNTI
  /// MAC CE type enum
  enum MacCeType_e
  {
    BSR, PHR, CRNTI, SLBSR
  } m_macCeType; ///< MAC CE type
  struct cv2x_MacCeValue_u m_macCeValue; ///< MAC CE value
};

/**
 * \brief See section 4.3.16 drxConfig
 */
struct cv2x_DrxConfig_s
{
  uint8_t   m_onDurationTimer; ///< on duration timer
  uint16_t  m_drxInactivityTimer; ///< inactivity timer
  uint16_t  m_drxRetransmissionTimer; ///< retransmission timer
  uint16_t  m_longDrxCycle; ///< long DRX cycle 
  uint16_t  m_longDrxCycleStartOffset; ///< long DRX cycle start offset
  uint16_t  m_shortDrxCycle; ///< short DRX cycle
  uint8_t   m_drxShortCycleTimer; ///< short DRX cycle timer
};

/**
 * \brief See section 4.3.17 spsConfig
 */
struct cv2x_SpsConfig_s
{
  uint16_t  m_semiPersistSchedIntervalUl; ///< UL semi persist schedule interval
  uint16_t  m_semiPersistSchedIntervalDl; ///< DL semi persist schedule interval
  uint8_t   m_numberOfConfSpsProcesses; ///< number of conf SPS process
  uint8_t   m_n1PucchAnPersistentListSize; ///< N1pu CCH  persistent list size
  std::vector <uint16_t> m_n1PucchAnPersistentList; ///< N1pu CCH persistent list
  uint8_t   m_implicitReleaseAfter; ///< implicit release after
};

/**
 * \brief See section 4.3.18 srConfig
 */
struct cv2x_SrConfig_s
{
  enum cv2x_SetupRelease_e m_action; ///< action
  uint8_t   m_schedInterval; ///< sched interval
  uint8_t   m_dsrTransMax; ///< trans max
};

/**
 * \brief See section 4.3.19 cqiConfig
 */
struct cv2x_CqiConfig_s
{
  enum cv2x_SetupRelease_e m_action; ///< action
  uint16_t  m_cqiSchedInterval; ///< CQI schedule interval
  uint8_t   m_riSchedInterval; ///< RI schedule interval
};

/**
 * \brief See section 4.3.20 ueCapabilities
 */
struct cv2x_UeCapabilities_s
{
  bool      m_halfDuplex; ///< half duplex
  bool      m_intraSfHopping; ///< intra SF hopping
  bool      m_type2Sb1; ///< type 2Sb1
  uint8_t   m_ueCategory; ///< UE category
  bool      m_resAllocType1; ///< alloc type 1
};

/**
 * \brief See section 4.3.22 siMessageListElement
 */
struct cv2x_SiMessageListElement_s
{
  uint16_t  m_periodicity; ///< periodicity
  uint16_t  m_length; ///< length
};

/**
 * \brief See section 4.3.21 siConfiguration
 */
struct cv2x_SiConfiguration_s
{
  uint16_t  m_sfn; ///< sfn
  uint16_t  m_sib1Length; ///< sib1 length
  uint8_t   m_siWindowLength; ///< window length
  std::vector <struct cv2x_SiMessageListElement_s> m_siMessageList; ///< message list
};

/**
 * \brief See section 4.3.23 dlInfoListElement
 */
struct cv2x_DlInfoListElement_s
{
  uint16_t  m_rnti; ///< RNTI
  uint8_t   m_harqProcessId; ///< HARQ process ID
  /// HARQ status enum
  enum HarqStatus_e
  {
    ACK, NACK, DTX
  };
  std::vector <enum HarqStatus_e> m_harqStatus; ///< HARQ status
};

/**
 * \brief See section 4.3.28 bwPart
 */
struct cv2x_BwPart_s
{
  uint8_t   m_bwPartIndex; ///< bw part index
  uint8_t   m_sb; ///< sb
  uint8_t   m_cqi; ///< CQI
};

/**
 * \brief See section 4.3.27 higherLayerSelected
 */
struct cv2x_HigherLayerSelected_s
{
  uint8_t   m_sbPmi; ///< sb PMI
  std::vector <uint8_t> m_sbCqi; ///< sb CQI
};

/**
 * \brief See section 4.3.26 ueSelected
 */
struct cv2x_UeSelected_s
{
  std::vector <uint8_t> m_sbList; ///< sb list
  uint8_t   m_sbPmi; ///< sb PMI
  std::vector <uint8_t> m_sbCqi; ///< sb CQI
};

/**
 * \brief See section 4.3.25 sbMeasResult
 */
struct cv2x_SbMeasResult_s
{
  struct cv2x_UeSelected_s           m_ueSelected; ///< UE selected
  std::vector <struct cv2x_HigherLayerSelected_s> m_higherLayerSelected; ///< higher layer selected
  struct cv2x_BwPart_s               m_bwPart; ///< bw part
};

/**
 * \brief See section 4.3.24 cqiListElement
 */
struct cv2x_CqiListElement_s
{
  uint16_t  m_rnti; ///< RNTI
  uint8_t   m_ri; ///< RI
  /// CqiType_e enumeration
  enum CqiType_e
  {
    P10, P11, P20, P21, A12, A22, A20, A30, A31
  } m_cqiType; ///< CQI type
  std::vector <uint8_t> m_wbCqi; ///< wb CQI
  uint8_t   m_wbPmi; ///< wb PMI

  struct cv2x_SbMeasResult_s m_sbMeasResult; ///< sb measure result
};

/**
 * \brief See section 4.3.29 ulCQI
 */
struct cv2x_UlCqi_s
{
  std::vector <uint16_t> m_sinr; ///< SINR
  /// Type_e enumeration
  enum Type_e
  {
    SRS,
    PUSCH,
    PUCCH_1,
    PUCCH_2,
    PRACH
  } m_type; ///< type
};

/**
 * \brief See section 4.3.30 pagingInfoListElement
 */
struct cv2x_PagingInfoListElement_s
{
  uint8_t   m_pagingIndex; ///< paging index
  uint16_t  m_pagingMessageSize; ///< paging message size
  uint8_t   m_pagingSubframe; ///< paging subframe
};

} // namespace ns3

#endif /* CV2X_FF_MAC_COMMON_H */

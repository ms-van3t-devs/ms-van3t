/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Marco Miozzo  <marco.miozzo@cttc.es>
 *         Nicola Baldo  <nbaldo@cttc.es>
 * Modified by:
 *          Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 *          Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)
 *          NIST (D2D)
 */

#ifndef CV2X_LTE_ENB_MAC_H
#define CV2X_LTE_ENB_MAC_H


#include <map>
#include <vector>
#include <ns3/cv2x_lte-common.h>
#include <ns3/cv2x_lte-mac-sap.h>
#include <ns3/cv2x_lte-enb-cmac-sap.h>
#include <ns3/cv2x_ff-mac-csched-sap.h>
#include <ns3/cv2x_ff-mac-sched-sap.h>
#include <ns3/cv2x_lte-enb-phy-sap.h>
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include <ns3/packet.h>
#include <ns3/packet-burst.h>
#include <ns3/cv2x_lte-ccm-mac-sap.h>

namespace ns3 {

class cv2x_DlCqiLteControlMessage;
class UlCqiLteControlMessage;
class PdcchMapLteControlMessage;

/// DlHarqProcessesBuffer_t typedef
typedef std::vector <std::vector < Ptr<PacketBurst> > > DlHarqProcessesBuffer_t;

/**
 * This class implements the MAC layer of the eNodeB device
 */
class cv2x_LteEnbMac :   public Object
{
  /// allow cv2x_EnbMacMemberLteEnbCmacSapProvider class friend access
  friend class cv2x_EnbMacMemberLteEnbCmacSapProvider;
  /// allow cv2x_EnbMacMemberLteMacSapProvider<cv2x_LteEnbMac> class friend access
  friend class cv2x_EnbMacMemberLteMacSapProvider<cv2x_LteEnbMac>;
  /// allow cv2x_EnbMacMemberFfMacSchedSapUser class friend access
  friend class cv2x_EnbMacMemberFfMacSchedSapUser;
  /// allow cv2x_EnbMacMemberFfMacCschedSapUser class friend access
  friend class cv2x_EnbMacMemberFfMacCschedSapUser;
  /// allow cv2x_EnbMacMemberLteEnbPhySapUser class friend access
  friend class cv2x_EnbMacMemberLteEnbPhySapUser;
  /// allow cv2x_MemberLteCcmMacSapProvider<cv2x_LteEnbMac> class friend access
  friend class cv2x_MemberLteCcmMacSapProvider<cv2x_LteEnbMac>;

public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  cv2x_LteEnbMac (void);
  virtual ~cv2x_LteEnbMac (void);
  virtual void DoDispose (void);

  /**
   * \brief Set the component carrier ID
   * \param index the component carrier ID
   */
  void SetComponentCarrierId (uint8_t index);
  /**
   * \brief Set the scheduler SAP provider
   * \param s a pointer SAP provider of the FF packet scheduler
   */
  void SetFfMacSchedSapProvider (cv2x_FfMacSchedSapProvider* s);
  /**
   * \brief Get the scheduler SAP user
   * \return a pointer to the SAP user of the scheduler
   */
  cv2x_FfMacSchedSapUser* GetFfMacSchedSapUser (void);
  /**
   * \brief Set the control scheduler SAP provider
   * \param s a pointer to the control scheduler SAP provider
   */
  void SetFfMacCschedSapProvider (cv2x_FfMacCschedSapProvider* s);
  /**
   * \brief Get the control scheduler SAP user
   * \return a pointer to the control scheduler SAP user
   */
  cv2x_FfMacCschedSapUser* GetFfMacCschedSapUser (void);



  /**
   * \brief Set the MAC SAP user
   * \param s a pointer to the MAC SAP user
   */
  void SetLteMacSapUser (cv2x_LteMacSapUser* s);
  /**
   * \brief Get the MAC SAP provider
   * \return a pointer to the SAP provider of the MAC
   */
  cv2x_LteMacSapProvider* GetLteMacSapProvider (void);
  /**
   * \brief Set the control MAC SAP user
   * \param s a pointer to the control MAC SAP user
   */
  void SetLteEnbCmacSapUser (cv2x_LteEnbCmacSapUser* s);
  /**
   * \brief Get the control MAC SAP provider
   * \return a pointer to the control MAC SAP provider
   */
  cv2x_LteEnbCmacSapProvider* GetLteEnbCmacSapProvider (void);


  /**
  * \brief Get the eNB-PHY SAP User
  * \return a pointer to the SAP User of the PHY
  */
  cv2x_LteEnbPhySapUser* GetLteEnbPhySapUser ();

  /**
  * \brief Set the PHY SAP Provider
  * \param s a pointer to the PHY SAP provider
  */
  void SetLteEnbPhySapProvider (cv2x_LteEnbPhySapProvider* s);

  /**
  * \brief Get the eNB-cv2x_ComponentCarrierManager SAP User
  * \return a pointer to the SAP User of the cv2x_ComponentCarrierManager
  */
  cv2x_LteCcmMacSapProvider* GetLteCcmMacSapProvider ();

  /**
  * \brief Set the cv2x_ComponentCarrierManager SAP user
  * \param s a pointer to the cv2x_ComponentCarrierManager provider
  */
  void SetLteCcmMacSapUser (cv2x_LteCcmMacSapUser* s);
  

  /**
   * TracedCallback signature for DL scheduling events.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] mcs0 The MCS for transport block.. 
   * \param [in] tbs0Size
   * \param [in] mcs1 The MCS for transport block.
   * \param [in] tbs1Size
   * \param [in] component carrier id
   */
  typedef void (* DlSchedulingTracedCallback)
    (const uint32_t frame, const uint32_t subframe, const uint16_t rnti,
     const uint8_t mcs0, const uint16_t tbs0Size,
     const uint8_t mcs1, const uint16_t tbs1Size, const uint8_t ccId);

  /**
   *  TracedCallback signature for UL scheduling events.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] mcs  The MCS for transport block
   * \param [in] tbsSize
   */
  typedef void (* UlSchedulingTracedCallback)
    (const uint32_t frame, const uint32_t subframe, const uint16_t rnti,
     const uint8_t mcs, const uint16_t tbsSize);
  
private:

  /**
  * \brief Receive a DL CQI ideal control message
  * \param msg the DL CQI message
  */
  void ReceiveDlCqiLteControlMessage  (Ptr<cv2x_DlCqiLteControlMessage> msg);

  /**
  * \brief Receive a DL CQI ideal control message
  * \param msg the DL CQI message
  */
  void DoReceiveLteControlMessage (Ptr<cv2x_LteControlMessage> msg);

  /**
  * \brief Receive a CE element containing the buffer status report
  * \param bsr the BSR message
  */
  void ReceiveBsrMessage  (cv2x_MacCeListElement_s bsr);

  /**
  * \brief UL CQI report
  * \param ulcqi cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters
  */
  void DoUlCqiReport (cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi);



  // forwarded from cv2x_LteEnbCmacSapProvider
  /**
  * \brief Configure MAC function
  * \param ulBandwidth the UL bandwidth
  * \param dlBandwidth the DL bandwidth
  */
  void DoConfigureMac (uint8_t ulBandwidth, uint8_t dlBandwidth);
  /**
  * \brief Add UE function
  * \param rnti the RNTI
  */
  void DoAddUe (uint16_t rnti);
  /**
  * \brief Remove UE function
  * \param rnti the RNTI
  */
  void DoRemoveUe (uint16_t rnti);
  /**
  * \brief Add LC function
  * \param lcinfo the LC info
  * \param msu the LTE MAC SAP user
  */
  void DoAddLc (cv2x_LteEnbCmacSapProvider::LcInfo lcinfo, cv2x_LteMacSapUser* msu);
  /**
  * \brief Reconfigure LC function
  * \param lcinfo the LC info
  */
  void DoReconfigureLc (cv2x_LteEnbCmacSapProvider::LcInfo lcinfo);
  /**
  * \brief Release LC function
  * \param rnti the RNTI
  * \param lcid the LCID
  */
  void DoReleaseLc (uint16_t  rnti, uint8_t lcid);
  /**
  * \brief UE Update configuration request function
  * \param params cv2x_LteEnbCmacSapProvider::UeConfig
  */
  void DoUeUpdateConfigurationReq (cv2x_LteEnbCmacSapProvider::UeConfig params);
  /**
   * \brief add pool function
   * \param group group
   * \param pool Ptr<SidelinkCommResourcePool>
   */
  void DoAddPool (uint32_t group, Ptr<SidelinkCommResourcePool> pool);
  /**
   * \brief remove pool function
   * \param group group
   */
  void DoRemovePool (uint32_t group);
  /**
  * \brief Get RACH configuration function
  * \returns cv2x_LteEnbCmacSapProvider::RachConfig
  */
  cv2x_LteEnbCmacSapProvider::RachConfig DoGetRachConfig ();
  /**
  * \brief Allocate NC RA preamble function
  * \param rnti the RNTI
  * \returns cv2x_LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue
  */
  cv2x_LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue DoAllocateNcRaPreamble (uint16_t rnti);

  // forwarded from cv2x_LteMacSapProvider
  /**
  * \brief Transmit PDU function
  * \param params cv2x_LteMacSapProvider::TransmitPduParameters
  */
  void DoTransmitPdu (cv2x_LteMacSapProvider::TransmitPduParameters params);
  /**
  * \brief Report Buffer Status function
  * \param params cv2x_LteMacSapProvider::ReportBufferStatusParameters
  */
  void DoReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params);


  // forwarded from FfMacCchedSapUser
  /**
  * \brief CSched Cell Config configure function
  * \param params cv2x_FfMacCschedSapUser::CschedCellConfigCnfParameters
  */
  void DoCschedCellConfigCnf (cv2x_FfMacCschedSapUser::CschedCellConfigCnfParameters params);
  /**
  * \brief CSched UE Config configure function
  * \param params cv2x_FfMacCschedSapUser::CschedUeConfigCnfParameters
  */
  void DoCschedUeConfigCnf (cv2x_FfMacCschedSapUser::CschedUeConfigCnfParameters params);
  /**
  * \brief CSched LC Config configure function
  * \param params cv2x_FfMacCschedSapUser::CschedLcConfigCnfParameters
  */
  void DoCschedLcConfigCnf (cv2x_FfMacCschedSapUser::CschedLcConfigCnfParameters params);
  /**
  * \brief CSched LC Release configure function
  * \param params cv2x_FfMacCschedSapUser::CschedLcReleaseCnfParameters
  */
  void DoCschedLcReleaseCnf (cv2x_FfMacCschedSapUser::CschedLcReleaseCnfParameters params);
  /**
  * \brief CSched UE Release configure function
  * \param params cv2x_FfMacCschedSapUser::CschedUeReleaseCnfParameters
  */
  void DoCschedUeReleaseCnf (cv2x_FfMacCschedSapUser::CschedUeReleaseCnfParameters params);
  /**
  * \brief CSched UE Config Update Indication function
  * \param params cv2x_FfMacCschedSapUser::CschedUeConfigUpdateIndParameters
  */
  void DoCschedUeConfigUpdateInd (cv2x_FfMacCschedSapUser::CschedUeConfigUpdateIndParameters params);
  /**
  * \brief CSched Cell Config Update Indication function
  * \param params cv2x_FfMacCschedSapUser::CschedCellConfigUpdateIndParameters
  */
  void DoCschedCellConfigUpdateInd (cv2x_FfMacCschedSapUser::CschedCellConfigUpdateIndParameters params);

  // forwarded from cv2x_FfMacSchedSapUser
  /**
  * \brief Sched DL Config Indication function
  * \param ind cv2x_FfMacSchedSapUser::SchedDlConfigIndParameters
  */
  void DoSchedDlConfigInd (cv2x_FfMacSchedSapUser::SchedDlConfigIndParameters ind);
  /**
  * \brief Sched UL Config Indication function
  * \param params cv2x_FfMacSchedSapUser::SchedUlConfigIndParameters
  */
  void DoSchedUlConfigInd (cv2x_FfMacSchedSapUser::SchedUlConfigIndParameters params);

  // forwarded from cv2x_LteEnbPhySapUser
  /**
  * \brief Subrame Indication function
  * \param frameNo frame number
  * \param subframeNo subframe number
  */
  void DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo);
  /**
  * \brief Receive RACH Preamble function
  * \param prachId PRACH ID number
  */
  void DoReceiveRachPreamble (uint8_t prachId);

  // forwarded by cv2x_LteCcmMacSapProvider
  /**
   * Report MAC CE to scheduler
   * \param bsr the BSR
   */
  void DoReportMacCeToScheduler (cv2x_MacCeListElement_s bsr);
  
public:
  /**
   * legacy public for use the Phy callback
   * \param p packet
   */
  void DoReceivePhyPdu (Ptr<Packet> p);

private:
  /**
  * \brief UL Info List ELements HARQ Feedback function
  * \param params cv2x_UlInfoListElement_s
  */
  void DoUlInfoListElementHarqFeeback (cv2x_UlInfoListElement_s params);
  /**
  * \brief DL Info List ELements HARQ Feedback function
  * \param params cv2x_DlInfoListElement_s
  */
  void DoDlInfoListElementHarqFeeback (cv2x_DlInfoListElement_s params);

  /// rnti, lcid, SAP of the RLC instance
  std::map <uint16_t, std::map<uint8_t, cv2x_LteMacSapUser*> > m_rlcAttached;

  std::vector <cv2x_CqiListElement_s> m_dlCqiReceived; ///< DL-CQI received
  std::vector <cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters> m_ulCqiReceived; ///< UL-CQI received
  std::vector <cv2x_MacCeListElement_s> m_ulCeReceived; ///< CE received (BSR up to now)

  std::vector <cv2x_DlInfoListElement_s> m_dlInfoListReceived; ///< DL HARQ feedback received

  std::vector <cv2x_UlInfoListElement_s> m_ulInfoListReceived; ///< UL HARQ feedback received


  /*
  * Map of UE's info element (see 4.3.12 of FF MAC Scheduler API)
  */
  //std::map <uint16_t,cv2x_UlInfoListElement_s> m_ulInfoListElements; 



  cv2x_LteMacSapProvider* m_macSapProvider; ///< the MAC SAP provider
  cv2x_LteEnbCmacSapProvider* m_cmacSapProvider; ///< the CMAC SAP provider
  cv2x_LteMacSapUser* m_macSapUser; ///< the MAC SAP user
  cv2x_LteEnbCmacSapUser* m_cmacSapUser; ///< the CMAC SAP user


  cv2x_FfMacSchedSapProvider* m_schedSapProvider; ///< the Sched SAP provider
  cv2x_FfMacCschedSapProvider* m_cschedSapProvider; ///< the Csched SAP provider
  cv2x_FfMacSchedSapUser* m_schedSapUser; ///< the Sched SAP user
  cv2x_FfMacCschedSapUser* m_cschedSapUser; ///< the CSched SAP user

  // PHY-SAP
  cv2x_LteEnbPhySapProvider* m_enbPhySapProvider; ///< the ENB Phy SAP provider
  cv2x_LteEnbPhySapUser* m_enbPhySapUser; ///< the ENB Phy SAP user

  // Sap For cv2x_ComponentCarrierManager 'Uplink case'
  cv2x_LteCcmMacSapProvider* m_ccmMacSapProvider; ///< CCM MAC SAP provider
  cv2x_LteCcmMacSapUser* m_ccmMacSapUser; ///< CCM MAC SAP user
  /**
   * frame number of current subframe indication
   */
  uint32_t m_frameNo;
  /**
   * subframe number of current subframe indication
   */
  uint32_t m_subframeNo;
  /**
   * Trace information regarding DL scheduling
   * Frame number, Subframe number, RNTI, MCS of TB1, size of TB1,
   * MCS of TB2 (0 if not present), size of TB2 (0 if not present)
   */
  TracedCallback<cv2x_DlSchedulingCallbackInfo> m_dlScheduling;

  /**
   * Trace information regarding UL scheduling
   * Frame number, Subframe number, RNTI, MCS of TB, size of TB, component carrier id
   */
  TracedCallback<uint32_t, uint32_t, uint16_t,
                 uint8_t, uint16_t, uint8_t> m_ulScheduling;
  
  uint8_t m_macChTtiDelay; ///< delay of MAC, PHY and channel in terms of TTIs


  std::map <uint16_t, DlHarqProcessesBuffer_t> m_miDlHarqProcessesPackets; ///< Packet under transmission of the DL HARQ process
  
  uint8_t m_numberOfRaPreambles; ///< number of RA preambles
  uint8_t m_preambleTransMax; ///< preamble transmit maximum
  uint8_t m_raResponseWindowSize; ///< RA response window size

  /**
   * info associated with a preamble allocated for non-contention based RA
   * 
   */
  struct NcRaPreambleInfo
  {   
    uint16_t rnti; ///< rnti previously allocated for this non-contention based RA procedure
    Time expiryTime; ///< value the expiration time of this allocation (so that stale preambles can be reused)
  };

  /**
   * map storing as key the random access preamble IDs allocated for
   * non-contention based access, and as value the associated info
   * 
   */
  std::map<uint8_t, NcRaPreambleInfo> m_allocatedNcRaPreambleMap;
 
  std::map<uint8_t, uint32_t> m_receivedRachPreambleCount; ///< received RACH preamble count

  std::map<uint8_t, uint32_t> m_rapIdRntiMap; ///< RAPID RNTI map

  /// component carrier Id used to address sap
  uint8_t m_componentCarrierId;
 
};

} // end namespace ns3

#endif /* LTE_ENB_MAC_ENTITY_H */

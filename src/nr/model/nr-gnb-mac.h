/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef NR_ENB_MAC_H
#define NR_ENB_MAC_H

#include "nr-phy-mac-common.h"
#include "nr-mac-sched-sap.h"
#include "nr-phy-sap.h"
#include "nr-mac-scheduler.h"
#include "nr-mac-pdu-info.h"

#include <ns3/lte-enb-cmac-sap.h>
#include <ns3/lte-mac-sap.h>
#include <ns3/lte-ccm-mac-sap.h>
#include <ns3/lte-mac-sap.h>
#include <ns3/lte-enb-cmac-sap.h>
#include <ns3/traced-callback.h>

namespace ns3 {

class NrControlMessage;
class NrRarMessage;
class BeamConfId;

/**
 * \ingroup gnb-mac
 * \brief The MAC class for the gnb
 *
 * \section gnb_mac_general General information
 *
 * \todo fill gnb-mac general information doxygen part
 *
 * \section gnb_mac_configuration Configuration
 *
 * The user can configure the class using the method NrHelper::SetGnbMacAttribute(),
 * or by directly calling `SetAttribute` on the MAC pointer. The list of
 * attributes is reported below, in the Attributes section.
 *
 * \section gnb_mac_trace CTRL Traces for CTRL messages
 *
 * The class has two attributes that signals to the eventual listener the
 * transmission or the reception of CTRL messages. One is GnbMacRxedCtrlMsgsTrace,
 * and the other is GnbMacTxedCtrlMsgsTrace. For what regards the UE, you will
 * find more information in the NrUePhy class documentation.
 */
class NrGnbMac : public Object
{
  friend class NrGnbMacMemberEnbCmacSapProvider;
  friend class NrMacEnbMemberPhySapUser;
  friend class NrMacMemberMacCschedSapUser;
  friend class NrMacMemberMacSchedSapUser;
  friend class EnbMacMemberLteMacSapProvider<NrGnbMac>;
  friend class MemberLteCcmMacSapProvider<NrGnbMac>;

public:
  /**
   * \brief Get the TypeId
   * \return the TypeId
   */
  static TypeId GetTypeId (void);
  /**
   * \brief NrGnbMac constructor
   */
  NrGnbMac (void);
  /**
   * \brief ~NrGnbMac
   */
  virtual ~NrGnbMac (void) override;

  /**
   * \brief Sets the number of RBs per RBG. Currently it can be 
   * configured by the user, while in the future it will be configured 
   * by the RRC based on the type of configuration and the bandwidth.
   * \param rbgSize Number of RBs per RBG
   */
  void SetNumRbPerRbg (uint32_t rbgSize);

  /**
   * \return The number of resource blocks per resource block group.
   * This function will be called through SAP interfaces by PHY and scheduler,
   * to obtain this information from MAC.
   * Note that this functions can be named without "Do" prefix,
   * since it does not change the state of the object and can be exposed to
   * everyone, not only through SAP.
   *
   */
  uint32_t GetNumRbPerRbg (void) const;

  /**
   * \brief Sets the number of HARQ processes
   * \param numHarqProcess the maximum number of harq processes
   */
  void SetNumHarqProcess (uint8_t numHarqProcess);

  /**
   * \return number of HARQ processes
   */
  uint8_t GetNumHarqProcess () const;

  /**
   * \brief Retrieve the number of DL ctrl symbols configured in the scheduler
   * \return the number of DL ctrl symbols
   */
  virtual uint8_t GetDlCtrlSyms () const;

  /**
   * \brief Retrieve the number of UL ctrl symbols configured in the scheduler
   * \return the number of UL ctrl symbols
   */
  virtual uint8_t GetUlCtrlSyms () const;

  /**
   * \brief Perform DL scheduling decision for the indicated slot
   * \param sfnSf the slot to fill with scheduling decisions
   * \param type TDD slot type
   *
   * The MAC should perform its operations (including the scheduler) for DL.
   * Please note that what is decided in this slot will reach the air later
   * (depending on the L1L2CTRL latency parameter).
   */
  virtual void DoSlotDlIndication (const SfnSf &sfnSf, LteNrTddSlotType type);

  /**
   * \brief Perform UL scheduling decision for the indicated slot
   * \param sfnSf the slot to fill with scheduling decisions
   * \param type TDD slot type
   *
   * The MAC should perform its operations (including the scheduler) for UL.
   * Please note that what is decided in this slot will reach the air later
   * (depending on the L1L2CTRL latency and the UL Sched delay (K2) parameters).
   */
  virtual void DoSlotUlIndication (const SfnSf &sfnSf, LteNrTddSlotType type) ;

  /**
   * \brief Set the current sfn
   * \param sfn Current sfn
   */
  virtual void SetCurrentSfn (const SfnSf &sfn);

  void SetForwardUpCallback (Callback <void, Ptr<Packet> > cb);

  NrGnbPhySapUser* GetPhySapUser ();
  void SetPhySapProvider (NrPhySapProvider* ptr);

  NrMacSchedSapUser* GetNrMacSchedSapUser ();
  void SetNrMacSchedSapProvider (NrMacSchedSapProvider* ptr);

  NrMacCschedSapUser* GetNrMacCschedSapUser ();
  void SetNrMacCschedSapProvider (NrMacCschedSapProvider* ptr);

  LteMacSapProvider* GetMacSapProvider (void);
  LteEnbCmacSapProvider* GetEnbCmacSapProvider (void);

  void SetEnbCmacSapUser (LteEnbCmacSapUser* s);



  /**
  * \brief Get the gNB-ComponentCarrierManager SAP User
  * \return a pointer to the SAP User of the ComponentCarrierManager
  */
  LteCcmMacSapProvider* GetLteCcmMacSapProvider ();

  /**
  * \brief Set the ComponentCarrierManager SAP user
  * \param s a pointer to the ComponentCarrierManager provider
  */
  void SetLteCcmMacSapUser (LteCcmMacSapUser* s);

  /**
   * \brief A BeamConf for a user has changed
   * \param beamConfId new beam ID
   * \param rnti RNTI of the user
   */
  void BeamChangeReport (BeamConfId beamConfId, uint8_t rnti);

  /**
   * TracedCallback signature for DL and UL data scheduling events.
   *
   * \param [in] frame Frame number
   * \param [in] subframe Subframe number
   * \param [in] slotNum Slot number
   * \param [in] symStart Symbol start
   * \param [in] numSym Number of symbols
   * \param [in] streamId Stream id
   * \param [in] tbSize The TB size
   * \param [in] mcs MCS
   * \param [in] rnti RNTI
   * \param [in] bwpId BandWidth Part id
   * ...
   */
  typedef void (*SchedulingTracedCallback)(uint32_t frameNum, uint32_t subframeNum,
                                              uint32_t slotNum, uint8_t symStart,
                                              uint8_t numSym, uint8_t streamId,
                                              uint32_t tbSize, uint32_t mcs,
                                              uint32_t rnti, uint8_t bwpId);

  /**
   * TracedCallback signature for SR scheduling events.
   *
   * \param [in] rnti The C-RNTI identifying the UE.
   * \param [in] bwpId The component carrier ID of this MAC.
   */
  typedef void (* SrTracedCallback) (const uint8_t bwpId, const uint16_t rnti);

  /**
   *  TracedCallback signature for Enb Mac Received Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] nodeId
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* RxedGnbMacCtrlMsgsTracedCallback)
      (const SfnSf sfn, const uint16_t nodeId, const uint16_t rnti,
       const uint8_t bwpId, Ptr<NrControlMessage>);

  /**
   *  TracedCallback signature for Enb Mac Transmitted Control Messages.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] nodeId
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] pointer to msg to get the msg type
   */
  typedef void (* TxedGnbMacCtrlMsgsTracedCallback)
      (const SfnSf sfn, const uint16_t nodeId, const uint16_t rnti,
       const uint8_t bwpId, Ptr<NrControlMessage>);

protected:
  /**
   * \brief DoDispose method inherited from Object
   */
  void virtual DoDispose () override;
  /**
   * \brief Get the bwp id of this MAC
   * \return the bwp id
   */
  uint16_t GetBwpId () const;

  /**
   * \brief Get the cell id of this MAC
   * \return the cell id
   */
  uint16_t GetCellId () const;

  /**
   * \brief Get a DCI for the DL CTRL symbol
   * \return a DL CTRL allocation
   */
  std::shared_ptr<DciInfoElementTdma> GetDlCtrlDci () const;

  /**
   * \brief Get a DCI for the UL CTRL symbol
   * \return a UL CTRL allocation
   */
  std::shared_ptr<DciInfoElementTdma> GetUlCtrlDci () const;

private:
  void ReceiveRachPreamble (uint32_t raId);
  void DoReceiveRachPreamble (uint32_t raId);
  void ReceiveBsrMessage  (MacCeElement bsr);
  void DoReportMacCeToScheduler (MacCeListElement_s bsr);
  /**
   * \brief Called by CCM to inform us that we are the addressee of a SR.
   * \param rnti RNTI that requested to be scheduled
   */
  void DoReportSrToScheduler (uint16_t rnti);
  void DoReceivePhyPdu (Ptr<Packet> p);
  void DoReceiveControlMessage  (Ptr<NrControlMessage> msg);
  virtual void DoSchedConfigIndication (NrMacSchedSapUser::SchedConfigIndParameters ind);
  // forwarded from LteMacSapProvider
  void DoTransmitPdu (LteMacSapProvider::TransmitPduParameters);
  void DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters);
  void DoUlCqiReport (NrMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi);
  // forwarded from NrMacCchedSapUser
  void DoCschedCellConfigCnf (NrMacCschedSapUser::CschedCellConfigCnfParameters params);
  void DoCschedUeConfigCnf (NrMacCschedSapUser::CschedUeConfigCnfParameters params);
  void DoCschedLcConfigCnf (NrMacCschedSapUser::CschedLcConfigCnfParameters params);
  void DoCschedLcReleaseCnf (NrMacCschedSapUser::CschedLcReleaseCnfParameters params);
  void DoCschedUeReleaseCnf (NrMacCschedSapUser::CschedUeReleaseCnfParameters params);
  void DoCschedUeConfigUpdateInd (NrMacCschedSapUser::CschedUeConfigUpdateIndParameters params);
  void DoCschedCellConfigUpdateInd (NrMacCschedSapUser::CschedCellConfigUpdateIndParameters params);
  // forwarded from LteEnbCmacSapProvider
  void DoConfigureMac (uint16_t ulBandwidth, uint16_t dlBandwidth);
  void DoAddUe (uint16_t rnti);
  void DoRemoveUe (uint16_t rnti);
  void DoAddLc (LteEnbCmacSapProvider::LcInfo lcinfo, LteMacSapUser* msu);
  void DoReconfigureLc (LteEnbCmacSapProvider::LcInfo lcinfo);
  void DoReleaseLc (uint16_t  rnti, uint8_t lcid);
  void UeUpdateConfigurationReq (LteEnbCmacSapProvider::UeConfig params);
  LteEnbCmacSapProvider::RachConfig DoGetRachConfig ();
  LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue DoAllocateNcRaPreamble (uint16_t rnti);

  /**
   * \brief Process the newly received DL HARQ feedback
   * \param params the DL HARQ feedback
   */
  void DoDlHarqFeedback (const DlHarqInfo &params);
  /**
   * \brief Process the newly received UL HARQ feedback
   * \param params the UL HARQ feedback
   */
  void DoUlHarqFeedback (const UlHarqInfo &params);

  /**
   * \brief Send to PHY the RAR messages
   * \param rarList list of messages that come from scheduler
   *
   * Clears m_rapIdRntiMap.
   */
  void SendRar (const std::vector<BuildRarListElement_s> &rarList);

private:

  struct HarqProcessInfoSingleStream
  {
    Ptr<PacketBurst> m_pktBurst;
    // maintain list of LCs contained in this TB
    // used to signal HARQ failure to RLC handlers
    std::vector<uint8_t> m_lcidList;
  };

  struct NrDlHarqProcessInfo
  {
    std::vector<HarqProcessInfoSingleStream> m_infoPerStream;
  };

  typedef std::vector < NrDlHarqProcessInfo> NrDlHarqProcessesBuffer_t;

  LteMacSapProvider* m_macSapProvider;
  LteEnbCmacSapProvider* m_cmacSapProvider;
  LteEnbCmacSapUser* m_cmacSapUser;
  NrPhySapProvider* m_phySapProvider {nullptr};
  NrGnbPhySapUser* m_phySapUser;

  NrMacSchedSapProvider* m_macSchedSapProvider;
  NrMacSchedSapUser* m_macSchedSapUser;
  NrMacCschedSapProvider* m_macCschedSapProvider;
  NrMacCschedSapUser* m_macCschedSapUser;

  // Sap For ComponentCarrierManager 'Uplink case'
  LteCcmMacSapProvider* m_ccmMacSapProvider;   ///< CCM MAC SAP provider
  LteCcmMacSapUser* m_ccmMacSapUser;   ///< CCM MAC SAP user

  int32_t m_numRbPerRbg {-1};   //!< number of resource blocks within the channel bandwidth

  uint8_t m_numHarqProcess {20}; //!< number of HARQ processes

  std::unordered_map<uint32_t, struct NrMacPduInfo> m_macPduMap;

  Callback <void, Ptr<Packet> > m_forwardUpCallback;

  std::vector <DlCqiInfo> m_dlCqiReceived;
  std::vector <NrMacSchedSapProvider::SchedUlCqiInfoReqParameters> m_ulCqiReceived;
  std::vector <MacCeElement> m_ulCeReceived;   // CE received (BSR up to now)


  std::unordered_map<uint8_t, uint32_t> m_receivedRachPreambleCount;

  std::unordered_map <uint16_t, std::unordered_map<uint8_t, LteMacSapUser*> > m_rlcAttached;

  std::vector <DlHarqInfo> m_dlHarqInfoReceived;   // DL HARQ feedback received
  std::vector <UlHarqInfo> m_ulHarqInfoReceived;   // UL HARQ feedback received
  std::unordered_map <uint16_t, NrDlHarqProcessesBuffer_t> m_miDlHarqProcessesPackets;   // Packet under trasmission of the DL HARQ process

  TracedCallback<NrSchedulingCallbackInfo> m_dlScheduling;
  TracedCallback<NrSchedulingCallbackInfo> m_ulScheduling;

  std::list<uint16_t> m_srRntiList; //!< List of RNTI that requested a SR

  std::unordered_map<uint8_t, uint32_t> m_rapIdRntiMap; //!< RAPID RNTI map

  TracedCallback<uint8_t, uint16_t> m_srCallback; //!< Callback invoked when a UE requested a SR

  uint16_t m_bandwidthInRbg {0}; //!< BW in RBG. Set by RRC through ConfigureMac

  SfnSf m_currentSlot;

  /**
   * Trace information regarding ENB MAC Received Control Messages
   * Frame number, Subframe number, slot, VarTtti, nodeId, rnti,
   * bwpId, pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint16_t, uint8_t, Ptr<const NrControlMessage>> m_macRxedCtrlMsgsTrace;

  /**
   * Trace information regarding ENB MAC Transmitted Control Messages
   * Frame number, Subframe number, slot, VarTtti, nodeId, rnti,
   * bwpId, pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint16_t, uint8_t, Ptr<const NrControlMessage>> m_macTxedCtrlMsgsTrace;

  /**
   * Trace DL HARQ info list elements.
   */
  TracedCallback<const DlHarqInfo&> m_dlHarqFeedback;
};

}

#endif /* NR_ENB_MAC_H */

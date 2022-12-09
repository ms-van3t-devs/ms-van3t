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

#ifndef NR_UE_PHY_H
#define NR_UE_PHY_H

#include "nr-phy.h"
#include "nr-amc.h"
#include "nr-harq-phy.h"
#include <ns3/lte-ue-phy-sap.h>
#include <ns3/lte-ue-cphy-sap.h>
#include <ns3/nr-sl-ue-cphy-sap.h>
#include "nr-sl-ue-phy-sap.h"
#include <ns3/traced-callback.h>

#include "nr-sl-sci-f1a-header.h"
#include "nr-sl-mac-pdu-tag.h"

namespace ns3 {

class NrChAccessManager;
class BeamManager;
class BeamId;
class NrUePowerControl;
class NrSlCommResourcePool;

/**
 * \ingroup ue-phy
 * \brief The UE PHY class
 *
 * This class represents the PHY in the User Equipment. Much of the processing
 * and scheduling is done inside the gNb, so the user is a mere "executor"
 * of the decision of the base station.
 *
 * The slot processing is the same as the gnb phy, working as a state machine
 * in which the processing is done at the beginning of the slot.
 *
 * \section ue_phy_conf Configuration
 *
 * The attributes of this class (described in the section Attributes) can be
 * configured through a direct call to `SetAttribute` or, before the PHY creation,
 * with the helper method NrHelper::SetUePhyAttribute().
 *
 * \section ue_phy_attach Attachment to a GNB
 *
 * In theory, much of the configuration should pass through RRC, and through
 * messages that come from the gNb. However, we still are not at this level,
 * and we have to rely on direct calls to configure the same values between
 * the gnb and the ue. At this moment, the call that the helper has to perform
 * are in NrHelper::AttachToEnb().
 *
 * To initialize the class, you must call also SetSpectrumPhy() and StartEventLoop().
 * Usually, this is taken care inside the helper.
 *
 * \see NrPhy::SetSpectrumPhy()
 * \see NrPhy::StartEventLoop()
 */
class NrUePhy : public NrPhy
{
  friend class UeMemberLteUePhySapProvider;
  friend class MemberLteUeCphySapProvider<NrUePhy>;
  friend class MemberNrSlUeCphySapProvider<NrUePhy>;
  //NR SL
  /// allow MemberNrSlUePhySapProvider<NrUePhy> class friend access
  friend class MemberNrSlUePhySapProvider<NrUePhy>;

public:
  /**
   * \brief Get the object TypeId
   * \return the object type id
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrUePhy default constructor.
   */
  NrUePhy ();

  /**
   * \brief ~NrUePhy
   */
  virtual ~NrUePhy () override;

  /**
   * \brief Retrieve the pointer for the C PHY SAP provider (AKA the PHY interface towards the RRC)
   * \return the C PHY SAP pointer
   */
  LteUeCphySapProvider* GetUeCphySapProvider () __attribute__((warn_unused_result));

  /**
   * \brief Install ue C PHY SAP user (AKA the PHY interface towards the RRC)
   * \param s the C PHY SAP user pointer to install
   */
  void SetUeCphySapUser (LteUeCphySapUser* s);

  /**
   * \brief Install the PHY sap user (AKA the UE MAC)
   *
   * \param ptr the PHY SAP user pointer to install
   */
  void SetPhySapUser (NrUePhySapUser* ptr);

  /*
   * \brief Enable or disable uplink power control
   * \param enable parameter that enables or disables power control
   */
  void SetEnableUplinkPowerControl (bool enable);

  /**
   * \brief Set the transmission power for the UE
   *
   * Please note that there is also an attribute ("NrUePhy::TxPower")
   * \param pow power
   */
  void SetTxPower (double pow);

  /**
   * \brief Retrieve the TX power of the UE
   *
   * Please note that there is also an attribute ("NrGnbPhy::TxPower")
   * \return the TX power of the UE
   */
  virtual double GetTxPower () const override;

  /**
   * \brief Returns the latest measured RSRP value
   * Called by NrUePowerControl.
   */
  double GetRsrp () const;

  /**
   * \brief Get LTE uplink power control entity
   *
   * \return ptr pointer to LTE uplink power control entity
   */
  Ptr<NrUePowerControl> GetUplinkPowerControl () const;

  /**
   * \brief Allow configuration of uplink power control algorithm.
   * E.g. necessary in FDD, when measurements are received in 
   * downlink BWP, but they are used in uplink BWP
   * NOTE: This way of configuring is a temporal solution until 
   * BWP manager has this function implemented for UL PC, FFR, 
   * algorithm and simillar algorithms, in which is needed to have 
   * a pair of DL and UL BWPs. In future this function will be called 
   * only by a friend class.
   * \param pc Pointer to NrUePowerControl
   */
  void SetUplinkPowerControl (Ptr<NrUePowerControl> pc);

  /**
   * \brief Register the UE to a certain Enb
   *
   * Install the configuration parameters in the UE.
   *
   * \param bwpId the bwp id to which this PHY is attaching to
   *
   *
   */
  void RegisterToEnb (uint16_t bwpId);

  /**
   * \brief Set the AMC pointer from the GNB
   * \param amc The DL AMC of the GNB. This will be used to create the DL CQI
   * that will be sent to the GNB.
   *
   * This function will be soon deprecated, hopefully with some values that
   * comes from RRC. For the moment, it is called by the helper at the
   * registration time.
   *
   */
  void SetDlAmc (const Ptr<const NrAmc> &amc);

  /**
   * \brief Set the number of UL CTRL symbols
   * \param ulCtrlSyms value
   *
   * This function will be soon deprecated, hopefully with a value that
   * comes from RRC. For the moment, it is called by the helper at the
   * registration time.
   */
  void SetUlCtrlSyms (uint8_t ulCtrlSyms);

  /**
   * \brief Set the number of DL CTRL symbols
   * \param dlCtrlSyms value
   *
   * This function will be soon deprecated, hopefully with a value that
   * comes from RRC. For the moment, it is called by the helper at the
   * registration time.
   */
  void SetDlCtrlSyms (uint8_t dlCtrlSyms);

  /**
   * \brief Function that sets the number of RBs per RBG.
   * This function will be soon deprecated, as soon as all the functions at
   * gNb PHY, MAC and UE PHY that work with DCI bitmask start
   * to work on level of RBs instead of RBGs.
   * This function is configured by helper
   *
   * \param numRbPerRbg Number of RBs per RBG
   */
  void SetNumRbPerRbg (uint32_t numRbPerRbg);

  /**
   * \brief Set the UE pattern.
   *
   * Temporary.
   * \param pattern The UE pattern
   */
  void SetPattern (const std::string &pattern);

  /**
   * \brief Receive a list of CTRL messages
   *
   * Connected by the helper to a callback of the spectrum.
   *
   * \param msg Message
   */
  void PhyCtrlMessagesReceived (const Ptr<NrControlMessage> &msg);

  /**
   * \brief Receive a PHY data packet
   *
   * Connected by the helper to a callback of the spectrum.
   *
   * \param p Received packet
   */
  void PhyDataPacketReceived (const Ptr<Packet> &p);

  /**
   * \brief Generate a DL CQI report
   *
   * Connected by the helper to a callback in corresponding ChunkProcessor
   *
   * \param sinr the SINR
   */
  void GenerateDlCqiReport (const SpectrumValue& sinr);

  /**
   * \brief Get the current RNTI of the user
   *
   * \return the current RNTI of the user
   */
  uint16_t GetRnti () __attribute__((warn_unused_result));

  /**
   * \brief Get the HARQ feedback (on the transmission) from
   * NrSpectrumPhy and send it through ideal PUCCH to gNB
   *
   * Connected by the helper to a spectrum phy callback
   *
   * \param m the HARQ feedback
   */
  void EnqueueDlHarqFeedback (const DlHarqInfo &m);

  /**
   *  TracedCallback signature for Ue Phy Received Control Messages.
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
  typedef void (* RxedUePhyCtrlMsgsTracedCallback)
      (const SfnSf sfnSf, const uint16_t nodeId, const uint16_t rnti,
       const uint8_t bwpId, Ptr<NrControlMessage>);

  /**
   *  TracedCallback signature for Ue Phy Transmitted Control Messages.
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
  typedef void (* TxedUePhyCtrlMsgsTracedCallback)
      (const SfnSf sfnSf, const uint16_t nodeId, const uint16_t rnti,
       const uint8_t bwpId, Ptr<NrControlMessage>);

  /**
   *  TracedCallback signature for Ue Phy DL DCI reception.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] nodeId
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] harq ID
   * \param [in] K1 Delay
   */
  typedef void (* RxedUePhyDlDciTracedCallback)
      (const SfnSf sfnSf, const uint16_t nodeId, const uint16_t rnti,
       const uint8_t bwpId, uint8_t harqId, uint32_t K1Delay);

  /**
   *  TracedCallback signature for Ue Phy DL HARQ Feedback transmission.
   *
   * \param [in] frame Frame number.
   * \param [in] subframe Subframe number.
   * \param [in] slot number.
   * \param [in] VarTti
   * \param [in] nodeId
   * \param [in] rnti
   * \param [in] bwpId
   * \param [in] harq ID
   * \param [in] K1 Delay
   */
  typedef void (* TxedUePhyHarqFeedbackTracedCallback)
      (const SfnSf sfnSf, const uint16_t nodeId, const uint16_t rnti,
       const uint8_t bwpId, uint8_t harqId, uint32_t K1Delay);

  /**
   * \brief Set the channel access manager interface for this instance of the PHY
   * \param cam the pointer to the interface
   */
  void SetCam (const Ptr<NrChAccessManager> &cam);

  const SfnSf & GetCurrentSfnSf () const override;

  // From nr phy. Not used in the UE
  virtual BeamId GetBeamId (uint16_t rnti) const override;

  /**
   * \brief Start the ue Event Loop
   *
   * As parameters, there are the initial values for some variables.
   *
   * \param nodeId the UE nodeId
   * \param frame Frame
   * \param subframe SubF.
   * \param slot Slot
   */
  virtual void ScheduleStartEventLoop (uint32_t nodeId, uint16_t frame, uint8_t subframe, uint16_t slot) override;

  /**
   * \brief Called when rsReceivedPower is
   */
  void ReportRsReceivedPower (const SpectrumValue& power);

  /**
   * \brief TracedCallback signature for power trace source
   *
   * It depends on the TxPower configured attribute, and the number of
   * RB allocated.
   *
   * \param [in] sfnSf Slot number
   * \param [in] v Power Spectral Density
   * \param [in] time Time for which this power is set
   * \param [in] rnti RNTI of the UE
   * \param [in] imsi IMSI of the UE
   * \param [in] bwpId Reference BWP ID
   * \param [in] cellId Reference Cell ID
   */
  typedef void (* PowerSpectralDensityTracedCallback)(const SfnSf &sfnSf, Ptr<const SpectrumValue> v,
                                                      const Time &time, uint16_t rnti,
                                                      uint64_t imsi, uint16_t bwpId, uint16_t cellId);

  //NR SL

  /**
   * \brief pre-configure sidelink bandwidth
   *
   * This method will used in out of coverage
   * scenarios to set the channel bandwidth.
   * In in-coverage scenario the channel bandwidth
   * is configured by RRC after receiving the MIB.
   *
   * \param slBandwidth The total sidelink channel bandwidth
   */
  void PreConfigSlBandwidth (uint16_t slBandwidth);
  /**
   * \brief Register sidelink bandwidthpart id
   *
   * \param bwpId The bandwidthpart id
   */
  void RegisterSlBwpId (uint16_t bwpId);

protected:
  /**
   * \brief DoDispose method inherited from Object
   */
  void virtual DoDispose () override;
  uint32_t GetNumRbPerRbg () const override;

private:

  /**
   * \brief Compute the AvgSinr (copied from LteUePhy)
   * \param sinr the SINR
   * \return the average on all the RB
   */
  static double ComputeAvgSinr (const SpectrumValue& sinr);

  void StartEventLoop (uint16_t frame, uint8_t subframe, uint16_t slot);

  /**
   * \brief Channel access granted, invoked after the LBT
   *
   * \param time Time of the grant
   */
  void ChannelAccessGranted (const Time &time);

  /**
   * \brief Channel access denied
   */
  void ChannelAccessDenied ();

  /**
   * \brief RequestAccess
   */
  void RequestAccess ();

  /**
   * \brief Forward the received RAR to the MAC
   * \param rarMsg RAR message
   */
  void DoReceiveRar (Ptr<NrRarMessage> rarMsg);
  /**
   * \brief Create a DlCqiFeedback message
   * \param sinr the SINR value
   * \return a CTRL message with the CQI feedback
   */
  Ptr<NrDlCqiMessage> CreateDlCqiFeedbackMessage (const SpectrumValue& sinr) __attribute__((warn_unused_result));
  /**
   * \brief Receive DL CTRL and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of DL CTRL will end
   */
  Time DlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));
  /**
   * \brief Transmit UL CTRL and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the transmission of UL CTRL will end
   */
  Time UlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));
  /**
   * \brief Transmit UL SRS and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the transmission of UL SRS will end
   */
  Time UlSrs (const std::shared_ptr<DciInfoElementTdma> &dci);
  /**
   * \brief Receive DL data and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of DL data will end
   */
  Time DlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Transmit UL data and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the transmission of UL data will end
   */
  Time UlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Try to perform an lbt before UL CTRL
   *
   * This function should be called after we receive the DL_DCI for the slot,
   * and then checks if we can re-use the channel through shared MCOT. Otherwise,
   * schedule an LBT before the transmission of the UL CTRL.
   */
  void TryToPerformLbt ();

  /**
   * \brief Start the slot processing
   * \param s the slot number
   */
  void StartSlot (const SfnSf &s);

  /**
   * \brief Start the processing of a variable TTI
   * \param dci the DCI of the variable TTI
   *
   * This time can be a DL CTRL, a DL data, a UL data, or UL CTRL, with
   * any number of symbols (limited to the number of symbols per slot).
   *
   * At the end of processing, schedule the method EndVarTtti that will finish
   * the processing of the variable tti allocation.
   *
   * \see DlCtrl
   * \see UlCtrl
   * \see DlData
   * \see UlData
   */
  void StartVarTti (const std::shared_ptr<DciInfoElementTdma> &dci);

  /**
   * \brief End the processing of a variable tti
   * \param lastDci the DCI of the variable TTI that has just passed
   *
   * The end of the variable tti indicates that the allocation has been
   * transmitted/received. Depending on the variable tti left, the method
   * will schedule another var tti (StartVarTti()) or will wait until the
   * end of the slot (EndSlot()).
   *
   * \see StartVarTti
   * \see EndSlot
   */
  void EndVarTti (const std::shared_ptr<DciInfoElementTdma> &dci);

  /**
   * \brief Set the Tx power spectral density based on the RB index vector
   * \param mask vector of the index of the RB (in SpectrumValue array)
   * in which there is a transmission
   * \param numSym number of symbols of the transmission
   */
  void SetSubChannelsForTransmission (const std::vector<int> &mask, uint32_t numSym);
  /**
   * \brief Send ctrl msgs considering L1L2CtrlLatency
   * \param msg The ctrl msg to be sent
   */
  void DoSendControlMessage (Ptr<NrControlMessage> msg);
  /**
   * \brief Send ctrl msgs without considering L1L2CtrlLatency
   * \param msg The ctrl msg to be sent
   */
  void DoSendControlMessageNow (Ptr<NrControlMessage> msg);

  /**
   * \brief Process a received data Dci
   * \param ulSfnSf The slot to which the Dci refers to
   * \param dciInfoElem the DCI
   */
  void ProcessDataDci (const SfnSf &ulSfnSf, const std::shared_ptr<DciInfoElementTdma> &dciInfoElem);

  /**
   * \brief Process a received SRS Dci
   * \param ulSfnSf The slot to which the Dci refers to
   * \param dciInfoElem the DCI
   */
  void ProcessSrsDci (const SfnSf &ulSfnSf, const std::shared_ptr<DciInfoElementTdma> &dciInfoElem);

  /**
   * \brief Transmit to the spectrum phy the data stored in pb
   *
   * \param pb Data to transmit
   * \param duration period of transmission
   * \param ctrlMsg Control messages
   */
  void SendDataChannels (const Ptr<PacketBurst> &pb,
                         const std::list<Ptr<NrControlMessage> > &ctrlMsg,
                         const Time &duration);
  /**
   * \brief Transmit the control channel
   *
   * \param varTtiPeriod the period of transmission
   *
   * Call the NrSpectrumPhy class, indicating the control message to transmit.
   */
  void SendCtrlChannels (Time prd);

  // SAP methods
  void DoReset ();
  void DoStartCellSearch (uint16_t dlEarfcn);
  void DoSynchronizeWithEnb (uint16_t cellId);
  void DoSynchronizeWithEnb (uint16_t cellId, uint16_t dlEarfcn);
  void DoSetPa (double pa);
  /**
   * \param rsrpFilterCoefficient value. Determines the strength of
   * smoothing effect induced by layer 3 filtering of RSRP
   * used for uplink power control in all attached UE.
   * If equals to 0, no layer 3 filtering is applicable.
   */
  void DoSetRsrpFilterCoefficient (uint8_t rsrpFilterCoefficient);

  /**
   * \brief It is called to set an initial bandwidth
   * that will be used until bandwidth is being configured
   */
  void DoSetInitialBandwidth ();
  /**
   * \brief Function that is called by RRC SAP.
   * TODO This function and its name can be updated once NR RRC SAP is implemented
   */
  void DoSetDlBandwidth (uint16_t ulBandwidth);
  /**
   * \brief Function that is called by RRC SAP.
   * TODO This function and its name can be updated once NR RRC SAP is implemented
   */
  void DoConfigureUplink (uint16_t ulEarfcn, uint8_t ulBandwidth);
  void DoConfigureReferenceSignalPower (int8_t referenceSignalPower);
  void DoSetRnti (uint16_t rnti);
  void DoSetTransmissionMode (uint8_t txMode);
  void DoSetSrsConfigurationIndex (uint16_t srcCi);
  /**
   * \brief Reset Phy after radio link failure function
   *
   * It resets the physical layer parameters of the
   * UE after RLF.
   *
   */
  void DoResetPhyAfterRlf ();
  /**
   * \brief Reset radio link failure parameters
   *
   * Upon receiving N311 in Sync indications from the UE
   * PHY, the UE RRC instructs the UE PHY to reset the
   * RLF parameters so, it can start RLF detection again.
   *
   */
  void DoResetRlfParams ();

  /**
   * \brief Start in Snyc detection function
   *
   * When T310 timer is started, it indicates that physical layer
   * problems are detected at the UE and the recovery process is
   * started by checking if the radio frames are in-sync for N311
   * consecutive times.
   *
   */
  void DoStartInSnycDetection ();

  /**
   * \brief Set IMSI
   *
   * \param imsi the IMSI of the UE
   */
  void DoSetImsi (uint64_t imsi);

  /**
   * \brief Push proper DL CTRL/UL CTRL symbols in the current slot allocation
   * \param currentSfnSf The current sfnSf
   *
   * The symbols are inserted based on the current TDD pattern; if no pattern
   * is known (e.g., we are in the first slot, and the SIB has not reached
   * yet the UE) it is automatically inserted a DL CTRL symbol.
   */
  void PushCtrlAllocations (const SfnSf currentSfnSf);

  /**
   * \brief Inserts the received DCI for the current slot allocation
   * \param dci The DCI description of the current slot
   */
  void InsertAllocation (const std::shared_ptr<DciInfoElementTdma> &dci);

  /**
   * \brief Inserts the received DCI for a future slot allocation
   * \param dci The DCI description of the future slot
   */
  void InsertFutureAllocation (const SfnSf &sfnSf, const std::shared_ptr<DciInfoElementTdma> &dci);

  NrUePhySapUser* m_phySapUser;             //!< SAP pointer
  LteUeCphySapProvider* m_ueCphySapProvider;    //!< SAP pointer
  LteUeCphySapUser* m_ueCphySapUser;            //!< SAP pointer

  bool m_enableUplinkPowerControl {false}; //!< Flag that indicates whether power control is enabled
  Ptr<NrUePowerControl> m_powerControl; //!< UE power control entity

  Ptr<const NrAmc> m_amc;  //!< AMC model used to compute the CQI feedback

  Time m_wbCqiLast;
  Time m_lastSlotStart; //!< Time of the last slot start

  bool m_ulConfigured {false};     //!< Flag to indicate if RRC configured the UL
  bool m_receptionEnabled {false}; //!< Flag to indicate if we are currently receiveing data
  uint16_t m_rnti {0};             //!< Current RNTI of the user
  uint32_t m_currTbs {0};          //!< Current TBS of the receiveing DL data (used to compute the feedback)
  uint64_t m_imsi {0}; ///< The IMSI of the UE
  std::unordered_map<uint8_t, uint32_t> m_harqIdToK1Map;  //!< Map that holds the K1 delay for each Harq process id

  int64_t m_numRbPerRbg {-1};   //!< number of resource blocks within the channel bandwidth, this parameter is configured by MAC through phy SAP provider interface

  SfnSf m_currentSlot;

  /**
   * \brief Status of the channel for the PHY
   */
  enum ChannelStatus
  {
    NONE,        //!< The PHY doesn't know the channel status
    REQUESTED,   //!< The PHY requested channel access
    GRANTED      //!< The PHY has the channel, it can transmit
  };

  ChannelStatus m_channelStatus {NONE}; //!< The channel status
  Ptr<NrChAccessManager> m_cam; //!< Channel Access Manager
  Time m_lbtThresholdForCtrl; //!< Threshold for LBT before the UL CTRL
  bool m_tryToPerformLbt {false}; //!< Boolean value set in DlCtrl() method
  EventId m_lbtEvent;
  uint8_t m_dlCtrlSyms {1}; //!< Number of CTRL symbols in DL
  uint8_t m_ulCtrlSyms {1}; //!< Number of CTRL symbols in UL

  double m_rsrp {0}; //!< The latest measured RSRP value

  /**
   * The `ReportCurrentCellRsrpSinr` trace source. Trace information regarding
   * RSRP and average SINR (see TS 36.214). Exporting cell ID, RNTI, RSRP, and
   * SINR. Moreover it reports the m_componentCarrierId.
   */
  TracedCallback<uint16_t, uint16_t, double, double, uint16_t> m_reportCurrentCellRsrpSinrTrace;
  TracedCallback<uint64_t, uint64_t> m_reportUlTbSize; //!< Report the UL TBS
  TracedCallback<uint64_t, uint64_t> m_reportDlTbSize; //!< Report the DL TBS
  TracedCallback<const SfnSf &, Ptr<const SpectrumValue>, const Time &, uint16_t, uint64_t, uint16_t, uint16_t> m_reportPowerSpectralDensity; //!< Report the Tx power

  /**
   * Trace information regarding Ue PHY Received Control Messages
   * Frame number, Subframe number, slot, VarTtti, nodeId, rnti, bwpId,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint16_t, uint8_t, Ptr<const NrControlMessage>> m_phyRxedCtrlMsgsTrace;

  /**
   * Trace information regarding Ue PHY Transmitted Control Messages
   * Frame number, Subframe number, slot, VarTtti, nodeId, rnti, bwpId,
   * pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint16_t, uint8_t, Ptr<const NrControlMessage>> m_phyTxedCtrlMsgsTrace;

  /**
   * Trace information regarding Ue PHY Rxed DL DCI Messages
   * Frame number, Subframe number, slot, VarTtti, nodeId,
   * rnti, bwpId, Harq ID, K1 delay
   */
  TracedCallback<SfnSf, uint16_t, uint16_t, uint8_t, uint8_t, uint32_t> m_phyUeRxedDlDciTrace;

  /**
   * Trace information regarding Ue PHY Txed Harq Feedback
   * Frame number, Subframe number, slot, VarTtti, nodeId,
   * rnti, bwpId, Harq ID, K1 delay
   */
  TracedCallback<SfnSf, uint16_t, uint16_t, uint8_t, uint8_t, uint32_t> m_phyUeTxedHarqFeedbackTrace;

  //NR SL
public:
  /**
   * \brief Get the NR Sidelik UE Control PHY SAP offered by PHY to RRC
   *
   * \return the NR Sidelik UE Control PHY SAP provider interface offered by
   *         PHY to RRC.
   */
  NrSlUeCphySapProvider* GetNrSlUeCphySapProvider ();

  /**
   * \brief Set the NR Sidelik UE Control MAC SAP offered by RRC to PHY
   *
   * \param s the NR Sidelik UE Control MAC SAP user interface offered by
   *          RRC to PHY.
   */
  void SetNrSlUeCphySapUser (NrSlUeCphySapUser* s);

  /**
   * \brief Set the NR Sidelik UE PHY SAP offered by UE MAC to UE PHY
   *
   * \param s the NR Sidelik UE PHY SAP user interface offered to the
   *          UE PHY by UE MAC
   */
  void SetNrSlUePhySapUser (NrSlUePhySapUser* s);
  /**
   * \brief Receive new PSCCH PHY pdu from SpectrumPhy
   * \param p The packet received
   */
  void PhyPscchPduReceived (const Ptr<Packet> &p, const SpectrumValue &psd);
  /**
   * \brief Receive new successfully decoded PSSCH PHY pdu from SpectrumPhy
   * \param pb The packet burst received
   */
  void PhyPsschPduReceived (const Ptr<PacketBurst> &pb);

protected:
  /**
   * \brief Add NR Sidelink communication transmission pool
   *
   * Adds transmission pool for NR Sidelink communication
   *
   * \param txPool The pointer to the NrSlCommResourcePool
   */
  void DoAddNrSlCommTxPool (Ptr<const NrSlCommResourcePool> txPool);
  /**
   * \brief Add NR Sidelink communication reception pool
   *
   * Adds reception pool for NR Sidelink communication
   *
   * \param rxPool The pointer to the NrSlCommResourcePool
   */
  void DoAddNrSlCommRxPool (Ptr<const NrSlCommResourcePool> rxPool);

private:
  /**
   * \brief Sidelink RX grant information about the expected NR SL transport
   *        block at a certain point in the slot
   *
   * This information will be passed by the NrUePhy to NrSpectrumhy through a
   * call to AddSlExpectedTb
   */
 struct SlRxGrantInfo
 {
   /**
    * \brief constructor
    * \param rnti Tx RNTI
    * \param dstId Destination id
    * \param tbSize TB Size
    * \param mcs MCS
    * \param rbMap RB map
    * \param symStart Starting symbol index
    * \param numSym Total number of symbols
    * \param sfn SfnSf
    */
   SlRxGrantInfo (uint16_t rnti, uint32_t dstId, uint32_t tbSize, uint8_t mcs,
                  const std::vector<int> &rbMap, uint8_t symStart,
                  uint8_t numSym, const SfnSf &sfn) :
     rnti {rnti},
     dstId {dstId},
     tbSize (tbSize),
     mcs (mcs),
     rbBitmap (rbMap),
     symStart (symStart),
     numSym (numSym),
     sfn (sfn) { }
   SlRxGrantInfo () = delete;
   SlRxGrantInfo (const SlRxGrantInfo &o) = default;

   uint16_t rnti             {0}; //!< Tx RNTI
   uint32_t dstId            {0}; //!< Destination id
   uint32_t tbSize           {0}; //!< TBSize
   uint8_t mcs               {0}; //!< MCS
   std::vector<int> rbBitmap;     //!< RB Bitmap
   uint8_t symStart          {0}; //!< Sym start
   uint8_t numSym            {0}; //!< Num sym
   SfnSf sfn;                     //!< SFN
 };
  /**
   * \brief Start the NR SL slot processing
   * \param s the slot number
   */
  void StartNrSlSlot (const SfnSf &s);
  /**
   * \brief Start the processing of a NR Sidelink variable TTI
   * \param varTtiInfo the slot VarTti allocation info of the variable TTI
   *
   * This time can be a SL CTRL, a SL data, SL HARQ feedback (not available yet), with
   * any number of symbols (limited to the number of symbols per slot).
   *
   * At the end of processing, it schedules the method EndNrSlVarTtti that will finish
   * the processing of the variable TTI allocation.
   *
   * \see EndNrSlVarTti
   */
  void StartNrSlVarTti (const NrSlVarTtiAllocInfo &varTtiInfo);
  /**
   * \brief End the processing of a NR Sidelink variable TTI
   * \param varTtiInfo the slot VarTti allocation info of the variable TTI
   *
   * The end of the NR SL variable TTI indicates that the allocation has been
   * transmitted. Depending on the variable TTI left with the slot, this method
   * will schedule another NR SL var TTI (StartNrSlVarTti()) or will start
   * new slot.
   *
   * \see StartNrSlVarTti
   * \see StartNrSlSlot
   */
  void EndNrSlVarTti (const NrSlVarTtiAllocInfo &varTtiInfo);
  /**
   * \brief Transmit NR SL CTRL and return the time at which the transmission will end
   * \param varTtiInfo the current slot VarTti allocation info to TX NR SL CTRL
   * \return the time at which the transmission of NR SL CTRL will end
   */
  Time SlCtrl (const NrSlVarTtiAllocInfo &varTtiInfo) __attribute__((warn_unused_result));
  /**
   * \brief Transmit to the spectrum phy the NR SL CTRL packet burst
   *
   * \param pb Packet burst to transmit
   * \param varTtiPeriod period of transmission
   * \param varTtiInfo the slot VarTti allocation info of the variable TTI
   */
  void SendNrSlCtrlChannels (const Ptr<PacketBurst> &pb, const Time &varTtiPeriod, const NrSlVarTtiAllocInfo &varTtiInfo);
  /**
   * \brief Transmit to the spectrum phy the NR SL CTRL packet burst
   *
   * \param pb Packet burst to transmit
   * \param varTtiPeriod period of transmission
   * \param varTtiInfo the slot VarTti allocation info of the variable TTI
   */
  void SendNrSlDataChannels (const Ptr<PacketBurst> &pb, const Time &varTtiPeriod, const NrSlVarTtiAllocInfo &varTtiInfo);
  /**
   * \brief Transmit NR SL DATA and return the time at which the transmission will end
   * \param varTtiInfo the current slot VarTti allocation info to TX NR SL DATA
   * \return the time at which the transmission of NR SL DATA will end
   */
  Time SlData (const NrSlVarTtiAllocInfo &varTtiInfo) __attribute__((warn_unused_result));
  /**
   * \brief Get the Sidelink RSRP value in dBm
   *
   * At the moment, SL RSRP is computed using the PSD of the signal in PSCCH
   * for which we have successfully decoded the SCI-1A.
   *
   * \param psd the power spectral density per each RB
   * \return Sidelink RSRP in dBm
   */
  double GetSidelinkRsrp (SpectrumValue psd);
  /**
   * \brief Save the future Sidelink RX grants indicated by SCI 1-A
   * \param sciF1a SCI 1-A header
   * \param tag NrSlMacPduTag
   * \param sbChSize The sub-channel size in RBs
   */
  void SaveFutureSlRxGrants (const NrSlSciF1aHeader& sciF1a, const NrSlMacPduTag& tag, const uint16_t sbChSize);
  /**
   * \brief Send Sidelink expected TB info to NrSpectrumPhy
   * \param s The SfnSf
   *
   * This method will go over the \link m_slRxGrants \endlink list, which stores
   * the info about the possible expected TBs to be received in the current
   * slot without SCI 1-A, and send this info to NrSpectrumPhy.
   */
  void SendSlExpectedTbInfo (const SfnSf &s);
  NrSlUeCphySapProvider* m_nrSlUeCphySapProvider; //!< Control SAP interface to receive calls from the UE RRC instance
  NrSlUeCphySapUser* m_nrSlUeCphySapUser {nullptr}; //!< Control SAP interface to call the methods of UE RRC instance
  NrSlUePhySapUser* m_nrSlUePhySapUser {nullptr}; //!< SAP interface to call the methods of UE MAC instance
  Ptr<const NrSlCommResourcePool> m_slTxPool; //!< Sidelink communication transmission pools
  Ptr<const NrSlCommResourcePool> m_slRxPool; //!< Sidelink communication reception pools
  std::deque <SlRxGrantInfo> m_slRxGrants; //!< Sidelink RX grants indicated by SCI 1-A
};

}

#endif /* NR_UE_PHY_H */

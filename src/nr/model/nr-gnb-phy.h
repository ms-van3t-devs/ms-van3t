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
 */
#ifndef NR_ENB_PHY_H
#define NR_ENB_PHY_H

#include "nr-phy.h"
#include "nr-control-messages.h"
#include <ns3/lte-enb-phy-sap.h>
#include <ns3/lte-enb-cphy-sap.h>
#include <ns3/nr-harq-phy.h>
#include <functional>
#include "ns3/ideal-beamforming-algorithm.h"
#include "beam-conf-id.h"

namespace ns3 {

class NetDevice;
class NrNetDevice;
class PacketBurst;
class NrUePhy;
class NrGnbMac;
class NrChAccessManager;
class BeamManager;

/**
 *
 * \ingroup gnb-phy
 *
 * \brief The gNb physical layer
 *
 * This class represent the gNb physical layer, and is the main class for
 * what regards the gNb. It has different features, that spans from the
 * spectrum transmission (delegated to the NrSpectrumPhy class), the
 * slot timings and settings (including numerology, pattern...), and the
 * channel access in unlicensed spectrum.
 *
 * \section gnb_phy_spectrum Spectrum transmission
 *
 * The PHY has a pointer to NrSpectrumPhy class, which is the entry point
 * in the spectrum transmission domain, and the connection point with the channel.
 * The PHY has an attribute, TxPower, that is responsible of the transmission
 * power of the node. The selection of the RB where that power is set is done
 * by the MAC during the scheduling phase, generating a DCI that will be used
 * as a reference.
 *
 * The transmission of control message is fictitious; the scheduler will reserve
 * symbols in which the transmission will be done, but the list of the messages
 * will be passed to the UE without any interference or error model applied.
 *
 * \section gnb_phy_timings Slot Timings, and event processing
 *
 * The node works as a state-machine where all the processing is done at the
 * beginning of the slot boundary. The slot duration is dictated by the
 * selected numerology and the number of symbols per slot. At the beginning
 * of the slot (StartSlot()) the PHY will check the channel status, and (if
 * applicable) will call the MAC for its processing, through the SAP interface.
 * The MAC will pass the allocations that it has generated, and the PHY will
 * transmit on the air the allocations that are planned for this slot.
 *
 * We model the processing times of the layer through a parameter called
 * L1L2CtrlLatency. This latency, set to 2, indicates that a DL allocation done by
 * MAC will require two slot before going in the air. For this reason, MAC always
 * work "in the future".
 *
 * \section gnb_phy_unlicensed Unlicensed access
 *
 * The PHY is prepared to postpone its processing if the channel access manager
 * (an interface specified in the NrChAccessManager class) indicates that
 * the channel is busy, hence not available for the transmission.
 *
 * \section gnb_phy_conf Configuration
 *
 * The initialization of the class in done through NrHelper; the user
 * can interact with it through the various attribute that are present, trying
 * to avoid using direct function calls. The attributes can be changed by
 * a direct call to `SetAttribute` on the pointer of the PHY, or (before the PHY
 * creation) through the helper method NrHelper::SetGnbPhyAttribute().
 *
 *
 * \see NrPhy::SetSpectrumPhy
 * \see NrPhy::StartEventLoop
 * \see NrPhy::StartSlot
 */
class NrGnbPhy : public NrPhy
{
  friend class MemberLteEnbCphySapProvider<NrGnbPhy>;
  friend class NrMemberPhySapProvider;
public:
  /**
   * \brief Get Type id
   * \return the type id of the NrGnbPhy
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrGnbPhy constructor. Please use the other one.
   */
  NrGnbPhy ();

  /**
   * \brief ~NrGnbPhy
   */
  virtual ~NrGnbPhy () override;

  /**
   * \brief Set the C PHY SAP user
   * \param s the C PHY SAP user
   */
  void SetEnbCphySapUser (LteEnbCphySapUser* s);
  /**
   * \brief Get the C PHY SAP provider
   * \return the C PHY SAP provider pointer
   */
  LteEnbCphySapProvider* GetEnbCphySapProvider ();

  /**
    * \brief: Set the minimum processing delay (in slots)
    * to decode DL DCI and decode DL data. It is not defined in NR specs.
    * It defaults to 0 slots.
    */
   void SetN0Delay (uint32_t delay);

   /**
    * \brief: Set the minimum processing delay (in slots)
    * to decode DL Data and send Harq feedback.
    *
    * It is defined in TS 38.214 Table 5.3-1 and Table 5.3-2 for UE
    * capabilities 1 and UE capability 2, respectively, and depends on the
    * numerology. In the specs it is defined in multiples of OFDM symbols, but
    * we define it in multiples of slots, since then it is used to compute
    * flexible K1 timing that is measured in slots. For UE Capability 1,
    * it can take 1 or 2 slots. For UE Capability 2, it is not
    * larger than 1 slot.
    * \param delay the N1 delay
    */
   void SetN1Delay (uint32_t delay);

   /**
    * \brief: Set the minimum processing delay (in slots)
    * to decode UL DCI and prepare UL data.
    *
    * It is defined in TS 38.214 Table 6.4-1 and Table 6.4-2 for UE
    * capabilities 1 and UE capability 2, respectively, and depends on the
    * numerology. In the specs it is defined in multiples of OFDM symbols, but
    * we define it in multiples of slots, since then it is used to compute
    * flexible K2 timing that is measured in slots. For UE Capability 1,
    * it can take 1, 2 or 3 slots. For UE Capability 2, it is not
    * larger than 1 slot.
    * \param delay the N2 delay
    */
   void SetN2Delay (uint32_t delay);

  /**
   * \brief: Get the minimum processing delay (in slots)
   * to decode DL DCI and decode DL Data
   */
  uint32_t GetN0Delay (void) const;

  /**
   * \brief: Get the minimum processing delay (in slots)
   * to decode DL Data and send Harq feedback
   */
  uint32_t GetN1Delay (void) const;

  /**
   * \brief: Get the minimum processing delay (in slots)
   * to decode UL DCI and prepare UL data
   */
  uint32_t GetN2Delay (void) const;

  /**
   * \brief Get the BeamConfId for the selected user
   * \param rnti the selected UE
   * \return the BeamConfId of the UE
   */
  BeamConfId GetBeamConfId (uint16_t rnti) const override;

  /**
   * \brief Set the channel access manager interface for this instance of the PHY
   * \param s the pointer to the interface
   */
  void SetCam (const Ptr<NrChAccessManager> &s);

  /**
   * \brief Get the channel access manager for the PHY
   * \return the CAM of the PHY
   */
  Ptr<NrChAccessManager> GetCam () const;

  /**
   * \brief Set the transmission power for the UE
   *
   * Please note that there is also an attribute ("NrUePhy::TxPower")
   * \param pow power
   */
  void SetTxPower (double pow);

  /**
   * \brief Retrieve the TX power of the gNB
   *
   * Please note that there is also an attribute ("NrUePhy::TxPower")
   * \return the TX power of the gNB
   */
  virtual double GetTxPower () const override;

  /**
   * \brief Set the Tx power spectral density based on the RB index vector
   * \param rbIndexVector vector of the index of the RB (in SpectrumValue array)
   * in which there is a transmission
   * \param activeStreams the number of active streams
   */
  void SetSubChannels (const std::vector<int> &rbIndexVector, uint8_t activeStreams);

  /**
   * \brief Add the UE to the list of this gnb UEs.
   *
   * Usually called by the helper when a UE register to this gnb.
   * \param imsi IMSI of the device
   * \param ueDevice Device
   * \return
   */
  bool RegisterUe (uint64_t imsi, const Ptr<NrUeNetDevice> &ueDevice);

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
   * \param streamId the index of the stream
   */
  void GenerateDataCqiReport (const SpectrumValue& sinr, uint8_t streamId) const;

  /**
   * \brief Receive a list of CTRL messages
   *
   * Connected by the helper to a callback of the spectrum.
   *
   * \param msg the message
   */
  void PhyCtrlMessagesReceived (const Ptr<NrControlMessage> &msg);

  /**
   * \brief Get the power of the enb
   * \return the power
   */
  int8_t DoGetReferenceSignalPower () const;

  /**
   * \brief Install the PHY SAP user (which is in this case the MAC)
   *
   * \param ptr the PHY SAP user pointer to install
   */
  void SetPhySapUser (NrGnbPhySapUser* ptr);

  /**
   * \brief Get the HARQ feedback from NrSpectrumPhy
   * and forward it to the scheduler
   *
   * Connected by the helper to a spectrum phy callback
   *
   * \param mes the HARQ feedback
   */
  void ReportUlHarqFeedback (const UlHarqInfo &mes);

  /**
   * \brief Set the pattern that the gnb will utilize.
   *
   * \param pattern A string containing slot types separated by the character '|'.
   *
   * For example, a valid pattern would be "DL|DL|UL|UL|DL|DL|UL|UL|". The slot
   * types allowed are:
   *
   * - "DL" for downlink only
   * - "UL" for uplink only
   * - "F" for flexible (dl and ul)
   * - "S" for special slot (LTE-compatibility)
   */
  void SetPattern (const std::string &pattern);

  /**
   * \brief Retrieve the currently installed pattern
   * \return the installed pattern
   */
  std::string GetPattern() const;

  /**
   * \brief Set this PHY as primary
   *
   * A primary PHY will send MIB and SIB1. By default, a PHY is "non-primary".
   */
  void SetPrimary ();

  /**
   * \brief Start the ue Event Loop
   * \param nodeId the UE nodeId
   * \param frame Frame
   * \param subframe SubF.
   * \param slot Slot
   */
  virtual void ScheduleStartEventLoop (uint32_t nodeId, uint16_t frame, uint8_t subframe, uint16_t slot) override;

  /**
   *  TracedCallback signature for Received Control Messages.
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
  typedef void (* RxedGnbPhyCtrlMsgsTracedCallback)
      (const SfnSf sfn, const uint16_t nodeId, const uint16_t rnti,
       const uint8_t bwpId, Ptr<NrControlMessage>);

  /**
   *  TracedCallback signature for Transmitted Control Messages.
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
  typedef void (* TxedGnbPhyCtrlMsgsTracedCallback)
      (const SfnSf sfn, const uint16_t nodeId, const uint16_t rnti,
       const uint8_t bwpId, Ptr<NrControlMessage>);

  /**
   * \brief TracedCallback signature for slot statistics
   *
   * \param [in] sfnSf Slot number
   * \param [in] scheduledUe The number of scheduled UE in the slot
   * \param [in] usedReg Used Resource Element Group (1 sym x 1 RB)
   * \param [in] usedSym Used symbols
   * \param [in] availableRb Available RBs
   * \param [in] availableSym Available symbols
   * \param [in] bwpId BWP ID
   * \param [in] cellId Cell ID
   */
  typedef void (* SlotStatsTracedCallback)(const SfnSf &sfnSf, uint32_t scheduledUe,
                                           uint32_t usedReg, uint32_t usedSym,
                                           uint32_t availableRb,
                                           uint32_t availableSym, uint16_t bwpId,
                                           uint16_t cellId);

  /**
   * \brief TracedCallback signature for RB statistics
   *
   * \param [in] sfnSf Slot number
   * \param [in] sym Symbol
   * \param [in] rbMap RB Map, in the spectrum format (vector of indexes of the active RB)
   * \param [in] bwpId BWP ID
   * \param [in] cellId Cell ID
   */
  typedef void (* RBStatsTracedCallback)(const SfnSf &sfnSf, uint8_t sym,
                                         const std::vector<int> &rbMap,
                                         uint16_t bwpId, uint16_t cellId);

  /**
   * \brief Retrieve the number of RB per RBG
   * \return the number of RB per RBG
   *
   * The method will ask the MAC for the value. Don't store it as it may change.
   */
  uint32_t GetNumRbPerRbg () const override;

  const SfnSf & GetCurrentSfnSf () const override;

  /**
   * TODO change to private and add documentation
   */
  void ChangeBeamformingVector (Ptr<NetDevice> dev);
  /**
   * TODO change to private and add documentation
   */
  void ChangeToQuasiOmniBeamformingVector ();

protected:
  /**
   * \brief DoDispose method inherited from Object
   */
  void virtual DoDispose () override;

private:
  /**
   * \brief Set the current slot pattern (better to call it only once..)
   * \param pattern the pattern
   *
   * It does not support dynamic change of pattern during the simulation
   */
  void SetTddPattern (const std::vector<LteNrTddSlotType> &pattern);

  /**
   * \brief Start the slot processing.
   * \param startSlot slot number
   *
   * The method will look at the channel status, and if applicable, will start
   * the MAC processing. If the channel is available, it will start the real
   * slot processing in the method DoStartSlot().
   *
   * \see DoStartSlot
   */
  void StartSlot (const SfnSf &startSlot);

  /**
   * \brief End the slot processing
   *
   * It will close the slot processing, calling then StartSlot for the next
   * slot that is coming.
   *
   * \see StartSlot
   */
  void EndSlot (void);

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
  void EndVarTti (const std::shared_ptr<DciInfoElementTdma> &lastDci);

  /**
   * \brief Transmit to the spectrum phy the data stored in pb
   *
   * \param pb Data to transmit
   * \param varTtiPeriod period of transmission
   * \param dci DCI of the transmission
   * \param streamId The id of the stream, which identifies the instance of the
   *        NrSpecturmPhy to be used to transmit the packet burst
   */
  void SendDataChannels (const Ptr<PacketBurst> &pb, const Time &varTtiPeriod,
                         const std::shared_ptr<DciInfoElementTdma> &dci,
                         const uint8_t &streamId);

  /**
   * \brief Transmit the control channel
   *
   * \param varTtiPeriod the period of transmission
   *
   * Call the NrSpectrumPhy class, indicating the control message to transmit.
   */
  void SendCtrlChannels (const Time &varTtiPeriod);

  /**
   * \brief Create a list of messages that contains the DCI to send in the slot specified
   * \param sfn Slot from which take all the DCI
   * \return a list of DCI
   */
  std::list <Ptr<NrControlMessage>> RetrieveMsgsFromDCIs (const SfnSf &sfn) __attribute__((warn_unused_result));

  /**
   * \brief Channel access granted, invoked after the LBT
   *
   * \param time Time of the grant
   */
  void ChannelAccessGranted (const Time &time);

  /**
   * \brief Channel access lost, the grant has expired or the LBT denied the access
   */
  void ChannelAccessLost ();

  /**
   * \brief Transmit DL CTRL and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the transmission of DL CTRL will end
   *
   * The method will get the messages to transmit, and call SendCtrlChannels.
   *
   * \see SendCtrlChannels
   */
  Time DlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));
  /**
   * \brief Receive UL CTRL and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of UL CTRL will end
   *
   * The method will put the PHY in the listening mode, to get any control message
   * sent by the UEs.
   */
  Time UlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Transmit DL data and return the time at which the transmission will end
   * \param varTtiInfo the current varTti
   * \return the time at which the transmission of DL data will end
   *
   * The method will get the data to transmit, and call SendDataChannels.
   *
   * \see SendDataChannels
   */
  Time DlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Receive UL data and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of UL data will end
   *
   * The method will put the PHY in listening mode, to get any data sent by
   * the UEs.
   */
  Time UlData (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Receive UL SRS and return the time at which the transmission will end
   * \param dci the current DCI
   * \return the time at which the reception of UL data will end
   *
   * The method will put the PHY in listening mode, to get any data sent by
   * the UEs. This data would be a CTRL message (e.g., a SRS)
   */
  Time UlSrs (const std::shared_ptr<DciInfoElementTdma> &dci) __attribute__((warn_unused_result));

  /**
   * \brief Queue a MIB message, to be sent (hopefully) in this slot
   *
   * Only "primary" PHY will send the message.
   */
  void QueueMib ();
  /**
   * \brief Queue a SIB message, to be sent (hopefully) in this slot
   *
   * Only "primary" PHY will send the message.
   */
  void QueueSib ();

  /**
   * \brief Effectively start the slot, as we have the channel.
   *
   * For each variable TTI, schedule a call to StartVarTti.
   *
   * \see StartVarTti.
   */
  void DoStartSlot ();

  void GenerateAllocationStatistics (const SlotAllocInfo &allocInfo) const;

  // LteEnbCphySapProvider forwarded methods
  void DoSetBandwidth (uint16_t ulBandwidth, uint16_t dlBandwidth);
  void DoSetEarfcn (uint16_t dlEarfcn, uint16_t ulEarfcn);
  void DoAddUe (uint16_t rnti);
  void DoRemoveUe (uint16_t rnti);
  void DoSetPa (uint16_t rnti, double pa);
  void DoSetTransmissionMode (uint16_t  rnti, uint8_t txMode);
  void DoSetSrsConfigurationIndex (uint16_t  rnti, uint16_t srcCi);
  void DoSetMasterInformationBlock (LteRrcSap::MasterInformationBlock mib);
  void DoSetSystemInformationBlockType1 (LteRrcSap::SystemInformationBlockType1 sib1);
  void DoSetEarfcn (uint16_t Earfcn );

  /**
   * \brief Store the RBG allocation in the symStart, rbg map.
   * \param map the MAP
   * \param dci DCI
   *
   */
  void StoreRBGAllocation (std::unordered_map<uint8_t, std::vector<uint8_t> > *map,
                           const std::shared_ptr<DciInfoElementTdma> &dci) const;

  /**
   * \brief Generate the generate/send DCI structures from a pattern
   * \param pattern The pattern to analyze
   * \param toSendDl The structure toSendDl to fill
   * \param toSendUl The structure toSendUl to fill
   * \param generateDl The structure generateDl to fill
   * \param generateUl The structure generateUl to fill
   * \param dlHarqfbPosition The structure dlHarqfbPosition to fill
   * \param n0 N0 parameter
   * \param n2 N2 parameter
   * \param n1 N1 parameter
   * \param l1l2CtrlLatency L1L2CtrlLatency of the system
   */
  static void GenerateStructuresFromPattern (const std::vector<LteNrTddSlotType> &pattern,
                                             std::map<uint32_t, std::vector<uint32_t> > *toSendDl,
                                             std::map<uint32_t, std::vector<uint32_t> > *toSendUl,
                                             std::map<uint32_t, std::vector<uint32_t> > *generateDl,
                                             std::map<uint32_t, std::vector<uint32_t> > *generateUl,
                                             std::map<uint32_t, uint32_t> *dlHarqfbPosition,
                                             uint32_t n0, uint32_t n2, uint32_t n1, uint32_t l1l2CtrlLatency);

  /**
   * \brief Call MAC for retrieve the slot indication. Currently calls UL and DL.
   * \param currentSlot Current slot
   */
  void CallMacForSlotIndication (const SfnSf &currentSlot);

  /**
   * \brief Retrieve a DCI list for the allocation passed as parameter
   * \param alloc The allocation we are searching in
   * \param format The format of the DCI (UL or DL)
   * \param kDelay The K0 or K2 delay
   * \return A list of control messages that can be sent
   *
   * PS: This function ignores CTRL allocations.
   */
  std::list <Ptr<NrControlMessage>>
  RetrieveDciFromAllocation (const SlotAllocInfo &alloc,
                             const DciInfoElementTdma::DciFormat &format,
                             uint32_t kDelay, uint32_t k1Delay);

  /**
   * \brief Insert a fake DL allocation in the allocation list
   * \param sfnSf The sfnSf to which we need a fake allocation
   *
   * Usually called at the beginning of the simulation to fill
   * the slot allocation queue until the generation take place
   */
  void PushDlAllocation (const SfnSf &sfnSf) const;

  /**
   * \brief Insert a fake UL allocation in the allocation list
   * \param sfnSf The sfnSf to which we need a fake allocation
   *
   * Usually called at the beginning of the simulation to fill
   * the slot allocation queue until the generation take place
   */
  void PushUlAllocation (const SfnSf &sfnSf) const;

  /**
   * \brief Start the processing event loop
   * \param frame Frame number
   * \param subframe Subframe number
   * \param slot Slot number
   */
  void StartEventLoop (uint16_t frame, uint8_t subframe, uint16_t slot);

  /**
   * \brief See if the channel should be released at the end of the slot
   *
   * If the channel has to be released, then m_channelStatus will be
   * TO_LOSE.
   */
  void DoCheckOrReleaseChannel ();

  /**
   * \brief Check the control messages, and route them to the NetDevice
   *
   * For FDD, we route the CTRL messages to the netdevice (maybe we are in a
   * UL bwp, and our ctrl messages have to be sent from the DL bwp).
   */
  void RetrievePrepareEncodeCtrlMsgs ();

  /**
   * \brief Prepare the RBG power distribution map for the allocations.
   *
   * \param allocations scheduler allocation for this slot.
   */
  void PrepareRbgAllocationMap (const std::deque<VarTtiAllocInfo> &allocations);

  /**
   * \brief Prepare and schedule all the events needed for the current slot.
   */
  void FillTheEvent ();

private:
  NrGnbPhySapUser* m_phySapUser {nullptr};           //!< MAC SAP user pointer, MAC is user of services of PHY, implements e.g. ReceiveRachPreamble
  LteEnbCphySapProvider* m_enbCphySapProvider {nullptr}; //!< PHY SAP provider pointer, PHY provides control services to RRC, RRC can call e.g SetBandwidth
  LteEnbCphySapUser* m_enbCphySapUser {nullptr};         //!< PHY CSAP user pointer, RRC can receive control information by PHY, currently configured but not used

  std::set <uint64_t> m_ueAttached; //!< Set of attached UE (by IMSI)
  std::set <uint16_t> m_ueAttachedRnti; //!< Set of attached UE (by RNTI)
  std::vector< Ptr<NrUeNetDevice> > m_deviceMap; //!< Vector of UE devices

  LteRrcSap::SystemInformationBlockType1 m_sib1; //!< SIB1 message
  Time m_lastSlotStart; //!< Time at which the last slot started
  uint8_t m_currSymStart {0}; //!< Symbol at which the current allocation started
  std::unordered_map<uint8_t, std::vector<uint8_t> > m_rbgAllocationPerSym;  //!< RBG allocation in each sym
  std::unordered_map<uint8_t, std::vector<uint8_t> > m_rbgAllocationPerSymDataStat;  //!< RBG allocation in each sym, for statistics (UL and DL included, only data)

  TracedCallback< uint64_t, SpectrumValue&, SpectrumValue& > m_ulSinrTrace; //!< SINR trace

  /**
   * Trace information regarding Received Control Messages
   * Frame number, Subframe number, slot, VarTtti, nodeId, rnti,
   * bwpId, pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint16_t, uint8_t, Ptr<const NrControlMessage>> m_phyRxedCtrlMsgsTrace;

  /**
   * Trace information regarding Transmitted Control Messages
   * Frame number, Subframe number, slot, VarTtti, nodeId, rnti,
   * bwpId, pointer to message in order to get the msg type
   */
  TracedCallback<SfnSf, uint16_t, uint16_t, uint8_t, Ptr<const NrControlMessage>> m_phyTxedCtrlMsgsTrace;

  /**
   * \brief Trace information for the ctrl slot statistics
   */
  TracedCallback<const SfnSf &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint16_t, uint16_t> m_phySlotCtrlStats;
  /**
   * \brief Trace information for the data slot statistics
   */
  TracedCallback<const SfnSf &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint16_t, uint16_t> m_phySlotDataStats;

  TracedCallback<const SfnSf &, uint8_t, const std::vector<int>&, uint16_t, uint16_t> m_rbStatistics;

  std::map<uint32_t, std::vector<uint32_t>> m_toSendDl; //!< Map that indicates, for each slot, what DL DCI we have to send
  std::map<uint32_t, std::vector<uint32_t>> m_toSendUl; //!< Map that indicates, for each slot, what UL DCI we have to send
  std::map<uint32_t, std::vector<uint32_t>> m_generateUl; //!< Map that indicates, for each slot, what UL DCI we have to generate
  std::map<uint32_t, std::vector<uint32_t>> m_generateDl; //!< Map that indicates, for each slot, what DL DCI we have to generate

  std::map<uint32_t, uint32_t> m_dlHarqfbPosition; //!< Map that indicates, for each DL slot, where the UE has to send the Harq Feedback

  /**
   * \brief Status of the channel for the PHY
   */
  enum ChannelStatus
  {
    NONE,        //!< The PHY doesn't know the channel status
    REQUESTED,   //!< The PHY requested channel access
    GRANTED,     //!< The PHY has the channel, it can transmit
    TO_LOSE      //!< The PHY channel is granted, but it will be lost at the end of the slot
  };

  ChannelStatus m_channelStatus {NONE}; //!< The channel status
  EventId m_channelLostTimer; //!< Timer that, when expires, indicates that the channel is lost

  Ptr<NrChAccessManager> m_cam; //!< Channel Access Manager

  friend class LtePatternTestCase;

  uint32_t m_n0Delay {0}; //!< minimum processing delay (in slots) needed to decode DL DCI and decode DL data (UE side)
  uint32_t m_n1Delay {0}; //!< minimum processing delay (in slots) from the end of DL Data reception to the earliest possible start of the corresponding ACK/NACK transmission (UE side)
  uint32_t m_n2Delay {0}; //!< minimum processing delay (in slots) needed to decode UL DCI and prepare UL data (UE side)

  SfnSf m_currentSlot;      //!< The current slot number
  bool m_isPrimary {false}; //!< Is this PHY a primary phy?
};

}


#endif /* NR_ENB_PHY_H */

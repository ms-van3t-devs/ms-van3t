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

#ifndef NR_PHY_H
#define NR_PHY_H

#include "nr-phy-sap.h"
#include "nr-phy-mac-common.h"
#include <ns3/nr-spectrum-value-helper.h>
#include "nr-sl-ue-phy-sap.h"
#include "nr-sl-phy-mac-common.h"

namespace ns3 {

class NrNetDevice;
class NrControlMessage;
class NrSpectrumPhy;
class AntennaArrayBasicModel;
class UniformPlanarArray;
class BeamConfId;

/**
 * \ingroup ue-phy
 * \ingroup gnb-phy
 *
 * \brief The base class for gNb and UE physical layer
 *
 * From this class descends NrGnbPhy and NrUePhy, the physical layer
 * classes for the gNb and the UE. This class has four main duties:
 *
 * \section phy_management_ctrl Management of the control message list
 *
 * The control message list is maintained as a list that has, always, a number
 * of element equals to the latency between PHY and MAC, plus one. The list
 * is initialized by a call to InitializeMessageList(). The messages
 * are enqueued by MAC at the end of the list through the method EnqueueCtrlMessage().
 * If the PHY has the necessity of adding a message, then it can use the
 * no-latency version of it, namely EnqueueCtrlMsgNow(). The messages for the
 * current slot (i.e., the messages at the front of the list) can be retrieved
 * with PopCurrentSlotCtrlMsgs(). To know if there are messages for the current
 * slot, use IsCtrlMsgListEmpty(). The list is stored in the variable
 * m_controlMessageQueue.
 *
 * \section phy_slot Management of the slot allocation list
 *
 * At the gNb, After the MAC does the slot allocation, it is saved in the PHY with the method
 * PushBackSlotAllocInfo(), and if an allocation for the same slot is already
 * present, the two will be merged together. The slot allocation is stored
 * inside the variable m_slotAllocInfo.
 *
 * \section phy_mac_pdu Management of the MAC PDU that waits to be transmitted
 *
 * With each allocation, will come also one (or more) MAC PDU, that are stored
 * within the method SetMacPdu(). The storage is based on the SfnSf inside the
 * variable m_packetBurstMap.
 *
 * \section phy_numerology Configuration of the numerology and the related settings
 *
 * When the numerology is configured, quite a lot of parameters change as well.
 * The entire process of updating the relative parameters is done inside the
 * function SetNumerology().
 *
 * \section phy_antenna Antenna and BeamManager object installation
 *
 */
class NrPhy : public Object
{
public:
  /**
   * \brief NrPhy constructor
   */
  NrPhy ();

  /**
   * \brief ~NrPhy
   */
  virtual ~NrPhy () override;

  /**
   * \brief Get the TypeId of the Object
   * \return the TypeId of the Object
   */
  static TypeId GetTypeId (void);

  // Called by SAP
  /**
   * \brief Enqueue a ctrl message, keeping in consideration L1L2CtrlDelay
   * \param m the message to enqueue
   */
  void EnqueueCtrlMessage (const Ptr<NrControlMessage> &m);

  /**
   * \brief Store a MAC PDU
   * \param p the MAC PDU
   * \param sfn The SfnSf at which store the PDU
   * \param symStart The symbol inside the SfnSf at which the data will be transmitted
   * \param streamId The stream id through which this pkt would be transmitted
   *
   * It will be saved in the PacketBurst map following the SfnSf present in the tag.
   */
  void SetMacPdu (const Ptr<Packet> &p, const SfnSf & sfn, uint8_t symStart, uint8_t streamId);

  /**
   * \brief Send the RachPreamble
   *
   * The RACH PREAMBLE is sent ASAP, without applying any delay,
   * since it is sent in the PRACH channel
   *
   * \param PreambleId preamble ID
   * \param Rnti RNTI
   */
  void SendRachPreamble (uint32_t PreambleId, uint32_t Rnti);

  /**
   * \brief Store the slot allocation info
   * \param slotAllocInfo the allocation to store
   *
   * This method expect that the sfn of the allocation will match the sfn
   * when the allocation will be retrieved.
   */
  void PushBackSlotAllocInfo (const SlotAllocInfo &slotAllocInfo);

  /**
   * \brief Notify PHY about the successful RRC connection
   * establishment.
   */
  void NotifyConnectionSuccessful ();

  /**
   * \brief Configures TB decode latency
   * \param us decode latency
   */
  virtual void SetTbDecodeLatency (const Time &us);

  /**
   * \brief Returns Transport Block decode latency
   * \return The TB decode latency
   */
  virtual Time GetTbDecodeLatency (void) const;

  /**
   * \brief Get the beam conf id for the specified user
   * \param rnti RNTI
   * \return the BeamConfId associated to the specified RNTI
   */
  virtual BeamConfId GetBeamConfId (uint16_t rnti) const = 0;

  /**
   * \brief Get the spectrum model of the PHY
   * \return a pointer to the spectrum model
   */
  Ptr<const SpectrumModel> GetSpectrumModel ();

  /**
   * \brief Get the number of symbols in a slot
   * \return the number of symbol per slot
   */
  uint32_t GetSymbolsPerSlot () const;

  /**
   * \brief Get the slot period
   * \return the slot period (depend on the numerology)
   */
  Time GetSlotPeriod () const;

  /**
   * \brief Get SymbolPeriod
   * \return the symbol period; the value changes every time the user changes
   * the numerology
   */
  Time GetSymbolPeriod () const;

  /**
   * \brief Set the NoiseFigure value
   * \param d the Noise figure value
   */
  void SetNoiseFigure (double d);

  /**
   * \brief Get the NoiseFigure value
   * \return the noise figure
   */
  double GetNoiseFigure () const;

  /**
   * \brief Retrieve the Tx power
   *
   * \return the Tx power
   */
   virtual double GetTxPower () const = 0;

  // Installation / Helpers
  /**
   * \brief Retrieve a pointer to an instance of NrPhySapProvider
   * \return NrPhySapProvider pointer, that points at this instance
   */
  NrPhySapProvider* GetPhySapProvider ();

  /**
   * \brief Set the owner device
   * \param d the owner device
   */
  void SetDevice (Ptr<NrNetDevice> d);

  /**
   * \brief Install the PHY over a particular central frequency
   * \param f the frequency in Hz
   * The helper will call this function, making sure it is equal to the value
   * of the channel.
   */
  void InstallCentralFrequency (double f);

  /**
   * \brief Set GNB or UE numerology
   * \param numerology numerology
   *
   * For the GNB, this is an attribute that can be changed at any time; for the
   * UE, it is set by the helper at the attachment, and then is not changed anymore.
   */
  void SetNumerology (uint16_t numerology);

  /**
   * \brief Get the configured numerology
   * \return the configured numerology
   */
  uint16_t GetNumerology () const;

  /**
   * \brief Set the number of symbol per slot
   * \param symbolsPerSlot the value of symbol per slot (12 or 14)
   */
  void SetSymbolsPerSlot (uint16_t symbolsPerSlot);

  /**
   * \brief Set the bandwidth overhead for calculating the usable RB number
   * \param oh the overhead
   */
  void SetRbOverhead (double oh);

  /**
   * \brief Get the bandwidth overhead used when calculating the usable RB number
   * \return the bandwidth overhead
   */
  double GetRbOverhead () const;

  /**
   * \brief Returns the number of streams, which corresponds to the number of NrSpectrumPhys per NrPhy
   */
  uint8_t GetNumberOfStreams  () const;

  /**
   * \brief Retrieve the SpectrumPhy pointer
   *
   * As this function is used mainly to get traced values out of Spectrum,
   * it should be removed and the traces connected (and redirected) here.
   * \param streamIndex the index of the stream of which we will return the SpectrumPhy
   * \return A pointer to the SpectrumPhy of this UE
   */
  Ptr<NrSpectrumPhy> GetSpectrumPhy (uint8_t streamIndex = 0) const;

  /**
   * \brief Set the SpectrumPhy associated with this PHY
   * \param spectrumPhy the spectrumPhy
   */
  void InstallSpectrumPhy (const Ptr<NrSpectrumPhy> &spectrumPhy);

  /**
   * \brief Schedule the start of the NR event loop
   *
   * \param nodeId node id (for log messages)
   * \param frame Starting frame number
   * \param subframe Starting subframe number
   * \param slot Starting slot number
   *
   * The method is scheduling the start of the first slot at time 0.0.
   */
  virtual void ScheduleStartEventLoop (uint32_t nodeId, uint16_t frame, uint8_t subframe, uint16_t slot) = 0;

  /**
   * \brief Set the bwp id. Called by helper.
   */
  void SetBwpId (uint16_t bwpId);

  /**
   * \return the BWP ID
   */
  uint16_t GetBwpId () const;

  /**
   * \return the cell ID
   */
  uint16_t GetCellId () const;

  /**
   * \brief Get the number of Resource block configured
   *
   * It changes with the numerology and the channel bandwidth
   */
  uint32_t GetRbNum () const ;

  /**
   * \brief Retrieve the channel bandwidth, in Hz
   * \return the channel bandwidth in Hz
   */
  uint32_t GetChannelBandwidth () const;

  /**
   * \brief Retrieve the subcarrier spacing in Hz. Subcarrier spacing
   * is updated when the numerology is being updated.
   * \return the sucarrier spacing in Hz
   */
  uint32_t GetSubcarrierSpacing () const;

  /**
   * \return the latency (in n. of slots) between L1 and L2 layers. Default to 2.
   *
   * Before it was an attribute; as we are unsure if it works for values different
   * from 2, we decided to make it static until the need to have it different
   * from 2 arises.
   */
  uint32_t GetL1L2CtrlLatency () const;

  // SAP
  /**
   * \brief Set the cell ID
   * \param cellId the cell id.
   *
   * Called by lte-enb-cphy-sap only.
   */
  void DoSetCellId (uint16_t cellId);

  /**
   * \brief Take the control messages, and put it in a list that will be sent at the first occasion
   * \param msg Message to "encode" and transmit
   */
  void EncodeCtrlMsg (const Ptr<NrControlMessage> &msg);

  /**
   * \brief Go through the current pattern and see if at least one slot is DL or F.
   *
   * \return true if at least one slot is DL or F.
   */
  bool HasDlSlot () const;

  /**
   * \brief Go trough the current pattern and see if at least one slot is UL or F.
   *
   * \return true if at least one slot is UL or F.
   */
  bool HasUlSlot () const;

  /**
   * \brief See if at least one slot is DL or F.
   *
   * \param pattern Pattern to check
   * \return true if at least one slot is DL or F.
   */
  static bool HasDlSlot (const std::vector<LteNrTddSlotType> &pattern);

  /**
   * \brief See if at least one slot is UL or F.
   *
   * \param pattern Pattern to check
   * \return true if at least one slot is UL or F.
   */
  static bool HasUlSlot (const std::vector<LteNrTddSlotType> &pattern);

  /**
   * \brief Retrieve the frequency (in Hz) of this PHY's channel
   * \return the frequency of the channel in Hz
   *
   * The function will assert if it is called without having set a frequency first.
   */
  double GetCentralFrequency () const;

  /**
   * \brief Get the current SfnSf
   * \return the current SfnSf
   */
  virtual const SfnSf & GetCurrentSfnSf () const = 0;

  /**
   * \brief Get a string representation of a pattern
   * \param pattern the TDD pattern
   * \return a string representation of the pattern, such as "F|F|F|F|..."
   */
  static std::string GetPattern (const std::vector<LteNrTddSlotType> &pattern);

  /**
   * \brief Set power allocation type. There are currently supported two types:
   * one that distributes uniformly energy among all bandwidth (all RBs),
   * and another only over used or active RBs
   * \param powerAllocationType a type of power allocation to be used
   */
  void SetPowerAllocationType (enum NrSpectrumValueHelper::PowerAllocationType powerAllocationType);

  /**
   * \brief Get the power allocation type
   */
  enum NrSpectrumValueHelper::PowerAllocationType GetPowerAllocationType () const;

protected:
  /**
   * \brief DoDispose method inherited from Object
   */
  void virtual DoDispose () override;
  /**
   * \brief Update the number of RB. Usually called after bandwidth changes
   */
  void DoUpdateRbNum ();

  /**
   * \brief Function to set the channel bandwidth, used also by child classes, i.e.,
   * see functions DoSetDlBanwidth in NrUePhy and DoSetBandwidth in NrGnbPhy.
   * This function is also called by NrHelper when creating gNB and UE devices.
   * See CreateGnbPhy and CreateUePhy in NrHelper.
   * This function updates the number of RBs and thus the spectrum model, i.e.,
   * for noise PSD and for future transmissions.
   * \param bandwidth channel bandwidth in kHz * 100
   */
  void SetChannelBandwidth (uint16_t bandwidth);

  /**
   * \brief Check if a pattern is TDD
   * \param pattern the pattern to check
   * \return true if the pattern has at least one mixed DL/UL
   */
  static bool IsTdd (const std::vector<LteNrTddSlotType> &pattern);

  /**
   * \brief Transform a MAC-made vector of RBG to a PHY-ready vector of SINR indices
   * \param rbgBitmask Bitmask which indicates with 1 the RBG in which there is a transmission,
   * with 0 a RBG in which there is not a transmission
   * \return a vector of indices.
   *
   * Example (4 RB per RBG, 4 total RBG assignable):
   * rbgBitmask = <0,1,1,0>
   * output = <4,5,6,7,8,9,10,11>
   *
   * (the rbgBitmask expressed as rbBitmask would be:
   * <0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0> , and therefore the places in which there
   * is a 1 are from the 4th to the 11th, and that is reflected in the output)
   */
  std::vector<int> FromRBGBitmaskToRBAssignment (const std::vector<uint8_t> rbgBitmask) const;

  /**
   * \brief Protected function that is used to get the number of resource
   * blocks per resource block group.
   * \return Returns the number of RBs per RBG
   */
  virtual uint32_t GetNumRbPerRbg () const = 0;

  /**
   * \brief Retrieve the PacketBurst at the slot/symbol specified
   * \param sym Symbol at which the PacketBurst should be sent
   * \param streamId The stream id through which this pkt burst would be transmitted
   * \return a pointer to the burst, if present, otherwise nullptr
   */
  Ptr<PacketBurst> GetPacketBurst (SfnSf sf, uint8_t sym, uint8_t streamId);

  /**
   * \brief Create Noise Power Spectral density
   * \return A SpectrumValue array with fixed size, in which a value is
   * update to a particular value of the noise
   */
  Ptr<SpectrumValue> GetNoisePowerSpectralDensity ();

  /**
   * Create Tx Power Spectral Density
   * \param rbIndexVector vector of the index of the RB (in SpectrumValue array)
   * in which there is a transmission
   * \param activeStreams the number of active streams
   * \return A SpectrumValue array with fixed size, in which each value
   * is updated to a particular value if the correspond RB index was inside the rbIndexVector,
   * or is left untouched otherwise.
   * \see NrSpectrumValueHelper::CreateTxPowerSpectralDensity
   */
  Ptr<SpectrumValue> GetTxPowerSpectralDensity (const std::vector<int> &rbIndexVector, uint8_t activeStreams);

  /**
   * \brief Store the slot allocation info at the front
   * \param slotAllocInfo the allocation to store
   *
   * Increase the sfn of all allocations to be chronologically "in order".
   */
  void PushFrontSlotAllocInfo (const SfnSf &newSfnSf, const SlotAllocInfo &slotAllocInfo);

  /**
   * \brief Check if the SlotAllocationInfo for that slot exists
   * \param sfnsf slot to check
   * \return true if the allocation exists
   */
  bool SlotAllocInfoExists (const SfnSf &sfnsf) const;

  /**
   * \brief Get the head for the slot allocation info, and delete it from the
   * internal list
   * \return the Slot allocation info head
   */
  SlotAllocInfo RetrieveSlotAllocInfo ();

  /**
   * \brief Get the SlotAllocationInfo for the specified slot, and delete it
   * from the internal list
   *
   * \param sfnsf slot specified
   * \return the SlotAllocationInfo
   */
  SlotAllocInfo RetrieveSlotAllocInfo (const SfnSf &sfnsf);

  /**
   * \brief Peek the SlotAllocInfo at the SfnSf specified
   * \param sfnsf (existing) SfnSf to look for
   * \return a reference to the SlotAllocInfo
   *
   * The method will assert if sfnsf does not exits (please check with SlotExists())
   */
  SlotAllocInfo & PeekSlotAllocInfo (const SfnSf & sfnsf);

  /**
   * \brief Retrieve the size of the SlotAllocInfo list
   * \return the allocation list size
   */
  size_t SlotAllocInfoSize () const;

  /**
   * \brief Check if there are no control messages queued for this slot
   * \return true if there are no control messages queued for this slot
   */
  bool IsCtrlMsgListEmpty () const;

  /**
   * \brief Enqueue a CTRL message without considering L1L2CtrlLatency
   * \param msg The message to enqueue
   */
  void EnqueueCtrlMsgNow (const Ptr<NrControlMessage> &msg);

  /**
   * \brief Enqueue a CTRL message without considering L1L2CtrlLatency
   * \param listOfMsgs The list of messages to enqueue
   */
  void EnqueueCtrlMsgNow (const std::list<Ptr<NrControlMessage> > &listOfMsgs);

  /**
   * \brief Initialize the message list
   */
  void InitializeMessageList ();
  /**
   * \brief Extract and return the message list that is at the beginning of the queue
   * \return a list of control messages that are meant to be sent in the current slot
   */
  virtual std::list<Ptr<NrControlMessage> > PopCurrentSlotCtrlMsgs (void);

protected:
  Ptr<NrNetDevice> m_netDevice;      //!< Pointer to the owner netDevice.
  std::vector<Ptr<NrSpectrumPhy>> m_spectrumPhys;  //!< vector to the (owned) spectrum phy instances

  double m_txPower {0.0};                //!< Transmission power (attribute)
  double m_noiseFigure {0.0};            //!< Noise figure (attribute)

  std::unordered_map<uint64_t, Ptr<PacketBurst> > m_packetBurstMap; //!< Map between SfnSf and PacketBurst

  SlotAllocInfo m_currSlotAllocInfo;  //!< Current slot allocation

  NrPhySapProvider* m_phySapProvider; //!< Pointer to the MAC

  uint32_t m_raPreambleId {0}; //!< Preamble ID

  std::list <Ptr<NrControlMessage>> m_ctrlMsgs; //!< CTRL messages to be sent

  std::vector<LteNrTddSlotType> m_tddPattern = { F, F, F, F, F, F, F, F, F, F}; //!< Pattern

private:
  std::list<SlotAllocInfo> m_slotAllocInfo; //!< slot allocation info list
  std::vector<std::list<Ptr<NrControlMessage>>> m_controlMessageQueue; //!< CTRL message queue

  Time m_tbDecodeLatencyUs {MicroSeconds(100)}; //!< transport block decode latency
  double m_centralFrequency {-1.0};             //!< Channel central frequency -- set by the helper
  uint16_t m_channelBandwidth {0};  //!< Value in kHz * 100. Set by RRC. 
                                    // E.g. if set to 200, the bandwidth will be 20 MHz (= 200 * 100 KHz)

  uint16_t m_cellId {0};             //!< Cell ID which identify this BWP.
  uint16_t m_bwpId {UINT16_MAX};     //!< Bwp ID -- in the GNB, it is set by RRC, in the UE, by the helper
  uint16_t m_numerology {0};         //!< Numerology: defines the scs, RB width, slot length, slots per subframe
  uint16_t m_symbolsPerSlot {14};    //!< number of OFDM symbols per slot

  uint16_t m_slotsPerSubframe {0};              //!< Number of slots per subframe, changes with numerology
  Time m_slotPeriod {0};                        //!< NR slot length (changes with numerology and symbolsPerSlot)
  Time m_symbolPeriod {0};                      //!< OFDM symbol length (changes with numerology)
  uint32_t m_subcarrierSpacing {0};             //!< subcarrier spacing (it is determined by the numerology), can be 15KHz, 30KHz, 60KHz, 120KHz, ...
  uint32_t m_rbNum {0};                         //!< number of resource blocks within the channel bandwidth
  double m_rbOh {0.04};                         //!< Overhead for the RB calculation
  enum NrSpectrumValueHelper::PowerAllocationType m_powerAllocationType {NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_USED}; //!< The type of power allocation, supported modes to distribute power uniformly over all RBs, or only used RBs

//NR Sidelink

public:
  /**
   * \brief Get the NR Sidelik UE PHY SAP offered by UE PHY to UE MAC
   *
   * \return the NR Sidelik UE PHY SAP provider interface offered by
   *         UE PHY to UE MAC
   */
  NrSlUePhySapProvider* GetNrSlUePhySapProvider ();
  /**
   * \brief Get the slot period
   * \return the slot period (depend on the numerology)
   */
  Time DoGetSlotPeriod () const;
  /**
   * \brief Ask the PHY the bandwidth in RBs
   *
   * \return the bandwidth in RBs
   */
  uint32_t DoGetBwInRbs () const;
  /**
   * \brief Send NR Sidelink PSCCH MAC PDU
   *
   * In future, when PSCCH could go without data, consider receiving
   * the pscch pkt and its var tti info using one method, e.g.,
   * SendPscchMacPdu (const SfnSf &sfn, const NrSlVarTtiAllocInfo& varTtiInfo, Ptr<Packet> p).
   * This will make sure that PHY receives var tti info of each MAC PDU
   * transmission. Then, within this method call DoSetNrSlVarTtiAllocInfo
   * and SetPscchMacPdu.
   *
   * \param p The packet
   */
  void DoSendPscchMacPdu (Ptr<Packet> p);
  /**
   * \brief Send NR Sidelink PSSCH MAC PDU
   *
   * In future, when PSCCH could go without data, consider receiving
   * the pssch pkt and its var tti info using one method, e.g.,
   * SendPsschMacPdu (const SfnSf &sfn, const NrSlVarTtiAllocInfo& varTtiInfo, Ptr<Packet> p).
   * This will make sure that PHY receives var tti info of each MAC PDU
   * transmission. Then, within this method call DoSetNrSlVarTtiAllocInfo
   * and SetPscchMacPdu.
   *
   * \param p The packet
   */
  void DoSendPsschMacPdu (Ptr<Packet> p);
  /**
   * \brief Set the allocation info for NR SL slot in PHY
   * \param sfn The SfnSf
   * \param varTtiInfo The Variable TTI allocation info
   */
  void DoSetNrSlVarTtiAllocInfo (const SfnSf &sfn, const NrSlVarTtiAllocInfo& varTtiInfo);

protected:
  /**
   * \brief Pop the NR Sidelink PSCCH packet burst
   * \return The packet burst
   */
  Ptr<PacketBurst> PopPscchPacketBurst ();
  /**
   * \brief Pop the NR Sidelink PSSCH packet burst
   * \return The packet burst
   */
  Ptr<PacketBurst> PopPsschPacketBurst ();
  /**
   * \brief Check if there is an allocation for NR SL slot
   * \param sfn the sfn
   * \return true, if allocation info sfn matches the \p sfn and the
   *         CTRL and data queues have packet (s)
   */
  bool NrSlSlotAllocInfoExists (const SfnSf &sfn) const;

  std::vector< Ptr<PacketBurst> > m_nrSlPscchPacketBurstQueue; //!< A queue of NR SL PSCCH (SCI format 0) packet bursts to be sent
  std::vector< Ptr<PacketBurst> > m_nrSlPsschPacketBurstQueue; //!< A queue of NR SL PSSCH (SCI format 1 + Data) packet bursts to be sent.
  std::deque <NrSlPhySlotAlloc> m_nrSlAllocInfoQueue; //!< Current NR SL allocation info for a slot
  NrSlPhySlotAlloc m_nrSlCurrentAlloc; //!< Current NR SL allocation info for a slot

private:
  /**
   * \brief Set NR Sidelink PSCCH MAC PDU
   *
   * Add the PSCCH MAC PDU to the queue
   *
   * \param p The packet
   */
  void SetPscchMacPdu (Ptr<Packet> p);
  /**
   * \brief Set NR Sidelink PSSCH MAC PDU
   *
   * Add the PSSCH MAC PDU to the queue
   *
   * \param p The packet
   */
  void SetPsschMacPdu (Ptr<Packet> p);
  //NR Sidelink
  NrSlUePhySapProvider* m_nrSlUePhySapProvider; //!< SAP interface to receive calls from UE MAC instance for NR Sidelink
};

} // namespace ns3

#endif /* NR_PHY_H */

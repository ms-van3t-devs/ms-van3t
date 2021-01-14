/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 */

#ifndef CV2X_ENB_LTE_PHY_H
#define CV2X_ENB_LTE_PHY_H


#include <ns3/cv2x_lte-control-messages.h>
#include <ns3/cv2x_lte-enb-phy-sap.h>
#include <ns3/cv2x_lte-enb-cphy-sap.h>
#include <ns3/cv2x_lte-phy.h>
#include <ns3/cv2x_lte-harq-phy.h>

#include <map>
#include <set>



namespace ns3 {

class PacketBurst;
class cv2x_LteNetDevice;
class cv2x_LteUePhy;

/**
 * \ingroup lte
 * cv2x_LteEnbPhy models the physical layer for the eNodeB
 */
class cv2x_LteEnbPhy : public cv2x_LtePhy
{
  /// allow cv2x_EnbMemberLteEnbPhySapProvider class friend access
  friend class cv2x_EnbMemberLteEnbPhySapProvider;
  /// allow cv2x_MemberLteEnbCphySapProvider<cv2x_LteEnbPhy> class friend access
  friend class cv2x_MemberLteEnbCphySapProvider<cv2x_LteEnbPhy>;

public:
  /**
   * @warning the default constructor should not be used
   */
  cv2x_LteEnbPhy ();

  /**
   *
   * \param dlPhy the downlink cv2x_LteSpectrumPhy instance
   * \param ulPhy the uplink cv2x_LteSpectrumPhy instance
   * \param EnbEnablePhyLayer enable enb phy layer ?
   */
  cv2x_LteEnbPhy (Ptr<cv2x_LteSpectrumPhy> dlPhy, Ptr<cv2x_LteSpectrumPhy> ulPhy, bool EnbEnablePhyLayer = true);

  virtual ~cv2x_LteEnbPhy ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  // inherited from Object
  virtual void DoInitialize (void);
  virtual void DoDispose (void);


  /**
  * \brief Get the PHY SAP provider
  * \return a pointer to the SAP Provider of the PHY
  */
  cv2x_LteEnbPhySapProvider* GetLteEnbPhySapProvider ();

  /**
  * \brief Set the PHY SAP User
  * \param s a pointer to the PHY SAP user
  */
  void SetLteEnbPhySapUser (cv2x_LteEnbPhySapUser* s);

  /**
   * \brief Get the CPHY SAP provider
   * \return a pointer to the SAP Provider
   */
  cv2x_LteEnbCphySapProvider* GetLteEnbCphySapProvider ();

  /**
  * \brief Set the CPHY SAP User
  * \param s a pointer to the SAP user
  */
  void SetLteEnbCphySapUser (cv2x_LteEnbCphySapUser* s);

  /**
   * \param pow the transmission power in dBm
   */
  void SetTxPower (double pow);

  /**
   * \return the transmission power in dBm
   */
  double GetTxPower () const;

  /**
   * \return the transmission power in dBm
   */
  int8_t DoGetReferenceSignalPower () const;

  /**
   * \param pow the noise figure in dB
   */
  void SetNoiseFigure (double pow);

  /**
   * \return the noise figure in dB
   */
  double GetNoiseFigure () const;

  /**
   * \param delay the TTI delay between MAC and channel
   */
  void SetMacChDelay (uint8_t delay);

  /**
   * \returns the TTI delay between MAC and channel
   */
  uint8_t GetMacChDelay (void) const;

  /**
   * \return a pointer to the cv2x_LteSpectrumPhy instance relative to the downlink
   */
  Ptr<cv2x_LteSpectrumPhy> GetDlSpectrumPhy () const;

  /**
   * \return a pointer to the cv2x_LteSpectrumPhy instance relative to the uplink
   */
  Ptr<cv2x_LteSpectrumPhy> GetUlSpectrumPhy () const;


  /**
   * \brief set the resource blocks (a.k.a. sub channels) to be used in the downlink for transmission
   * 
   * \param mask a vector of integers, if the i-th value is j it means
   * that the j-th resource block is used for transmission in the
   * downlink. If there is no i such that the value of the i-th
   * element is j, it means that RB j is not used.
   */
  void SetDownlinkSubChannels (std::vector<int> mask );

  /**
   * \brief set the resource blocks (a.k.a. sub channels) and its power
   * to be used in the downlink for transmission
   *
   * \param mask a vector of integers, if the i-th value is j it means
   * that the j-th resource block is used for transmission in the
   * downlink. If there is no i such that the value of the i-th
   * element is j, it means that RB j is not used.
   */
  void SetDownlinkSubChannelsWithPowerAllocation (std::vector<int> mask);
  /**
   * 
   * \return  a vector of integers, if the i-th value is j it means
   * that the j-th resource block is used for transmission in the
   * downlink. If there is no i such that the value of the i-th
   * element is j, it means that RB j is not used.
   */
  std::vector<int> GetDownlinkSubChannels (void);


  /**
   * \brief Generate power allocation map (i.e. tx power level for each RB)
   *
   * \param rnti indicates which UE will occupy this RB
   * \param rbId indicates which RB UE is using,
   * power level for this RB is power level of UE
   */
  void GeneratePowerAllocationMap (uint16_t rnti, int rbId);

  /**
   * \brief Create the PSD for TX
   * \returns the PSD
   */
  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensity ();

  /**
   * \brief Create the PSD for TX with power allocation for each RB
   * \return the PSD
   */
  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensityWithPowerAllocation ();

  /**
   * \brief Calculate the channel quality for a given UE
   * \param sinr a list of computed SINR
   * \param ue the UE
   */
  void CalcChannelQualityForUe (std::vector <double> sinr, Ptr<cv2x_LteSpectrumPhy> ue);

  /**
   * \brief Receive the control message
   * \param msg the received message
   */
  virtual void ReceiveLteControlMessage (Ptr<cv2x_LteControlMessage> msg);

  /**
  * \brief Create the UL CQI feedback from SINR values perceived at
  * the physical layer with the PUSCH signal received from eNB
  * \param sinr SINR values vector
  * \return UL CQI feedback in the format usable by an FF MAC scheduler
  */
  cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters CreatePuschCqiReport (const SpectrumValue& sinr);

  /**
  * \brief Create the UL CQI feedback from SINR values perceived at
  * the physical layer with the SRS signal received from eNB
  * \param sinr SINR values vector
  * \return UL CQI feedback in the format usable by an FF MAC scheduler
  */
  cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters CreateSrsCqiReport (const SpectrumValue& sinr);

  /**
  * \brief Send the PDCCH and PCFICH in the first 3 symbols
  * \param ctrlMsgList the list of control messages of PDCCH
  */
  void SendControlChannels (std::list<Ptr<cv2x_LteControlMessage> > ctrlMsgList);

  /**
  * \brief Send the PDSCH
  * \param pb the PacketBurst to be sent
  */
  void SendDataChannels (Ptr<PacketBurst> pb);

  /**
  * \param m the UL-CQI to be queued
  */
  void QueueUlDci (cv2x_UlDciLteControlMessage m);

  /**
  * \returns the list of UL-CQI to be processed
  */
  std::list<cv2x_UlDciLteControlMessage> DequeueUlDci (void);


  /**
   * \brief Start a LTE frame
   */
  void StartFrame (void);
  /**
   * \brief Start a LTE sub frame
   */
  void StartSubFrame (void);
  /**
   * \brief End a LTE sub frame
   */
  void EndSubFrame (void);
  /**
   * \brief End a LTE frame
   */
  void EndFrame (void);

  /**
   * \brief PhySpectrum received a new PHY-PDU
   * \param p the packet received
   */
  void PhyPduReceived (Ptr<Packet> p);

  /**
   * \brief PhySpectrum received a new list of cv2x_LteControlMessage
   */
  virtual void ReceiveLteControlMessageList (std::list<Ptr<cv2x_LteControlMessage> >);

  // inherited from cv2x_LtePhy
  virtual void GenerateCtrlCqiReport (const SpectrumValue& sinr);
  virtual void GenerateDataCqiReport (const SpectrumValue& sinr);
  virtual void ReportInterference (const SpectrumValue& interf);
  virtual void ReportRsReceivedPower (const SpectrumValue& power);



  /**
   * \brief PhySpectrum generated a new UL HARQ feedback
   * \param mes cv2x_UlInfoListElement_s
   */
  virtual void ReceiveLteUlHarqFeedback (cv2x_UlInfoListElement_s mes);

  /**
   * \brief PhySpectrum generated a new UL HARQ feedback
   *
   * \param harq the HARQ Phy
   */
  void SetHarqPhyModule (Ptr<cv2x_LteHarqPhy> harq);

  /**
   * TracedCallback signature for the linear average of SRS SINRs.
   *
   * \param [in] cellId
   * \param [in] rnti
   * \param [in] sinrLinear
   */
  typedef void (* ReportUeSinrTracedCallback)
    (uint16_t cellId, uint16_t rnti, double sinrLinear, uint8_t componentCarrierId);

  /**
   * TracedCallback signature for the linear average of SRS SINRs.
   *
   * \param [in] cellId
   * \param [in] spectrumValue
   * \deprecated The non-const \c Ptr<SpectrumValue> argument is deprecated
   * and will be changed to \c Ptr<const SpectrumValue> in a future release.
   */
  typedef void (* ReportInterferenceTracedCallback)
    (uint16_t cellId, Ptr<SpectrumValue> spectrumValue);

private:

  // cv2x_LteEnbCphySapProvider forwarded methods
  /**
   * Set bandwidth function
   *
   * \param ulBandwidth UL bandwidth
   * \param dlBandwidth DL bandwidth
   */
  void DoSetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth);
  /**
   * Set EARFCN
   *
   * \param dlEarfcn DL EARFCN
   * \param ulEarfcn UL EARFCN
   */
  void DoSetEarfcn (uint32_t dlEarfcn, uint32_t ulEarfcn);
  /**
   * Add UE
   *
   * \param rnti RNTI
   */
  void DoAddUe (uint16_t rnti);
  /**
   * Remove UE
   *
   * \param rnti RNTI
   */
  void DoRemoveUe (uint16_t rnti);
  /**
   * Set PA
   *
   * \param rnti RNTI
   * \param pa PA
   */
  void DoSetPa (uint16_t rnti, double pa);
  /**
   * Set transmission mode
   *
   * \param rnti RNTI
   * \param txMode transmit mode
   */
  void DoSetTransmissionMode (uint16_t  rnti, uint8_t txMode);
  /**
   * Set source configuration index
   *
   * \param rnti RNTI
   * \param srcCi source configuration index
   */
  void DoSetSrsConfigurationIndex (uint16_t  rnti, uint16_t srcCi);
  /**
   * Set master information block
   *
   * \param mib cv2x_LteRrcSap::MasterInformationBlock
   */
  void DoSetMasterInformationBlock (cv2x_LteRrcSap::MasterInformationBlock mib);
  /**
   * Set system information block
   *
   * \param sib1 cv2x_LteRrcSap::SystemInformationBlockType1
   */
  void DoSetSystemInformationBlockType1 (cv2x_LteRrcSap::SystemInformationBlockType1 sib1);

  // cv2x_LteEnbPhySapProvider forwarded methods
  void DoSendMacPdu (Ptr<Packet> p);
  /**
   * Send LTE Control Message function
   *
   * \param msg LTE control message
   */
  void DoSendLteControlMessage (Ptr<cv2x_LteControlMessage> msg);
  /**
   * Get MAC ch TTI delay function
   *
   * \returns delay value
   */
  uint8_t DoGetMacChTtiDelay ();

  /**
   * Add the given RNTI to the list of attached UE #m_ueAttached.
   * \param rnti RNTI of a UE
   * \return true if the RNTI has _not_ existed before, or false otherwise.
   */
  bool AddUePhy (uint16_t rnti);
  /**
   * Remove the given RNTI from the list of attached UE #m_ueAttached.
   * \param rnti RNTI of a UE
   * \return true if the RNTI has existed before, or false otherwise.
   */
  bool DeleteUePhy (uint16_t rnti);

  /**
   * Create SRS report function
   *
   * \param rnti the RNTI
   * \param srs the SRS
   */
  void CreateSrsReport (uint16_t rnti, double srs);

  /**
   * List of RNTI of attached UEs. Used for quickly determining whether a UE is
   * attached to this eNodeB or not.
   */
  std::set <uint16_t> m_ueAttached;


  /// P_A per UE RNTI.
  std::map <uint16_t,double> m_paMap;

  /// DL power allocation map.
  std::map <int, double> m_dlPowerAllocationMap;

  /**
   * A vector of integers, if the i-th value is j it means that the j-th
   * resource block is used for transmission in the downlink. If there is
   * no i such that the value of the i-th element is j, it means that RB j
   * is not used.
   */
  std::vector <int> m_listOfDownlinkSubchannel;

  std::vector <int> m_dlDataRbMap; ///< DL data RB map

  /// For storing info on future receptions.
  std::vector< std::list<cv2x_UlDciLteControlMessage> > m_ulDciQueue;

  cv2x_LteEnbPhySapProvider* m_enbPhySapProvider; ///< ENB Phy SAP provider
  cv2x_LteEnbPhySapUser* m_enbPhySapUser; ///< ENB Phy SAP user

  cv2x_LteEnbCphySapProvider* m_enbCphySapProvider; ///< ENB CPhy SAP provider
  cv2x_LteEnbCphySapUser* m_enbCphySapUser; ///< ENB CPhy SAP user

  /**
   * The frame number currently served. In ns-3, frame number starts from 1.
   * In contrast, the 3GPP standard's frame number starts from 0.
   */
  uint32_t m_nrFrames;
  /**
   * The subframe number currently served. In ns-3, subframe number starts
   * from 1. In contrast, the 3GPP standard's subframe number starts from 0.
   * The number resets to the beginning again after 10 subframes.
   */
  uint32_t m_nrSubFrames;

  uint16_t m_srsPeriodicity; ///< SRS periodicity
  Time m_srsStartTime; ///< SRS start time
  std::map <uint16_t,uint16_t> m_srsCounter; ///< SRS counter
  std::vector <uint16_t> m_srsUeOffset; ///< SRS UE offset
  uint16_t m_currentSrsOffset; ///< current SRS offset

  /**
   * The Master Information Block message to be broadcasted every frame.
   * The message content is specified by the upper layer through the RRC SAP.
   */
  cv2x_LteRrcSap::MasterInformationBlock m_mib;
  /**
   * The System Information Block Type 1 message to be broadcasted. SIB1 is
   * broadcasted every 6th subframe of every odd-numbered radio frame.
   * The message content is specified by the upper layer through the RRC SAP.
   */
  cv2x_LteRrcSap::SystemInformationBlockType1 m_sib1;

  Ptr<cv2x_LteHarqPhy> m_harqPhyModule; ///< HARQ Phy module

  /**
   * The `ReportUeSinr` trace source. Reporting the linear average of SRS SINR.
   * Exporting cell ID, RNTI, SINR in linear unit and cv2x_ComponentCarrierId
   */
  TracedCallback<uint16_t, uint16_t, double, uint8_t> m_reportUeSinr;
  /**
   * The `UeSinrSamplePeriod` trace source. The sampling period for reporting
   * UEs' SINR stats.
   */
  uint16_t m_srsSamplePeriod;
  std::map <uint16_t,uint16_t> m_srsSampleCounterMap; ///< SRS sample counter map

  /**
   * The `ReportInterference` trace source. Reporting the interference per PHY
   * RB (TS 36.214 section 5.2.2, measured on DATA). Exporting cell ID and
   * interference linear power per RB basis.
   * \deprecated The non-const \c Ptr<SpectrumValue> argument is deprecated
   * and will be changed to \c Ptr<const SpectrumValue> in a future release.
   */
  TracedCallback<uint16_t, Ptr<SpectrumValue> > m_reportInterferenceTrace;
  /**
   * The `InterferenceSamplePeriod` attribute. The sampling period for
   * reporting interference stats.
   * \todo In what unit is this?
   */
  uint16_t m_interferenceSamplePeriod;
  uint16_t m_interferenceSampleCounter; ///< interference sample counter

  /**
   * The `DlPhyTransmission` trace source. Contains trace information regarding
   * PHY stats from DL Tx perspective. Exporting a structure with type
   * cv2x_PhyTransmissionStatParameters.
   */
  TracedCallback<cv2x_PhyTransmissionStatParameters> m_dlPhyTransmission;

}; // end of `class cv2x_LteEnbPhy`


}

#endif /* LTE_ENB_PHY_H */

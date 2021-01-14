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
 * Authors: Giuseppe Piro  <g.piro@poliba.it>
 *          Marco Miozzo <mmiozzo@cttc.es>
 * Modified by: NIST (D2D)
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#ifndef CV2X_LTE_UE_PHY_H
#define CV2X_LTE_UE_PHY_H


#include <ns3/cv2x_lte-phy.h>
#include <ns3/cv2x_ff-mac-common.h>

#include <cstdlib>
#include <ns3/cv2x_lte-control-messages.h>
#include <ns3/cv2x_lte-amc.h>
#include <ns3/cv2x_lte-ue-phy-sap.h>
#include <ns3/cv2x_lte-ue-cphy-sap.h>
#include <ns3/ptr.h>
#include <ns3/cv2x_lte-amc.h>
#include <set>
#include <ns3/cv2x_lte-ue-power-control.h>


namespace ns3 {

class PacketBurst;
class cv2x_LteEnbPhy;
class cv2x_LteHarqPhy;


/**
 * \ingroup lte
 *
 * The cv2x_LteSpectrumPhy models the physical layer of LTE
 */
class cv2x_LteUePhy : public cv2x_LtePhy
{

  /// allow cv2x_UeMemberLteUePhySapProvider class friend access
  friend class cv2x_UeMemberLteUePhySapProvider;
  /// allow cv2x_MemberLteUeCphySapProvider<cv2x_LteUePhy> class friend access
  friend class cv2x_MemberLteUeCphySapProvider<cv2x_LteUePhy>;

public:
  /**
   * \brief The states of the UE PHY entity
   */
  enum State
  {
    CELL_SEARCH = 0,
    SYNCHRONIZED,
    NUM_STATES
  };

  /**
   * @warning the default constructor should not be used
   */
  cv2x_LteUePhy ();

  /**
   *
   * \param dlPhy the downlink cv2x_LteSpectrumPhy instance
   * \param ulPhy the uplink cv2x_LteSpectrumPhy instance
   */
  cv2x_LteUePhy (Ptr<cv2x_LteSpectrumPhy> dlPhy, Ptr<cv2x_LteSpectrumPhy> ulPhy);

  virtual ~cv2x_LteUePhy ();

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
   * \return a pointer to the SAP Provider 
   */
  cv2x_LteUePhySapProvider* GetLteUePhySapProvider ();

  /**
  * \brief Set the PHY SAP User
  * \param s a pointer to the SAP user
  */
  void SetLteUePhySapUser (cv2x_LteUePhySapUser* s);

  /**
   * \brief Get the CPHY SAP provider
   * \return a pointer to the SAP Provider
   */
  cv2x_LteUeCphySapProvider* GetLteUeCphySapProvider ();

  /**
  * \brief Set the CPHY SAP User
  * \param s a pointer to the SAP user
  */
  void SetLteUeCphySapUser (cv2x_LteUeCphySapUser* s);


  /**
   * \param pow the transmission power in dBm
   */
  void SetTxPower (double pow);

  /**
   * \return the transmission power in dBm
   */
  double GetTxPower () const;

  /**
   * \return ptr to UE Uplink Power Control entity
   */
  Ptr<cv2x_LteUePowerControl> GetUplinkPowerControl () const;

  /**
   * \param nf the noise figure in dB
   */
  void SetNoiseFigure (double nf);

  /**
   * \return the noise figure in dB
   */
  double GetNoiseFigure () const;

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
   * set the spectrum phy instance for sidelink reception
   * \param phy the sidelink spectrum phy
   */
  void SetSlSpectrumPhy (Ptr<cv2x_LteSpectrumPhy> phy);
  
  /**
   * \return a pointer to the cv2x_LteSpectrumPhy instance relative to the sidelink reception
   */
  Ptr<cv2x_LteSpectrumPhy> GetSlSpectrumPhy () const;

  /**
   * \brief Create the PSD for the TX
   * \return the pointer to the PSD
   */
  virtual Ptr<SpectrumValue> CreateTxPowerSpectralDensity ();

  /**
   * \brief Set a list of sub channels to use in TX
   * \param mask a list of sub channels
   */
  void SetSubChannelsForTransmission (std::vector <int> mask);
  /**
   * \brief Get a list of sub channels to use in RX
   * \return a list of sub channels
   */
  std::vector <int> GetSubChannelsForTransmission (void);

  /**
   * \brief Get a list of sub channels to use in RX
   * \param mask list of sub channels
   */
  void SetSubChannelsForReception (std::vector <int> mask);
  /**
   * \brief Get a list of sub channels to use in RX
   * \return a list of sub channels
   */
  std::vector <int> GetSubChannelsForReception (void);

  /**
  * \brief Create the DL CQI feedback from SINR values perceived at
  * the physical layer with the signal received from eNB
  * \param sinr SINR values vector
  * \return a DL CQI control message containing the CQI feedback
  */
  Ptr<cv2x_DlCqiLteControlMessage> CreateDlCqiFeedbackMessage (const SpectrumValue& sinr);



  // inherited from cv2x_LtePhy
  virtual void GenerateCtrlCqiReport (const SpectrumValue& sinr);
  virtual void GenerateDataCqiReport (const SpectrumValue& sinr);
  /**
  * \brief Create the mixed CQI report
  *
  * \param sinr SINR values vector
  */
  virtual void GenerateMixedCqiReport (const SpectrumValue& sinr);
  virtual void ReportInterference (const SpectrumValue& interf);
  /**
  * \brief Create the mixed CQI report
  *
  * \param interf interference values vector
  */
  virtual void ReportDataInterference (const SpectrumValue& interf);
  virtual void ReportRsReceivedPower (const SpectrumValue& power);

  // callbacks for cv2x_LteSpectrumPhy
  /**
  * \brief Receive LTE control message list function
  *
  * \param msgList LTE control message list
  */
  virtual void ReceiveLteControlMessageList (std::list<Ptr<cv2x_LteControlMessage> > msgList);
  /**
  * \brief Receive PSS function
  *
  * \param cellId the cell ID
  * \param p PSS list
  */
  virtual void ReceivePss (uint16_t cellId, Ptr<SpectrumValue> p);


  /**
   * \brief PhySpectrum received a new PHY-PDU
   * \param p the packet received
   */
  void PhyPduReceived (Ptr<Packet> p);

  /**
  * \brief trigger from eNB the start from a new frame
  *
  * \param frameNo frame number
  * \param subframeNo subframe number
  */
  void SubframeIndication (uint32_t frameNo, uint32_t subframeNo);


  /**
   * \brief Send the SRS signal in the last symbols of the frame
   */
  void SendSrs ();

  /**
   * \brief PhySpectrum generated a new DL HARQ feedback
   * \param mes the cv2x_DlInfoListElement_s
   */
  virtual void ReceiveLteDlHarqFeedback (cv2x_DlInfoListElement_s mes);

  /**
   * \brief Set the HARQ PHY module
   * \param harq the HARQ PHY module
   */
  void SetHarqPhyModule (Ptr<cv2x_LteHarqPhy> harq);

  /**
   * \return The current state
   */
  State GetState () const;

  /**
   * TracedCallback signature for state transition events.
   *
   * \param [in] cellId
   * \param [in] rnti
   * \param [in] oldState
   * \param [in] newState
   */
  typedef void (* StateTracedCallback)
    (uint16_t cellId, uint16_t rnti, State oldState, State newState);


  /**
   * TracedCallback signature for cell RSRP and SINR report.
   *
   * \param [in] cellId
   * \param [in] rnti
   * \param [in] rsrp
   * \param [in] sinr
   * \param [in] componentCarrierId
   */
  typedef void (* RsrpSinrTracedCallback)
    (uint16_t cellId, uint16_t rnti,
     double rsrp, double sinr, uint8_t componentCarrierId);

  /**
   * TracedCallback signature for cell RSRP and RSRQ.
   *
   * \param [in] rnti
   * \param [in] cellId
   * \param [in] rsrp
   * \param [in] rsrq
   * \param [in] isServingCell
   * \param [in] componentCarrierId
   */
  typedef void (* RsrpRsrqTracedCallback)
    (uint16_t rnti, uint16_t cellId, double rsrp, double rsrq,
     bool isServingCell, uint8_t componentCarrierId);

  /**
   * Set the time in which the first SyncRef selection will be performed by the UE
   * \param t the time to perform the first SyncRef selection (relative to the
   *          simulation time when the function is called, i.e., relative to 0, if it is called
   *          before the start of the simulation)
   * \note The UE will never perform the SyncRef selection process if this function
   * is not called before the start of the simulation
   */
  void SetFirstScanningTime(Time t);
  /**
   * Get the time in which the first SyncRef selection will be performed
   */
  Time GetFirstScanningTime();
  /**
   * Stores the S-RSRP of the detected SLSSs during the SyncRef selection  process
   * (i.e., during SyncRef scanning or measurement)
   * \param slssid the SLSSID of the SyncRef
   * \param p the received signal
   * \note The S-RSRP is not stored if: it is below the minimum required, or the UE
   *       is not performing the SyncRef selection process
   */
  virtual void ReceiveSlss (uint16_t slssid, Ptr<SpectrumValue> p); //callback for cv2x_LteSpectrumPhy

private:

  /**
   * Set transmit mode 1 gain function
   *
   * \param [in] gain
   */
  void SetTxMode1Gain (double gain);
  /**
   * Set transmit mode 2 gain function
   *
   * \param [in] gain
   */
  void SetTxMode2Gain (double gain);
  /**
   * Set transmit mode 3 gain function
   *
   * \param [in] gain
   */
  void SetTxMode3Gain (double gain);
  /**
   * Set transmit mode 4 gain function
   *
   * \param [in] gain
   */
  void SetTxMode4Gain (double gain);
  /**
   * Set transmit mode 5 gain function
   *
   * \param [in] gain
   */
  void SetTxMode5Gain (double gain);
  /**
   * Set transmit mode 6 gain function
   *
   * \param [in] gain
   */
  void SetTxMode6Gain (double gain);
  /**
   * Set transmit mode 7 gain function
   *
   * \param [in] gain
   */
  void SetTxMode7Gain (double gain);
  /**
   * Set transmit mode gain function
   *
   * \param [in] txMode
   * \param [in] gain
   */
  void SetTxModeGain (uint8_t txMode, double gain);

  /**
   * queue subchannels for transmission function
   *
   * \param [in] rbMap
   */
  void QueueSubChannelsForTransmission (std::vector <int> rbMap);


  /** 
   * internal method that takes care of generating CQI reports,
   * calculating the RSRP and RSRQ metrics, and generating RSRP+SINR traces
   * 
   * \param sinr 
   */
  void GenerateCqiRsrpRsrq (const SpectrumValue& sinr);


  /**
   * \brief Layer-1 filtering of RSRP and RSRQ measurements and reporting to
   *        the RRC entity.
   *
   * Initially executed at +0.200s, and then repeatedly executed with
   * periodicity as indicated by the *UeMeasurementsFilterPeriod* attribute.
   */
  void ReportUeMeasurements ();

  /**
   * Switch the UE PHY to the given state.
   * \param s the destination state
   */
  void SwitchToState (State s);

  // UE CPHY SAP methods
  /// Reset function
  void DoReset ();
  /**
   * Start the cell search function
   * \param dlEarfcn the DL EARFCN
   */
  void DoStartCellSearch (uint32_t dlEarfcn);
  /**
   * Synchronize with ENB function
   * \param cellId the cell ID
   */
  void DoSynchronizeWithEnb (uint16_t cellId);
  /**
   * Synchronize with ENB function
   * \param cellId the cell ID
   * \param dlEarfcn the DL EARFCN
   */
  void DoSynchronizeWithEnb (uint16_t cellId, uint32_t dlEarfcn);
  /**
   * Set DL bandwidth function
   * \param dlBandwidth the DL bandwidth
   */
  void DoSetDlBandwidth (uint8_t dlBandwidth);
  /**
   * Configure UL uplink function
   * \param ulEarfcn UL EARFCN
   * \param ulBandwidth the UL bandwidth
   */
  void DoConfigureUplink (uint32_t ulEarfcn, uint8_t ulBandwidth);
  /**
   * Configure reference signal power function
   * \param referenceSignalPower reference signal power
   */
  void DoConfigureReferenceSignalPower (int8_t referenceSignalPower);
  /**
   * Set RNTI function
   * \param rnti the RNTI
   */
  void DoSetRnti (uint16_t rnti);
  /**
   * Set transmission mode function
   * \param txMode the transmission mode
   */
  void DoSetTransmissionMode (uint8_t txMode);
  /**
   * Set SRS configuration index function
   * \param srcCi the SRS configuration index
   */
  void DoSetSrsConfigurationIndex (uint16_t srcCi);
  /**
   * Set PA function
   * \param pa the PA value
   */
  void DoSetPa (double pa);

  //discovery
  /**
   * Set sidelink tx pool function
   * \param pool Ptr<SidelinkTxDiscResourcePool>
   */
  void DoSetSlTxPool (Ptr<SidelinkTxDiscResourcePool> pool);
  /**
   * Remove sidelink tx pool function
   * \param disc bool
   */
  void DoRemoveSlTxPool (bool disc);
  /**
   * Set sidelink rx pools function
   * \param pools std::list<Ptr<SidelinkRxDiscResourcePool> >
   */
  void DoSetSlRxPools (std::list<Ptr<SidelinkRxDiscResourcePool> > pools);
  /**
   * Set discovery grant info function
   * \param resPsdch
   */
  void DoSetDiscGrantInfo (uint8_t resPsdch);
  /**
   * Add discovery tx apps function
   * \param apps std::list<uint32_t>
   */
  void DoAddDiscTxApps (std::list<uint32_t> apps);
  /**
   * Add discovery rx apps function
   * \param apps std::list<uint32_t>
   */
  void DoAddDiscRxApps (std::list<uint32_t> apps);

  //communication
  /**
   * Set sidelink tx pool function
   * \param pool Ptr<SidelinkTxCommResourcePool>
   */
  void DoSetSlTxPool (Ptr<SidelinkTxCommResourcePool> pool);
  /**
   * Remove sidelink tx pool function
   */
  void DoRemoveSlTxPool ();
  /**
   * Set sidelink rx pools function
   * \param pools std::list<Ptr<SidelinkRxCommResourcePool> >
   */
  void DoSetSlRxPools (std::list<Ptr<SidelinkRxCommResourcePool> > pools);
  /**
   * Add sidelink destination function
   * \param destination
   */
  void DoAddSlDestination (uint32_t destination);
  /**
   * Remove sidelink destination function
   * \param destination
   */
  void DoRemoveSlDestination (uint32_t destination);

  //V2X communication
  /**
   * Set sidelink V2X tx pool function
   * \param pool Ptr<SidelinkTxCommResourcePoolV2X>
   */
  void DoSetSlV2xTxPool (Ptr<SidelinkTxCommResourcePoolV2x> pool);
  
  /**
   * Remove sidelink V2X tx pool function
   */
  void DoRemoveSlV2xTxPool ();
  
  /**
   * Set sidelink V2X RX pools function
   * \param pools std::list<Ptr<SidelinkRxCommResourcePoolV2X> >
   */
  void DoSetSlV2xRxPools (std::list<Ptr<SidelinkRxCommResourcePoolV2x> > pools);

  /**
   * Return the S-RSRP value in dBm
   * \param p vector of signal perceived per each RB per sidelink packet
   */
  double GetSidelinkRsrp (std::vector <SpectrumValue> sig);

  /**
   * Return the S-RSSI value in dBm
   * \param sig vector of signal perceived per each RB per sidelink packet
   * \param interference vector of interference perceived per each RB per sidelink packet
   */
  double GetSidelinkRssi (std::vector <SpectrumValue> sig, std::vector <SpectrumValue> interference);

  // UE PHY SAP methods 
  virtual void DoSendMacPdu (Ptr<Packet> p);
  /**
   * Send LTE control message function
   * \param msg the LTE control message
   */
  virtual void DoSendLteControlMessage (Ptr<cv2x_LteControlMessage> msg);
  
  /**
   * Send RACH preamble function
   * \param prachId the RACH preamble ID
   * \param raRnti the rnti
   */
  virtual void DoSendRachPreamble (uint32_t prachId, uint32_t raRnti);


  /// A list of sub channels to use in TX.
  std::vector <int> m_subChannelsForTransmission;
  /// A list of sub channels to use in RX.
  std::vector <int> m_subChannelsForReception;

  std::vector< std::vector <int> > m_subChannelsForTransmissionQueue; ///< subchannels for transmission queue


  Ptr<cv2x_LteAmc> m_amc; ///< AMC


  /**
   * The `EnableUplinkPowerControl` attribute. If true, Uplink Power Control
   * will be enabled.
   */
  bool m_enableUplinkPowerControl;
  /// Pointer to UE Uplink Power Control entity.
  Ptr<cv2x_LteUePowerControl> m_powerControl;

  /// Wideband Periodic CQI. 2, 5, 10, 16, 20, 32, 40, 64, 80 or 160 ms.
  Time m_p10CqiPeriodicity;
  Time m_p10CqiLast; ///< last periodic CQI

  /**
   * SubBand Aperiodic CQI. Activated by DCI format 0 or Random Access Response
   * Grant.
   * \note Defines a periodicity for academic studies.
   */
  Time m_a30CqiPeriodicity;
  Time m_a30CqiLast; ///< last aperiodic CQI

  cv2x_LteUePhySapProvider* m_uePhySapProvider; ///< UE Phy SAP provider
  cv2x_LteUePhySapUser* m_uePhySapUser; ///< UE Phy SAP user

  cv2x_LteUeCphySapProvider* m_ueCphySapProvider; ///< UE CPhy SAP provider
  cv2x_LteUeCphySapUser* m_ueCphySapUser; ///< UE CPhy SAP user

  uint16_t  m_rnti; ///< the RNTI
 
  uint8_t m_transmissionMode; ///< the transmission mode
  std::vector <double> m_txModeGain; ///< the transmit mode gain

  uint16_t m_srsPeriodicity; ///< SRS periodicity
  uint16_t m_srsSubframeOffset; ///< SRS subframe offset
  uint16_t m_srsConfigured; ///< SRS configured
  Time     m_srsStartTime; ///< SRS start time

  double m_paLinear; ///< PA linear

  bool m_dlConfigured; ///< DL configured?
  bool m_ulConfigured; ///< UL configured?

  /// The current UE PHY state.
  State m_state;
  /**
   * The `StateTransition` trace source. Fired upon every UE PHY state
   * transition. Exporting the serving cell ID, RNTI, old state, and new state.
   */
  TracedCallback<uint16_t, uint16_t, State, State> m_stateTransitionTrace;

  /**
   * The `DiscoveryMsgSent` trace source. Track the transmission of discovery message (announce)
   * Exporting cellId, RNTI, ProSe App Code.
   */
  TracedCallback<uint16_t, uint16_t, uint32_t> m_discoveryAnnouncementTrace;

  /**
   * The `SidelinkV2xMsgSent` trace source. Track the transmission of v2x message (announce)
   */
  TracedCallback<> m_sidelinkV2xAnnouncementTrace;


  /// \todo Can be removed.
  uint8_t m_subframeNo;

  bool m_rsReceivedPowerUpdated; ///< RS receive power updated?
  SpectrumValue m_rsReceivedPower; ///< RS receive power

  bool m_rsInterferencePowerUpdated; ///< RS interference power updated?
  SpectrumValue m_rsInterferencePower; ///< RS interference power

  bool m_dataInterferencePowerUpdated; ///< data interference power updated?
  SpectrumValue m_dataInterferencePower; ///< data interference power

  bool m_v2xEnabled; 

  bool m_pssReceived; ///< PSS received?
  /// PssElement structure
  struct PssElement 
  {
    uint16_t cellId; ///< cell ID
    double pssPsdSum; ///< PSS PSD sum
    uint16_t nRB; ///< number of RB
  };
  std::list <PssElement> m_pssList; ///< PSS list

  /**
   * The `RsrqUeMeasThreshold` attribute. Receive threshold for PSS on RSRQ
   * in dB.
   */
  double m_pssReceptionThreshold;

  /**
   * The `RsrpUeMeasThreshold` attribute. Receive threshold for RSRP
   * in dB.
   */
  double m_rsrpReceptionThreshold;

  /// Summary results of measuring a specific cell. Used for layer-1 filtering.
  struct UeMeasurementsElement
  {
    double rsrpSum;   ///< Sum of RSRP sample values in linear unit.
    uint8_t rsrpNum;  ///< Number of RSRP samples.
    double rsrqSum;   ///< Sum of RSRQ sample values in linear unit.
    uint8_t rsrqNum;  ///< Number of RSRQ samples.
  };

  /**
   * Store measurement results during the last layer-1 filtering period.
   * Indexed by the cell ID where the measurements come from.
   */
  std::map <uint16_t, UeMeasurementsElement> m_ueMeasurementsMap;
  /**
   * The `UeMeasurementsFilterPeriod` attribute. Time period for reporting UE
   * measurements, i.e., the length of layer-1 filtering (default 200 ms).
   */
  Time m_ueMeasurementsFilterPeriod;
  /// \todo Can be removed.
  Time m_ueMeasurementsFilterLast;

  Ptr<cv2x_LteHarqPhy> m_harqPhyModule; ///< HARQ phy module

  uint32_t m_raPreambleId; ///< RA preamble ID
  uint32_t m_raRnti; ///< RA rnti

  /**
   * The `ReportCurrentCellRsrpSinr` trace source. Trace information regarding
   * RSRP and average SINR (see TS 36.214). Exporting cell ID, RNTI, RSRP, and
   * SINR. Moreover it reports the m_componentCarrierId.
   */
  TracedCallback<uint16_t, uint16_t, double, double, uint8_t> m_reportCurrentCellRsrpSinrTrace;
  /**
   * The `RsrpSinrSamplePeriod` attribute. The sampling period for reporting
   * RSRP-SINR stats.
   */
  uint16_t m_rsrpSinrSamplePeriod;
  /**
   * The `RsrpSinrSampleCounter` attribute. The sampling counter for reporting
   * RSRP-SINR stats.
   */
  uint16_t m_rsrpSinrSampleCounter;

  /**
   * The `ReportUeMeasurements` trace source. Contains trace information
   * regarding RSRP and RSRQ measured from a specific cell (see TS 36.214).
   * Exporting RNTI, the ID of the measured cell, RSRP (in dBm), RSRQ (in dB),
   * and whether the cell is the serving cell. Moreover it report the m_componentCarrierId.
   */
  TracedCallback<uint16_t, uint16_t, double, double, bool, uint8_t> m_reportUeMeasurements;

  EventId m_sendSrsEvent; ///< send SRS event

  /**
   * The `UlPhyTransmission` trace source. Contains trace information regarding
   * PHY stats from UL Tx perspective. Exporting a structure with type
   * cv2x_PhyTransmissionStatParameters.
   */
  TracedCallback<cv2x_PhyTransmissionStatParameters> m_ulPhyTransmission;

  
  Ptr<SpectrumValue> m_noisePsd; ///< Noise power spectral density for
                                 ///the configured bandwidth

  /**
   * The sidelink cv2x_LteSpectrumPhy associated to this cv2x_LteUePhy. 
   */
  Ptr<cv2x_LteSpectrumPhy> m_sidelinkSpectrumPhy;

  Ptr<SpectrumValue> m_slNoisePsd; ///< Noise power spectral density for
                                 ///the configured bandwidth 


  struct SidelinkGrant
  {
    //fields common with SL_DCI
    uint16_t m_rnti;
    uint16_t m_resPscch;
    uint8_t m_tpc;
    uint8_t m_hopping;
    uint8_t m_rbStart; //models rb assignment
    uint8_t m_rbLen;   //models rb assignment
    uint8_t m_trp;
    uint8_t m_groupDstId;

    //other fields
    uint8_t m_mcs;
    uint32_t m_tbSize;

    uint32_t frameNo;
    uint32_t subframeNo;
  };

  struct SidelinkGrantInfo
  {
    SidelinkGrant m_grant;
    std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> m_pscchTx; //list of PSCCH transmissions within the pool
    std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo> m_psschTx; //list of PSSCH transmissions within the pool
    bool m_grant_received;
  };
  
  struct PoolInfo
  {
    Ptr<SidelinkCommResourcePool> m_pool; //the pool
    SidelinkCommResourcePool::SubframeInfo m_currentScPeriod; //start of current period
    SidelinkCommResourcePool::SubframeInfo m_nextScPeriod; //start of next period

    uint32_t m_npscch; // number of PSCCH available in the pool

    //SidelinkGrant m_currentGrant; //grant for the current SC period
    std::map<uint16_t, SidelinkGrantInfo> m_currentGrants;
  };

  PoolInfo m_slTxPoolInfo;
  std::list <PoolInfo> m_sidelinkRxPools;
  std::list <uint32_t> m_destinations;
  
// V2X communication
  struct SidelinkGrantV2x {
    uint16_t m_rnti; 
    uint8_t m_prio;
    uint16_t m_pRsvp;
    uint16_t m_riv; 
    uint8_t m_sfGap;
    uint8_t m_mcs;
    uint8_t m_reTxIdx;
    uint32_t m_tbSize; 

    uint8_t m_resPscch;  // added for modelling 
    uint32_t frameNo; 
    uint32_t subframeNo;
  };

  struct SidelinkGrantInfoV2x {
    SidelinkGrantV2x m_grant;
    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> m_pscchTx;
    std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> m_psschTx; 
    bool m_grant_received;
  };

  struct PoolInfoV2x {
    Ptr<SidelinkCommResourcePoolV2x> m_pool; // the pool
    SidelinkCommResourcePoolV2x::SubframeInfo m_currentFrameInfo; // current frame/subframe
    std::map<uint16_t, SidelinkGrantInfoV2x> m_currentGrants; 
  };

  PoolInfoV2x m_sidelinkTxPoolsV2x;
  PoolInfoV2x m_slTxPoolInfoV2x;
  std::list <PoolInfoV2x> m_sidelinkRxPoolsV2x;

  //discovery
  struct DiscGrant
  {
    uint16_t m_rnti;
    uint8_t m_resPsdch;
  };

  struct DiscGrantInfo
  {
    DiscGrant m_grant;
    std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo> m_psdchTx;//list of PSDCH transmissions within the pool
    bool m_grant_received;
  };

  struct DiscPoolInfo
  {
    Ptr<SidelinkDiscResourcePool> m_pool; //the pool
    SidelinkDiscResourcePool::SubframeInfo m_currentDiscPeriod; //start of current period 
    SidelinkDiscResourcePool::SubframeInfo m_nextDiscPeriod; //start of next period
    uint32_t m_npsdch; // number of PSDCH available in the pool
    std::map<uint16_t, DiscGrantInfo> m_currentGrants;
  };

  DiscPoolInfo m_discTxPools;
  std::list <DiscPoolInfo> m_discRxPools;
  
  uint8_t m_discResPsdch ;

  std::list<uint32_t> m_discTxApps;
  std::list<uint32_t> m_discRxApps;

  /**
   * Summary results of measuring a specific SyncRef. Used for layer-1 filtering.
   */
  struct UeSlssMeasurementsElement
  {
    double srsrpSum;   ///< Sum of S-RSRP sample values in linear unit.
    uint8_t srsrpNum;  ///< Number of S-RSRP samples.
  };

  /**
   * 
   */
  struct UeSrsrpMeasurementsElement
  {
    double srsrpSum;   ///< Sum of S-RSRP sample values in linear unit.
    uint8_t srsrpNum;  ///< Number of S-RSRP samples.
  };

  /**
   * Stores the S-RSRP information of the SyncRefs detected during the scanning process,
   * indexed by the SLSSID of the SyncRef and the offset it uses for transmitting the SLSSs
   */
  std::map <std::pair<uint16_t,uint16_t>, UeSlssMeasurementsElement> m_ueSlssDetectionMap;
  /**
   * 
   */ 
  std::map <std::pair<uint16_t, uint16_t>, UeSrsrpMeasurementsElement> m_ueSrsrpDetectionMap; 
  /**
   * Represents the S-RSRP measurement schedule for the current measurement process.
   * It is used for knowing when the UE needs to take samples of the detected SyncRefs S-RSRP.
   * The index is the simulation time and the elements are the SyncRef identifiers (SLSSID and offset)
   */
  std::map <int64_t, std::pair<uint16_t,uint16_t> > m_ueSlssMeasurementsSched;
  /**
   * Stores the S-RSRP information of the SyncRefs in measurement during the measurement process
   * indexed by the SLSSID of the SyncRef and the offset it uses for transmitting the SLSSs
   */
  std::map <std::pair<uint16_t,uint16_t>, UeSlssMeasurementsElement> m_ueSlssMeasurementsMap;
  /**
   * Time period for searching/scanning to detect available SyncRefs (supporting SyncRef selection)
   */
  Time m_ueSlssScanningPeriod;
  /**
   * Time period for taking S-RSRP samples of the detected SyncRefs (supporting SyncRef selection)
   */
  Time m_ueSlssMeasurementPeriod;
  /**
   * Time period for taking S-RSRP samples of the selected SyncRef (supporting SLSS transmission decision)
   */
  Time m_ueSlssEvaluationPeriod; //How long the UE will evaluate the selected SyncRef for determine cease/initiation of SLSS tx
  /**
   *  The number of samples the UE takes during the measurement and evaluation periods
   */
  uint16_t m_nSamplesSrsrpMeas;
  /**
   * Time for the first SyncRef selection period
   */
  Time m_tFirstScanning;
  /**
   * Random number generator used for determining the time between SyncRef selection processes
   */
  Ptr<UniformRandomVariable> m_nextScanRdm;
  /**
   * True if a SyncRef selection is in progress and the UE is performing the SyncRef search/scanning
   */
  bool m_ueSlssScanningInProgress;
  /**
   * True if a SyncRef selection is in progress and the UE is performing the S-RSRP measurements,
   * either of the detected SyncRefs (measurement sub-process) or the selected one (evaluation sub-process)
   */
  bool m_ueSlssMeasurementInProgress;
  /**
   * Number of S-RSRP measurement periods already performed in SyncRef selection process in progress
   * (1 = measurement, 2 = measurement + evaluation)
   */
  uint16_t m_currNMeasPeriods;
  /**
   * The minimum S-RSRP value for considering a SyncRef S-RSRP sample valid
   */
  double m_minSrsrp;
  /**
   * Stores the received MIB-SL during the SyncRef search/scanning.
   * Used to determine the detected SyncRefs, as SyncRefs with valid S-RSRP but without
   * successfully decoded/received MIB-SL are not considered detected
   */
  std::map <std::pair<uint16_t,uint16_t>, cv2x_LteRrcSap::MasterInformationBlockSL> m_detectedMibSl;
  /**
   * Initial frame number
   */
  uint32_t m_initFrameNo;
  /**
   * Initial subframe number
   */
  uint32_t m_initSubframeNo;
  /**
   * Current frame number
   */
  uint32_t m_currFrameNo;
  /**
   * Current subframe number
   */
  uint32_t m_currSubframeNo;
  /**
   * Configuration needed for the timely change of subframe indication upon synchronization to
   * a different SyncRef
   */
  struct ResyncParams
  {
    uint16_t newSubframeNo;
    uint16_t newFrameNo;
    cv2x_LteRrcSap::MasterInformationBlockSL syncRefMib;
    uint16_t offset;
  };
  /**
   * Parameters to be used for the change of subframe indication upon synchronization to
   * a different SyncRef
   */
  ResyncParams m_resyncParams;
  /**
   * True if the RRC instructed to synchronize to a different SyncRef
   */
  bool m_resyncRequested;
  /**
   * True if the UE changed of timing (synchronized to a different SyncRef) and have to
   * wait the start of a new sideliink communication period for transmitting the data
   * (the subframe indication changed and the data indication in the SCI of the current
   * period is not valid anymore)
   */
  bool m_waitingNextScPeriod;

  /**
   * Schedules the first call of the function SubframeIndication with the appropriate
   * values for its parameters: frameNo and subframeNo
   * \param rdm when true the values of frameNo and subframeNo are selected randomly,
   *            when false, frameNo=1 and subframeNo=1
   */
  void SetInitialSubFrameIndication(bool rdm);
  /**
   * Set the upper limit for the random values generated by m_nextScanRdm
   * \param t the upper limit for m_nextScanRdm
   */
  void SetUeSlssInterScanningPeriodMax(Time t);
  /**
   * Set the lower limit for the random values generated by m_nextScanRdm
   * \param the lower limit for m_nextScanRdm
   */
  void SetUeSlssInterScanningPeriodMin(Time t);
  /**
   * Notify the start of a new SyncRef selection process, starting with the
   * SyncRef search/scanning
   */
  void StartSlssScanning ();
  /**
   * Notify the end of the SyncRef search/scanning,
   * keep only the six detected SyncRef (with received MIB-SL) with highest S-RSRP,
   * create the S-RSRP measurement schedule for each of them and start the measurement
   * process
   */
  void EndSlssScanning ();

  /**
   * Notify the start of a S-RSRP measurement process.
   * The S-RSRP measurement is used for two sub-processes:
   * 1. Measurement: collect the S-RSRP samples of the detected SyncRefs for determining
   *    the suitable SyncRef to select and synchronize with, and
   * 2. Evaluation: collect the S-RSRP of the selected SyncRef (if any) to determine if the UE needs
   *    to become itself a SyncRef and start transmitting SLSS
   * \param slssid the SLSSID of the selected SyncRef if the function is called for Evaluation,
   *               0 if it is called for Measurement
   * \param offset the offset in which the selected SyncRef sends SLSS if the function is called
   *               for Evaluation, 0 if it is called for Measurement
   */
  void StartSlssMeasurements (uint64_t slssid, uint16_t offset);
  /**
   * Perform L1 filtering of the S-RSRP samples collected during the measurement process
   * for each SyncRef, and report them to the RRC
   * \param slssid the SLSSID of the selected SyncRef if the measurement process was called for
   *               Evaluation, 0 if was is called for Measurement
   * \param offset the offset in which the selected SyncRef sends SLSS if the process was called
   *               for Evaluation, 0 if it was called for Measurement
   */
  void ReportSlssMeasurements (uint64_t slssid,uint16_t offset);
  /**
   * Schedule the next SyncRef selection process.
   * The function is called at the end of the SyncRef selection process in progress
   * \param endOfPrevious indicates after which sub-process the SyncRef selection process ended:
   *                      0 if it ended after scanning, 1 if it ended after measurement,
   *                      or 2 if it ends after evaluation
   */
  void ScheduleNextSyncRefReselection(uint16_t endOfPrevious);
  /**
   * Apply the change of timing (change of frame/subframe indication) when appropriate.
   * The change of timing is instructed when the UE selected and wants to synchronize to a given
   * SyncRef. The change is applied immediately upon resynchronization request if the UE is
   * not transmitting sidelink communication at the moment. Otherwise, the change is delayed until
   * the end of the current sidelink communication period to avoid the loss of already scheduled
   * transmissions (the subframe indication will change, and the data indication in the SCI
   * of the current period will not be valid anymore).
   * \param frameNo the current frame number
   * \param subframeNo the current subframe number
   */
  bool ChangeOfTiming(uint32_t frameNo, uint32_t subframeNo);

  // UE CPHY SAP methods related to synchronization
  // The RRC set the SLSSID value for lower layers
  void DoSetSlssId(uint64_t slssid);
  // The RRC instructs the PHY to send a MIB-SL in the PSBCH
  void DoSendSlss (cv2x_LteRrcSap::MasterInformationBlockSL mibSl);
  // The RRC instructs the PHY to synchronize to a given SyncRef and apply the corresponding change of timing
  void DoSynchronizeToSyncRef (cv2x_LteRrcSap::MasterInformationBlockSL mibSl);

}; // end of `class cv2x_LteUePhy`


}

#endif /* CV2X_LTE_UE_PHY_H */

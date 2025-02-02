/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 CTTC
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
 *          Giuseppe Piro  <g.piro@poliba.it>
 * Modified by:
 *          Marco Miozzo <mmiozzo@cttc.es> (introduce physical error model)
 *          NIST (D2D)
 *          Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *          Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#ifndef CV2X_LTE_SPECTRUM_PHY_H
#define CV2X_LTE_SPECTRUM_PHY_H

#include <ns3/event-id.h>
#include <ns3/spectrum-value.h>
#include <ns3/mobility-model.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include <ns3/net-device.h>
#include <ns3/spectrum-phy.h>
#include <ns3/spectrum-channel.h>
#include <ns3/spectrum-interference.h>
#include <ns3/data-rate.h>
#include <ns3/generic-phy.h>
#include <ns3/packet-burst.h>
#include <ns3/cv2x_lte-interference.h>
#include <ns3/cv2x_lte-sl-interference.h>
#include <ns3/cv2x_lte-phy-error-model.h>
#include "ns3/random-variable-stream.h"
#include <map>
#include <ns3/cv2x_ff-mac-common.h>
#include <ns3/cv2x_lte-harq-phy.h>
#include <ns3/cv2x_lte-common.h>
#include <ns3/cv2x_sl-pool.h>

namespace ns3 {

/// cv2x_TbId_t structure
struct cv2x_TbId_t
{
  uint16_t m_rnti; ///< RNTI
  uint8_t m_layer; ///< layer
  
  public:
  cv2x_TbId_t ();
  /**
   * Constructor
   *
   * \param a rnti
   * \param b layer
   */
  cv2x_TbId_t (const uint16_t a, const uint8_t b);
  
  friend bool operator == (const cv2x_TbId_t &a, const cv2x_TbId_t &b);
  friend bool operator < (const cv2x_TbId_t &a, const cv2x_TbId_t &b);
};

  
/// cv2x_tbInfo_t structure
struct cv2x_tbInfo_t
{
  uint8_t ndi; ///< ndi
  uint16_t size; ///< size
  uint8_t mcs; ///< mcs
  std::vector<int> rbBitmap; ///< rb bitmap
  uint8_t harqProcessId; ///< HARQ process id
  uint8_t rv; ///< rv
  double mi; ///< mi
  bool downlink; ///< whether is downlink
  bool corrupt; ///< whether is corrupt
  bool harqFeedbackSent; ///< is HARQ feedback sent
  double sinr; ///< mean SINR
};

typedef std::map<cv2x_TbId_t, cv2x_tbInfo_t> cv2x_expectedTbs_t; ///< expectedTbs_t typedef

struct cv2x_SlTbId_t
{
  uint16_t m_rnti; //source SL-RNTI
  uint8_t m_l1dst; //layer 1 group Id
  
  public:
  cv2x_SlTbId_t ();
  cv2x_SlTbId_t (const uint16_t a, const uint8_t b);
  
  friend bool operator == (const cv2x_SlTbId_t &a, const cv2x_SlTbId_t &b);
  friend bool operator < (const cv2x_SlTbId_t &a, const cv2x_SlTbId_t &b);
};
 
struct cv2x_SltbInfo_t
{
  uint8_t ndi; ///< ndi
  uint16_t size; ///< size
  uint8_t mcs; ///< mcs
  std::vector<int> rbBitmap; ///< rb bitmap
  // uint8_t harqProcessId; ///< HARQ process id
  uint8_t rv; ///< rv
  double mi; ///< mi
  // bool downlink; ///< whether is downlink
  bool corrupt; ///< whether is corrupt
  bool harqFeedbackSent; ///< is HARQ feedback sent
  double sinr; ///< mean SINR
};

typedef std::map<cv2x_SlTbId_t, cv2x_SltbInfo_t> expectedSlTbs_t;

struct cv2x_SlV2xTbId_t
{
  uint16_t m_rnti; //source SL-RNTI
  
  public:
  cv2x_SlV2xTbId_t ();
  cv2x_SlV2xTbId_t (const uint16_t a);
  
  friend bool operator == (const cv2x_SlV2xTbId_t &a, const cv2x_SlV2xTbId_t &b);
  friend bool operator < (const cv2x_SlV2xTbId_t &a, const cv2x_SlV2xTbId_t &b);
};

struct cv2x_SlV2xTbInfo_t{
  uint32_t size; ///< size
  uint8_t mcs; ///< mcs
  std::vector<int> rbBitmap; ///< rb bitmap
  double mi; ///< mi
  bool corrupt; ///< whether is corrupt
  bool harqFeedbackSent; ///< is HARQ feedback sent
  double sinr; ///< mean SINR
};

typedef std::map<cv2x_SlV2xTbId_t, cv2x_SlV2xTbInfo_t> expectedSlV2xTbs_t; 


struct cv2x_DiscTbId_t
{
  uint16_t m_rnti; //source SL-RNTI
  uint8_t m_resPsdch; 
  
  public:
  cv2x_DiscTbId_t ();
  cv2x_DiscTbId_t (const uint16_t a, const uint8_t b);
  
  friend bool operator == (const cv2x_DiscTbId_t &a, const cv2x_DiscTbId_t &b);
  friend bool operator < (const cv2x_DiscTbId_t &a, const cv2x_DiscTbId_t &b);
  
};

struct cv2x_DisctbInfo_t
{
  uint8_t ndi; ///< ndi
  uint8_t resPsdch;
  std::vector<int> rbBitmap;///< rb bitmap
  uint8_t rv; ///< rv
  double mi; ///< mi
  bool corrupt; ///< whether is corrupt
  bool harqFeedbackSent; ///< is HARQ feedback sent
  double sinr; //mean SINR
};

typedef std::map<cv2x_DiscTbId_t, cv2x_DisctbInfo_t> expectedDiscTbs_t;

class cv2x_LteNetDevice;
class AntennaModel;
class cv2x_LteControlMessage;
struct cv2x_LteSpectrumSignalParametersDataFrame;
struct cv2x_LteSpectrumSignalParametersDlCtrlFrame;
struct cv2x_LteSpectrumSignalParametersUlSrsFrame;
struct cv2x_LteSpectrumSignalParametersSlFrame;

/**
 * Structure for sidelink packets being received
 */
struct cv2x_SlRxPacketInfo_t
{
  std::vector<int> rbBitmap;
  Ptr<PacketBurst> m_rxPacketBurst;
  Ptr<cv2x_LteControlMessage> m_rxControlMessage;
};

struct cv2x_SlCtrlPacketInfo_t
{
  double sinr;
  int index;

  friend bool operator == (const cv2x_SlCtrlPacketInfo_t &a, const cv2x_SlCtrlPacketInfo_t &b);
  friend bool operator < (const cv2x_SlCtrlPacketInfo_t &a, const cv2x_SlCtrlPacketInfo_t &b);
};

/**
* This method is used by the cv2x_LteSpectrumPhy to notify the PHY that a
* previously started RX attempt has terminated without success
*/
typedef Callback< void > cv2x_LtePhyRxDataEndErrorCallback;

/**
* This method is used by the cv2x_LteSpectrumPhy to notify the PHY that a
* previously started RX attempt has been successfully completed.
*
* @param packet the received Packet
*/
typedef Callback< void, Ptr<Packet> > cv2x_LtePhyRxDataEndOkCallback;


/**
* This method is used by the cv2x_LteSpectrumPhy to notify the PHY that a
* previously started RX of a control frame attempt has been 
* successfully completed.
*
* @param packet the received Packet
*/
typedef Callback< void, std::list<Ptr<cv2x_LteControlMessage> > > cv2x_LtePhyRxCtrlEndOkCallback;

/**
* This method is used by the cv2x_LteSpectrumPhy to notify the PHY that a
* previously started RX of a control frame attempt has terminated 
* without success.
*/
typedef Callback< void > cv2x_LtePhyRxCtrlEndErrorCallback;

/**
* This method is used by the cv2x_LteSpectrumPhy to notify the UE PHY that a
* PSS has been received
*/
typedef Callback< void, uint16_t, Ptr<SpectrumValue> > cv2x_LtePhyRxPssCallback;

/**
* This method is used by the cv2x_LteSpectrumPhy to notify the UE PHY that a
* SLSS has been received
*/
typedef Callback< void, uint16_t, Ptr<SpectrumValue> > cv2x_LtePhyRxSlssCallback;

/**
* This method is used by the cv2x_LteSpectrumPhy to notify the PHY about
* the status of a certain DL HARQ process
*/
typedef Callback< void, cv2x_DlInfoListElement_s > cv2x_LtePhyDlHarqFeedbackCallback;

/**
* This method is used by the cv2x_LteSpectrumPhy to notify the PHY about
* the status of a certain UL HARQ process
*/
typedef Callback< void, cv2x_UlInfoListElement_s > cv2x_LtePhyUlHarqFeedbackCallback;



/**
 * \ingroup lte
 * \class cv2x_LteSpectrumPhy
 *
 * The cv2x_LteSpectrumPhy models the physical layer of LTE
 *
 * It supports a single antenna model instance which is
 * used for both transmission and reception.  
 */
class cv2x_LteSpectrumPhy : public SpectrumPhy
{

public:
  cv2x_LteSpectrumPhy ();
  virtual ~cv2x_LteSpectrumPhy ();

  /**
   *  PHY states
   */
  enum State
  {
    IDLE, TX_DL_CTRL, TX_DATA, TX_UL_V2X_SCI, TX_UL_SRS, RX_DL_CTRL, RX_DATA, RX_UL_SRS
  };

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  // inherited from Object
  virtual void DoDispose ();

  Ptr<SpectrumValue> GetTxPowerSpectralDensity () {return m_txPsd;}

  // inherited from SpectrumPhy
  void SetChannel (Ptr<SpectrumChannel> c);
  void SetMobility (Ptr<MobilityModel> m);
  void SetDevice (Ptr<NetDevice> d);
  Ptr<MobilityModel> GetMobility () const;
  Ptr<NetDevice> GetDevice () const;
  Ptr<const SpectrumModel> GetRxSpectrumModel () const;
  Ptr<Object> GetAntenna () const;
  void StartRx (Ptr<SpectrumSignalParameters> params);
  /**
   * \brief Start receive data function
   * \param params Ptr<cv2x_LteSpectrumSignalParametersDataFrame>
   */
  void StartRxData (Ptr<cv2x_LteSpectrumSignalParametersDataFrame> params);
  /**
   * \brief Start receive DL control function
   * \param lteDlCtrlRxParams Ptr<cv2x_LteSpectrumSignalParametersDlCtrlFrame>
   */
  void StartRxDlCtrl (Ptr<cv2x_LteSpectrumSignalParametersDlCtrlFrame> lteDlCtrlRxParams);
  /**
   * \brief Start receive SL data function
   * \param params Ptr<cv2x_LteSpectrumSignalParametersSlFrame>
   */
  void StartRxSlData (Ptr<cv2x_LteSpectrumSignalParametersSlFrame> params);
  /**
   * \brief Start receive UL SRS function
   * \param lteUlSrsRxParams Ptr<cv2x_LteSpectrumSignalParametersUlSrsFrame>
   */
  void StartRxUlSrs (Ptr<cv2x_LteSpectrumSignalParametersUlSrsFrame> lteUlSrsRxParams);
  /**
   * \brief Set HARQ phy function
   * \param harq the HARQ phy module
   */
  void SetHarqPhyModule (Ptr<cv2x_LteHarqPhy> harq);

  /**
   * set the Power Spectral Density of outgoing signals in W/Hz.
   *
   * @param txPsd
   */
  void SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd);

  /**
   * \brief set the noise power spectral density
   * @param noisePsd the Noise Power Spectral Density in power units
   * (Watt, Pascal...) per Hz.
   */
  void SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd);

  /** 
   * reset the internal state
   * 
   */
  void Reset ();
  
  /**
   * Clear expected SL TBs
   *
   */
  void ClearExpectedSlTb ();

  /**
   * Clear expected SL TBs
   *
   */
  void ClearExpectedSlV2xTb ();

  /**
   * set the AntennaModel to be used
   * 
   * \param a the Antenna Model
   */
  void SetAntenna (Ptr<AntennaModel> a);
  
  /**
  * Start a transmission of data frame in DL and UL
  *
  *
  * @param pb the burst of packets to be transmitted in PDSCH/PUSCH
  * @param ctrlMsgList the list of cv2x_LteControlMessage to send
  * @param duration the duration of the data frame 
  *
  * @return true if an error occurred and the transmission was not
  * started, false otherwise.
  */
  bool StartTxDataFrame (Ptr<PacketBurst> pb, std::list<Ptr<cv2x_LteControlMessage> > ctrlMsgList, Time duration);

  /**
  * Start a transmission of data frame in DL and UL
  *
  *
  * @param pb the burst of packets to be transmitted in PDSCH/PUSCH
  * @param ctrlMsgList the list of cv2x_LteControlMessage to send
  * @param duration the duration of the data frame 
  *
  * @return true if an error occurred and the transmission was not
  * started, false otherwise.
  */
  bool StartTxSlDataFrame (Ptr<PacketBurst> pb, std::list<Ptr<cv2x_LteControlMessage> > ctrlMsgList, Time duration, uint8_t groupId);

  /**
  * Start a transmission of control frame in DL
  *
  *
  * @param ctrlMsgList the burst of control messages to be transmitted
  * @param pss the flag for transmitting the primary synchronization signal
  *
  * @return true if an error occurred and the transmission was not
  * started, false otherwise.
  */
  bool StartTxDlCtrlFrame (std::list<Ptr<cv2x_LteControlMessage> > ctrlMsgList, bool pss);
  
  
  /**
  * Start a transmission of control frame in UL
  *
  * @return true if an error occurred and the transmission was not
  * started, false otherwise.
  */
  bool StartTxUlSrsFrame ();

  /**
   * set the callback for the end of a RX in error, as part of the
   * interconnections between the PHY and the MAC
   *
   * @param c the callback
   */
  void SetLtePhyRxDataEndErrorCallback (cv2x_LtePhyRxDataEndErrorCallback c);

  /**
   * set the callback for the successful end of a RX, as part of the
   * interconnections between the PHY and the MAC
   *
   * @param c the callback
   */
  void SetLtePhyRxDataEndOkCallback (cv2x_LtePhyRxDataEndOkCallback c);
  
  /**
  * set the callback for the successful end of a RX ctrl frame, as part 
  * of the interconnections between the cv2x_LteSpectrumPhy and the PHY
  *
  * @param c the callback
  */
  void SetLtePhyRxCtrlEndOkCallback (cv2x_LtePhyRxCtrlEndOkCallback c);
  
  /**
  * set the callback for the erroneous end of a RX ctrl frame, as part 
  * of the interconnections between the cv2x_LteSpectrumPhy and the PHY
  *
  * @param c the callback
  */
  void SetLtePhyRxCtrlEndErrorCallback (cv2x_LtePhyRxCtrlEndErrorCallback c);

  /**
  * set the callback for the reception of the PSS as part
  * of the interconnections between the cv2x_LteSpectrumPhy and the UE PHY
  *
  * @param c the callback
  */
  void SetLtePhyRxPssCallback (cv2x_LtePhyRxPssCallback c);

  /**
  * set the callback for the DL HARQ feedback as part of the 
  * interconnections between the cv2x_LteSpectrumPhy and the PHY
  *
  * @param c the callback
  */
  void SetLtePhyDlHarqFeedbackCallback (cv2x_LtePhyDlHarqFeedbackCallback c);

  /**
  * set the callback for the UL HARQ feedback as part of the
  * interconnections between the cv2x_LteSpectrumPhy and the PHY
  *
  * @param c the callback
  */
  void SetLtePhyUlHarqFeedbackCallback (cv2x_LtePhyUlHarqFeedbackCallback c);

  /**
   * \brief Set the state of the phy layer
   * \param newState the state
   */
  void SetState (State newState);

  /** 
   * 
   * 
   * \param cellId the Cell Identifier
   */
  void SetCellId (uint16_t cellId);

  /**
   *
   * \param componentCarrierId the component carrier id
   */
  void SetComponentCarrierId (uint8_t componentCarrierId);

  /** 
   * \brief Add a new L1 group for filtering
   * 
   * \param groupId the L1 Group Identifier
   */
  void AddL1GroupId (uint8_t groupId);

  /** 
   * \brief Remove a new L1 group for filtering
   * 
   * \param groupId the L1 Group Identifier
   */
  void RemoveL1GroupId (uint8_t groupId);

  /**
  *
  *
  * \param p the new cv2x_LteChunkProcessor to be added to the RS power
  *          processing chain
  */
  void AddRsPowerChunkProcessor (Ptr<cv2x_LteChunkProcessor> p);
  
  /**
  *
  *
  * \param p the new cv2x_LteChunkProcessor to be added to the Data Channel power
  *          processing chain
  */
  void AddDataPowerChunkProcessor (Ptr<cv2x_LteChunkProcessor> p);

  /** 
  * 
  * 
  * \param p the new cv2x_LteChunkProcessor to be added to the data processing chain
  */
  void AddDataSinrChunkProcessor (Ptr<cv2x_LteChunkProcessor> p);

  /**
  *  cv2x_LteChunkProcessor devoted to evaluate interference + noise power
  *  in control symbols of the subframe
  *
  * \param p the new cv2x_LteChunkProcessor to be added to the data processing chain
  */
  void AddInterferenceCtrlChunkProcessor (Ptr<cv2x_LteChunkProcessor> p);

  /**
  *  cv2x_LteChunkProcessor devoted to evaluate interference + noise power
  *  in data symbols of the subframe
  *
  * \param p the new cv2x_LteChunkProcessor to be added to the data processing chain
  */
  void AddInterferenceDataChunkProcessor (Ptr<cv2x_LteChunkProcessor> p);
  
  /** 
  * 
  * 
  * \param p the new cv2x_LteChunkProcessor to be added to the ctrl processing chain
  */
  void AddCtrlSinrChunkProcessor (Ptr<cv2x_LteChunkProcessor> p);

  /** 
  * 
  * 
  * \param p the new cv2x_LteSlChunkProcessor to be added to the sidelink processing chain
  */
  void AddSlSinrChunkProcessor (Ptr<cv2x_LteSlChunkProcessor> p);

  /** 
  * 
  * 
  * \param p the new cv2x_LteSlChunkProcessor to be added to the sidelink processing chain
  */
  void AddSlSignalChunkProcessor (Ptr<cv2x_LteSlChunkProcessor> p);
 
  /** 
  * 
  * 
  * \param p the new cv2x_LteSlChunkProcessor to be added to the sidelink processing chain
  */
  void AddSlInterferenceChunkProcessor (Ptr<cv2x_LteSlChunkProcessor> p);
   
  
  /** 
  * 
  * 
  * \param rnti the rnti of the source of the TB
  * \param ndi new data indicator flag
  * \param size the size of the TB
  * \param mcs the MCS of the TB
  * \param map the map of RB(s) used
  * \param layer the layer (in case of MIMO tx)
  * \param harqId the id of the HARQ process (valid only for DL)
  * \param rv the rv
  * \param downlink true when the TB is for DL
  */
  void AddExpectedTb (uint16_t  rnti, uint8_t ndi, uint16_t size, uint8_t mcs, std::vector<int> map, uint8_t layer, uint8_t harqId, uint8_t rv, bool downlink);

  /** 
  * 
  * 
  * \param rnti the rnti of the source of the TB
  * \param l1dst the layer 1 destination id of the TB
  * \param ndi new data indicator flag
  * \param size the size of the TB
  * \param mcs the MCS of the TB
  * \param map the map of RB(s) used
  * \param rv the rv
  */
  void AddExpectedTb (uint16_t  rnti, uint8_t l1dst, uint8_t ndi, uint16_t size, uint8_t mcs, std::vector<int> map, uint8_t rv);

  /** 
  * 
  * for v2x communication
  * \param rnti the rnti of the source of the TB
  * \param l1dst the layer 1 destination id of the TB
  * \param size the size of the TB
  * \param mcs the MCS of the TB
  * \param map the map of RB(s) used
  */
  void AddExpectedTbV2x (uint16_t rnti, uint16_t size, uint8_t mcs, std::vector<int> map);

  /**
   * for discovery
   * no mcs, size fixe to 232, no l1dst
   * \param rnti the rnti of the source of the TB
   * \param resPsdch indetifier
   * \param ndi new data indicator flag
   * \param map map of RBs used
   * \param rv revision
   */
  void AddExpectedTb (uint16_t  rnti, uint8_t resPsdch, uint8_t ndi, std::vector<int> map, uint8_t rv);

  /** 
  * 
  * 
  * \param sinr vector of sinr perceived per each RB
  */
  void UpdateSinrPerceived (const SpectrumValue& sinr);
  
  /** 
  * 
  * 
  * \param sinr vector of sinr perceived per each RB per sidelink packet
  */
  void UpdateSlSinrPerceived (std::vector <SpectrumValue> sinr);
 
  /** 
  * 
  * 
  * \param sinr vector of signal perceived per each RB per sidelink packet
  */
  void UpdateSlSigPerceived (std::vector <SpectrumValue> signal);

  /**
   * 
   * 
   * \return vector of signal perceived per each RB per sidelink packet
   */
  std::vector <SpectrumValue> GetSlSignalPerceived();

  /**
   * 
   * 
   * \return 
   */
  std::vector <SpectrumValue> GetSlInterferencePerceived();

  /** 
  * 
  * 
  * \param sinr vector of interference perceived per each RB per sidelink packet
  */
  void UpdateSlIntPerceived (std::vector <SpectrumValue> interference);

  //void UpdateSlIntPerceived (SpectrumValue interference, Time duration);

  /** 
  * 
  * 
  * \param txMode UE transmission mode (SISO, MIMO tx diversity, ...)
  */
  void SetTransmissionMode (uint8_t txMode);
  
  /** 
   * 
   * \return the previously set channel
   */
  Ptr<SpectrumChannel> GetChannel ();

  /**
    * Set the slssid of the SyncRef to which the UE is synchronized
    * \param slssid the SyncRef identifier
    */
  void SetSlssid (uint64_t slssid);

   /**
    * set the callback for the reception of the SLSS as part
    * of the interconnections between the cv2x_LteSpectrumPhy and the UE PHY
    *
    * @param c the callback
    */
  void SetLtePhyRxSlssCallback (cv2x_LtePhyRxSlssCallback c);

  /// allow cv2x_LteUePhy class friend access
  friend class cv2x_LteUePhy;
  
 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);
  State GetState ();

  void SetRxPool (Ptr<SidelinkDiscResourcePool> newpool);
  void SetRxPool (Ptr<SidelinkCommResourcePoolV2x> newpool); 

  void AddDiscTxApps (std::list<uint32_t> apps);
  void AddDiscRxApps (std::list<uint32_t> apps);
     
  void SetDiscNumRetx (uint8_t retx);

  Ptr<cv2x_LteInterference> GetDataInterferencePointer() {return m_interferenceData;}

  Ptr<cv2x_LteSlInterference> GetSlInterferencePointer() {return m_interferenceSl;}

private:
  /** 
  * \brief Change state function
  * 
  * \param newState the new state to set
  */
  void ChangeState (State newState);
  /// End transmit data function
  void EndTxData ();
  /// End transmit DL control function
  void EndTxDlCtrl ();
  /// End transmit UL SRS function
  void EndTxUlSrs ();
  /// End receive data function
  void EndRxData ();
  /// End receive DL control function
  void EndRxDlCtrl ();
  /// End receive UL SRS function
  void EndRxUlSrs ();
  /// End reveive SL data function
  void EndRxSlData ();
  
  /** 
  * \brief Set transmit mode gain function
  * 
  * \param txMode the transmit mode
  * \param gain the gain to set
  */
  void SetTxModeGain (uint8_t txMode, double gain);

  double GetMeanSinr (const SpectrumValue& sinr, const std::vector<int>& map);
  
  bool FilterRxApps (cv2x_SlDiscMsg disc);
  
  Ptr<MobilityModel> m_mobility; ///< the modility model
  Ptr<AntennaModel> m_antenna; ///< the antenna model
  Ptr<NetDevice> m_device; ///< the device

  Ptr<SpectrumChannel> m_channel; ///< the channel

  Ptr<const SpectrumModel> m_rxSpectrumModel; ///< the spectrum model
  Ptr<SpectrumValue> m_txPsd; ///< the transmit PSD
  Ptr<PacketBurst> m_txPacketBurst; ///< the transmit packet burst
  std::list<Ptr<PacketBurst> > m_rxPacketBurstList; ///< the receive burst list
  
  std::list<Ptr<cv2x_LteControlMessage> > m_txControlMessageList; ///< the transmit control message list
  std::list<Ptr<cv2x_LteControlMessage> > m_rxControlMessageList; ///< the receive control message list
  
  
  State m_state; ///< the state
  Time m_firstRxStart; ///< the first receive start
  Time m_firstRxDuration; ///< the first receive duration

  TracedCallback<Ptr<const PacketBurst> > m_phyTxStartTrace; ///< the phy transmit start trace callback
  TracedCallback<Ptr<const PacketBurst> > m_phyTxEndTrace; ///< the phy transmit end trace callback
  TracedCallback<Ptr<const PacketBurst> > m_phyRxStartTrace; ///< the phy receive start trace callback
  TracedCallback<Ptr<const Packet> >      m_phyRxEndOkTrace; ///< the phy receive end ok trace callback
  TracedCallback<Ptr<const Packet> >      m_phyRxEndErrorTrace; ///< the phy receive end error trace callback

  cv2x_LtePhyRxDataEndErrorCallback   m_ltePhyRxDataEndErrorCallback; ///< the LTE phy receive data end error callback 
  cv2x_LtePhyRxDataEndOkCallback      m_ltePhyRxDataEndOkCallback; ///< the LTE phy receive data end ok callback
  
  cv2x_LtePhyRxCtrlEndOkCallback     m_ltePhyRxCtrlEndOkCallback; ///< the LTE phy receive control end ok callback
  cv2x_LtePhyRxCtrlEndErrorCallback  m_ltePhyRxCtrlEndErrorCallback; ///< the LTE phy receive control end error callback
  cv2x_LtePhyRxPssCallback  m_ltePhyRxPssCallback; ///< the LTE phy receive PSS callback

  Ptr<cv2x_LteInterference> m_interferenceData; ///< the data interference
  Ptr<cv2x_LteInterference> m_interferenceCtrl; ///< the control interference

  uint16_t m_cellId; ///< the cell ID
  
  uint8_t m_componentCarrierId; ///< the component carrier ID
  cv2x_expectedTbs_t m_expectedTbs; ///< the expected TBS
  expectedDiscTbs_t m_expectedDiscTbs; ///< the expected discovery TBS
  SpectrumValue m_sinrPerceived; ///< the preceived SINR 

  // Information for sidelink communication
  Ptr<cv2x_LteSlInterference> m_interferenceSl;
  std::set<uint8_t> m_l1GroupIds; // identifiers for D2D layer 1 filtering
  expectedSlTbs_t m_expectedSlTbs;  
  std::vector<SpectrumValue> m_slSinrPerceived; //SINR for each D2D packet received
  std::vector<SpectrumValue> m_slSignalPerceived; //Signal for each D2D packet received
  std::vector<SpectrumValue> m_slInterferencePerceived; //Interference for each D2D packet received
  //std::map<Ptr<cv2x_LteControlMessage>, std::vector <int> > m_rxControlMessageRbMap;
  std::vector<cv2x_SlRxPacketInfo_t> m_rxPacketInfo;

  // Information for sidelink V2x communication
  expectedSlV2xTbs_t m_expectedSlV2xTbs;

  /// Provides uniform random variables.
  Ptr<UniformRandomVariable> m_random;
  bool m_dataErrorModelEnabled; ///< when true (default) the phy error model is enabled
  bool m_ctrlErrorModelEnabled; ///< when true (default) the phy error model is enabled for DL ctrl frame

  bool m_ctrlFullDuplexEnabled; // when true the PSCCH operates in Full Duplexmode (disabled by default).

  bool m_dropRbOnCollisionEnabled; //when true, drop all receptions on colliding RBs regardless SINR value.

  /// NIST Physical error model
  bool m_nistErrorModelEnabled; // when true (not default) use NIST error model
  cv2x_LtePhyErrorModel::LteFadingModel m_fadingModel;

  bool m_slBlerEnabled; //(true by default) when false BLER in the PSSCH is not modeled.

  uint8_t m_transmissionMode; ///< for UEs: store the transmission mode
  uint8_t m_layersNum; ///< layers num
  std::vector <double> m_txModeGain; ///< duplicate value of cv2x_LteUePhy

  bool m_ulDataSlCheck;

  Ptr<cv2x_LteHarqPhy> m_harqPhyModule; ///< the HARQ phy module
  cv2x_LtePhyDlHarqFeedbackCallback m_ltePhyDlHarqFeedbackCallback; ///< the LTE phy DL HARQ feedback callback
  cv2x_LtePhyUlHarqFeedbackCallback m_ltePhyUlHarqFeedbackCallback; ///< the LTE phy UL HARQ feedback callback

  Ptr<cv2x_LteSpectrumPhy> m_halfDuplexPhy;
  bool m_errorModelHarqD2dDiscoveryEnabled;
  
  std::list< Ptr<SidelinkDiscResourcePool> > m_discRxPools;

  std::list< Ptr<SidelinkCommResourcePoolV2x>> m_slV2xRxPools; 
  
  std::list<uint32_t> m_discTxApps;
  std::list<uint32_t> m_discRxApps;

  /**
   * Trace information regarding PHY stats from DL Rx perspective
   * cv2x_PhyReceptionStatParameters (see lte-common.h)
   */
  TracedCallback<cv2x_PhyReceptionStatParameters> m_dlPhyReception;

  /**
   * Trace information regarding PHY stats from UL Rx perspective
   * cv2x_PhyReceptionStatParameters (see lte-common.h)
   */
  TracedCallback<cv2x_PhyReceptionStatParameters> m_ulPhyReception;

  /**
   * Trace information regarding PHY stats from UL Rx perspective
   * cv2x_PhyReceptionStatParameters (see lte-common.h)
   */
  TracedCallback<cv2x_PhyReceptionStatParameters> m_slPhyReception;

  /**
   * Trace information regarding PHY stats from SL Rx PSCCH perspective
   * cv2x_PhyReceptionStatParameters (see lte-common.h)
   */
  TracedCallback<cv2x_PhyReceptionStatParameters> m_slPscchReception;

  EventId m_endTxEvent; ///< end transmit event
  EventId m_endRxDataEvent; ///< end receive data event
  EventId m_endRxDlCtrlEvent; ///< end receive DL control event
  EventId m_endRxUlSrsEvent; ///< end receive UL SRS event

  /**
   * The Sidelink Synchronization Signal Identifier (SLSSID)
   */
  uint64_t m_slssId;
  /**
   * Callback used to notify the PHY about the reception of a SLSS
   */
  cv2x_LtePhyRxSlssCallback  m_ltePhyRxSlssCallback;

};






}

#endif /* CV2X_LTE_SPECTRUM_PHY_H */

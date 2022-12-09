/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef NR_SPECTRUM_PHY_H
#define NR_SPECTRUM_PHY_H

#include <functional>
#include <ns3/random-variable-stream.h>
#include <ns3/traced-callback.h>
#include <ns3/spectrum-channel.h>
#include <ns3/net-device.h>
#include "nr-phy.h"
#include "nr-harq-phy.h"
#include "nr-interference.h"
#include "nr-spectrum-signal-parameters.h"
#include "nr-control-messages.h"
#include <ns3/lte-chunk-processor.h>
#include "nr-sl-interference.h"
#include "nr-sl-chunk-processor.h"
#include "nr-amc.h"

namespace ns3 {

  class UniformPlanarArray;

/**
 * \ingroup ue-phy
 * \ingroup gnb-phy
 * \ingroup spectrum
 *
 * \brief Interface between the physical layer and the channel
 *
 * \section spectrum_phy_general General information
 *
 * NrSpectrumPhy models some of the basic
 * physical layer functionalities such as transmitting CTRL or DATA,
 * receiving the signals, decoding them, and distinguishing whether the
 * signal is useful (CTRL, DATA), i.e. sent to this NR device NrPhy
 * instance, or it should be considered as interference. It
 * is also responsible for obtaining measurements, and for HARQ feedback
 * generation by leveraging HARQ module.
 *
 * This class implements the interface between the NrPhy
 * and the spectrum channel, and provides to NrPhy aforementioned
 * functionalities. Each NR device's NrPhy has its
 * own NrSpectrumPhy that is in charge of providing these basic
 * phy layer functionalities. In order to be able to receive signals from the channel,
 * each NrSpectrumPhy should be registered to listen events from its channel.
 * To achieve that, during the configuration of NR device's NrPhy
 * at some point should be called AddRx function of the spectrum channel
 * to register the NrSpectrumPhy instance as a receiver on that channel.
 *
 * This class also has the interface with NrInterference class to
 * pass the necessary information for the interference calculation and to
 * obtain the interference calculation results.
 *
 * Also it has interface with HARQ module, to which it passes necessary
 * information for the HARQ feedback generation, which is then forwarded
 * to NrPhy.
 *
 * \section spectrum_phy_configuration Configuration
 *
 * The user can configure the class using the method NrHelper::SetGnbSpectrumAttribute(),
 * or NrHelper::SetUeSpectrumAttribute(), depending on the type of user
 * you want to configure, or by directly calling `SetAttribute` on the pointer.
 * The list of  attributes is reported below, in the Attributes section.
 */
class NrSpectrumPhy : public SpectrumPhy
{
public:
  /**
   * \brief Get the object TypeId
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrSpectrumPhy constructor
   */
  NrSpectrumPhy ();

  /**
   * \brief ~NrSpectrumPhy
   */
  virtual ~NrSpectrumPhy () override;

  /**
   * \brief Enum that defines possible states of the spectrum phy
   */
  enum State
  {
    IDLE = 0,  //!< IDLE state (no action in progress)
    TX,        //!< Transmitting state (data or ctrl)
    RX_DATA,   //!< Receiving data
    RX_DL_CTRL,//!< Receiveing DL CTRL
    RX_UL_CTRL,//!< Receiving UL CTRL
    RX_UL_SRS, //!< Receiving SRS
    CCA_BUSY   //!< BUSY state (channel occupied by another entity)
  };

  //callbacks typefefs and setters
  /**
   * \brief This callback method type is used to notify that DATA is received
   */
  typedef Callback< void, const Ptr<Packet> &> NrPhyRxDataEndOkCallback;
  /**
   * \brief This callback method type is used to notify that CTRL is received
   */
  typedef std::function<void (const std::list<Ptr<NrControlMessage> > &, uint8_t)> NrPhyRxCtrlEndOkCallback;
  /**
   * This callback method type is used by the NrSpectrumPhy to notify the PHY about
   * the status of a DL HARQ feedback
   */
  typedef Callback< void, const DlHarqInfo& > NrPhyDlHarqFeedbackCallback;
  /**
   * This callback method type is used by the NrSpectrumPhy to notify the PHY about
   * the status of a UL HARQ feedback
   */
  typedef Callback< void, const UlHarqInfo &> NrPhyUlHarqFeedbackCallback;

  /**
   * \brief Sets the callback to be called when DATA is received successfully
   * \param c the callback function
   */
  void SetPhyRxDataEndOkCallback (const NrPhyRxDataEndOkCallback& c);
  /**
   * \brief Sets the callback to be called when CTRL is received successfully
   * \param c the callback function
   */
  void SetPhyRxCtrlEndOkCallback (const NrPhyRxCtrlEndOkCallback& c);
  /**
   * \brief Sets the callback to be called when DL HARQ feedback is generated
   */
  void SetPhyDlHarqFeedbackCallback (const NrPhyDlHarqFeedbackCallback& c);
  /**
   * \brief Sets the callback to be called when UL HARQ feedback is generated
   */
  void SetPhyUlHarqFeedbackCallback (const NrPhyUlHarqFeedbackCallback& c);

  //Methods inherited from spectrum phy
  void SetDevice (Ptr<NetDevice> d) override;
  Ptr<NetDevice> GetDevice () const override;
  void SetMobility (Ptr<MobilityModel> m) override;
  Ptr<MobilityModel> GetMobility () const override;
  void SetChannel (Ptr<SpectrumChannel> c) override;
  Ptr<const SpectrumModel> GetRxSpectrumModel () const override;
  /**
   * \brief Inherited from SpectrumPhy
   * Note: Implements GetRxAntenna function from SpectrumPhy. This
   * function should not be called for NR devices, since NR devices do not use
   * AntennaModel. This is because 3gpp channel model implementation only
   * supports PhasedArrayModel antenna type.
   * \return should not return anything
   */
  virtual Ptr<AntennaModel> GetRxAntenna () const override;
  /**
   * \brief Inherited from SpectrumPhy. When this function is called
   * this spectrum phy starts receiving a signal from its spectrum channel.
   * \param params SpectrumSignalParameters object that will be used to process this signal
   */
  void StartRx (Ptr<SpectrumSignalParameters> params) override;

  // Attributes setters
  /**
   * \brief Set clear channel assessment (CCA) threshold
   * \param thresholdDBm - CCA threshold in dBms
   */
  void SetCcaMode1Threshold (double thresholdDBm);
  /**
   * Returns clear channel assesment (CCA) threshold
   * \return CCA threshold in dBms
   */
  double GetCcaMode1Threshold (void) const;
  /**
   * \brief Sets whether to perform in unclicensed mode in which the channel monitoring is enabled
   * \param unlicensedMode if true the unlicensed mode is enabled
   */
  void SetUnlicensedMode (bool unlicensedMode);
  /**
   * \brief Enables or disabled data error model
   * \param dataErrorModelEnabled boolean saying whether the data error model should be enabled
   */
  void SetDataErrorModelEnabled (bool dataErrorModelEnabled);
  /**
   * \brief Sets the error model type
   */
  void SetErrorModelType (TypeId errorModelType);

  // other methods
  /**
   * \brief Sets noise power spectral density to be used by this device
   * \param noisePsd SpectrumValue object holding noise PSD
   */
  void SetNoisePowerSpectralDensity (const Ptr<const SpectrumValue>& noisePsd);
  /**
   * \brief Sets transmit power spectral density
   * \param txPsd transmit power spectral density to be used for the upcoming transmissions by this spectrum phy
   */
  void SetTxPowerSpectralDensity (const Ptr<SpectrumValue>& txPsd);
  /**
   * \brief Starts transmission of data frames on connected spectrum channel object
   * \param pb packet burst to be transmitted
   * \param ctrlMsgList control message list
   * \param duration the duration of transmission
   */
 void StartTxDataFrames (const Ptr<PacketBurst>& pb, const std::list<Ptr<NrControlMessage> >& ctrlMsgList, Time duration);
  /**
   * \brief Starts transmission of DL CTRL
   * \param duration the duration of this transmission
   */
  void StartTxDlControlFrames (const std::list<Ptr<NrControlMessage> > &ctrlMsgList, const Time &duration);   // control frames from enb to ue
  /**
   * \brief Start transmission of UL CTRL
   * \param ctrlMsgList the list of control messages to be transmitted in UL
   * \param duration the duration of the CTRL messages transmission
   */
  void StartTxUlControlFrames (const std::list<Ptr<NrControlMessage> > &ctrlMsgList, const Time &duration);
  /**
   * \brief Adds the chunk processor that will process the power for the data
   * \param p the chunk processor
   */
  void AddDataPowerChunkProcessor (const Ptr<LteChunkProcessor>& p);
  /**
   * \brief Adds the chunk processor that will process the interference
   * \param p the chunk processor
   */
  void AddDataSinrChunkProcessor (const Ptr<LteChunkProcessor>& p);

  /*
   * \brief Adds the chunk processort that will process the interference for SRS signals at gNBs
   * \param p the chunk processor
   */
  void AddSrsSinrChunkProcessor (const Ptr<LteChunkProcessor>& p);

  /**
   * \brief Adds the chunk processor that will process the received power
   * \param p the chunk processor
   */
  void AddRsPowerChunkProcessor (const Ptr<LteChunkProcessor>& p);
  /**
   * \brief SpectrumPhy that will be called when the SINR for the received
   * DATA is being calculated by the interference object over DATA chunk
   * processor
   * \param sinr the resulting SINR spectrum value
   */
  void UpdateSinrPerceived (const SpectrumValue& sinr);

  /**
    * \brief SpectrumPhy that will be called when the SINR for the received
    * SRS at gNB is being calculated by the interference object over SRS chunk
    * processor
    * \param sinr the resulting SRS SINR spectrum value
    */
   void UpdateSrsSinrPerceived (const SpectrumValue& srsSinr);
   /**
     * \brief SpectrumPhy that will be called when the SNR for the received
     * SRS at gNB is being calculated
     * \param snr the resulting SRS SNR
     */
    void UpdateSrsSnrPerceived (const double srsSnr);
  /**
   * \brief Install HARQ phy module of this spectrum phy
   * \param harq Harq module of this spectrum phy
   */
  void InstallHarqPhyModule (const Ptr<NrHarqPhy>& harq);
  /**
   * \brief Set NrPhy of this spectrum phy in order to be able
   * to obtain information such as cellId, bwpId, etc.
   */
  void InstallPhy (const Ptr<const NrPhy> &phyModel);
  /**
   * \return Returns UniformPlanarArray instance of this spectrum phy
   */
  Ptr<const UniformPlanarArray> GetAntennaArray (void) const;
  /**
   * \brief Returns spectrum channel object to which is attached this spectrum phy instance
   */
  Ptr<SpectrumChannel> GetSpectrumChannel (void) const;
  /**
   * \return HARQ module of this spectrum phy
   */
  Ptr<NrHarqPhy> GetHarqPhyModule (void) const;
  /**
   * \return NrInterference instance of this spectrum phy
   */
  Ptr<NrInterference> GetNrInterference (void) const;
  /**
   * \brief Instruct the Spectrum Model of a incoming transmission.
   * \param rnti RNTI
   * \param ndi New data indicator (0 for retx)
   * \param size TB Size
   * \param mcs MCS of the transmission
   * \param rbMap Resource Block map (PHY-ready vector of SINR indices)
   * \param harqId ID of the HARQ process in the MAC
   * \param rv Redundancy Version: number of times the HARQ has been retransmitted
   * \param downlink indicate if it is downling
   * \param symStart Sym start
   * \param numSym Num of symbols
   * \param sfn SFN
   */
  void AddExpectedTb (uint16_t rnti, uint8_t ndi, uint32_t size, uint8_t mcs, const std::vector<int> &rbMap,
                      uint8_t harqId, uint8_t rv, bool downlink, uint8_t symStart, uint8_t numSym,
                      const SfnSf &sfn);

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

  /**
   * \brief TracedCallback signature for RB statistics
   *
   * \param [in] sfnSf SfnSf
   * \param [in] v rxPsd values
   * \param [in] t duration of the reception
   * \param [in] bwpId BWP ID
   * \param [in] cellId Cell ID
   */
  typedef void (* RxDataTracedCallback)(const SfnSf & sfnSf, Ptr<const SpectrumValue> v,
                                        const Time & t, uint16_t bwpId, uint16_t cellId);


  void AddExpectedSrsRnti (uint16_t rnti);

  /*
   * \brief SRS SINR callback whose input parameters are cellid, rnti, SRS SINR value
   */
  typedef Callback < void, uint16_t, uint16_t, double> SrsSinrReportCallback;
  typedef Callback < void, uint16_t, uint16_t, double> SrsSnrReportCallback;

  /**
   * \brief It adds callback to the list of callbacks that will be notified
   * once SRS is being received
   * \param callback callback to be added to the list of callbacks
   */
  void AddSrsSinrReportCallback (SrsSinrReportCallback callback);
  /**
   * \brief It adds callback to the list of callbacks that will be notified
   * once SRS is being received
   * \param callback callback to be added to the list of callbacks
   */
  void AddSrsSnrReportCallback (SrsSnrReportCallback callback);

protected:
  /**
   * \brief DoDispose method inherited from Object
   */
  void virtual DoDispose () override;

private:

  /**
   * \brief Function is called when what is being received is holding data
   * \para params spectrum parameters that are holding information regarding data frame
   */
  void StartRxData (const Ptr<NrSpectrumSignalParametersDataFrame>& params);
  /**
   * \brief Function that is called when is being received DL CTRL
   * \param params holds DL CTRL frame signal parameters structure
   */
  void StartRxDlCtrl (const Ptr<NrSpectrumSignalParametersDlCtrlFrame>& params);
  /**
   * \brief Function that is called when is being received UL CTRL
   * \param params holds UL CTRL frame signal parameters structure
   */
  void StartRxUlCtrl (const Ptr<NrSpectrumSignalParametersUlCtrlFrame>& params);
  /**
   * \brief Function that is called when is being received SRS
   * \param param should hold UL CTRL frame singal parameters containing only
   * one CTRL message which should be of type SRS
   */
  void StartRxSrs (const Ptr<NrSpectrumSignalParametersUlCtrlFrame>& params);
  /**
   * \return the cell id
   */
  uint16_t GetCellId () const;
  /**
   * \return the bwp id
   */
  uint16_t GetBwpId () const;
  /**
   * \return true if this class is inside an enb/gnb
   */
  bool IsEnb () const;
  /**
   * \brief Update the state of the spectrum phy. The states are:
   *  IDLE, TX, RX_DATA, RX_DL_CTRL, RX_UL_CTRL, CCA_BUSY.
   * \param newState the new state
   * \param duration how much time the spectrum phy will be in the new state
   */
  void ChangeState (State newState, Time duration);
  /**
   * \brief Function that is called when the transmission has ended. It is
   * used to update spectrum phy state.
   */
  void EndTx ();
  /**
   * \brief Function that is called when the spectrum phy finishes the reception of DATA. This
   * function processed the data being received and generated HARQ feedback.
   * It also updates spectrum phy state.
   */
  void EndRxData ();
  /**
   * \brief Function that is called when the spectrum phy finishes the reception of CTRL.
   * It stores CTRL messages and updates spectrum phy state.
   */
   void EndRxCtrl ();
   /**
    * \brief Function that is celled when the spectrum phy finishes the reception of SRS.
    * It stores SRS message, calles the interference calculator to notify the end of the
    * reception which will trigger SRS SINR calculation, and it also updates the spectrum phy state.
    */
   void EndRxSrs ();
   /**
    * \brief Check if the channel is busy. If yes, updates the spectrum phy state.
    */
   void MaybeCcaBusy ();
   /**
    * \brief Function used to schedule an event to check if state should be switched from CCA_BUSY to IDLE.
    * This function should be used only for this transition of state machine. After finishing
    * reception (RX_DL_CTRL or RX_UL_CTRL or RX_DATA) function MaybeCcaBusy should be called instead to check
    * if to switch to IDLE or CCA_BUSY, and then new event may be created in the case that the
    * channel is BUSY to switch back from busy to idle.
    */
   void CheckIfStillBusy ();
   /**
    * \brief Checks whether the CTRL message list contains only SRS control message.
    * Only if the list has only one CTRL message and that message is SRS the function
    * will return true, otherwise it will return false.
    * \ctrlMsgList uplink control message list
    * \returns an indicator whether the ctrlListMessage contains only SRS message
    */
   bool IsOnlySrs (const std::list<Ptr<NrControlMessage> >& ctrlMsgList);

   /**
    * \brief Information about the expected transport block at a certain point in the slot
    *
    * Information passed by the PHY through a call to AddExpectedTb
    */
  struct ExpectedTb
  {
    ExpectedTb (uint8_t ndi, uint32_t tbSize, uint8_t mcs, const std::vector<int> &rbBitmap,
                uint8_t harqProcessId, uint8_t rv, bool isDownlink, uint8_t symStart,
                uint8_t numSym, const SfnSf &sfn) :
      m_ndi (ndi),
      m_tbSize (tbSize),
      m_mcs (mcs),
      m_rbBitmap (rbBitmap),
      m_harqProcessId (harqProcessId),
      m_rv (rv),
      m_isDownlink (isDownlink),
      m_symStart (symStart),
      m_numSym (numSym),
      m_sfn (sfn) { }
    ExpectedTb () = delete;
    ExpectedTb (const ExpectedTb &o) = default;

    uint8_t m_ndi               {0}; //!< New data indicator
    uint32_t m_tbSize           {0}; //!< TBSize
    uint8_t m_mcs               {0}; //!< MCS
    std::vector<int> m_rbBitmap;     //!< RB Bitmap
    uint8_t m_harqProcessId     {0}; //!< HARQ process ID (MAC)
    uint8_t m_rv                {0}; //!< RV
    bool m_isDownlink           {0}; //!< is Downlink?
    uint8_t m_symStart          {0}; //!< Sym start
    uint8_t m_numSym            {0}; //!< Num sym
    SfnSf m_sfn;                     //!< SFN
  };

  struct TransportBlockInfo
  {
    TransportBlockInfo (const ExpectedTb &expected) :
      m_expected (expected) { }
    TransportBlockInfo () = delete;

    ExpectedTb m_expected;                //!< Expected data from the PHY. Filled by AddExpectedTb
    bool m_isCorrupted {false};           //!< True if the ErrorModel indicates that the TB is corrupted.
                                          //    Filled at the end of data rx/tx
    bool m_harqFeedbackSent {false};      //!< Indicate if the feedback has been sent for an entire TB
    Ptr<NrErrorModelOutput> m_outputOfEM; //!< Output of the Error Model (depends on the EM type)
    double m_sinrAvg {0.0};               //!< AVG SINR (only for the RB used to transmit the TB)
    double m_sinrMin {0.0};               //!< MIN SINR (only between the RB used to transmit the TB)
  };

  //attributes
  TypeId m_errorModelType {Object::GetTypeId()}; //!< Error model type by default is NrLteMiErrorModel
  bool m_dataErrorModelEnabled {true}; //!< whether the phy error model for DATA is enabled, by default is enabled
  double m_ccaMode1ThresholdW {0}; //!< Clear channel assessment (CCA) threshold in Watts, attribute that it configures it is
                                   //   CcaMode1Threshold and is configured in dBm
  bool m_unlicensedMode {false}; //!< Whether this spectrum phy is configure to work in an unlicensed mode.
                                 //   Unlicensed mode additionally to licensed mode allows channel monitoring to discover if is busy before transmission.

  Ptr<SpectrumChannel> m_channel {nullptr}; //!< channel is needed to be able to connect listener spectrum phy (AddRx) or to start transmission StartTx
  Ptr<const SpectrumModel> m_rxSpectrumModel {nullptr}; //!< the spectrum model of this spectrum phy
  Ptr<MobilityModel> m_mobility {nullptr}; //!< the mobility model of the node to which belongs this spectrum phy
  Ptr<NetDevice> m_device {nullptr}; //!< the device to which belongs this spectrum phy
  Ptr<const NrPhy> m_phy {nullptr}; //!< a pointer to phy instance to which belongs this spectrum phy
  Ptr<NrHarqPhy> m_harqPhyModule {nullptr}; //!< the HARQ module of this spectrum phy instance
  Ptr<NrInterference> m_interferenceData {nullptr}; //!<the interference object used to calculate the interference for this spectrum phy
  Ptr<NrInterference> m_interferenceCtrl {nullptr}; //!<the interference object used to calculate the interference for this spectrum phy
  Ptr<NrInterference> m_interferenceSrs {nullptr}; //!<the interference object used to calculate the interference for this spectrum phy, exists only at gNB phy
  Ptr<SpectrumValue> m_txPsd {nullptr}; //!< tx power spectral density
  Ptr<UniformRandomVariable> m_random {nullptr}; //!< the random variable used for TB decoding

  std::unordered_map<uint16_t, TransportBlockInfo> m_transportBlocks; //!< Transport block map per RNTI of TBs which are expected to be received by reading DL or UL DCIs
  std::list<Ptr<PacketBurst> > m_rxPacketBurstList; //!< the list of received packets
  std::list<Ptr<NrControlMessage> > m_rxControlMessageList; //!< the list of received control messages

  Time m_firstRxStart {Seconds (0)}; //!< this is needed to save the time at which we lock down onto signal
  Time m_firstRxDuration {Seconds (0)}; //!< the duration of the current reception
  State m_state {IDLE}; //!<spectrum phy state
  SpectrumValue m_sinrPerceived; //!< SINR that is being update at the end of the DATA reception and is used for TB decoding
  std::list<SrsSinrReportCallback> m_srsSinrReportCallback; //!< list of SRS SINR callbacks
  std::list<SrsSnrReportCallback> m_srsSnrReportCallback; //!< list of SRS SNR callbacks
  uint16_t m_currentSrsRnti {0};
  EventId m_checkIfIsIdleEvent; //!< Event used to check if state should be switched from CCA_BUSY to IDLE.
  Time m_busyTimeEnds {Seconds (0)}; //!< Used to schedule switch from CCA_BUSY to IDLE, this is absolute time

  //callbacks for CTRL and DATA, and UL/DL HARQ
  NrPhyRxCtrlEndOkCallback m_phyRxCtrlEndOkCallback; //!< callback that is notified when the CTRL is received
  NrPhyRxDataEndOkCallback m_phyRxDataEndOkCallback; //!< callback that is notified when the DATA is received
  NrPhyDlHarqFeedbackCallback m_phyDlHarqFeedbackCallback; //!< callback that is notified when the DL HARQ feedback is being generated
  NrPhyUlHarqFeedbackCallback m_phyUlHarqFeedbackCallback; //!< callback that is notified when the UL HARQ feedback is being generated

  //traces
  TracedCallback <Time> m_channelOccupied; //!< trace callback that is notifying of total time that this spectrum phy sees the channel occupied, by others and by itself
  TracedCallback <Time> m_txDataTrace; //!< trace callback that is notifying when this spectrum phy starts to occupy the channel with data transmission
  TracedCallback <Time> m_txCtrlTrace; //!< trace callback that is notifying when this spectrum phy starts to occupy the channel with transmission of CTRL
  TracedCallback<RxPacketTraceParams> m_rxPacketTraceEnb; //!< trace callback that is notifying when eNb received the packet
  TracedCallback<RxPacketTraceParams> m_rxPacketTraceUe; //!< trace callback that is notifying when UE received the packet
  TracedCallback<GnbPhyPacketCountParameter > m_txPacketTraceEnb; //!< trace callback that is notifying when eNb transmts the packet
  TracedCallback<const SfnSf &, Ptr<const SpectrumValue>, const Time &, uint16_t, uint16_t> m_rxDataTrace;

  // NR SL
public:
  /**
   * \brief Structure to store NR Sidelink signal parameter being received
   *        along with the vector indicating the indexes of the RBs this signal
   *        is transmitted over.
   */
  struct SlRxSigParamInfo
  {
    Ptr<NrSpectrumSignalParametersSlFrame> params; //!< Parameters of sidelink signal
    std::vector<int> rbBitmap;  //!< RB bitmap
  };
  /**
   * \brief SlCtrlSigParamInfo structure
   */
  struct SlCtrlSigParamInfo
  {
    double sinrAvg {0.0}; //!< Average SINR
    double sinrMin {0.0}; //!< Minimum SINR
    uint32_t index {std::numeric_limits <uint32_t>::max ()}; //!< Index of the signal received in the reception buffer

    /**
     * \brief Implements equal operator
     * \param a element to compare
     * \param b element to compare
     * \return true if the elements are equal
     */
    friend bool operator == (const SlCtrlSigParamInfo &a, const SlCtrlSigParamInfo &b);

    /**
     * \brief Implements less operator
     * \param a element to compare
     * \param b element to compare
     * \return true if a is less than b
     */
    friend bool operator < (const SlCtrlSigParamInfo &a, const SlCtrlSigParamInfo &b);
  };
  /**
   * \brief struct to store the PSCCH PDU info
   *
   * This struct is used to store the packet and PSD of a
   * PSCCH transmission.
   */
  struct PscchPduInfo
  {
    Ptr <Packet> packet; //!< PSCCH packet
    SpectrumValue psd;  //!< PSD of the received packet
  };
  /**
   * \brief SlTbId struct
   */
  struct SlTbId
  {
    uint16_t m_rnti; //!< source SL-RNTI
    uint32_t m_dstId; //!< The destination id
  };

  /**
   * \brief Information about the expected NR SL transport block at a certain point in the slot
   *
   * Information passed by the PHY through a call to AddSlExpectedTb
   */
 struct SlExpectedTb
 {
    /**
     * \brief constructor
     * \param dstId The Destination id
     * \param tbSize The TB size
     * \param mcs The MCS
     * \param rbMap The RB map
     * \param symStart The starting symbol index
     * \param numSym The total number of symbols
     * \param sfn The SfnSf
     */
   SlExpectedTb (uint32_t dstId, uint32_t tbSize, uint8_t mcs, const std::vector<int> &rbMap,
               uint8_t symStart, uint8_t numSym, const SfnSf &sfn) :
     dstId {dstId},
     tbSize (tbSize),
     mcs (mcs),
     rbBitmap (rbMap),
     symStart (symStart),
     numSym (numSym),
     sfn (sfn) { }
   SlExpectedTb () = delete;
   /**
    * \brief default copy constructor
    * \param o other SlExpectedTb object
    */
   SlExpectedTb (const SlExpectedTb &o) = default;

   uint32_t dstId            {0}; //!< Destination id
   uint32_t tbSize           {0}; //!< TBSize
   uint8_t mcs               {0}; //!< MCS
   std::vector<int> rbBitmap;     //!< RB Bitmap
   uint8_t symStart          {0}; //!< Sym start
   uint8_t numSym            {0}; //!< Num sym
   SfnSf sfn;                     //!< SFN
 };

 /**
  * \brief Sidelink transport block info
  */
 struct SlTransportBlockInfo
 {
   /**
    * \brief constructor
    * \param expectedTb Expected NR SL data from the PHY
    */
   SlTransportBlockInfo (const SlExpectedTb &expectedTb) :
     expectedTb (expectedTb) { }
   SlTransportBlockInfo () = delete;

   SlExpectedTb expectedTb;                   //!< Expected NR SL data from the PHY. Filled by AddSlExpectedTb
   bool isDataCorrupted {false};              //!< True if the ErrorModel indicates that the data TB is corrupted.
   bool isSci2Corrupted {false};              //!< True if the ErrorModel indicates that the SCI stage 2 is corrupted.
                                              //    Filled at the end of data rx/tx
   Ptr<NrErrorModelOutput> outputEmForData;   //!< Output of the Error Model (depends on the EM type) for data
   Ptr<NrErrorModelOutput> outputEmForSci2;   //!< Output of the Error Model (depends on the EM type) for SCI stage 2
   SpectrumValue sinrPerceived;               //!< SINR that is being update at the end of the DATA reception and is used for TB decoding
   bool sinrUpdated {false};                  //!< Flag to indicate the successful update of sinrPerceived
   double sinrAvg {0.0};                      //!< AVG SINR (only for the RB used to transmit the TB)
   double sinrMin {0.0};                       //!< MIN SINR (only between the RB used to transmit the TB)
   uint32_t pktIndex {std::numeric_limits <uint32_t>::max ()}; //!< Index of the TB in the \p m_slRxSigParamInfo buffer
 };
  /**
   * \brief This callback method type is used to notify about a successful PSCCH reception
   */
  typedef std::function<void (const Ptr<Packet>&, const SpectrumValue&)> NrPhyRxPscchEndOkCallback;
  /**
   * \brief This callback method type is used to notify about a successful
   *        PSSCH reception.
   */
  typedef std::function<void (const Ptr<PacketBurst>&)> NrPhyRxPsschEndOkCallback;
  /**
   * \brief This callback method type is used to notify about a unsuccessful
   *        PSSCH reception.
   */
  typedef std::function<void (const Ptr<PacketBurst>&)> NrPhyRxPsschEndErrorCallback;
  /**
   * \brief Sets the NR sidelink error model type
   *
   * \param errorModelType The TypeId of the error model to be used.
   */
  void SetSlErrorModelType (TypeId errorModelType);
  /**
   * \brief Enables or disabled NR Sidelink data error model
   * \param slDataErrorModelEnabled boolean saying whether the NR SL data error model should be enabled
   */
  void SetSlDataErrorModelEnabled (bool slDataErrorModelEnabled);
  /**
   * \brief Enables or disabled NR Sidelink CTRL error model
   * \param slCtrlErrorModelEnabled boolean saying whether the NR SL CTRL error model should be enabled
   */
  void SetSlCtrlErrorModelEnabled (bool slCtrlErrorModelEnabled);
  /**
   * \brief Enable or disable the drop of a NR SL TB whose RB collided with
   *        other TB.
   *  Note: The same flag is used to enable/disable the drop NR SL PSCCH
   *        packet burst.
   * \param drop If true, a TB (for PSSCH) or a packet burst (for PSCCH),
   *        regardless of SINR value, is drop if its RBs collided with other
   *        TB or packet burst. Otherwise, its reception will depend on
   *        the error model output and a random probability of decoding.
   */
  void DropTbOnRbOnCollision (bool drop);
  /**
   * \brief Starts transmission of NR SL data frames on connected spectrum channel object
   * \param pb packet burst to be transmitted
   * \param duration the duration of transmission
   */
  void StartTxSlDataFrames (const Ptr<PacketBurst>& pb, Time duration);
  /**
   * \brief Starts transmission of NR SL CTRL data frames on connected spectrum channel object
   * \param pb packet burst to be transmitted
   * \param duration the duration of transmission
   */
  void StartTxSlCtrlFrames (const Ptr<PacketBurst>& pb, Time duration);
  /**
   * \brief Adds the NR SL chunk processor that passes the SINR of received
   *        signal (s) to this SpectrumPhy once its reception ends.
   * \param p The new NrSlChunkProcessor to be added to the NR Sidelink processing chain
   */
  void AddSlSinrChunkProcessor (Ptr<NrSlChunkProcessor> p);
  /**
   * \brief Adds the NR SL chunk processor that passes the PSD of received
   *        signal (s) to this SpectrumPhy once its reception ends.
   * \param p The new NrSlChunkProcessor to be added to the NR Sidelink processing chain
   */
  void AddSlSignalChunkProcessor (Ptr<NrSlChunkProcessor> p);
  /**
   * \brief This method will be called when the SINR for the received
   *        NR Sidelink signal, i.e., PSCCH or PSSCH is being calculated by
   *        the interference object over Sidelink chunk processor.
   * \param sinr The vector of the resulting SINR values per Sidelink packet.
   *        These SINR values are spectrum values per each RB.
   */
  void UpdateSlSinrPerceived (std::vector <SpectrumValue> sinr);
  /**
   * \brief This method will be called when the PSD for the received
   *        NR Sidelink signal, i.e., PSCCH or PSSCH is being calculated by
   *        the interference object over Sidelink chunk processor.
   * \param sig The vector of the resulting PSD values per Sidelink packet.
   *        These PSD values are spectrum values per each RB.
   */
  void UpdateSlSignalPerceived (std::vector <SpectrumValue> sig);
  /**
   * \brief Set NR SL AMC
   *
   * I needed to add the to compute the size of PSCCH TB to compute BLER.
   * Theoretically, the SL scheduler should compute this TB size and NrUeMac
   * should include it in Tag or something. At the moment, I am already
   * consuming 20 bytes with the NrSlMacPduTag. If I will remove some fields
   * in the future, maybe, I will include PSCCH TB size there.
   *
   * \param slAmc NR SL AMC
   */
  void SetSlAmc (Ptr <NrAmc> slAmc);
  /**
   * \brief Set the callback for the successful end of a PSCCH RX, as part of the
   * interconnections between the PHY and the MAC
   *
   * \param c The callback
   */
  void SetNrPhyRxPscchEndOkCallback (NrPhyRxPscchEndOkCallback c);
  /**
   * \brief Set the callback for the end of a successful PSCCH RX.
   * \param c The callback
   */
  void SetNrPhyRxPsschEndOkCallback (NrPhyRxPsschEndOkCallback c);
  /**
   * \brief Set the callback for the end of a unsuccessful PSCCH RX.
   * \param c The callback
   */
  void SetNrPhyRxPsschEndErrorCallback (NrPhyRxPsschEndErrorCallback c);
  /**
   * \brief Add sidelink expected Transport Block (TB)
   * \param rnti The RNTI of the UE from whom to expect the TB
   * \param dstId The destination L2 id
   * \param tbSize The TB size
   * \param mcs The MCS
   * \param rbMap Bitmap indicating the RBs used to tx the TB
   * \param symStart The index of the starting symbol
   * \param numSym The total number of symbols used
   * \param sfn The sfn
   */
  void AddSlExpectedTb (uint16_t rnti, uint32_t dstId, uint32_t tbSize,
                        uint8_t mcs, const std::vector<int> &rbMap,
                        uint8_t symStart, uint8_t numSym,const SfnSf &sfn);

  /**
   * \brief Clear the buffer of NR SL expected transport block
   */
  void ClearExpectedSlTb ();

private:
  struct SinrStats
  {
    double sinrAvg {0.0};  //!< Average SINR
    double sinrMin {0.0};  //!< Minimum SINR
  };
  /**
   * \brief Function that is called this SpectrumPhy receives a NR Sidelink
   *        signal from the channel.
   * \param params holds NR Sidelink frame signal parameters structure
   */
  void StartRxSlFrame (Ptr<NrSpectrumSignalParametersSlFrame> params);
  /**
   * \brief End receive Sidelink frame function
   */
  void EndRxSlFrame ();
  /**
   * \brief Function to process received PSCCH signals/messages
   * \param paramIndexes Indexes of received PSCCH signals/messages parameters
   */
  void RxSlPscch (std::vector<uint32_t> paramIndexes);
  /**
   * \brief Function to process received PSSCH signals/messages function
   * \param paramIndexes Indexes of received PSSCH signals/messages parameters
   */
  void RxSlPssch (std::vector<uint32_t> paramIndexes);
  /**
   * \brief Get SINR stats function
   *
   * This method computes an average SINR and a minimum SINR among the RBs in
   * linear scale
   *
   * \param sinr The SINR values
   * \param rbBitMap The vector whose size is equal to the number active RBs
   * \return The SINR stats
   */
  const SinrStats GetSinrStats (const SpectrumValue& sinr, const std::vector<int>& rbBitmap);
  /**
   * \brief Retrieve the SCI stage 2 from the PSSCH packet burst
   * \param pktIndex The index of the packet burst received in \p m_slRxSigParamInfo
   * \return The SCI stage 2 packet
   */
  Ptr<Packet> ReteriveSci2FromPktBurst (uint32_t pktIndex);
  TypeId m_slErrorModelType {Object::GetTypeId()}; //!< Sidelink Error model type by default is NrLteMiErrorModel
  Ptr<NrSlInterference> m_slInterference; //!< the Sidelink interference
  std::vector<SpectrumValue> m_slSinrPerceived; //!< SINR for each NR Sidelink packet received
  std::vector<SpectrumValue> m_slSigPerceived; //!< PSD for each NR Sidelink packet received
  std::vector< SlRxSigParamInfo > m_slRxSigParamInfo; //!< NR Sidelink received signal parameter info
  bool m_dropTbOnRbCollisionEnabled {false}; //!< when true, drop all receptions on colliding RBs regardless SINR value.
  bool m_slDataErrorModelEnabled {true}; //!< whether the phy error model for NR Sidelink DATA is enabled, by default is enabled
  bool m_slCtrlErrorModelEnabled {true}; //!< whether the phy error model for NR Sidelinl CTRL is enabled, by default is enabled
  Ptr<NrAmc> m_slAmc {nullptr}; //!< AMC for SL
  NrPhyRxPscchEndOkCallback m_nrPhyRxPscchEndOkCallback; //!< the callback for the NR SL PHY PSCCH successful reception
  NrPhyRxPsschEndOkCallback m_nrPhyRxPsschEndOkCallback; //!< The callback for the NR SL PHY PSSCH successful reception
  NrPhyRxPsschEndErrorCallback m_nrPhyRxPsschEndErrorCallback; //!< The callback for the NR SL PHY PSSCH unsuccessful reception
  /**
   * \brief typedef for NR SL transport block map per RNTI of TBs which are
   *        expected to be received after successful decoding of SCI stage-1.
   */
  typedef std::unordered_map<uint16_t , SlTransportBlockInfo> SlTransportBlocks;

  SlTransportBlocks m_slTransportBlocks; //!< Map of type SlTransportBlocks

  TracedCallback<SlRxCtrlPacketTraceParams> m_rxPscchTraceUe; //!< trace source for PSCCH reception
  TracedCallback<SlRxDataPacketTraceParams> m_rxPsschTraceUe; //!< trace source for PSSCH reception
};

}


#endif /* NR_SPECTRUM_PHY_H */

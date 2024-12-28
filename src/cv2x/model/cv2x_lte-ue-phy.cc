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
 *          Marco Miozzo <marco.miozzo@cttc.es>
 *          Nicola Baldo <nbaldo@cttc.es>
 * Modified by: NIST (D2D)
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <cfloat>
#include <cmath>
#include <ns3/simulator.h>
#include <ns3/double.h>
#include "cv2x_lte-ue-phy.h"
#include "cv2x_lte-enb-phy.h"
#include "cv2x_lte-net-device.h"
#include "cv2x_lte-ue-net-device.h"
#include "cv2x_lte-enb-net-device.h"
#include "cv2x_lte-spectrum-value-helper.h"
#include "cv2x_lte-amc.h"
#include "cv2x_lte-ue-mac.h"
#include "cv2x_ff-mac-common.h"
#include "cv2x_lte-chunk-processor.h"
#include <ns3/cv2x_lte-common.h>
#include <ns3/pointer.h>
#include <ns3/boolean.h>
#include <ns3/cv2x_lte-ue-power-control.h>
#include "cv2x_lte-radio-bearer-tag.h"
#include <ns3/node.h>
#include <fstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteUePhy");



/**
 * Duration of the data portion of a UL subframe.
 * Equals to "TTI length - 1 symbol length for SRS - margin".
 * The margin is 1 nanosecond and is intended to avoid overlapping simulator
 * events. The duration of one symbol is TTI/14 (rounded). In other words,
 * duration of data portion of UL subframe = 1 ms * (13/14) - 1 ns.
 */
static const Time UL_DATA_DURATION = NanoSeconds (1e6 - 71429 - 1); 

/**
 * Delay from subframe start to transmission of SRS.
 * Equals to "TTI length - 1 symbol for SRS".
 */
static const Time UL_SRS_DELAY_FROM_SUBFRAME_START = NanoSeconds (1e6 - 71429); 




////////////////////////////////////////
// member SAP forwarders
////////////////////////////////////////

/// cv2x_UeMemberLteUePhySapProvider class
class cv2x_UeMemberLteUePhySapProvider : public cv2x_LteUePhySapProvider
{
public:
  /**
   * Constructor
   *
   * \param phy the LTE UE Phy
   */
  cv2x_UeMemberLteUePhySapProvider (cv2x_LteUePhy* phy);

  // inherited from cv2x_LtePhySapProvider
  virtual void SendMacPdu (Ptr<Packet> p);
  virtual void SendLteControlMessage (Ptr<cv2x_LteControlMessage> msg);
  virtual void SendRachPreamble (uint32_t prachId, uint32_t raRnti);

private:
  cv2x_LteUePhy* m_phy; ///< the Phy
};

cv2x_UeMemberLteUePhySapProvider::cv2x_UeMemberLteUePhySapProvider (cv2x_LteUePhy* phy) : m_phy (phy)
{

}

void
cv2x_UeMemberLteUePhySapProvider::SendMacPdu (Ptr<Packet> p)
{
  m_phy->DoSendMacPdu (p);
}

void
cv2x_UeMemberLteUePhySapProvider::SendLteControlMessage (Ptr<cv2x_LteControlMessage> msg)
{
  m_phy->DoSendLteControlMessage (msg);
}

void
cv2x_UeMemberLteUePhySapProvider::SendRachPreamble (uint32_t prachId, uint32_t raRnti)
{
  m_phy->DoSendRachPreamble (prachId, raRnti);
}


////////////////////////////////////////
// cv2x_LteUePhy methods
////////////////////////////////////////

/// Map each of UE PHY states to its string representation.
static const std::string g_uePhyStateName[cv2x_LteUePhy::NUM_STATES] =
{
  "CELL_SEARCH",
  "SYNCHRONIZED"
};

/**
 * \param s The UE PHY state.
 * \return The string representation of the given state.
 */
static inline const std::string & ToString (cv2x_LteUePhy::State s)
{
  return g_uePhyStateName[s];
}


NS_OBJECT_ENSURE_REGISTERED (cv2x_LteUePhy);


cv2x_LteUePhy::cv2x_LteUePhy ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("This constructor should not be called");
}

cv2x_LteUePhy::cv2x_LteUePhy (Ptr<cv2x_LteSpectrumPhy> dlPhy, Ptr<cv2x_LteSpectrumPhy> ulPhy)
  : cv2x_LtePhy (dlPhy, ulPhy),
    m_p10CqiPeriodicity (MilliSeconds (1)),  // ideal behavior
    m_a30CqiPeriodicity (MilliSeconds (1)),  // ideal behavior
    m_uePhySapUser (0),
    m_ueCphySapUser (0),
    m_state (CELL_SEARCH),
    m_subframeNo (0),
    m_rsReceivedPowerUpdated (false),
    m_rsInterferencePowerUpdated (false),
    m_dataInterferencePowerUpdated (false),
    m_pssReceived (false),
    m_ueMeasurementsFilterPeriod (MilliSeconds (200)),
    m_ueMeasurementsFilterLast (MilliSeconds (0)),
    m_rsrpSinrSampleCounter (0),
    m_tFirstScanning(MilliSeconds (0)),
    m_ueSlssScanningInProgress(false),
    m_ueSlssMeasurementInProgress(false),
    m_currNMeasPeriods(0),
    m_currFrameNo(0),
    m_currSubframeNo(0),
    m_resyncRequested(false),
    m_waitingNextScPeriod(false)
{
  m_amc = CreateObject <cv2x_LteAmc> ();
  m_powerControl = CreateObject <cv2x_LteUePowerControl> ();
  m_uePhySapProvider = new cv2x_UeMemberLteUePhySapProvider (this);
  m_ueCphySapProvider = new cv2x_MemberLteUeCphySapProvider<cv2x_LteUePhy> (this);
  m_macChTtiDelay = UL_PUSCH_TTIS_DELAY;

  m_nextScanRdm = CreateObject<UniformRandomVariable> ();

  NS_ASSERT_MSG (Simulator::Now ().GetNanoSeconds () == 0,
                 "Cannot create UE devices after simulation started");

  //Simulator::ScheduleNow (&cv2x_LteUePhy::SubframeIndication, this, 1, 1); //Now it is done in the function SetInitialSubFrameIndication

  Simulator::Schedule (m_ueMeasurementsFilterPeriod, &cv2x_LteUePhy::ReportUeMeasurements, this);

  m_slTxPoolInfo.m_pool = NULL;
  m_slTxPoolInfo.m_currentScPeriod.frameNo = 0;
  m_slTxPoolInfo.m_currentScPeriod.subframeNo = 0;
  m_slTxPoolInfo.m_nextScPeriod.frameNo = 0;
  m_slTxPoolInfo.m_nextScPeriod.subframeNo = 0;

  m_slTxPoolInfoV2x.m_pool = NULL; 
  m_slTxPoolInfoV2x.m_currentFrameInfo.frameNo = 0; 
  m_slTxPoolInfoV2x.m_currentFrameInfo.subframeNo = 0;
  
  m_discTxPools.m_pool = NULL;
  m_discTxPools.m_currentDiscPeriod.frameNo = 0;
  m_discTxPools.m_currentDiscPeriod.subframeNo = 0;
  m_discTxPools.m_nextDiscPeriod.frameNo = 0;
  m_discTxPools.m_nextDiscPeriod.subframeNo = 0;

  m_discRxApps.clear (); 
  m_discTxApps.clear ();

  DoReset ();
}


cv2x_LteUePhy::~cv2x_LteUePhy ()
{
  m_txModeGain.clear ();
}

void
cv2x_LteUePhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_uePhySapProvider;
  delete m_ueCphySapProvider;
  if (m_sidelinkSpectrumPhy)
    {
      m_sidelinkSpectrumPhy->Dispose ();
      m_sidelinkSpectrumPhy = 0;
    }
  cv2x_LtePhy::DoDispose ();
}



TypeId
cv2x_LteUePhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteUePhy")
    .SetParent<cv2x_LtePhy> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteUePhy> ()
    .AddAttribute ("TxPower",
                   "Transmission power in dBm",
                   DoubleValue (10.0),
                   MakeDoubleAccessor (&cv2x_LteUePhy::SetTxPower, 
                                       &cv2x_LteUePhy::GetTxPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("NoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                   " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to the noise output of an "
                   " ideal receiver with the same overall gain and bandwidth when the receivers "
                   " are connected to sources at the standard noise temperature T0.\" "
                   "In this model, we consider T0 = 290K.",
                   DoubleValue (9.0),
                   MakeDoubleAccessor (&cv2x_LteUePhy::SetNoiseFigure, 
                                       &cv2x_LteUePhy::GetNoiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxMode1Gain",
                   "Transmission mode 1 gain in dB",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_LteUePhy::SetTxMode1Gain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxMode2Gain",
                   "Transmission mode 2 gain in dB",
                   DoubleValue (4.2),
                   MakeDoubleAccessor (&cv2x_LteUePhy::SetTxMode2Gain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxMode3Gain",
                   "Transmission mode 3 gain in dB",
                   DoubleValue (-2.8),
                   MakeDoubleAccessor (&cv2x_LteUePhy::SetTxMode3Gain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxMode4Gain",
                   "Transmission mode 4 gain in dB",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_LteUePhy::SetTxMode4Gain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxMode5Gain",
                   "Transmission mode 5 gain in dB",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_LteUePhy::SetTxMode5Gain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxMode6Gain",
                   "Transmission mode 6 gain in dB",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_LteUePhy::SetTxMode6Gain),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxMode7Gain",
                   "Transmission mode 7 gain in dB",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&cv2x_LteUePhy::SetTxMode7Gain),
                   MakeDoubleChecker<double> ())
    .AddTraceSource ("ReportCurrentCellRsrpSinr",
                     "RSRP and SINR statistics.",
                     MakeTraceSourceAccessor (&cv2x_LteUePhy::m_reportCurrentCellRsrpSinrTrace),
                     "ns3::cv2x_LteUePhy::RsrpSinrTracedCallback")
    .AddAttribute ("RsrpSinrSamplePeriod",
                   "The sampling period for reporting RSRP-SINR stats (default value 1)",
                   UintegerValue (1),
                   MakeUintegerAccessor (&cv2x_LteUePhy::m_rsrpSinrSamplePeriod),
                   MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("UlPhyTransmission",
                     "DL transmission PHY layer statistics.",
                     MakeTraceSourceAccessor (&cv2x_LteUePhy::m_ulPhyTransmission),
                     "ns3::cv2x_PhyTransmissionStatParameters::TracedCallback")
    .AddAttribute ("DlSpectrumPhy",
                   "The downlink cv2x_LteSpectrumPhy associated to this cv2x_LtePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteUePhy::GetDlSpectrumPhy),
                   MakePointerChecker <cv2x_LteSpectrumPhy> ())
    .AddAttribute ("UlSpectrumPhy",
                   "The uplink cv2x_LteSpectrumPhy associated to this cv2x_LtePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteUePhy::GetUlSpectrumPhy),
                   MakePointerChecker <cv2x_LteSpectrumPhy> ())
    .AddAttribute ("SlSpectrumPhy",
                   "The uplink cv2x_LteSpectrumPhy associated to this cv2x_LtePhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteUePhy::GetSlSpectrumPhy),
                   MakePointerChecker <cv2x_LteSpectrumPhy> ())
    .AddAttribute ("RsrqUeMeasThreshold",
                   "Receive threshold for PSS on RSRQ [dB]",
                   DoubleValue (-1000.0),
                   MakeDoubleAccessor (&cv2x_LteUePhy::m_pssReceptionThreshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RsrpUeMeasThreshold",
                   "Receive threshold for RSRP [dB]",
                   DoubleValue (-1000.0), //to avoid changing the default behavior, make it low so that it acts as if it was not used
                   MakeDoubleAccessor (&cv2x_LteUePhy::m_rsrpReceptionThreshold),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("UeMeasurementsFilterPeriod",
                   "Time period for reporting UE measurements, i.e., the"
                   "length of layer-1 filtering.",
                   TimeValue (MilliSeconds (200)),
                   MakeTimeAccessor (&cv2x_LteUePhy::m_ueMeasurementsFilterPeriod),
                   MakeTimeChecker ())
    .AddTraceSource ("ReportUeMeasurements",
                     "Report UE measurements RSRP (dBm) and RSRQ (dB).",
                     MakeTraceSourceAccessor (&cv2x_LteUePhy::m_reportUeMeasurements),
                     "ns3::cv2x_LteUePhy::RsrpRsrqTracedCallback")
    .AddTraceSource ("StateTransition",
                     "Trace fired upon every UE PHY state transition",
                     MakeTraceSourceAccessor (&cv2x_LteUePhy::m_stateTransitionTrace),
                     "ns3::cv2x_LteUePhy::StateTracedCallback")
    .AddAttribute ("EnableUplinkPowerControl",
                   "If true, Uplink Power Control will be enabled.",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_LteUePhy::m_enableUplinkPowerControl),
                   MakeBooleanChecker ())
    .AddAttribute ("UeSlssInterScanningPeriodMax",
                   "The upper bound of the uniform random variable for the interval between SyncRef selection processes",
                   TimeValue(MilliSeconds(2000)),
                   MakeTimeAccessor(&cv2x_LteUePhy::SetUeSlssInterScanningPeriodMax),
                   MakeTimeChecker())
    .AddAttribute ("UeSlssInterScanningPeriodMin",
                   "The lower bound of the uniform random variable for the interval between SyncRef selection processes",
                   TimeValue(MilliSeconds(2000)),
                   MakeTimeAccessor(&cv2x_LteUePhy::SetUeSlssInterScanningPeriodMin),
                   MakeTimeChecker())
    .AddAttribute ("UeSlssScanningPeriod",
                   "How long the UE will search for SyncRefs (scanning)",
                   TimeValue(MilliSeconds(40)),
                   MakeTimeAccessor(&cv2x_LteUePhy::m_ueSlssScanningPeriod),
                   MakeTimeChecker())
    .AddAttribute ("UeSlssMeasurementPeriod",
                   "How long the UE will perform SLSS L1 measurements for SyncRef selection (measurement)",
                   TimeValue(MilliSeconds(400)),
                   MakeTimeAccessor(&cv2x_LteUePhy::m_ueSlssMeasurementPeriod),
                   MakeTimeChecker())
    .AddAttribute ("UeSlssEvaluationPeriod",
                   "How long the UE will perform SLSS L1 measurements to determine cease/initiation of SLSS transmission (evaluation)",
                   TimeValue(MilliSeconds(800)),
                   MakeTimeAccessor(&cv2x_LteUePhy::m_ueSlssEvaluationPeriod),
                   MakeTimeChecker())
    .AddAttribute ("NSamplesSrsrpMeas",
                   "The maximum number of samples to take during SLSS L1 measurements for each SyncRef",
                   UintegerValue (4),
                   MakeUintegerAccessor (&cv2x_LteUePhy::m_nSamplesSrsrpMeas),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("UeRandomInitialSubframeIndication",
                   "If True, the first frame and subframe values (beginning of the simulation) are chosen randomly, if False they are fixed to 1,1 respectively",
                   BooleanValue(false),
                   MakeBooleanAccessor(&cv2x_LteUePhy::SetInitialSubFrameIndication),
                   MakeBooleanChecker())
    .AddAttribute ("MinSrsrp",
                   "The minimum S-RSRP required to consider a SyncRef detectable",
                   DoubleValue(-125),
                   MakeDoubleAccessor (&cv2x_LteUePhy::m_minSrsrp),
                   MakeDoubleChecker<double>())
    //discovery
    .AddTraceSource ("DiscoveryAnnouncement",
                     "trace to track the announcement of discovery messages",
                     MakeTraceSourceAccessor (&cv2x_LteUePhy::m_discoveryAnnouncementTrace),
                     "ns3::cv2x_LteUePhy::DiscoveryAnnouncementTracedCallback")
    .AddTraceSource ("SidelinkV2xAnnouncement",
                     "trace to track the announcement of Sidelink messages",
                     MakeTraceSourceAccessor (&cv2x_LteUePhy::m_sidelinkV2xAnnouncementTrace),
                     "ns3::cv2x_LteUePhy::SidelinkV2xAnnouncementTracedCallback")
    //enable V2X
    .AddAttribute ("EnableV2x",
                   "If true, V2X transmission will be enabled.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&cv2x_LteUePhy::m_v2xEnabled),
                   MakeBooleanChecker ())
  ;
  return tid;
}

void
cv2x_LteUePhy::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  bool haveNodeId = false;
  uint32_t nodeId = 0;
  if (m_netDevice != 0)
    {
      Ptr<Node> node = m_netDevice->GetNode ();
      if (node != 0)
        {
          nodeId = node->GetId ();
          haveNodeId = true;
        }
    }
  if (haveNodeId)
    {
      Simulator::ScheduleWithContext (nodeId, Seconds (0), &cv2x_LteUePhy::SubframeIndication, this, 1, 1);
    }
  else
    {
      Simulator::ScheduleNow (&cv2x_LteUePhy::SubframeIndication, this, 1, 1);
    }  
  cv2x_LtePhy::DoInitialize ();
}

void
cv2x_LteUePhy::SetLteUePhySapUser (cv2x_LteUePhySapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_uePhySapUser = s;
}

cv2x_LteUePhySapProvider*
cv2x_LteUePhy::GetLteUePhySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return (m_uePhySapProvider);
}


void
cv2x_LteUePhy::SetLteUeCphySapUser (cv2x_LteUeCphySapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_ueCphySapUser = s;
}

cv2x_LteUeCphySapProvider*
cv2x_LteUePhy::GetLteUeCphySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return (m_ueCphySapProvider);
}

void
cv2x_LteUePhy::SetNoiseFigure (double nf)
{
  NS_LOG_FUNCTION (this << nf);
  m_noiseFigure = nf;
}

double
cv2x_LteUePhy::GetNoiseFigure () const
{
  NS_LOG_FUNCTION (this);
  return m_noiseFigure;
}

void
cv2x_LteUePhy::SetTxPower (double pow)
{
  NS_LOG_FUNCTION (this << pow);
  m_txPower = pow;
  m_powerControl->SetTxPower (pow);
}

double
cv2x_LteUePhy::GetTxPower () const
{
  NS_LOG_FUNCTION (this);
  return m_txPower;
}

Ptr<cv2x_LteUePowerControl>
cv2x_LteUePhy::GetUplinkPowerControl () const
{
  NS_LOG_FUNCTION (this);
  return m_powerControl;
}

uint8_t
cv2x_LteUePhy::GetMacChDelay (void) const
{
  return (m_macChTtiDelay);
}

Ptr<cv2x_LteSpectrumPhy>
cv2x_LteUePhy::GetDlSpectrumPhy () const
{
  return m_downlinkSpectrumPhy;
}

Ptr<cv2x_LteSpectrumPhy>
cv2x_LteUePhy::GetUlSpectrumPhy () const
{
  return m_uplinkSpectrumPhy;
}

void
cv2x_LteUePhy::SetSlSpectrumPhy (Ptr<cv2x_LteSpectrumPhy> phy) 
{
  m_sidelinkSpectrumPhy = phy;
}

Ptr<cv2x_LteSpectrumPhy>
cv2x_LteUePhy::GetSlSpectrumPhy () const
{
  return m_sidelinkSpectrumPhy;
}

void
cv2x_LteUePhy::DoSendMacPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);

  SetMacPdu (p);
}

void
cv2x_LteUePhy::PhyPduReceived (Ptr<Packet> p)
{
  m_uePhySapUser->ReceivePhyPdu (p);
}

void
cv2x_LteUePhy::SetSubChannelsForTransmission (std::vector <int> mask)
{
  NS_LOG_FUNCTION (this);

  m_subChannelsForTransmission = mask;

  Ptr<SpectrumValue> txPsd = CreateTxPowerSpectralDensity ();
  m_uplinkSpectrumPhy->SetTxPowerSpectralDensity (txPsd);
}

void
cv2x_LteUePhy::SetSubChannelsForReception (std::vector <int> mask)
{
  NS_LOG_FUNCTION (this);
  m_subChannelsForReception = mask;
}

std::vector <int>
cv2x_LteUePhy::GetSubChannelsForTransmission ()
{
  NS_LOG_FUNCTION (this);
  return m_subChannelsForTransmission;
}

std::vector <int>
cv2x_LteUePhy::GetSubChannelsForReception ()
{
  NS_LOG_FUNCTION (this);
  return m_subChannelsForReception;
}

Ptr<SpectrumValue>
cv2x_LteUePhy::CreateTxPowerSpectralDensity ()
{
  NS_LOG_FUNCTION (this);
  cv2x_LteSpectrumValueHelper psdHelper;
  Ptr<SpectrumValue> psd = psdHelper.CreateUlTxPowerSpectralDensity (m_ulEarfcn, m_ulBandwidth, m_txPower, GetSubChannelsForTransmission ());

  return psd;
}

void
cv2x_LteUePhy::GenerateCtrlCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this);
  
  GenerateCqiRsrpRsrq (sinr);
}


void
cv2x_LteUePhy::GenerateCqiRsrpRsrq (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this << sinr);

  NS_ASSERT (m_state != CELL_SEARCH);
  NS_ASSERT (m_cellId > 0);

  if (m_dlConfigured && m_ulConfigured && (m_rnti > 0))
    {
      // check periodic wideband CQI
      if (Simulator::Now () > m_p10CqiLast + m_p10CqiPeriodicity)
        {
          Ptr<cv2x_LteUeNetDevice> thisDevice = GetDevice ()->GetObject<cv2x_LteUeNetDevice> ();
          Ptr<cv2x_DlCqiLteControlMessage> msg = CreateDlCqiFeedbackMessage (sinr);
          if (msg)
            {
              DoSendLteControlMessage (msg);
            }
          m_p10CqiLast = Simulator::Now ();
        }
      // check aperiodic high-layer configured subband CQI
      if  (Simulator::Now () > m_a30CqiLast + m_a30CqiPeriodicity)
        {
          Ptr<cv2x_LteUeNetDevice> thisDevice = GetDevice ()->GetObject<cv2x_LteUeNetDevice> ();
          Ptr<cv2x_DlCqiLteControlMessage> msg = CreateDlCqiFeedbackMessage (sinr);
          if (msg)
            {
              DoSendLteControlMessage (msg);
            }
          m_a30CqiLast = Simulator::Now ();
        }
    }

  // Generate PHY trace
  m_rsrpSinrSampleCounter++;
  if (m_rsrpSinrSampleCounter==m_rsrpSinrSamplePeriod)
    {
      NS_ASSERT_MSG (m_rsReceivedPowerUpdated, " RS received power info obsolete");
      // RSRP evaluated as averaged received power among RBs
      double sum = 0.0;
      uint8_t rbNum = 0;
      Values::const_iterator it;
      for (it = m_rsReceivedPower.ConstValuesBegin (); it != m_rsReceivedPower.ConstValuesEnd (); it++)
        {
          // convert PSD [W/Hz] to linear power [W] for the single RE
          // we consider only one RE for the RS since the channel is 
          // flat within the same RB 
          double powerTxW = ((*it) * 180000.0) / 12.0;
          sum += powerTxW;
          rbNum++;
        }
      double rsrp = (rbNum > 0) ? (sum / rbNum) : DBL_MAX;
      // averaged SINR among RBs
      sum = 0.0;
      rbNum = 0;
      for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
        {
          sum += (*it);
          rbNum++;
        }
      double avSinr = (rbNum > 0) ? (sum / rbNum) : DBL_MAX;
      NS_LOG_INFO (this << " cellId " << m_cellId << " rnti " << m_rnti << " RSRP " << rsrp << " SINR " << avSinr << " cv2x_ComponentCarrierId " << (uint16_t) m_componentCarrierId);

      m_reportCurrentCellRsrpSinrTrace (m_cellId, m_rnti, rsrp, avSinr, (uint16_t) m_componentCarrierId);
      m_rsrpSinrSampleCounter = 0;
    }

  if (m_pssReceived)
    {
      // measure instantaneous RSRQ now
      NS_ASSERT_MSG (m_rsInterferencePowerUpdated, " RS interference power info obsolete");

      std::list <PssElement>::iterator itPss = m_pssList.begin ();
      while (itPss != m_pssList.end ())
        {
          uint16_t rbNum = 0;
          double rssiSum = 0.0;

          Values::const_iterator itIntN = m_rsInterferencePower.ConstValuesBegin ();
          Values::const_iterator itPj = m_rsReceivedPower.ConstValuesBegin ();
          for (itPj = m_rsReceivedPower.ConstValuesBegin ();
               itPj != m_rsReceivedPower.ConstValuesEnd ();
               itIntN++, itPj++)
            {
              rbNum++;
              // convert PSD [W/Hz] to linear power [W] for the single RE
              double interfPlusNoisePowerTxW = ((*itIntN) * 180000.0) / 12.0;
              double signalPowerTxW = ((*itPj) * 180000.0) / 12.0;
              rssiSum += (2 * (interfPlusNoisePowerTxW + signalPowerTxW));
            }

          NS_ASSERT (rbNum == (*itPss).nRB);
          double rsrq_dB = 10 * log10 ((*itPss).pssPsdSum / rssiSum);

          if (rsrq_dB > m_pssReceptionThreshold)
            {
              NS_LOG_INFO (this << " PSS RNTI " << m_rnti << " cellId " << m_cellId
                                << " has RSRQ " << rsrq_dB << " and RBnum " << rbNum);
              // store measurements
              std::map <uint16_t, UeMeasurementsElement>::iterator itMeasMap;
              itMeasMap = m_ueMeasurementsMap.find ((*itPss).cellId);
              if (itMeasMap != m_ueMeasurementsMap.end ())
                {
                  (*itMeasMap).second.rsrqSum += rsrq_dB;
                  (*itMeasMap).second.rsrqNum++;
                }
              else
                {
                  NS_LOG_WARN ("race condition of bug 2091 occurred");
                }
            }

          itPss++;

        } // end of while (itPss != m_pssList.end ())

      m_pssList.clear ();

    } // end of if (m_pssReceived)

} // end of void cv2x_LteUePhy::GenerateCtrlCqiReport (const SpectrumValue& sinr)

void
cv2x_LteUePhy::GenerateDataCqiReport (const SpectrumValue& sinr)
{
  // Not used by UE, CQI are based only on RS
}

void
cv2x_LteUePhy::GenerateMixedCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_state != CELL_SEARCH);
  NS_ASSERT (m_cellId > 0);

  SpectrumValue mixedSinr = (m_rsReceivedPower * m_paLinear);
  if (m_dataInterferencePowerUpdated)
    {
      // we have a measurement of interf + noise for the denominator
      // of SINR = S/(I+N)
      mixedSinr /= m_dataInterferencePower;
      m_dataInterferencePowerUpdated = false;
      NS_LOG_LOGIC ("data interf measurement available, SINR = " << mixedSinr);
    }
  else
    {
      // we did not see any interference on data, so interference is
      // there and we have only noise at the denominator of SINR
      mixedSinr /= (*m_noisePsd);
      NS_LOG_LOGIC ("no data interf measurement available, SINR = " << mixedSinr);
    }

  /*
   * some RBs are not used in PDSCH and their SINR is very high
   * for example with bandwidth 25, last RB is not used
   * it can make avgSinr value very high, what is incorrect
   */
  uint32_t rbgSize = GetRbgSize ();
  uint32_t modulo = m_dlBandwidth % rbgSize;
  double avgMixedSinr = 0;
  uint32_t usedRbgNum = 0;
  for(uint32_t i = 0; i < (m_dlBandwidth-1-modulo); i++) 
    {
      usedRbgNum++;
      avgMixedSinr+=mixedSinr[i];
    }
  avgMixedSinr = avgMixedSinr/usedRbgNum;
  for(uint32_t i = 0; i < modulo; i++) 
    {
      mixedSinr[m_dlBandwidth-1-i] = avgMixedSinr;
    }

  GenerateCqiRsrpRsrq (mixedSinr);
}

void
cv2x_LteUePhy::ReportInterference (const SpectrumValue& interf)
{
  NS_LOG_FUNCTION (this << interf);
  m_rsInterferencePowerUpdated = true;
  m_rsInterferencePower = interf;
}

void
cv2x_LteUePhy::ReportDataInterference (const SpectrumValue& interf)
{
  NS_LOG_FUNCTION (this << interf);

  m_dataInterferencePowerUpdated = true;
  m_dataInterferencePower = interf;
}

void
cv2x_LteUePhy::ReportRsReceivedPower (const SpectrumValue& power)
{
  NS_LOG_FUNCTION (this << power);
  m_rsReceivedPowerUpdated = true;
  m_rsReceivedPower = power;

  if (m_enableUplinkPowerControl)
    {
      double sum = 0;
      uint32_t rbNum = 0;
      Values::const_iterator it;
      for (it = m_rsReceivedPower.ConstValuesBegin (); it != m_rsReceivedPower.ConstValuesEnd (); it++)
        {
          double powerTxW = ((*it) * 180000);
          sum += powerTxW;
          rbNum++;
        }
      double rsrp = 10 * log10 (sum) + 30;

      NS_LOG_INFO ("RSRP: " << rsrp);
      m_powerControl->SetRsrp (rsrp);
    }
}

Ptr<cv2x_DlCqiLteControlMessage>
cv2x_LteUePhy::CreateDlCqiFeedbackMessage (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this);


  // apply transmission mode gain
  NS_ASSERT (m_transmissionMode < m_txModeGain.size ());
  SpectrumValue newSinr = sinr;
  newSinr *= m_txModeGain.at (m_transmissionMode);

  // CREATE cv2x_DlCqiLteControlMessage
  Ptr<cv2x_DlCqiLteControlMessage> msg = Create<cv2x_DlCqiLteControlMessage> ();
  cv2x_CqiListElement_s dlcqi;
  std::vector<int> cqi;
  if (Simulator::Now () > m_p10CqiLast + m_p10CqiPeriodicity)
    {
      cqi = m_amc->CreateCqiFeedbacks (newSinr, m_dlBandwidth);

      int nLayer = cv2x_TransmissionModesLayers::TxMode2LayerNum (m_transmissionMode);
      int nbSubChannels = cqi.size ();
      double cqiSum = 0.0;
      int activeSubChannels = 0;
      // average the CQIs of the different RBs
      for (int i = 0; i < nbSubChannels; i++)
        {
          if (cqi.at (i) != -1)
            {
              cqiSum += cqi.at (i);
              activeSubChannels++;
            }
          NS_LOG_DEBUG (this << " subch " << i << " cqi " <<  cqi.at (i));
        }
      dlcqi.m_rnti = m_rnti;
      dlcqi.m_ri = 1; // not yet used
      dlcqi.m_cqiType = cv2x_CqiListElement_s::P10; // Periodic CQI using PUCCH wideband
      NS_ASSERT_MSG (nLayer > 0, " nLayer negative");
      NS_ASSERT_MSG (nLayer < 3, " nLayer limit is 2s");
      for (int i = 0; i < nLayer; i++)
        {
          if (activeSubChannels > 0)
            {
              dlcqi.m_wbCqi.push_back ((uint16_t) cqiSum / activeSubChannels);
            }
          else
            {
              // approximate with the worst case -> CQI = 1
              dlcqi.m_wbCqi.push_back (1);
            }
        }
      //NS_LOG_DEBUG (this << " Generate P10 CQI feedback " << (uint16_t) cqiSum / activeSubChannels);
      dlcqi.m_wbPmi = 0; // not yet used
      // dl.cqi.m_sbMeasResult others CQI report modes: not yet implemented
    }
  else if (Simulator::Now () > m_a30CqiLast + m_a30CqiPeriodicity)
    {
      cqi = m_amc->CreateCqiFeedbacks (newSinr, GetRbgSize ());
      int nLayer = cv2x_TransmissionModesLayers::TxMode2LayerNum (m_transmissionMode);
      int nbSubChannels = cqi.size ();
      int rbgSize = GetRbgSize ();
      double cqiSum = 0.0;
      int cqiNum = 0;
      cv2x_SbMeasResult_s rbgMeas;
      //NS_LOG_DEBUG (this << " Create A30 CQI feedback, RBG " << rbgSize << " cqiNum " << nbSubChannels << " band "  << (uint16_t)m_dlBandwidth);
      for (int i = 0; i < nbSubChannels; i++)
        {
          if (cqi.at (i) != -1)
            {
              cqiSum += cqi.at (i);
            }
          // else "nothing" no CQI is treated as CQI = 0 (worst case scenario)
          cqiNum++;
          if (cqiNum == rbgSize)
            {
              // average the CQIs of the different RBGs
              //NS_LOG_DEBUG (this << " RBG CQI "  << (uint16_t) cqiSum / rbgSize);
              cv2x_HigherLayerSelected_s hlCqi;
              hlCqi.m_sbPmi = 0; // not yet used
              for (int i = 0; i < nLayer; i++)
                {
                  hlCqi.m_sbCqi.push_back ((uint16_t) cqiSum / rbgSize);
                }
              rbgMeas.m_higherLayerSelected.push_back (hlCqi);
              cqiSum = 0.0;
              cqiNum = 0;
            }
        }
      dlcqi.m_rnti = m_rnti;
      dlcqi.m_ri = 1; // not yet used
      dlcqi.m_cqiType = cv2x_CqiListElement_s::A30; // Aperidic CQI using PUSCH
      //dlcqi.m_wbCqi.push_back ((uint16_t) cqiSum / nbSubChannels);
      dlcqi.m_wbPmi = 0; // not yet used
      dlcqi.m_sbMeasResult = rbgMeas;
    }

  msg->SetDlCqi (dlcqi);
  return msg;
}


void
cv2x_LteUePhy::ReportUeMeasurements ()
{
  NS_LOG_FUNCTION (this << Simulator::Now ());
  NS_LOG_DEBUG (this << " Report UE Measurements ");

  cv2x_LteUeCphySapUser::UeMeasurementsParameters ret;

  std::map <uint16_t, UeMeasurementsElement>::iterator it;
  for (it = m_ueMeasurementsMap.begin (); it != m_ueMeasurementsMap.end (); it++)
    {
      double avg_rsrp = (*it).second.rsrpSum / (double)(*it).second.rsrpNum;
      double avg_rsrq = (*it).second.rsrqSum / (double)(*it).second.rsrqNum;
      /*
       * In CELL_SEARCH state, this may result in avg_rsrq = 0/0 = -nan.
       * UE RRC must take this into account when receiving measurement reports.
       * TODO remove this shortcoming by calculating RSRQ during CELL_SEARCH
       */
      NS_LOG_DEBUG (this << " CellId " << (*it).first
                         << " RSRP " << avg_rsrp
                         << " (nSamples " << (uint16_t)(*it).second.rsrpNum << ")"
                         << " RSRQ " << avg_rsrq
                         << " (nSamples " << (uint16_t)(*it).second.rsrqNum << ")"
                         << " cv2x_ComponentCarrierID " << (uint16_t)m_componentCarrierId);
      if (avg_rsrp >= m_rsrpReceptionThreshold)
      {
        cv2x_LteUeCphySapUser::UeMeasurementsElement newEl;
        newEl.m_cellId = (*it).first;
        newEl.m_rsrp = avg_rsrp;
        newEl.m_rsrq = avg_rsrq;
        ret.m_ueMeasurementsList.push_back (newEl);
        ret.m_componentCarrierId = m_componentCarrierId;
      }

      // report to UE measurements trace
      m_reportUeMeasurements (m_rnti, (*it).first, avg_rsrp, avg_rsrq, ((*it).first == m_cellId ? 1 : 0), m_componentCarrierId);
    }

  // report to RRC
  m_ueCphySapUser->ReportUeMeasurements (ret);

  m_ueMeasurementsMap.clear ();
  Simulator::Schedule (m_ueMeasurementsFilterPeriod, &cv2x_LteUePhy::ReportUeMeasurements, this);
}

void
cv2x_LteUePhy::DoSendLteControlMessage (Ptr<cv2x_LteControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);

  SetControlMessages (msg);
}

void 
cv2x_LteUePhy::DoSendRachPreamble (uint32_t raPreambleId, uint32_t raRnti)
{
  NS_LOG_FUNCTION (this << raPreambleId);

  // unlike other control messages, RACH preamble is sent ASAP
  Ptr<cv2x_RachPreambleLteControlMessage> msg = Create<cv2x_RachPreambleLteControlMessage> ();
  msg->SetRapId (raPreambleId);
  m_raPreambleId = raPreambleId;
  m_raRnti = raRnti;
  m_controlMessagesQueue.at (0).push_back (msg);
}


void
cv2x_LteUePhy::ReceiveLteControlMessageList (std::list<Ptr<cv2x_LteControlMessage> > msgList)
{
  NS_LOG_FUNCTION (this);

  std::list<Ptr<cv2x_LteControlMessage> >::iterator it;
  NS_LOG_DEBUG (this << " I am rnti = " << m_rnti << " and I received msgs " << (uint16_t) msgList.size ());
  for (it = msgList.begin (); it != msgList.end (); it++)
    {
      Ptr<cv2x_LteControlMessage> msg = (*it);

      if (msg->GetMessageType () == cv2x_LteControlMessage::DL_DCI)
        {
          Ptr<cv2x_DlDciLteControlMessage> msg2 = DynamicCast<cv2x_DlDciLteControlMessage> (msg);

          cv2x_DlDciListElement_s dci = msg2->GetDci ();
          if (dci.m_rnti != m_rnti)
            {
              // DCI not for me
              continue;
            }

          if (dci.m_resAlloc != 0)
            {
              NS_FATAL_ERROR ("Resource Allocation type not implemented");
            }

          std::vector <int> dlRb;

          // translate the DCI to Spectrum framework
          uint32_t mask = 0x1;
          for (int i = 0; i < 32; i++)
            {
              if (((dci.m_rbBitmap & mask) >> i) == 1)
                {
                  for (int k = 0; k < GetRbgSize (); k++)
                    {
                      dlRb.push_back ((i * GetRbgSize ()) + k);
//             NS_LOG_DEBUG(this << " RNTI " << m_rnti << " RBG " << i << " DL-DCI allocated PRB " << (i*GetRbgSize()) + k);
                    }
                }
              mask = (mask << 1);
            }
          if (m_enableUplinkPowerControl)
            {
              m_powerControl->ReportTpc (dci.m_tpc);
            }


          // send TB info to cv2x_LteSpectrumPhy
          NS_LOG_DEBUG (this << " UE " << m_rnti << " DL-DCI " << dci.m_rnti << " bitmap "  << dci.m_rbBitmap);
          for (uint8_t i = 0; i < dci.m_tbsSize.size (); i++)
            {
              m_downlinkSpectrumPhy->AddExpectedTb (dci.m_rnti, dci.m_ndi.at (i), dci.m_tbsSize.at (i), dci.m_mcs.at (i), dlRb, i, dci.m_harqProcess, dci.m_rv.at (i), true /* DL */);
            }

          SetSubChannelsForReception (dlRb);


        }
      else if (msg->GetMessageType () == cv2x_LteControlMessage::UL_DCI)
        {
          // set the uplink bandwidth according to the UL-CQI
          Ptr<cv2x_UlDciLteControlMessage> msg2 = DynamicCast<cv2x_UlDciLteControlMessage> (msg);
          cv2x_UlDciListElement_s dci = msg2->GetDci ();
          if (dci.m_rnti != m_rnti)
            {
              // DCI not for me
              continue;
            }
          NS_LOG_INFO (this << " UL DCI");
          std::vector <int> ulRb;
          for (int i = 0; i < dci.m_rbLen; i++)
            {
              ulRb.push_back (i + dci.m_rbStart);
              //NS_LOG_DEBUG (this << " UE RB " << i + dci.m_rbStart);
            }
          QueueSubChannelsForTransmission (ulRb);
          // fire trace of UL Tx PHY stats
          cv2x_HarqProcessInfoList_t harqInfoList = m_harqPhyModule->GetHarqProcessInfoUl (m_rnti, 0);
          cv2x_PhyTransmissionStatParameters params;
          params.m_cellId = m_cellId;
          params.m_imsi = 0; // it will be set by DlPhyTransmissionCallback in cv2x_LteHelper
          params.m_timestamp = Simulator::Now ().GetMilliSeconds () + UL_PUSCH_TTIS_DELAY;
          params.m_rnti = m_rnti;
          params.m_txMode = 0; // always SISO for UE
          params.m_layer = 0;
          params.m_mcs = dci.m_mcs;
          params.m_size = dci.m_tbSize;
          params.m_rv = harqInfoList.size ();
          params.m_ndi = dci.m_ndi;
          params.m_ccId = m_componentCarrierId;
          m_ulPhyTransmission (params);
          // pass the info to the MAC
          m_uePhySapUser->ReceiveLteControlMessage (msg);
        }
      else if (msg->GetMessageType () == cv2x_LteControlMessage::RAR)
        {
          Ptr<cv2x_RarLteControlMessage> rarMsg = DynamicCast<cv2x_RarLteControlMessage> (msg);
          if (rarMsg->GetRaRnti () == m_raRnti)
            {
              for (std::list<cv2x_RarLteControlMessage::Rar>::const_iterator it = rarMsg->RarListBegin (); it != rarMsg->RarListEnd (); ++it)
                {
                  if (it->rapId != m_raPreambleId)
                    {
                      // UL grant not for me
                      continue;
                    }
                  else
                    {
                      NS_LOG_INFO ("received RAR RNTI " << m_raRnti);
                      // set the uplink bandwidth according to the UL grant
                      std::vector <int> ulRb;
                      for (int i = 0; i < it->rarPayload.m_grant.m_rbLen; i++)
                        {
                          ulRb.push_back (i + it->rarPayload.m_grant.m_rbStart);
                        }

                      QueueSubChannelsForTransmission (ulRb);
                      // pass the info to the MAC
                      m_uePhySapUser->ReceiveLteControlMessage (msg);
                      // reset RACH variables with out of range values
                      m_raPreambleId = 255;
                      m_raRnti = 11;
                    }
                }
            }
        }
      else if (msg->GetMessageType () == cv2x_LteControlMessage::MIB)
        {
          NS_LOG_INFO ("received MIB");
          NS_ASSERT (m_cellId > 0);
          Ptr<cv2x_MibLteControlMessage> msg2 = DynamicCast<cv2x_MibLteControlMessage> (msg);
          m_ueCphySapUser->RecvMasterInformationBlock (m_cellId, msg2->GetMib ());
        }
      else if (msg->GetMessageType () == cv2x_LteControlMessage::SIB1)
        {
          NS_LOG_INFO ("received SIB1");
          NS_ASSERT (m_cellId > 0);
          Ptr<Sib1cv2x_LteControlMessage> msg2 = DynamicCast<Sib1cv2x_LteControlMessage> (msg);
          m_ueCphySapUser->RecvSystemInformationBlockType1 (m_cellId, msg2->GetSib1 ());
        }
      else if (msg->GetMessageType () == cv2x_LteControlMessage::SL_DCI)
        {
          Ptr<cv2x_SlDciLteControlMessage> msg2 = DynamicCast<cv2x_SlDciLteControlMessage> (msg);
          cv2x_SlDciListElement_s dci = msg2->GetDci ();
          if (dci.m_rnti != m_rnti)
            {
              // DCI not for me
              continue;
            }
          NS_LOG_INFO ("received SL_DCI");
          m_uePhySapUser->ReceiveLteControlMessage (msg);
        }
      else if (msg->GetMessageType () == cv2x_LteControlMessage::SCI)
        {
          Ptr<cv2x_SciLteControlMessage> msg2 = DynamicCast<cv2x_SciLteControlMessage> (msg);
          cv2x_SciListElement_s sci = msg2->GetSci ();
          //must check if the destination is one to monitor
          std::list <uint32_t>::iterator it;
          bool for_me = false;
          for (it = m_destinations.begin (); it != m_destinations.end () && !for_me; it++)
            {
              if (sci.m_groupDstId == ((*it) & 0xFF))
                {
                  NS_LOG_INFO ("received SCI for group " << (uint32_t)((*it) & 0xFF) << " from rnti " << sci.m_rnti);

                  //todo, how to find the pool among the available ones?
                  //right now just use the first one
                  std::list <PoolInfo>::iterator poolIt = m_sidelinkRxPools.begin();
                  if (poolIt == m_sidelinkRxPools.end())
                    {
                      NS_LOG_INFO (this << " No Rx pool configured");
                    }
                  else
                    {
                      //this is the first transmission of PSCCH
                      std::map<uint16_t, SidelinkGrantInfo>::iterator grantIt = poolIt->m_currentGrants.find (sci.m_rnti);
                      if (grantIt == poolIt->m_currentGrants.end())
                      {
                        SidelinkGrantInfo txInfo;
                    
                        txInfo.m_grant_received = true;
                        txInfo.m_grant.m_rnti = sci.m_rnti;
                        txInfo.m_grant.m_resPscch = sci.m_resPscch;
                        txInfo.m_grant.m_rbStart = sci.m_rbStart;
                        txInfo.m_grant.m_rbLen = sci.m_rbLen;
                        txInfo.m_grant.m_trp = sci.m_trp;
                        txInfo.m_grant.m_groupDstId = sci.m_groupDstId;
                        txInfo.m_grant.m_mcs = sci.m_mcs;
                        txInfo.m_grant.m_tbSize = sci.m_tbSize;

                        //insert grant
                        poolIt->m_currentGrants.insert (std::pair <uint16_t, SidelinkGrantInfo> (sci.m_rnti, txInfo));
                      } // else it should be the transmission and the data should be the same...add check
                      else 
                      {
                        NS_LOG_DEBUG ("SCI Grant already present");
                      }
                    }   
                  //m_uePhySapUser->ReceiveLteControlMessage (msg);
                }             
            }
        }
      else if (msg->GetMessageType() == cv2x_LteControlMessage::SCI_V2X) // SCI-1 received
        {
          Ptr<cv2x_SciLteControlMessageV2x> msg2 = DynamicCast<cv2x_SciLteControlMessageV2x> (msg);
          cv2x_SciListElementV2x sci1 = msg2->GetSci(); 

          NS_LOG_INFO ("received SCI Format 1 from rnti " << sci1.m_rnti);
          std::list<PoolInfoV2x>::iterator poolIt = m_sidelinkRxPoolsV2x.begin();
          
          if (poolIt == m_sidelinkRxPoolsV2x.end())
            {
              NS_LOG_INFO (this << " No Rx pool configured");
            }
          else 
            {
              SidelinkGrantInfoV2x txInfo;
              txInfo.m_grant_received = true;
              txInfo.m_grant.m_rnti = sci1.m_rnti; 
              txInfo.m_grant.m_prio = sci1.m_prio;
              txInfo.m_grant.m_pRsvp = sci1.m_pRsvp;
              txInfo.m_grant.m_riv = sci1.m_riv;  
              txInfo.m_grant.m_sfGap = sci1.m_sfGap; 
              txInfo.m_grant.m_mcs = sci1.m_mcs; 
              txInfo.m_grant.m_reTxIdx = sci1.m_reTxIdx; 
              txInfo.m_grant.m_tbSize = sci1.m_tbSize; 
              txInfo.m_grant.m_resPscch = sci1.m_resPscch;  

              // insert grant
              NS_LOG_LOGIC (this << " insert grant for rnti " << sci1.m_rnti << " with size " << sci1.m_tbSize);
              poolIt->m_currentGrants.insert (std::pair<uint16_t, SidelinkGrantInfoV2x> (sci1.m_rnti,txInfo));              
            }
        }
      else if (msg->GetMessageType() == cv2x_LteControlMessage::MIB_SL)
        {
          Ptr<cv2x_MibSLLteControlMessage> msgMibSL = DynamicCast<cv2x_MibSLLteControlMessage> (msg);
          cv2x_LteRrcSap::MasterInformationBlockSL mibSL = msgMibSL->GetMibSL();

          //Pass the message to the RRC
          m_ueCphySapUser->ReceiveMibSL(mibSL);

          //Store the received MIB-SL during the SyncRef search
          if (m_ueSlssScanningInProgress)
            {
              mibSL.rxOffset = Simulator::Now().GetMilliSeconds() % 40;
              m_detectedMibSl.insert (std::pair <std::pair<uint16_t,uint16_t>, cv2x_LteRrcSap::MasterInformationBlockSL> (std::pair<uint16_t,uint16_t>(mibSL.slssid, mibSL.rxOffset), mibSL));
            }
        }
      //discovery
      else if (msg->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG)
        {
          Ptr<cv2x_SlDiscMessage> msg2 = DynamicCast<cv2x_SlDiscMessage> (msg);
          cv2x_SlDiscMsg disc = msg2->GetSlDiscMessage ();

          NS_LOG_INFO ("received discovery from rnti " << disc.m_rnti << " with resPsdch: " << disc.m_resPsdch);
          m_uePhySapUser->ReceiveLteControlMessage (msg);
        }
      else
        {
          // pass the message to UE-MAC
          m_uePhySapUser->ReceiveLteControlMessage (msg);
        }
    }
}


void
cv2x_LteUePhy::ReceivePss (uint16_t cellId, Ptr<SpectrumValue> p)
{
  NS_LOG_FUNCTION (this << cellId << (*p));

  double sum = 0.0;
  uint16_t nRB = 0;
  Values::const_iterator itPi;
  for (itPi = p->ConstValuesBegin (); itPi != p->ConstValuesEnd (); itPi++)
    {
      // convert PSD [W/Hz] to linear power [W] for the single RE
      double powerTxW = ((*itPi) * 180000.0) / 12.0;
      sum += powerTxW;
      nRB++;
    }

  // measure instantaneous RSRP now
  double rsrp_dBm = 10 * log10 (1000 * (sum / (double)nRB));
  NS_LOG_INFO (this << " PSS RNTI " << m_rnti << " cellId " << m_cellId
                    << " has RSRP " << rsrp_dBm << " and RBnum " << nRB);
  // note that m_pssReceptionThreshold does not apply here

  // store measurements
  std::map <uint16_t, UeMeasurementsElement>::iterator itMeasMap = m_ueMeasurementsMap.find (cellId);
  if (itMeasMap == m_ueMeasurementsMap.end ())
    {
      // insert new entry
      UeMeasurementsElement newEl;
      newEl.rsrpSum = rsrp_dBm;
      newEl.rsrpNum = 1;
      newEl.rsrqSum = 0;
      newEl.rsrqNum = 0;
      m_ueMeasurementsMap.insert (std::pair <uint16_t, UeMeasurementsElement> (cellId, newEl));
    }
  else
    {
      (*itMeasMap).second.rsrpSum += rsrp_dBm;
      (*itMeasMap).second.rsrpNum++;
    }

  /*
   * Collect the PSS for later processing in GenerateCtrlCqiReport()
   * (to be called from ChunkProcessor after RX is finished).
   */
  m_pssReceived = true;
  PssElement el;
  el.cellId = cellId;
  el.pssPsdSum = sum;
  el.nRB = nRB;
  m_pssList.push_back (el);

} // end of void cv2x_LteUePhy::ReceivePss (uint16_t cellId, Ptr<SpectrumValue> p)


void
cv2x_LteUePhy::QueueSubChannelsForTransmission (std::vector <int> rbMap)
{
  m_subChannelsForTransmissionQueue.at (m_macChTtiDelay - 1) = rbMap;
}

void
cv2x_LteUePhy::SubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  NS_LOG_FUNCTION (this << " frame " << frameNo << " subframe " << subframeNo << " rnti " << m_rnti);

  NS_ASSERT_MSG (frameNo > 0, "the SRS index check code assumes that frameNo starts at 1");

  // refresh internal variables
  m_rsReceivedPowerUpdated = false;
  m_rsInterferencePowerUpdated = false;
  m_pssReceived = false;

  // Clear expected TB not received in previous subframes
  m_sidelinkSpectrumPhy->ClearExpectedSlTb();
  m_sidelinkSpectrumPhy->ClearExpectedSlV2xTb();

  //Notify RRC about the current Subframe indication
  m_ueCphySapUser->ReportSubframeIndication(frameNo, subframeNo);

  //If a change of timing (resynchronization) was requested before, do the change of frameNo and subframeNo if possible
  // Do it here for avoiding  miss alignments of subframe indications
  if (m_resyncRequested)
    {
      NS_LOG_LOGIC(this <<" (re)synchronization requested ");
      if(ChangeOfTiming(frameNo, subframeNo) )
        {
          frameNo = m_currFrameNo;
          subframeNo = m_currSubframeNo;
          NS_LOG_LOGIC(this << " (re)synchronization successfully performed ");
        }
      else
        {
          NS_LOG_LOGIC(this <<" (re)synchronization postponed ");
        }
    }

  if (m_ulConfigured)
    {
      if (m_slTxPoolInfo.m_pool)
        {
          //Check if we need to initialize the Tx pool
          if (m_slTxPoolInfo.m_nextScPeriod.frameNo == 0) {
              //pool not initialized yet
              m_slTxPoolInfo.m_nextScPeriod = m_slTxPoolInfo.m_pool->GetNextScPeriod (frameNo, subframeNo);
              //adjust because scheduler starts with frame/subframe = 1
              m_slTxPoolInfo.m_nextScPeriod.frameNo++;
              m_slTxPoolInfo.m_nextScPeriod.subframeNo++;
              NS_LOG_INFO (this << " Tx Pool initialized");
          }
          //Check if this is a new SC period
          //NS_LOG_DEBUG (this << "Checking if beginning of next period " << m_slTxPoolInfo.m_nextScPeriod.frameNo << "/" << m_slTxPoolInfo.m_nextScPeriod.subframeNo);
          if (frameNo == m_slTxPoolInfo.m_nextScPeriod.frameNo && subframeNo == m_slTxPoolInfo.m_nextScPeriod.subframeNo)
            {
              m_slTxPoolInfo.m_currentScPeriod = m_slTxPoolInfo.m_nextScPeriod;
              m_slTxPoolInfo.m_nextScPeriod = m_slTxPoolInfo.m_pool->GetNextScPeriod (frameNo, subframeNo);
              //adjust because scheduler starts with frame/subframe = 1
              m_slTxPoolInfo.m_nextScPeriod.frameNo++;
              m_slTxPoolInfo.m_nextScPeriod.subframeNo++;
              NS_LOG_INFO (this << " Starting new SC period for TX pool " << ". Next period at " << m_slTxPoolInfo.m_nextScPeriod.frameNo << "/" << m_slTxPoolInfo.m_nextScPeriod.subframeNo);

              if (m_waitingNextScPeriod)
                {
                  NS_LOG_LOGIC (this << " the UE was waiting for next SC period and it just started");
                  m_waitingNextScPeriod = false;
                }

              //clear any previous grant
              m_slTxPoolInfo.m_currentGrants.clear();
            }
        }
      if(m_slTxPoolInfoV2x.m_pool)
      {
        //Check if we need to initialize the V2X Tx pool
        if (m_slTxPoolInfoV2x.m_currentFrameInfo.frameNo == 0)
        {
          //pool not initialized yet
          m_slTxPoolInfoV2x.m_currentFrameInfo.subframeNo++;
          m_slTxPoolInfoV2x.m_currentFrameInfo.frameNo++; 
          NS_LOG_INFO (this << " V2X Tx Pool initialized"); 
        }
        //clear any previous grant
        m_slTxPoolInfoV2x.m_currentGrants.clear();
      }
      if (m_discTxPools.m_pool)
      {
        //Check if we need to initialize the discovery Tx pool
        if (m_discTxPools.m_nextDiscPeriod.frameNo == 0) 
        {
          //pool not initialized yet
          m_discTxPools.m_nextDiscPeriod = m_discTxPools.m_pool->GetNextDiscPeriod (frameNo, subframeNo);
          //adjust because scheduler starts with frame/subframe = 1
          m_discTxPools.m_nextDiscPeriod.frameNo++;
          m_discTxPools.m_nextDiscPeriod.subframeNo++;
          NS_LOG_INFO (this << "Discovery Tx Pool initialized");
        }
        //Check if this is a new discovery period
        if (frameNo == m_discTxPools.m_nextDiscPeriod.frameNo && subframeNo == m_discTxPools.m_nextDiscPeriod.subframeNo)
        {
          m_discTxPools.m_currentDiscPeriod = m_discTxPools.m_nextDiscPeriod;
          m_discTxPools.m_nextDiscPeriod = m_discTxPools.m_pool->GetNextDiscPeriod (frameNo, subframeNo);
          //adjust because scheduler starts with frame/subframe = 1
          m_discTxPools.m_nextDiscPeriod.frameNo++;
          m_discTxPools.m_nextDiscPeriod.subframeNo++;
          NS_LOG_INFO (this << " Starting new discovery period for TX pool " << ". Next period at " << m_discTxPools.m_nextDiscPeriod.frameNo << "/" << m_discTxPools.m_nextDiscPeriod.subframeNo);
             
          //clear any previous grant
          m_discTxPools.m_currentGrants.clear();
        }
      }

      //check if we received grants for sidelink
      //compute the reception slots for the PSSCH. Do this here because
      //we did not have access to the frame/subframe no at the reception
      std::list <PoolInfo>::iterator it;
      for (it = m_sidelinkRxPools.begin() ; it != m_sidelinkRxPools.end () ; it++)
        { 
          std::map <uint16_t, SidelinkGrantInfo>::iterator grantIt = it->m_currentGrants.begin();
          while (grantIt != it->m_currentGrants.end())
            {
              std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator rxIt;
              if (grantIt->second.m_grant_received)
                {
                  NS_LOG_INFO (this << " New grant received");
                  //TODO: how to identify pool if multiple are presents?      
                  SidelinkCommResourcePool::SubframeInfo tmp = it->m_pool->GetCurrentScPeriod(frameNo, subframeNo);
                  grantIt->second.m_psschTx = it->m_pool->GetPsschTransmissions (tmp, grantIt->second.m_grant.m_trp, grantIt->second.m_grant.m_rbStart, grantIt->second.m_grant.m_rbLen);

                  for (rxIt = grantIt->second.m_psschTx.begin (); rxIt != grantIt->second.m_psschTx.end (); rxIt++)
                    {
                      //adjust for index starting at 1
                      rxIt->subframe.frameNo++;
                      rxIt->subframe.subframeNo++;
                      NS_LOG_INFO (this << " Subframe Rx" << rxIt->subframe.frameNo << "/" << rxIt->subframe.subframeNo << ": rbStart=" << (uint32_t) rxIt->rbStart << ", rbLen=" << (uint32_t) rxIt->nbRb);
                    }

                  grantIt->second.m_grant_received = false;
                }
              //now check if there is any grant for the current subframe and pass them to lower layer
              rxIt = grantIt->second.m_psschTx.begin();
              if (rxIt != grantIt->second.m_psschTx.end())
                {
                  NS_LOG_DEBUG (frameNo << "/" << subframeNo << " RNTI=" << m_rnti << " next pssch at " << (*rxIt).subframe.frameNo << "/" << (*rxIt).subframe.subframeNo);
                }
              if (rxIt != grantIt->second.m_psschTx.end() && (*rxIt).subframe.frameNo == frameNo && (*rxIt).subframe.subframeNo == subframeNo)
                {
                  //reception
                  NS_LOG_INFO (this << " Expecting PSSCH reception RB " << (uint16_t) grantIt->second.m_grant.m_rbStart << " to " << (uint16_t) (grantIt->second.m_grant.m_rbStart + grantIt->second.m_grant.m_rbLen - 1));
                  std::vector <int> rbMap;
                  for (int i = grantIt->second.m_grant.m_rbStart; i < grantIt->second.m_grant.m_rbStart + grantIt->second.m_grant.m_rbLen; i++)
                    {
                      rbMap.push_back (i);
                    }

                  m_sidelinkSpectrumPhy->AddExpectedTb (grantIt->second.m_grant.m_rnti, grantIt->second.m_grant.m_groupDstId, grantIt->second.m_psschTx.size() % 4 == 0, grantIt->second.m_grant.m_tbSize, grantIt->second.m_grant.m_mcs, rbMap, (4 - grantIt->second.m_psschTx.size () % 4));

                  //remove reception information
                  grantIt->second.m_psschTx.erase (rxIt);

                }
              if (grantIt->second.m_psschTx.size() == 0)
                {
                  //no more PSSCH transmission, clear the grant
                  it->m_currentGrants.erase (grantIt++);
                }
              else {
                  grantIt++;
              }
            }
        }

      //check if we received grants for sidelink
      //compute the reception slots for the PSSCH. Do this here because
      //we did not have access to the frame/subframe no at the reception
      std::list <PoolInfoV2x>::iterator it2; 
      for (it2 = m_sidelinkRxPoolsV2x.begin() ; it2 != m_sidelinkRxPoolsV2x.end (); it2++) 
        { 
          std::map <uint16_t, SidelinkGrantInfoV2x>::iterator grantIt = it2->m_currentGrants.begin();
          while (grantIt != it2->m_currentGrants.end()) 
            {   
              std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator rxIt; 

              if(grantIt->second.m_grant_received) 
                {
                  NS_LOG_INFO (this << " New grant received");
                  SidelinkCommResourcePoolV2x::SubframeInfo subframe; 
                  subframe.subframeNo = subframeNo; 
                  subframe.frameNo = frameNo; 
                  grantIt->second.m_pscchTx = it2->m_pool->GetPscchTransmissions (subframe, grantIt->second.m_grant.m_riv, grantIt->second.m_grant.m_pRsvp, grantIt->second.m_grant.m_sfGap, grantIt->second.m_grant.m_reTxIdx, grantIt->second.m_grant.m_resPscch, 1);
                  grantIt->second.m_psschTx = it2->m_pool->GetPsschTransmissions (subframe, grantIt->second.m_grant.m_riv, grantIt->second.m_grant.m_pRsvp, grantIt->second.m_grant.m_sfGap, grantIt->second.m_grant.m_reTxIdx, grantIt->second.m_grant.m_resPscch, 1);

                  for(rxIt = grantIt->second.m_pscchTx.begin(); rxIt != grantIt->second.m_pscchTx.end(); rxIt++)
                    {
                      //std::cout << "Rnti="<< m_rnti << "\t PHY PSCCH RX at " << rxIt->subframe.frameNo << "/" << rxIt->subframe.subframeNo << "\t rbStart=" << (uint32_t) rxIt->rbStart << "\t rbLen=" << (uint32_t) rxIt->rbLen << << "\t from rnti=" << grantIt->first << std::endl;
                      NS_LOG_INFO (this << " PSCCH Rx " << rxIt->subframe.frameNo << "/"<< rxIt->subframe.subframeNo << ": rbStart=" << (uint32_t) rxIt->rbStart << ", rbLen=" << (uint32_t) rxIt->rbLen);
                    }
                  for(rxIt = grantIt->second.m_psschTx.begin(); rxIt != grantIt->second.m_psschTx.end(); rxIt++)
                    {
                      //std::cout << "Rnti=" << m_rnti << "\t PHY PSSCH RX at " << rxIt->subframe.frameNo << "/"<< rxIt->subframe.subframeNo << "\t rbStart=" << (uint32_t) rxIt->rbStart << "\t rbLen=" << (uint32_t) rxIt->rbLen << "\t from rnti=" << grantIt->first <<std::endl;
                      NS_LOG_INFO (this << " PSSCH Rx " << rxIt->subframe.frameNo << "/"<< rxIt->subframe.subframeNo << ": rbStart=" << (uint32_t) rxIt->rbStart << ", rbLen=" << (uint32_t) rxIt->rbLen);
                    }

                  double slRsrp_dBm = GetSidelinkRsrp(m_sidelinkSpectrumPhy->GetSlSignalPerceived()); 
                  double slRssi_dBm = GetSidelinkRssi(m_sidelinkSpectrumPhy->GetSlSignalPerceived(), m_sidelinkSpectrumPhy->GetSlInterferencePerceived());
                  m_uePhySapUser->PassSensingData(frameNo, subframeNo, grantIt->second.m_grant.m_pRsvp, grantIt->second.m_psschTx.begin()->rbStart, grantIt->second.m_psschTx.begin()->rbLen, grantIt->second.m_grant.m_prio, slRsrp_dBm, slRssi_dBm); 

                  grantIt->second.m_grant_received = false;
                }
              //now check if there is any grant for the current subframe and pass them to lower layer
              rxIt = grantIt->second.m_psschTx.begin(); 
              if (rxIt != grantIt->second.m_psschTx.end() && (*rxIt).subframe.frameNo == frameNo && (*rxIt).subframe.subframeNo == subframeNo)
                {
                  // remove reception information
                  grantIt->second.m_psschTx.erase(rxIt); 
                }

              if(grantIt->second.m_psschTx.size () == 0)
                {
                  // no more PSSCH transmission, clear the grant
                  it2->m_currentGrants.erase (grantIt++); 
                }
              else
                {
                  grantIt++;
                }
            }
        }

      // update uplink transmission mask according to previous UL-CQIs
      std::vector <int> rbMask = m_subChannelsForTransmissionQueue.at (0);
      SetSubChannelsForTransmission (m_subChannelsForTransmissionQueue.at (0));

      // shift the queue
      for (uint8_t i = 1; i < m_macChTtiDelay; i++)
        {
          m_subChannelsForTransmissionQueue.at (i-1) = m_subChannelsForTransmissionQueue.at (i);
        }
      m_subChannelsForTransmissionQueue.at (m_macChTtiDelay-1).clear ();

      if (m_srsConfigured && (m_srsStartTime <= Simulator::Now ()))
        {

          NS_ASSERT_MSG (subframeNo > 0 && subframeNo <= 10, "the SRS index check code assumes that subframeNo starts at 1");
          if ((((frameNo-1)*10 + (subframeNo-1)) % m_srsPeriodicity) == m_srsSubframeOffset)
            {
              NS_LOG_INFO ("frame " << frameNo << " subframe " << subframeNo << " sending SRS (offset=" << m_srsSubframeOffset << ", period=" << m_srsPeriodicity << ")");
              m_sendSrsEvent = Simulator::Schedule (UL_SRS_DELAY_FROM_SUBFRAME_START, 
                                                    &cv2x_LteUePhy::SendSrs,
                                                    this);
            }
        }

      //If rbMask has non empty RBs, it means we are expected to send messages in the uplink
      //otherwise we check if there are sidelink transmissions
      //Is this true if there are only control messages?
      std::list<Ptr<cv2x_LteControlMessage> > ctrlMsg = GetControlMessages ();
      // send packets in queue
      NS_LOG_LOGIC (this << " UE - start slot for PUSCH + PUCCH - RNTI " << m_rnti << " CELLID " << m_cellId);
      // send the current burst of packets
      Ptr<PacketBurst> pb = GetPacketBurst ();

      bool sciDiscfound = false;
      bool mibslfound = false;

      if (rbMask.size () == 0)
        {
          //we do not have uplink data to send. Normally, uplink has priority over sidelink but
          //since we send UL CQI messages all the time, we can remove them if we have a sidelink
          //transmission
          std::list<Ptr<cv2x_LteControlMessage> >::iterator ctrlIt;
          for (ctrlIt=ctrlMsg.begin() ; ctrlIt != ctrlMsg.end() && !sciDiscfound; ctrlIt++) {
              sciDiscfound = (*ctrlIt)->GetMessageType () == cv2x_LteControlMessage::SCI || (*ctrlIt)->GetMessageType() == cv2x_LteControlMessage::SCI_V2X || (*ctrlIt)->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG;
              mibslfound = (*ctrlIt)->GetMessageType() == cv2x_LteControlMessage::MIB_SL;
          }
          if (pb || sciDiscfound || mibslfound)
            {
              //we have sidelink to send, purge the control messages
              ctrlIt=ctrlMsg.begin();
              while (ctrlIt != ctrlMsg.end())
                {
                  if ((*ctrlIt)->GetMessageType () == cv2x_LteControlMessage::DL_CQI
                      || (*ctrlIt)->GetMessageType () == cv2x_LteControlMessage::BSR)
                    {
                      ctrlIt = ctrlMsg.erase (ctrlIt);
                    }
                  else
                    {
                      NS_ASSERT ((*ctrlIt)->GetMessageType () == cv2x_LteControlMessage::SCI
                                 || (*ctrlIt)->GetMessageType () == cv2x_LteControlMessage::SCI_V2X
                                 || (*ctrlIt)->GetMessageType() == cv2x_LteControlMessage::MIB_SL
                                 || (*ctrlIt)->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG
                      );
                      ctrlIt++;
                    }

                }
            }
        }
      if (rbMask.size() != 0 || (ctrlMsg.size () > 0 && (*ctrlMsg.begin())->GetMessageType () != cv2x_LteControlMessage::SCI 
          && (*ctrlMsg.begin())->GetMessageType () != cv2x_LteControlMessage::SCI_V2X
          && (*ctrlMsg.begin())->GetMessageType () != cv2x_LteControlMessage::MIB_SL
          && (*ctrlMsg.begin())->GetMessageType () != cv2x_LteControlMessage::SL_DISC_MSG ))
        {
          // send packets in queue
          NS_LOG_LOGIC (this << " UE - start slot for PUSCH + PUCCH - RNTI " << m_rnti << " CELLID " << m_cellId);

          if (pb)
            {
              //sanity check if this is a sidelink
              Ptr<Packet> packet = (*(pb->Begin()))->Copy ();
              cv2x_LteRadioBearerTag tag;
              packet->RemovePacketTag (tag);
              NS_ASSERT (tag.GetDestinationL2Id () == 0);

              if (m_enableUplinkPowerControl)
                {
                  m_txPower = m_powerControl->GetPuschTxPower (rbMask);
                  SetSubChannelsForTransmission (rbMask);
                }
              m_uplinkSpectrumPhy->StartTxDataFrame (pb, ctrlMsg, UL_DATA_DURATION);
            }
          else
            {
              // send only PUCCH (ideal: fake null bandwidth signal)
              if (ctrlMsg.size ()>0)
                {
                  NS_LOG_LOGIC (this << " UE - start TX PUCCH (NO PUSCH)");
                  std::vector <int> dlRb;

                  if (m_enableUplinkPowerControl)
                    {
                      m_txPower = m_powerControl->GetPucchTxPower (dlRb);
                    }

                  SetSubChannelsForTransmission (dlRb);
                  m_uplinkSpectrumPhy->StartTxDataFrame (pb, ctrlMsg, UL_DATA_DURATION);
                }
              else
                {
                  NS_LOG_LOGIC (this << " UE - UL NOTHING TO SEND");
                }
            }
        }
      else
        {
          //check sidelink

          //check if there is a SLSS message to be transmitted
          bool mibSLfound = false;
          std::list<Ptr<cv2x_LteControlMessage> >::iterator ctrlIt;
          for (ctrlIt = ctrlMsg.begin(); ctrlIt != ctrlMsg.end(); ctrlIt++)
          {
            if ((*ctrlIt)->GetMessageType() == cv2x_LteControlMessage::MIB_SL)
            {
              mibSLfound = true;
            }
          }
          if (!m_waitingNextScPeriod && !m_v2xEnabled)
            {
              // since we only have 1 Tx pool we can either send PSCCH or PSSCH but not both
              // send packets in queue
              NS_LOG_LOGIC (this << " UE - start slot for PSSCH + PSCCH - RNTI " << m_rnti << " CELLID " << m_cellId);
              // send the current burst of packets
              //Ptr<PacketBurst> pb = GetPacketBurst ();
              if (pb)
                {
                  //NS_ASSERT (ctrlMsg.size () == 0); //(In the future we can have PSSCH and MIB-SL in the same subframe)
                  NS_LOG_LOGIC (this << " UE - start TX PSSCH");
                  NS_LOG_DEBUG (this << " TX Burst containing " << pb->GetNPackets() << " packets");

                  //tx pool only has 1 grant so we can go straight to the first element
                  //find the matching transmission opportunity. This is needed in case some opportunities
                  //were skipped because the queue was empty
                  std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator txIt = m_slTxPoolInfo.m_currentGrants.begin()->second.m_psschTx.begin ();
                  while (txIt->subframe.frameNo < frameNo || (txIt->subframe.frameNo == frameNo && txIt->subframe.subframeNo < subframeNo))
                    {
                      txIt = m_slTxPoolInfo.m_currentGrants.begin()->second.m_psschTx.erase (txIt);
                      if (txIt == m_slTxPoolInfo.m_currentGrants.begin()->second.m_psschTx.end()) {
                          NS_LOG_ERROR ("Reached end of transmission list");
                      }
                    }
                  NS_ASSERT (txIt != m_slTxPoolInfo.m_currentGrants.begin()->second.m_psschTx.end()); //must be at least one element
                  NS_ASSERT_MSG (txIt->subframe.frameNo == frameNo && txIt->subframe.subframeNo == subframeNo, "Found " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo); //there must be an opportunity in this subframe
                  NS_ASSERT (rbMask.size() == 0);
                  for (int i = txIt->rbStart ; i < txIt->rbStart + txIt->nbRb ; i++)
                    {
                      NS_LOG_LOGIC (this << " Transmitting PSSCH on RB " << i);
                      rbMask.push_back (i);
                    }
                  m_slTxPoolInfo.m_currentGrants.begin()->second.m_psschTx.erase (txIt);
                  
                  // if (m_slTxPoolInfo.m_currentGrants.begin()->second.m_psschTx.size() == 0) {
                  //   //no more PSSCH transmission, clear the grant
                  //   m_slTxPoolInfo.m_currentGrants.clear ();
                  // }

                  if (m_enableUplinkPowerControl)
                    {
                      m_txPower = m_powerControl->GetPsschTxPower (rbMask);
                    }

                  //SetSubChannelsForTransmission (rbMask);
                  //m_uplinkSpectrumPhy->StartTxSlDataFrame (pb, ctrlMsg, UL_DATA_DURATION, m_slTxPoolInfo.m_currentGrants.begin()->second.m_grant.m_groupDstId);

                  //Synchronization has priority over communication
                  //The PSSCH is transmitted only if no synchronization operations are being performed
                  if (!mibSLfound)
                    {
                      if(m_ueSlssScanningInProgress)
                        {
                          NS_LOG_LOGIC(this <<" trying to do a PSSCH transmission while there is a scanning in progress... Ignoring transmission");
                        }
                      else if(m_ueSlssMeasurementsSched.find(Simulator::Now().GetMilliSeconds()) != m_ueSlssMeasurementsSched.end())
                        {
                          NS_LOG_LOGIC(this << " trying to do a PSSCH transmission while measuring S-RSRP in the same subframe... Ignoring transmission");
                        }
                      else
                        {
                          SetSubChannelsForTransmission (rbMask);
                          m_uplinkSpectrumPhy->StartTxSlDataFrame (pb, ctrlMsg, UL_DATA_DURATION, m_slTxPoolInfo.m_currentGrants.begin()->second.m_grant.m_groupDstId);
                        }
                    }
                  else
                    {
                      //TODO: Make the transmission possible if using different RBs than MIB-SL
                      NS_LOG_LOGIC(this << " trying to do a PSSCH transmission while there is a PSBCH (SLSS) transmission scheduled... Ignoring transmission ");
                    }
                }
              else
                {
                  // send only PSCCH (ideal: fake null bandwidth signal)
                  if (ctrlMsg.size ()>0 && sciDiscfound)
                    {
                      std::list<Ptr<cv2x_LteControlMessage> >::iterator msgIt = ctrlMsg.begin();
                      //skiping the MIB-SL if it is the first in the list
                      if((*msgIt)->GetMessageType () != cv2x_LteControlMessage::SCI && (*msgIt)->GetMessageType () != cv2x_LteControlMessage::SL_DISC_MSG)
                        {
                          msgIt++;
                        }

                      else if ((*msgIt)->GetMessageType () == cv2x_LteControlMessage::SCI)
                        {
                          NS_LOG_LOGIC (this << " UE - start TX PSCCH");
                          //access the control message to store the PSSCH grant and be able to
                          //determine the subframes/RBs for PSSCH transmissions/ discovery

                          NS_ASSERT_MSG ((*msgIt)->GetMessageType () == cv2x_LteControlMessage::SCI, "Received " << (*msgIt)->GetMessageType ());

                          Ptr<cv2x_SciLteControlMessage> msg2 = DynamicCast<cv2x_SciLteControlMessage> (*msgIt);
                          cv2x_SciListElement_s sci = msg2->GetSci ();

                          std::map<uint16_t, SidelinkGrantInfo>::iterator grantIt = m_slTxPoolInfo.m_currentGrants.find (sci.m_rnti);
                          if (grantIt == m_slTxPoolInfo.m_currentGrants.end ())
                            {
                              SidelinkGrantInfo grantInfo;
                              //this is the first transmission of PSCCH
                              grantInfo.m_grant_received = true;
                              grantInfo.m_grant.m_rnti = sci.m_rnti;
                              grantInfo.m_grant.m_resPscch = sci.m_resPscch;
                              grantInfo.m_grant.m_rbStart = sci.m_rbStart;
                              grantInfo.m_grant.m_rbLen = sci.m_rbLen;
                              grantInfo.m_grant.m_trp = sci.m_trp;
                              grantInfo.m_grant.m_groupDstId = sci.m_groupDstId;
                              grantInfo.m_grant.m_mcs = sci.m_mcs;
                              grantInfo.m_grant.m_tbSize = sci.m_tbSize;

                              grantInfo.m_grant.frameNo = frameNo;
                              grantInfo.m_grant.subframeNo = subframeNo;

                              grantInfo.m_pscchTx = m_slTxPoolInfo.m_pool->GetPscchTransmissions (sci.m_resPscch);
                              SidelinkCommResourcePool::SubframeInfo tmp = m_slTxPoolInfo.m_pool->GetCurrentScPeriod(frameNo, subframeNo);
                              grantInfo.m_psschTx = m_slTxPoolInfo.m_pool->GetPsschTransmissions (tmp, grantInfo.m_grant.m_trp, grantInfo.m_grant.m_rbStart, grantInfo.m_grant.m_rbLen);

                              std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator txIt;
                              for (txIt = grantInfo.m_psschTx.begin (); txIt != grantInfo.m_psschTx.end (); txIt++)
                                {
                                  //adjust for index starting at 1
                                  txIt->subframe.frameNo++;
                                  txIt->subframe.subframeNo++;
                                  NS_LOG_INFO (this << " Subframe Tx " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << ": rbStart=" << (uint32_t) txIt->rbStart << ", rbLen=" << (uint32_t) txIt->nbRb);
                                }

                              //insert grant
                              m_slTxPoolInfo.m_currentGrants.insert (std::pair <uint16_t, SidelinkGrantInfo> (sci.m_rnti, grantInfo));
                              NS_LOG_DEBUG (this <<  " Creating grant at " << grantInfo.m_grant.frameNo << "/" << grantInfo.m_grant.subframeNo);
                            }
                          else
                            {
                              NS_LOG_DEBUG (this <<  " Grant created at " << grantIt->second.m_grant.frameNo << "/" << grantIt->second.m_grant.subframeNo);
                            }
                          std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator txIt = m_slTxPoolInfo.m_currentGrants.begin()->second.m_pscchTx.begin ();
                          NS_ASSERT (txIt != m_slTxPoolInfo.m_currentGrants.begin()->second.m_pscchTx.end()); //must be at least one element
                          std::vector <int> slRb;
                          for (int i = txIt->rbStart ; i < txIt->rbStart + txIt->nbRb ; i++)
                            {
                              NS_LOG_LOGIC (this << " Transmitting PSCCH on RB " << i);
                              slRb.push_back (i);
                            }
                          m_slTxPoolInfo.m_currentGrants.begin()->second.m_pscchTx.erase (txIt);

                          //in some cases (when frames reached 1024), there can be a period with no
                          //slots for PSSCH. This causes the grant to not be cleared during the transmission
                          //of PSSCH frames. We check and clear here after the transmission of the second PSCCH
                          // if (m_slTxPoolInfo.m_currentGrants.begin()->second.m_pscchTx.size() == 0 && m_slTxPoolInfo.m_currentGrants.begin()->second.m_psschTx.size() == 0) {
                          //   m_slTxPoolInfo.m_currentGrants.clear ();
                          // }

                          if (m_enableUplinkPowerControl)
                            {
                              m_txPower = m_powerControl->GetPscchTxPower (slRb);
                            }

                          //SetSubChannelsForTransmission (slRb);
                          //m_uplinkSpectrumPhy->StartTxSlDataFrame (pb, ctrlMsg, UL_DATA_DURATION, sci.m_groupDstId);

                          //Synchronization has priority over communication
                          //The PSCCH is transmitted only if no synchronization operations are being performed
                          if (!mibSLfound)
                            {
                              if(m_ueSlssScanningInProgress)
                                {
                                  NS_LOG_LOGIC(this << "trying to do a PSCCH transmission while there is a scanning in progress... Ignoring transmission");

                                }
                              else if(m_ueSlssMeasurementsSched.find(Simulator::Now().GetMilliSeconds()) != m_ueSlssMeasurementsSched.end()) //Measurement in this subframe
                                {
                                  NS_LOG_LOGIC(this << " trying to do a PSCCH transmission while measuring S-RSRP in the same subframe... Ignoring transmission");
                                }
                              else
                                {
                                  SetSubChannelsForTransmission (slRb);
                                  m_uplinkSpectrumPhy->StartTxSlDataFrame (pb, ctrlMsg, UL_DATA_DURATION, sci.m_groupDstId);
                                }
                            }
                          else
                            {
                              //TODO: Make the transmission possible if using different RBs than MIB-SL
                              NS_LOG_LOGIC(this << " trying to do a PSCCH transmission while there is a PSBCH (SLSS) transmission scheduled... Ignoring transmission ");
                            }
                      }                           
                    else if ((*msgIt)->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG)
                      {
                        NS_LOG_LOGIC (this << " UE - start Tx PSDCH");
                        NS_ASSERT_MSG ((*msgIt)->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG, "Received " << (*msgIt)->GetMessageType ());
                        
                        std::map<uint16_t, DiscGrantInfo>::iterator grantIt = m_discTxPools.m_currentGrants.find (m_rnti);
                        if (grantIt == m_discTxPools.m_currentGrants.end ()) 
                          {
                            DiscGrantInfo grantInfo;
                            grantInfo.m_grant_received = true;
                            grantInfo.m_grant.m_rnti = m_rnti;
                            grantInfo.m_grant.m_resPsdch = m_discResPsdch;

                            grantInfo.m_psdchTx = m_discTxPools.m_pool->GetPsdchTransmissions (grantInfo.m_grant.m_resPsdch);
                            
                            uint8_t retx = m_discTxPools.m_pool->GetNumRetx ();
                            m_uplinkSpectrumPhy->SetDiscNumRetx (retx);
                            
                              std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo>::iterator txIt;
                            for (txIt = grantInfo.m_psdchTx.begin (); txIt != grantInfo.m_psdchTx.end (); txIt++)
                              {
                                //adjust for index starting at 1
                                txIt->subframe.frameNo++;
                                txIt->subframe.subframeNo++;
                              }

                            NS_LOG_DEBUG (this <<  " Creating grant");
                            m_discTxPools.m_currentGrants.insert (std::pair<uint16_t,DiscGrantInfo> (m_rnti, grantInfo));
                          }
                        else 
                          {
                            NS_LOG_DEBUG (this <<  " Grant already created");
                          }    

                        std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo>::iterator txIt = m_discTxPools.m_currentGrants.begin()->second.m_psdchTx.begin ();
                        NS_ASSERT (txIt != m_discTxPools.m_currentGrants.begin()->second.m_psdchTx.end()); 
                        std::vector <int> slRb;
                        for (int i = txIt->rbStart ; i < txIt->rbStart + txIt->nbRb ; i++)
                          {
                            NS_LOG_LOGIC (this << " Transmitting PSDCH on RB " << i);
                            slRb.push_back (i);
                          }       
                        m_discTxPools.m_currentGrants.begin()->second.m_psdchTx.erase (txIt);
                        
                        //TODO: UL power control for discovery
                        if (m_enableUplinkPowerControl)
                          {
                            m_txPower = m_powerControl->GetPsdchTxPower (slRb);
                          }
                
                        SetSubChannelsForTransmission (slRb);

                        //0 added to passby the group Id
                        //to be double checked
                        //
                        m_uplinkSpectrumPhy->StartTxSlDataFrame (pb, ctrlMsg, UL_DATA_DURATION,0);
                      
                        for (std::list<Ptr<cv2x_LteControlMessage> >::iterator msg = ctrlMsg.begin(); msg != ctrlMsg.end(); ++msg)
                        {
                          NS_LOG_LOGIC (this << ((*msg)->GetMessageType ()) << " discovery msg");
                          Ptr<cv2x_SlDiscMessage> msg2 = DynamicCast<cv2x_SlDiscMessage> ((*msg));
                          if (msg2)
                            {
                              cv2x_SlDiscMsg disc = msg2->GetSlDiscMessage ();
                              m_discoveryAnnouncementTrace (m_cellId, m_rnti,(uint32_t)disc.m_proSeAppCode.to_ulong());
                            }
                        }                  
                    }
                  else
                    {
                      NS_LOG_LOGIC (this << " UE - SL/UL NOTHING TO SEND");
                    }
                  }
                }     
            }//end if !m_waitingNextScPeriod
          else if(m_v2xEnabled)
            {
              NS_LOG_LOGIC (this << " V2X");
              // send packets in queue
              NS_LOG_LOGIC (this << " UE - start slot for PSSCH + PSCCH - RNTI " << m_rnti << " CELLID " << m_cellId);
              // send the current burst of packets
              // send only PSCCH (ideal: fake null bandwidth signal)
             
              //if (ctrlMsg.size ()>0 && sciDiscfound)
              if (ctrlMsg.size ()>0)
                { 
                  std::list<Ptr<cv2x_LteControlMessage> >::iterator msgIt = ctrlMsg.begin();
                  //skiping the MIB-SL if it is the first in the list
                  if((*msgIt)->GetMessageType () != cv2x_LteControlMessage::SCI_V2X && (*msgIt)->GetMessageType () != cv2x_LteControlMessage::SL_DISC_MSG)
                    {
                      NS_LOG_LOGIC (this << " skiping the MIB-SL if it is the first in the list");
                      msgIt++;
                    }
                  else if ((*msgIt)->GetMessageType () == cv2x_LteControlMessage::SCI_V2X)
                    {
                      NS_LOG_LOGIC (this << " UE - start TX PSCCH");
                      //access the control message to store the PSSCH grant and be able to
                      //determine the subframes/RBs for PSSCH transmissions
                      
                      NS_ASSERT_MSG ((*msgIt)->GetMessageType () == cv2x_LteControlMessage::SCI_V2X, "Received " << (*msgIt)->GetMessageType ());

                      Ptr<cv2x_SciLteControlMessageV2x> msg2 = DynamicCast<cv2x_SciLteControlMessageV2x> (*msgIt);
                      cv2x_SciListElementV2x sci1 = msg2->GetSci ();

                      std::map<uint16_t, SidelinkGrantInfoV2x>::iterator grantIt = m_slTxPoolInfoV2x.m_currentGrants.find (sci1.m_rnti);
                      if (grantIt == m_slTxPoolInfoV2x.m_currentGrants.end ())
                        {
                          SidelinkGrantInfoV2x grantInfo; 
                          // this is the first transmission of PSCCH
                          grantInfo.m_grant_received = true;
                          grantInfo.m_grant.m_rnti = sci1.m_rnti; 
                          grantInfo.m_grant.m_prio = sci1.m_prio; 
                          grantInfo.m_grant.m_pRsvp = sci1.m_pRsvp;
                          grantInfo.m_grant.m_reTxIdx = sci1.m_reTxIdx; 
                          grantInfo.m_grant.m_riv = sci1.m_riv;
                          grantInfo.m_grant.m_sfGap = sci1.m_sfGap; 
                          grantInfo.m_grant.m_mcs = sci1.m_mcs;  

                          grantInfo.m_grant.m_resPscch = sci1.m_resPscch;
                          grantInfo.m_grant.m_tbSize = sci1.m_tbSize;

                          grantInfo.m_grant.frameNo = frameNo; 
                          grantInfo.m_grant.subframeNo = subframeNo; 

                          SidelinkCommResourcePoolV2x::SubframeInfo tmp;
                          tmp.frameNo = frameNo;
                          tmp.subframeNo = subframeNo; 
                                      
                          grantInfo.m_pscchTx = m_slTxPoolInfoV2x.m_pool->GetPscchTransmissions(tmp, sci1.m_riv, sci1.m_pRsvp, sci1.m_sfGap, sci1.m_reTxIdx, sci1.m_resPscch, 1);
                          grantInfo.m_psschTx = m_slTxPoolInfoV2x.m_pool->GetPsschTransmissions(tmp, sci1.m_riv, sci1.m_pRsvp, sci1.m_sfGap, sci1.m_reTxIdx, sci1.m_resPscch, 1); 

                          std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator txIt;
                          for (txIt = grantInfo.m_pscchTx.begin (); txIt != grantInfo.m_pscchTx.end (); txIt++)
                            {
                              NS_ASSERT (txIt->subframe.frameNo == frameNo && txIt->subframe.subframeNo == subframeNo);
                              //std::cout << " Rnti="<< grantInfo.m_grant.m_rnti << "\t PHY PSCCH TX at " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << "\t rbStart=" << (uint32_t) txIt->rbStart << "\t rbLen=" << (uint32_t) txIt->rbLen << std::endl;
                              NS_LOG_INFO (this << "PSCCH Tx " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << "\t rbStart=" << (uint32_t) txIt->rbStart << "\t rbLen=" << (uint32_t) txIt->rbLen);
                            }
                          //std::cout << "----" << std::endl; 
                          for (txIt = grantInfo.m_psschTx.begin (); txIt != grantInfo.m_psschTx.end (); txIt++)
                            {
                              NS_ASSERT (txIt->subframe.frameNo == frameNo && txIt->subframe.subframeNo == subframeNo);
                              //std::cout << "Rnti=" << grantInfo.m_grant.m_rnti << "\t PHY PSSCH TX at " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << "\t rbStart=" << (uint32_t) txIt->rbStart << "\t rbLen=" << (uint32_t) txIt->rbLen << std::endl;
                              NS_LOG_INFO (this << "PSSCH TX " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << "\t rbStart=" << (uint32_t) txIt->rbStart << "\t rbLen=" << (uint32_t) txIt->rbLen);
                            }
                          //insert grant
                          m_slTxPoolInfoV2x.m_currentGrants.insert (std::pair <uint16_t, SidelinkGrantInfoV2x> (sci1.m_rnti, grantInfo));
                          NS_LOG_DEBUG (this <<  " Creating grant at " << grantInfo.m_grant.frameNo << "/" << grantInfo.m_grant.subframeNo);
                        }

                      std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator txIt = m_slTxPoolInfoV2x.m_currentGrants.begin()->second.m_pscchTx.begin ();
                      NS_ASSERT (txIt != m_slTxPoolInfoV2x.m_currentGrants.begin()->second.m_pscchTx.end()); //must be at least one element
                      std::vector <int> pscchRbs;
                      for (int i = txIt->rbStart ; i < txIt->rbStart + txIt->rbLen ; i++)
                        {
                          NS_LOG_LOGIC (this << " Transmitting PSCCH on RB " << i);
                          pscchRbs.push_back (i);
                        }
                      m_slTxPoolInfoV2x.m_currentGrants.begin()->second.m_pscchTx.erase (txIt);

                      if (m_enableUplinkPowerControl)
                        {
                          m_txPowerPscch = m_powerControl->GetPscchTxPower (pscchRbs);
                        }

                      //Synchronization has priority over communication
                      //The PSCCH is transmitted only if no synchronization operations are being performed
                      if (!mibSLfound)
                        {
                          if(m_ueSlssScanningInProgress)
                            {
                              NS_LOG_LOGIC(this << "trying to do a PSCCH transmission while there is a scanning in progress... Ignoring transmission");

                            }
                          else if(m_ueSlssMeasurementsSched.find(Simulator::Now().GetMilliSeconds()) != m_ueSlssMeasurementsSched.end()) //Measurement in this subframe
                            {
                              NS_LOG_LOGIC(this << " trying to do a PSCCH transmission while measuring S-RSRP in the same subframe... Ignoring transmission");
                            }
                          else
                            {
                              //SetSubChannelsForTransmission (slRb);
                              //m_uplinkSpectrumPhy->StartTxSlDataFrame (pb, ctrlMsg, UL_DATA_DURATION, 0);
                              m_uplinkSpectrumPhy->ChangeState (cv2x_LteSpectrumPhy::State::TX_UL_V2X_SCI);
                            }
                        }
                      else
                        {
                          NS_LOG_LOGIC(this << " trying to do a PSCCH transmission while there is a PSBCH (SLSS) transmission scheduled... Ignoring transmission ");
                        }
                    }                  
                  else
                    {
                      NS_LOG_LOGIC (this << " UE - SL/UL NOTHING TO SEND");
                    }
                }
              if (pb)
                {
                  NS_LOG_LOGIC (this << " UE - start TX PSSCH");
                  NS_LOG_DEBUG (this << " TX Burst containing " << pb->GetNPackets() << " packets");

                  //tx pool only has 1 grant so we can go straight to the first element
                  //find the matching transmission opportunity. This is needed in case some opportunities
                  //were skipped because the queue was empty
                  std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator txIt = m_slTxPoolInfoV2x.m_currentGrants.begin()->second.m_psschTx.begin ();

                  NS_ASSERT (txIt != m_slTxPoolInfoV2x.m_currentGrants.begin()->second.m_psschTx.end()); //must be at least one element
                  NS_ASSERT_MSG (txIt->subframe.frameNo == frameNo && txIt->subframe.subframeNo == subframeNo, "Found " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << "," << frameNo << "/" << subframeNo); //there must be an opportunity in this subframe
                  NS_ASSERT (rbMask.size() == 0);
                  
                  std::vector<int> psschRbs; 
                  for (int i = txIt->rbStart ; i < txIt->rbStart + txIt->rbLen ; i++)
                  {
                    NS_LOG_LOGIC (this << " Transmitting PSSCH on RB " << i);
                    psschRbs.push_back(i); 
                  }

                  for (int i = txIt->rbStart-2 ; i < txIt->rbStart + txIt->rbLen ; i++)
                  {
                    rbMask.push_back (i);
                  }

                  m_slTxPoolInfoV2x.m_currentGrants.begin()->second.m_psschTx.erase (txIt);

                  if (m_slTxPoolInfoV2x.m_currentGrants.begin()->second.m_psschTx.size() == 0) 
                    {
                      //no more PSSCH transmission, clear the grant
                      m_slTxPoolInfoV2x.m_currentGrants.clear ();
                    }

                  if (m_enableUplinkPowerControl)
                    {
                      //m_txPower = m_powerControl->GetPsschTxPower (rbMask);
                      m_txPowerPssch = m_powerControl->GetPscchTxPower (psschRbs);
                    }

                  //Synchronization has priority over communication
                  //The PSSCH is transmitted only if no synchronization operations are being performed
                  if (!mibSLfound)
                    {
                      if(m_ueSlssScanningInProgress)
                        {
                          NS_LOG_LOGIC(this <<" trying to do a PSSCH transmission while there is a scanning in progress... Ignoring transmission");
                        }
                      else if(m_ueSlssMeasurementsSched.find(Simulator::Now().GetMilliSeconds()) != m_ueSlssMeasurementsSched.end())
                        {
                          NS_LOG_LOGIC(this << " trying to do a PSSCH transmission while measuring S-RSRP in the same subframe... Ignoring transmission");
                        }
                      else
                        {
                          SetSubChannelsForTransmission (rbMask);
                          m_uplinkSpectrumPhy->StartTxSlDataFrame (pb, ctrlMsg, UL_DATA_DURATION, 0);
                        }
                    }
                  else
                    {
                      NS_LOG_LOGIC(this << " trying to do a PSSCH transmission while there is a PSBCH (SLSS) transmission scheduled... Ignoring transmission ");
                    }
                }                                                            
            } //end if V2X
          else
            {
              NS_LOG_LOGIC (this << " the UE changed of timing and it is waiting for the start of a new SC period using the new timing... Delaying transmissions ");
            } 
          //Transmit the SLSS
          if (mibSLfound)
            {
              //Remove all other control packets (i.e., SCI)
              ctrlIt=ctrlMsg.begin();
              while (ctrlIt != ctrlMsg.end())
                {
                  if ((*ctrlIt)->GetMessageType () != cv2x_LteControlMessage::MIB_SL)
                    {
                      ctrlIt = ctrlMsg.erase (ctrlIt);
                    }
                  else
                    {
                      ctrlIt++;
                    }
                }

              ctrlIt=ctrlMsg.begin();

              //We assume the SyncRef selection has priority over SLSS transmission
              //The SLSS is sent only if no scanning or measurement is performed in this subframe
              if(m_ueSlssScanningInProgress)
                {
                  NS_LOG_LOGIC(this << " trying to do a PSBCH transmission while there is a scanning in progress... Ignoring transmission");
                }
              else if(m_ueSlssMeasurementsSched.find(Simulator::Now().GetMilliSeconds()) != m_ueSlssMeasurementsSched.end()) //Measurement in this subframe
                {
                  NS_LOG_LOGIC(this << " trying to do a PSBCH transmission while measuring S-RSRP in the same subframe... Ignoring transmission");
                }
              else
                {
                  std::vector<int> dlRb;
                  for (uint8_t i = 22; i < 28; i++)
                    {
                      dlRb.push_back(i);
                    }
                  if (m_enableUplinkPowerControl)
                    {
                      //TODO: Set the transmission power corresponding to PSBCH
                      m_txPower = m_powerControl->GetPscchTxPower(dlRb);
                    }
                  SetSubChannelsForTransmission(dlRb);
                  m_uplinkSpectrumPhy->StartTxSlDataFrame(pb, ctrlMsg, UL_DATA_DURATION,0);
                }
            }
        }
    }  // m_configured

  // trigger the MAC
  m_uePhySapUser->SubframeIndication (frameNo, subframeNo);

  m_subframeNo = subframeNo;
  ++subframeNo;
  if (subframeNo > 10)
    {
      ++frameNo;
      if (frameNo > 1024) {
          frameNo = 1;
      }
      subframeNo = 1;
    }

  // schedule next subframe indication
  Simulator::Schedule (Seconds (GetTti ()), &cv2x_LteUePhy::SubframeIndication, this, frameNo, subframeNo);
}


void
cv2x_LteUePhy::SendSrs ()
{
  NS_LOG_FUNCTION (this << " UE " << m_rnti << " start tx SRS, cell Id " << (uint32_t) m_cellId);
  NS_ASSERT (m_cellId > 0);
  // set the current tx power spectral density (full bandwidth)
  std::vector <int> dlRb;
  for (uint8_t i = 0; i < m_ulBandwidth; i++)
    {
      dlRb.push_back (i);
    }

  if (m_enableUplinkPowerControl)
    {
      m_txPower = m_powerControl->GetSrsTxPower (dlRb);
    }

  SetSubChannelsForTransmission (dlRb);
  m_uplinkSpectrumPhy->StartTxUlSrsFrame ();
}


void
cv2x_LteUePhy::DoReset ()
{
  NS_LOG_FUNCTION (this);

  m_rnti = 0;
  m_transmissionMode = 0;
  m_srsPeriodicity = 0;
  m_srsConfigured = false;
  m_dlConfigured = false;
  m_ulConfigured = false;
  m_raPreambleId = 255; // value out of range
  m_raRnti = 11; // value out of range
  m_rsrpSinrSampleCounter = 0;
  m_p10CqiLast = Simulator::Now ();
  m_a30CqiLast = Simulator::Now ();
  m_paLinear = 1;

  m_packetBurstQueue.clear ();
  m_controlMessagesQueue.clear ();
  m_subChannelsForTransmissionQueue.clear ();
  for (int i = 0; i < m_macChTtiDelay; i++)
    {
      Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
      m_packetBurstQueue.push_back (pb);
      std::list<Ptr<cv2x_LteControlMessage> > l;
      m_controlMessagesQueue.push_back (l);
    }
  std::vector <int> ulRb;
  m_subChannelsForTransmissionQueue.resize (m_macChTtiDelay, ulRb);

  m_sendSrsEvent.Cancel ();
  m_downlinkSpectrumPhy->Reset ();
  m_uplinkSpectrumPhy->Reset ();

} // end of void cv2x_LteUePhy::DoReset ()

void
cv2x_LteUePhy::DoStartCellSearch (uint32_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << dlEarfcn);
  m_dlEarfcn = dlEarfcn;
  DoSetDlBandwidth (6); // configure DL for receiving PSS
  SwitchToState (CELL_SEARCH);
}

void
cv2x_LteUePhy::DoSynchronizeWithEnb (uint16_t cellId, uint32_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << cellId << dlEarfcn);
  m_dlEarfcn = dlEarfcn;
  DoSynchronizeWithEnb (cellId);
}

void
cv2x_LteUePhy::DoSynchronizeWithEnb (uint16_t cellId)
{
  NS_LOG_FUNCTION (this << cellId);

  if (cellId == 0)
    {
      NS_FATAL_ERROR ("Cell ID shall not be zero");
    }

  m_cellId = cellId;
  m_downlinkSpectrumPhy->SetCellId (cellId);
  m_uplinkSpectrumPhy->SetCellId (cellId);

  // configure DL for receiving the BCH with the minimum bandwidth
  DoSetDlBandwidth (6);

  m_dlConfigured = false;
  m_ulConfigured = false;

  SwitchToState (SYNCHRONIZED);
}

void
cv2x_LteUePhy::DoSetDlBandwidth (uint8_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << (uint32_t) dlBandwidth);
  if (m_dlBandwidth != dlBandwidth or !m_dlConfigured)
    {
      m_dlBandwidth = dlBandwidth;

      static const int Type0AllocationRbg[4] = {
        10,     // RGB size 1
        26,     // RGB size 2
        63,     // RGB size 3
        110     // RGB size 4
      };  // see table 7.1.6.1-1 of 36.213
      for (int i = 0; i < 4; i++)
        {
          if (dlBandwidth < Type0AllocationRbg[i])
            {
              m_rbgSize = i + 1;
              break;
            }
        }

      m_noisePsd = cv2x_LteSpectrumValueHelper::CreateNoisePowerSpectralDensity (m_dlEarfcn, m_dlBandwidth, m_noiseFigure);
      m_downlinkSpectrumPhy->SetNoisePowerSpectralDensity (m_noisePsd);
      m_downlinkSpectrumPhy->GetChannel ()->AddRx (m_downlinkSpectrumPhy);
    }
  m_dlConfigured = true;
}


void 
cv2x_LteUePhy::DoConfigureUplink (uint32_t ulEarfcn, uint8_t ulBandwidth)
{
  NS_LOG_FUNCTION (this << ulEarfcn << (uint16_t) ulBandwidth);
  m_ulEarfcn = ulEarfcn;
  m_ulBandwidth = ulBandwidth;
  m_ulConfigured = true;

  //configure sidelink with UL
  if (m_sidelinkSpectrumPhy)
    {
      m_slNoisePsd = cv2x_LteSpectrumValueHelper::CreateNoisePowerSpectralDensity (m_ulEarfcn, m_ulBandwidth, m_noiseFigure);
      m_sidelinkSpectrumPhy->SetNoisePowerSpectralDensity (m_slNoisePsd);
      m_sidelinkSpectrumPhy->GetChannel ()->AddRx (m_sidelinkSpectrumPhy);
    }
}

void
cv2x_LteUePhy::DoConfigureReferenceSignalPower (int8_t referenceSignalPower)
{
  NS_LOG_FUNCTION (this);
  m_powerControl->ConfigureReferenceSignalPower (referenceSignalPower);
}
 
void
cv2x_LteUePhy::DoSetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << " ID:" << m_uplinkSpectrumPhy->GetDevice()->GetNode()->GetId() << " RNTI: " << rnti);
  m_rnti = rnti;

  m_powerControl->SetCellId (m_cellId);
  m_powerControl->SetRnti (m_rnti);
}
 
void
cv2x_LteUePhy::DoSetTransmissionMode (uint8_t txMode)
{
  NS_LOG_FUNCTION (this << (uint16_t)txMode);
  m_transmissionMode = txMode;
  m_downlinkSpectrumPhy->SetTransmissionMode (txMode);
}

void
cv2x_LteUePhy::DoSetSrsConfigurationIndex (uint16_t srcCi)
{
  NS_LOG_FUNCTION (this << srcCi);
  m_srsPeriodicity = GetSrsPeriodicity (srcCi);
  m_srsSubframeOffset = GetSrsSubframeOffset (srcCi);
  m_srsConfigured = true;

  // a guard time is needed for the case where the SRS periodicity is changed dynamically at run time
  // if we use a static one, we can have a 0ms guard time
  m_srsStartTime = Simulator::Now () + MilliSeconds (0);
  NS_LOG_DEBUG (this << " UE SRS P " << m_srsPeriodicity << " RNTI " << m_rnti << " offset " << m_srsSubframeOffset << " cellId " << m_cellId << " CI " << srcCi);
}

void
cv2x_LteUePhy::DoSetPa (double pa)
{
  NS_LOG_FUNCTION (this << pa);
  m_paLinear = pow (10,(pa/10));
}

void
cv2x_LteUePhy::DoSetSlTxPool (Ptr<SidelinkTxDiscResourcePool> pool)
{
  m_discTxPools.m_pool = pool;
  m_discTxPools.m_npsdch = pool->GetNPsdch ();
  m_discTxPools.m_currentGrants.clear ();  
  m_discTxPools.m_nextDiscPeriod.frameNo = 0;
  m_discTxPools.m_nextDiscPeriod.subframeNo = 0;

}


void
cv2x_LteUePhy::DoRemoveSlTxPool (bool disc)
{
  m_discTxPools.m_pool = NULL;
  m_discTxPools.m_npsdch = 0;
  m_discTxPools.m_currentGrants.clear ();
}


void
cv2x_LteUePhy::DoSetSlRxPools (std::list<Ptr<SidelinkRxDiscResourcePool> > pools)
{
  
  std::list<Ptr<SidelinkRxDiscResourcePool> >::iterator poolIt;
  for (poolIt = pools.begin (); poolIt != pools.end(); poolIt++)
    {
      bool found = false;
      std::list<DiscPoolInfo >::iterator currentPoolIt;
      for (currentPoolIt = m_discRxPools.begin (); currentPoolIt != m_discRxPools.end() && !found; currentPoolIt++)
        {
          if (*poolIt == currentPoolIt->m_pool)
            {
              found = true;
            }
        }
      if (!found)
        {
          DiscPoolInfo newpool;
          newpool.m_pool = *poolIt;
          newpool.m_npsdch = (*poolIt)->GetNPsdch ();
          newpool.m_currentGrants.clear ();
          m_discRxPools.push_back (newpool);
          //
          m_sidelinkSpectrumPhy->SetRxPool (newpool.m_pool);
          //
        }
    }
}

void
cv2x_LteUePhy::DoSetDiscGrantInfo (uint8_t resPsdch)
{
  m_discResPsdch = resPsdch;
}

void 
cv2x_LteUePhy::DoAddDiscTxApps (std::list<uint32_t> apps)
{
  m_discTxApps = apps;
  m_sidelinkSpectrumPhy->AddDiscTxApps (apps);
}

void 
cv2x_LteUePhy::DoAddDiscRxApps (std::list<uint32_t> apps)
{
  m_discRxApps = apps;
  m_sidelinkSpectrumPhy->AddDiscRxApps (apps);
}

void
cv2x_LteUePhy::DoSetSlTxPool (Ptr<SidelinkTxCommResourcePool> pool)
{
  m_slTxPoolInfo.m_pool = pool;
  m_slTxPoolInfo.m_npscch = pool->GetNPscch();
  m_slTxPoolInfo.m_currentGrants.clear();
  m_slTxPoolInfo.m_nextScPeriod.frameNo = 0; //init to 0 to make it invalid
  m_slTxPoolInfo.m_nextScPeriod.subframeNo = 0; //init to 0 to make it invalid
}

void 
cv2x_LteUePhy::DoSetSlV2xTxPool (Ptr<SidelinkTxCommResourcePoolV2x> pool)
{
  m_slTxPoolInfoV2x.m_pool = pool;
  m_slTxPoolInfoV2x.m_currentGrants.clear();
  m_slTxPoolInfoV2x.m_currentFrameInfo.frameNo = 0; //init to 0 to make it invalid
  m_slTxPoolInfoV2x.m_currentFrameInfo.subframeNo = 0; //init to 0 to make it invalid
}

void
cv2x_LteUePhy::DoRemoveSlTxPool ()
{
  m_slTxPoolInfo.m_pool = NULL;
  m_slTxPoolInfo.m_npscch = 0;
  m_slTxPoolInfo.m_currentGrants.clear();
}

void
cv2x_LteUePhy::DoRemoveSlV2xTxPool ()
{
  m_slTxPoolInfoV2x.m_pool = NULL;
  m_slTxPoolInfoV2x.m_currentGrants.clear(); 
}

void
cv2x_LteUePhy::DoSetSlRxPools (std::list<Ptr<SidelinkRxCommResourcePool> > pools)
{
  //update the pools that have changed
  std::list<Ptr<SidelinkRxCommResourcePool> >::iterator poolIt;
  for (poolIt = pools.begin (); poolIt != pools.end(); poolIt++)
    {
      bool found = false;
      std::list<PoolInfo >::iterator currentPoolIt;
      for (currentPoolIt = m_sidelinkRxPools.begin (); currentPoolIt != m_sidelinkRxPools.end() && !found; currentPoolIt++)
        {
          if (*poolIt == currentPoolIt->m_pool)
            {
              found = true;
            }
        }
      if (!found)
        {
          PoolInfo newpool;
          newpool.m_pool = *poolIt;
          newpool.m_npscch = (*poolIt)->GetNPscch();
          newpool.m_currentGrants.clear();
          m_sidelinkRxPools.push_back (newpool);
        }
    }
  //TODO: should remove the ones no longer needed.
  //Find a clean way to handle updates
  //m_sidelinkRxPools.clear ();
  
}

void 
cv2x_LteUePhy::DoSetSlV2xRxPools (std::list<Ptr<SidelinkRxCommResourcePoolV2x> > pools)
{
  std::list<Ptr<SidelinkRxCommResourcePoolV2x> >::iterator poolIt;
  for (poolIt = pools.begin(); poolIt != pools.end(); poolIt++)
  {
    bool found = false;
    std::list<PoolInfoV2x >::iterator currentPoolIt;
    for (currentPoolIt = m_sidelinkRxPoolsV2x.begin (); currentPoolIt != m_sidelinkRxPoolsV2x.end () && !found; currentPoolIt++)
    {
      if(*poolIt == currentPoolIt->m_pool)
      {
        found = true;
      }
    }
    if(!found)
    {
      PoolInfoV2x newpool;
      newpool.m_pool = *poolIt;
      newpool.m_currentGrants.clear();
      m_sidelinkRxPoolsV2x.push_back (newpool);
      m_sidelinkSpectrumPhy->SetRxPool (newpool.m_pool);
    }
  }
}


void
cv2x_LteUePhy::DoAddSlDestination (uint32_t destination)
{
  std::list <uint32_t>::iterator it;
  for (it = m_destinations.begin (); it != m_destinations.end ();it++) {
    if ((*it) == destination) {
      break;
    }
  }
  if (it == m_destinations.end ()) {
    //did not find it, so insert
    m_destinations.push_back (destination);

    if (m_sidelinkSpectrumPhy) {
      m_sidelinkSpectrumPhy->AddL1GroupId (destination);
    }
  }
}

  
void
cv2x_LteUePhy::DoRemoveSlDestination (uint32_t destination)
{
  std::list <uint32_t>::iterator it = m_destinations.begin ();
  while (it != m_destinations.end ()) {
    if ((*it) == destination) {
      m_destinations.erase (it);
      if (m_sidelinkSpectrumPhy) {
        m_sidelinkSpectrumPhy->RemoveL1GroupId (destination);
      }
      break;//leave the loop
    }
    it++;
  }
}

void 
cv2x_LteUePhy::SetTxMode1Gain (double gain)
{
  SetTxModeGain (1, gain);
}

void 
cv2x_LteUePhy::SetTxMode2Gain (double gain)
{
  SetTxModeGain (2, gain);
}

void 
cv2x_LteUePhy::SetTxMode3Gain (double gain)
{
  SetTxModeGain (3, gain);
}

void 
cv2x_LteUePhy::SetTxMode4Gain (double gain)
{
  SetTxModeGain (4, gain);
}

void 
cv2x_LteUePhy::SetTxMode5Gain (double gain)
{
  SetTxModeGain (5, gain);
}

void 
cv2x_LteUePhy::SetTxMode6Gain (double gain)
{
  SetTxModeGain (6, gain);
}

void 
cv2x_LteUePhy::SetTxMode7Gain (double gain)
{
  SetTxModeGain (7, gain);
}


void
cv2x_LteUePhy::SetTxModeGain (uint8_t txMode, double gain)
{
  NS_LOG_FUNCTION (this << gain);
  // convert to linear
  double gainLin = std::pow (10.0, (gain / 10.0));
  if (m_txModeGain.size () < txMode)
    {
      m_txModeGain.resize (txMode);
    }
  std::vector <double> temp;
  temp = m_txModeGain;
  m_txModeGain.clear ();
  for (uint8_t i = 0; i < temp.size (); i++)
    {
      if (i==txMode-1)
        {
          m_txModeGain.push_back (gainLin);
        }
      else
        {
          m_txModeGain.push_back (temp.at (i));
        }
    }
  // forward the info to DL cv2x_LteSpectrumPhy
  m_downlinkSpectrumPhy->SetTxModeGain (txMode, gain);
}



void
cv2x_LteUePhy::ReceiveLteDlHarqFeedback (cv2x_DlInfoListElement_s m)
{
  NS_LOG_FUNCTION (this);
  // generate feedback to eNB and send it through ideal PUCCH
  Ptr<cv2x_DlHarqFeedbackLteControlMessage> msg = Create<cv2x_DlHarqFeedbackLteControlMessage> ();
  msg->SetDlHarqFeedback (m);
  SetControlMessages (msg);
}

void
cv2x_LteUePhy::SetHarqPhyModule (Ptr<cv2x_LteHarqPhy> harq)
{
  m_harqPhyModule = harq;
}


cv2x_LteUePhy::State
cv2x_LteUePhy::GetState () const
{
  NS_LOG_FUNCTION (this);
  return m_state;
}


void
cv2x_LteUePhy::SwitchToState (State newState)
{
  NS_LOG_FUNCTION (this << newState);
  State oldState = m_state;
  m_state = newState;
  NS_LOG_INFO (this << " cellId=" << m_cellId << " rnti=" << m_rnti
                    << " UePhy " << ToString (oldState)
                    << " --> " << ToString (newState));
  m_stateTransitionTrace (m_cellId, m_rnti, oldState, newState);
}

void cv2x_LteUePhy::SetFirstScanningTime(Time t){
  NS_LOG_FUNCTION (this);
  m_tFirstScanning = t;
  Simulator::Schedule(m_tFirstScanning,&cv2x_LteUePhy::StartSlssScanning, this);
}

Time cv2x_LteUePhy::GetFirstScanningTime(){
  NS_LOG_FUNCTION (this);
  return m_tFirstScanning;
}

double cv2x_LteUePhy::GetSidelinkRsrp (std::vector<SpectrumValue> p)
{
  // Measure instantaneous S-RSRP...
  double sum = 0.0;
  uint16_t nRB = 0; 
  uint8_t ctr = 0; 

  for(std::vector<SpectrumValue>::iterator it = p.begin(); it != p.end(); it++)
  {
    for (Values::const_iterator itPi = it->ConstValuesBegin(); itPi != it->ConstValuesEnd(); itPi++)
      {
        if((*itPi))
          { 
            // skip first two values because the RBs are used for PSCCH
            if(ctr <= 1)
            {
              ctr++; 
            }
            else
            {
              double powerTxW = ((*itPi) * 180000.0) / 12.0; // convert PSD [W/Hz] to linear power [W]
              sum += powerTxW;
              nRB++;
            }
          }
      }
  }
    
  double s_rsrp_W = (sum / (double) nRB);
  double s_rsrp_dBm = 10 * log10(1000 * (s_rsrp_W)); 

  return s_rsrp_dBm; 
}

double cv2x_LteUePhy::GetSidelinkRssi (std::vector<SpectrumValue> sig, std::vector<SpectrumValue> interference)
{
  // Measure instantaneous S-RSSI ...
  double sum = 0.0;
  uint16_t nRB = 0; 
  uint8_t ctr = 0; 

  std::vector<SpectrumValue>::iterator itSig = sig.begin();
  std::vector<SpectrumValue>::iterator itInt = interference.begin(); 

  for(itSig = sig.begin() ; itSig != sig.end(); itSig++, itInt++)
  {
      Values::const_iterator itIntN = itSig->ConstValuesBegin ();
      Values::const_iterator itPj = itInt->ConstValuesBegin ();
      
      for (itPj = itSig->ConstValuesBegin ();
               itPj != itSig->ConstValuesEnd ();
               itIntN++, itPj++)
            {
              if(ctr <= 1)
              {
                ctr++;
              }
              else
              {
                  nRB++;
                  // convert PSD [W/Hz] to linear power [W] for the single RE
                  double interfPlusNoisePowerTxW = ((*itIntN) * 180000.0) / 12.0;
                  double signalPowerTxW = ((*itPj) * 180000.0) / 12.0;
                  sum += (2 * (interfPlusNoisePowerTxW + signalPowerTxW));
              }
            }
  }
    
  double s_rssi_W = (sum / (double) nRB);
  double s_rssi_dBm = 10 * log10(1000 * (s_rssi_W)); 

  return s_rssi_dBm; 
}

void cv2x_LteUePhy::ReceiveSlss(uint16_t slssid, Ptr<SpectrumValue> p)
{
  NS_LOG_FUNCTION(this << slssid);

  if (m_ueSlssScanningInProgress || m_ueSlssMeasurementInProgress)
    {
      NS_LOG_LOGIC (this << " The UE is currently performing the SyncRef scanning or S-RSRP measurement");

      //Measure instantaneous S-RSRP...
      double sum = 0.0;
      uint16_t nRB = 0;
      Values::const_iterator itPi;
      for (itPi = p->ConstValuesBegin(); itPi != p->ConstValuesEnd(); itPi++)
        {
          if((*itPi))
            {
              double powerTxW = ((*itPi) * 180000.0) / 12.0; // convert PSD [W/Hz] to linear power [W]
              sum += powerTxW;
              nRB++;
            }
        }
      double s_rsrp_W = (sum / (double) nRB);
      double s_rsrp_dBm = 10 * log10(1000 * (s_rsrp_W));
      uint16_t offset = Simulator::Now().GetMilliSeconds() % 40;

      NS_LOG_INFO(this << " UE RNTI " << m_rnti << " received SLSS from SyncRef with SLSSID " << slssid
                  << " offset "<< offset << " with S-RSRP " << s_rsrp_dBm << " dBm" );

      //If it is not detectable, ignore
      if (s_rsrp_dBm < m_minSrsrp)
        {
          NS_LOG_LOGIC (this << " The S-RSRP is below the minimum required... Ignoring");
          return;
        }

      //Store the SLSS and S-RSRP
      //Note that a SyncRef is identified by SLSSID and reception offset.
      //SLSSs coming from different UEs, but having the same SyncRef info (same SLSSID and reception offset)
      //are considered as different S-RSRP samples of the same SyncRef
      std::map <std::pair<uint16_t,uint16_t>, UeSlssMeasurementsElement>::iterator
      itMeasMap = m_ueSlssMeasurementsMap.find(std::pair<uint16_t,uint16_t>(slssid,offset));

      if (itMeasMap == m_ueSlssMeasurementsMap.end()) //First entry
        {
          UeSlssMeasurementsElement newEl;
          newEl.srsrpSum = s_rsrp_W;
          newEl.srsrpNum = 1;

          if(m_ueSlssScanningInProgress)
            {
              NS_LOG_LOGIC (this << " SyncRef scan in progress, first detected entry");
              m_ueSlssDetectionMap.insert(std::pair< std::pair<uint16_t,uint16_t>,
                                          UeSlssMeasurementsElement>(std::pair<uint16_t,uint16_t>(slssid,offset), newEl));
            }
          else if(m_ueSlssMeasurementInProgress)
            {
              NS_LOG_LOGIC (this << " S-RSRP measurement in progress, first measurement entry");
              //Insert new measurement only if it was already detected
              std::map <std::pair<uint16_t,uint16_t>, UeSlssMeasurementsElement>::iterator
              itDetectionMap = m_ueSlssDetectionMap.find(std::pair<uint16_t,uint16_t>(slssid,offset));
              if (itDetectionMap != m_ueSlssDetectionMap.end())
                {
                  NS_LOG_LOGIC (this << " SyncRef already detected, storing measurement");
                  m_ueSlssMeasurementsMap.insert(std::pair< std::pair<uint16_t,uint16_t>,
                                                 UeSlssMeasurementsElement>(std::pair<uint16_t,uint16_t>(slssid,offset), newEl));
                }
              else
                {
                  NS_LOG_LOGIC (this << " SyncRef was not detected during SyncRef search/scanning... Ignoring");
                }
            }
        }
      else
        {
          NS_LOG_LOGIC (this << " Measurement entry found... Adding values");

          (*itMeasMap).second.srsrpSum += s_rsrp_W;
          (*itMeasMap).second.srsrpNum++;
        }
    }
  else
    {
      NS_LOG_LOGIC (this << " The UE is not currently performing SyncRef scanning or S-RSRP measurement... Ignoring");
    }
}



void cv2x_LteUePhy::SetInitialSubFrameIndication(bool rdm)
{
  NS_LOG_FUNCTION (this << rdm);

  if (rdm)
    {
      NS_LOG_LOGIC (this << " Random initial frame/subframe indication");

      Ptr<UniformRandomVariable> frameRdm = CreateObject<UniformRandomVariable> ();
      Ptr<UniformRandomVariable> subframeRdm = CreateObject<UniformRandomVariable> ();
      frameRdm->SetAttribute("Min", DoubleValue(1));
      frameRdm->SetAttribute("Max", DoubleValue(1024));
      subframeRdm->SetAttribute("Min", DoubleValue(1));
      subframeRdm->SetAttribute("Max", DoubleValue(10));
      Simulator::ScheduleNow(&cv2x_LteUePhy::SubframeIndication, this,
                             frameRdm->GetInteger(), subframeRdm->GetInteger());
    }
  //else
  //  {
  //    NS_LOG_LOGIC (this << " Standard initial frame/subframe indication (frameNo=1, subframeNo=1");
  //    Simulator::ScheduleNow(&cv2x_LteUePhy::SubframeIndication, this, 1, 1);
  //  }
}

void cv2x_LteUePhy::SetUeSlssInterScanningPeriodMax(Time t)
{
  NS_LOG_FUNCTION (this);
  m_nextScanRdm->SetAttribute("Max",DoubleValue(t.GetMilliSeconds()) );
}

void cv2x_LteUePhy::SetUeSlssInterScanningPeriodMin(Time t)
{
  NS_LOG_FUNCTION (this);
  m_nextScanRdm->SetAttribute("Min",DoubleValue(t.GetMilliSeconds()) );
}

void cv2x_LteUePhy::StartSlssScanning()
{
  NS_LOG_FUNCTION (this);
  m_ueSlssScanningInProgress = true;
  m_detectedMibSl.clear();
  Simulator::Schedule(m_ueSlssScanningPeriod, &cv2x_LteUePhy::EndSlssScanning, this);

}

void cv2x_LteUePhy::EndSlssScanning()
{
  NS_LOG_FUNCTION (this);
  m_ueSlssScanningInProgress = false;

  //Filter to keep only the SyncRefs with received MIB-SL
  std::map <std::pair<uint16_t,uint16_t>, UeSlssMeasurementsElement>::iterator itDetectionMap;
  for (itDetectionMap = m_ueSlssDetectionMap.begin(); itDetectionMap != m_ueSlssDetectionMap.end();itDetectionMap++  )
    {
      NS_LOG_LOGIC(this <<" UE RNTI "<<m_rnti<<" detected SyncRef with SLSSID "<<itDetectionMap->first.first <<" offset " << itDetectionMap->first.second <<" S-RSRP "<< itDetectionMap->second.srsrpSum / itDetectionMap->second.srsrpNum);
      std::map <std::pair<uint16_t,uint16_t>, cv2x_LteRrcSap::MasterInformationBlockSL>::iterator itMap =
          m_detectedMibSl.find (std::pair<uint16_t,uint16_t>(itDetectionMap->first.first, itDetectionMap->first.second));
      //If the MIB-SL wasn't received, erase it from the detection map
      if (itMap == m_detectedMibSl.end ())
        {
          NS_LOG_LOGIC(this << " MIB-SL was not found... Removing from detection list");
          m_ueSlssDetectionMap.erase(itDetectionMap);
        }
    }

  //Select the 6 SyncRefs with higher S-RSRP. Remove the others form the detected list
  std::map <double, std::pair<uint16_t,uint16_t> > tmp;
  for (itDetectionMap = m_ueSlssDetectionMap.begin(); itDetectionMap != m_ueSlssDetectionMap.end();itDetectionMap++  )
    {
      tmp.insert(std::pair<double, std::pair<uint16_t,uint16_t> >(itDetectionMap->second.srsrpSum / itDetectionMap->second.srsrpNum, itDetectionMap->first));
    }
  while (tmp.size() > 6)
    {
      NS_LOG_LOGIC(this << " The UE detected more than 6 SyncRefs... Removing lowest S-RSRP SyncRef: SLSSID"<< tmp.begin()->second.first <<"ofset "<<tmp.begin()->second.second<< "S-RSRP " <<tmp.begin()->first);
      tmp.erase(tmp.begin()->first);
    }
  std::map <std::pair<uint16_t,uint16_t>, UeSlssMeasurementsElement> ret;
  std::map <double, std::pair<uint16_t,uint16_t> >::iterator itTmp;
  for (itTmp = tmp.begin(); itTmp != tmp.end();itTmp++  )
    {
      std::map<std::pair<uint16_t,uint16_t>, UeSlssMeasurementsElement>::iterator
      itDetectionMapTwo = m_ueSlssDetectionMap.find(itTmp->second);
      if (itDetectionMapTwo != m_ueSlssDetectionMap.end())
        {
          ret.insert(std::pair<std::pair<uint16_t,uint16_t>,UeSlssMeasurementsElement>(itDetectionMapTwo->first, itDetectionMapTwo->second));
        }
    }

  m_ueSlssDetectionMap = ret; // It contains now only the 6 SyncRefs with higher S-RSRP
  m_ueSlssMeasurementsMap= ret;// It contains now only the 6 SyncRefs with higher S-RSRP (we use the S-RSRP measurements during scanning as first measurements)

  uint32_t nDetectedSyncRef = m_ueSlssDetectionMap.size();

  if (nDetectedSyncRef > 0)
    {
      NS_LOG_LOGIC(this << " At least one SyncRef detected, creating measurement schedule and starting measurement sub-process");
      //Create measurement schedule
      std::map <std::pair<uint16_t,uint16_t>, UeSlssMeasurementsElement>::iterator itMeasMap;
      for (itMeasMap = m_ueSlssMeasurementsMap.begin(); itMeasMap != m_ueSlssMeasurementsMap.end();itMeasMap++  )
        {
          uint16_t currOffset = Simulator::Now().GetMilliSeconds()%40;
          int64_t t;
          if ( currOffset < itMeasMap->first.second)
            {
              t = Simulator::Now().GetMilliSeconds() + (itMeasMap->first.second - currOffset);
            }
          else
            {
              t = Simulator::Now().GetMilliSeconds() + (40 - currOffset + itMeasMap->first.second);
            }
          uint16_t count = 1;
          while (t < (Simulator::Now().GetMilliSeconds()+ m_ueSlssMeasurementPeriod.GetMilliSeconds() - 40))
            {
              NS_LOG_INFO(this << " UE RNTI "<<m_rnti<<" will measure S-RSRP of SyncRef SLSSID "<< itMeasMap->first.first<<" offset "<< itMeasMap->first.second<<" at t:"<< t<<" ms");
              m_ueSlssMeasurementsSched.insert(std::pair<int64_t, std::pair<uint16_t,uint16_t> >(t,itMeasMap->first));
              count ++;
              if (count > m_nSamplesSrsrpMeas){
                  break;
              }
              t = t + 40;
            }
        }
      //Start measurement process of the 6 SyncRefs with higher S-RSRP
      StartSlssMeasurements(0,0);
    }
  else
    {
      NS_LOG_LOGIC(this << " No SyncRef detected... Ending SyncRef selection process");
      ScheduleNextSyncRefReselection(0); //The process ended after scanning
    }
}

void cv2x_LteUePhy::StartSlssMeasurements(uint64_t slssid, uint16_t offset)
{
  NS_LOG_FUNCTION (this);

  m_ueSlssMeasurementInProgress = true;
  Time t;
  if (slssid == 0) //Measurement
    {
      t = m_ueSlssMeasurementPeriod;
      NS_LOG_LOGIC(this << " Starting S-RSRP measurement corresponding to the measurement sub-process... Report happening in "<<t<<" ms");
    }
  else{ //Evaluation
      t = m_ueSlssEvaluationPeriod;
      NS_LOG_LOGIC(this << " Starting S-RSRP measurement corresponding to the evaluation sub-process... Report happening in "<<t<<" ms");
  }
  Simulator::Schedule(t,&cv2x_LteUePhy::ReportSlssMeasurements, this, slssid, offset);
}


void cv2x_LteUePhy::ReportSlssMeasurements(uint64_t slssid, uint16_t offset)
{
  NS_LOG_FUNCTION (this);

  cv2x_LteUeCphySapUser::UeSlssMeasurementsParameters ret;
  std::map<std::pair<uint16_t,uint16_t>, UeSlssMeasurementsElement>::iterator it;

  if(slssid == 0) //Report all
    {
      NS_LOG_LOGIC(this << " End of S-RSRP measurement corresponding to the measurement sub-process... Reporting L1 filtered S-RSRP values of detected SyncRefs");

      for (it = m_ueSlssMeasurementsMap.begin(); it != m_ueSlssMeasurementsMap.end(); it++)
        {
          //L1 filtering: linear average
          double avg_s_rsrp_W = (*it).second.srsrpSum/ (double) (*it).second.srsrpNum;
          //The stored values are in W, the report to the RRC should be in dBm
          double avg_s_rsrp_dBm = 10 * log10(1000 * (avg_s_rsrp_W));

          NS_LOG_INFO(this <<" UE RNTI "<<m_rnti<< " report SyncRef with SLSSID "
                      << (*it).first.first << " offset "<< (*it).first.second << " L1 filtered S-RSRP " << avg_s_rsrp_dBm
                      << " from " << (double) (*it).second.srsrpNum <<" samples");

          cv2x_LteUeCphySapUser::UeSlssMeasurementReportElement newEl;
          newEl.m_slssid = (*it).first.first;
          newEl.m_srsrp = avg_s_rsrp_dBm;
          newEl.m_offset = (*it).first.second;
          ret.m_ueSlssMeasurementsList.push_back(newEl);
        }

    }
  else // Report only of the selected SyncRef
    {
      NS_LOG_LOGIC(this << " End of S-RSRP measurement corresponding to the evaluation sub-process");
      NS_LOG_LOGIC(this << " Reporting L1 filtered S-RSRP values of the SyncRef SLSSID " << slssid <<" offset "<<offset);

      it = m_ueSlssMeasurementsMap.find (std::pair<uint16_t,uint16_t>(slssid,offset));
      if (it != m_ueSlssMeasurementsMap.end ())
        {
          //L1 filtering: linear average
          double avg_s_rsrp_W = (*it).second.srsrpSum/ (double) (*it).second.srsrpNum;
          //The stored values are in W, the report to the RRC should be in dBm
          double avg_s_rsrp_dBm = 10 * log10(1000 * (avg_s_rsrp_W));

          NS_LOG_INFO(this << Simulator::Now().GetMilliSeconds()<< " UE RNTI "<<m_rnti<< " Report SyncRef with SLSSID "
                      << (*it).first.first << " offset "<< (*it).first.second << " L1 filtered S-RSRP " << avg_s_rsrp_dBm
                      << " from " << (double) (*it).second.srsrpNum <<" samples");

          cv2x_LteUeCphySapUser::UeSlssMeasurementReportElement newEl;
          newEl.m_slssid = (*it).first.first;
          newEl.m_srsrp = avg_s_rsrp_dBm;
          newEl.m_offset = (*it).first.second;
          ret.m_ueSlssMeasurementsList.push_back(newEl);
        }
    }

  //Report to RRC
  m_ueCphySapUser->ReportSlssMeasurements(ret,slssid,offset);

  //Cleaning for next process
  m_ueSlssMeasurementsMap.clear();
  m_ueSlssMeasurementsSched.clear();
  m_ueSlssMeasurementInProgress = false;

  //Schedule the start of the measurement period for evaluation of selected SyncRef if appropriated
  m_currNMeasPeriods ++;
  if (m_currNMeasPeriods == 1 && m_resyncRequested)
    {
      NS_LOG_LOGIC(this << " The measurement sub-process ended and RRC selected a SyncRef for (re)synchronization");

      //Schedule the measurement for evaluation of the selected SyncRef for initiation/cease of SlSS transmission
      NS_LOG_INFO(this <<" UE RNTI "<<m_rnti<< " will start evaluation of selected SyncRef with SLSSID " <<m_resyncParams.syncRefMib.slssid <<" offset" << m_resyncParams.syncRefMib.rxOffset );
      Simulator::ScheduleNow(&cv2x_LteUePhy::StartSlssMeasurements, this, m_resyncParams.syncRefMib.slssid, m_resyncParams.syncRefMib.rxOffset);

      //Create measurement schedule for the evaluation
      uint16_t currOffset = Simulator::Now().GetMilliSeconds()%40;
      int64_t t;
      if ( currOffset < m_resyncParams.syncRefMib.rxOffset)
        {
          t = Simulator::Now().GetMilliSeconds() + (m_resyncParams.syncRefMib.rxOffset - currOffset);
        }
      else
        {
          t = Simulator::Now().GetMilliSeconds() + (40 - currOffset + m_resyncParams.syncRefMib.rxOffset);
        }
      uint16_t count = 1;
      while (t < (Simulator::Now().GetMilliSeconds()+ m_ueSlssMeasurementPeriod.GetMilliSeconds() - 40))
        {
          NS_LOG_INFO(this <<" UE RNTI "<<m_rnti<< " will measure SyncRef with SLSSID"<< m_resyncParams.syncRefMib.slssid<<" offset "<<  m_resyncParams.syncRefMib.rxOffset<<" at t:"<< t<<" ms");
          m_ueSlssMeasurementsSched.insert(std::pair<int64_t, std::pair<uint16_t,uint16_t> >(t, std::pair<uint16_t,uint16_t>(m_resyncParams.syncRefMib.slssid , m_resyncParams.syncRefMib.rxOffset)));
          count ++;
          if (count > m_nSamplesSrsrpMeas)
            {
              break;
            }
          t = t + 40;
        }
    }
  else
    { //m_currNMeasPeriods == 2 (after evaluation) || !m_resyncRequested (after measurement without selecting a SyncRef)

      //End of the selection+evaluation process, reinitialize variables for next process and schedule it
      m_ueSlssDetectionMap.clear();

      if (m_currNMeasPeriods ==1)
        {
          NS_LOG_LOGIC(this << " The measurement sub-process ended and RRC did not selected a SyncRef... Ending SyncRef selection process");
          ScheduleNextSyncRefReselection(1); //The process ended after measurement
        }
      if (m_currNMeasPeriods == 2)
        {
          NS_LOG_LOGIC(this << " The evaluation sub-process ended... Ending SyncRef selection process");
          ScheduleNextSyncRefReselection(2); // The process ended after evaluation
        }
      m_currNMeasPeriods = 0;
    }
}

void cv2x_LteUePhy::ScheduleNextSyncRefReselection(uint16_t endOfPrevious ){
  NS_LOG_FUNCTION (this);

  int32_t t_nextProcess = m_nextScanRdm->GetInteger();

  switch(endOfPrevious)
  {
    case 0:
      NS_LOG_LOGIC(this << " SyncRef selection process ended after scanning sub-process");
      t_nextProcess = t_nextProcess - m_ueSlssScanningPeriod.GetMilliSeconds();
      break;
    case 1:
      NS_LOG_LOGIC(this << " SyncRef selection process ended after measurement sub-process");
      t_nextProcess = t_nextProcess - (m_ueSlssScanningPeriod.GetMilliSeconds() + m_ueSlssMeasurementPeriod.GetMilliSeconds());
      break;
    case 2:
      NS_LOG_LOGIC(this << " SyncRef selection process ended after evaluation sub-process");
      t_nextProcess = t_nextProcess - (m_ueSlssScanningPeriod.GetMilliSeconds() + m_ueSlssMeasurementPeriod.GetMilliSeconds()+ m_ueSlssEvaluationPeriod.GetMilliSeconds());
      break;
  }

  //The standard requires at least one SyncRef selection process within 20s
  if (t_nextProcess > 20000)
    {
      NS_LOG_LOGIC(this << " Attempted to schedule the next SyncRef selection process for a period larger than 20 s... Scheduling it for 20 s");
      t_nextProcess = 20000;
    }

  //Do not travel to the past
  if (t_nextProcess <= 0)
    {
      NS_LOG_LOGIC(this << " Attempted to schedule the next SyncRef selection process for the past... Scheduling it for next subframe");
      t_nextProcess = 1;
    }
  NS_LOG_INFO(this <<" UE RNTI "<<m_rnti<< " will start the next SyncRef selection process in t: "<<t_nextProcess<<" ms");
  Simulator::Schedule(MilliSeconds(t_nextProcess), &cv2x_LteUePhy::StartSlssScanning,this);
}

bool
cv2x_LteUePhy::ChangeOfTiming(uint32_t frameNo, uint32_t subframeNo)
{
  NS_LOG_FUNCTION (this);

  if (m_slTxPoolInfo.m_pool)
    {
      NS_LOG_LOGIC(this << " The UE is currently transmitting sidelink communication");

      //Is it the start of a new period?
      if (((frameNo == m_slTxPoolInfo.m_nextScPeriod.frameNo && subframeNo
          == m_slTxPoolInfo.m_nextScPeriod.subframeNo)
          || m_slTxPoolInfo.m_nextScPeriod.frameNo == 0))
        {
          NS_LOG_LOGIC(this << " The current subframe corresponds to the start of a new sidelink communication period... Applying the change of timing");

          //Apply the change of Timing
          frameNo = m_resyncParams.newFrameNo;
          subframeNo = m_resyncParams.newSubframeNo;
          m_resyncRequested = false;
          NS_LOG_INFO(this <<" UE RNTI " << m_rnti
                      << " has a TxPool and changed the Subframe Indication from:"
                      << " frame " << m_slTxPoolInfo.m_nextScPeriod.frameNo
                      << " subframe " << m_slTxPoolInfo.m_nextScPeriod.subframeNo
                      << " to: frame " << frameNo << " subframe " << subframeNo);

          //Notify RRC about the successful change of SyncRef and timing
          m_ueCphySapUser->ReportChangeOfSyncRef(m_resyncParams.syncRefMib,frameNo, subframeNo);

          //Notify MAC about the successful change of SyncRef and timing. Some adjustments first

          //Adjusting MAC subframe indication:
          //There is a delay between the MAC scheduling and the PHY: the MAC is 4 subframes ahead
          uint32_t macSubframeNo = subframeNo;
          uint32_t macFrameNo = frameNo;
          macSubframeNo += 4;
          if (macSubframeNo > 10)
            {
              ++macFrameNo;
              if (macFrameNo > 1024)
                macFrameNo = 1;
              macSubframeNo -= 10;
            }
          //Adjusting the sidelink communication parameters
          //We calculate the next period using the frameNo/subframeNo of the MAC.
          //Thus we avoid miss alignment due to the delay
          m_slTxPoolInfo.m_currentScPeriod
          = m_slTxPoolInfo.m_pool->GetCurrentScPeriod(macFrameNo,macSubframeNo);
          m_slTxPoolInfo.m_nextScPeriod
          = m_slTxPoolInfo.m_pool->GetNextScPeriod(
              m_slTxPoolInfo.m_currentScPeriod.frameNo,
              m_slTxPoolInfo.m_currentScPeriod.subframeNo);
          //adjust because scheduler starts with frame/subframe = 1
          m_slTxPoolInfo.m_nextScPeriod.frameNo++;
          m_slTxPoolInfo.m_nextScPeriod.subframeNo++;
          NS_LOG_INFO(this << " UE RNTI " << m_rnti << " Next sidelink communication Tx period at frame/subframe: "
                      << m_slTxPoolInfo.m_nextScPeriod.frameNo << "/"
                      << m_slTxPoolInfo.m_nextScPeriod.subframeNo);
          //clear any previous grant
          m_slTxPoolInfo.m_currentGrants.clear();

          //Don't try to send sidelink communication until the start of the next period
          m_waitingNextScPeriod = true;

          //Finally, notify the MAC (Note the parameters are the PHY frameNo and subframeNo)
          m_uePhySapUser->NotifyChangeOfTiming(frameNo, subframeNo);

          //Store the new values
          m_currFrameNo = frameNo;
          m_currSubframeNo = subframeNo;

          //Notify the SpectrumPhy about the change of SLSSID
          m_uplinkSpectrumPhy->SetSlssid(m_resyncParams.syncRefMib.slssid);
          m_sidelinkSpectrumPhy->SetSlssid(m_resyncParams.syncRefMib.slssid);

          return true;
        }
      else
        {//Delay the change of Timing
          NS_LOG_LOGIC(this << " The current subframe does not correspond to the start of a new sidelink communication period... Delaying the change of timing");

          //Adjusting subframe indication to still match the SyncRef when the change of timing will be performed
          ++m_resyncParams.newSubframeNo;
          if (m_resyncParams.newSubframeNo > 10)
            {
              ++m_resyncParams.newFrameNo;
              if (m_resyncParams.newFrameNo > 1024)
                {
                  m_resyncParams.newFrameNo = 1;
                }
              m_resyncParams.newSubframeNo = 1;
            }
          return false;
        }
    }
  else
    {
      //No pool, apply directly the change of Timing
      NS_LOG_LOGIC(this << " The UE is not currently transmitting sidelink communication... Applying the change of timing");

      frameNo = m_resyncParams.newFrameNo;
      subframeNo = m_resyncParams.newSubframeNo;
      m_resyncRequested = false;
      NS_LOG_INFO(this << " UE RNTI " << m_rnti << "did not have a Tx pool and"
                  << " changed the Subframe Indication from: "
                  << " frame "<<m_currFrameNo<< "subframe "<<m_currSubframeNo
                  << " to: frame "<< frameNo << " subframe " << subframeNo);

      //Notify RRC about the successful change of SyncRef and timing
      m_ueCphySapUser->ReportChangeOfSyncRef(m_resyncParams.syncRefMib, frameNo, subframeNo);

      m_currFrameNo = frameNo;
      m_currSubframeNo = subframeNo;

      //Notify the SpectrumPhy about the change of SLSSID
      m_uplinkSpectrumPhy->SetSlssid(m_resyncParams.syncRefMib.slssid);
      m_sidelinkSpectrumPhy->SetSlssid(m_resyncParams.syncRefMib.slssid);

      return true;
    }
}

void cv2x_LteUePhy::DoSetSlssId(uint64_t slssid)
{
  NS_LOG_FUNCTION (this);
  m_uplinkSpectrumPhy->SetSlssid(slssid);
  m_sidelinkSpectrumPhy->SetSlssid(slssid);
}

void cv2x_LteUePhy::DoSendSlss(cv2x_LteRrcSap::MasterInformationBlockSL mibSl)
{
  NS_LOG_FUNCTION (this);
  Ptr<cv2x_MibSLLteControlMessage> msg = Create<cv2x_MibSLLteControlMessage> ();
  msg->SetMibSL(mibSl);
  NS_LOG_LOGIC(this << " Adding a MIB-SL to the queue of control messages to be send ");
  DoSendLteControlMessage(msg);
  //Notify the SpectrumPhy about the SLSSID used for transmitting
  //Do it here to have the correct SLSSID in the SpectrumPhy and cover the case in which
  //the UE reselects a random SLSSID without change of timing, i.e., out-of-coverage and without SyncRef
  m_uplinkSpectrumPhy->SetSlssid(mibSl.slssid);
  m_sidelinkSpectrumPhy->SetSlssid(mibSl.slssid);

}

void cv2x_LteUePhy::DoSynchronizeToSyncRef(cv2x_LteRrcSap::MasterInformationBlockSL mibSl)
{
  NS_LOG_FUNCTION (this);

  //Estimate the current timing (frame/subframe indication) of the SyncRef
  //using the information in the MIB-SL and the creation and reception timestamps
  uint32_t mibCreationAge = Simulator::Now().GetMilliSeconds() - mibSl.creationTimestamp.GetMilliSeconds();
  uint32_t mibRxAge = Simulator::Now().GetMilliSeconds() - mibSl.rxTimestamp.GetMilliSeconds();

  uint32_t frameOffsetSyncRef = 0;
  if (mibCreationAge >= 10)
    {
      frameOffsetSyncRef = uint32_t(mibCreationAge / 10);
    }
  uint32_t frameSyncRef = mibSl.directFrameNo + frameOffsetSyncRef;
  if (frameSyncRef > 1024)
    {
      frameSyncRef = frameSyncRef - 1024;
    }
  uint32_t subframeOffsetSyncRef = mibCreationAge % 10;
  uint32_t subframeSyncRef = mibSl.directSubframeNo + subframeOffsetSyncRef;
  if (subframeSyncRef > 10)
    {
      subframeSyncRef = subframeSyncRef % 10;
      frameSyncRef++;
      if (frameSyncRef > 1024)
        {
          frameSyncRef = 1;
        }
    }
  NS_LOG_INFO(this << " Synchronizing to SyncRef SLSSSID " << mibSl.slssid <<" offset " << mibSl.rxOffset);
  NS_LOG_INFO(this << " Its last mib was received " << mibRxAge << " ms ago, and it was created by the SyncRef "<<mibCreationAge<<" ms ago");
  NS_LOG_INFO(this << " The subframe indication in the MIB-SL, i.e., when created (frame/subframe):" << mibSl.directFrameNo <<"/"<< mibSl.directSubframeNo);
  NS_LOG_INFO(this << " The estimated CURRENT subframe indication of the SyncRef (frame/subframe): "<< frameSyncRef << "/"<< subframeSyncRef);
  NS_LOG_INFO(this << " The CURRENT subframe indication of this UE (frame/subframe): "<< m_currFrameNo << "/"<< m_currSubframeNo);

  //Request the synchronization (change of timing) for the next subframe
  m_resyncRequested = true;
  ++subframeSyncRef; //Update frame/subframe number to be used (in the next subframe)
  if (subframeSyncRef > 10)
    {
      ++frameSyncRef;
      if (frameSyncRef > 1024)
        {
          frameSyncRef = 1;
        }
      subframeSyncRef = 1;
    }
  m_resyncParams.newFrameNo = frameSyncRef;
  m_resyncParams.newSubframeNo = subframeSyncRef;
  m_resyncParams.syncRefMib = mibSl;
}


} // namespace ns3

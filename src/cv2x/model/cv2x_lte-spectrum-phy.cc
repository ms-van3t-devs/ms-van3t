/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009, 2011 CTTC
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 *         Giuseppe Piro  <g.piro@poliba.it>
 *         Marco Miozzo <marco.miozzo@cttc.es> (add physical error model)
 * Modified by: NIST (D2D)
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */


#include <ns3/object-factory.h>
#include <ns3/log.h>
#include <cmath>
#include <ns3/simulator.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/antenna-model.h>
#include "cv2x_lte-spectrum-phy.h"
#include "cv2x_lte-spectrum-signal-parameters.h"
#include "cv2x_lte-net-device.h"
#include "cv2x_lte-radio-bearer-tag.h"
#include "cv2x_lte-chunk-processor.h"
#include "cv2x_lte-sl-chunk-processor.h"
#include "cv2x_lte-phy-tag.h"
#include <ns3/cv2x_lte-mi-error-model.h>
#include <ns3/cv2x_lte-radio-bearer-tag.h>
#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/config.h>
#include <ns3/node.h>
#include "ns3/enum.h"
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteSpectrumPhy");


/// duration of SRS portion of UL subframe  
/// = 1 symbol for SRS -1ns as margin to avoid overlapping simulator events
static const Time UL_SRS_DURATION = NanoSeconds (71429 -1);  

/// duration of the control portion of a subframe
/// = 0.001 / 14 * 3 (ctrl fixed to 3 symbols) -1ns as margin to avoid overlapping simulator events
static const Time DL_CTRL_DURATION = NanoSeconds (214286 -1);

/// Effective coding rate
static const double EffectiveCodingRate[29] = {
  0.08,
  0.1,
  0.11,
  0.15,
  0.19,
  0.24,
  0.3,
  0.37,
  0.44,
  0.51,
  0.3,
  0.33,
  0.37,
  0.42,
  0.48,
  0.54,
  0.6,
  0.43,
  0.45,
  0.5,
  0.55,
  0.6,
  0.65,
  0.7,
  0.75,
  0.8,
  0.85,
  0.89,
  0.92
};



  
cv2x_TbId_t::cv2x_TbId_t ()
{
}

cv2x_TbId_t::cv2x_TbId_t (const uint16_t a, const uint8_t b)
: m_rnti (a),
  m_layer (b)
{
}

/**
 * Equality operator
 *
 * \param a lhs
 * \param b rhs
 * \returns true if rnti and layer are equal
 */
bool
operator == (const cv2x_TbId_t &a, const cv2x_TbId_t &b)
{
  return ( (a.m_rnti == b.m_rnti) && (a.m_layer == b.m_layer) );
}

/**
 * Less than operator
 *
 * \param a lhs
 * \param b rhs
 * \returns true if rnti less than ro rnti equal and layer less than
 */
bool
operator < (const cv2x_TbId_t& a, const cv2x_TbId_t& b)
{
  return ( (a.m_rnti < b.m_rnti) || ( (a.m_rnti == b.m_rnti) && (a.m_layer < b.m_layer) ) );
}

cv2x_SlTbId_t::cv2x_SlTbId_t ()
{
}

cv2x_SlTbId_t::cv2x_SlTbId_t (const uint16_t a, const uint8_t b)
: m_rnti (a),
  m_l1dst (b)
{
}

bool
operator == (const cv2x_SlTbId_t &a, const cv2x_SlTbId_t &b)
{
  return ( (a.m_rnti == b.m_rnti) && (a.m_l1dst == b.m_l1dst) );
}

bool
operator < (const cv2x_SlTbId_t& a, const cv2x_SlTbId_t& b)
{
  return ( (a.m_rnti < b.m_rnti) || ( (a.m_rnti == b.m_rnti) && (a.m_l1dst < b.m_l1dst) ) );
}

cv2x_SlV2xTbId_t::cv2x_SlV2xTbId_t ()
{
}

cv2x_SlV2xTbId_t::cv2x_SlV2xTbId_t (const uint16_t a)
: m_rnti (a)
{
}

bool
operator == (const cv2x_SlV2xTbId_t &a, const cv2x_SlV2xTbId_t &b)
{
  return ( (a.m_rnti == b.m_rnti) );
}

bool
operator < (const cv2x_SlV2xTbId_t& a, const cv2x_SlV2xTbId_t& b)
{
  return ( (a.m_rnti < b.m_rnti) );
}

bool
operator == (const cv2x_SlCtrlPacketInfo_t &a, const cv2x_SlCtrlPacketInfo_t &b)
{
  return (a.sinr == b.sinr);
}

bool
operator < (const cv2x_SlCtrlPacketInfo_t& a, const cv2x_SlCtrlPacketInfo_t& b)
{
  return (a.sinr > b.sinr); //we want by decreasing SINR
}

cv2x_DiscTbId_t::cv2x_DiscTbId_t ()
{
}

cv2x_DiscTbId_t::cv2x_DiscTbId_t (const uint16_t a, const uint8_t b)
: m_rnti (a),
  m_resPsdch (b)
{
}

bool
operator == (const cv2x_DiscTbId_t &a, const cv2x_DiscTbId_t &b)
{
  return ( (a.m_rnti == b.m_rnti) && (a.m_resPsdch == b.m_resPsdch) );
}

bool
operator < (const cv2x_DiscTbId_t& a, const cv2x_DiscTbId_t& b)
{
  return ( (a.m_rnti < b.m_rnti) || ( (a.m_rnti == b.m_rnti) && (a.m_resPsdch < b.m_resPsdch) ) );
}

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteSpectrumPhy);

cv2x_LteSpectrumPhy::cv2x_LteSpectrumPhy ()
  : m_state (IDLE),
    m_cellId (0),
    m_componentCarrierId (0),
    m_transmissionMode (0),
    m_layersNum (1),
    m_ulDataSlCheck (false),
    m_slssId(0)
{
  NS_LOG_FUNCTION (this);
  m_random = CreateObject<UniformRandomVariable> ();
  m_random->SetAttribute ("Min", DoubleValue (0.0));
  m_random->SetAttribute ("Max", DoubleValue (1.0));
  m_interferenceData = CreateObject<cv2x_LteInterference> ();
  m_interferenceCtrl = CreateObject<cv2x_LteInterference> ();
  m_interferenceSl = CreateObject<cv2x_LteSlInterference> ();

  for (uint8_t i = 0; i < 7; i++)
    {
      m_txModeGain.push_back (1.0);
    }
}


cv2x_LteSpectrumPhy::~cv2x_LteSpectrumPhy ()
{
  NS_LOG_FUNCTION (this);
  m_expectedTbs.clear ();
  m_expectedSlTbs.clear ();
  m_expectedSlV2xTbs.clear(); 
  m_txModeGain.clear ();
}

void cv2x_LteSpectrumPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_channel = 0;
  m_mobility = 0;
  m_device = 0;
  m_interferenceData->Dispose ();
  m_interferenceData = 0;
  m_interferenceCtrl->Dispose ();
  m_interferenceCtrl = 0;
  m_interferenceSl->Dispose ();
  m_interferenceSl = 0;
  m_ulDataSlCheck = false;
  m_ltePhyRxDataEndErrorCallback = MakeNullCallback< void > ();
  m_ltePhyRxDataEndOkCallback    = MakeNullCallback< void, Ptr<Packet> >  ();
  m_ltePhyRxCtrlEndOkCallback = MakeNullCallback< void, std::list<Ptr<cv2x_LteControlMessage> > > ();
  m_ltePhyRxCtrlEndErrorCallback = MakeNullCallback< void > ();
  m_ltePhyDlHarqFeedbackCallback = MakeNullCallback< void, cv2x_DlInfoListElement_s > ();
  m_ltePhyUlHarqFeedbackCallback = MakeNullCallback< void, cv2x_UlInfoListElement_s > ();
  m_ltePhyRxPssCallback = MakeNullCallback< void, uint16_t, Ptr<SpectrumValue> > ();
  m_ltePhyRxSlssCallback = MakeNullCallback< void, uint16_t, Ptr<SpectrumValue> > ();
  SpectrumPhy::DoDispose ();
} 

/**
 * Output stream output operator
 *
 * \param os output stream
 * \param s state
 * \returns output stream
 */
std::ostream& operator<< (std::ostream& os, cv2x_LteSpectrumPhy::State s)
{
  switch (s)
    {
    case cv2x_LteSpectrumPhy::IDLE:
      os << "IDLE";
      break;
    case cv2x_LteSpectrumPhy::RX_DATA:
      os << "RX_DATA";
      break;
    case cv2x_LteSpectrumPhy::RX_DL_CTRL:
      os << "RX_DL_CTRL";
      break;
    case cv2x_LteSpectrumPhy::TX_DATA:
      os << "TX_DATA";
      break;
    case cv2x_LteSpectrumPhy::TX_DL_CTRL:
      os << "TX_DL_CTRL";
      break;
    case cv2x_LteSpectrumPhy::TX_UL_SRS:
      os << "TX_UL_SRS";
      break;
    case cv2x_LteSpectrumPhy::TX_UL_V2X_SCI:
      os << "TX_UL_V2X_SCI"; 
      break;  
    default:
      os << "UNKNOWN";
      break;
    }
  return os;
}

TypeId
cv2x_LteSpectrumPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteSpectrumPhy")
    .SetParent<SpectrumPhy> ()
    .SetGroupName("Lte")
    .AddTraceSource ("TxStart",
                     "Trace fired when a new transmission is started",
                     MakeTraceSourceAccessor (&cv2x_LteSpectrumPhy::m_phyTxStartTrace),
                     "ns3::PacketBurst::TracedCallback")
    .AddTraceSource ("TxEnd",
                     "Trace fired when a previously started transmission is finished",
                     MakeTraceSourceAccessor (&cv2x_LteSpectrumPhy::m_phyTxEndTrace),
                     "ns3::PacketBurst::TracedCallback")
    .AddTraceSource ("RxStart",
                     "Trace fired when the start of a signal is detected",
                     MakeTraceSourceAccessor (&cv2x_LteSpectrumPhy::m_phyRxStartTrace),
                     "ns3::PacketBurst::TracedCallback")
    .AddTraceSource ("RxEndOk",
                     "Trace fired when a previously started RX terminates successfully",
                     MakeTraceSourceAccessor (&cv2x_LteSpectrumPhy::m_phyRxEndOkTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("RxEndError",
                     "Trace fired when a previously started RX terminates with an error",
                     MakeTraceSourceAccessor (&cv2x_LteSpectrumPhy::m_phyRxEndErrorTrace),
                     "ns3::Packet::TracedCallback")
    .AddAttribute ("DropRbOnCollisionEnabled",
                   "Activate/Deactivate the dropping colliding RBs regardless SINR value [by default is deactive].",
                    BooleanValue (false),
                    MakeBooleanAccessor (&cv2x_LteSpectrumPhy::m_dropRbOnCollisionEnabled),
                    MakeBooleanChecker ())                 
    .AddAttribute ("DataErrorModelEnabled",
                    "Activate/Deactivate the error model of data (TBs of PDSCH and PUSCH) [by default is active].",
                    BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_LteSpectrumPhy::m_dataErrorModelEnabled),
                    MakeBooleanChecker ())
    .AddAttribute ("CtrlErrorModelEnabled",
                    "Activate/Deactivate the error model of control (PCFICH-PDCCH decodification) [by default is active].",
                    BooleanValue (true),
                    MakeBooleanAccessor (&cv2x_LteSpectrumPhy::m_ctrlErrorModelEnabled),
                    MakeBooleanChecker ())
    .AddAttribute ("NistErrorModelEnabled",
                   "Activate/Deactivate the NIST based error model [by default is active].",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_LteSpectrumPhy::m_nistErrorModelEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("SlDataBLERModelEnabled",
                   "Activate/Deactivate the PSSCH BLER model [by default is active].",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_LteSpectrumPhy::m_slBlerEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("FadingModel",
                   "Fading model",
                   EnumValue (cv2x_LtePhyErrorModel::AWGN),
                   MakeEnumAccessor (&cv2x_LteSpectrumPhy::m_fadingModel),
                   MakeEnumChecker (cv2x_LtePhyErrorModel::AWGN, "AWGN"))                
    .AddTraceSource ("DlPhyReception",
                     "DL reception PHY layer statistics.",
                     MakeTraceSourceAccessor (&cv2x_LteSpectrumPhy::m_dlPhyReception),
                     "ns3::cv2x_PhyReceptionStatParameters::TracedCallback")
    .AddTraceSource ("UlPhyReception",
                     "DL reception PHY layer statistics.",
                     MakeTraceSourceAccessor (&cv2x_LteSpectrumPhy::m_ulPhyReception),
                     "ns3::cv2x_PhyReceptionStatParameters::TracedCallback")
    .AddTraceSource ("SlPhyReception",
                     "SL reception PHY layer statistics.",
                     MakeTraceSourceAccessor (&cv2x_LteSpectrumPhy::m_slPhyReception),
                     "ns3::cv2x_PhyReceptionStatParameters::TracedCallback")
    .AddTraceSource ("SlPscchReception",
                     "SL reception PCCH PHY layer statistics.",
                     MakeTraceSourceAccessor (&cv2x_LteSpectrumPhy::m_slPscchReception),
                     "ns3::cv2x_PhyReceptionStatParameters::TracedCallback")
    .AddAttribute ("HalfDuplexPhy",
                   "a pointer to a spectrum phy object",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteSpectrumPhy::m_halfDuplexPhy),
                   MakePointerChecker <cv2x_LteSpectrumPhy> ())
    .AddAttribute ("CtrlFullDuplexEnabled",
                    "Activate/Deactivate the full duplex in the PSCCH [by default is disable].",
                    BooleanValue (false),
                    MakeBooleanAccessor (&cv2x_LteSpectrumPhy::m_ctrlFullDuplexEnabled),
                    MakeBooleanChecker ())
    .AddAttribute ("ErrorModelHarqD2dDiscoveryEnabled",
                   "enable the error model and harq for D2D Discovery",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_LteSpectrumPhy::m_errorModelHarqD2dDiscoveryEnabled),
                   MakeBooleanChecker ())
  ;
  return tid;
}



Ptr<NetDevice>
cv2x_LteSpectrumPhy::GetDevice () const
{
  NS_LOG_FUNCTION (this);
  return m_device;
}


Ptr<MobilityModel>
cv2x_LteSpectrumPhy::GetMobility ()
{
  NS_LOG_FUNCTION (this);
  return m_mobility;
}


void
cv2x_LteSpectrumPhy::SetDevice (Ptr<NetDevice> d)
{
  NS_LOG_FUNCTION (this << d);
  m_device = d;
}


void
cv2x_LteSpectrumPhy::SetMobility (Ptr<MobilityModel> m)
{
  NS_LOG_FUNCTION (this << m);
  m_mobility = m;
}


void
cv2x_LteSpectrumPhy::SetChannel (Ptr<SpectrumChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  m_channel = c;
}

Ptr<SpectrumChannel> 
cv2x_LteSpectrumPhy::GetChannel ()
{
  return m_channel;
}

Ptr<const SpectrumModel>
cv2x_LteSpectrumPhy::GetRxSpectrumModel () const
{
  return m_rxSpectrumModel;
}


void
cv2x_LteSpectrumPhy::SetTxPowerSpectralDensity (Ptr<SpectrumValue> txPsd)
{
  NS_LOG_FUNCTION (this << txPsd);
  NS_ASSERT (txPsd);
  m_txPsd = txPsd;
}


void
cv2x_LteSpectrumPhy::SetNoisePowerSpectralDensity (Ptr<const SpectrumValue> noisePsd)
{
  NS_LOG_FUNCTION (this << noisePsd);
  NS_ASSERT (noisePsd);
  m_rxSpectrumModel = noisePsd->GetSpectrumModel ();
  m_interferenceData->SetNoisePowerSpectralDensity (noisePsd);
  m_interferenceCtrl->SetNoisePowerSpectralDensity (noisePsd);
  m_interferenceSl->SetNoisePowerSpectralDensity (noisePsd);
}

  
void 
cv2x_LteSpectrumPhy::Reset ()
{
  NS_LOG_FUNCTION (this);
  m_cellId = 0;
  m_slssId = 0;
  m_state = IDLE;
  m_transmissionMode = 0;
  m_layersNum = 1;
  m_endTxEvent.Cancel ();
  m_endRxDataEvent.Cancel ();
  m_endRxDlCtrlEvent.Cancel ();
  m_endRxUlSrsEvent.Cancel ();
  m_rxControlMessageList.clear ();
  m_expectedTbs.clear ();
  m_txControlMessageList.clear ();
  m_rxPacketBurstList.clear ();
  m_txPacketBurst = 0;
  m_rxSpectrumModel = 0;
  m_halfDuplexPhy = 0;
  m_ulDataSlCheck = false;
}

void 
cv2x_LteSpectrumPhy::ClearExpectedSlTb ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (this << " Expected TBs: " << m_expectedSlTbs.size ());
  m_expectedSlTbs.clear ();
  NS_LOG_DEBUG (this << " After clearing Expected TBs size: " << m_expectedSlTbs.size ());
}

void 
cv2x_LteSpectrumPhy::ClearExpectedSlV2xTb ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (this << " Expected TBs: " << m_expectedSlV2xTbs.size ());
  m_expectedSlV2xTbs.clear(); 
  NS_LOG_DEBUG (this << " After clearing Expected TBs size: " << m_expectedSlV2xTbs.size());
}

void
cv2x_LteSpectrumPhy::SetLtePhyRxDataEndErrorCallback (cv2x_LtePhyRxDataEndErrorCallback c)
{
  NS_LOG_FUNCTION (this);
  m_ltePhyRxDataEndErrorCallback = c;
}


void
cv2x_LteSpectrumPhy::SetLtePhyRxDataEndOkCallback (cv2x_LtePhyRxDataEndOkCallback c)
{
  NS_LOG_FUNCTION (this);
  m_ltePhyRxDataEndOkCallback = c;
}

void
cv2x_LteSpectrumPhy::SetLtePhyRxCtrlEndOkCallback (cv2x_LtePhyRxCtrlEndOkCallback c)
{
  NS_LOG_FUNCTION (this);
  m_ltePhyRxCtrlEndOkCallback = c;
}

void
cv2x_LteSpectrumPhy::SetLtePhyRxCtrlEndErrorCallback (cv2x_LtePhyRxCtrlEndErrorCallback c)
{
  NS_LOG_FUNCTION (this);
  m_ltePhyRxCtrlEndErrorCallback = c;
}


void
cv2x_LteSpectrumPhy::SetLtePhyRxPssCallback (cv2x_LtePhyRxPssCallback c)
{
  NS_LOG_FUNCTION (this);
  m_ltePhyRxPssCallback = c;
}

void
cv2x_LteSpectrumPhy::SetLtePhyDlHarqFeedbackCallback (cv2x_LtePhyDlHarqFeedbackCallback c)
{
  NS_LOG_FUNCTION (this);
  m_ltePhyDlHarqFeedbackCallback = c;
}

void
cv2x_LteSpectrumPhy::SetLtePhyUlHarqFeedbackCallback (cv2x_LtePhyUlHarqFeedbackCallback c)
{
  NS_LOG_FUNCTION (this);
  m_ltePhyUlHarqFeedbackCallback = c;
}


Ptr<AntennaModel>
cv2x_LteSpectrumPhy::GetRxAntenna ()
{
  return m_antenna;
}

void
cv2x_LteSpectrumPhy::SetAntenna (Ptr<AntennaModel> a)
{
  NS_LOG_FUNCTION (this << a);
  m_antenna = a;
}

void
cv2x_LteSpectrumPhy::SetState (State newState)
{
  ChangeState (newState);
}


void
cv2x_LteSpectrumPhy::ChangeState (State newState)
{
  NS_LOG_LOGIC (this << " state: " << m_state << " -> " << newState);
  m_state = newState;
}


void
cv2x_LteSpectrumPhy::SetHarqPhyModule (Ptr<cv2x_LteHarqPhy> harq)
{
  m_harqPhyModule = harq;
}




bool
cv2x_LteSpectrumPhy::StartTxDataFrame (Ptr<PacketBurst> pb, std::list<Ptr<cv2x_LteControlMessage> > ctrlMsgList, Time duration)
{
  NS_LOG_FUNCTION (this << pb);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);

  
  m_phyTxStartTrace (pb);
  
  switch (m_state)
    {
    case RX_DATA:
    case RX_DL_CTRL:
    case RX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while RX: according to FDD channel access, the physical layer for transmission cannot be used for reception");
      break;

    case TX_DATA:
    case TX_DL_CTRL:      
    case TX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while already TX: the MAC should avoid this");
      break;
      
    case IDLE:
    {
      /*
      m_txPsd must be set by the device, according to
      (i) the available subchannel for transmission
      (ii) the power transmission
      */
      NS_ASSERT (m_txPsd);
      m_txPacketBurst = pb;
      
      // we need to convey some PHY meta information to the receiver
      // to be used for simulation purposes (e.g., the CellId). This
      // is done by setting the ctrlMsgList parameter of
      // cv2x_LteSpectrumSignalParametersDataFrame
      ChangeState (TX_DATA);
      NS_ASSERT (m_channel);
      Ptr<cv2x_LteSpectrumSignalParametersDataFrame> txParams = Create<cv2x_LteSpectrumSignalParametersDataFrame> ();
      txParams->duration = duration;
      txParams->txPhy = GetObject<SpectrumPhy> ();
      txParams->txAntenna = m_antenna;
      txParams->psd = m_txPsd;
      txParams->packetBurst = pb;
      txParams->ctrlMsgList = ctrlMsgList;
      txParams->cellId = m_cellId;
      if (pb) 
      {
        m_ulDataSlCheck = true;
      }
      m_channel->StartTx (txParams);
      m_endTxEvent = Simulator::Schedule (duration, &cv2x_LteSpectrumPhy::EndTxData, this);
    }
    return false;
    break;
    
    default:
      NS_FATAL_ERROR ("unknown state");
      return true;
      break;
  }
}

bool
cv2x_LteSpectrumPhy::StartTxSlDataFrame (Ptr<PacketBurst> pb, std::list<Ptr<cv2x_LteControlMessage> > ctrlMsgList, Time duration, uint8_t groupId)
{
  NS_LOG_FUNCTION (this << pb);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);
  
  m_phyTxStartTrace (pb);
  
  switch (m_state)
  {
    case RX_DATA:
    case RX_DL_CTRL:
    case RX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while RX: according to FDD channel acces, the physical layer for transmission cannot be used for reception");
      break;

    case TX_DATA:
    case TX_DL_CTRL:      
    case TX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while already TX: the MAC should avoid this");
      break;

    case TX_UL_V2X_SCI:
    case IDLE:
    {
      
      //m_txPsd must be setted by the device, according to
      //(i) the available subchannel for transmission
      //(ii) the power transmission
      
      NS_ASSERT (m_txPsd);
      m_txPacketBurst = pb;
      
      // we need to convey some PHY meta information to the receiver
      // to be used for simulation purposes (e.g., the CellId). This
      // is done by setting the ctrlMsgList parameter of
      // cv2x_LteSpectrumSignalParametersDataFrame
      ChangeState (TX_DATA);
      NS_ASSERT (m_channel);
      Ptr<cv2x_LteSpectrumSignalParametersSlFrame> txParams = Create<cv2x_LteSpectrumSignalParametersSlFrame> ();
      txParams->duration = duration;
      txParams->txPhy = GetObject<SpectrumPhy> ();
      txParams->txAntenna = m_antenna;
      txParams->psd = m_txPsd;
      txParams->nodeId = GetDevice()->GetNode()->GetId();
      txParams->groupId = groupId;
      txParams->slssId = m_slssId;
      txParams->packetBurst = pb;
      txParams->ctrlMsgList = ctrlMsgList;
      m_ulDataSlCheck = true;

      m_channel->StartTx (txParams);
      m_endTxEvent = Simulator::Schedule (duration, &cv2x_LteSpectrumPhy::EndTxData, this);
    
      return false;
      break;

    }
    default:
      NS_FATAL_ERROR ("unknown state");
      return true;
      break;
  }
}

bool
cv2x_LteSpectrumPhy::StartTxDlCtrlFrame (std::list<Ptr<cv2x_LteControlMessage> > ctrlMsgList, bool pss)
{
  NS_LOG_FUNCTION (this << " PSS " << (uint16_t)pss);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);
  
  switch (m_state)
  {
    case RX_DATA:
    case RX_DL_CTRL:
    case RX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while RX: according to FDD channel access, the physical layer for transmission cannot be used for reception");
      break;
      
    case TX_DATA:
    case TX_DL_CTRL:
    case TX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while already TX: the MAC should avoid this");
      break;
      
    case IDLE:
    {
      /*
      m_txPsd must be set by the device, according to
      (i) the available subchannel for transmission
      (ii) the power transmission
      */
      NS_ASSERT (m_txPsd);
      
      // we need to convey some PHY meta information to the receiver
      // to be used for simulation purposes (e.g., the CellId). This
      // is done by setting the cellId parameter of
      // cv2x_LteSpectrumSignalParametersDlCtrlFrame
      ChangeState (TX_DL_CTRL);
      NS_ASSERT (m_channel);

      Ptr<cv2x_LteSpectrumSignalParametersDlCtrlFrame> txParams = Create<cv2x_LteSpectrumSignalParametersDlCtrlFrame> ();
      txParams->duration = DL_CTRL_DURATION;
      txParams->txPhy = GetObject<SpectrumPhy> ();
      txParams->txAntenna = m_antenna;
      txParams->psd = m_txPsd;
      txParams->cellId = m_cellId;
      txParams->pss = pss;
      txParams->ctrlMsgList = ctrlMsgList;
      m_channel->StartTx (txParams);
      m_endTxEvent = Simulator::Schedule (DL_CTRL_DURATION, &cv2x_LteSpectrumPhy::EndTxDlCtrl, this);
    }
    return false;
    break;
    
    default:
      NS_FATAL_ERROR ("unknown state");
      return true;
      break;
  }
}


bool
cv2x_LteSpectrumPhy::StartTxUlSrsFrame ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);
  
  switch (m_state)
    {
    case RX_DATA:
    case RX_DL_CTRL:
    case RX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while RX: according to FDD channel access, the physical layer for transmission cannot be used for reception");
      break;
      
    case TX_DL_CTRL:
    case TX_DATA:
    case TX_UL_SRS:
      NS_FATAL_ERROR ("cannot TX while already TX: the MAC should avoid this");
      break;
      
    case IDLE:
    {
      /*
      m_txPsd must be set by the device, according to
      (i) the available subchannel for transmission
      (ii) the power transmission
      */
      NS_ASSERT (m_txPsd);
      NS_LOG_LOGIC (this << " m_txPsd: " << *m_txPsd);
      
      // we need to convey some PHY meta information to the receiver
      // to be used for simulation purposes (e.g., the CellId). This
      // is done by setting the cellId parameter of 
      // cv2x_LteSpectrumSignalParametersDlCtrlFrame
      ChangeState (TX_UL_SRS);
      NS_ASSERT (m_channel);
      Ptr<cv2x_LteSpectrumSignalParametersUlSrsFrame> txParams = Create<cv2x_LteSpectrumSignalParametersUlSrsFrame> ();
      txParams->duration = UL_SRS_DURATION;
      txParams->txPhy = GetObject<SpectrumPhy> ();
      txParams->txAntenna = m_antenna;
      txParams->psd = m_txPsd;
      txParams->cellId = m_cellId;
      m_channel->StartTx (txParams);
      m_endTxEvent = Simulator::Schedule (UL_SRS_DURATION, &cv2x_LteSpectrumPhy::EndTxUlSrs, this);
    }
    return false;
    break;
    
    default:
      NS_FATAL_ERROR ("unknown state");
      return true;
      break;
  }
}



void
cv2x_LteSpectrumPhy::EndTxData ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);

  //NS_ASSERT (m_state == TX_DATA);
  m_phyTxEndTrace (m_txPacketBurst);
  m_txPacketBurst = 0;
  m_ulDataSlCheck = false;
  ChangeState (IDLE);
}

void
cv2x_LteSpectrumPhy::EndTxDlCtrl ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);

  NS_ASSERT (m_state == TX_DL_CTRL);
  NS_ASSERT (m_txPacketBurst == 0);
  ChangeState (IDLE);
}

void
cv2x_LteSpectrumPhy::EndTxUlSrs ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);

  NS_ASSERT (m_state == TX_UL_SRS);
  NS_ASSERT (m_txPacketBurst == 0);
  ChangeState (IDLE);
}

void
cv2x_LteSpectrumPhy::StartRx (Ptr<SpectrumSignalParameters> spectrumRxParams)
{
  NS_LOG_FUNCTION (this << spectrumRxParams);
  NS_LOG_LOGIC (this << " state: " << m_state);
  
  Ptr <const SpectrumValue> rxPsd = spectrumRxParams->psd;
  Time duration = spectrumRxParams->duration;
  
  // the device might start RX only if the signal is of a type
  // understood by this device - in this case, an LTE signal.
  Ptr<cv2x_LteSpectrumSignalParametersDataFrame> lteDataRxParams = DynamicCast<cv2x_LteSpectrumSignalParametersDataFrame> (spectrumRxParams);
  Ptr<cv2x_LteSpectrumSignalParametersDlCtrlFrame> lteDlCtrlRxParams = DynamicCast<cv2x_LteSpectrumSignalParametersDlCtrlFrame> (spectrumRxParams);
  Ptr<cv2x_LteSpectrumSignalParametersUlSrsFrame> lteUlSrsRxParams = DynamicCast<cv2x_LteSpectrumSignalParametersUlSrsFrame> (spectrumRxParams);
  Ptr<cv2x_LteSpectrumSignalParametersSlFrame> lteSlRxParams = DynamicCast<cv2x_LteSpectrumSignalParametersSlFrame> (spectrumRxParams);
  if (lteDataRxParams != 0)
    {
      m_interferenceData->AddSignal (rxPsd, duration);
      m_interferenceSl->AddSignal (rxPsd, duration); //to compute UL/SL interference
      StartRxData (lteDataRxParams);
    }
  else if (lteSlRxParams !=0)
    {
      m_interferenceSl->AddSignal (rxPsd, duration); 
      m_interferenceData->AddSignal (rxPsd, duration); //to compute UL/SL interference
      if(m_ctrlFullDuplexEnabled && lteSlRxParams->ctrlMsgList.size () > 0) 
      { 
        StartRxSlData (lteSlRxParams);
      }
      else if (!m_halfDuplexPhy || m_halfDuplexPhy->GetState () == IDLE || !(m_halfDuplexPhy->m_ulDataSlCheck))
      {
        StartRxSlData (lteSlRxParams);
      }
    }
  else if (lteDlCtrlRxParams!=0)
    {
      m_interferenceCtrl->AddSignal (rxPsd, duration);
      StartRxDlCtrl (lteDlCtrlRxParams);
    }
  else if (lteUlSrsRxParams!=0)
    {
      m_interferenceCtrl->AddSignal (rxPsd, duration);
      StartRxUlSrs (lteUlSrsRxParams);
    }
  else
    {
      // other type of signal (could be 3G, GSM, whatever) -> interference
      m_interferenceData->AddSignal (rxPsd, duration);
      m_interferenceCtrl->AddSignal (rxPsd, duration);
      m_interferenceSl->AddSignal (rxPsd, duration); 
    }    
}

void
cv2x_LteSpectrumPhy::StartRxData (Ptr<cv2x_LteSpectrumSignalParametersDataFrame> params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);
  switch (m_state)
    {
      case TX_DATA:
      case TX_DL_CTRL:
      case TX_UL_SRS:
        NS_FATAL_ERROR ("cannot RX while TX: according to FDD channel access, the physical layer for transmission cannot be used for reception");
        break;
      case RX_DL_CTRL:
        NS_FATAL_ERROR ("cannot RX Data while receiving control");
        break;
      case IDLE:
      case RX_DATA:
        // the behavior is similar when
        // we're IDLE or RX because we can receive more signals
        // simultaneously (e.g., at the eNB).
        {
          // To check if we're synchronized to this signal, we check
          // for the CellId which is reported in the
          //  cv2x_LteSpectrumSignalParametersDataFrame
          if (params->cellId  == m_cellId)
            {
              NS_LOG_LOGIC (this << " synchronized with this signal (cellId=" << params->cellId << ")");
              if ((m_rxPacketBurstList.empty ())&&(m_rxControlMessageList.empty ()))
                {
                  NS_ASSERT (m_state == IDLE);
                  // first transmission, i.e., we're IDLE and we
                  // start RX
                  m_firstRxStart = Simulator::Now ();
                  m_firstRxDuration = params->duration;
                  NS_LOG_LOGIC (this << " scheduling EndRx with delay " << params->duration.GetSeconds () << "s");
                  m_endRxDataEvent = Simulator::Schedule (params->duration, &cv2x_LteSpectrumPhy::EndRxData, this);
                }
              else
                {
                  NS_ASSERT (m_state == RX_DATA);
                  // sanity check: if there are multiple RX events, they
                  // should occur at the same time and have the same
                  // duration, otherwise the interference calculation
                  // won't be correct
                  NS_ASSERT ((m_firstRxStart == Simulator::Now ()) 
                  && (m_firstRxDuration == params->duration));
                }
              
              ChangeState (RX_DATA);
              if (params->packetBurst)
                {
                  m_rxPacketBurstList.push_back (params->packetBurst);
                  m_interferenceData->StartRx (params->psd);
                  
                  m_phyRxStartTrace (params->packetBurst);
                }
                NS_LOG_DEBUG (this << " insert msgs " << params->ctrlMsgList.size ());
              m_rxControlMessageList.insert (m_rxControlMessageList.end (), params->ctrlMsgList.begin (), params->ctrlMsgList.end ());
              
              NS_LOG_LOGIC (this << " numSimultaneousRxEvents = " << m_rxPacketBurstList.size ());
            }
          else
            {
              NS_LOG_LOGIC (this << " not in sync with this signal (cellId=" 
              << params->cellId  << ", m_cellId=" << m_cellId << ")");
            }
        }
        break;
        
        default:
          NS_FATAL_ERROR ("unknown state");
          break;
      }
      
   NS_LOG_LOGIC (this << " state: " << m_state);
}

void
cv2x_LteSpectrumPhy::StartRxSlData (Ptr<cv2x_LteSpectrumSignalParametersSlFrame> params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);

  switch (m_state)
  {
    case TX_DATA:
      case TX_DL_CTRL:
      case TX_UL_SRS:
        NS_FATAL_ERROR ("cannot RX while TX: according to FDD channel access, the physical layer for transmission cannot be used for reception");
        break;
      case RX_DL_CTRL:
        NS_FATAL_ERROR ("cannot RX Data while receiving control");
        break;
      case IDLE:
    case RX_DATA:
      // the behavior is similar when
      // we're IDLE or RX because we can receive more signals
      // simultaneously (e.g., at the eNB).
      {
        
        // check it is not an eNB and not the same sending node (sidelink : discovery & communication )
        if (m_cellId == 0 && params->nodeId != GetDevice()->GetNode()->GetId())
          {
            NS_LOG_LOGIC (this << " the signal is neither from eNodeB nor from this UE ");

            //SLSSs (PSBCH) should be received by all UEs
            //Checking if it is a SLSS, and if it is: measure S-RSRP and receive MIB-SL
            if (params->ctrlMsgList.size () >0)
              {
                std::list<Ptr<cv2x_LteControlMessage> >::iterator ctrlIt;
                for (ctrlIt=params->ctrlMsgList.begin() ; ctrlIt != params->ctrlMsgList.end(); ctrlIt++)
                  {
                    //Detection of a SLSS and callback for measurement of S-RSRP
                    if( (*ctrlIt)->GetMessageType () == cv2x_LteControlMessage::MIB_SL)
                      {
                        NS_LOG_LOGIC (this << " receiving a SLSS");
                        Ptr<cv2x_MibSLLteControlMessage> msg = DynamicCast<cv2x_MibSLLteControlMessage> (*ctrlIt);
                        cv2x_LteRrcSap::MasterInformationBlockSL mibSL = msg->GetMibSL ();
                        //Measure S-RSRP
                        if (!m_ltePhyRxSlssCallback.IsNull ())
                          {
                            m_ltePhyRxSlssCallback (mibSL.slssid, params->psd);
                          }
                        //Receive MIB-SL
                        if (m_rxPacketInfo.empty ())
                          {
                            NS_ASSERT (m_state == IDLE);
                            // first transmission, i.e., we're IDLE and we start RX
                            m_firstRxStart = Simulator::Now ();
                            m_firstRxDuration = params->duration;
                            NS_LOG_LOGIC (this << " scheduling EndRxSl with delay " << params->duration.GetSeconds () << "s");
                              
                            m_endRxDataEvent = Simulator::Schedule (params->duration, &cv2x_LteSpectrumPhy::EndRxSlData, this);
                          }
                        else
                          {
                            NS_ASSERT (m_state == RX_DATA);
                            // sanity check: if there are multiple RX events, they
                            // should occur at the same time and have the same
                            // duration, otherwise the interference calculation
                            // won't be correct
                            NS_ASSERT ((m_firstRxStart == Simulator::Now ())
                                       && (m_firstRxDuration == params->duration));
                          }
                        ChangeState (RX_DATA);
                        m_interferenceSl->StartRx (params->psd);
                        cv2x_SlRxPacketInfo_t packetInfo;
                        packetInfo.m_rxPacketBurst = params->packetBurst;
                        packetInfo.m_rxControlMessage = *ctrlIt;
                        //convert the PSD to RB map so we know which RBs were used to transmit the control message
                        //will be used later to compute error rate
                        std::vector <int> rbMap;
                        int i = 0;
                        for (Values::const_iterator it=params->psd->ConstValuesBegin (); it != params->psd->ConstValuesEnd () ; it++, i++)
                          {
                            if (*it != 0)
                              {
                                NS_LOG_INFO (this << " SL MIB-SL arriving on RB " << i);
                                rbMap.push_back (i);
                              }
                          }
                        packetInfo.rbBitmap = rbMap;
                        m_rxPacketInfo.push_back (packetInfo);
                        params->ctrlMsgList.erase(ctrlIt);
                        break;
                      }
                  }
              }


            //Receive PSCCH, PSSCH and PSDCH only if synchronized to the transmitter (having the same SLSSID)
            //and belonging to the destination group
            if (params->slssId == m_slssId && (params->groupId == 0 || m_l1GroupIds.find (params->groupId) != m_l1GroupIds.end()))
              {
                if (m_rxPacketInfo.empty ())
                  {
                    NS_ASSERT (m_state == IDLE);
                    // first transmission, i.e., we're IDLE and we start RX
                    m_firstRxStart = Simulator::Now ();
                    m_firstRxDuration = params->duration;
                    NS_LOG_LOGIC (this << " scheduling EndRxSl with delay " << params->duration.GetSeconds () << "s");
                    m_endRxDataEvent = Simulator::Schedule (params->duration, &cv2x_LteSpectrumPhy::EndRxSlData, this);
                  }
                else
                  {
                    NS_ASSERT (m_state == RX_DATA);
                    // sanity check: if there are multiple RX events, they
                    // should occur at the same time and have the same
                    // duration, otherwise the interference calculation
                    // won't be correct
                    NS_ASSERT ((m_firstRxStart == Simulator::Now ())
                               && (m_firstRxDuration == params->duration));
                  }
                ChangeState (RX_DATA);
                m_interferenceSl->StartRx (params->psd);

                cv2x_SlRxPacketInfo_t packetInfo;
                packetInfo.m_rxPacketBurst = params->packetBurst;
                if (params->ctrlMsgList.size () >0)
                  {
                    NS_ASSERT (params->ctrlMsgList.size () == 1);
                    packetInfo.m_rxControlMessage = *(params->ctrlMsgList.begin());
                  }
                //convert the PSD to RB map so we know which RBs were used to transmit the control message
                //will be used later to compute error rate
                std::vector <int> rbMap;
                int i = 0;
                for (Values::const_iterator it=params->psd->ConstValuesBegin (); it != params->psd->ConstValuesEnd () ; it++, i++)
                  {
                    if (*it != 0)
                      {
                        NS_LOG_INFO (this << " SL Message arriving on RB " << i);
                        rbMap.push_back (i);
                      }
                  }
                packetInfo.rbBitmap = rbMap;
                m_rxPacketInfo.push_back (packetInfo);

                if (params->packetBurst)
                  {
                    m_phyRxStartTrace (params->packetBurst);
                    NS_LOG_DEBUG (this << " RX Burst containing " << params->packetBurst->GetNPackets() << " packets");
                  }
                NS_LOG_DEBUG (this << " insert sidelink ctrl msgs " << params->ctrlMsgList.size ());
                NS_LOG_LOGIC (this << " numSimultaneousRxEvents = " << m_rxPacketInfo.size ());
              }

            else
              {
                NS_LOG_LOGIC (this << " not in sync with this sidelink signal... Ignoring ");
              }
          }
        else
        {
        	NS_LOG_LOGIC (this << " the signal is from eNodeB or from this UE... Ignoring");
        }
      }
      break;

    default:
      NS_FATAL_ERROR ("unknown state");
      break;
  }

  NS_LOG_LOGIC (this << " state: " << m_state);
}


void
cv2x_LteSpectrumPhy::StartRxDlCtrl (Ptr<cv2x_LteSpectrumSignalParametersDlCtrlFrame> lteDlCtrlRxParams)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);

  // To check if we're synchronized to this signal, we check
  // for the CellId which is reported in the
  // cv2x_LteSpectrumSignalParametersDlCtrlFrame
  uint16_t cellId;        
  NS_ASSERT (lteDlCtrlRxParams != 0);
  cellId = lteDlCtrlRxParams->cellId;

  switch (m_state)
    {
    case TX_DATA:
    case TX_DL_CTRL:
    case TX_UL_SRS:
    case RX_DATA:
    case RX_UL_SRS:
      NS_FATAL_ERROR ("unexpected event in state " << m_state);
      break;

    case RX_DL_CTRL:
    case IDLE:

      // common code for the two states
      // check presence of PSS for UE measuerements
      if (lteDlCtrlRxParams->pss == true)
        {
          if (!m_ltePhyRxPssCallback.IsNull ())
              {
                m_ltePhyRxPssCallback (cellId, lteDlCtrlRxParams->psd);
              }
        }   

      // differentiated code for the two states
      switch (m_state)
        {
        case RX_DL_CTRL:
          NS_ASSERT_MSG (m_cellId != cellId, "any other DlCtrl should be from a different cell");
          NS_LOG_LOGIC (this << " ignoring other DlCtrl (cellId=" 
                        << cellId  << ", m_cellId=" << m_cellId << ")");      
          break;
          
        case IDLE:
          if (cellId  == m_cellId)
            {
              NS_LOG_LOGIC (this << " synchronized with this signal (cellId=" << cellId << ")");
              
              NS_ASSERT (m_rxControlMessageList.empty ());
              m_firstRxStart = Simulator::Now ();
              m_firstRxDuration = lteDlCtrlRxParams->duration;
              NS_LOG_LOGIC (this << " scheduling EndRx with delay " << lteDlCtrlRxParams->duration);
              
              // store the DCIs
              m_rxControlMessageList = lteDlCtrlRxParams->ctrlMsgList;
              m_endRxDlCtrlEvent = Simulator::Schedule (lteDlCtrlRxParams->duration, &cv2x_LteSpectrumPhy::EndRxDlCtrl, this);
              ChangeState (RX_DL_CTRL);
              m_interferenceCtrl->StartRx (lteDlCtrlRxParams->psd);            
            }
          else
            {
              NS_LOG_LOGIC (this << " not synchronizing with this signal (cellId=" 
                            << cellId  << ", m_cellId=" << m_cellId << ")");          
            }
          break;
          
        default:
          NS_FATAL_ERROR ("unexpected event in state " << m_state);
          break;
        }
      break; // case RX_DL_CTRL or IDLE
      
    default:
      NS_FATAL_ERROR ("unknown state");
      break;
    }
  
  NS_LOG_LOGIC (this << " state: " << m_state);
}




void
cv2x_LteSpectrumPhy::StartRxUlSrs (Ptr<cv2x_LteSpectrumSignalParametersUlSrsFrame> lteUlSrsRxParams)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);
  switch (m_state)
    {
    case TX_DATA:
    case TX_DL_CTRL:
    case TX_UL_SRS:
      NS_FATAL_ERROR ("cannot RX while TX: according to FDD channel access, the physical layer for transmission cannot be used for reception");
      break;

    case RX_DATA:
    case RX_DL_CTRL:
      NS_FATAL_ERROR ("cannot RX SRS while receiving something else");
      break;

    case IDLE:
    case RX_UL_SRS:
      // the behavior is similar when
      // we're IDLE or RX_UL_SRS because we can receive more signals
      // simultaneously at the eNB
      {
        // To check if we're synchronized to this signal, we check
        // for the CellId which is reported in the
        // cv2x_LteSpectrumSignalParametersDlCtrlFrame
        uint16_t cellId;
        cellId = lteUlSrsRxParams->cellId;
        if (cellId  == m_cellId)
          {
            NS_LOG_LOGIC (this << " synchronized with this signal (cellId=" << cellId << ")");
            if (m_state == IDLE)
              {
                // first transmission, i.e., we're IDLE and we
                // start RX
                NS_ASSERT (m_rxControlMessageList.empty ());
                m_firstRxStart = Simulator::Now ();
                m_firstRxDuration = lteUlSrsRxParams->duration;
                NS_LOG_LOGIC (this << " scheduling EndRx with delay " << lteUlSrsRxParams->duration);

                m_endRxUlSrsEvent = Simulator::Schedule (lteUlSrsRxParams->duration, &cv2x_LteSpectrumPhy::EndRxUlSrs, this);
              }
            else if (m_state == RX_UL_SRS)
              {
                // sanity check: if there are multiple RX events, they
                // should occur at the same time and have the same
                // duration, otherwise the interference calculation
                // won't be correct
                NS_ASSERT ((m_firstRxStart == Simulator::Now ()) 
                           && (m_firstRxDuration == lteUlSrsRxParams->duration));
              }            
            ChangeState (RX_UL_SRS);
            m_interferenceCtrl->StartRx (lteUlSrsRxParams->psd);          
          }
        else
          {
            NS_LOG_LOGIC (this << " not in sync with this signal (cellId=" 
                          << cellId  << ", m_cellId=" << m_cellId << ")");          
          }
      }
      break;
      
    default:
      NS_FATAL_ERROR ("unknown state");
      break;
    }
  
  NS_LOG_LOGIC (this << " state: " << m_state);
}


void
cv2x_LteSpectrumPhy::UpdateSinrPerceived (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this << sinr);
  m_sinrPerceived = sinr;
}

void
cv2x_LteSpectrumPhy::UpdateSlSinrPerceived (std::vector <SpectrumValue> sinr)
{
  NS_LOG_FUNCTION (this);
  m_slSinrPerceived = sinr;
}

void
cv2x_LteSpectrumPhy::UpdateSlSigPerceived (std::vector <SpectrumValue> signal)
{
  NS_LOG_FUNCTION (this);
  m_slSignalPerceived = signal;
}

void
cv2x_LteSpectrumPhy::UpdateSlIntPerceived (std::vector <SpectrumValue> interference)
{
  NS_LOG_FUNCTION (this);
  m_slInterferencePerceived = interference;
}

std::vector <SpectrumValue> 
cv2x_LteSpectrumPhy::GetSlSignalPerceived ()
{
  NS_LOG_FUNCTION (this);
  return m_slSignalPerceived; 
}

std::vector <SpectrumValue>
cv2x_LteSpectrumPhy::GetSlInterferencePerceived ()
{
  NS_LOG_FUNCTION (this); 
  return m_slInterferencePerceived; 
}

void
cv2x_LteSpectrumPhy::AddExpectedTb (uint16_t  rnti, uint8_t ndi, uint16_t size, uint8_t mcs, std::vector<int> map, uint8_t layer, uint8_t harqId,uint8_t rv,  bool downlink)
{
  NS_LOG_FUNCTION (this << " rnti: " << rnti << " NDI " << (uint16_t)ndi << " size " << size << " mcs " << (uint16_t)mcs << " layer " << (uint16_t)layer << " rv " << (uint16_t)rv);
  cv2x_TbId_t tbId;
  tbId.m_rnti = rnti;
  tbId.m_layer = layer;
  expectedTbs_t::iterator it;
  it = m_expectedTbs.find (tbId);
  if (it != m_expectedTbs.end ())
    {
      // migth be a TB of an unreceived packet (due to high progpalosses)
      m_expectedTbs.erase (it);
    }
  // insert new entry
  cv2x_tbInfo_t tbInfo = {ndi, size, mcs, map, harqId, rv, 0.0, downlink, false, false};
  m_expectedTbs.insert (std::pair<cv2x_TbId_t, cv2x_tbInfo_t> (tbId,tbInfo));
}

void
cv2x_LteSpectrumPhy::AddExpectedTb (uint16_t  rnti, uint8_t l1dst, uint8_t ndi, uint16_t size, uint8_t mcs, std::vector<int> map, uint8_t rv)
{
  NS_LOG_FUNCTION (this << " rnti: " << rnti << " group " << (uint16_t) l1dst << " NDI " << (uint16_t)ndi << " size " << size << " mcs " << (uint16_t)mcs << " rv " << (uint16_t)rv);
  cv2x_SlTbId_t tbId;
  tbId.m_rnti = rnti;
  tbId.m_l1dst = l1dst;
  expectedSlTbs_t::iterator it;
  it = m_expectedSlTbs.find (tbId);
  if (it != m_expectedSlTbs.end ())
    {
      // migth be a TB of an unreceived packet (due to high progpalosses)
      m_expectedSlTbs.erase (it);
    }
  // insert new entry
  cv2x_SltbInfo_t tbInfo = {ndi, size, mcs, map, rv, 0.0, false, false};
  m_expectedSlTbs.insert (std::pair<cv2x_SlTbId_t, cv2x_SltbInfo_t> (tbId,tbInfo));

  // if it is for new data, reset the HARQ process
  if (ndi)
    {
      m_harqPhyModule->ResetSlHarqProcessStatus (rnti, l1dst);
    }
}


void
cv2x_LteSpectrumPhy::AddExpectedTbV2x (uint16_t  rnti, uint16_t size, uint8_t mcs, std::vector<int> map)
{
  NS_LOG_FUNCTION (this << " rnti: " << rnti  << " size " << size << " mcs " << (uint16_t)mcs);
  cv2x_SlV2xTbId_t tbId;
  tbId.m_rnti = rnti;
  expectedSlV2xTbs_t::iterator it;
  it = m_expectedSlV2xTbs.find (tbId);
  if (it != m_expectedSlV2xTbs.end ())
    {
      // migth be a TB of an unreceived packet (due to high progpalosses)
      m_expectedSlV2xTbs.erase (it);
    }
  // insert new entry
  cv2x_SlV2xTbInfo_t tbInfo = {size, mcs, map, 0.0, false, false};
  m_expectedSlV2xTbs.insert (std::pair<cv2x_SlV2xTbId_t, cv2x_SlV2xTbInfo_t> (tbId,tbInfo));
}


void
cv2x_LteSpectrumPhy::AddExpectedTb (uint16_t  rnti, uint8_t resPsdch, uint8_t ndi, std::vector<int> map, uint8_t rv)
{
  NS_LOG_FUNCTION (this << " rnti: " << rnti << " resPsdch " << resPsdch << " NDI " << (uint16_t)ndi << " rv " << (uint16_t)rv);
  cv2x_DiscTbId_t tbId;
  tbId.m_rnti = rnti;
  tbId.m_resPsdch = resPsdch;
  expectedDiscTbs_t::iterator it;
  it = m_expectedDiscTbs.find (tbId);
  if (it != m_expectedDiscTbs.end ())
    {
      // migth be a TB of an unreceived packet (due to high progpalosses)
      m_expectedDiscTbs.erase (it);
    }
  // insert new entry
  cv2x_DisctbInfo_t tbInfo = {ndi, resPsdch, map, rv, 0.0, false, false};

  m_expectedDiscTbs.insert (std::pair<cv2x_DiscTbId_t, cv2x_DisctbInfo_t> (tbId,tbInfo));

  // if it is for new data, reset the HARQ process
  if (ndi)
    {
      m_harqPhyModule->ResetDiscHarqProcessStatus (rnti, resPsdch);
    }
}


void
cv2x_LteSpectrumPhy::EndRxData ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);

  NS_ASSERT (m_state == RX_DATA);

  // this will trigger CQI calculation and Error Model evaluation
  // as a side effect, the error model should update the error status of all TBs
  m_interferenceData->EndRx ();
  NS_LOG_DEBUG (this << " No. of burst " << m_rxPacketBurstList.size ());
  NS_LOG_DEBUG (this << " Expected TBs " << m_expectedTbs.size ());
  expectedTbs_t::iterator itTb = m_expectedTbs.begin ();
  
  // apply transmission mode gain
  NS_LOG_DEBUG (this << " txMode " << (uint16_t)m_transmissionMode << " gain " << m_txModeGain.at (m_transmissionMode));
  NS_ASSERT (m_transmissionMode < m_txModeGain.size ());
  m_sinrPerceived *= m_txModeGain.at (m_transmissionMode);
  
  while (itTb!=m_expectedTbs.end ())
    {
      if ((m_dataErrorModelEnabled)&&(m_rxPacketBurstList.size ()>0)) // avoid to check for errors when there is no actual data transmitted
        {
          // retrieve HARQ info
          HarqProcessInfoList_t harqInfoList;
          if ((*itTb).second.ndi == 0)
            {
              // TB retxed: retrieve HARQ history
              uint16_t ulHarqId = 0;
              if ((*itTb).second.downlink)
                {
                  harqInfoList = m_harqPhyModule->GetHarqProcessInfoDl ((*itTb).second.harqProcessId, (*itTb).first.m_layer);
                }
              else
                {
                  harqInfoList = m_harqPhyModule->GetHarqProcessInfoUl ((*itTb).first.m_rnti, ulHarqId);
                }
            }
          cv2x_TbStats_t tbStats = cv2x_LteMiErrorModel::GetTbDecodificationStats (m_sinrPerceived, (*itTb).second.rbBitmap, (*itTb).second.size, (*itTb).second.mcs, harqInfoList);
          (*itTb).second.mi = tbStats.mi;
          (*itTb).second.corrupt = m_random->GetValue () > tbStats.tbler ? false : true;
          NS_LOG_DEBUG (this << "RNTI " << (*itTb).first.m_rnti << " size " << (*itTb).second.size << " mcs " << (uint32_t)(*itTb).second.mcs << " bitmap " << (*itTb).second.rbBitmap.size () << " layer " << (uint16_t)(*itTb).first.m_layer << " TBLER " << tbStats.tbler << " corrupted " << (*itTb).second.corrupt);
          // fire traces on DL/UL reception PHY stats
          cv2x_PhyReceptionStatParameters params;
          params.m_timestamp = Simulator::Now ().GetMilliSeconds ();
          params.m_cellId = m_cellId;
          params.m_imsi = 0; // it will be set by DlPhyTransmissionCallback in cv2x_LteHelper
          params.m_rnti = (*itTb).first.m_rnti;
          params.m_txMode = m_transmissionMode;
          params.m_layer =  (*itTb).first.m_layer;
          params.m_mcs = (*itTb).second.mcs;
          params.m_size = (*itTb).second.size;
          params.m_rv = (*itTb).second.rv;
          params.m_ndi = (*itTb).second.ndi;
          params.m_correctness = (uint8_t)!(*itTb).second.corrupt;
          params.m_ccId = m_componentCarrierId;

          //Aziza
          //SpectrumValue& m_sinrPerceived : the perceived sinrs in the whole bandwidth
          //std::vector<int>& (*itTb).second.rbBitmap : the actives RBs for the TB

          SpectrumValue sinrCopy = m_sinrPerceived;
          std::vector<int> map = (*itTb).second.rbBitmap;
          double sum = 0.0;
          for (uint32_t i = 0; i < map.size (); ++i)
          {
            double sinrLin = sinrCopy[map.at (i)];
            sum = sum + sinrLin;
            //std::cout << "RB " << map.at (i) << "\tSINR " << 10 * std::log10 (sinrLin) << " dB" << std::endl;
          }
          params.m_sinrPerRb = sum / map.size ();
          //std::cout << "average sinr " << 10 * std::log10 (params.m_sinrPerRb) << " dB" <<  std::endl;
          //

          if ((*itTb).second.downlink)
            {
              // DL
              m_dlPhyReception (params);
            }
          else
            {
              // UL
              params.m_rv = harqInfoList.size ();
              m_ulPhyReception (params);
            }
       }
      
      itTb++;
    }
    std::map <uint16_t, cv2x_DlInfoListElement_s> harqDlInfoMap;
    for (std::list<Ptr<PacketBurst> >::const_iterator i = m_rxPacketBurstList.begin (); 
    i != m_rxPacketBurstList.end (); ++i)
      {
        for (std::list<Ptr<Packet> >::const_iterator j = (*i)->Begin (); j != (*i)->End (); ++j)
          {
            // retrieve TB info of this packet 
            cv2x_LteRadioBearerTag tag;
            (*j)->PeekPacketTag (tag);
            cv2x_TbId_t tbId;
            tbId.m_rnti = tag.GetRnti ();
            tbId.m_layer = tag.GetLayer ();
            itTb = m_expectedTbs.find (tbId);
            NS_LOG_INFO (this << " Packet of " << tbId.m_rnti << " layer " <<  (uint16_t) tag.GetLayer ());
            if (itTb!=m_expectedTbs.end ())
              {
                if (!(*itTb).second.corrupt)
                  {
                    m_phyRxEndOkTrace (*j);
                
                    if (!m_ltePhyRxDataEndOkCallback.IsNull ())
                      {
                        m_ltePhyRxDataEndOkCallback (*j);
                      }
                  }
                else
                  {
                    // TB received with errors
                    m_phyRxEndErrorTrace (*j);
                  }

                // send HARQ feedback (if not already done for this TB)
                if (!(*itTb).second.harqFeedbackSent)
                  {
                    (*itTb).second.harqFeedbackSent = true;
                    if (!(*itTb).second.downlink)
                      {
                        cv2x_UlInfoListElement_s harqUlInfo;
                        harqUlInfo.m_rnti = tbId.m_rnti;
                        harqUlInfo.m_tpc = 0;
                        if ((*itTb).second.corrupt)
                          {
                            harqUlInfo.m_receptionStatus = cv2x_UlInfoListElement_s::NotOk;
                            NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " send UL-HARQ-NACK");
                            m_harqPhyModule->UpdateUlHarqProcessStatus (tbId.m_rnti, (*itTb).second.mi, (*itTb).second.size, (*itTb).second.size / EffectiveCodingRate [(*itTb).second.mcs]);
                          }
                        else
                          {
                            harqUlInfo.m_receptionStatus = cv2x_UlInfoListElement_s::Ok;
                            NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " send UL-HARQ-ACK");
                            m_harqPhyModule->ResetUlHarqProcessStatus (tbId.m_rnti, (*itTb).second.harqProcessId);
                          }
                          if (!m_ltePhyUlHarqFeedbackCallback.IsNull ())
                            {
                              m_ltePhyUlHarqFeedbackCallback (harqUlInfo);
                            }
                      }
                    else
                      {
                        std::map <uint16_t, cv2x_DlInfoListElement_s>::iterator itHarq = harqDlInfoMap.find (tbId.m_rnti);
                        if (itHarq==harqDlInfoMap.end ())
                          {
                            cv2x_DlInfoListElement_s harqDlInfo;
                            harqDlInfo.m_harqStatus.resize (m_layersNum, cv2x_DlInfoListElement_s::ACK);
                            harqDlInfo.m_rnti = tbId.m_rnti;
                            harqDlInfo.m_harqProcessId = (*itTb).second.harqProcessId;
                            if ((*itTb).second.corrupt)
                              {
                                harqDlInfo.m_harqStatus.at (tbId.m_layer) = cv2x_DlInfoListElement_s::NACK;
                                NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " harqId " << (uint16_t)(*itTb).second.harqProcessId << " layer " <<(uint16_t)tbId.m_layer << " send DL-HARQ-NACK");
                                m_harqPhyModule->UpdateDlHarqProcessStatus ((*itTb).second.harqProcessId, tbId.m_layer, (*itTb).second.mi, (*itTb).second.size, (*itTb).second.size / EffectiveCodingRate [(*itTb).second.mcs]);
                              }
                            else
                              {

                                harqDlInfo.m_harqStatus.at (tbId.m_layer) = cv2x_DlInfoListElement_s::ACK;
                                NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " harqId " << (uint16_t)(*itTb).second.harqProcessId << " layer " <<(uint16_t)tbId.m_layer << " size " << (*itTb).second.size << " send DL-HARQ-ACK");
                                m_harqPhyModule->ResetDlHarqProcessStatus ((*itTb).second.harqProcessId);
                              }
                            harqDlInfoMap.insert (std::pair <uint16_t, cv2x_DlInfoListElement_s> (tbId.m_rnti, harqDlInfo));
                          }
                        else
                        {
                          if ((*itTb).second.corrupt)
                            {
                              (*itHarq).second.m_harqStatus.at (tbId.m_layer) = cv2x_DlInfoListElement_s::NACK;
                              NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " harqId " << (uint16_t)(*itTb).second.harqProcessId << " layer " <<(uint16_t)tbId.m_layer << " size " << (*itHarq).second.m_harqStatus.size () << " send DL-HARQ-NACK");
                              m_harqPhyModule->UpdateDlHarqProcessStatus ((*itTb).second.harqProcessId, tbId.m_layer, (*itTb).second.mi, (*itTb).second.size, (*itTb).second.size / EffectiveCodingRate [(*itTb).second.mcs]);
                            }
                          else
                            {
                              NS_ASSERT_MSG (tbId.m_layer < (*itHarq).second.m_harqStatus.size (), " layer " << (uint16_t)tbId.m_layer);
                              (*itHarq).second.m_harqStatus.at (tbId.m_layer) = cv2x_DlInfoListElement_s::ACK;
                              NS_LOG_DEBUG (this << " RNTI " << tbId.m_rnti << " harqId " << (uint16_t)(*itTb).second.harqProcessId << " layer " << (uint16_t)tbId.m_layer << " size " << (*itHarq).second.m_harqStatus.size () << " send DL-HARQ-ACK");
                              m_harqPhyModule->ResetDlHarqProcessStatus ((*itTb).second.harqProcessId);
                            }
                        }
                      } // end if ((*itTb).second.downlink) HARQ
                  } // end if (!(*itTb).second.harqFeedbackSent)
              }
          }
      }

  // send DL HARQ feedback to cv2x_LtePhy
  std::map <uint16_t, cv2x_DlInfoListElement_s>::iterator itHarq;
  for (itHarq = harqDlInfoMap.begin (); itHarq != harqDlInfoMap.end (); itHarq++)
    {
      if (!m_ltePhyDlHarqFeedbackCallback.IsNull ())
        {
          m_ltePhyDlHarqFeedbackCallback ((*itHarq).second);
        }
    }
  // forward control messages of this frame to cv2x_LtePhy
  if (!m_rxControlMessageList.empty ())
    {
      if (!m_ltePhyRxCtrlEndOkCallback.IsNull ())
        {
          m_ltePhyRxCtrlEndOkCallback (m_rxControlMessageList);
        }
    }
  ChangeState (IDLE);
  m_rxPacketBurstList.clear ();
  m_rxControlMessageList.clear ();
  m_expectedTbs.clear ();
}

void
cv2x_LteSpectrumPhy::EndRxSlData ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " ID:" << GetDevice()->GetNode()->GetId() << " state: " << m_state);
  NS_ASSERT (m_state == RX_DATA);

  // this will trigger CQI calculation and Error Model evaluation
  // as a side effect, the error model should update the error status of all TBs
  m_interferenceSl->EndRx ();
  NS_LOG_DEBUG (this << " No. of SL burst " << m_rxPacketInfo.size ());
  NS_LOG_DEBUG (this << " Expected TBs (D2D communication) " << m_expectedSlTbs.size ());
  NS_LOG_DEBUG (this << " Expected TBs (V2X communication) " << m_expectedSlV2xTbs.size()); 
  NS_LOG_DEBUG (this << " Expected TBs (discovery) " << m_expectedDiscTbs.size ());
  NS_LOG_DEBUG (this << " No. of Ctrl messages " << m_rxControlMessageList.size());

  // apply transmission mode gain
  // TODO: Check what is the mode for D2D (SIMO?)
  //       should it be done to each SINR reported?  
  //NS_LOG_DEBUG (this << " txMode " << (uint16_t)m_transmissionMode << " gain " << m_txModeGain.at (m_transmissionMode));
  NS_ASSERT (m_transmissionMode < m_txModeGain.size ());  
  //m_sinrPerceived *= m_txModeGain.at (m_transmissionMode);
  
  //Compute error on PSSCH
  //Create a mapping between the packet tag and the index of the packet bursts. We need this information to access the right SINR measurement.
  std::map <cv2x_SlTbId_t, uint32_t> expectedTbToSinrIndex;
  std::map <cv2x_SlV2xTbId_t, uint32_t> expectedTbToSinrIndexV2x;
  for (uint32_t i = 0 ; i < m_rxPacketInfo.size() ; i++)
    {
      //even though there may be multiple packets, they all have
      //the same tag
      if (m_rxPacketInfo[i].m_rxPacketBurst) //if data packet
        {
          std::list<Ptr<Packet> >::const_iterator j = m_rxPacketInfo[i].m_rxPacketBurst->Begin (); 
          // retrieve TB info of this packet 
          cv2x_LteRadioBearerTag tag;
          (*j)->PeekPacketTag (tag);
          /*if (m_expectedSlV2xTbs.size() > 0)
            {
              cv2x_SlV2xTbId_t tbId;
              tbId.m_rnti = tag.GetRnti ();
              expectedTbToSinrIndexV2x.insert (std::pair<cv2x_SlV2xTbId_t, uint32_t> (tbId, i));
            }
          else*/
          if (m_expectedSlTbs.size() >0)
            {
              cv2x_SlTbId_t tbId;
              tbId.m_rnti = tag.GetRnti ();
              tbId.m_l1dst = tag.GetDestinationL2Id () & 0xFF;
              expectedTbToSinrIndex.insert (std::pair<cv2x_SlTbId_t, uint32_t> (tbId, i));
            }
        }
    }

  // V2x communication
  for (uint32_t i = 0; i < m_rxPacketInfo.size(); i++)
    {
      if (m_rxPacketInfo[i].m_rxControlMessage)
      {
        if (m_rxPacketInfo[i].m_rxControlMessage->GetMessageType () == cv2x_LteControlMessage::SCI_V2X)
        {
          Ptr<cv2x_LteControlMessage> rxCtrlMsg = m_rxPacketInfo[i].m_rxControlMessage;
          Ptr<cv2x_SciLteControlMessageV2x> msg = DynamicCast<cv2x_SciLteControlMessageV2x> (rxCtrlMsg);
          cv2x_SciListElementV2x sci = msg->GetSci (); 

          cv2x_SlV2xTbId_t tbId; 
          tbId.m_rnti = sci.m_rnti; 
          expectedTbToSinrIndexV2x.insert (std::pair<cv2x_SlV2xTbId_t, uint32_t> (tbId,i)); 

          std::list<Ptr<SidelinkCommResourcePoolV2x > >::iterator sciIt;
          for (sciIt = m_slV2xRxPools.begin(); sciIt != m_slV2xRxPools.end(); sciIt++)
          {
            std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo> m_psschTx = (*sciIt)->GetPsschTransmissions(sci.m_riv,sci.m_resPscch);
            std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator rxIt = m_psschTx.begin(); 
            if(rxIt != m_psschTx.end())
            {
              //reception
              std::list<SidelinkCommResourcePoolV2x::SidelinkTransmissionInfo>::iterator txIt = m_psschTx.begin(); 
              std::vector<int> rbMap; 
              for (int i = txIt->rbStart; i< txIt->rbStart+txIt->rbLen; i++)
              {
                rbMap.push_back(i); 
              }
              AddExpectedTbV2x (sci.m_rnti, sci.m_tbSize, sci.m_mcs, rbMap);
            }
          }
        }
      }
    }

  std::set<int> rbDecodedBitmap;
  if (m_dropRbOnCollisionEnabled)
    {
      NS_LOG_DEBUG (this << " PSSCH DropOnCollisionEnabled: Identifying RB Collisions");
      //Add new loop to make one pass and identify which RB have collisions
      
      std::set<int> rbDecodedBitmapTemp;
      for (expectedSlTbs_t::iterator itTb = m_expectedSlTbs.begin (); itTb != m_expectedSlTbs.end (); itTb++ )
        {
          for (std::vector<int>::iterator rbIt =  (*itTb).second.rbBitmap.begin (); rbIt != (*itTb).second.rbBitmap.end(); rbIt++)
            {
              if (rbDecodedBitmapTemp.find (*rbIt) != rbDecodedBitmapTemp.end()) 
                {
		              //collision, update the bitmap
                  rbDecodedBitmap.insert (*rbIt);  
                }
              else
                {
                  //store resources used by the packet to detect collision
                  rbDecodedBitmapTemp.insert (*rbIt);  
                }
            }
        }
      for (expectedSlV2xTbs_t::iterator itTb = m_expectedSlV2xTbs.begin(); itTb != m_expectedSlV2xTbs.end(); itTb++)
        {
          for (std::vector<int>::iterator rbIt =  (*itTb).second.rbBitmap.begin (); rbIt != (*itTb).second.rbBitmap.end(); rbIt++)
            {
              if (rbDecodedBitmapTemp.find (*rbIt) != rbDecodedBitmapTemp.end()) 
                {
		              //collision, update the bitmap
                  rbDecodedBitmap.insert (*rbIt);  
                }
              else
                {
                  //store resources used by the packet to detect collision
                  rbDecodedBitmapTemp.insert (*rbIt);  
                }
            }
        }

    }

  //Compute error for each expected Tb
  expectedSlTbs_t::iterator itTb = m_expectedSlTbs.begin ();
  std::map <cv2x_SlTbId_t, uint32_t>::iterator itSinr;
  while (itTb!=m_expectedSlTbs.end ())
    { 
      itSinr = expectedTbToSinrIndex.find ((*itTb).first);
      if ((m_dataErrorModelEnabled)&&(m_rxPacketInfo.size ()>0)&&(itSinr != expectedTbToSinrIndex.end())) // avoid to check for errors when there is no actual data transmitted
        {
          // retrieve HARQ info
          HarqProcessInfoList_t harqInfoList;
          if ((*itTb).second.ndi == 0)
            {
              harqInfoList = m_harqPhyModule->GetHarqProcessInfoSl ((*itTb).first.m_rnti, (*itTb).first.m_l1dst);
              NS_LOG_DEBUG (this << " Nb Retx=" << harqInfoList.size());
              //std::cout << this << " Nb Retx=" << harqInfoList.size() << std::endl;
            }
          
          NS_LOG_DEBUG(this << "\t" << Simulator::Now ().GetMilliSeconds () << "\tFrom: " << (*itTb).first.m_rnti << "\tCorrupt: " << (*itTb).second.corrupt);

          bool rbDecoded = false;
          if (m_dropRbOnCollisionEnabled)
            {
              NS_LOG_DEBUG (this << " PSSCH DropOnCollisionEnabled: Labeling Corrupted TB");
              //Check if any of the RBs have been decoded
              for (std::vector<int>::iterator rbIt =  (*itTb).second.rbBitmap.begin (); rbIt != (*itTb).second.rbBitmap.end(); rbIt++)
                 {
                   if (rbDecodedBitmap.find (*rbIt) != rbDecodedBitmap.end ())
                     {
                        NS_LOG_DEBUG( this << "\t" << *rbIt << " decoded, labeled as corrupted!");
                        rbDecoded = true;
                        (*itTb).second.corrupt = true;
                        break;
                     }
                  }
            }

          if (!m_nistErrorModelEnabled)
            {
              cv2x_TbStats_t tbStats = cv2x_LteMiErrorModel::GetTbDecodificationStats (m_slSinrPerceived[(*itSinr).second]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, (*itTb).second.rbBitmap, (*itTb).second.size, (*itTb).second.mcs, harqInfoList);
              (*itTb).second.mi = tbStats.mi;
                if(m_slBlerEnabled)
                  {
                    if(!rbDecoded)
                      {
                        (*itTb).second.corrupt = m_random->GetValue () > tbStats.tbler ? false : true;
                      }
                  }
              NS_LOG_DEBUG (this << " from RNTI " << (*itTb).first.m_rnti << " size " << (*itTb).second.size << " mcs " << (uint32_t)(*itTb).second.mcs << " bitmap " << (*itTb).second.rbBitmap.size () << " TBLER " << tbStats.tbler << " corrupted " << (*itTb).second.corrupt);
              //std::cout << this << " from RNTI " << (*itTb).first.m_rnti << " size " << (*itTb).second.size << " mcs " << (uint32_t)(*itTb).second.mcs << " bitmap " << (*itTb).second.rbBitmap.size () << " TBLER " << tbStats.tbler << " corrupted " << (*itTb).second.corrupt << " mean SINR " << GetMeanSinr (m_slSinrPerceived[(*itSinr).second]*4, (*itTb).second.rbBitmap) << std::endl;
            } 
          else 
            {
              TbErrorStats_t tbStats = cv2x_LtePhyErrorModel::GetPsschBler (m_fadingModel,cv2x_LtePhyErrorModel::SISO, (*itTb).second.mcs, GetMeanSinr (m_slSinrPerceived[(*itSinr).second]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, (*itTb).second.rbBitmap),  harqInfoList);
              (*itTb).second.sinr = tbStats.sinr;
              if(m_slBlerEnabled)
                {
                  if (!rbDecoded)
                    {
                      (*itTb).second.corrupt = m_random->GetValue () > tbStats.tbler ? false : true;
                    }
                }
              NS_LOG_DEBUG (this << " from RNTI " << (*itTb).first.m_rnti << " size " << (*itTb).second.size << " mcs " << (uint32_t)(*itTb).second.mcs << " bitmap " << (*itTb).second.rbBitmap.size () << " TBLER " << tbStats.tbler << " corrupted " << (*itTb).second.corrupt);
              //std::cout << this << " from RNTI " << (*itTb).first.m_rnti << " size " << (*itTb).second.size << " mcs " << (uint32_t)(*itTb).second.mcs << " bitmap " << (*itTb).second.rbBitmap.size () << " TBLER " << tbStats.tbler << " corrupted " << (*itTb).second.corrupt << " mean SINR " << GetMeanSinr (m_slSinrPerceived[(*itSinr).second]*4, (*itTb).second.rbBitmap) << std::endl;

            }
          // std::cout << "Expected transmission on RBs= ";
          // for (uint32_t i = 0; i < (*itTb).second.rbBitmap.size (); i++)
          //   {
          //     std::cout << (*itTb).second.rbBitmap.at(i) << ",";
          //   }
          // std::cout << std::endl;
          // fire traces on SL reception PHY stats
          cv2x_PhyReceptionStatParameters params;
          params.m_timestamp = Simulator::Now ().GetMilliSeconds ();
          params.m_cellId = m_cellId;
          params.m_imsi = 0; // it will be set by DlPhyTransmissionCallback in cv2x_LteHelper
          params.m_rnti = (*itTb).first.m_rnti;
          params.m_txMode = m_transmissionMode;
          params.m_layer =  0;
          params.m_mcs = (*itTb).second.mcs;
          params.m_size = (*itTb).second.size;
          params.m_rv = (*itTb).second.rv;
          params.m_ndi = (*itTb).second.ndi;
          params.m_correctness = (uint8_t)!(*itTb).second.corrupt;
          params.m_sinrPerRb = GetMeanSinr (m_slSinrPerceived[(*itSinr).second]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, (*itTb).second.rbBitmap);

          params.m_rv = harqInfoList.size ();
          m_slPhyReception (params);          
        }
      
      itTb++;
    }

  //Compute error for each expected Tb
  expectedSlV2xTbs_t::iterator itTbV2x = m_expectedSlV2xTbs.begin ();
  std::map <cv2x_SlV2xTbId_t, uint32_t>::iterator itSinrV2x;
  while (itTbV2x != m_expectedSlV2xTbs.end ())
    {
      NS_LOG_LOGIC (this << " V2X: compute error for each expected Tb");
     
      itSinrV2x = expectedTbToSinrIndexV2x.find ((*itTbV2x).first);
      if ((m_dataErrorModelEnabled)&&(m_rxPacketInfo.size ()>0)&&(itSinrV2x != expectedTbToSinrIndexV2x.end())) // avoid to check for errors when there is no actual data transmitted
        {
          // retrieve HARQ info 
          HarqProcessInfoList_t harqInfoList;

          NS_LOG_DEBUG(this << "\t" << Simulator::Now ().GetMilliSeconds () << "\tFrom: " << (*itTbV2x).first.m_rnti << "\tCorrupt: " << (*itTbV2x).second.corrupt);

          bool rbDecoded = false;
          if (m_dropRbOnCollisionEnabled)
          {
            NS_LOG_DEBUG (this << " PSSCH DropOnCollisionEnabled: Labeling Corrupted TB");
            //Check if any of the RBs have been decoded
            for (std::vector<int>::iterator rbIt =  (*itTbV2x).second.rbBitmap.begin (); rbIt != (*itTbV2x).second.rbBitmap.end(); rbIt++)
                {
                  if (rbDecodedBitmap.find (*rbIt) != rbDecodedBitmap.end ())
                    {
                      NS_LOG_DEBUG( this << "\t" << *rbIt << " decoded, labeled as corrupted!");
                      rbDecoded = true;
                      (*itTbV2x).second.corrupt = true;
                      break;
                    }
                }
          }

          if (!m_nistErrorModelEnabled)
          {
            NS_LOG_LOGIC (this << " nist error model not enabled");
            cv2x_TbStats_t tbStats = cv2x_LteMiErrorModel::GetTbDecodificationStats (m_slSinrPerceived[(*itSinrV2x).second]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, (*itTbV2x).second.rbBitmap, (*itTbV2x).second.size, (*itTbV2x).second.mcs, harqInfoList);
            (*itTbV2x).second.mi = tbStats.mi;
              if(m_slBlerEnabled)
                {
                  if(!rbDecoded)
                    {
                      (*itTbV2x).second.corrupt = m_random->GetValue () > tbStats.tbler ? false : true;
                    }
                }
            NS_LOG_DEBUG (this << " from RNTI " << (*itTbV2x).first.m_rnti << " size " << (*itTbV2x).second.size << " mcs " << (uint32_t)(*itTbV2x).second.mcs << " bitmap " << (*itTbV2x).second.rbBitmap.size () << " TBLER " << tbStats.tbler << " corrupted " << (*itTbV2x).second.corrupt);
          } 
          else 
          {
            NS_LOG_LOGIC (this << " nist error model enabled");
            TbErrorStats_t tbStats = cv2x_LtePhyErrorModel::GetPsschBler (m_fadingModel,cv2x_LtePhyErrorModel::SISO, (*itTbV2x).second.mcs, GetMeanSinr (m_slSinrPerceived[(*itSinrV2x).second]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, (*itTbV2x).second.rbBitmap),  harqInfoList);
            (*itTbV2x).second.sinr = tbStats.sinr;
            if(m_slBlerEnabled)
              {
                if (!rbDecoded)
                  {
                    (*itTbV2x).second.corrupt = m_random->GetValue () > tbStats.tbler ? false : true;
                  }
              }
            /*std::cout  << " from RNTI " << (*itTbV2x).first.m_rnti 
                                << " size " << (*itTbV2x).second.size 
                                << " mcs " << (uint32_t)(*itTbV2x).second.mcs 
                                << " bitmap " << (*itTbV2x).second.rbBitmap.size () 
                                << " TBLER " << tbStats.tbler 
                                << " SINR " << tbStats.sinr 
                                << " corrupted " << (*itTbV2x).second.corrupt << std::endl;*/
            NS_LOG_DEBUG (this  << " from RNTI " << (*itTbV2x).first.m_rnti 
                                << " size " << (*itTbV2x).second.size 
                                << " mcs " << (uint32_t)(*itTbV2x).second.mcs 
                                << " bitmap " << (*itTbV2x).second.rbBitmap.size () 
                                << " TBLER " << tbStats.tbler 
                                << " corrupted " << (*itTbV2x).second.corrupt);
          }

        // fire traces on SL reception PHY stats
        cv2x_PhyReceptionStatParameters params;
        params.m_timestamp = Simulator::Now ().GetMilliSeconds ();
        params.m_cellId = m_cellId;
        params.m_imsi = 0; // it will be set by DlPhyTransmissionCallback in cv2x_LteHelper
        params.m_rnti = (*itTbV2x).first.m_rnti;
        params.m_txMode = m_transmissionMode;
        params.m_layer =  0;
        params.m_mcs = (*itTbV2x).second.mcs;
        params.m_size = (*itTbV2x).second.size;
        params.m_correctness = (uint8_t)!(*itTbV2x).second.corrupt;
        params.m_sinrPerRb = GetMeanSinr (m_slSinrPerceived[(*itSinrV2x).second]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, (*itTbV2x).second.rbBitmap);
        m_slPhyReception (params);          
      }
      itTbV2x++;
    }

  for (uint32_t i = 0 ; i < m_rxPacketInfo.size() ; i++)
    {
      //even though there may be multiple packets, they all have
      //the same tag
      if (m_rxPacketInfo[i].m_rxPacketBurst) //if data packet
        {
          for (std::list<Ptr<Packet> >::const_iterator j = m_rxPacketInfo[i].m_rxPacketBurst->Begin (); j != m_rxPacketInfo[i].m_rxPacketBurst->End (); ++j)
            {
              // retrieve TB info of this packet 
              cv2x_LteRadioBearerTag tag;
              (*j)->PeekPacketTag (tag);
              if (m_expectedSlV2xTbs.size() > 0)
                {
                  cv2x_SlV2xTbId_t tbId;
                  tbId.m_rnti = tag.GetRnti ();
                  NS_LOG_INFO (this << " Packet of " << tbId.m_rnti);

                  itTbV2x = m_expectedSlV2xTbs.find (tbId);
                  if (itTbV2x!=m_expectedSlV2xTbs.end ())
                    {
                      if (!(*itTbV2x).second.corrupt)
                        {
                          NS_LOG_LOGIC (this << " packet OK");
                          m_phyRxEndOkTrace (*j);
                    
                          if (!m_ltePhyRxDataEndOkCallback.IsNull ())
                            {
                              m_ltePhyRxDataEndOkCallback (*j);
                            }
                        }
                      else
                        {
                          // TB received with errors
                          NS_LOG_LOGIC (this << " TB received with errors");
                          m_phyRxEndErrorTrace (*j);
                        }
                    }
                  NS_ASSERT (itTb!=m_expectedSlTbs.end () || itTbV2x!=m_expectedSlV2xTbs.end ());
                }
              else
                {
                  cv2x_SlTbId_t tbId;
                  tbId.m_rnti = tag.GetRnti ();
                  tbId.m_l1dst = tag.GetDestinationL2Id () & 0xFF;
                  NS_LOG_INFO (this << " Packet of " << tbId.m_rnti << " group " <<  (uint16_t) tbId.m_l1dst);
                  
                  itTb = m_expectedSlTbs.find (tbId);
                  if (itTb!=m_expectedSlTbs.end ())
                    {
                      if (!(*itTb).second.corrupt)
                        {
                          NS_LOG_LOGIC (this << " packet OK");
                          m_phyRxEndOkTrace (*j);
                    
                          if (!m_ltePhyRxDataEndOkCallback.IsNull ())
                            {
                              m_ltePhyRxDataEndOkCallback (*j);
                            }
                        }
                      else
                        {
                          // TB received with errors
                          NS_LOG_LOGIC (this << " TB received with errors");
                          m_phyRxEndErrorTrace (*j);
                        }

                      //store HARQ information
                      if (!(*itTb).second.harqFeedbackSent)
                        {
                          (*itTb).second.harqFeedbackSent = true;
                          //because we do not have feedbacks we do not reset HARQ now.
                          //we will do it when we expect a new data
                          if ((*itTb).second.corrupt)
                            {
                              if (!m_nistErrorModelEnabled)
                                {
                                  m_harqPhyModule->UpdateSlHarqProcessStatus (tbId.m_rnti, tbId.m_l1dst, (*itTb).second.mi, (*itTb).second.size, (*itTb).second.size / EffectiveCodingRate [(*itTb).second.mcs]);
                                }
                              else
                                {
                                  m_harqPhyModule->UpdateSlHarqProcessStatus (tbId.m_rnti, tbId.m_l1dst, (*itTb).second.sinr);
                                }
                            }
                          else
                            {
                              //m_harqPhyModule->ResetSlHarqProcessStatus (tbId.m_rnti, tbId.m_l1dst);
                            }
                          /*
                            if (!m_ltePhySlHarqFeedbackCallback.IsNull ())
                            {
                            m_ltePhySlHarqFeedbackCallback (harqSlInfo);
                            }
                          */
                        }
                    }
                }
            }
        }
    }


  /* Currently the MIB-SL is treated as a control message. Thus, the following logic applies also to the MIB-SL
   * The differences: calculation of BLER */
  // When control messages collide in the PSCCH, the receiver cannot know how many transmissions occured
  // we sort the messages by SINR and try to decode the ones with highest average SINR per RB first
  // only one message per RB can be decoded
  std::list<Ptr<cv2x_LteControlMessage> > rxControlMessageOkList;
  bool error = true; 
  bool ctrlMessageFound = false;
  std::multiset<cv2x_SlCtrlPacketInfo_t> sortedControlMessages;
  rbDecodedBitmap.clear ();

  for (uint32_t i = 0 ; i < m_rxPacketInfo.size() ; i++)
    {
      if (m_rxPacketInfo[i].m_rxControlMessage && m_rxPacketInfo[i].m_rxControlMessage->GetMessageType () != cv2x_LteControlMessage::SL_DISC_MSG) //if control packet               
        {
          double meanSinr = GetMeanSinr (m_slSinrPerceived[i], m_rxPacketInfo[i].rbBitmap);
          cv2x_SlCtrlPacketInfo_t pInfo;
          pInfo.sinr = meanSinr;          
          pInfo.index = i;
          sortedControlMessages.insert (pInfo);
          //ctrl_msg_count++;
        }
    }
  
  if (m_dropRbOnCollisionEnabled)
    {
      NS_LOG_DEBUG (this << "Ctrl DropOnCollisionEnabled");
      //Add new loop to make one pass and identify which RB have collisions
      std::set<int> rbDecodedBitmapTemp;
      //DEBUG:
      //std::cout<<Simulator::Now ().GetMilliSeconds () << " DEBUG sortedControlMessages RB:";

      for (std::multiset<cv2x_SlCtrlPacketInfo_t>::iterator it =sortedControlMessages.begin(); it != sortedControlMessages.end(); it++ )
        {
          int i = (*it).index;
          
          //DEBUG:
          //for (std::vector<int>::iterator rbIt =  m_rxPacketInfo[i].rbBitmap.begin (); rbIt != m_rxPacketInfo[i].rbBitmap.end(); rbIt++)
          //  {
          //    std::cout<< " " << *rbIt;
          //  }
          ///////

          for (std::vector<int>::iterator rbIt =  m_rxPacketInfo[i].rbBitmap.begin (); rbIt != m_rxPacketInfo[i].rbBitmap.end(); rbIt++)
            {
              if (rbDecodedBitmapTemp.find (*rbIt) != rbDecodedBitmapTemp.end()) 
                {
		          //collision, update the bitmap
                  rbDecodedBitmap.insert ( m_rxPacketInfo[i].rbBitmap.begin(), m_rxPacketInfo[i].rbBitmap.end());  
                  break;
                }
              else
                {
                  //store resources used by the packet to detect collision
                  rbDecodedBitmapTemp.insert ( m_rxPacketInfo[i].rbBitmap.begin(), m_rxPacketInfo[i].rbBitmap.end());  
                }
            }
        }
      
      /////// DEBUG: /////////
      //std::cout<<std::endl;
      //std::cout<< this << " DEBUG rbDecodedBitmapTemp:";
      //for(std::set<int>::iterator it = rbDecodedBitmapTemp.begin(); it!= rbDecodedBitmapTemp.end(); it++)
      //  {
      //    std::cout<< " " << *it;
      //  }
      //  std::cout<<std::endl;
      ///////////////////////
    }
  /////// DEBUG: /////////
  //std::cout<< this << " DEBUG rbDecodedBitmap:";
  //for(std::set<int>::iterator it = rbDecodedBitmap.begin(); it!= rbDecodedBitmap.end(); it++)
  //  {
  //    std::cout<< " " << *it;
  //  }
  //  std::cout<<std::endl;
  ///////////////////////

  for (std::multiset<cv2x_SlCtrlPacketInfo_t>::iterator it = sortedControlMessages.begin(); it != sortedControlMessages.end() ; it++ )
    {
      int i = (*it).index;

      bool ctrlError = false;
      ctrlMessageFound = true;
      
      if (m_ctrlErrorModelEnabled)
        {
          bool rbDecoded = false;
          for (std::vector<int>::iterator rbIt =  m_rxPacketInfo[i].rbBitmap.begin ();  rbIt != m_rxPacketInfo[i].rbBitmap.end() ; rbIt++)
            {
              if (rbDecodedBitmap.find (*rbIt) != rbDecodedBitmap.end()) 
                {
                  rbDecoded = true;
                  ctrlError = true;
                  NS_LOG_DEBUG (this << " RB " << *rbIt << " has already been decoded ");
                  break;
                }
            }
          if (!rbDecoded)
            {
              //if (!m_dropRbOnCollisionEnabled)//uncomment IF_CONDITION if PSCCH BLER is not considered in this mode!
                //{
                  double  errorRate;
                  if (m_rxPacketInfo[i].m_rxControlMessage->GetMessageType() == cv2x_LteControlMessage::SCI)
                    {
                      errorRate = cv2x_LtePhyErrorModel::GetPscchBler (m_fadingModel,cv2x_LtePhyErrorModel::SISO, GetMeanSinr (m_slSinrPerceived[i]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, m_rxPacketInfo[i].rbBitmap)).tbler;
                      ctrlError = m_random->GetValue () > errorRate ? false : true;
                      NS_LOG_DEBUG (this << " PSCCH Decoding, errorRate " << errorRate << " error " << ctrlError);
                    }
                  else if (m_rxPacketInfo[i].m_rxControlMessage->GetMessageType() == cv2x_LteControlMessage::SCI_V2X)
                    {
                      std::vector<int> pscchBitmap = m_rxPacketInfo[i].rbBitmap;
                      while (pscchBitmap.size() != 2)
                      {
                        pscchBitmap.pop_back(); 
                      }
                      errorRate = cv2x_LtePhyErrorModel::GetPscchBler (m_fadingModel,cv2x_LtePhyErrorModel::SISO, GetMeanSinr (m_slSinrPerceived[i]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, pscchBitmap)).tbler;
                      ctrlError = m_random->GetValue () > errorRate ? false : true;
                      NS_LOG_DEBUG (this << " PSCCH Decoding, errorRate " << errorRate << " error " << ctrlError);
                    }
                  else if (m_rxPacketInfo[i].m_rxControlMessage->GetMessageType() == cv2x_LteControlMessage::MIB_SL)
                    {
                      errorRate = cv2x_LtePhyErrorModel::GetPsbchBler (m_fadingModel,cv2x_LtePhyErrorModel::SISO, GetMeanSinr (m_slSinrPerceived[i]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, m_rxPacketInfo[i].rbBitmap)).tbler;
                      ctrlError = m_random->GetValue () > errorRate ? false : true;
                      NS_LOG_DEBUG (this << " PSBCH Decoding, errorRate " << errorRate << " error " << ctrlError);
                    }
                  else
                    {
                      NS_LOG_DEBUG (this << " Unknown SL control message ");
                    }
                //}
            }
        }

      if (!ctrlError)
        {
          error = false; //at least one control packet is OK
          rxControlMessageOkList.push_back (m_rxPacketInfo[i].m_rxControlMessage);
          rbDecodedBitmap.insert ( m_rxPacketInfo[i].rbBitmap.begin(), m_rxPacketInfo[i].rbBitmap.end());
        }

      if(m_rxPacketInfo[i].m_rxControlMessage->GetMessageType () == cv2x_LteControlMessage::SCI)
        {

          // Add PSCCH trace.
          NS_ASSERT(m_rxPacketInfo[i].m_rxControlMessage->GetMessageType () == cv2x_LteControlMessage::SCI);
          Ptr<cv2x_SciLteControlMessage> msg2 = DynamicCast<cv2x_SciLteControlMessage> (m_rxPacketInfo[i].m_rxControlMessage);
          cv2x_SciListElement_s sci = msg2->GetSci ();

          cv2x_PhyReceptionStatParameters params;
          params.m_timestamp = Simulator::Now ().GetMilliSeconds ();
          params.m_cellId = m_cellId;
          params.m_imsi = 0; // it will be set by DlPhyTransmissionCallback in cv2x_LteHelper
          params.m_rnti = sci.m_rnti;
          params.m_layer =  0;
          params.m_mcs = sci.m_mcs;
          params.m_size = sci.m_tbSize;
          params.m_rv = sci.m_rbStart;    // Using m_rv to store the RB start
          params.m_ndi = sci.m_rbLen;     // Using m_ndi to store the number of RBs used
          params.m_correctness = (uint8_t)!ctrlError;
          params.m_sinrPerRb = 0; // NOT USED, JUST INITIALIZED TO AVOID COMPILATION WARNING!
          params.m_txMode = m_transmissionMode; // NOT USED, JUST INITIALIZED TO AVOID COMPILATION WARNING!
          // Call trace
          m_slPscchReception (params);
        }
      else if (m_rxPacketInfo[i].m_rxControlMessage->GetMessageType() == cv2x_LteControlMessage::SCI_V2X)
        {
          // Add PSCCH trace.
          Ptr<cv2x_SciLteControlMessageV2x> msg2 = DynamicCast<cv2x_SciLteControlMessageV2x> (m_rxPacketInfo[i].m_rxControlMessage);
          cv2x_SciListElementV2x sci = msg2->GetSci ();
          
          cv2x_PhyReceptionStatParameters params;
          params.m_timestamp = Simulator::Now().GetMilliSeconds();
          params.m_cellId = m_cellId; 
          params.m_imsi = 0;
          params.m_rnti = sci.m_rnti; 
          params.m_layer = 0;
          params.m_mcs = sci.m_mcs;
          params.m_size = sci.m_tbSize; 
          params.m_rv = sci.m_resPscch; // Using m_rv to store the pscch resource
          params.m_ndi = 0; 
          params.m_correctness = (uint8_t)!ctrlError; 
          params.m_sinrPerRb = 0; // NOT USED, JUST INITIALIZED TO AVOID COMPILATION WARNING!
          params.m_txMode = m_transmissionMode; // NOT USED, JUST INITIALIZED TO AVOID COMPILATION WARNING!
          // Call trace
          m_slPscchReception(params);
        }
    }

  if (ctrlMessageFound)
    {
      if (!error)
        {
          if (!m_ltePhyRxCtrlEndOkCallback.IsNull ())
            {
              NS_LOG_DEBUG (this << " PSCCH OK");
              m_ltePhyRxCtrlEndOkCallback (rxControlMessageOkList);
            }
        }
      else
        {
          if (!m_ltePhyRxCtrlEndErrorCallback.IsNull ())
            {
              NS_LOG_DEBUG (this << " PSCCH Error");
              m_ltePhyRxCtrlEndErrorCallback ();
            }
        }
    }

    //discovery
    
    // error model and harq enabled for d2d discovery 
    if (m_errorModelHarqD2dDiscoveryEnabled)
    {
      std::map<cv2x_DiscTbId_t, uint32_t> expectedTbToSinrDiscIndex;
      for (uint32_t i = 0 ; i < m_rxPacketInfo.size() ; i++)
        {
          //data isn't included and control is discovery message
          if (m_rxPacketInfo[i].m_rxControlMessage)
          {
            if (!m_rxPacketInfo[i].m_rxPacketBurst && m_rxPacketInfo[i].m_rxControlMessage->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG)
              {
                Ptr<cv2x_LteControlMessage> rxCtrlMsg= m_rxPacketInfo[i].m_rxControlMessage;
                Ptr<cv2x_SlDiscMessage> msg = DynamicCast<cv2x_SlDiscMessage> (rxCtrlMsg);
                cv2x_SlDiscMsg disc = msg->GetSlDiscMessage ();
                bool exist = FilterRxApps (disc);
                if (exist)
                {
                  // retrieve TB info of this packet
                  cv2x_DiscTbId_t tbId;
                  tbId.m_rnti = disc.m_rnti;
                  tbId.m_resPsdch = disc.m_resPsdch;
                  expectedTbToSinrDiscIndex.insert (std::pair<cv2x_DiscTbId_t, uint8_t> (tbId, i));

                  std::list<Ptr<SidelinkDiscResourcePool> >::iterator discIt;
                  for (discIt = m_discRxPools.begin (); discIt != m_discRxPools.end(); discIt++)
                    {
                      std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo> m_psdchTx = (*discIt)->GetPsdchTransmissions (disc.m_resPsdch);
                      std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo>::iterator rxIt = m_psdchTx.begin ();
                      if (rxIt != m_psdchTx.end())
                        {
                          //reception
                          NS_LOG_INFO (this << " Expecting PSDCH reception RB " << (uint16_t) (disc.m_resPsdch));
                          std::list<SidelinkDiscResourcePool::SidelinkTransmissionInfo>::iterator txIt = m_psdchTx.begin ();
                          std::vector <int> rbMap;
                          for (int i = txIt->rbStart ; i < txIt->rbStart + txIt->nbRb ; i++)
                          {
                            NS_LOG_LOGIC (this << " Receiving PSDCH on RB " << i);
                            rbMap.push_back (i);
                          }
                      
                          AddExpectedTb (disc.m_rnti, disc.m_resPsdch, m_psdchTx.size() % 4 == 0, rbMap, (4 - m_psdchTx.size () % 4));
                        }
                    }
                }
              }
          }
        }

      //           
      expectedDiscTbs_t::iterator itTbDisc = m_expectedDiscTbs.begin ();
      std::map<cv2x_DiscTbId_t, uint32_t>::iterator itSinrDisc;
      while (itTbDisc!= m_expectedDiscTbs.end ())
        {
          itSinrDisc = expectedTbToSinrDiscIndex.find ((*itTbDisc).first);
          //to check: m_ctrlErrorModelEnabled or should we have m_dataErrorModelEnabled enabled?
          if ((m_ctrlErrorModelEnabled)&&(m_rxPacketInfo.size ()>0)&&(itSinrDisc != expectedTbToSinrDiscIndex.end())) // avoid to check for errors when there is no actual discovery transmitted
            {   
              // retrieve HARQ info
              HarqProcessInfoList_t harqInfoList;
              if ((*itTbDisc).second.ndi == 0)
                {
                  harqInfoList = m_harqPhyModule->GetHarqProcessInfoDisc ((*itTbDisc).first.m_rnti,(*itTbDisc).first.m_resPsdch);
                  NS_LOG_DEBUG (this << " Nb Retx=" << harqInfoList.size());
                  //std::cout << this << " Nb Retx=" << harqInfoList.size() << std::endl;
                }
                           
              if (!m_nistErrorModelEnabled)
                {
                  NS_LOG_ERROR ("Any error model other than the NistErrorModel is not supported");
                } 
              else 
                {
                  TbErrorStats_t tbStats = cv2x_LtePhyErrorModel::GetPsdchBler (m_fadingModel,cv2x_LtePhyErrorModel::SISO, GetMeanSinr (m_slSinrPerceived[(*itSinrDisc).second]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, (*itTbDisc).second.rbBitmap),  harqInfoList);
                  (*itTbDisc).second.sinr = tbStats.sinr;
                  (*itTbDisc).second.corrupt = m_random->GetValue () > tbStats.tbler ? false : true;
                  NS_LOG_DEBUG (this << " from RNTI " << (*itTbDisc).first.m_rnti << " TBLER " << tbStats.tbler << " corrupted " << (*itTbDisc).second.corrupt);
                  //std::cout << this << " from RNTI " << (*itTbDisc).first.m_rnti << " TBLER " << tbStats.tbler << " corrupted " << (*itTbDisc).second.corrupt << " mean SINR " << GetMeanSinr (m_slSinrPerceived[(*itSinrDisc).second]*4, (*itTbDisc).second.rbBitmap) << std::endl;
                }
                  
              //traces for discovery rx
              //we would know it is discovery mcs=0 and size=232
              cv2x_PhyReceptionStatParameters params;
              params.m_timestamp = Simulator::Now ().GetMilliSeconds ();
              params.m_cellId = m_cellId;
              params.m_imsi = 0; // it will be set by DlPhyTransmissionCallback in cv2x_LteHelper
              params.m_rnti = (*itTbDisc).first.m_rnti;
              params.m_txMode = m_transmissionMode;
              params.m_layer =  0;
              params.m_mcs = 0; //for discovery, we use a fixed modulation (no mcs defined), use 0 to identify discovery
              params.m_size = 232; // discovery message has a static size
              params.m_rv = (*itTbDisc).second.rv;
              params.m_ndi = (*itTbDisc).second.ndi;
              params.m_correctness = (uint8_t)!(*itTbDisc).second.corrupt;
              params.m_sinrPerRb = GetMeanSinr (m_slSinrPerceived[(*itSinrDisc).second]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, (*itTbDisc).second.rbBitmap);
              params.m_rv = harqInfoList.size ();
              m_slPhyReception (params);  
            }
          itTbDisc++;
        }

      

      // handling collison for discovery
      // same as sidelink control but taking into account HARQ
      std::list<Ptr<cv2x_LteControlMessage> > rxDiscMessageOkList;
      
      bool discError = true; 
      bool discMessageFound = false;
      std::set<cv2x_SlCtrlPacketInfo_t> sortedDiscMessages;
      std::set<int> discDecodedBitmap;

      for (uint32_t i = 0 ; i < m_rxPacketInfo.size() ; i++)
        {
          Ptr<cv2x_LteControlMessage> rxCtrlMsg= m_rxPacketInfo[i].m_rxControlMessage;
          Ptr<cv2x_SlDiscMessage> msg = DynamicCast<cv2x_SlDiscMessage> (rxCtrlMsg);
          if (msg)
          {
            cv2x_SlDiscMsg disc = msg->GetSlDiscMessage ();
            bool exist = FilterRxApps (disc);
            if (exist)
            {
              if (m_rxPacketInfo[i].m_rxControlMessage->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG) //if discovery message     
                {
                  double meanSinr = GetMeanSinr (m_slSinrPerceived[i], m_rxPacketInfo[i].rbBitmap);
                  cv2x_SlCtrlPacketInfo_t pInfo;
                  pInfo.sinr = meanSinr;          
                  pInfo.index = i;
                  sortedDiscMessages.insert (pInfo);
                }
            }
          }
        }
     
      // check all sorted discovery messages
      for (std::set<cv2x_SlCtrlPacketInfo_t>::iterator it =sortedDiscMessages.begin(); it != sortedDiscMessages.end() ; it++ )
        {
          discMessageFound = true;
          // retrieve TB info of this packet
          int i = (*it).index;
          Ptr<cv2x_LteControlMessage> rxCtrlMsg= m_rxPacketInfo[i].m_rxControlMessage;
          Ptr<cv2x_SlDiscMessage> msg = DynamicCast<cv2x_SlDiscMessage> (rxCtrlMsg);
          cv2x_SlDiscMsg disc = msg->GetSlDiscMessage ();
          cv2x_DiscTbId_t tbId;
          tbId.m_rnti = disc.m_rnti;
          tbId.m_resPsdch = disc.m_resPsdch;
          
          itTbDisc =  m_expectedDiscTbs.find (tbId);
          NS_LOG_INFO (this << " Packet of " << tbId.m_rnti << " resPsdch " <<  (uint16_t) tbId.m_resPsdch); 

          bool rbDecoded = false;
          for (std::vector<int>::iterator rbIt =  m_rxPacketInfo[i].rbBitmap.begin ();  rbIt != m_rxPacketInfo[i].rbBitmap.end() ; rbIt++)
            {
              if (discDecodedBitmap.find (*rbIt) != discDecodedBitmap.end()) 
              {
                rbDecoded = true;
                NS_LOG_DEBUG (this << " RB " << *rbIt << " has already been decoded ");
                break;
              }
            }

          if (itTbDisc!=m_expectedDiscTbs.end () && !rbDecoded)
          {
            if (!(*itTbDisc).second.corrupt)
            {
              discError = false; 
              rxDiscMessageOkList.push_back (m_rxPacketInfo[i].m_rxControlMessage);
              discDecodedBitmap.insert ( m_rxPacketInfo[i].rbBitmap.begin(), m_rxPacketInfo[i].rbBitmap.end());
                        
              //to add theta and sinr to DiscTxProbabilityModule
              //assuming we have only one pool
              Ptr<cv2x_SlDiscMessage> msg = DynamicCast<cv2x_SlDiscMessage> (m_rxPacketInfo[i].m_rxControlMessage);
              cv2x_SlDiscMsg disc = msg->GetSlDiscMessage ();
              NS_LOG_DEBUG (this << " from RNTI " << disc.m_rnti << " ProSeAppCode " << disc.m_proSeAppCode.to_ulong () << " SINR " << (*it).sinr);
            }

            //store HARQ information
            if (!(*itTbDisc).second.harqFeedbackSent)
            {
              (*itTbDisc).second.harqFeedbackSent = true;
              //because we do not have feedbacks we do not reset HARQ now.
              //we will do it when we expect a new data
              if ((*itTbDisc).second.corrupt)
              {
                if (!m_nistErrorModelEnabled)
                {
                  NS_LOG_ERROR ("Any error model other than the NistErrorModel is not supported");
                }
                else
                {
                  m_harqPhyModule->UpdateDiscHarqProcessStatus (tbId.m_rnti, tbId.m_resPsdch, (*itTbDisc).second.sinr);
                }
              }
            }
          }     
        }

      if (discMessageFound)
      {        
        if (!discError)
        {
          if (!m_ltePhyRxCtrlEndOkCallback.IsNull ())
          {
            NS_LOG_DEBUG (this << " Discovery OK");
            m_ltePhyRxCtrlEndOkCallback (rxDiscMessageOkList);
          }
        }
        else
        {
          if (!m_ltePhyRxCtrlEndErrorCallback.IsNull ())
          {
            NS_LOG_DEBUG (this << " Discovery Error");
            m_ltePhyRxCtrlEndErrorCallback ();
          }
        }
      }
    }
      
    //error model and harq not enabled for d2d discovery
    //discard colliding packets for discovery
    else
    {
      for (uint32_t i = 0 ; i < m_rxPacketInfo.size() ; i++)
        { 
          if (m_rxPacketInfo[i].m_rxControlMessage) //if control packet               
            {
              if (!m_rxPacketInfo[i].m_rxPacketBurst && m_rxPacketInfo[i].m_rxControlMessage->GetMessageType () == cv2x_LteControlMessage::SL_DISC_MSG)
              {
                Ptr<cv2x_LteControlMessage> rxCtrlMsg= m_rxPacketInfo[i].m_rxControlMessage;
                Ptr<cv2x_SlDiscMessage> msg = DynamicCast<cv2x_SlDiscMessage> (rxCtrlMsg);
                cv2x_SlDiscMsg disc = msg->GetSlDiscMessage ();
                bool exist = FilterRxApps (disc);
                if (exist)
                {
                  double meanSinr = GetMeanSinr (m_slSinrPerceived[i], m_rxPacketInfo[i].rbBitmap);
                  cv2x_SlCtrlPacketInfo_t pInfo;
                  pInfo.sinr = meanSinr;          
                  pInfo.index = i;
                  sortedControlMessages.insert (pInfo);
                }
              }
            }
        }
       
      NS_ASSERT (m_slSinrPerceived.size ()>0);
      uint32_t countRb=0;
      for (Values::iterator vit = m_slSinrPerceived[0].ValuesBegin (); vit != m_slSinrPerceived[0].ValuesEnd (); ++vit)
      {
        countRb++;
      }

      std::vector<uint32_t> rbsUsed (countRb, 0);
      for (uint32_t i = 0; i < m_rxPacketInfo.size (); i++) 
        {
          for (std::vector<int>::iterator rbIt =  m_rxPacketInfo[i].rbBitmap.begin ();  rbIt != m_rxPacketInfo[i].rbBitmap.end() ; rbIt++)
            {
              rbsUsed[*rbIt]++;
            }
        }

     
      for (std::set<cv2x_SlCtrlPacketInfo_t>::iterator it =sortedControlMessages.begin(); it != sortedControlMessages.end() ; it++ )
        {
          int i = (*it).index;

          bool ctrlError = false;
          ctrlMessageFound = true;
          
          if (m_ctrlErrorModelEnabled)
            {          
              bool rbDecoded = false;
              for (std::vector<int>::iterator rbIt =  m_rxPacketInfo[i].rbBitmap.begin ();  rbIt != m_rxPacketInfo[i].rbBitmap.end() ; rbIt++)
                {
                  if (rbDecodedBitmap.find (*rbIt) != rbDecodedBitmap.end()) 
                    {
                      rbDecoded = true;
                      NS_LOG_DEBUG (this << " RB " << *rbIt << " has already been decoded ");
                      break;
                    }
                }
              if (!rbDecoded)
                {
                  bool ok = true;
                  for (std::vector<int>::iterator rbIt =  m_rxPacketInfo[i].rbBitmap.begin ();  rbIt != m_rxPacketInfo[i].rbBitmap.end() ; rbIt++)
                    {
                      if (rbsUsed [*rbIt] > 1) 
                        {
                          ok = false;
                          break;
                        }
                    }
                  if (ok)
                    {
                      double  errorRate = cv2x_LtePhyErrorModel::GetPscchBler (m_fadingModel,cv2x_LtePhyErrorModel::SISO, GetMeanSinr (m_slSinrPerceived[i]*4 /* Average gain for SIMO based on [CatreuxMIMO] */, m_rxPacketInfo[i].rbBitmap)).tbler;
                      ctrlError = m_random->GetValue () > errorRate ? false : true;
                      NS_LOG_DEBUG (this << " Discovery Decodification, errorRate " << errorRate << " error " << ctrlError);
                    } 
                  else
                    {
                      ctrlError = true;
                    }
                }
              else {
                ctrlError = true;
              }
            }
          
          if (!ctrlError)
            {
              error = false; //at least one control packet is OK
              rxControlMessageOkList.push_back (m_rxPacketInfo[i].m_rxControlMessage);
              rbDecodedBitmap.insert ( m_rxPacketInfo[i].rbBitmap.begin(), m_rxPacketInfo[i].rbBitmap.end());

              Ptr<cv2x_SlDiscMessage> msg = DynamicCast<cv2x_SlDiscMessage> (m_rxPacketInfo[i].m_rxControlMessage);
              cv2x_SlDiscMsg disc = msg->GetSlDiscMessage ();
              NS_LOG_DEBUG (this << " from RNTI " << disc.m_rnti << " ProSeAppCode " << disc.m_proSeAppCode.to_ulong () << " SINR " << (*it).sinr);
            } 
        }

        if (ctrlMessageFound)
        {
          if (!error)
            {
              if (!m_ltePhyRxCtrlEndOkCallback.IsNull ())
                {
                  NS_LOG_DEBUG (this << "Discovery OK");
                  m_ltePhyRxCtrlEndOkCallback (rxControlMessageOkList);
                }
            }
          else
            {
              if (!m_ltePhyRxCtrlEndErrorCallback.IsNull ())
                {
                  NS_LOG_DEBUG (this << "Discovery Error");
                  m_ltePhyRxCtrlEndErrorCallback ();
                }
            }
        }    
    }

  //done with sidelink data, control and discovery
  ChangeState (IDLE);
  m_rxPacketBurstList.clear ();
  m_rxControlMessageList.clear ();
  //m_rxControlMessageRbMap.clear ();
  m_rxPacketInfo.clear ();
  m_expectedSlTbs.clear ();
  m_expectedSlV2xTbs.clear ();
  m_expectedDiscTbs.clear ();
}

void
cv2x_LteSpectrumPhy::EndRxDlCtrl ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_LOGIC (this << " state: " << m_state);
  
  NS_ASSERT (m_state == RX_DL_CTRL);
  
  // this will trigger CQI calculation and Error Model evaluation
  // as a side effect, the error model should update the error status of all TBs
  m_interferenceCtrl->EndRx ();
  // apply transmission mode gain
  NS_LOG_DEBUG (this << " txMode " << (uint16_t)m_transmissionMode << " gain " << m_txModeGain.at (m_transmissionMode));
  NS_ASSERT (m_transmissionMode < m_txModeGain.size ());
  if (m_transmissionMode>0)
    {
      // in case of MIMO, ctrl is always txed as TX diversity
      m_sinrPerceived *= m_txModeGain.at (1);
    }
//   m_sinrPerceived *= m_txModeGain.at (m_transmissionMode);
  bool error = false;
  if (m_ctrlErrorModelEnabled)
    {
      double  errorRate = cv2x_LteMiErrorModel::GetPcfichPdcchError (m_sinrPerceived);
      error = m_random->GetValue () > errorRate ? false : true;
      NS_LOG_DEBUG (this << " PCFICH-PDCCH Decodification, errorRate " << errorRate << " error " << error);
    }

  if (!error)
    {
      if (!m_ltePhyRxCtrlEndOkCallback.IsNull ())
        {
          NS_LOG_DEBUG (this << " PCFICH-PDCCH Rxed OK");
          m_ltePhyRxCtrlEndOkCallback (m_rxControlMessageList);
        }
    }
  else
    {
      if (!m_ltePhyRxCtrlEndErrorCallback.IsNull ())
        {
          NS_LOG_DEBUG (this << " PCFICH-PDCCH Error");
          m_ltePhyRxCtrlEndErrorCallback ();
        }
    }
  ChangeState (IDLE);
  m_rxControlMessageList.clear ();
}

void
cv2x_LteSpectrumPhy::EndRxUlSrs ()
{
  NS_ASSERT (m_state == RX_UL_SRS);
  ChangeState (IDLE);
  m_interferenceCtrl->EndRx ();
  // nothing to do (used only for SRS at this stage)
}

void 
cv2x_LteSpectrumPhy::SetCellId (uint16_t cellId)
{
  m_cellId = cellId;
}

void 
cv2x_LteSpectrumPhy::AddL1GroupId (uint8_t groupId)
{
  NS_LOG_FUNCTION (this << (uint16_t) groupId);
  m_l1GroupIds.insert(groupId);
}

void 
cv2x_LteSpectrumPhy::RemoveL1GroupId (uint8_t groupId)
{
  m_l1GroupIds.erase (groupId);
}

void
cv2x_LteSpectrumPhy::SetComponentCarrierId (uint8_t componentCarrierId)
{
  m_componentCarrierId = componentCarrierId;
}

void
cv2x_LteSpectrumPhy::AddRsPowerChunkProcessor (Ptr<cv2x_LteChunkProcessor> p)
{
  m_interferenceCtrl->AddRsPowerChunkProcessor (p);
}

void
cv2x_LteSpectrumPhy::AddDataPowerChunkProcessor (Ptr<cv2x_LteChunkProcessor> p)
{
  m_interferenceData->AddRsPowerChunkProcessor (p);
}

void
cv2x_LteSpectrumPhy::AddDataSinrChunkProcessor (Ptr<cv2x_LteChunkProcessor> p)
{
  m_interferenceData->AddSinrChunkProcessor (p);
}

void
cv2x_LteSpectrumPhy::AddInterferenceCtrlChunkProcessor (Ptr<cv2x_LteChunkProcessor> p)
{
  m_interferenceCtrl->AddInterferenceChunkProcessor (p);
}

void
cv2x_LteSpectrumPhy::AddInterferenceDataChunkProcessor (Ptr<cv2x_LteChunkProcessor> p)
{
  m_interferenceData->AddInterferenceChunkProcessor (p);
}

void
cv2x_LteSpectrumPhy::AddCtrlSinrChunkProcessor (Ptr<cv2x_LteChunkProcessor> p)
{
  m_interferenceCtrl->AddSinrChunkProcessor (p);
}

void
cv2x_LteSpectrumPhy::AddSlSinrChunkProcessor (Ptr<cv2x_LteSlChunkProcessor> p)
{
  m_interferenceSl->AddSinrChunkProcessor (p);
}

void
cv2x_LteSpectrumPhy::AddSlSignalChunkProcessor (Ptr<cv2x_LteSlChunkProcessor> p)
{
  m_interferenceSl->AddRsPowerChunkProcessor (p);
}

void
cv2x_LteSpectrumPhy::AddSlInterferenceChunkProcessor (Ptr<cv2x_LteSlChunkProcessor> p)
{
  m_interferenceSl->AddInterferenceChunkProcessor (p);
}

void 
cv2x_LteSpectrumPhy::SetTransmissionMode (uint8_t txMode)
{
  NS_LOG_FUNCTION (this << (uint16_t) txMode);
  NS_ASSERT_MSG (txMode < m_txModeGain.size (), "TransmissionMode not available: 1.." << m_txModeGain.size ());
  m_transmissionMode = txMode;
  m_layersNum = cv2x_TransmissionModesLayers::TxMode2LayerNum (txMode);
}


void 
cv2x_LteSpectrumPhy::SetTxModeGain (uint8_t txMode, double gain)
{
  NS_LOG_FUNCTION (this << " txmode " << (uint16_t)txMode << " gain " << gain);
  // convert to linear
  gain = std::pow (10.0, (gain / 10.0));
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
      m_txModeGain.push_back (gain);
    }
    else
    {
      m_txModeGain.push_back (temp.at (i));
    }
  }
}

int64_t
cv2x_LteSpectrumPhy::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_random->SetStream (stream);
  return 1;
}

double 
cv2x_LteSpectrumPhy::GetMeanSinr (const SpectrumValue& sinr, const std::vector<int>& map)
{
  SpectrumValue sinrCopy = sinr;
  double sinrLin = 0;
  for (uint32_t i = 0; i < map.size (); i++)
    {
      sinrLin += sinrCopy[map.at (i)];
    }
  return sinrLin / map.size();
}

cv2x_LteSpectrumPhy::State
cv2x_LteSpectrumPhy::GetState ()
{
  return m_state;
}

void
cv2x_LteSpectrumPhy::SetSlssid (uint64_t slssid)
{
  NS_LOG_FUNCTION (this);
  m_slssId = slssid;
}

void
cv2x_LteSpectrumPhy::SetLtePhyRxSlssCallback (cv2x_LtePhyRxSlssCallback c)
{
  NS_LOG_FUNCTION (this);
  m_ltePhyRxSlssCallback = c;
}

void 
cv2x_LteSpectrumPhy::SetRxPool (Ptr<SidelinkDiscResourcePool> newpool)
{
  m_discRxPools.push_back (newpool);
}

void 
cv2x_LteSpectrumPhy::SetRxPool (Ptr<SidelinkCommResourcePoolV2x> newpool)
{
  m_slV2xRxPools.push_back (newpool);
}

void 
cv2x_LteSpectrumPhy::AddDiscTxApps (std::list<uint32_t> apps)
{
    m_discTxApps = apps;
}

void 
cv2x_LteSpectrumPhy::AddDiscRxApps (std::list<uint32_t> apps)
{
  m_discRxApps = apps;
}

bool 
cv2x_LteSpectrumPhy::FilterRxApps (cv2x_SlDiscMsg disc)
{
  NS_LOG_FUNCTION (this << disc.m_proSeAppCode);
  bool exist = false;
  for (std::list<uint32_t>::iterator it = m_discRxApps.begin (); it != m_discRxApps.end (); ++it)
  {
    //std::cout << "app=" << *it  << std::endl;
    if ((std::bitset <184>)*it == disc.m_proSeAppCode)
    {
      exist = true;
    }
  }

  return exist;
}

void
cv2x_LteSpectrumPhy::SetDiscNumRetx (uint8_t retx)
{
  NS_LOG_FUNCTION (this << retx);
  m_harqPhyModule->SetDiscNumRetx (retx);
}


} // namespace ns3

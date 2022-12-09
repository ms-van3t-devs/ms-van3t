/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015 NYU WIRELESS, Tandon School of Engineering, New York University
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

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      std::clog << " [ CellId " << GetCellId() << ", bwpId "             \
                << GetBwpId () << "] ";                                  \
    }                                                                    \
  while (false);

#include "nr-ue-phy.h"
#include <ns3/log.h>
#include <ns3/simulator.h>
#include <ns3/node.h>
#include <ns3/double.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/nr-sl-comm-resource-pool.h>
#include <algorithm>
#include <cfloat>
#include <ns3/boolean.h>
#include <ns3/pointer.h>
#include "beam-manager.h"
#include "nr-ue-net-device.h"
#include "nr-ch-access-manager.h"
#include "nr-ue-power-control.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrUePhy");
NS_OBJECT_ENSURE_REGISTERED (NrUePhy);

NrUePhy::NrUePhy ()
{
  NS_LOG_FUNCTION (this);
  m_wbCqiLast = Simulator::Now ();
  m_ueCphySapProvider = new MemberLteUeCphySapProvider<NrUePhy> (this);
  m_powerControl = CreateObject <NrUePowerControl> (this);
  m_nrSlUeCphySapProvider = new MemberNrSlUeCphySapProvider<NrUePhy> (this);
  DoReset ();
}

NrUePhy::~NrUePhy ()
{
  NS_LOG_FUNCTION (this);
}

void
NrUePhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ueCphySapProvider;
  NrPhy::DoDispose ();
  delete m_nrSlUeCphySapProvider;
  m_slTxPool = nullptr;
  m_slRxPool = nullptr;
}

TypeId
NrUePhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrUePhy")
    .SetParent<NrPhy> ()
    .AddConstructor<NrUePhy> ()
    .AddAttribute ("TxPower",
                   "Transmission power in dBm",
                   DoubleValue (2.0),
                   MakeDoubleAccessor (&NrUePhy::m_txPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("NoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                   " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to the noise output of an "
                   " ideal receiver with the same overall gain and bandwidth when the receivers "
                   " are connected to sources at the standard noise temperature T0.\" "
                  "In this model, we consider T0 = 290K.",
                   DoubleValue (5.0), // nr code from NYU and UniPd assumed in the code the value of 5dB, thats why we configure the default value to that
                   MakeDoubleAccessor (&NrUePhy::m_noiseFigure),
                   MakeDoubleChecker<double> ())
     .AddAttribute ("PowerAllocationType",
                    "Defines the type of the power allocation. Currently are supported "
                    "two types: \"UniformPowerAllocBw\", which is a uniform power allocation over all "
                    "bandwidth (over all RBs), and \"UniformPowerAllocBw\", which is a uniform "
                    "power allocation over used (active) RBs. By default is set a uniform power "
                    "allocation over used RBs .",
                    EnumValue (NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_USED),
                    MakeEnumAccessor (&NrPhy::SetPowerAllocationType,
                                      &NrPhy::GetPowerAllocationType),
                    MakeEnumChecker ( NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_BW, "UniformPowerAllocBw",
                                      NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_USED, "UniformPowerAllocUsed"
                                    ))
    .AddAttribute ("SpectrumPhy",
                   "The SpectrumPhy associated to this NrPhy",
                   TypeId::ATTR_GET,
                   PointerValue (),
                   MakePointerAccessor (&NrPhy::GetSpectrumPhy),
                   MakePointerChecker <NrSpectrumPhy> ())
    .AddAttribute ("LBTThresholdForCtrl",
                   "After a DL/UL transmission, if we have less than this value to send the UL CTRL, we consider the channel as granted",
                   TimeValue (MicroSeconds (25)),
                   MakeTimeAccessor (&NrUePhy::m_lbtThresholdForCtrl),
                   MakeTimeChecker ())
    .AddAttribute ("TbDecodeLatency",
                   "Transport block decode latency",
                   TimeValue (MicroSeconds (100)),
                   MakeTimeAccessor (&NrPhy::SetTbDecodeLatency,
                   &NrPhy::GetTbDecodeLatency),
                   MakeTimeChecker ())
    .AddAttribute ("EnableUplinkPowerControl",
                   "If true, Uplink Power Control will be enabled.",
                    BooleanValue (false),
                    MakeBooleanAccessor (&NrUePhy::SetEnableUplinkPowerControl),
                    MakeBooleanChecker ())
    .AddTraceSource ("ReportCurrentCellRsrpSinr",
                     "RSRP and SINR statistics.",
                     MakeTraceSourceAccessor (&NrUePhy::m_reportCurrentCellRsrpSinrTrace),
                     "ns3::CurrentCellRsrpSinr::TracedCallback")
    .AddTraceSource ("ReportUplinkTbSize",
                     "Report allocated uplink TB size for trace.",
                     MakeTraceSourceAccessor (&NrUePhy::m_reportUlTbSize),
                     "ns3::UlTbSize::TracedCallback")
    .AddTraceSource ("ReportDownlinkTbSize",
                     "Report allocated downlink TB size for trace.",
                     MakeTraceSourceAccessor (&NrUePhy::m_reportDlTbSize),
                     "ns3::DlTbSize::TracedCallback")
    .AddTraceSource ("UePhyRxedCtrlMsgsTrace",
                     "Ue PHY Control Messages Traces.",
                     MakeTraceSourceAccessor (&NrUePhy::m_phyRxedCtrlMsgsTrace),
                     "ns3::NrPhyRxTrace::RxedUePhyCtrlMsgsTracedCallback")
    .AddTraceSource ("UePhyTxedCtrlMsgsTrace",
                     "Ue PHY Control Messages Traces.",
                     MakeTraceSourceAccessor (&NrUePhy::m_phyTxedCtrlMsgsTrace),
                     "ns3::NrPhyRxTrace::TxedUePhyCtrlMsgsTracedCallback")
    .AddTraceSource ("UePhyRxedDlDciTrace",
                     "Ue PHY DL DCI Traces.",
                     MakeTraceSourceAccessor (&NrUePhy::m_phyUeRxedDlDciTrace),
                     "ns3::NrPhyRxTrace::RxedUePhyDlDciTracedCallback")
    .AddTraceSource ("UePhyTxedHarqFeedbackTrace",
                     "Ue PHY DL HARQ Feedback Traces.",
                     MakeTraceSourceAccessor (&NrUePhy::m_phyUeTxedHarqFeedbackTrace),
                     "ns3::NrPhyRxTrace::TxedUePhyHarqFeedbackTracedCallback")
    .AddTraceSource ("ReportPowerSpectralDensity",
                     "Power Spectral Density data.",
                     MakeTraceSourceAccessor (&NrUePhy::m_reportPowerSpectralDensity),
                     "ns3::NrUePhy::PowerSpectralDensityTracedCallback")
      ;
  return tid;
}

void
NrUePhy::ChannelAccessGranted (const Time &time)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (time);
  // That will be granted only till the end of the slot
  m_channelStatus = GRANTED;
}

void
NrUePhy::ChannelAccessDenied ()
{
  NS_LOG_FUNCTION (this);
  m_channelStatus = NONE;
}

void
NrUePhy::SetUeCphySapUser (LteUeCphySapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_ueCphySapUser = s;
}

LteUeCphySapProvider*
NrUePhy::GetUeCphySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return (m_ueCphySapProvider);
}

void
NrUePhy::SetEnableUplinkPowerControl (bool enable)
{
  m_enableUplinkPowerControl = enable;
}

void
NrUePhy::SetTxPower (double pow)
{
  m_txPower = pow;
  m_powerControl->SetTxPower (pow);
}
double
NrUePhy::GetTxPower () const
{
  return m_txPower;
}

double
NrUePhy::GetRsrp () const
{
  return m_rsrp;
}

Ptr<NrUePowerControl>
NrUePhy::GetUplinkPowerControl () const
{
  NS_LOG_FUNCTION (this);
  return m_powerControl;
}

void
NrUePhy::SetUplinkPowerControl (Ptr<NrUePowerControl> pc)
{
  m_powerControl = pc;
}

void
NrUePhy::SetDlAmc(const Ptr<const NrAmc> &amc)
{
  m_amc = amc;
}

void
NrUePhy::SetSubChannelsForTransmission (const std::vector <int> &mask, uint32_t numSym)
{
  Ptr<SpectrumValue> txPsd = GetTxPowerSpectralDensity (mask);
  NS_ASSERT (txPsd);

  m_reportPowerSpectralDensity (m_currentSlot, txPsd, numSym * GetSymbolPeriod (), m_rnti, m_imsi, GetBwpId (), GetCellId ());
  m_spectrumPhy->SetTxPowerSpectralDensity (txPsd);
}

void
NrUePhy::DoSendControlMessage (Ptr<NrControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  EnqueueCtrlMessage (msg);
}

void
NrUePhy::DoSendControlMessageNow (Ptr<NrControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  EnqueueCtrlMsgNow (msg);
}

void
NrUePhy::ProcessDataDci (const SfnSf &ulSfnSf,
                         const std::shared_ptr<DciInfoElementTdma> &dciInfoElem)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("UE" << m_rnti <<
                " UL-DCI received for slot " << ulSfnSf <<
                " symStart " << static_cast<uint32_t> (dciInfoElem->m_symStart) <<
                " numSym " << static_cast<uint32_t> (dciInfoElem->m_numSym) <<
                " tbs " << dciInfoElem->m_tbSize <<
                " harqId " << static_cast<uint32_t> (dciInfoElem->m_harqProcess));

  if (ulSfnSf == m_currentSlot)
    {
      InsertAllocation (dciInfoElem);
    }
  else
    {
      InsertFutureAllocation (ulSfnSf, dciInfoElem);
    }
}

void
NrUePhy::ProcessSrsDci (const SfnSf &ulSfnSf, const std::shared_ptr<DciInfoElementTdma> &dciInfoElem)
{
  NS_LOG_FUNCTION (this);
  NS_UNUSED (ulSfnSf);
  NS_UNUSED (dciInfoElem);
  // Instruct PHY for transmitting the SRS
  if (ulSfnSf == m_currentSlot)
    {
      InsertAllocation (dciInfoElem);
    }
  else
    {
      InsertFutureAllocation (ulSfnSf, dciInfoElem);
    }
}

void
NrUePhy::RegisterToEnb (uint16_t bwpId)
{
  NS_LOG_FUNCTION (this);

  InitializeMessageList ();
  DoSetCellId (bwpId);
}

void
NrUePhy::SetUlCtrlSyms(uint8_t ulCtrlSyms)
{
  m_ulCtrlSyms = ulCtrlSyms;
}

void
NrUePhy::SetDlCtrlSyms(uint8_t dlCtrlSyms)
{
  m_dlCtrlSyms = dlCtrlSyms;
}

void
NrUePhy::SetNumRbPerRbg (uint32_t numRbPerRbg)
{
  m_numRbPerRbg = numRbPerRbg;
}

void
NrUePhy::SetPattern (const std::string &pattern)
{
  NS_LOG_FUNCTION (this);

  static std::unordered_map<std::string, LteNrTddSlotType> lookupTable =
  {
    { "DL", LteNrTddSlotType::DL },
    { "UL", LteNrTddSlotType::UL },
    { "S",  LteNrTddSlotType::S },
    { "F",  LteNrTddSlotType::F },
  };

  std::vector<LteNrTddSlotType> vector;
  std::stringstream ss (pattern);
  std::string token;
  std::vector<std::string> extracted;

   while (std::getline(ss, token, '|'))
     {
       extracted.push_back(token);
     }

   for (const auto & v : extracted)
     {
       vector.push_back (lookupTable[v]);
     }

   m_tddPattern = vector;
}

uint32_t
NrUePhy::GetNumRbPerRbg () const
{
  return m_numRbPerRbg;
}


double
NrUePhy::ComputeAvgSinr (const SpectrumValue &sinr)
{
  // averaged SINR among RBs
  double sum = 0.0;
  uint8_t rbNum = 0;
  Values::const_iterator it;

  for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
    {
      sum += (*it);
      rbNum++;
    }

  double avrgSinr = (rbNum > 0) ? (sum / rbNum) : DBL_MAX;

  return avrgSinr;
}

void
NrUePhy::InsertAllocation (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  VarTtiAllocInfo varTtiInfo (dci);
  m_currSlotAllocInfo.m_varTtiAllocInfo.push_back (varTtiInfo);
  std::sort (m_currSlotAllocInfo.m_varTtiAllocInfo.begin (), m_currSlotAllocInfo.m_varTtiAllocInfo.end ());
}

void
NrUePhy::InsertFutureAllocation (const SfnSf &sfnSf,
                                     const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  VarTtiAllocInfo varTtiInfo (dci);
  if (SlotAllocInfoExists (sfnSf))
    {
      auto & ulSlot = PeekSlotAllocInfo (sfnSf);
      ulSlot.m_varTtiAllocInfo.push_back (varTtiInfo);
      std::sort (ulSlot.m_varTtiAllocInfo.begin (), ulSlot.m_varTtiAllocInfo.end ());
    }
  else
    {
      SlotAllocInfo slotAllocInfo = SlotAllocInfo (sfnSf);
      slotAllocInfo.m_varTtiAllocInfo.push_back (varTtiInfo);
      PushBackSlotAllocInfo (slotAllocInfo);
    }
}

void
NrUePhy::PhyCtrlMessagesReceived (const Ptr<NrControlMessage> &msg)
{
  NS_LOG_FUNCTION (this);

  if (msg->GetMessageType () == NrControlMessage::DL_DCI)
    {
      auto dciMsg = DynamicCast<NrDlDciMessage> (msg);
      auto dciInfoElem = dciMsg->GetDciInfoElement ();

      m_phyRxedCtrlMsgsTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), msg);

      if (dciInfoElem->m_rnti != 0 && dciInfoElem->m_rnti != m_rnti)
        {
          return;   // DCI not for me
        }

      SfnSf dciSfn = m_currentSlot;
      uint32_t k0Delay = dciMsg->GetKDelay ();
      dciSfn.Add (k0Delay);

      NS_LOG_DEBUG ("UE" << m_rnti << " DL-DCI received for slot " << dciSfn <<
                    " symStart " << static_cast<uint32_t> (dciInfoElem->m_symStart) <<
                    " numSym " << static_cast<uint32_t> (dciInfoElem->m_numSym) <<
                    " tbs " << dciInfoElem->m_tbSize <<
                    " harqId " << static_cast<uint32_t> (dciInfoElem->m_harqProcess));

      /* BIG ASSUMPTION: We assume that K0 is always 0 */

      auto it = m_harqIdToK1Map.find (dciInfoElem->m_harqProcess);
      if (it!=m_harqIdToK1Map.end ())
        {
          m_harqIdToK1Map.erase (m_harqIdToK1Map.find (dciInfoElem->m_harqProcess));
        }

      m_harqIdToK1Map.insert (std::make_pair (dciInfoElem->m_harqProcess, dciMsg->GetK1Delay ()));

      m_phyUeRxedDlDciTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), dciInfoElem->m_harqProcess, dciMsg->GetK1Delay ());

      InsertAllocation (dciInfoElem);

      m_phySapUser->ReceiveControlMessage (msg);

      if (m_enableUplinkPowerControl)
        {
          m_powerControl->ReportTpcPusch (dciInfoElem->m_tpc);
          m_powerControl->ReportTpcPucch (dciInfoElem->m_tpc);
        }
    }
  else if (msg->GetMessageType () == NrControlMessage::UL_DCI)
    {
      auto dciMsg = DynamicCast<NrUlDciMessage> (msg);
      auto dciInfoElem = dciMsg->GetDciInfoElement ();

      m_phyRxedCtrlMsgsTrace (m_currentSlot,  GetCellId (), m_rnti, GetBwpId (), msg);

      if (dciInfoElem->m_rnti != 0 && dciInfoElem->m_rnti != m_rnti)
        {
          return;   // DCI not for me
        }

      SfnSf ulSfnSf = m_currentSlot;
      uint32_t k2Delay = dciMsg->GetKDelay ();
      ulSfnSf.Add (k2Delay);

      if (dciInfoElem->m_type == DciInfoElementTdma::DATA)
        {
          ProcessDataDci (ulSfnSf, dciInfoElem);
          m_phySapUser->ReceiveControlMessage (msg);
        }
      else if (dciInfoElem->m_type == DciInfoElementTdma::SRS)
        {
          ProcessSrsDci (ulSfnSf, dciInfoElem);
          // Do not pass the DCI to MAC
        }
    }
  else if (msg->GetMessageType () == NrControlMessage::MIB)
    {
      NS_LOG_INFO ("received MIB");
      Ptr<NrMibMessage> msg2 = DynamicCast<NrMibMessage> (msg);
      m_phyRxedCtrlMsgsTrace (m_currentSlot,  GetCellId (), m_rnti, GetBwpId (), msg);
      m_ueCphySapUser->RecvMasterInformationBlock (GetCellId (), msg2->GetMib ());
    }
  else if (msg->GetMessageType () == NrControlMessage::SIB1)
    {
      Ptr<NrSib1Message> msg2 = DynamicCast<NrSib1Message> (msg);
      m_phyRxedCtrlMsgsTrace (m_currentSlot,  GetCellId (), m_rnti, GetBwpId (), msg);
      m_ueCphySapUser->RecvSystemInformationBlockType1 (GetCellId (), msg2->GetSib1 ());
    }
  else if (msg->GetMessageType () == NrControlMessage::RAR)
    {
      Ptr<NrRarMessage> rarMsg = DynamicCast<NrRarMessage> (msg);

      Simulator::Schedule ((GetSlotPeriod () * (GetL1L2CtrlLatency ()/2)), &NrUePhy::DoReceiveRar, this, rarMsg);
    }
  else
    {
      NS_LOG_INFO ("Message type not recognized " << msg->GetMessageType ());
      m_phyRxedCtrlMsgsTrace (m_currentSlot,  GetCellId (), m_rnti, GetBwpId (), msg);
      m_phySapUser->ReceiveControlMessage (msg);
    }
}

void
NrUePhy::TryToPerformLbt ()
{
  NS_LOG_FUNCTION (this);
  uint8_t ulCtrlSymStart = 0;
  uint8_t ulCtrlNumSym = 0;

  for (const auto & alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
      if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL && alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
          ulCtrlSymStart = alloc.m_dci->m_symStart;
          ulCtrlNumSym = alloc.m_dci->m_numSym;
          break;
        }
    }

  if (ulCtrlNumSym != 0)
    {
      // We have an UL CTRL symbol scheduled and we have to transmit CTRLs..
      // .. so we check that we have at least 25 us between the latest DCI,
      // or we have to schedule an LBT event.

      Time limit = m_lastSlotStart + GetSlotPeriod () -
          ((GetSymbolsPerSlot () - ulCtrlSymStart) * GetSymbolPeriod ()) -
          m_lbtThresholdForCtrl;

      for (const auto & alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
        {
          int64_t symbolPeriod = GetSymbolPeriod ().GetMicroSeconds ();
          int64_t dciEndsAt = m_lastSlotStart.GetMicroSeconds () +
              ((alloc.m_dci->m_numSym + alloc.m_dci->m_symStart) * symbolPeriod);

          if (alloc.m_dci->m_type != DciInfoElementTdma::DATA)
            {
              continue;
            }

          if (limit.GetMicroSeconds () < dciEndsAt)
            {
              NS_LOG_INFO ("This data DCI ends at " << MicroSeconds (dciEndsAt) <<
                           " which is inside the LBT shared COT (the limit is " <<
                           limit << "). No need for LBT");
              m_lbtEvent.Cancel (); // Forget any LBT we previously set, because of the new
                                    // DCI information
              m_channelStatus = GRANTED;
            }
          else
            {
              NS_LOG_INFO ("This data DCI starts at " << +alloc.m_dci->m_symStart << " for " <<
                           +alloc.m_dci->m_numSym << " ends at " << MicroSeconds (dciEndsAt) <<
                           " which is outside the LBT shared COT (the limit is " <<
                           limit << ").");
            }
        }
      if (m_channelStatus != GRANTED)
        {
          Time sched = m_lastSlotStart - Simulator::Now () +
              (GetSymbolPeriod () * ulCtrlSymStart) - MicroSeconds (25);
          NS_LOG_INFO ("Scheduling an LBT for sending the UL CTRL at " <<
                       Simulator::Now () + sched);
          m_lbtEvent.Cancel ();
          m_lbtEvent = Simulator::Schedule (sched, &NrUePhy::RequestAccess, this);
        }
      else
        {
          NS_LOG_INFO ("Not scheduling LBT: the UE has a channel status that is GRANTED");
        }
    }
  else
    {
      NS_LOG_INFO ("Not scheduling LBT; the UE has no UL CTRL symbols available");
    }
}

void
NrUePhy::RequestAccess ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Request access at " << Simulator::Now () << " because we have to transmit UL CTRL");
  m_cam->RequestAccess (); // This will put the m_channelStatus to granted when
                           // the channel will be granted.
}

void
NrUePhy::DoReceiveRar (Ptr<NrRarMessage> rarMsg)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Received RAR in slot " << m_currentSlot);
  m_phyRxedCtrlMsgsTrace (m_currentSlot,  GetCellId (), m_rnti, GetBwpId (), rarMsg);

  for (auto it = rarMsg->RarListBegin (); it != rarMsg->RarListEnd (); ++it)
    {
      if (it->rapId == m_raPreambleId)
        {
          m_phySapUser->ReceiveControlMessage (rarMsg);
        }
    }
}

void
NrUePhy::PushCtrlAllocations (const SfnSf currentSfnSf)
{
  NS_LOG_FUNCTION (this);

  // The UE does not know anything from the GNB yet, so listen on the default
  // bandwidth.
  std::vector<uint8_t> rbgBitmask (GetRbNum (), 1);

  // The UE still doesn't know the TDD pattern, so just add a DL CTRL
  if (m_tddPattern.size () == 0)
    {
      NS_LOG_INFO ("TDD Pattern unknown, insert DL CTRL at the beginning of the slot");
      VarTtiAllocInfo dlCtrlSlot (std::make_shared<DciInfoElementTdma> (0, m_dlCtrlSyms,
                                                                        DciInfoElementTdma::DL,
                                                                        DciInfoElementTdma::CTRL, rbgBitmask));
      m_currSlotAllocInfo.m_varTtiAllocInfo.push_front (dlCtrlSlot);
      return;
    }

  uint64_t currentSlotN = currentSfnSf.Normalize () % m_tddPattern.size ();

  if (m_tddPattern[currentSlotN] < LteNrTddSlotType::UL)
    {
      NS_LOG_INFO ("The current TDD pattern indicates that we are in a " <<
                   m_tddPattern[currentSlotN] <<
                   " slot, so insert DL CTRL at the beginning of the slot");
      VarTtiAllocInfo dlCtrlSlot (std::make_shared<DciInfoElementTdma> (0, m_dlCtrlSyms,
                                                                        DciInfoElementTdma::DL,
                                                                        DciInfoElementTdma::CTRL, rbgBitmask));
      m_currSlotAllocInfo.m_varTtiAllocInfo.push_front (dlCtrlSlot);
    }
  if (m_tddPattern[currentSlotN] > LteNrTddSlotType::DL)
    {
      NS_LOG_INFO ("The current TDD pattern indicates that we are in a " <<
                   m_tddPattern[currentSlotN] <<
                   " slot, so insert UL CTRL at the end of the slot");
      VarTtiAllocInfo ulCtrlSlot (std::make_shared<DciInfoElementTdma> (GetSymbolsPerSlot () - m_ulCtrlSyms,
                                                                        m_ulCtrlSyms,
                                                                        DciInfoElementTdma::UL,
                                                                        DciInfoElementTdma::CTRL, rbgBitmask));
      m_currSlotAllocInfo.m_varTtiAllocInfo.push_back (ulCtrlSlot);
    }
}

void
NrUePhy::StartSlot (const SfnSf &s)
{
  NS_LOG_FUNCTION (this);
  m_currentSlot = s;
  m_lastSlotStart = Simulator::Now ();

  // Call MAC before doing anything in PHY
  m_phySapUser->SlotIndication (m_currentSlot);   // trigger mac

  // update the current slot object, and insert DL/UL CTRL allocations depending on the TDD pattern
  bool nrAllocExists = SlotAllocInfoExists (m_currentSlot);
  bool slAllocExists = NrSlSlotAllocInfoExists (m_currentSlot);

  /*
   * Clear SL expected TB not received in previous slot.
   * It may happen that a UE is expecting to receive a TB in a slot, however,
   * in the same slot it decided to transmit. In this case, due to the half-duplex
   * nature of the Sidelink it will not receive that TB. Thus, the information
   * inserted in the m_slTransportBlocks buffer will be out-dated in the next slot,
   * hence, must be removed at the beginning of the next slot. This is also due
   * to the fact that in current implementation we always prioritize transmission
   * over reception without looking at the priority of the two TBs, i.e, the one
   * which needs to be transmitted and the one which need to be received.
   * As per the 3GPP standard, a device might prioritize RX over TX as per
   * the priority or vice versa.
   */
  m_spectrumPhy->ClearExpectedSlTb ();

  SendSlExpectedTbInfo (s);

  if (slAllocExists)
    {
      NS_ASSERT_MSG (!nrAllocExists, "Can not start SL slot when there is UL allocation");
      StartNrSlSlot (s);
      return;
    }

  if (nrAllocExists)
    {
      m_currSlotAllocInfo = RetrieveSlotAllocInfo (m_currentSlot);
    }
  else
    {
      m_currSlotAllocInfo = SlotAllocInfo (m_currentSlot);
    }

  PushCtrlAllocations (m_currentSlot);

  NS_ASSERT (m_currSlotAllocInfo.m_sfnSf == m_currentSlot);

  NS_LOG_INFO ("UE " << m_rnti << " start slot " << m_currSlotAllocInfo.m_sfnSf <<
               " composed by the following allocations, total " << m_currSlotAllocInfo.m_varTtiAllocInfo.size ());
  for (const auto & alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
      std::string direction, type;
      if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL)
        {
          type = "CTRL";
        }
      else if (alloc.m_dci->m_type == DciInfoElementTdma::SRS)
        {
          type = "SRS";
        }
      else
        {
          type = "DATA";
        }

      if (alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
          direction = "UL";
        }
      else
        {
          direction = "DL";
        }
      NS_LOG_INFO ("Allocation from sym " << static_cast<uint32_t> (alloc.m_dci->m_symStart) <<
                   " to sym " << static_cast<uint32_t> (alloc.m_dci->m_numSym + alloc.m_dci->m_symStart) <<
                   " direction " << direction << " type " << type);
    }

  TryToPerformLbt ();

  VarTtiAllocInfo allocation = m_currSlotAllocInfo.m_varTtiAllocInfo.front ();
  m_currSlotAllocInfo.m_varTtiAllocInfo.pop_front ();

  auto nextVarTtiStart = GetSymbolPeriod () * allocation.m_dci->m_symStart;


  auto ctrlMsgs = PopCurrentSlotCtrlMsgs ();
  if (m_netDevice)
    {
      DynamicCast<NrUeNetDevice> (m_netDevice)->RouteOutgoingCtrlMsgs (ctrlMsgs, GetBwpId ());
    }
  else
    {
      // No netDevice (that could happen in tests) so just redirect them to us
      for (const auto & msg : ctrlMsgs)
        {
          EncodeCtrlMsg (msg);
        }

    }


  Simulator::Schedule (nextVarTtiStart, &NrUePhy::StartVarTti, this, allocation.m_dci);
}


Time
NrUePhy::DlCtrl(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  NS_LOG_DEBUG ("UE" << m_rnti <<
                " RXing DL CTRL frame for"
                " symbols "  << +dci->m_symStart <<
                "-" << +(dci->m_symStart + dci->m_numSym - 1) <<
                "\t start " << Simulator::Now () <<
                " end " << (Simulator::Now () + varTtiPeriod));

  m_tryToPerformLbt = true;

  return varTtiPeriod;
}


Time
NrUePhy::UlSrs (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  std::vector<int> channelRbs;
  for (uint32_t i = 0; i < GetRbNum (); i++)
    {
      channelRbs.push_back (static_cast<int> (i));
    }
  SetSubChannelsForTransmission (channelRbs, dci->m_numSym);

  std::list <Ptr<NrControlMessage>> srsMsg;
  Ptr<NrSrsMessage> srs = Create<NrSrsMessage> ();
  srs->SetSourceBwp (GetBwpId());
  srsMsg.push_back (srs);
  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  m_spectrumPhy->StartTxUlControlFrames (srsMsg, varTtiPeriod - NanoSeconds (1.0));

  NS_LOG_DEBUG ("UE" << m_rnti << " TXing UL SRS frame for symbols " <<
                  +dci->m_symStart << "-" <<
                  +(dci->m_symStart + dci->m_numSym - 1) <<
                  "\t start " << Simulator::Now () << " end " <<
                  (Simulator::Now () + varTtiPeriod - NanoSeconds (1.0)));

  ChannelAccessDenied (); // Reset the channel status
  return varTtiPeriod;
}

Time
NrUePhy::UlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  if (m_ctrlMsgs.size () == 0)
    {
      NS_LOG_INFO   ("UE" << m_rnti << " reserved space for UL CTRL frame for symbols " <<
                    +dci->m_symStart << "-" <<
                    +(dci->m_symStart + dci->m_numSym - 1) <<
                    "\t start " << Simulator::Now () << " end " <<
                    (Simulator::Now () + varTtiPeriod - NanoSeconds (1.0)) <<
                    " but no data to transmit");
      m_cam->Cancel ();
      return varTtiPeriod;
    }
  else if (m_channelStatus != GRANTED)
    {
      NS_LOG_INFO ("UE" << m_rnti << " has to transmit CTRL but channel not granted");
      m_cam->Cancel ();
      return varTtiPeriod;
    }

  for (const auto & msg : m_ctrlMsgs)
    {
      m_phyTxedCtrlMsgsTrace (m_currentSlot,  GetCellId (), dci->m_rnti, GetBwpId (), msg);

      if (msg->GetMessageType () == NrControlMessage::DL_HARQ)
        {
          Ptr<NrDlHarqFeedbackMessage> harqMsg = DynamicCast<NrDlHarqFeedbackMessage> (msg);
          uint8_t harqId = harqMsg->GetDlHarqFeedback ().m_harqProcessId;

          auto it = m_harqIdToK1Map.find (harqId);
          if (it!=m_harqIdToK1Map.end ())
            {
              m_phyUeTxedHarqFeedbackTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (),
                                            static_cast<uint32_t> (harqId), it->second);
            }
        }
    }

  std::vector<int> channelRbs;
  for (uint32_t i = 0; i < GetRbNum (); i++)
    {
      channelRbs.push_back (static_cast<int> (i));
    }

  if (m_enableUplinkPowerControl)
    {
      m_txPower = m_powerControl->GetPucchTxPower (channelRbs.size());
    }
  SetSubChannelsForTransmission (channelRbs, dci->m_numSym);

  NS_LOG_DEBUG ("UE" << m_rnti << " TXing UL CTRL frame for symbols " <<
                +dci->m_symStart << "-" <<
                +(dci->m_symStart + dci->m_numSym - 1) <<
                "\t start " << Simulator::Now () << " end " <<
                (Simulator::Now () + varTtiPeriod - NanoSeconds (1.0)));

  SendCtrlChannels (varTtiPeriod - NanoSeconds (1.0));

  ChannelAccessDenied (); // Reset the channel status
  return varTtiPeriod;
}

Time
NrUePhy::DlData (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  m_receptionEnabled = true;
  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  m_spectrumPhy->AddExpectedTb (dci->m_rnti, dci->m_ndi, dci->m_tbSize, dci->m_mcs,
                                        FromRBGBitmaskToRBAssignment (dci->m_rbgBitmask),
                                        dci->m_harqProcess, dci->m_rv, true,
                                        dci->m_symStart, dci->m_numSym, m_currentSlot);
  m_reportDlTbSize (m_netDevice->GetObject <NrUeNetDevice> ()->GetImsi (), dci->m_tbSize);
  NS_LOG_DEBUG ("UE" << m_rnti <<
                " RXing DL DATA frame for"
                " symbols "  << +dci->m_symStart <<
                "-" << +(dci->m_symStart + dci->m_numSym - 1) <<
                " num of rbg assigned: " << FromRBGBitmaskToRBAssignment (dci->m_rbgBitmask).size () <<
                "\t start " << Simulator::Now () <<
                " end " << (Simulator::Now () + varTtiPeriod));

  return varTtiPeriod;
}

Time
NrUePhy::UlData(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);
  if (m_enableUplinkPowerControl)
    {
      m_txPower = m_powerControl->GetPuschTxPower ((FromRBGBitmaskToRBAssignment(dci->m_rbgBitmask)).size());
    }
  SetSubChannelsForTransmission (FromRBGBitmaskToRBAssignment (dci->m_rbgBitmask), dci->m_numSym);
  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;
  std::list<Ptr<NrControlMessage> > ctrlMsg;
  Ptr<PacketBurst> pktBurst = GetPacketBurst (m_currentSlot, dci->m_symStart);
  if (pktBurst && pktBurst->GetNPackets () > 0)
    {
      std::list< Ptr<Packet> > pkts = pktBurst->GetPackets ();
      LteRadioBearerTag bearerTag;
      if (!pkts.front ()->PeekPacketTag (bearerTag))
        {
          NS_FATAL_ERROR ("No radio bearer tag");
        }
    }
  else
    {
      // put an error, as something is wrong. The UE should not be scheduled
      // if there is no data for him...
      NS_FATAL_ERROR ("The UE " << dci->m_rnti << " has been scheduled without data");
    }
  m_reportUlTbSize (m_netDevice->GetObject <NrUeNetDevice> ()->GetImsi (), dci->m_tbSize);

  NS_LOG_DEBUG ("UE" << m_rnti <<
                " TXing UL DATA frame for" <<
                " symbols "  << +dci->m_symStart <<
                "-" << +(dci->m_symStart + dci->m_numSym - 1)
                     << "\t start " << Simulator::Now () <<
                " end " << (Simulator::Now () + varTtiPeriod));

  Simulator::Schedule (NanoSeconds (1.0), &NrUePhy::SendDataChannels, this,
                       pktBurst, ctrlMsg, varTtiPeriod - NanoSeconds (2.0));
  return varTtiPeriod;
}

void
NrUePhy::StartVarTti (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);
  Time varTtiPeriod;

  m_currTbs = dci->m_tbSize;
  m_receptionEnabled = false;

  if (dci->m_type == DciInfoElementTdma::CTRL && dci->m_format == DciInfoElementTdma::DL)
    {
      varTtiPeriod = DlCtrl (dci);
    }
  else if (dci->m_type == DciInfoElementTdma::CTRL && dci->m_format == DciInfoElementTdma::UL)
    {
      varTtiPeriod = UlCtrl (dci);
    }
  else if (dci->m_type == DciInfoElementTdma::SRS && dci->m_format == DciInfoElementTdma::UL)
    {
      varTtiPeriod = UlSrs (dci);
    }
  else if (dci->m_type == DciInfoElementTdma::DATA && dci->m_format == DciInfoElementTdma::DL)
    {
      varTtiPeriod = DlData (dci);
    }
  else if (dci->m_type == DciInfoElementTdma::DATA && dci->m_format == DciInfoElementTdma::UL)
    {
      varTtiPeriod = UlData (dci);
    }

  Simulator::Schedule (varTtiPeriod, &NrUePhy::EndVarTti, this, dci);
}


void
NrUePhy::EndVarTti (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("DCI started at symbol " << static_cast<uint32_t> (dci->m_symStart) <<
               " which lasted for " << static_cast<uint32_t> (dci->m_numSym) <<
               " symbols finished");

  if (m_tryToPerformLbt)
    {
      TryToPerformLbt ();
      m_tryToPerformLbt = false;
    }

  if (m_currSlotAllocInfo.m_varTtiAllocInfo.size () == 0)
    {
      // end of slot
      m_currentSlot.Add (1);

      Simulator::Schedule (m_lastSlotStart + GetSlotPeriod () - Simulator::Now (),
                           &NrUePhy::StartSlot, this, m_currentSlot);
    }
  else
    {
      VarTtiAllocInfo allocation = m_currSlotAllocInfo.m_varTtiAllocInfo.front ();
      m_currSlotAllocInfo.m_varTtiAllocInfo.pop_front ();

      Time nextVarTtiStart = GetSymbolPeriod () * allocation.m_dci->m_symStart;

      Simulator::Schedule (nextVarTtiStart + m_lastSlotStart - Simulator::Now (),
                           &NrUePhy::StartVarTti, this, allocation.m_dci);
    }

  m_receptionEnabled = false;
}

void
NrUePhy::PhyDataPacketReceived (const Ptr<Packet> &p)
{
  Simulator::ScheduleWithContext (m_netDevice->GetNode ()->GetId (),
                                  GetTbDecodeLatency (),
                                  &NrUePhySapUser::ReceivePhyPdu,
                                  m_phySapUser,
                                  p);
  // m_phySapUser->ReceivePhyPdu (p);
}

void
NrUePhy::SendDataChannels (const Ptr<PacketBurst> &pb,
                           const std::list<Ptr<NrControlMessage> > &ctrlMsg,
                           const Time &duration)
{
  if (pb->GetNPackets () > 0)
    {
      LteRadioBearerTag tag;
      if (!pb->GetPackets ().front ()->PeekPacketTag (tag))
        {
          NS_FATAL_ERROR ("No radio bearer tag");
        }
    }

  m_spectrumPhy->StartTxDataFrames (pb, ctrlMsg, duration);
}

void
NrUePhy::SendCtrlChannels (Time prd)
{
  m_spectrumPhy->StartTxUlControlFrames (m_ctrlMsgs, prd);
  m_ctrlMsgs.clear ();
}

Ptr<NrDlCqiMessage>
NrUePhy::CreateDlCqiFeedbackMessage (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this);
  SpectrumValue newSinr = sinr;
  // CREATE DlCqiLteControlMessage
  Ptr<NrDlCqiMessage> msg = Create<NrDlCqiMessage> ();
  msg->SetSourceBwp (GetBwpId ());
  DlCqiInfo dlcqi;

  dlcqi.m_rnti = m_rnti;
  dlcqi.m_cqiType = DlCqiInfo::WB;

  std::vector<int> cqi;

  uint8_t mcs;
  dlcqi.m_wbCqi = m_amc->CreateCqiFeedbackWbTdma (newSinr, mcs);

  msg->SetDlCqi (dlcqi);
  return msg;
}

void
NrUePhy::GenerateDlCqiReport (const SpectrumValue& sinr)
{
  NS_LOG_FUNCTION (this);
  // Not totally sure what this is about. We have to check.
  if (m_ulConfigured && (m_rnti > 0) && m_receptionEnabled)
    {
      if (Simulator::Now () > m_wbCqiLast)
        {
          SpectrumValue newSinr = sinr;
          Ptr<NrDlCqiMessage> msg = CreateDlCqiFeedbackMessage (newSinr);

          if (msg)
            {
              DoSendControlMessage (msg);
            }
          m_reportCurrentCellRsrpSinrTrace (GetCellId (), m_rnti, 0.0, ComputeAvgSinr (sinr), GetBwpId ());
        }
    }
}

void
NrUePhy::EnqueueDlHarqFeedback (const DlHarqInfo &m)
{
  NS_LOG_FUNCTION (this);
  // get the feedback from NrSpectrumPhy and send it through ideal PUCCH to gNB
  Ptr<NrDlHarqFeedbackMessage> msg = Create<NrDlHarqFeedbackMessage> ();
  msg->SetSourceBwp (GetBwpId ());
  msg->SetDlHarqFeedback (m);

  auto k1It = m_harqIdToK1Map.find (m.m_harqProcessId);

  NS_LOG_DEBUG ("ReceiveLteDlHarqFeedback" << " Harq Process " <<
                static_cast<uint32_t> (k1It->first) <<
                " K1: " << k1It->second << " Frame " << m_currentSlot);

  Time event = m_lastSlotStart + (GetSlotPeriod () * k1It->second);
  if (event <= Simulator::Now ())
    {
      Simulator::ScheduleNow (&NrUePhy::DoSendControlMessageNow, this, msg);
    }
  else
    {
      Simulator::Schedule (event - Simulator::Now (), &NrUePhy::DoSendControlMessageNow, this, msg);
    }
}

void
NrUePhy::SetCam(const Ptr<NrChAccessManager> &cam)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (cam != nullptr);
  m_cam = cam;
  m_cam->SetAccessGrantedCallback (std::bind (&NrUePhy::ChannelAccessGranted, this,
                                              std::placeholders::_1));
  m_cam->SetAccessDeniedCallback (std::bind (&NrUePhy::ChannelAccessDenied, this));
}

const SfnSf &
NrUePhy::GetCurrentSfnSf () const
{
  return m_currentSlot;
}

uint16_t
NrUePhy::GetRnti ()
{
  return m_rnti;
}

void
NrUePhy::DoReset ()
{
  NS_LOG_FUNCTION (this);
  //initialize NR SL PSCCH packet queue
  m_nrSlPscchPacketBurstQueue.clear ();
  Ptr<PacketBurst> pbPscch = CreateObject <PacketBurst> ();
  m_nrSlPscchPacketBurstQueue.push_back (pbPscch);

  //initialize NR SL PSSCH packet queue
  m_nrSlPsschPacketBurstQueue.clear ();
  Ptr<PacketBurst> pbPssch = CreateObject <PacketBurst> ();
  m_nrSlPsschPacketBurstQueue.push_back (pbPssch);
}

void
NrUePhy::DoStartCellSearch (uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << dlEarfcn);
  DoSetInitialBandwidth ();
}

void
NrUePhy::DoSynchronizeWithEnb (uint16_t cellId, uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << cellId << dlEarfcn);
  DoSynchronizeWithEnb (cellId);
}

void
NrUePhy::DoSetPa (double pa)
{
  NS_LOG_FUNCTION (this << pa);
}

void
NrUePhy::DoSetRsrpFilterCoefficient (uint8_t rsrpFilterCoefficient)
{
  NS_LOG_FUNCTION (this << +rsrpFilterCoefficient);
}

void
NrUePhy::DoSynchronizeWithEnb (uint16_t cellId)
{
  NS_LOG_FUNCTION (this << cellId);
  DoSetCellId (cellId);
  DoSetInitialBandwidth ();
}

BeamId
NrUePhy::GetBeamId (uint16_t rnti) const
{
  NS_LOG_FUNCTION (this);
  // That's a bad specification: the UE PHY doesn't know anything about its beam id.
  NS_UNUSED (rnti);
  NS_FATAL_ERROR ("ERROR");
}

void
NrUePhy::ScheduleStartEventLoop (uint32_t nodeId, uint16_t frame, uint8_t subframe, uint16_t slot)
{
  NS_LOG_FUNCTION (this);
  Simulator::ScheduleWithContext (nodeId, MilliSeconds (0), &NrUePhy::StartEventLoop,
                                  this, frame, subframe, slot);
}

void
NrUePhy::ReportRsReceivedPower (const SpectrumValue& rsReceivedPower)
{
  NS_LOG_FUNCTION (this << rsReceivedPower);
  m_rsrp = 10 * log10 (Integral (rsReceivedPower)) + 30;
  NS_LOG_INFO ("RSRP value updated: " << m_rsrp);
  if (m_enableUplinkPowerControl)
    {
      m_powerControl->SetLoggingInfo (GetCellId(), m_rnti);
      m_powerControl->SetRsrp (m_rsrp);
    }
}

void
NrUePhy::StartEventLoop (uint16_t frame, uint8_t subframe, uint16_t slot)
{
  NS_LOG_FUNCTION (this);

  if (GetChannelBandwidth() == 0)
    {
      NS_LOG_INFO ("Initial bandwidth not set, configuring the default one for Cell ID:"<< GetCellId () << ", RNTI"<< GetRnti () <<", BWP ID:"<< GetBwpId ());
      DoSetInitialBandwidth ();
    }

  NS_LOG_DEBUG ("PHY starting. Configuration: "  << std::endl <<
                "\t TxPower: " << m_txPower << " dB" << std::endl <<
                "\t NoiseFigure: " << m_noiseFigure << std::endl <<
                "\t TbDecodeLatency: " << GetTbDecodeLatency ().GetMicroSeconds () << " us " << std::endl <<
                "\t Numerology: " << GetNumerology () << std::endl <<
                "\t SymbolsPerSlot: " << GetSymbolsPerSlot () << std::endl <<
                "\t Pattern: " << NrPhy::GetPattern (m_tddPattern) << std::endl <<
                "Attached to physical channel: " << std::endl <<
                "\t Channel bandwidth: " << GetChannelBandwidth () << " Hz" << std::endl <<
                "\t Channel central freq: " << GetCentralFrequency() << " Hz" << std::endl <<
                "\t Num. RB: " << GetRbNum ());
  SfnSf startSlot (frame, subframe, slot, GetNumerology ());
  StartSlot (startSlot);
}

void
NrUePhy::DoSetInitialBandwidth ()
{
  NS_LOG_FUNCTION (this);
  // configure initial bandwidth to 6 RBs
  double initialBandwidthHz = 6 * GetSubcarrierSpacing () * NrSpectrumValueHelper::SUBCARRIERS_PER_RB;
  // divided by 100*1000 because the parameter should be in 100KHz
  uint16_t initialBandwidthIn100KHz = ceil (initialBandwidthHz / (100 * 1000));
  // account for overhead that will be reduced when determining real BW
  uint16_t initialBandwidthWithOverhead = initialBandwidthIn100KHz / (1 - GetRbOverhead ());

  NS_ABORT_MSG_IF (initialBandwidthWithOverhead == 0, " Initial bandwidth could not be set. Parameters provided are: "
                   "\n dlBandwidthInRBNum = " << 6 <<
                   "\n m_subcarrierSpacing = " << GetSubcarrierSpacing() <<
                   "\n NrSpectrumValueHelper::SUBCARRIERS_PER_RB  = " << (unsigned) NrSpectrumValueHelper::SUBCARRIERS_PER_RB <<
                   "\n m_rbOh = " << GetRbOverhead() );

  DoSetDlBandwidth (initialBandwidthWithOverhead);
}

void
NrUePhy::DoSetDlBandwidth (uint16_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << +dlBandwidth);

  SetChannelBandwidth (dlBandwidth);

  NS_LOG_DEBUG ("PHY reconfiguring. Result: "  << std::endl <<
                "\t TxPower: " << m_txPower << " dB" << std::endl <<
                "\t NoiseFigure: " << m_noiseFigure << std::endl <<
                "\t TbDecodeLatency: " << GetTbDecodeLatency ().GetMicroSeconds () << " us " << std::endl <<
                "\t Numerology: " << GetNumerology () << std::endl <<
                "\t SymbolsPerSlot: " << GetSymbolsPerSlot () << std::endl <<
                "\t Pattern: " << NrPhy::GetPattern (m_tddPattern) << std::endl <<
                "Attached to physical channel: " << std::endl <<
                "\t Channel bandwidth: " << GetChannelBandwidth () << " Hz" << std::endl <<
                "\t Channel central freq: " << GetCentralFrequency() << " Hz" << std::endl <<
                "\t Num. RB: " << GetRbNum ());
}


void
NrUePhy::DoConfigureUplink (uint16_t ulEarfcn, uint8_t ulBandwidth)
{
  NS_LOG_FUNCTION (this << ulEarfcn << +ulBandwidth);
  // Ignore this; should be equal to dlBandwidth
  m_ulConfigured = true;
}

void
NrUePhy::DoConfigureReferenceSignalPower (int8_t referenceSignalPower)
{
  NS_LOG_FUNCTION (this << referenceSignalPower);
  m_powerControl->ConfigureReferenceSignalPower (referenceSignalPower);
}

void
NrUePhy::DoSetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  m_rnti = rnti;
}

void
NrUePhy::DoSetTransmissionMode (uint8_t txMode)
{
  NS_LOG_FUNCTION (this << +txMode);
}

void
NrUePhy::DoSetSrsConfigurationIndex (uint16_t srcCi)
{
  NS_LOG_FUNCTION (this << srcCi);
}

void
NrUePhy::SetPhySapUser (NrUePhySapUser* ptr)
{
  m_phySapUser = ptr;
}

void
NrUePhy::DoResetPhyAfterRlf ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("NrUePhy does not have RLF functionality yet");
}

void
NrUePhy::DoResetRlfParams ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("NrUePhy does not have RLF functionality yet");
}

void
NrUePhy::DoStartInSnycDetection ()
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("NrUePhy does not have RLF functionality yet");
}

void
NrUePhy::DoSetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this);
  m_imsi = imsi;
}

void
NrUePhy::PreConfigSlBandwidth (uint16_t slBandwidth)
{
  NS_LOG_FUNCTION (this << slBandwidth);
  if (GetChannelBandwidth () != slBandwidth)
    {
      SetChannelBandwidth (slBandwidth);
    }
}

void
NrUePhy::RegisterSlBwpId (uint16_t bwpId)
{
  NS_LOG_FUNCTION (this);

  // we initialize queues in DoReset;

  SetBwpId (bwpId);
}

NrSlUeCphySapProvider*
NrUePhy::GetNrSlUeCphySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_nrSlUeCphySapProvider;
}

void
NrUePhy::SetNrSlUeCphySapUser (NrSlUeCphySapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_nrSlUeCphySapUser = s;
}

void
NrUePhy::SetNrSlUePhySapUser(NrSlUePhySapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_nrSlUePhySapUser = s;
}


void
NrUePhy::DoAddNrSlCommTxPool (Ptr<const NrSlCommResourcePool> txPool)
{
  NS_LOG_FUNCTION (this);
  m_slTxPool = txPool;
}

void
NrUePhy::DoAddNrSlCommRxPool (Ptr<const NrSlCommResourcePool> rxPool)
{
  NS_LOG_FUNCTION (this);
  m_slRxPool = rxPool;
}

void
NrUePhy::StartNrSlSlot (const SfnSf &s)
{
  NS_LOG_FUNCTION (this);
  m_nrSlCurrentAlloc = m_nrSlAllocInfoQueue.front ();
  m_nrSlAllocInfoQueue.pop_front ();
  NS_ASSERT_MSG (m_nrSlCurrentAlloc.sfn == m_currentSlot, "Unable to find NR SL slot allocation");
  NrSlVarTtiAllocInfo varTtiInfo = *(m_nrSlCurrentAlloc.slvarTtiInfoList.begin());
  //erase the retrieved var TTI info
  m_nrSlCurrentAlloc.slvarTtiInfoList.erase (m_nrSlCurrentAlloc.slvarTtiInfoList.begin());
  auto nextVarTtiStart = GetSymbolPeriod () * varTtiInfo.symStart;
  Simulator::Schedule (nextVarTtiStart, &NrUePhy::StartNrSlVarTti, this, varTtiInfo);
}

void
NrUePhy::StartNrSlVarTti (const NrSlVarTtiAllocInfo &varTtiInfo)
{
  NS_LOG_FUNCTION (this);

  Time varTtiPeriod;

  if (varTtiInfo.SlVarTtiType == NrSlVarTtiAllocInfo::CTRL)
    {
      varTtiPeriod = SlCtrl (varTtiInfo);
    }
  else if (varTtiInfo.SlVarTtiType == NrSlVarTtiAllocInfo::DATA)
    {
      varTtiPeriod = SlData (varTtiInfo);
    }
  else
    {
      NS_FATAL_ERROR ("Invalid or unknown SL VarTti type " << varTtiInfo.SlVarTtiType);
    }


  Simulator::Schedule (varTtiPeriod, &NrUePhy::EndNrSlVarTti, this, varTtiInfo);
}

void
NrUePhy::EndNrSlVarTti (const NrSlVarTtiAllocInfo &varTtiInfo)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("NR SL var TTI started at symbol " << varTtiInfo.symStart <<
               " which lasted for " << varTtiInfo.symLength << " symbols");

  if (m_nrSlCurrentAlloc.slvarTtiInfoList.size () == 0)
      {
        // end of slot
        m_currentSlot.Add (1);
        //we need trigger the NR Slot start
        Simulator::Schedule (m_lastSlotStart + GetSlotPeriod () - Simulator::Now (),
                             &NrUePhy::StartSlot, this, m_currentSlot);
      }
    else
      {
        NrSlVarTtiAllocInfo nextVarTtiInfo = *(m_nrSlCurrentAlloc.slvarTtiInfoList.begin());
        //erase the retrieved var TTI info
        m_nrSlCurrentAlloc.slvarTtiInfoList.erase (m_nrSlCurrentAlloc.slvarTtiInfoList.begin());
        auto nextVarTtiStart = GetSymbolPeriod () * nextVarTtiInfo.symStart;

        Simulator::Schedule (nextVarTtiStart + m_lastSlotStart - Simulator::Now (),
                             &NrUePhy::StartNrSlVarTti, this, nextVarTtiInfo);
      }
}

Time
NrUePhy::SlCtrl (const NrSlVarTtiAllocInfo &varTtiInfo)
{
  NS_LOG_FUNCTION (this);

  Ptr<PacketBurst> pktBurst = PopPscchPacketBurst ();
  if (!pktBurst || pktBurst->GetNPackets () == 0)
    {
      NS_FATAL_ERROR ("No NR SL CTRL packet to transmit");
    }
  Time varTtiPeriod = GetSymbolPeriod () * varTtiInfo.symLength;
  // -1 ns ensures control ends before data period
  SendNrSlCtrlChannels (pktBurst, varTtiPeriod - NanoSeconds (1.0), varTtiInfo);

 return varTtiPeriod;
}

void
NrUePhy::SendNrSlCtrlChannels (const Ptr<PacketBurst> &pb, const Time &varTtiPeriod, const NrSlVarTtiAllocInfo &varTtiInfo)
{
  NS_LOG_FUNCTION (this);

  std::vector<int> channelRbs;
  uint32_t lastRbInPlusOne = (varTtiInfo.rbStart + varTtiInfo.rbLength);
  for (uint32_t i = varTtiInfo.rbStart; i < lastRbInPlusOne; i++)
    {
      channelRbs.push_back (static_cast<int> (i));
    }

  SetSubChannelsForTransmission (channelRbs, varTtiInfo.symLength);
  NS_LOG_DEBUG ("Sending PSCCH on SfnSf " << m_currentSlot);
  m_spectrumPhy->StartTxSlCtrlFrames (pb, varTtiPeriod);
}

Time
NrUePhy::SlData (const NrSlVarTtiAllocInfo &varTtiInfo)
{
  NS_LOG_FUNCTION (this);

  Time varTtiPeriod = GetSymbolPeriod () * varTtiInfo.symLength;
  Ptr<PacketBurst> pktBurst = PopPsschPacketBurst ();

  if (pktBurst && pktBurst->GetNPackets () > 0)
    {
      std::list< Ptr<Packet> > pkts = pktBurst->GetPackets ();
      LteRadioBearerTag bearerTag;
      if (!pkts.front ()->PeekPacketTag (bearerTag))
        {
          NS_FATAL_ERROR ("No radio bearer tag");
        }
    }
  else
    {
      // put an error, as something is wrong. The UE should not be scheduled
      // if there is no data for it...
      NS_FATAL_ERROR ("The UE " << m_rnti << " has been scheduled without NR SL data");
    }

  NS_LOG_DEBUG ("UE" << m_rnti <<
                " TXing NR SL DATA frame for symbols "  << varTtiInfo.symStart <<
                "-" << varTtiInfo.symLength - 1
                     << "\t start " << Simulator::Now () <<
                " end " << (Simulator::Now () + varTtiPeriod));

  Simulator::Schedule (NanoSeconds (1.0), &NrUePhy::SendNrSlDataChannels, this,
                       pktBurst, varTtiPeriod - NanoSeconds (2.0), varTtiInfo);
  return varTtiPeriod;
}

void
NrUePhy::SendNrSlDataChannels (const Ptr<PacketBurst> &pb, const Time &varTtiPeriod, const NrSlVarTtiAllocInfo &varTtiInfo)
{
  NS_LOG_FUNCTION (this);

  std::vector<int> channelRbs;
  uint32_t lastRbInPlusOne = (varTtiInfo.rbStart + varTtiInfo.rbLength);
  for (uint32_t i = varTtiInfo.rbStart; i < lastRbInPlusOne; i++)
    {
      channelRbs.push_back (static_cast<int> (i));
    }

  SetSubChannelsForTransmission (channelRbs, varTtiInfo.symLength);
  NS_LOG_DEBUG ("Sending PSSCH on SfnSf " << m_currentSlot);
  m_spectrumPhy->StartTxSlDataFrames (pb, varTtiPeriod);
}

void
NrUePhy::PhyPscchPduReceived (const Ptr<Packet> &p, const SpectrumValue &psd)
{
  NS_LOG_FUNCTION (this);
  NrSlSciF1aHeader sciF1a;
  NrSlMacPduTag tag;

  p->PeekHeader (sciF1a);
  p->PeekPacketTag (tag);

  std::unordered_set <uint32_t> destinations = m_nrSlUePhySapUser->GetSlRxDestinations ();

  NS_ASSERT_MSG (m_slRxPool != nullptr, "No receiving pools configured");
  uint16_t sbChSize = m_slRxPool->GetNrSlSubChSize (GetBwpId (), m_nrSlUePhySapUser->GetSlActiveTxPoolId ());
  uint16_t rbStart = sciF1a.GetIndexStartSubChannel () * sbChSize;
  uint16_t lastRbInPlusOne = (sciF1a.GetLengthSubChannel () * sbChSize) + rbStart;
  std::vector<int> rbBitMap;

  for (uint16_t i = rbStart; i < lastRbInPlusOne; ++i)
    {
      rbBitMap.push_back (i);
    }

  double rsrpDbm = GetSidelinkRsrp (psd);

  NS_LOG_DEBUG ("Sending sensing data to UE MAC. RSRP " << rsrpDbm << " dBm "
                << " Frame " << m_currentSlot.GetFrame ()
                << " SubFrame " << +m_currentSlot.GetSubframe ()
                << " Slot " << m_currentSlot.GetSlot ());

 SensingData sensingData (m_currentSlot, sciF1a.GetSlResourceReservePeriod (),
                          sciF1a.GetLengthSubChannel (),
                          sciF1a.GetIndexStartSubChannel (),
                          sciF1a.GetPriority (), rsrpDbm,
                          sciF1a.GetGapReTx1 (), sciF1a.GetIndexStartSbChReTx1(),
                          sciF1a.GetGapReTx2 (), sciF1a.GetIndexStartSbChReTx2());

  m_nrSlUePhySapUser->ReceiveSensingData (sensingData);

  auto it = destinations.find (tag.GetDstL2Id ());
  if (it != destinations.end ())
    {
      NS_LOG_INFO ("Received first stage SCI for destination " << *it << " from RNTI " << tag.GetRnti ());
      m_spectrumPhy->AddSlExpectedTb (tag.GetRnti (), tag.GetDstL2Id (),
                                      tag.GetTbSize (), sciF1a.GetMcs (),
                                      rbBitMap, tag.GetSymStart (),
                                      tag.GetNumSym (), tag.GetSfn ());
      SaveFutureSlRxGrants (sciF1a, tag, sbChSize);
    }
  else
    {
      NS_LOG_INFO ("Ignoring PSCCH! Destination " << tag.GetDstL2Id () << " is not monitored by RNTI " << m_rnti);
    }
}

void
NrUePhy::SaveFutureSlRxGrants (const NrSlSciF1aHeader& sciF1a,
                               const NrSlMacPduTag& tag,
                               const uint16_t sbChSize)
{
  NS_LOG_FUNCTION (this);

  if (sciF1a.GetGapReTx1 () != std::numeric_limits <uint8_t>::max ())
    {
      uint16_t rbStart = sciF1a.GetIndexStartSbChReTx1 () * sbChSize;
      uint16_t lastRbInPlusOne = (sciF1a.GetLengthSubChannel () * sbChSize) + rbStart;
      std::vector<int> rbBitMap;
      for (uint16_t i = rbStart; i < lastRbInPlusOne; ++i)
        {
          rbBitMap.push_back (i);
        }
      SlRxGrantInfo infoTb (tag.GetRnti (), tag.GetDstL2Id (),
                            tag.GetTbSize (), sciF1a.GetMcs (),
                            rbBitMap, tag.GetSymStart (),
                            tag.GetNumSym (),
                            tag.GetSfn().GetFutureSfnSf (sciF1a.GetGapReTx1 ()));
      m_slRxGrants.push_back (infoTb);
    }
  if (sciF1a.GetGapReTx2 () != std::numeric_limits <uint8_t>::max ())
    {
      uint16_t rbStart = sciF1a.GetIndexStartSbChReTx2 () * sbChSize;
      uint16_t lastRbInPlusOne = (sciF1a.GetLengthSubChannel () * sbChSize) + rbStart;
      std::vector<int> rbBitMap;
      for (uint16_t i = rbStart; i < lastRbInPlusOne; ++i)
        {
          rbBitMap.push_back (i);
        }
      SlRxGrantInfo infoTb (tag.GetRnti (), tag.GetDstL2Id (),
                            tag.GetTbSize (), sciF1a.GetMcs (),
                            rbBitMap, tag.GetSymStart (),
                            tag.GetNumSym (),
                            tag.GetSfn().GetFutureSfnSf (sciF1a.GetGapReTx2 ()));
      m_slRxGrants.push_back (infoTb);
    }

  NS_LOG_DEBUG ("Expecting " << m_slRxGrants.size () << " future PSSCH slots without SCI 1-A");
  for (const auto &it : m_slRxGrants)
    {
      NS_LOG_DEBUG ("Expecting on SfnSf " << it.sfn);
    }
}

void
NrUePhy::SendSlExpectedTbInfo (const SfnSf &s)
{
  NS_LOG_FUNCTION (this);
  if (m_slRxGrants.size () > 0)
    {
      auto expextedTbInfo = m_slRxGrants.front ();
      if (expextedTbInfo.sfn == s)
        {
          m_slRxGrants.pop_front ();
          m_spectrumPhy->AddSlExpectedTb (expextedTbInfo.rnti,
                                          expextedTbInfo.dstId,
                                          expextedTbInfo.tbSize,
                                          expextedTbInfo.mcs,
                                          expextedTbInfo.rbBitmap,
                                          expextedTbInfo.symStart,
                                          expextedTbInfo.numSym,
                                          expextedTbInfo.sfn);
        }
    }
}

void
NrUePhy::PhyPsschPduReceived (const Ptr<PacketBurst> &pb)
{
  NS_LOG_FUNCTION (this);
  Simulator::ScheduleWithContext (m_netDevice->GetNode ()->GetId (),
                                  GetTbDecodeLatency (),
                                  &NrSlUePhySapUser::ReceivePsschPhyPdu,
                                  m_nrSlUePhySapUser, pb);

  //m_nrSlUePhySapUser->ReceivePsschPhyPdu (pb);
}

double
NrUePhy::GetSidelinkRsrp (SpectrumValue psd)
{
  // Measure instantaneous S-RSRP...
  double sum = 0.0;
  uint16_t numRB = 0;

      for (Values::const_iterator itPi = psd.ConstValuesBegin(); itPi != psd.ConstValuesEnd(); itPi++)
        {
          if((*itPi))
            {
              uint32_t scSpacing = 15000 * static_cast<uint32_t> (std::pow (2, GetNumerology ()));
              uint32_t RbWidthInHz = static_cast<uint32_t> (scSpacing * NrSpectrumValueHelper::SUBCARRIERS_PER_RB);
              double powerTxWattPerRb = ((*itPi) * RbWidthInHz); //convert PSD [W/Hz] to linear power [W]
              double powerTxWattPerRe = (powerTxWattPerRb / NrSpectrumValueHelper::SUBCARRIERS_PER_RB); // power of one RE per RB
              double PowerTxWattDmrsPerRb = powerTxWattPerRe * 3.0; // TS 38.211 sec 8.4.1.3, 3 RE per RB carries PSCCH DMRS, i.e. Comb 4
              sum += PowerTxWattDmrsPerRb;
              numRB++;
            }
        }

  double avrgRsrpWatt = (sum / ((double) numRB * 3.0));
  double rsrpDbm = 10 * log10 (1000 * (avrgRsrpWatt));

  return rsrpDbm;
}

}



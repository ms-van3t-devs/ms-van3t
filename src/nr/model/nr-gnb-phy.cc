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

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      std::clog << " [ CellId " << GetCellId() << ", bwpId "             \
                << GetBwpId () << "] ";                                  \
    }                                                                    \
  while (false);

#include <ns3/log.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/node.h>
#include <algorithm>
#include <functional>
#include <string>
#include <unordered_set>

#include "nr-gnb-phy.h"
#include "nr-ue-phy.h"
#include "nr-net-device.h"
#include "nr-ue-net-device.h"
#include "nr-gnb-net-device.h"
#include "nr-radio-bearer-tag.h"
#include "nr-ch-access-manager.h"

#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/pointer.h>
#include <ns3/double.h>
#include <ns3/boolean.h>
#include "beam-manager.h"
#include <ns3/object-vector.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrGnbPhy");

NS_OBJECT_ENSURE_REGISTERED (NrGnbPhy);

NrGnbPhy::NrGnbPhy ():
    m_n0Delay (0),
    m_n1Delay (4)
{
  NS_LOG_FUNCTION (this);
  m_enbCphySapProvider = new MemberLteEnbCphySapProvider<NrGnbPhy> (this);
}

NrGnbPhy::~NrGnbPhy ()
{
}


void
NrGnbPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_enbCphySapProvider;
  NrPhy::DoDispose ();
}

TypeId
NrGnbPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrGnbPhy")
    .SetParent<NrPhy> ()
    .AddConstructor<NrGnbPhy> ()
    .AddAttribute ("RbOverhead",
                   "Overhead when calculating the usable RB number",
                   DoubleValue (0.04),
                   MakeDoubleAccessor (&NrGnbPhy::SetRbOverhead,
                                       &NrGnbPhy::GetRbOverhead),
                   MakeDoubleChecker <double> (0, 0.5))
    .AddAttribute ("TxPower",
                   "Transmission power in dBm",
                   DoubleValue (4.0),
                   MakeDoubleAccessor (&NrGnbPhy::SetTxPower,
                                       &NrGnbPhy::GetTxPower),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("NoiseFigure",
                   "Loss (dB) in the Signal-to-Noise-Ratio due to non-idealities in the receiver."
                   " According to Wikipedia (http://en.wikipedia.org/wiki/Noise_figure), this is "
                   "\"the difference in decibels (dB) between"
                   " the noise output of the actual receiver to the noise output of an "
                   " ideal receiver with the same overall gain and bandwidth when the receivers "
                   " are connected to sources at the standard noise temperature T0.\" "
                   "In this model, we consider T0 = 290K.",
                   DoubleValue (5.0),
                   MakeDoubleAccessor (&NrPhy::SetNoiseFigure,
                                       &NrPhy::GetNoiseFigure),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("PowerAllocationType",
                   "Defines the type of the power allocation. Currently are supported "
                   "two types: \"UniformPowerAllocBw\", which is a uniform power allocation over all "
                   "bandwidth (over all RBs), and \"UniformPowerAllocUsed\", which is a uniform "
                   "power allocation over used (active) RBs. By default is set a uniform power "
                   "allocation over used RBs .",
                   EnumValue (NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_USED),
                   MakeEnumAccessor (&NrPhy::SetPowerAllocationType,
                                     &NrPhy::GetPowerAllocationType),
                   MakeEnumChecker ( NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_BW, "UniformPowerAllocBw",
                                     NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_USED, "UniformPowerAllocUsed"
                                   ))
    .AddTraceSource ("UlSinrTrace",
                     "UL SINR statistics.",
                     MakeTraceSourceAccessor (&NrGnbPhy::m_ulSinrTrace),
                     "ns3::UlSinr::TracedCallback")
    .AddTraceSource ("GnbPhyRxedCtrlMsgsTrace",
                     "Enb PHY Rxed Control Messages Traces.",
                     MakeTraceSourceAccessor (&NrGnbPhy::m_phyRxedCtrlMsgsTrace),
                     "ns3::NrPhyRxTrace::RxedGnbPhyCtrlMsgsTracedCallback")
    .AddTraceSource ("GnbPhyTxedCtrlMsgsTrace",
                     "Enb PHY Txed Control Messages Traces.",
                     MakeTraceSourceAccessor (&NrGnbPhy::m_phyTxedCtrlMsgsTrace),
                     "ns3::NrPhyRxTrace::TxedGnbPhyCtrlMsgsTracedCallback")
    .AddAttribute ("N0Delay",
                   "Minimum processing delay needed to decode DL DCI and decode DL data",
                    UintegerValue (0),
                    MakeUintegerAccessor (&NrGnbPhy::SetN0Delay,
                                          &NrGnbPhy::GetN0Delay),
                    MakeUintegerChecker<uint32_t> (0, 1))
    .AddAttribute ("N1Delay",
                   "Minimum processing delay (UE side) from the end of DL Data reception to "
                   "the earliest possible start of the corresponding ACK/NACK transmission",
                    UintegerValue (2),
                    MakeUintegerAccessor (&NrGnbPhy::SetN1Delay,
                                          &NrGnbPhy::GetN1Delay),
                    MakeUintegerChecker<uint32_t> (0, 4))
    .AddAttribute ("N2Delay",
                   "Minimum processing delay needed to decode UL DCI and prepare UL data",
                   UintegerValue (2),
                   MakeUintegerAccessor (&NrGnbPhy::SetN2Delay,
                                         &NrGnbPhy::GetN2Delay),
                   MakeUintegerChecker<uint32_t> (0, 4))
    .AddAttribute ("TbDecodeLatency",
                   "Transport block decode latency",
                    TimeValue (MicroSeconds (100)),
                    MakeTimeAccessor (&NrPhy::SetTbDecodeLatency,
                                      &NrPhy::GetTbDecodeLatency),
                    MakeTimeChecker ())
    .AddAttribute ("Numerology",
                   "The 3GPP numerology to be used",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrPhy::SetNumerology,
                                         &NrPhy::GetNumerology),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("SymbolsPerSlot",
                   "Number of symbols in one slot",
                   UintegerValue (14),
                   MakeUintegerAccessor (&NrPhy::SetSymbolsPerSlot,
                                         &NrPhy::GetSymbolsPerSlot),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Pattern",
                   "The slot pattern",
                   StringValue ("F|F|F|F|F|F|F|F|F|F|"),
                   MakeStringAccessor (&NrGnbPhy::SetPattern,
                                       &NrGnbPhy::GetPattern),
                   MakeStringChecker ())
    .AddAttribute ("NrSpectrumPhyList", "List of all SpectrumPhy instances of this NrUePhy.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&NrGnbPhy::m_spectrumPhys),
                   MakeObjectVectorChecker<NrSpectrumPhy> ())
    .AddTraceSource ("SlotDataStats",
                     "Data statistics for the current slot: SfnSf, active UE, used RE, "
                     "used symbols, available RBs, available symbols, bwp ID, cell ID",
                     MakeTraceSourceAccessor (&NrGnbPhy::m_phySlotDataStats),
                     "ns3::NrGnbPhy::SlotStatsTracedCallback")
    .AddTraceSource ("SlotCtrlStats",
                     "Ctrl statistics for the current slot: SfnSf, active UE, used RE, "
                     "used symbols, available RBs, available symbols, bwp ID, cell ID",
                     MakeTraceSourceAccessor (&NrGnbPhy::m_phySlotCtrlStats),
                     "ns3::NrGnbPhy::SlotStatsTracedCallback")
    .AddTraceSource ("RBDataStats",
                     "Resource Block used for data: SfnSf, symbol, RB PHY map, bwp ID, cell ID",
                     MakeTraceSourceAccessor (&NrGnbPhy::m_rbStatistics),
                     "ns3::NrGnbPhy::RBStatsTracedCallback")
    ;
  return tid;

}

uint32_t
NrGnbPhy::GetNumRbPerRbg () const
{
  return m_phySapUser->GetNumRbPerRbg();
}

const SfnSf &
NrGnbPhy::GetCurrentSfnSf () const
{
  return m_currentSlot;
}

/**
 * \brief An intelligent way to calculate the modulo
 * \param n Number
 * \param m Modulo
 * \return n+=m until n < 0
 */
static uint32_t modulo (int n, uint32_t m)
{
  if (n >= 0)
    {
      return static_cast<uint32_t> (n) % m;
    }
  else
    {
      while (n < 0) {
          n += m;
        }
      return static_cast<uint32_t> (n);
    }
}

/**
 * \brief Return the slot in which the DL HARQ Feedback should be sent, according to the parameter N1
 * \param pattern The TDD pattern
 * \param pos The position of the data inside the pattern for which we want to find where the feedback should be sent
 * \param n1 The N1 parameter
 * \return k1 (after how many slots the DL HARQ Feedback should be sent)
 *
 * Please note that for the LTE TDD case, although the calculation follows the
 * logic of Table 10.1-1 of TS 36.213, some configurations are simplified in order
 * to avoid having a table from where we take the K1 values. In particular, for
 * configurations 3, 4 and 6 (starting form 0), the specification splits the
 * HARQ feedbacks among all UL subframes in an equal (as much as possible) manner.
 * This tactic is ommitted in this implementation.
 */
static int32_t
ReturnHarqSlot (const std::vector<LteNrTddSlotType> &pattern, uint32_t pos, uint32_t n1)
{
  int32_t k1 = static_cast<int32_t> (n1);

  uint32_t index = modulo (static_cast<int> (pos) + k1, static_cast<uint32_t> (pattern.size ()));

  while (pattern[index] < LteNrTddSlotType::F)
    {
      k1++;
      index = modulo (static_cast<int> (pos) + k1, static_cast<uint32_t> (pattern.size ()));
      NS_ASSERT (index < pattern.size ());
    }

  return k1;
}

struct DciKPair
{
  uint32_t indexDci {0};
  uint32_t k {0};
};

/**
 * \brief Return the slot in which the DCI should be send, according to the parameter n,
 * along with the number of slots required to add to the current slot to get the slot of DCI (k0/k2)
 * \param pattern The TDD pattern
 * \param pos The position inside the pattern for which we want to check where the DCI should be sent
 * \param n The N parameter (equal to N0 or N2, depending if it is DL or UL)
 * \return The slot position in which the DCI for the position specified should be sent and the k0/k2
 */
static DciKPair
ReturnDciSlot (const std::vector<LteNrTddSlotType> &pattern, uint32_t pos, uint32_t n)
{
  DciKPair ret;
  ret.k = n;
  ret.indexDci = modulo (static_cast<int> (pos) - static_cast<int> (ret.k),
                           static_cast<uint32_t> (pattern.size ()));

  while (pattern[ret.indexDci] > LteNrTddSlotType::F)
    {
      ret.k++;
      ret.indexDci = modulo (static_cast<int> (pos) - static_cast<int> (ret.k),
                      static_cast<uint32_t> (pattern.size ()));
      NS_ASSERT (ret.indexDci < pattern.size ());
    }

  return ret;
}

/**
 * \brief Generates the map tosendDl/Ul that holds the information of the DCI Slot and the
 * corresponding k0/k2 value, and the generateDl/Ul that includes the L1L2CtrlLatency.
 * \param pattern The TDD pattern, the pattern to analyze
 * \param toSend The structure toSendDl/tosendUl to fill
 * \param generate The structure generateDl/generateUl to fill
 * \param pos The position inside the pattern for which we want to check where the DCI should be sent
 * \param n The N parameter (equal to N0 or N2, depending if it is DL or UL)
 * \param l1l2CtrlLatency L1L2CtrlLatency of the system
 */
static void GenerateDciMaps (const std::vector<LteNrTddSlotType> &pattern,
                             std::map<uint32_t, std::vector<uint32_t>> *toSend,
                             std::map<uint32_t, std::vector<uint32_t>> *generate,
                             uint32_t pos, uint32_t n, uint32_t l1l2CtrlLatency)
{
  auto dciSlot = ReturnDciSlot (pattern, pos, n);
  uint32_t indexGen = modulo (static_cast<int>(dciSlot.indexDci) - static_cast<int> (l1l2CtrlLatency),
                              static_cast<uint32_t> (pattern.size ()));
  uint32_t kWithCtrlLatency = static_cast<uint32_t> (dciSlot.k) + l1l2CtrlLatency;

  (*toSend)[dciSlot.indexDci].push_back(static_cast<uint32_t> (dciSlot.k));
  (*generate)[indexGen].push_back (kWithCtrlLatency);
}

void
NrGnbPhy::GenerateStructuresFromPattern (const std::vector<LteNrTddSlotType> &pattern,
                                             std::map<uint32_t, std::vector<uint32_t>> *toSendDl,
                                             std::map<uint32_t, std::vector<uint32_t>> *toSendUl,
                                             std::map<uint32_t, std::vector<uint32_t>> *generateDl,
                                             std::map<uint32_t, std::vector<uint32_t>> *generateUl,
                                             std::map<uint32_t, uint32_t> *dlHarqfbPosition,
                                             uint32_t n0, uint32_t n2, uint32_t n1, uint32_t l1l2CtrlLatency)
{
  const uint32_t n = static_cast<uint32_t> (pattern.size ());

  // Create a pattern that is all F.
  std::vector<LteNrTddSlotType> fddGenerationPattern;
  fddGenerationPattern.resize (pattern.size (), LteNrTddSlotType::F);

  /* if we have to generate structs for a TDD pattern, then use the input pattern.
   * Otherwise, pass to the gen functions a pattern which is all F (therefore, the
   * the function will think that they will be able to transmit or
   * receive things following n0, n1, n2, that is what happen in FDD, just in
   * another band..
   */

  const std::vector<LteNrTddSlotType> *generationPattern;

  if (IsTdd (pattern))
    {
      generationPattern = &pattern;
    }
  else
    {
      generationPattern = &fddGenerationPattern;
    }

  for (uint32_t i = 0; i < n; i++)
    {
      if ((*generationPattern)[i] == LteNrTddSlotType::UL)
        {
          GenerateDciMaps (*generationPattern, toSendUl, generateUl, i, n2, l1l2CtrlLatency);
        }
      else if ((*generationPattern)[i] == LteNrTddSlotType::DL || pattern[i] == LteNrTddSlotType::S)
        {
          GenerateDciMaps (*generationPattern, toSendDl, generateDl, i, n0, l1l2CtrlLatency);

          int32_t k1 = ReturnHarqSlot (*generationPattern, i, n1);
          (*dlHarqfbPosition).insert (std::make_pair (i, k1));
        }
      else if ((*generationPattern)[i] == LteNrTddSlotType::F)
        {
          GenerateDciMaps (*generationPattern, toSendDl, generateDl, i, n0, l1l2CtrlLatency);
          GenerateDciMaps (*generationPattern, toSendUl, generateUl, i, n2, l1l2CtrlLatency);

          int32_t k1 = ReturnHarqSlot (*generationPattern, i, n1);
          (*dlHarqfbPosition).insert (std::make_pair (i, k1));
        }
    }

  /*
   * Now, if the input pattern is for FDD, remove the elements in the
   * opposite generate* structures: in the end, we don't want to generate DL
   * for a FDD-UL band, right?
   *
   * But.. maintain the toSend structures, as they will be used to send
   * feedback or other messages, like DCI.
   */

  if (! IsTdd (pattern))
    {
      if (HasUlSlot (pattern))
        {
          generateDl->clear ();
        }
      else
        {
          generateUl->clear();
        }
    }

  for (auto & list : (*generateUl))
    {
      std::sort (list.second.begin (), list.second.end ());
    }

  for (auto & list : (*generateDl))
    {
      std::sort (list.second.begin (), list.second.end ());
    }
}

void
NrGnbPhy::PushDlAllocation (const SfnSf &sfnSf) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phySapUser);

  auto dci = m_phySapUser->GetDlCtrlDci ();
  VarTtiAllocInfo dlCtrlVarTti (dci);

  SlotAllocInfo slotAllocInfo = SlotAllocInfo (sfnSf);

  slotAllocInfo.m_numSymAlloc = dlCtrlVarTti.m_dci->m_numSym;
  slotAllocInfo.m_type = SlotAllocInfo::DL;
  slotAllocInfo.m_varTtiAllocInfo.emplace_back (dlCtrlVarTti);

  m_phySapProvider->SetSlotAllocInfo (slotAllocInfo);
}

void
NrGnbPhy::PushUlAllocation (const SfnSf &sfnSf) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_phySapUser);

  auto dci = m_phySapUser->GetUlCtrlDci ();
  VarTtiAllocInfo ulCtrlVarTti (dci);

  SlotAllocInfo slotAllocInfo = SlotAllocInfo (sfnSf);

  slotAllocInfo.m_numSymAlloc = ulCtrlVarTti.m_dci->m_numSym;
  slotAllocInfo.m_type = SlotAllocInfo::UL;
  slotAllocInfo.m_varTtiAllocInfo.emplace_back (ulCtrlVarTti);

  m_phySapProvider->SetSlotAllocInfo (slotAllocInfo);
}

void
NrGnbPhy::SetTddPattern (const std::vector<LteNrTddSlotType> &pattern)
{
  NS_LOG_FUNCTION (this);

  std::stringstream ss;

  for (const auto & v : pattern)
    {
      ss << v << "|";
    }
  NS_LOG_INFO ("Set pattern : " << ss.str ());

  m_tddPattern = pattern;

  m_generateDl.clear ();
  m_generateUl.clear ();
  m_toSendDl.clear ();
  m_toSendUl.clear ();
  m_dlHarqfbPosition.clear ();

  GenerateStructuresFromPattern (pattern, &m_toSendDl, &m_toSendUl,
                                 &m_generateDl, &m_generateUl,
                                 &m_dlHarqfbPosition, 0,
                                 GetN2Delay (), GetN1Delay (),
                                 GetL1L2CtrlLatency ());
}

void
NrGnbPhy::ScheduleStartEventLoop (uint32_t nodeId, uint16_t frame, uint8_t subframe, uint16_t slot)
{
  NS_LOG_FUNCTION (this);
  Simulator::ScheduleWithContext (nodeId, MilliSeconds (0),
                                  &NrGnbPhy::StartEventLoop, this, frame, subframe, slot);
}

void
NrGnbPhy::StartEventLoop (uint16_t frame, uint8_t subframe, uint16_t slot)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("PHY starting. Configuration: "  << std::endl <<
                "\t TxPower: " << m_txPower << " dB" << std::endl <<
                "\t NoiseFigure: " << m_noiseFigure << std::endl <<
                "\t N0: " << m_n0Delay << std::endl <<
                "\t N1: " << m_n1Delay << std::endl <<
                "\t N2: " << m_n2Delay << std::endl <<
                "\t TbDecodeLatency: " << GetTbDecodeLatency ().GetMicroSeconds () << " us " << std::endl <<
                "\t Numerology: " << GetNumerology () << std::endl <<
                "\t SymbolsPerSlot: " << GetSymbolsPerSlot () << std::endl <<
                "\t Pattern: " << GetPattern () << std::endl <<
                "Attached to physical channel: " << std::endl <<
                "\t Channel bandwidth: " << GetChannelBandwidth () << " Hz" << std::endl <<
                "\t Channel central freq: " << GetCentralFrequency() << " Hz" << std::endl <<
                "\t Num. RB: " << GetRbNum ());
  SfnSf startSlot (frame, subframe, slot, GetNumerology ());
  InitializeMessageList ();
  StartSlot (startSlot);
}

void
NrGnbPhy::SetEnbCphySapUser (LteEnbCphySapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_enbCphySapUser = s;
}

LteEnbCphySapProvider*
NrGnbPhy::GetEnbCphySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_enbCphySapProvider;
}

uint32_t
NrGnbPhy::GetN0Delay (void) const
{
  return m_n0Delay;
}

uint32_t
NrGnbPhy::GetN1Delay (void) const
{
  return m_n1Delay;
}

uint32_t
NrGnbPhy::GetN2Delay () const
{
  return m_n2Delay;
}

void
NrGnbPhy::SetN0Delay (uint32_t delay)
{
  m_n0Delay = delay;
  SetTddPattern (m_tddPattern); // Update the generate/send structures
}

void
NrGnbPhy::SetN1Delay (uint32_t delay)
{
  m_n1Delay = delay;
  SetTddPattern (m_tddPattern); // Update the generate/send structures
}

void
NrGnbPhy::SetN2Delay (uint32_t delay)
{
  m_n2Delay = delay;
  SetTddPattern (m_tddPattern); // Update the generate/send structures
}

BeamConfId
NrGnbPhy::GetBeamConfId (uint16_t rnti) const
{
  NS_LOG_FUNCTION (this);

  NS_ABORT_MSG_UNLESS (m_spectrumPhys.size () == 1 || m_spectrumPhys.size () == 2, " Currently the BeamConfId implementation supports up to 2 antenna arrays per PHY instance.");

  for (uint8_t i = 0; i < m_deviceMap.size (); i++)
    {
      Ptr<NrUeNetDevice> ueDev = DynamicCast < NrUeNetDevice > (m_deviceMap.at (i));
      uint64_t ueRnti = (DynamicCast<NrUePhy>(ueDev->GetPhy (GetBwpId ())))->GetRnti ();

      if (ueRnti == rnti)
        {
          NS_ASSERT (m_spectrumPhys [0]->GetBeamManager ());
          BeamId beamId1 = m_spectrumPhys [0]->GetBeamManager ()->GetBeamId (m_deviceMap.at (i));
          BeamId beamId2 = BeamId::GetEmptyBeamId ();

          if (m_spectrumPhys.size () > 1)
            {
              m_spectrumPhys [1]->GetBeamManager ()->GetBeamId (m_deviceMap.at (i));
            }
          return BeamConfId (beamId1, beamId2);
        }
    }
  return BeamConfId (BeamId (0,0), BeamId::GetEmptyBeamId ());
}

void
NrGnbPhy::SetCam (const Ptr<NrChAccessManager> &cam)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (cam != nullptr);
  m_cam = cam;
  m_cam->SetAccessGrantedCallback (std::bind (&NrGnbPhy::ChannelAccessGranted, this,
                                              std::placeholders::_1));
  m_cam->SetAccessDeniedCallback (std::bind (&NrGnbPhy::ChannelAccessLost, this));
}

Ptr<NrChAccessManager>
NrGnbPhy::GetCam() const
{
  NS_LOG_FUNCTION (this);
  return m_cam;
}

void
NrGnbPhy::SetTxPower (double pow)
{
  m_txPower = pow;
}
double
NrGnbPhy::GetTxPower () const
{
  return m_txPower;
}

void
NrGnbPhy::SetSubChannels (const std::vector<int> &rbIndexVector, uint8_t activeStreams)
{
  Ptr<SpectrumValue> txPsd = GetTxPowerSpectralDensity (rbIndexVector, activeStreams);
  NS_ASSERT (txPsd);
  for (uint8_t streamIndex = 0; streamIndex < m_spectrumPhys.size(); streamIndex++)
    {
      m_spectrumPhys.at(streamIndex)->SetTxPowerSpectralDensity (txPsd);
    }

}

void
NrGnbPhy::QueueMib ()
{
  NS_LOG_FUNCTION (this);
  LteRrcSap::MasterInformationBlock mib;
  mib.dlBandwidth = GetChannelBandwidth () / (1000 * 100);
  mib.systemFrameNumber = 1;
  Ptr<NrMibMessage> mibMsg = Create<NrMibMessage> ();
  mibMsg->SetSourceBwp (GetBwpId ());
  mibMsg->SetMib (mib);
  EnqueueCtrlMsgNow (mibMsg);
}

void NrGnbPhy::QueueSib ()
{
  NS_LOG_FUNCTION (this);
  Ptr<NrSib1Message> msg = Create<NrSib1Message> ();
  msg->SetSib1 (m_sib1);
  msg->SetSourceBwp (GetBwpId ());
  EnqueueCtrlMsgNow (msg);
}

void
NrGnbPhy::CallMacForSlotIndication (const SfnSf &currentSlot)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (!m_generateDl.empty() || !m_generateUl.empty());

  m_phySapUser->SetCurrentSfn (currentSlot);

  uint64_t currentSlotN = currentSlot.Normalize () % m_tddPattern.size ();

  NS_LOG_INFO ("Start Slot " << currentSlot << ". In position " <<
               currentSlotN << " there is a slot of type " <<
               m_tddPattern[currentSlotN]);

  for (const auto & k2WithLatency : m_generateUl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;
      targetSlot.Add (k2WithLatency);

      uint64_t pos = targetSlot.Normalize () % m_tddPattern.size ();

      NS_LOG_INFO (" in slot " << currentSlot << " generate UL for " <<
                     targetSlot << " which is of type " << m_tddPattern[pos]);

      m_phySapUser->SlotUlIndication (targetSlot, m_tddPattern[pos]);
    }

  for (const auto & k0WithLatency : m_generateDl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;
      targetSlot.Add (k0WithLatency);

      uint64_t pos = targetSlot.Normalize () % m_tddPattern.size ();

      NS_LOG_INFO (" in slot " << currentSlot << " generate DL for " <<
                     targetSlot << " which is of type " << m_tddPattern[pos]);

      m_phySapUser->SlotDlIndication (targetSlot, m_tddPattern[pos]);
    }
}

void
NrGnbPhy::StartSlot (const SfnSf &startSlot)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_channelStatus != TO_LOSE);

  m_currentSlot = startSlot;
  m_lastSlotStart = Simulator::Now ();

  Simulator::Schedule (GetSlotPeriod (), &NrGnbPhy::EndSlot, this);

  // update the current slot allocation; if empty (e.g., at the beginning of simu)
  // then insert a dummy allocation, without anything.
  if (SlotAllocInfoExists (m_currentSlot))
    {
      m_currSlotAllocInfo = RetrieveSlotAllocInfo (m_currentSlot);
    }
  else
    {
      NS_LOG_WARN ("No allocation for the current slot. Using an empty one");
      m_currSlotAllocInfo = SlotAllocInfo (m_currentSlot);
    }

  if (m_isPrimary)
    {
      if (m_currentSlot.GetSlot () == 0)
        {
          bool mibOrSib = false;
          if (m_currentSlot.GetSubframe () == 0)   //send MIB at the beginning of each frame
            {
              QueueMib ();
              mibOrSib = true;
            }
          else if (m_currentSlot.GetSubframe () == 5)   // send SIB at beginning of second half-frame
            {
              QueueSib ();
              mibOrSib = true;
            }
          if (mibOrSib && !m_currSlotAllocInfo.ContainsDlCtrlAllocation ())
            {
              VarTtiAllocInfo dlCtrlSlot (m_phySapUser->GetDlCtrlDci ());
              m_currSlotAllocInfo.m_varTtiAllocInfo.push_front (dlCtrlSlot);
              m_currSlotAllocInfo.m_numSymAlloc += 1;
            }
        }
    }

  if (m_channelStatus == GRANTED)
    {
      NS_LOG_INFO ("Channel granted");
      CallMacForSlotIndication (m_currentSlot);
      DoStartSlot ();
    }
  else
    {
      bool hasUlDci = false;
      SfnSf ulSfn = m_currentSlot;
      ulSfn.Add (GetN2Delay ());

      if (GetN2Delay () > 0)
      {
        if (SlotAllocInfoExists (ulSfn))
          {
            SlotAllocInfo & ulSlot = PeekSlotAllocInfo (ulSfn);
            hasUlDci = ulSlot.ContainsDataAllocation () || ulSlot.ContainsUlCtrlAllocation ();
          }
      }
      // If there is a DL CTRL, try to obtain the channel to transmit it;
      // because, even if right now there isn't any message, maybe they
      // will come from another BWP.
      if (m_currSlotAllocInfo.ContainsDataAllocation () || m_currSlotAllocInfo.ContainsDlCtrlAllocation () || hasUlDci)
        {
          // Request the channel access
          if (m_channelStatus == NONE)
            {
              NS_LOG_INFO ("Channel not granted, request the channel");
              m_channelStatus = REQUESTED; // This goes always before RequestAccess()
              m_cam->RequestAccess ();
              if (m_channelStatus == GRANTED)
                {
                  // Repetition but we can have a CAM that gives the channel
                  // instantaneously
                  NS_LOG_INFO ("Channel granted; asking MAC for SlotIndication for the future and then start the slot");
                  CallMacForSlotIndication (m_currentSlot);
                  DoStartSlot ();
                  return; // Exit without calling anything else
                }
            }
          // If the channel was not granted, queue back the allocation,
          // without calling the MAC for a new slot
          auto slotAllocCopy = m_currSlotAllocInfo;
          auto newSfnSf = slotAllocCopy.m_sfnSf;
          newSfnSf.Add (1);
          NS_LOG_INFO ("Queueing allocation in front for " << newSfnSf );
          if (m_currSlotAllocInfo.ContainsDataAllocation ())
            {
              NS_LOG_INFO ("Reason: Current slot allocation has data");
            }
          else
            {
              NS_LOG_INFO ("Reason: CTRL message list is not empty");
            }

          PushFrontSlotAllocInfo (newSfnSf, slotAllocCopy);
        }
      else
        {
          // It's an empty slot; ask the MAC for a new one (maybe a new data will arrive..)
          // and just let the current one go away
          NS_LOG_INFO ("Empty slot, but asking MAC for SlotIndication for the future, maybe there will be data");
          CallMacForSlotIndication (m_currentSlot);
        }
      // If we have the UL CTRL, then schedule it (we are listening, so
      // we don't need the channel.

      if (m_currSlotAllocInfo.m_varTtiAllocInfo.size() > 0)
        {
          for (const auto & alloc : m_currSlotAllocInfo.m_varTtiAllocInfo)
            {
              if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL && alloc.m_dci->m_format == DciInfoElementTdma::UL)
                {
                  Time start = GetSymbolPeriod () * alloc.m_dci->m_symStart;
                  NS_LOG_INFO ("Schedule UL CTRL at " << start);
                  Simulator::Schedule (start, &NrGnbPhy::UlCtrl, this, alloc.m_dci);
                }
              else if (alloc.m_dci->m_type == DciInfoElementTdma::SRS && alloc.m_dci->m_format == DciInfoElementTdma::UL)
                {
                  Time start = GetSymbolPeriod () * alloc.m_dci->m_symStart;
                  NS_LOG_INFO ("Schedule UL SRS at " << start);
                  Simulator::Schedule (start, &NrGnbPhy::UlSrs, this, alloc.m_dci);
                }
            }
        }
    }
}


void
NrGnbPhy::DoCheckOrReleaseChannel ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_channelStatus == GRANTED);
  // The channel is granted, we have to check if we maintain it for the next
  // slot or we have to release it.

  // Assuming the scheduler assign contiguos symbol
  uint8_t lastDlSymbol = 0;
  for (auto & dci : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
      if (dci.m_dci->m_type == DciInfoElementTdma::DATA && dci.m_dci->m_format == DciInfoElementTdma::DL)
        {
          lastDlSymbol = std::max (lastDlSymbol,
                                   static_cast<uint8_t> (dci.m_dci->m_symStart + dci.m_dci->m_numSym));
        }
    }

  Time lastDataTime = GetSymbolPeriod () * lastDlSymbol;

  if (GetSlotPeriod () - lastDataTime > MicroSeconds (25))
    {
      NS_LOG_LOGIC ("Last symbol of data: " << +lastDlSymbol << ", to the end of slot we still have " <<
                   (GetSlotPeriod () - lastDataTime).GetMicroSeconds() <<
                   " us, so we're going to lose the channel");
      m_channelStatus = TO_LOSE;
    }
  else
    {
      NS_LOG_LOGIC ("Last symbol of data: " << +lastDlSymbol << ", to the end of slot we still have " <<
                   (GetSlotPeriod () - lastDataTime).GetMicroSeconds() <<
                   " us, so we're NOT going to lose the channel");
    }
}

void
NrGnbPhy::RetrievePrepareEncodeCtrlMsgs ()
{
  NS_LOG_FUNCTION (this);
  auto ctrlMsgs = PopCurrentSlotCtrlMsgs ();
  ctrlMsgs.merge (RetrieveMsgsFromDCIs (m_currentSlot));

  if (m_netDevice != nullptr)
    {
      DynamicCast<NrGnbNetDevice> (m_netDevice)->RouteOutgoingCtrlMsgs (ctrlMsgs, GetBwpId ());
    }
  else
    {
      // No netDevice (that could happen in tests) so just redirect them to us
      for (const auto & msg : ctrlMsgs)
        {
          EncodeCtrlMsg (msg);
        }
    }
}

void
NrGnbPhy::GenerateAllocationStatistics (const SlotAllocInfo &allocInfo) const
{
  NS_LOG_FUNCTION (this);
  std::unordered_set<uint16_t> activeUe;
  uint32_t availRb = GetRbNum ();
  uint32_t dataReg = 0;
  uint32_t ctrlReg = 0;
  uint32_t dataSym = 0;
  uint32_t ctrlSym = 0;

  int lastSymStart = -1;
  uint32_t symUsed = 0;

  for (const auto & allocation : allocInfo.m_varTtiAllocInfo)
    {
      uint32_t rbg = std::count (allocation.m_dci->m_rbgBitmask.begin (),
                                 allocation.m_dci->m_rbgBitmask.end (), 1);

      // First: Store the RNTI of the UE in the active list
      if (allocation.m_dci->m_rnti != 0)
        {
          activeUe.insert (allocation.m_dci->m_rnti);
        }

      NS_ASSERT (lastSymStart <= allocation.m_dci->m_symStart);

      auto rbgUsed = (rbg * GetNumRbPerRbg ()) * allocation.m_dci->m_numSym;
      if (allocation.m_dci->m_type == DciInfoElementTdma::DATA)
        {
          dataReg += rbgUsed;
        }
      else
        {
          ctrlReg += rbgUsed;
        }

      if (lastSymStart != allocation.m_dci->m_symStart)
        {
          symUsed += allocation.m_dci->m_numSym;

          if (allocation.m_dci->m_type == DciInfoElementTdma::DATA)
            {
              dataSym += allocation.m_dci->m_numSym;
            }
          else
            {
              ctrlSym += allocation.m_dci->m_numSym;
            }
        }

      lastSymStart = allocation.m_dci->m_symStart;
    }

  NS_ASSERT_MSG (symUsed == allocInfo.m_numSymAlloc,
                 "Allocated " << +allocInfo.m_numSymAlloc << " but only " << symUsed << " written in stats");

  m_phySlotDataStats (allocInfo.m_sfnSf, activeUe.size (), dataReg, dataSym,
                      availRb, GetSymbolsPerSlot () - ctrlSym, GetBwpId (), GetCellId ());
  m_phySlotCtrlStats (allocInfo.m_sfnSf, activeUe.size (), ctrlReg, ctrlSym,
                      availRb, GetSymbolsPerSlot () - dataSym, GetBwpId (), GetCellId ());
}

void
NrGnbPhy::DoStartSlot ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_ctrlMsgs.size () == 0); // This assert has to be re-evaluated for NR-U.
                                       // We can have messages before we weren't able to tx them before.

  uint64_t currentSlotN = m_currentSlot.Normalize () % m_tddPattern.size ();;

  NS_LOG_DEBUG ("Start Slot " << m_currentSlot << " of type " << m_tddPattern[currentSlotN]);

  GenerateAllocationStatistics (m_currSlotAllocInfo);

  if (m_currSlotAllocInfo.m_varTtiAllocInfo.size () == 0)
    {
      return;
    }

  NS_LOG_INFO ("Allocations of the current slot: " << std::endl << m_currSlotAllocInfo);

  DoCheckOrReleaseChannel ();

  RetrievePrepareEncodeCtrlMsgs ();

  PrepareRbgAllocationMap (m_currSlotAllocInfo.m_varTtiAllocInfo);

  FillTheEvent ();
}

void
NrGnbPhy::PrepareRbgAllocationMap (const std::deque<VarTtiAllocInfo> &allocations)
{
  NS_LOG_FUNCTION (this);

  // Start with a clean RBG allocation bitmask
  m_rbgAllocationPerSym.clear ();

  // Create RBG map to know where to put power in DL
  for (const auto & allocation : allocations)
    {
      if (allocation.m_dci->m_type != DciInfoElementTdma::CTRL)
        {
          if (allocation.m_dci->m_format == DciInfoElementTdma::DL)
            {
              // In m_rbgAllocationPerSym, store only the DL RBG set to 1:
              // these will used to put power
              StoreRBGAllocation (&m_rbgAllocationPerSym, allocation.m_dci);
            }

          // For statistics, store UL/DL allocations
          StoreRBGAllocation (&m_rbgAllocationPerSymDataStat, allocation.m_dci);
        }
    }

  for (const auto & s : m_rbgAllocationPerSymDataStat)
    {
      auto & rbgAllocation = s.second;
      m_rbStatistics (m_currentSlot, s.first, FromRBGBitmaskToRBAssignment (rbgAllocation),
                      GetBwpId (), GetCellId ());
    }

  m_rbgAllocationPerSymDataStat.clear ();
}

void
NrGnbPhy::FillTheEvent ()
{
  NS_LOG_FUNCTION (this);

  uint8_t lastSymStart = 0;
  bool useNextAllocationSameSymbol = true;
  for (const auto & allocation : m_currSlotAllocInfo.m_varTtiAllocInfo)
    {
      NS_ASSERT (lastSymStart <= allocation.m_dci->m_symStart);

      if (lastSymStart == allocation.m_dci->m_symStart && !useNextAllocationSameSymbol)
        {
          NS_LOG_INFO ("Ignored allocation " << *(allocation.m_dci) << " for OFDMA DL trick");
          continue;
        }
      else
        {
          useNextAllocationSameSymbol = true;
        }

      auto varTtiStart = GetSymbolPeriod () * allocation.m_dci->m_symStart;
      Simulator::Schedule (varTtiStart, &NrGnbPhy::StartVarTti, this, allocation.m_dci);
      lastSymStart = allocation.m_dci->m_symStart;

      // If the allocation is DL, then don't schedule anything that is in the
      // same symbol (see OFDMA DL trick documentation)
      if (allocation.m_dci->m_format == DciInfoElementTdma::DL)
        {
          useNextAllocationSameSymbol = false;
        }

      NS_LOG_INFO ("Scheduled allocation " << *(allocation.m_dci) << " at " << varTtiStart);
    }

  m_currSlotAllocInfo.m_varTtiAllocInfo.clear ();
}

void
NrGnbPhy::StoreRBGAllocation (std::unordered_map<uint8_t, std::vector<uint8_t> > *map,
                              const std::shared_ptr<DciInfoElementTdma> &dci) const
{
  NS_LOG_FUNCTION (this);

  auto itAlloc = map->find (dci->m_symStart);
  if (itAlloc == map->end ())
    {
      itAlloc = map->insert (std::make_pair (dci->m_symStart, dci->m_rbgBitmask)).first;
    }
  else
    {
      auto & existingRBGBitmask = itAlloc->second;
      NS_ASSERT (existingRBGBitmask.size () == dci->m_rbgBitmask.size ());
      for (uint32_t i = 0; i < existingRBGBitmask.size (); ++i)
        {
          existingRBGBitmask.at (i) = existingRBGBitmask.at (i) | dci->m_rbgBitmask.at (i);
        }
    }
}

std::list <Ptr<NrControlMessage>>
NrGnbPhy::RetrieveDciFromAllocation (const SlotAllocInfo &alloc,
                                         const DciInfoElementTdma::DciFormat &format,
                                         uint32_t kDelay, uint32_t k1Delay)
{
  NS_LOG_FUNCTION(this);
  std::list <Ptr<NrControlMessage>> ctrlMsgs;

  for (const auto & dlAlloc : alloc.m_varTtiAllocInfo)
    {
      if (dlAlloc.m_dci->m_type != DciInfoElementTdma::CTRL && dlAlloc.m_dci->m_format == format)
        {
          auto & dciElem = dlAlloc.m_dci;
          NS_ASSERT (dciElem->m_format == format);
          NS_ASSERT_MSG (dciElem->m_symStart + dciElem->m_numSym <= GetSymbolsPerSlot (),
                         "symStart: " << static_cast<uint32_t> (dciElem->m_symStart) <<
                         " numSym: " << static_cast<uint32_t> (dciElem->m_numSym) <<
                         " symPerSlot: " << static_cast<uint32_t> (GetSymbolsPerSlot ()));

          NS_LOG_INFO ("Send DCI to " << dciElem->m_rnti << " from sym " <<
                         +dciElem->m_symStart << " to " << +dciElem->m_symStart + dciElem->m_numSym);

          Ptr<NrControlMessage> msg;

          if (dciElem->m_format == DciInfoElementTdma::DL)
            {
              Ptr<NrDlDciMessage> dciMsg = Create<NrDlDciMessage> (dciElem);

              dciMsg->SetSourceBwp (GetBwpId ());
              dciMsg->SetKDelay (kDelay);
              dciMsg->SetK1Delay (k1Delay);
              msg = dciMsg;
            }
          else
            {
              Ptr<NrUlDciMessage> dciMsg = Create<NrUlDciMessage> (dciElem);

              dciMsg->SetSourceBwp (GetBwpId ());
              dciMsg->SetKDelay (kDelay);
              msg = dciMsg;
            }

          ctrlMsgs.push_back (msg);
        }
    }

  return ctrlMsgs;
}

std::list <Ptr<NrControlMessage> >
NrGnbPhy::RetrieveMsgsFromDCIs (const SfnSf &currentSlot)
{
  std::list <Ptr<NrControlMessage> > ctrlMsgs;
  uint64_t currentSlotN = currentSlot.Normalize () % m_tddPattern.size ();

  uint32_t k1delay = m_dlHarqfbPosition[currentSlotN];

  // TODO: copy paste :(
  for (const auto & k0delay : m_toSendDl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;

      targetSlot.Add (k0delay);

      if (targetSlot == currentSlot)
        {
          NS_LOG_INFO (" in slot " << currentSlot << " send DL DCI for the same slot");

          ctrlMsgs.merge (RetrieveDciFromAllocation (m_currSlotAllocInfo,
                                                     DciInfoElementTdma::DL, k0delay, k1delay));
        }
      else if (SlotAllocInfoExists (targetSlot))
        {
          NS_LOG_INFO (" in slot " << currentSlot << " send DL DCI for " <<
                         targetSlot);

          ctrlMsgs.merge (RetrieveDciFromAllocation (PeekSlotAllocInfo(targetSlot),
                                                     DciInfoElementTdma::DL, k0delay, k1delay));
        }
      else
        {
          NS_LOG_INFO ("No allocation found for slot " << targetSlot);
        }
    }

  for (const auto & k2delay : m_toSendUl[currentSlotN])
    {
      SfnSf targetSlot = currentSlot;

      targetSlot.Add (k2delay);

      if (targetSlot == currentSlot)
        {
          NS_LOG_INFO (" in slot " << currentSlot << " send UL DCI for the same slot");

          ctrlMsgs.merge (RetrieveDciFromAllocation (m_currSlotAllocInfo,
                                                     DciInfoElementTdma::UL, k2delay, k1delay));
        }
      else if (SlotAllocInfoExists (targetSlot))
        {
          NS_LOG_INFO (" in slot " << currentSlot << " send UL DCI for " <<
                         targetSlot);

          ctrlMsgs.merge (RetrieveDciFromAllocation (PeekSlotAllocInfo(targetSlot),
                                                     DciInfoElementTdma::UL, k2delay, k1delay));
        }
      else
        {
          NS_LOG_INFO ("No allocation found for slot " << targetSlot);
        }
    }

  return ctrlMsgs;
}

Time
NrGnbPhy::DlCtrl (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("Starting DL CTRL TTI at symbol " << +m_currSymStart <<
                " to " << +m_currSymStart + dci->m_numSym);

  // TX control period
  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  // The function that is filling m_ctrlMsgs is NrPhy::encodeCtrlMsgs
  if (m_ctrlMsgs.size () > 0)
    {
      NS_LOG_INFO ("ENB TXing DL CTRL with " << m_ctrlMsgs.size () << " msgs, frame " << m_currentSlot <<
                    " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                    "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                    " start " << Simulator::Now () <<
                    " end " << Simulator::Now () + varTtiPeriod - NanoSeconds (1.0));

      for (auto ctrlIt = m_ctrlMsgs.begin (); ctrlIt != m_ctrlMsgs.end (); ++ctrlIt)
        {
          Ptr<NrControlMessage> msg = (*ctrlIt);
          m_phyTxedCtrlMsgsTrace (m_currentSlot, GetCellId (), dci->m_rnti, GetBwpId (), msg);
        }

      SendCtrlChannels (varTtiPeriod - NanoSeconds (1.0)); // -1 ns ensures control ends before data period
    }
  else
    {
      NS_LOG_INFO ("No messages to send, skipping");
    }

  return varTtiPeriod;
}

Time
NrGnbPhy::UlCtrl(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("Starting UL CTRL TTI at symbol " << +m_currSymStart <<
                " to " << +m_currSymStart + dci->m_numSym);

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  NS_LOG_INFO ("ENB RXng UL CTRL frame " << m_currentSlot <<
                " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                " start " << Simulator::Now () <<
                " end " << Simulator::Now () + varTtiPeriod);
  return varTtiPeriod;
}

Time
NrGnbPhy::DlData (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Starting DL DATA TTI at symbol " << +m_currSymStart <<
                " to " << +m_currSymStart + dci->m_numSym);

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  for (uint8_t streamIndex = 0; streamIndex < m_spectrumPhys.size(); streamIndex++)
    {
      Ptr<PacketBurst> pktBurst = GetPacketBurst (m_currentSlot, dci->m_symStart, streamIndex);
      if (!pktBurst || pktBurst->GetNPackets () == 0)
        {
          // sometimes the UE will be scheduled when no data is queued.
          // In this case, don't send anything, don't put power... don't do nothing!
          // go to the next stream if any.
          continue;
        }

      NS_LOG_INFO ("ENB TXing DL DATA frame " << m_currentSlot <<
                    " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                    "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                    " start " << Simulator::Now () + NanoSeconds (1) <<
                    " end " << Simulator::Now () + varTtiPeriod - NanoSeconds (2.0));

      Simulator::Schedule (NanoSeconds (1.0), &NrGnbPhy::SendDataChannels, this,
                           pktBurst, varTtiPeriod - NanoSeconds (2.0), dci, streamIndex);
    }

  return varTtiPeriod;
}

Time
NrGnbPhy::UlData(const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("Starting UL DATA TTI at symbol " << +m_currSymStart <<
                " to " << +m_currSymStart + dci->m_numSym);

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  // We do not support MIMO in UL yet. DCI is prepared for stream 0. So, if there is
  // only one stream, do we need this for loop?
  uint8_t streamIndex = 0;

  if (dci->m_tbSize.at (streamIndex) > 0)
    {
      m_spectrumPhys.at(streamIndex)->AddExpectedTb (dci->m_rnti, dci->m_ndi.at (streamIndex),
                                                        dci->m_tbSize.at (streamIndex),
                                                        dci->m_mcs.at (streamIndex),
                                                        FromRBGBitmaskToRBAssignment (dci->m_rbgBitmask),
                                                        dci->m_harqProcess, dci->m_rv.at (streamIndex), false,
                                                        dci->m_symStart, dci->m_numSym, m_currentSlot);
     }

  bool found = false;
  for (uint8_t i = 0; i < m_deviceMap.size (); i++)
    {
      Ptr<NrUeNetDevice> ueDev = DynamicCast < NrUeNetDevice > (m_deviceMap.at (i));
      uint64_t ueRnti = (DynamicCast<NrUePhy>(ueDev->GetPhy (GetBwpId ())))->GetRnti ();
      if (dci->m_rnti == ueRnti)
        {
          // Even if we change the beamforming vector, we hope that the scheduler
          // has scheduled UEs within the same beam (and, therefore, have the same
          // beamforming vector)
          ChangeBeamformingVector (m_deviceMap.at (i)); //assume the control signal is omni
          found = true;
          break;
        }
    }
  NS_ASSERT (found);

  NS_LOG_INFO ("GNB RXing UL DATA frame " << m_currentSlot <<
                " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                " start " << Simulator::Now () <<
                " end " << Simulator::Now () + varTtiPeriod);
  return varTtiPeriod;
}


void
NrGnbPhy::ChangeBeamformingVector (Ptr<NetDevice> dev)
{
  for (uint8_t streamIndex = 0; streamIndex < m_spectrumPhys.size(); streamIndex++)
    {
      m_spectrumPhys.at (streamIndex)->GetBeamManager ()->ChangeBeamformingVector (dev);
    }
}


void
NrGnbPhy::ChangeToQuasiOmniBeamformingVector ()
{
  for (uint8_t streamIndex = 0; streamIndex < m_spectrumPhys.size(); streamIndex++)
    {
      m_spectrumPhys.at (streamIndex)->GetBeamManager ()->ChangeToQuasiOmniBeamformingVector ();
    }
}

Time
NrGnbPhy::UlSrs (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("Starting UL SRS TTI at symbol " << +m_currSymStart <<
                " to " << +m_currSymStart + dci->m_numSym);

  Time varTtiPeriod = GetSymbolPeriod () * dci->m_numSym;

  for (uint8_t streamIndex = 0; streamIndex < m_spectrumPhys.size(); streamIndex++)
    {
      m_spectrumPhys.at(streamIndex)->AddExpectedSrsRnti (dci->m_rnti);
    }

  bool found = false;
  uint16_t notValidRntiCounter = 0; // count if there are in the list of devices without initialized RNTI (rnti = 0)
                                    // if yes, and the rnti for the current SRS is not found in the list,
                                    // the code will not abort
  for (uint8_t i = 0; i < m_deviceMap.size (); i++)
    {
      Ptr<NrUeNetDevice> ueDev = DynamicCast < NrUeNetDevice > (m_deviceMap.at (i));
      uint64_t ueRnti = (DynamicCast<NrUePhy>(ueDev->GetPhy (0)))->GetRnti ();
      if (ueRnti == 0)
        {
          notValidRntiCounter++;
        }
      if (dci->m_rnti == ueRnti)
        {
          // Even if we change the beamforming vector, we hope that the scheduler
          // has scheduled UEs within the same beam (and, therefore, have the same
          // beamforming vector)
          ChangeBeamformingVector (m_deviceMap.at (i)); //assume the control signal is omni
          found = true;
          break;
        }
    }

  NS_ABORT_MSG_IF (!found && (notValidRntiCounter == 0), "All RNTIs are already set (all UEs received RAR message), "
                                                         "but the RNTI for this SRS was not found");

  if (!found)
    {
      NS_LOG_WARN ("The UE for which is scheduled this SRS does not have yet initialized RNTI. RAR message was not received yet.");
    }

  NS_LOG_INFO ("GNB RXing UL SRS frame " << m_currentSlot <<
                " symbols "  << static_cast<uint32_t> (dci->m_symStart) <<
                "-" << static_cast<uint32_t> (dci->m_symStart + dci->m_numSym - 1) <<
                " start " << Simulator::Now () <<
                " end " << Simulator::Now () + varTtiPeriod);
  return varTtiPeriod;
}

void
NrGnbPhy::StartVarTti (const std::shared_ptr<DciInfoElementTdma> &dci)
{
  NS_LOG_FUNCTION (this);
  ChangeToQuasiOmniBeamformingVector (); //assume the control signal is omni
  m_currSymStart = dci->m_symStart;

  Time varTtiPeriod;

  if (dci->m_type == DciInfoElementTdma::CTRL)
    {
      if (dci->m_format == DciInfoElementTdma::DL)
        {
          varTtiPeriod = DlCtrl (dci);
        }
      else if (dci->m_format == DciInfoElementTdma::UL)
        {
          varTtiPeriod = UlCtrl (dci);
        }
    }
  else if (dci->m_type == DciInfoElementTdma::DATA)
    {
      if (dci->m_format == DciInfoElementTdma::DL)
        {
          varTtiPeriod = DlData (dci);
        }
      else if (dci->m_format == DciInfoElementTdma::UL)
        {
          varTtiPeriod = UlData (dci);
        }
    }
  else if (dci->m_type == DciInfoElementTdma::SRS)
    {
      NS_ASSERT (dci->m_format == DciInfoElementTdma::UL);
      varTtiPeriod = UlSrs (dci);
    }

  Simulator::Schedule (varTtiPeriod, &NrGnbPhy::EndVarTti, this, dci);
}

void
NrGnbPhy::EndVarTti (const std::shared_ptr<DciInfoElementTdma> &lastDci)
{
  NS_LOG_FUNCTION (this << Simulator::Now ().GetSeconds ());

  NS_LOG_DEBUG ("DCI started at symbol " << static_cast<uint32_t> (lastDci->m_symStart) <<
                " which lasted for " << static_cast<uint32_t> (lastDci->m_numSym) <<
                " symbols finished");
}

void
NrGnbPhy::EndSlot (void)
{
  NS_LOG_FUNCTION (this);

  Time slotStart = m_lastSlotStart + GetSlotPeriod () - Simulator::Now ();

  if (m_channelStatus == TO_LOSE)
    {
      NS_LOG_INFO ("Release the channel because we did not have any data to maintain the grant");
      m_channelStatus = NONE;
      m_channelLostTimer.Cancel ();
    }

  NS_LOG_DEBUG ("Slot started at " << m_lastSlotStart << " ended");
  m_currentSlot.Add (1);
  Simulator::Schedule (slotStart, &NrGnbPhy::StartSlot, this, m_currentSlot);
}

void
NrGnbPhy::SendDataChannels (const Ptr<PacketBurst> &pb, const Time &varTtiPeriod,
                            const std::shared_ptr<DciInfoElementTdma> &dci,
                            const uint8_t &streamId)
{
  NS_LOG_FUNCTION (this);
  // update beamforming vectors (currently supports 1 user only)
  bool found = false;
  for (uint8_t i = 0; i < m_deviceMap.size (); i++)
    {
      Ptr<NrUeNetDevice> ueDev = DynamicCast<NrUeNetDevice> (m_deviceMap.at (i));
      uint64_t ueRnti = (DynamicCast<NrUePhy>(ueDev->GetPhy (GetBwpId ())))->GetRnti ();
      //NS_LOG_INFO ("Scheduled rnti:"<<rnti <<" ue rnti:"<< ueRnti);
      if (dci->m_rnti == ueRnti)
        {
          ChangeBeamformingVector(m_deviceMap.at (i));
          found = true;
          break;
        }
    }
  NS_ABORT_IF (!found);


  // in the map we stored the RBG allocated by the MAC for this symbol.
  // If the transmission last n symbol (n > 1 && n < 12) the SetSubChannels
  // doesn't need to be called again. In fact, SendDataChannels will be
  // invoked only when the symStart changes.
  NS_ASSERT (m_rbgAllocationPerSym.find(dci->m_symStart) != m_rbgAllocationPerSym.end ());

  uint8_t activeStreams = 0;
  for (const auto& tbSize : dci->m_tbSize)
    {
      if (tbSize)
        {
          activeStreams++;
        }
    }
  SetSubChannels (FromRBGBitmaskToRBAssignment (m_rbgAllocationPerSym.at (dci->m_symStart)), activeStreams);

  std::list<Ptr<NrControlMessage> > ctrlMsgs;
  m_spectrumPhys.at (streamId)->StartTxDataFrames (pb, ctrlMsgs, varTtiPeriod);
}

void
NrGnbPhy::SendCtrlChannels (const Time &varTtiPeriod)
{
  NS_LOG_FUNCTION (this << "Send Ctrl");

  std::vector <int> fullBwRb (GetRbNum ());
  // The first time set the right values for the phy
  for (uint32_t i = 0; i < fullBwRb.size (); ++i)
    {
      fullBwRb[i] = static_cast<int> (i);
    }

  // Currently all DL CTRL is sent only through one stream
  SetSubChannels (fullBwRb, 1);
  // DL control will be transmitted only through a single stream, we assume that it is the first one
  m_spectrumPhys.at(0)->StartTxDlControlFrames (m_ctrlMsgs, varTtiPeriod);
  m_ctrlMsgs.clear ();
}

bool
NrGnbPhy::RegisterUe (uint64_t imsi, const Ptr<NrUeNetDevice> &ueDevice)
{
  NS_LOG_FUNCTION (this << imsi);
  std::set <uint64_t>::iterator it;
  it = m_ueAttached.find (imsi);

  if (it == m_ueAttached.end ())
    {
      m_ueAttached.insert (imsi);
      m_deviceMap.push_back (ueDevice);
      return (true);
    }
  else
    {
      NS_LOG_ERROR ("Programming error...UE already attached");
      return (false);
    }
}

void
NrGnbPhy::PhyDataPacketReceived (const Ptr<Packet> &p)
{
  Simulator::ScheduleWithContext (m_netDevice->GetNode ()->GetId (),
                                  GetTbDecodeLatency (),
                                  &NrGnbPhySapUser::ReceivePhyPdu,
                                  m_phySapUser,
                                  p);
}

void
NrGnbPhy::GenerateDataCqiReport (const SpectrumValue& sinr, uint8_t streamId) const
{
  NS_LOG_FUNCTION (this << sinr);

  Values::const_iterator it;
  NrMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi;
  ulcqi.m_ulCqi.m_type = UlCqiInfo::PUSCH;
  for (it = sinr.ConstValuesBegin (); it != sinr.ConstValuesEnd (); it++)
    {
      //   double sinrdb = 10 * std::log10 ((*it));
      //       NS_LOG_INFO ("ULCQI RB " << i << " value " << sinrdb);
      // convert from double to fixed point notaltion Sxxxxxxxxxxx.xxx
      //   int16_t sinrFp = LteFfConverter::double2fpS11dot3 (sinrdb);
      ulcqi.m_ulCqi.m_sinr.push_back (*it);  // will be processed by NrMacSchedulerCQIManagement::UlSBCQIReported, it will look into a map of assignment
    }

  // here we use the start symbol index of the var tti in place of the var tti index because the absolute UL var tti index is
  // not known to the scheduler when m_allocationMap gets populated
  ulcqi.m_sfnSf = m_currentSlot;
  ulcqi.m_symStart = m_currSymStart;
  SpectrumValue newSinr = sinr;
  m_ulSinrTrace (0, newSinr, newSinr);
  m_phySapUser->UlCqiReport (ulcqi);
}


void
NrGnbPhy::PhyCtrlMessagesReceived (const Ptr<NrControlMessage> &msg)
{
  NS_LOG_FUNCTION (this);

  if (msg->GetMessageType () == NrControlMessage::DL_CQI)
    {
      Ptr<NrDlCqiMessage> dlcqi = DynamicCast<NrDlCqiMessage> (msg);
      DlCqiInfo dlcqiLE = dlcqi->GetDlCqi ();
      m_phyRxedCtrlMsgsTrace (m_currentSlot, GetCellId (), dlcqiLE.m_rnti, GetBwpId (), msg);

      NS_LOG_INFO ("Received DL_CQI for RNTI: " << dlcqiLE.m_rnti << " in slot " <<
                   m_currentSlot);

      m_phySapUser->ReceiveControlMessage (msg);
    }
  else if (msg->GetMessageType () == NrControlMessage::RACH_PREAMBLE)
    {
      NS_LOG_INFO ("received RACH_PREAMBLE");

      Ptr<NrRachPreambleMessage> rachPreamble = DynamicCast<NrRachPreambleMessage> (msg);
      m_phyRxedCtrlMsgsTrace (m_currentSlot,  GetCellId (), 0, GetBwpId (), msg);
      NS_LOG_INFO ("Received RACH Preamble in slot " << m_currentSlot);
      m_phySapUser->ReceiveRachPreamble (rachPreamble->GetRapId ());

    }
  else if (msg->GetMessageType () == NrControlMessage::DL_HARQ)
    {
      Ptr<NrDlHarqFeedbackMessage> dlharqMsg = DynamicCast<NrDlHarqFeedbackMessage> (msg);
      DlHarqInfo dlharq = dlharqMsg->GetDlHarqFeedback ();
      if (m_ueAttachedRnti.find (dlharq.m_rnti) != m_ueAttachedRnti.end ())
        {
          m_phyRxedCtrlMsgsTrace (m_currentSlot,  GetCellId (), dlharq.m_rnti, GetBwpId (), msg);

          NS_LOG_INFO ("Received DL_HARQ for RNTI: " << dlharq.m_rnti << " in slot " <<
                       m_currentSlot);
          m_phySapUser->ReceiveControlMessage (msg);
        }
    }
  else
    {
      m_phyRxedCtrlMsgsTrace (m_currentSlot,  GetCellId (), 0, GetBwpId (), msg);
      m_phySapUser->ReceiveControlMessage (msg);
    }
}


////////////////////////////////////////////////////////////
/////////                     sap                 /////////
///////////////////////////////////////////////////////////

void
NrGnbPhy::DoSetBandwidth (uint16_t ulBandwidth, uint16_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << +ulBandwidth << +dlBandwidth);
  NS_ASSERT (ulBandwidth == dlBandwidth);
  SetChannelBandwidth (dlBandwidth);
}

void
NrGnbPhy::DoSetEarfcn (uint16_t ulEarfcn, uint16_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << ulEarfcn << dlEarfcn);
}


void
NrGnbPhy::DoAddUe ([[maybe_unused]] uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::set <uint16_t>::iterator it;
  it = m_ueAttachedRnti.find (rnti);
  if (it == m_ueAttachedRnti.end ())
    {
      m_ueAttachedRnti.insert (rnti);
    }
}

void
NrGnbPhy::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);

  std::set <uint16_t>::iterator it = m_ueAttachedRnti.find (rnti);
  if (it != m_ueAttachedRnti.end ())
    {
      m_ueAttachedRnti.erase (it);
    }
  else
    {
      NS_FATAL_ERROR ("Impossible to remove UE, not attached!");
    }
}

void
NrGnbPhy::DoSetPa (uint16_t rnti, double pa)
{
  NS_LOG_FUNCTION (this << rnti << pa);
}

void
NrGnbPhy::DoSetTransmissionMode (uint16_t  rnti, uint8_t txMode)
{
  NS_LOG_FUNCTION (this << rnti << +txMode);
  // UL supports only SISO MODE
}

void
NrGnbPhy::DoSetSrsConfigurationIndex (uint16_t  rnti, uint16_t srcCi)
{
  NS_LOG_FUNCTION (this << rnti << srcCi);
}

void
NrGnbPhy::DoSetMasterInformationBlock ([[maybe_unused]] LteRrcSap::MasterInformationBlock mib)
{
  NS_LOG_FUNCTION (this);
}

void
NrGnbPhy::DoSetSystemInformationBlockType1 (LteRrcSap::SystemInformationBlockType1 sib1)
{
  NS_LOG_FUNCTION (this);
  m_sib1 = sib1;
}

int8_t
NrGnbPhy::DoGetReferenceSignalPower () const
{
  NS_LOG_FUNCTION (this);
  return static_cast<int8_t> (m_txPower);
}

void
NrGnbPhy::SetPhySapUser (NrGnbPhySapUser* ptr)
{
  m_phySapUser = ptr;
}

void
NrGnbPhy::ReportUlHarqFeedback (const UlHarqInfo &mes)
{
  NS_LOG_FUNCTION (this);
  // forward to scheduler
  if (m_ueAttachedRnti.find (mes.m_rnti) != m_ueAttachedRnti.end ())
    {
      NS_LOG_INFO ("Received UL HARQ feedback " << mes.IsReceivedOk() <<
                   " and forwarding to the scheduler");
      m_phySapUser->UlHarqFeedback (mes);
    }
}

void
NrGnbPhy::SetPattern (const std::string &pattern)
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
       if (lookupTable.find (v) == lookupTable.end())
         {
           NS_FATAL_ERROR ("Pattern type " << v << " not valid. Valid values are: DL UL F S");
         }
       vector.push_back (lookupTable[v]);
     }

   SetTddPattern (vector);
}

std::string
NrGnbPhy::GetPattern () const
{
  return NrPhy::GetPattern (m_tddPattern);
}

void
NrGnbPhy::SetPrimary ()
{
  NS_LOG_FUNCTION (this);
  m_isPrimary = true;
}

void
NrGnbPhy::ChannelAccessGranted (const Time &time)
{
  NS_LOG_FUNCTION (this);

  if (time < GetSlotPeriod ())
    {
      NS_LOG_INFO ("Channel granted for less than the slot time. Ignoring the grant.");
      m_channelStatus = NONE;
      return;
    }

  m_channelStatus = GRANTED;

  Time toNextSlot = m_lastSlotStart + GetSlotPeriod () - Simulator::Now ();
  Time grant = time - toNextSlot;
  int64_t slotGranted = grant.GetNanoSeconds () / GetSlotPeriod().GetNanoSeconds ();

  NS_LOG_INFO ("Channel access granted for " << time.GetMilliSeconds () <<
               " ms, which corresponds to " << slotGranted << " slot in which each slot is " <<
               GetSlotPeriod() << " ms. We lost " <<
               toNextSlot.GetMilliSeconds() << " ms. ");
  NS_ASSERT(! m_channelLostTimer.IsRunning ());

  if (slotGranted < 1)
    {
      slotGranted = 1;
    }
  m_channelLostTimer = Simulator::Schedule (GetSlotPeriod () * slotGranted - NanoSeconds (1),
                                            &NrGnbPhy::ChannelAccessLost, this);
}

void
NrGnbPhy::ChannelAccessLost ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Channel access lost");
  m_channelStatus = NONE;
}

}

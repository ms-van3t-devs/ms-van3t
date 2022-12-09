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

#define NS_LOG_APPEND_CONTEXT                                            \
  do                                                                     \
    {                                                                    \
      std::clog << " [ CellId " << GetCellId() << ", bwpId "             \
                << GetBwpId () << "] ";                                  \
    }                                                                    \
  while (false);

#include "nr-phy.h"
#include "nr-spectrum-phy.h"
#include "nr-net-device.h"
#include "nr-ue-net-device.h"
#include "nr-gnb-net-device.h"
#include "beam-manager.h"
#include "ns3/uniform-planar-array.h"
#include <ns3/boolean.h>

#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrPhy");

NS_OBJECT_ENSURE_REGISTERED ( NrPhy);

/*   SAP   */
class NrMemberPhySapProvider : public NrPhySapProvider
{
public:
  NrMemberPhySapProvider (NrPhy* phy);

  virtual void SendMacPdu (const Ptr<Packet> &p, const SfnSf & sfn, uint8_t symStart) override;

  virtual void SendControlMessage (Ptr<NrControlMessage> msg) override;

  virtual void SendRachPreamble (uint8_t PreambleId, uint8_t Rnti) override;

  virtual void SetSlotAllocInfo (const SlotAllocInfo &slotAllocInfo) override;

  virtual BeamId GetBeamId (uint8_t rnti) const override;

  virtual Ptr<const SpectrumModel> GetSpectrumModel () override;

  virtual void NotifyConnectionSuccessful () override;

  virtual uint16_t GetBwpId () const override;

  virtual uint16_t GetCellId () const override;

  virtual uint32_t GetSymbolsPerSlot () const override;

  virtual Time GetSlotPeriod () const override;

  virtual uint32_t GetRbNum () const override;

private:
  NrPhy* m_phy;
};

NrMemberPhySapProvider::NrMemberPhySapProvider (NrPhy* phy)
  : m_phy (phy)
{
  //  Nothing more to do
}

void
NrMemberPhySapProvider::SendMacPdu (const Ptr<Packet> &p, const SfnSf & sfn, uint8_t symStart)
{
  m_phy->SetMacPdu (p, sfn, symStart);
}

void
NrMemberPhySapProvider::SendControlMessage (Ptr<NrControlMessage> msg)
{
  m_phy->EnqueueCtrlMessage (msg);  //May need to change
}

void
NrMemberPhySapProvider::SendRachPreamble (uint8_t PreambleId, uint8_t Rnti)
{
  m_phy->SendRachPreamble (PreambleId, Rnti);
}

void
NrMemberPhySapProvider::SetSlotAllocInfo (const SlotAllocInfo &slotAllocInfo)
{
  m_phy->PushBackSlotAllocInfo (slotAllocInfo);
}

BeamId
NrMemberPhySapProvider::GetBeamId (uint8_t rnti) const
{
  return m_phy->GetBeamId (rnti);
}

Ptr<const SpectrumModel>
NrMemberPhySapProvider::GetSpectrumModel ()
{
  return m_phy->GetSpectrumModel ();
}

void
NrMemberPhySapProvider::NotifyConnectionSuccessful ()
{
  m_phy->NotifyConnectionSuccessful ();
}

uint16_t
NrMemberPhySapProvider::GetBwpId () const
{
  return m_phy->GetBwpId ();
}

uint16_t
NrMemberPhySapProvider::GetCellId () const
{
  return m_phy->GetCellId ();
}

uint32_t
NrMemberPhySapProvider::GetSymbolsPerSlot () const
{
  return m_phy->GetSymbolsPerSlot ();
}

Time
NrMemberPhySapProvider::GetSlotPeriod () const
{
  return m_phy->GetSlotPeriod ();
}

uint32_t
NrMemberPhySapProvider::GetRbNum () const
{
  return m_phy->GetRbNum ();
}

/* ======= */

TypeId
NrPhy::GetTypeId ()
{
  static TypeId
    tid =
    TypeId ("ns3::NrPhy")
    .SetParent<Object> ()
  ;

  return tid;
}

std::vector<int>
NrPhy::FromRBGBitmaskToRBAssignment (const std::vector<uint8_t> rbgBitmask) const
{
  std::vector<int> ret;

  for (uint32_t i = 0; i < rbgBitmask.size (); ++i)
    {
      if (rbgBitmask.at (i) == 1)
        {
          for (uint32_t k = 0; k < GetNumRbPerRbg (); ++k)
            {
              ret.push_back ((i * GetNumRbPerRbg ()) + k);
            }
        }
    }

  NS_ASSERT (static_cast<uint32_t> (std::count (rbgBitmask.begin (), rbgBitmask.end (), 1) * GetNumRbPerRbg ())
             == ret.size ());
  return ret;
}

NrPhy::NrPhy ()
  : m_currSlotAllocInfo (SfnSf (0,0,0,0)),
    m_tbDecodeLatencyUs (100.0)
{
  NS_LOG_FUNCTION (this);
  m_phySapProvider = new NrMemberPhySapProvider (this);
  m_nrSlUePhySapProvider = new MemberNrSlUePhySapProvider<NrPhy> (this);
}

NrPhy::~NrPhy ()
{
  NS_LOG_FUNCTION (this);
}

void
NrPhy::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_slotAllocInfo.clear ();
  m_controlMessageQueue.clear ();
  m_packetBurstMap.clear();
  m_ctrlMsgs.clear ();
  m_tddPattern.clear ();
  m_netDevice = nullptr;
  m_beamManager = nullptr;
  if (m_spectrumPhy)
    {
      m_spectrumPhy->Dispose ();
    }
  m_spectrumPhy = nullptr;
  delete m_phySapProvider;
  delete m_nrSlUePhySapProvider;
}

void
NrPhy::InstallAntenna (const Ptr<BeamManager> beamManager, const Ptr<UniformPlanarArray> &antenna)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_spectrumPhy != nullptr);
  m_beamManager = beamManager;
  m_beamManager->Configure(antenna);
}

void
NrPhy::SetDevice (Ptr<NrNetDevice> d)
{
  NS_LOG_FUNCTION (this);
  m_netDevice = d;
}

void
NrPhy::InstallCentralFrequency (double f)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (m_centralFrequency >= 0.0);
  m_centralFrequency = f;
}

void
NrPhy::SetChannelBandwidth (uint16_t channelBandwidth)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("SetChannelBandwidth called with channel bandwidth value: "<< channelBandwidth * 100 * 1000 << "Hz, "
                "and the previous value of channel bandwidth was: " << GetChannelBandwidth () << " Hz");

  if (m_channelBandwidth != channelBandwidth)
    {
      m_channelBandwidth = channelBandwidth;
      // number of RB and noise PSD must be updated when bandwidth or numerology gets changed
      DoUpdateRbNum ();
    }
}

void
NrPhy::SetNumerology (uint16_t numerology)
{
  NS_LOG_FUNCTION (this);
  m_numerology = numerology;
  m_slotsPerSubframe  = static_cast<uint16_t> (std::pow (2, numerology));
  m_slotPeriod = Seconds (0.001 / m_slotsPerSubframe);
  m_subcarrierSpacing = 15000 * static_cast<uint32_t> (std::pow (2, numerology));
  m_symbolPeriod = (m_slotPeriod / m_symbolsPerSlot);

  // number of RB and noise PSD must be updated when bandwidth or numerology gets changed
  if (m_channelBandwidth != 0)
    {
      DoUpdateRbNum ();

      NS_LOG_INFO (" Numerology configured:" << GetNumerology () <<
                   " slots per subframe: " << m_slotsPerSubframe <<
                   " slot period:" << GetSlotPeriod () <<
                   " symbol period:" << GetSymbolPeriod () <<
                   " subcarrier spacing: " << GetSubcarrierSpacing () <<
                   " number of RBs: " << GetRbNum () );
    }
  else
    {
      NS_LOG_INFO ("Numerology is set, but the channel bandwidth not yet, so the number of RBs cannot be updated now.");
    }
}

uint16_t
NrPhy::GetNumerology() const
{
  return m_numerology;
}

void
NrPhy::SetSymbolsPerSlot (uint16_t symbolsPerSlot)
{
  NS_LOG_FUNCTION (this);
  m_symbolsPerSlot = symbolsPerSlot;
  m_symbolPeriod = (m_slotPeriod / m_symbolsPerSlot);
}

void NrPhy::SetRbOverhead (double oh)
{
  m_rbOh = oh;
}

double NrPhy::GetRbOverhead() const
{
  return m_rbOh;
}

uint32_t
NrPhy::GetSymbolsPerSlot () const
{
  return m_symbolsPerSlot;
}

Time
NrPhy::GetSlotPeriod () const
{
  NS_ABORT_IF (m_slotPeriod.IsNegative ());
  return m_slotPeriod;
}

void
NrPhy::DoSetCellId (uint16_t cellId)
{
  NS_LOG_FUNCTION (this);
  m_cellId = cellId;
}

void
NrPhy::SendRachPreamble (uint32_t PreambleId, uint32_t Rnti)
{
  NS_LOG_FUNCTION (this);
  m_raPreambleId = PreambleId;
  Ptr<NrRachPreambleMessage> msg = Create<NrRachPreambleMessage> ();
  msg->SetSourceBwp (GetBwpId ());
  msg->SetRapId (PreambleId);
  EnqueueCtrlMsgNow (msg);
}

void
NrPhy::SetMacPdu (const Ptr<Packet> &p, const SfnSf & sfn, uint8_t symStart)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (sfn.GetNumerology () == GetNumerology());
  uint64_t key = sfn.GetEncodingWithSymStart (symStart);
  auto it = m_packetBurstMap.find (key);

  if (it == m_packetBurstMap.end ())
    {
      it = m_packetBurstMap.insert (std::make_pair (key, CreateObject<PacketBurst> ())).first;
    }
  it->second->AddPacket (p);
  NS_LOG_INFO ("Adding a packet for the Packet Burst of " << sfn <<
               " at sym " << +symStart << std::endl);
}

void
NrPhy::NotifyConnectionSuccessful ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<PacketBurst>
NrPhy::GetPacketBurst (SfnSf sfn, uint8_t sym)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (sfn.GetNumerology () == GetNumerology());
  Ptr<PacketBurst> pburst;
  auto it = m_packetBurstMap.find (sfn.GetEncodingWithSymStart (sym));

  if (it == m_packetBurstMap.end ())
    {
      // For instance, this can happen with low BW and low MCS: The MAC
      // ignores the txOpportunity.
      NS_LOG_WARN ("Packet burst not found for " << sfn << " at sym " << +sym);
      return pburst;
    }
  else
    {
      pburst = it->second;
      m_packetBurstMap.erase (it);
    }
  return pburst;
}

Ptr<SpectrumValue>
NrPhy::GetNoisePowerSpectralDensity ()
{
  return NrSpectrumValueHelper::CreateNoisePowerSpectralDensity (m_noiseFigure, GetSpectrumModel ());
}

Ptr<SpectrumValue>
NrPhy::GetTxPowerSpectralDensity (const std::vector<int> &rbIndexVector)
{
  Ptr<const SpectrumModel> sm = GetSpectrumModel ();

  return NrSpectrumValueHelper::CreateTxPowerSpectralDensity (m_txPower, rbIndexVector, sm, m_powerAllocationType );
}

double
NrPhy::GetCentralFrequency() const
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (m_centralFrequency < 0.0);
  return m_centralFrequency;
}

std::string
NrPhy::GetPattern (const std::vector<LteNrTddSlotType> &pattern)
{
  static std::unordered_map<LteNrTddSlotType, std::string, std::hash<int>> lookupTable =
  {
    { LteNrTddSlotType::DL, "DL"},
    { LteNrTddSlotType::UL, "UL"},
    { LteNrTddSlotType::S,  "S"},
    { LteNrTddSlotType::F,  "F"}
  };

  std::stringstream ss;

  for (const auto & v : pattern)
    {
      ss << lookupTable[v] << "|";
    }

  return ss.str ();
}


void
NrPhy::SetPowerAllocationType (enum NrSpectrumValueHelper::PowerAllocationType powerAllocationType)
{
  m_powerAllocationType = powerAllocationType;
}

enum NrSpectrumValueHelper::PowerAllocationType
NrPhy::GetPowerAllocationType () const
{
  return m_powerAllocationType;
}

void
NrPhy::EnqueueCtrlMessage (const Ptr<NrControlMessage> &m)
{
  NS_LOG_FUNCTION (this);

  m_controlMessageQueue.at (m_controlMessageQueue.size () - 1).push_back (m);
}

void
NrPhy::EnqueueCtrlMsgNow (const Ptr<NrControlMessage> &msg)
{
  NS_LOG_FUNCTION (this);

  m_controlMessageQueue.at (0).push_back (msg);
}

void
NrPhy::EnqueueCtrlMsgNow (const std::list<Ptr<NrControlMessage> > &listOfMsgs)
{
  for (const auto & msg : listOfMsgs)
    {
      m_controlMessageQueue.at (0).push_back (msg);
    }
}

void
NrPhy::EncodeCtrlMsg (const Ptr<NrControlMessage> &msg)
{
  NS_LOG_FUNCTION (this);
  m_ctrlMsgs.push_back (msg);
}

bool
NrPhy::HasDlSlot () const
{
  return NrPhy::HasDlSlot (m_tddPattern);
}

bool NrPhy::HasUlSlot () const
{
  return NrPhy::HasUlSlot (m_tddPattern);
}

bool
NrPhy::HasDlSlot (const std::vector<LteNrTddSlotType> &pattern)
{
  for (const auto & v : pattern)
    {
      if (v == LteNrTddSlotType::F || v == LteNrTddSlotType::DL)
        {
          return true;
        }
    }
  return false;
}

bool
NrPhy::HasUlSlot (const std::vector<LteNrTddSlotType> &pattern)
{
  for (const auto & v : pattern)
    {
      if (v == LteNrTddSlotType::F || v == LteNrTddSlotType::UL)
        {
          return true;
        }
    }
  return false;
}


uint32_t
NrPhy::GetRbNum () const
{
  return m_rbNum;
}

uint32_t
NrPhy::GetChannelBandwidth () const
{
  // m_channelBandwidth is in kHz * 100
  return m_channelBandwidth * 1000 * 100;
}

uint32_t
NrPhy::GetSubcarrierSpacing () const
{
  return m_subcarrierSpacing;
}

void
NrPhy::DoUpdateRbNum ()
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (m_channelBandwidth == 0, "Channel bandwidth not set");

  double realBw = GetChannelBandwidth () * (1 - m_rbOh);
  uint32_t rbWidth = m_subcarrierSpacing * NrSpectrumValueHelper::SUBCARRIERS_PER_RB;

  NS_ABORT_MSG_IF (rbWidth > realBw, "Bandwidth and numerology not correctly set. Bandwidth after reduction of overhead is :" << realBw <<
                   ", while RB width is: "<< rbWidth);

  m_rbNum = static_cast<uint32_t> (realBw / rbWidth);
  NS_ASSERT (GetRbNum () > 0);

  NS_LOG_INFO ("Updated RbNum to " << GetRbNum ());

  if (m_spectrumPhy)
    {
      // Update the noisePowerSpectralDensity, as it depends on m_rbNum
      m_spectrumPhy->SetNoisePowerSpectralDensity (GetNoisePowerSpectralDensity());

      // once we have set noise power spectral density which will
      // initialize SpectrumModel of our SpectrumPhy, we can
      // call AddRx function of the SpectrumChannel
      if (m_spectrumPhy->GetSpectrumChannel())
        {
          m_spectrumPhy->GetSpectrumChannel()->AddRx (m_spectrumPhy);
        }
      else
        {
          NS_LOG_WARN ("Working without channel (i.e., under test)");
        }
      NS_LOG_INFO ("Noise Power Spectral Density updated");
    }
}

bool
NrPhy::IsTdd (const std::vector<LteNrTddSlotType> &pattern)
{
  bool anUl = false;
  bool aDl = false;

  for (const auto & v : pattern)
    {
      // An F slot: we are TDD
      if (v == LteNrTddSlotType::F)
        {
          return true;
        }

      if (v == LteNrTddSlotType::UL)
        {
          anUl = true;
        }
      else if (v == LteNrTddSlotType::DL)
        {
          aDl = true;
        }
    }

  return ! (anUl ^ aDl);
}

void
NrPhy::InitializeMessageList ()
{
  NS_LOG_FUNCTION (this);
  m_controlMessageQueue.clear ();

  for (unsigned i = 0; i <= GetL1L2CtrlLatency (); i++)
    {
      m_controlMessageQueue.push_back (std::list<Ptr<NrControlMessage> > ());
    }
}


std::list<Ptr<NrControlMessage> >
NrPhy::PopCurrentSlotCtrlMsgs (void)
{
  NS_LOG_FUNCTION (this);
  if (m_controlMessageQueue.empty ())
    {
      std::list<Ptr<NrControlMessage> > emptylist;
      return (emptylist);
    }

  if (m_controlMessageQueue.at (0).size () > 0)
    {
      std::list<Ptr<NrControlMessage> > ret = m_controlMessageQueue.front ();
      m_controlMessageQueue.erase (m_controlMessageQueue.begin ());
      std::list<Ptr<NrControlMessage> > newlist;
      m_controlMessageQueue.push_back (newlist);
      return (ret);
    }
  else
    {
      m_controlMessageQueue.erase (m_controlMessageQueue.begin ());
      std::list<Ptr<NrControlMessage> > newlist;
      m_controlMessageQueue.push_back (newlist);
      std::list<Ptr<NrControlMessage> > emptylist;
      return (emptylist);
    }
}

void
NrPhy::InstallSpectrumPhy (const Ptr<NrSpectrumPhy> &spectrumPhy)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_IF (m_spectrumPhy != nullptr);
  m_spectrumPhy = spectrumPhy;
}

void NrPhy::SetBwpId (uint16_t bwpId)
{
  m_bwpId = bwpId;
}

uint16_t
NrPhy::GetBwpId () const
{
  return m_bwpId;
}

uint16_t
NrPhy::GetCellId () const
{
  return m_cellId;
}

uint32_t
NrPhy::GetL1L2CtrlLatency() const
{
  return 2;
}

Ptr<NrSpectrumPhy>
NrPhy::GetSpectrumPhy () const
{
  return m_spectrumPhy;
}


NrPhySapProvider*
NrPhy::GetPhySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_phySapProvider;
}

void
NrPhy::PushBackSlotAllocInfo (const SlotAllocInfo &slotAllocInfo)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_DEBUG ("setting info for slot " << slotAllocInfo.m_sfnSf);

  // That's not so complex, as the list would typically be of 2 or 3 elements.
  bool updated = false;
  for (auto & alloc : m_slotAllocInfo)
    {
      if (alloc.m_sfnSf == slotAllocInfo.m_sfnSf)
        {
          NS_LOG_INFO ("Merging inside existing allocation");
          alloc.Merge (slotAllocInfo);
          updated = true;
          break;
        }
    }
  if (! updated)
    {
      m_slotAllocInfo.push_back (slotAllocInfo);
      m_slotAllocInfo.sort ();
      NS_LOG_INFO ("Pushing allocation at the end of the list");
    }

  std::stringstream output;

  for (const auto & alloc : m_slotAllocInfo)
    {
      output << alloc;
    }
  NS_LOG_INFO (output.str ());
}

void
NrPhy::PushFrontSlotAllocInfo (const SfnSf &newSfnSf, const SlotAllocInfo &slotAllocInfo)
{
  NS_LOG_FUNCTION (this);

  m_slotAllocInfo.push_front (slotAllocInfo);
  SfnSf currentSfn = newSfnSf;
  std::unordered_map<uint64_t, Ptr<PacketBurst>> newBursts; // map between new sfn and the packet burst
  std::unordered_map<uint64_t, uint64_t> sfnMap; // map between new and old sfn, for debugging

  // all the slot allocations  (and their packet burst) have to be "adjusted":
  // directly modify the sfn for the allocation, and temporarly store the
  // burst (along with the new sfn) into newBursts.
  for (auto it = m_slotAllocInfo.begin (); it != m_slotAllocInfo.end (); ++it)
    {
      auto slotSfn = it->m_sfnSf;
      for (const auto &alloc : it->m_varTtiAllocInfo)
        {
          if (alloc.m_dci->m_type == DciInfoElementTdma::DATA)
            {
              Ptr<PacketBurst> pburst = GetPacketBurst (slotSfn, alloc.m_dci->m_symStart);
              if (pburst && pburst->GetNPackets() > 0)
                {
                  newBursts.insert (std::make_pair (currentSfn.GetEncodingWithSymStart (alloc.m_dci->m_symStart), pburst));
                  sfnMap.insert (std::make_pair (currentSfn.GetEncodingWithSymStart (alloc.m_dci->m_symStart),
                                                 it->m_sfnSf.GetEncodingWithSymStart (alloc.m_dci->m_symStart)));
                }
              else
                {
                  NS_LOG_INFO ("No packet burst found for " << slotSfn);
                }
            }
        }

      NS_LOG_INFO ("Set slot allocation for " << it->m_sfnSf << " to " << currentSfn);
      it->m_sfnSf = currentSfn;
      currentSfn.Add (1);
    }

  for (const auto & burstPair : newBursts)
    {
      SfnSf old, latest;
      old.Decode (sfnMap.at (burstPair.first));
      latest.Decode (burstPair.first);
      m_packetBurstMap.insert (std::make_pair (burstPair.first, burstPair.second));
      NS_LOG_INFO ("PacketBurst with " << burstPair.second->GetNPackets() <<
                   "packets for SFN " << old << " now moved to SFN " << latest);
    }
}


bool
NrPhy::SlotAllocInfoExists (const SfnSf &retVal) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (retVal.GetNumerology () == GetNumerology ());
  for (const auto & alloc : m_slotAllocInfo)
    {
      if (alloc.m_sfnSf == retVal)
        {
          return true;
        }
    }
  return false;
}

SlotAllocInfo
NrPhy::RetrieveSlotAllocInfo ()
{
  NS_LOG_FUNCTION (this);
  SlotAllocInfo ret = *m_slotAllocInfo.begin ();
  m_slotAllocInfo.erase (m_slotAllocInfo.begin ());
  return ret;
}


SlotAllocInfo
NrPhy::RetrieveSlotAllocInfo (const SfnSf &sfnsf)
{
  NS_LOG_FUNCTION (" slot " << sfnsf);
  NS_ASSERT (sfnsf.GetNumerology () == GetNumerology ());

  for (auto allocIt = m_slotAllocInfo.begin(); allocIt != m_slotAllocInfo.end (); ++allocIt)
    {
      if (allocIt->m_sfnSf == sfnsf)
        {
          SlotAllocInfo ret = *allocIt;
          m_slotAllocInfo.erase (allocIt);
          return ret;
        }
    }

  NS_FATAL_ERROR("Didn't found the slot");
  return SlotAllocInfo (sfnsf);
}

SlotAllocInfo &
NrPhy::PeekSlotAllocInfo (const SfnSf &sfnsf)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (sfnsf.GetNumerology () == GetNumerology ());
  for (auto & alloc : m_slotAllocInfo)
    {
      if (alloc.m_sfnSf == sfnsf)
        {
          return alloc;
        }
    }

  NS_FATAL_ERROR ("Didn't found the slot");
}

size_t
NrPhy::SlotAllocInfoSize() const
{
  NS_LOG_FUNCTION (this);
  return m_slotAllocInfo.size ();
}

bool
NrPhy::IsCtrlMsgListEmpty() const
{
  NS_LOG_FUNCTION (this);
  return m_controlMessageQueue.empty () || m_controlMessageQueue.at (0).empty();
}

Ptr<BeamManager> NrPhy::GetBeamManager() const
{
  return m_beamManager;
}

Ptr<const SpectrumModel>
NrPhy::GetSpectrumModel ()
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF (GetSubcarrierSpacing () < 0.0, "Set a valid numerology");
  NS_ABORT_MSG_IF (m_channelBandwidth == 0, "Channel bandwidth not set.");
  return NrSpectrumValueHelper::GetSpectrumModel (GetRbNum (),
                                                  GetCentralFrequency (),
                                                  GetSubcarrierSpacing ());
}

Time
NrPhy::GetSymbolPeriod () const
{
  NS_LOG_FUNCTION (this);
  return m_symbolPeriod;
}

Ptr<const UniformPlanarArray>
NrPhy::GetAntennaArray() const
{
  return m_beamManager->GetAntennaArray ();
}

void
NrPhy::SetNoiseFigure (double d)
{
  m_noiseFigure = d;

  if (m_spectrumPhy && GetRbNum()!=0)
    {
      m_spectrumPhy->SetNoisePowerSpectralDensity (GetNoisePowerSpectralDensity());
    }
}

double
NrPhy::GetNoiseFigure () const
{
  return m_noiseFigure;
}


void
NrPhy::SetTbDecodeLatency (const Time &us)
{
  m_tbDecodeLatencyUs = us;
}

Time
NrPhy::GetTbDecodeLatency (void) const
{
  return m_tbDecodeLatencyUs;
}

//NR Sidelink

NrSlUePhySapProvider*
NrPhy::GetNrSlUePhySapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_nrSlUePhySapProvider;
}

Time
NrPhy::DoGetSlotPeriod () const
{
  return GetSlotPeriod ();
}

uint32_t
NrPhy::DoGetBwInRbs () const
{
 return GetRbNum ();
}

void
NrPhy::DoSendPscchMacPdu (Ptr<Packet> p)
{
  SetPscchMacPdu (p);
}

void
NrPhy::DoSendPsschMacPdu (Ptr<Packet> p)
{
  SetPsschMacPdu (p);
}

void
NrPhy::SetPscchMacPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  //Since we must send one SCI msg at a given time, initially the queue size must
  //be 1 with an empty packet burst
  NS_ASSERT (m_nrSlPscchPacketBurstQueue.size () == 1 && m_nrSlPscchPacketBurstQueue.at (0)->GetNPackets () == 0);
  m_nrSlPscchPacketBurstQueue.at (0)->AddPacket (p);
}

void
NrPhy::SetPsschMacPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  //At a given time, we must send only one packet burst, which mimics a TB.
  //The packets in this packet burst would be equal to the number of LCs
  //multiplexed together plus one SCI format 2 packet
  NS_ASSERT (m_nrSlPsschPacketBurstQueue.size () == 1);
  m_nrSlPsschPacketBurstQueue.at (0)->AddPacket (p);
}

Ptr<PacketBurst>
NrPhy::PopPscchPacketBurst (void)
{
  NS_LOG_FUNCTION (this);
  if (m_nrSlPscchPacketBurstQueue.at (0)->GetSize () > 0)
    {
      //At this stage we assume that PSCCH is not sent without data.
      //Check the doxy of DoSendPscchMacPdu, and DoSendPsschMacPdu.
      NS_ASSERT_MSG (m_nrSlPsschPacketBurstQueue.at (0)->GetSize () > 0,
                     "Currently, PSCCH always goes with data.");

      Ptr<PacketBurst> ret = m_nrSlPscchPacketBurstQueue.at (0)->Copy ();
      m_nrSlPscchPacketBurstQueue.erase (m_nrSlPscchPacketBurstQueue.begin ());
      m_nrSlPscchPacketBurstQueue.push_back (CreateObject <PacketBurst> ());
      return (ret);
    }
  else
    {
      m_nrSlPscchPacketBurstQueue.erase (m_nrSlPscchPacketBurstQueue.begin ());
      m_nrSlPscchPacketBurstQueue.push_back (CreateObject <PacketBurst> ());
      return (0);
    }
}

Ptr<PacketBurst>
NrPhy::PopPsschPacketBurst (void)
{
  NS_LOG_FUNCTION (this);
  if (m_nrSlPsschPacketBurstQueue.at (0)->GetSize () > 0)
    {
      Ptr<PacketBurst> ret = m_nrSlPsschPacketBurstQueue.at (0)->Copy ();
      m_nrSlPsschPacketBurstQueue.erase (m_nrSlPsschPacketBurstQueue.begin ());
      m_nrSlPsschPacketBurstQueue.push_back (CreateObject <PacketBurst> ());
      return (ret);
    }
  else
    {
      m_nrSlPsschPacketBurstQueue.erase (m_nrSlPsschPacketBurstQueue.begin ());
      m_nrSlPsschPacketBurstQueue.push_back (CreateObject <PacketBurst> ());
      return (0);
    }
}

void
NrPhy::DoSetNrSlVarTtiAllocInfo (const SfnSf &sfn, const NrSlVarTtiAllocInfo& varTtiInfo)
{
  NS_LOG_FUNCTION (this);
  //only one allocInfo should exist, which would be the slot allocation
  //info for the current slot.
  if (m_nrSlAllocInfoQueue.empty ())
    {
      //new allocation
      NrSlPhySlotAlloc alloc;
      alloc.sfn = sfn;
      //each Var TTI in slvarTtiInfoList list must differ in their symbol start
      bool insertStatus = alloc.slvarTtiInfoList.emplace (varTtiInfo).second;
      NS_ASSERT_MSG (insertStatus, "Insertion failed. A Var TTI starting from symbol " << varTtiInfo.symStart << " already exists");
      m_nrSlAllocInfoQueue.emplace_front (alloc);
    }
  else
    {
      auto it = m_nrSlAllocInfoQueue.begin();
      NS_ASSERT_MSG (it->sfn == sfn, "Allocation queue must contain the allocation info of the same slot");
      //each Var TTI in slvarTtiInfoList list must differ in their symbol start
      bool insertStatus = it->slvarTtiInfoList.emplace (varTtiInfo).second;
      NS_ASSERT_MSG (insertStatus, "Insertion failed. A Var TTI starting from symbol " << varTtiInfo.symStart << " already exists");
    }
}

bool
NrPhy::NrSlSlotAllocInfoExists (const SfnSf &sfn) const
{
  NS_LOG_FUNCTION (this);
  if (m_nrSlPsschPacketBurstQueue.at (0)->GetNPackets () > 0
      && !m_nrSlAllocInfoQueue.empty ())
    {
      return true;
    }

  return false;
}


}

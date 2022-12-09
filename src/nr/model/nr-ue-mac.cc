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
                << GetBwpId () << ", rnti " << m_rnti << "] ";           \
    }                                                                    \
  while (false);

#include "nr-ue-mac.h"
#include <ns3/log.h>
#include <ns3/boolean.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/random-variable-stream.h>
#include <ns3/integer.h>
#include <ns3/double.h>
#include "nr-phy-sap.h"
#include "nr-control-messages.h"
#include "nr-mac-header-vs.h"
#include "nr-mac-short-bsr-ce.h"
#include "nr-sl-ue-mac-csched-sap.h"
#include "nr-sl-ue-mac-harq.h"
#include "nr-sl-sci-f1a-header.h"
#include "nr-sl-sci-f2a-header.h"
#include "nr-sl-mac-pdu-tag.h"
#include "ns3/lte-rlc-tag.h"
#include <algorithm>
#include <bitset>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrUeMac");
NS_OBJECT_ENSURE_REGISTERED (NrUeMac);

uint8_t NrUeMac::g_raPreambleId = 0;

///////////////////////////////////////////////////////////
// SAP forwarders
///////////////////////////////////////////////////////////


class UeMemberNrUeCmacSapProvider : public LteUeCmacSapProvider
{
public:
  UeMemberNrUeCmacSapProvider (NrUeMac* mac);

  // inherited from LteUeCmacSapProvider
  virtual void ConfigureRach (RachConfig rc);
  virtual void StartContentionBasedRandomAccessProcedure ();
  virtual void StartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId, uint8_t prachMask);
  virtual void AddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu);
  virtual void RemoveLc (uint8_t lcId);
  virtual void Reset ();
  virtual void SetRnti (uint16_t rnti);
  virtual void NotifyConnectionSuccessful ();
  virtual void SetImsi (uint64_t imsi);

private:
  NrUeMac* m_mac;
};


UeMemberNrUeCmacSapProvider::UeMemberNrUeCmacSapProvider (NrUeMac* mac)
  : m_mac (mac)
{
}

void
UeMemberNrUeCmacSapProvider::ConfigureRach (RachConfig rc)
{
  m_mac->DoConfigureRach (rc);
}

void
UeMemberNrUeCmacSapProvider::StartContentionBasedRandomAccessProcedure ()
{
  m_mac->DoStartContentionBasedRandomAccessProcedure ();
}

void
UeMemberNrUeCmacSapProvider::StartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t preambleId, uint8_t prachMask)
{
  m_mac->DoStartNonContentionBasedRandomAccessProcedure (rnti, preambleId, prachMask);
}


void
UeMemberNrUeCmacSapProvider::AddLc (uint8_t lcId, LogicalChannelConfig lcConfig, LteMacSapUser* msu)
{
  m_mac->AddLc (lcId, lcConfig, msu);
}

void
UeMemberNrUeCmacSapProvider::RemoveLc (uint8_t lcid)
{
  m_mac->DoRemoveLc (lcid);
}

void
UeMemberNrUeCmacSapProvider::Reset ()
{
  m_mac->DoReset ();
}

void
UeMemberNrUeCmacSapProvider::SetRnti (uint16_t rnti)
{
  m_mac->SetRnti (rnti);
}

void
UeMemberNrUeCmacSapProvider::NotifyConnectionSuccessful ()
{
  m_mac->DoNotifyConnectionSuccessful ();
}

void
UeMemberNrUeCmacSapProvider::SetImsi (uint64_t imsi)
 {
   m_mac->DoSetImsi (imsi);
 }

class UeMemberNrMacSapProvider : public LteMacSapProvider
{
public:
  UeMemberNrMacSapProvider (NrUeMac* mac);

  // inherited from LteMacSapProvider
  virtual void TransmitPdu (TransmitPduParameters params);
  virtual void ReportBufferStatus (ReportBufferStatusParameters params);

private:
  NrUeMac* m_mac;
};


UeMemberNrMacSapProvider::UeMemberNrMacSapProvider (NrUeMac* mac)
  : m_mac (mac)
{
}

void
UeMemberNrMacSapProvider::TransmitPdu (TransmitPduParameters params)
{
  m_mac->DoTransmitPdu (params);
}


void
UeMemberNrMacSapProvider::ReportBufferStatus (ReportBufferStatusParameters params)
{
  m_mac->DoReportBufferStatus (params);
}


class NrUePhySapUser;

class MacUeMemberPhySapUser : public NrUePhySapUser
{
public:
  MacUeMemberPhySapUser (NrUeMac* mac);

  virtual void ReceivePhyPdu (Ptr<Packet> p) override;

  virtual void ReceiveControlMessage (Ptr<NrControlMessage> msg) override;

  virtual void SlotIndication (SfnSf sfn) override;

  //virtual void NotifyHarqDeliveryFailure (uint8_t harqId);

  virtual uint8_t GetNumHarqProcess () const override;

private:
  NrUeMac* m_mac;
};

MacUeMemberPhySapUser::MacUeMemberPhySapUser (NrUeMac* mac)
  : m_mac (mac)
{

}
void
MacUeMemberPhySapUser::ReceivePhyPdu (Ptr<Packet> p)
{
  m_mac->DoReceivePhyPdu (p);
}

void
MacUeMemberPhySapUser::ReceiveControlMessage (Ptr<NrControlMessage> msg)
{
  m_mac->DoReceiveControlMessage (msg);
}

void
MacUeMemberPhySapUser::SlotIndication (SfnSf sfn)
{
  m_mac->DoSlotIndication (sfn);
}

uint8_t
MacUeMemberPhySapUser::GetNumHarqProcess () const
{
  return m_mac->GetNumHarqProcess();
}


class MemberNrSlUeMacSchedSapUser : public NrSlUeMacSchedSapUser
{

public:
  /**
   * \brief constructor
   * \param mac The pointer to the NrUeMac using this SAP
   */
  MemberNrSlUeMacSchedSapUser (NrUeMac* mac);

  virtual void SchedUeNrSlConfigInd (const std::set<NrSlSlotAlloc>& params);
  virtual uint8_t GetTotalSubCh () const;
  virtual uint8_t GetSlMaxTxTransNumPssch () const;

private:
  NrUeMac* m_mac; //!< The pointer to the NrUeMac using this SAP
};

MemberNrSlUeMacSchedSapUser::MemberNrSlUeMacSchedSapUser (NrUeMac* mac)
:m_mac (mac)
{
}
void
MemberNrSlUeMacSchedSapUser::SchedUeNrSlConfigInd (const std::set<NrSlSlotAlloc>& params)
{
  m_mac->DoSchedUeNrSlConfigInd (params);
}

uint8_t
MemberNrSlUeMacSchedSapUser::GetTotalSubCh () const
{
  return m_mac->DoGetTotalSubCh ();
}

uint8_t
MemberNrSlUeMacSchedSapUser::GetSlMaxTxTransNumPssch () const
{
  return m_mac->DoGetSlMaxTxTransNumPssch ();
}

class MemberNrSlUeMacCschedSapUser : public NrSlUeMacCschedSapUser
{

public:
  MemberNrSlUeMacCschedSapUser (NrUeMac* mac);
  virtual void  CschedUeNrSlLcConfigCnf (uint8_t lcg, uint8_t lcId);


private:
  NrUeMac* m_mac;
};

MemberNrSlUeMacCschedSapUser::MemberNrSlUeMacCschedSapUser (NrUeMac* mac)
:m_mac (mac)
{
}

void
MemberNrSlUeMacCschedSapUser::CschedUeNrSlLcConfigCnf (uint8_t lcg, uint8_t lcId)
{
  m_mac->DoCschedUeNrSlLcConfigCnf (lcg, lcId);
}

//-----------------------------------------------------------------------

TypeId
NrUeMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrUeMac")
    .SetParent<Object> ()
    .AddConstructor<NrUeMac> ()
    .AddAttribute ("NumHarqProcess",
                   "Number of concurrent stop-and-wait Hybrid ARQ processes per user",
                    UintegerValue (20),
                    MakeUintegerAccessor (&NrUeMac::SetNumHarqProcess,
                                          &NrUeMac::GetNumHarqProcess),
                    MakeUintegerChecker<uint8_t> ())
    .AddTraceSource ("UeMacRxedCtrlMsgsTrace",
                     "Ue MAC Control Messages Traces.",
                     MakeTraceSourceAccessor (&NrUeMac::m_macRxedCtrlMsgsTrace),
                     "ns3::NrMacRxTrace::RxedUeMacCtrlMsgsTracedCallback")
    .AddTraceSource ("UeMacTxedCtrlMsgsTrace",
                     "Ue MAC Control Messages Traces.",
                     MakeTraceSourceAccessor (&NrUeMac::m_macTxedCtrlMsgsTrace),
                     "ns3::NrMacRxTrace::TxedUeMacCtrlMsgsTracedCallback")
    .AddAttribute ("EnableSensing",
                   "Flag to enable NR Sidelink resource selection based on sensing; otherwise, use random selection",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrUeMac::EnableSensing),
                   MakeBooleanChecker ())
    .AddAttribute ("Tproc0",
                   "t_proc0 in slots",
                   UintegerValue (1),
                   MakeUintegerAccessor (&NrUeMac::SetTproc0,
                                         &NrUeMac::GetTproc0),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("T1",
                   "The start of the selection window in physical slots, accounting for physical layer processing delay",
                   UintegerValue (2),
                   MakeUintegerAccessor (&NrUeMac::SetT1,
                                         &NrUeMac::GetT1),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("T2",
                   "The end of the selection window in physical slots",
                   UintegerValue (33),
                   MakeUintegerAccessor (&NrUeMac::SetT2,
                                         &NrUeMac::GetT2),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ActivePoolId",
                   "The pool id of the active pool used for TX and RX",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrUeMac::SetSlActivePoolId,
                                         &NrUeMac::GetSlActivePoolId),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("ReservationPeriod",
                   "Resource Reservation Interval for NR Sidelink in ms"
                   "Must be among the values included in LteRrcSap::SlResourceReservePeriod",
                   TimeValue(MilliSeconds(100)),
                   MakeTimeAccessor (&NrUeMac::SetReservationPeriod,
                                     &NrUeMac::GetReservationPeriod),
                   MakeTimeChecker ())
     .AddAttribute ("NumSidelinkProcess",
                    "Number of concurrent stop-and-wait Sidelink processes for this UE",
                    UintegerValue (4),
                    MakeUintegerAccessor (&NrUeMac::SetNumSidelinkProcess,
                                          &NrUeMac::GetNumSidelinkProcess),
                    MakeUintegerChecker<uint8_t> ())
     .AddAttribute ("EnableBlindReTx",
                    "Flag to enable NR Sidelink blind retransmissions",
                    BooleanValue (true),
                    MakeBooleanAccessor (&NrUeMac::EnableBlindReTx),
                    MakeBooleanChecker ())
     .AddAttribute ("SlThresPsschRsrp",
                    "A threshold in dBm used for sensing based UE autonomous resource selection",
                    IntegerValue (-128),
                    MakeIntegerAccessor (&NrUeMac::SetSlThresPsschRsrp,
                                         &NrUeMac::GetSlThresPsschRsrp),
                    MakeIntegerChecker<int> ())
     .AddAttribute ("ResourcePercentage",
                    "The percentage threshold to indicate the minimum number of"
                    "candidate single-slot resources to be selected using sensing"
                    "procedure",
                    UintegerValue (20),
                    MakeUintegerAccessor (&NrUeMac::SetResourcePercentage,
                                          &NrUeMac::GetResourcePercentage),
                    MakeUintegerChecker<uint8_t> (1, 100))
    .AddTraceSource ("SlPscchScheduling",
                     "Information regarding NR SL PSCCH UE scheduling",
                     MakeTraceSourceAccessor (&NrUeMac::m_slPscchScheduling),
                     "ns3::SlPscchUeMacStatParameters::TracedCallback")
    .AddTraceSource ("SlPsschScheduling",
                     "Information regarding NR SL PSSCH UE scheduling",
                     MakeTraceSourceAccessor (&NrUeMac::m_slPsschScheduling),
                     "ns3::SlPsschUeMacStatParameters::TracedCallback")
    .AddTraceSource ("RxRlcPduWithTxRnti",
                     "PDU received trace also exporting TX UE RNTI in SL.",
                     MakeTraceSourceAccessor (&NrUeMac::m_rxRlcPduWithTxRnti),
                     "ns3::NrUeMac::ReceiveWithTxRntiTracedCallback")
          ;

  return tid;
}

NrUeMac::NrUeMac (void) : Object ()
{
  NS_LOG_FUNCTION (this);
  m_cmacSapProvider = new UeMemberNrUeCmacSapProvider (this);
  m_macSapProvider = new UeMemberNrMacSapProvider (this);
  m_phySapUser = new MacUeMemberPhySapUser (this);
  m_raPreambleUniformVariable = CreateObject<UniformRandomVariable> ();
  //NR SL
  m_nrSlMacSapProvider = new MemberNrSlMacSapProvider <NrUeMac> (this);
  m_nrSlUeCmacSapProvider = new MemberNrSlUeCmacSapProvider<NrUeMac> (this);
  m_nrSlUePhySapUser = new MemberNrSlUePhySapUser<NrUeMac> (this);
  m_nrSlUeMacCschedSapUser = new MemberNrSlUeMacCschedSapUser (this);
  m_nrSlUeMacSchedSapUser = new MemberNrSlUeMacSchedSapUser (this);
  m_ueSelectedUniformVariable = CreateObject<UniformRandomVariable> ();
  m_nrSlHarq = CreateObject<NrSlUeMacHarq> ();
}

NrUeMac::~NrUeMac (void)
{
}

void
NrUeMac::DoDispose ()
{
  m_miUlHarqProcessesPacket.clear ();
  m_miUlHarqProcessesPacketTimer.clear ();
  m_ulBsrReceived.clear ();
  m_lcInfoMap.clear ();
  m_raPreambleUniformVariable = nullptr;
  m_ueSelectedUniformVariable = nullptr;
  delete m_macSapProvider;
  delete m_cmacSapProvider;
  delete m_phySapUser;
  delete m_nrSlMacSapProvider;
  delete m_nrSlUeCmacSapProvider;
  delete m_nrSlUePhySapUser;
  delete m_nrSlUeMacCschedSapUser;
  delete m_nrSlUeMacSchedSapUser;
  m_nrSlHarq->Dispose ();
  m_nrSlHarq = nullptr;
  m_slTxPool = nullptr;
  m_slRxPool = nullptr;
}

void
NrUeMac::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  m_rnti = rnti;
}

void
NrUeMac::DoNotifyConnectionSuccessful ()
{
  NS_LOG_FUNCTION (this);
  m_phySapProvider->NotifyConnectionSuccessful ();
}

void
NrUeMac::DoSetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this);
  m_imsi = imsi;
}

uint16_t
NrUeMac::GetBwpId () const
{
  if (m_phySapProvider)
    {
      return m_phySapProvider->GetBwpId ();
    }
  else
    {
      return UINT16_MAX;
    }
}

uint16_t
NrUeMac::GetCellId () const
{
  if (m_phySapProvider)
    {
      return m_phySapProvider->GetCellId ();
    }
  else
    {
      return UINT16_MAX;
    }
}

uint32_t
NrUeMac::GetTotalBufSize () const
{
  uint32_t ret = 0;
  for (auto it = m_ulBsrReceived.cbegin (); it != m_ulBsrReceived.cend (); ++it)
    {
      ret += ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
    }
  return ret;
}

/**
 * \brief Sets the number of HARQ processes
 * \param numHarqProcesses the maximum number of harq processes
 */
void
NrUeMac::SetNumHarqProcess (uint8_t numHarqProcess)
{
  m_numHarqProcess = numHarqProcess;

  m_miUlHarqProcessesPacket.resize (GetNumHarqProcess ());
  for (uint8_t i = 0; i < m_miUlHarqProcessesPacket.size (); i++)
    {
      if (m_miUlHarqProcessesPacket.at (i).m_pktBurst == nullptr)
        {
          Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
          m_miUlHarqProcessesPacket.at (i).m_pktBurst = pb;
        }
    }
  m_miUlHarqProcessesPacketTimer.resize (GetNumHarqProcess (), 0);
}

/**
 * \return number of HARQ processes
 */
uint8_t
NrUeMac::GetNumHarqProcess () const
{
  return m_numHarqProcess;
}

// forwarded from MAC SAP
void
NrUeMac::DoTransmitPdu (LteMacSapProvider::TransmitPduParameters params)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_ulDci->m_harqProcess == params.harqProcessId);

  m_miUlHarqProcessesPacket.at (params.harqProcessId).m_lcidList.push_back (params.lcid);

  NrMacHeaderVs header;
  header.SetLcId (params.lcid);
  header.SetSize (params.pdu->GetSize ());

  params.pdu->AddHeader (header);

  LteRadioBearerTag bearerTag (params.rnti, params.lcid, params.layer);
  params.pdu->AddPacketTag (bearerTag);

  m_miUlHarqProcessesPacket.at (params.harqProcessId).m_pktBurst->AddPacket (params.pdu);
  m_miUlHarqProcessesPacketTimer.at (params.harqProcessId) = GetNumHarqProcess();

  m_ulDciTotalUsed += params.pdu->GetSize ();

  NS_ASSERT_MSG (m_ulDciTotalUsed <= m_ulDci->m_tbSize.at (0), "We used more data than the DCI allowed us.");

  m_phySapProvider->SendMacPdu (params.pdu, m_ulDciSfnsf, m_ulDci->m_symStart, params.layer);
}

void
NrUeMac::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this << static_cast<uint32_t> (params.lcid));

  auto it = m_ulBsrReceived.find (params.lcid);

  NS_LOG_INFO ("Received BSR for LC Id" << static_cast<uint32_t>(params.lcid));

  if (it != m_ulBsrReceived.end ())
    {
      // update entry
      (*it).second = params;
    }
  else
    {
      it = m_ulBsrReceived.insert (std::make_pair (params.lcid, params)).first;
    }

  if (m_srState == INACTIVE)
    {
      NS_LOG_INFO ("INACTIVE -> TO_SEND, bufSize " << GetTotalBufSize ());
      m_srState = TO_SEND;
    }
}


void
NrUeMac::SendReportBufferStatus (const SfnSf &dataSfn, uint8_t symStart)
{
  NS_LOG_FUNCTION (this);

  if (m_rnti == 0)
    {
      NS_LOG_INFO ("MAC not initialized, BSR deferred");
      return;
    }

  if (m_ulBsrReceived.size () == 0)
    {
      NS_LOG_INFO ("No BSR report to transmit");
      return;
    }
  MacCeElement bsr = MacCeElement ();
  bsr.m_rnti = m_rnti;
  bsr.m_macCeType = MacCeElement::BSR;

  // BSR is reported for each LCG
  std::unordered_map <uint8_t, LteMacSapProvider::ReportBufferStatusParameters>::iterator it;
  std::vector<uint32_t> queue (4, 0);   // one value per each of the 4 LCGs, initialized to 0
  for (it = m_ulBsrReceived.begin (); it != m_ulBsrReceived.end (); it++)
    {
      uint8_t lcid = it->first;
      std::unordered_map <uint8_t, LcInfo>::iterator lcInfoMapIt;
      lcInfoMapIt = m_lcInfoMap.find (lcid);
      NS_ASSERT (lcInfoMapIt !=  m_lcInfoMap.end ());
      NS_ASSERT_MSG ((lcid != 0) || (((*it).second.txQueueSize == 0)
                                     && ((*it).second.retxQueueSize == 0)
                                     && ((*it).second.statusPduSize == 0)),
                     "BSR should not be used for LCID 0");
      uint8_t lcg = lcInfoMapIt->second.lcConfig.logicalChannelGroup;
      queue.at (lcg) += ((*it).second.txQueueSize + (*it).second.retxQueueSize + (*it).second.statusPduSize);
    }

  NS_LOG_INFO ("Sending BSR with this info for the LCG: " << queue.at (0) << " " <<
               queue.at (1) << " " << queue.at(2) << " " << queue.at(3));
  // FF API says that all 4 LCGs are always present
  bsr.m_macCeValue.m_bufferStatus.push_back (NrMacShortBsrCe::FromBytesToLevel (queue.at (0)));
  bsr.m_macCeValue.m_bufferStatus.push_back (NrMacShortBsrCe::FromBytesToLevel (queue.at (1)));
  bsr.m_macCeValue.m_bufferStatus.push_back (NrMacShortBsrCe::FromBytesToLevel (queue.at (2)));
  bsr.m_macCeValue.m_bufferStatus.push_back (NrMacShortBsrCe::FromBytesToLevel (queue.at (3)));

  // create the message. It is used only for tracing, but we don't send it...
  Ptr<NrBsrMessage> msg = Create<NrBsrMessage> ();
  msg->SetSourceBwp (GetBwpId ());
  msg->SetBsr (bsr);

  m_macTxedCtrlMsgsTrace (m_currentSlot, GetCellId (), bsr.m_rnti, GetBwpId (), msg);

  // Here we send the real SHORT_BSR, as a subpdu.
  Ptr<Packet> p = Create<Packet> ();

  // Please note that the levels are defined from the standard. In this case,
  // we have 5 bit available, so use such standard levels. In the future,
  // when LONG BSR will be implemented, this have to change.
  NrMacShortBsrCe header;
  header.m_bufferSizeLevel_0 = NrMacShortBsrCe::FromBytesToLevel (queue.at (0));
  header.m_bufferSizeLevel_1 = NrMacShortBsrCe::FromBytesToLevel (queue.at (1));
  header.m_bufferSizeLevel_2 = NrMacShortBsrCe::FromBytesToLevel (queue.at (2));
  header.m_bufferSizeLevel_3 = NrMacShortBsrCe::FromBytesToLevel (queue.at (3));

  p->AddHeader (header);

  LteRadioBearerTag bearerTag (m_rnti, NrMacHeaderFsUl::SHORT_BSR, 0);
  p->AddPacketTag (bearerTag);

  m_ulDciTotalUsed += p->GetSize ();
  NS_ASSERT_MSG (m_ulDciTotalUsed <= m_ulDci->m_tbSize.at (0), "We used more data than the DCI allowed us.");

  //MIMO is not supported for UL yet.
  //Therefore, there will be only
  //one stream with stream Id 0.
  uint8_t streamId = 0;

  m_phySapProvider->SendMacPdu (p, dataSfn, symStart, streamId);
}

void
NrUeMac::SetUeCmacSapUser (LteUeCmacSapUser* s)
{
  m_cmacSapUser = s;
}

LteUeCmacSapProvider*
NrUeMac::GetUeCmacSapProvider (void)
{
  return m_cmacSapProvider;
}

void
NrUeMac::RefreshHarqProcessesPacketBuffer (void)
{
  NS_LOG_FUNCTION (this);

  for (uint16_t i = 0; i < m_miUlHarqProcessesPacketTimer.size (); i++)
    {
      if (m_miUlHarqProcessesPacketTimer.at (i) == 0 && m_miUlHarqProcessesPacket.at (i).m_pktBurst)
        {
          if (m_miUlHarqProcessesPacket.at (i).m_pktBurst->GetSize () > 0)
            {
              // timer expired: drop packets in buffer for this process
              NS_LOG_INFO ("HARQ Proc Id " << i << " packets buffer expired");
              Ptr<PacketBurst> emptyPb = CreateObject <PacketBurst> ();
              m_miUlHarqProcessesPacket.at (i).m_pktBurst = emptyPb;
              m_miUlHarqProcessesPacket.at (i).m_lcidList.clear ();
            }
        }
      else
        {
          //m_miUlHarqProcessesPacketTimer.at (i)--;  // ignore HARQ timeout
        }
    }
}

void
NrUeMac::DoSlotIndication (const SfnSf &sfn)
{
  NS_LOG_FUNCTION (this);
  m_currentSlot = sfn;
  NS_LOG_INFO ("Slot " << m_currentSlot);

  RefreshHarqProcessesPacketBuffer ();

  if (m_srState == TO_SEND)
    {
      NS_LOG_INFO ("Sending SR to PHY in slot " << sfn);
      SendSR ();
      m_srState = ACTIVE;
    }

  if (m_nrSlUeCmacSapUser != nullptr)
    {
      //trigger SL only when it is a SL slot
      if (m_slTxPool->IsSidelinkSlot (GetBwpId (), m_poolId, sfn.Normalize ()))
        {
          DoNrSlSlotIndication (sfn);
        }
    }
  // Feedback missing
}

void
NrUeMac::SendSR () const
{
  NS_LOG_FUNCTION (this);

  if (m_rnti == 0)
    {
      NS_LOG_INFO ("MAC not initialized, SR deferred");
      return;
    }

  // create the SR to send to the gNB
  Ptr<NrSRMessage> msg = Create<NrSRMessage> ();
  msg->SetSourceBwp (GetBwpId ());
  msg->SetRNTI (m_rnti);

  m_macTxedCtrlMsgsTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), msg);
  m_phySapProvider->SendControlMessage (msg);
}

void
NrUeMac::DoReceivePhyPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);

  LteRadioBearerTag tag;
  p->RemovePacketTag (tag);

  if (tag.GetRnti() != m_rnti) // Packet is for another user
    {
      return;
    }

  NrMacHeaderVs header;
  p->RemoveHeader (header);

  LteMacSapUser::ReceivePduParameters rxParams;
  rxParams.p = p;
  rxParams.rnti = m_rnti;
  rxParams.lcid = header.GetLcId ();

  auto it = m_lcInfoMap.find (header.GetLcId());

  // p can be empty. Well, right now no, but when someone will add CE in downlink,
  // then p can be empty.
  if (rxParams.p->GetSize () > 0)
    {
      it->second.macSapUser->ReceivePdu (rxParams);
    }
}

void
NrUeMac::RecvRaResponse (BuildRarListElement_s raResponse)
{
  NS_LOG_FUNCTION (this);
  m_waitingForRaResponse = false;
  m_rnti = raResponse.m_rnti;
  m_cmacSapUser->SetTemporaryCellRnti (m_rnti);
  m_cmacSapUser->NotifyRandomAccessSuccessful ();
}

void
NrUeMac::ProcessUlDci (const Ptr<NrUlDciMessage> &dciMsg)
{
  NS_LOG_FUNCTION (this);

  SfnSf dataSfn = m_currentSlot;
  dataSfn.Add (dciMsg->GetKDelay ());

  // Saving the data we need in DoTransmitPdu
  m_ulDciSfnsf = dataSfn;
  m_ulDciTotalUsed = 0;
  m_ulDci = dciMsg->GetDciInfoElement ();

  m_macRxedCtrlMsgsTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), dciMsg);

  NS_LOG_INFO ("UL DCI received, transmit data in slot " << dataSfn <<
               " Harq Process " << +m_ulDci->m_harqProcess <<
               " TBS " << m_ulDci->m_tbSize.at (0) << " total queue " << GetTotalBufSize ());

  if (m_ulDci->m_ndi.at (0) == 0)
    {
      // This method will retransmit the data saved in the harq buffer
      TransmitRetx ();

      // This method will transmit a new BSR.
      SendReportBufferStatus (dataSfn, m_ulDci->m_symStart);
    }
  else if (m_ulDci->m_ndi.at (0) == 1)
    {
      SendNewData ();

      NS_LOG_INFO ("After sending NewData, bufSize " << GetTotalBufSize ());

      // Send a new BSR. SendNewData() already took into account the size of
      // the BSR.
      SendReportBufferStatus (dataSfn, m_ulDci->m_symStart);

      NS_LOG_INFO ("UL DCI processing done, sent to PHY a total of " << m_ulDciTotalUsed <<
                   " B out of " << m_ulDci->m_tbSize.at (0) << " allocated bytes ");

      if (GetTotalBufSize () == 0)
        {
          m_srState = INACTIVE;
          NS_LOG_INFO ("ACTIVE -> INACTIVE, bufSize " << GetTotalBufSize ());

          // the UE may have been scheduled, but we didn't use a single byte
          // of the allocation. So send an empty PDU. This happens because the
          // byte reporting in the BSR is not accurate, due to RLC and/or
          // BSR quantization.
          if (m_ulDciTotalUsed == 0)
            {
              NS_LOG_WARN ("No byte used for this UL-DCI, sending empty PDU");

              LteMacSapProvider::TransmitPduParameters txParams;

              txParams.pdu = Create<Packet> ();
              txParams.lcid = 3;
              txParams.rnti = m_rnti;
              txParams.layer = 0;
              txParams.harqProcessId = m_ulDci->m_harqProcess;
              txParams.componentCarrierId = GetBwpId ();

              DoTransmitPdu (txParams);
            }
        }
    }
}

void
NrUeMac::TransmitRetx ()
{
  NS_LOG_FUNCTION (this);

  Ptr<PacketBurst> pb = m_miUlHarqProcessesPacket.at (m_ulDci->m_harqProcess).m_pktBurst;

  if (pb == nullptr)
    {
      NS_LOG_WARN ("The previous transmission did not contain any new data; "
                   "probably it was BSR only. To not send an old BSR to the scheduler, "
                   "we don't send anything back in this allocation. Eventually, "
                   "the Harq timer at gnb will expire, and soon this allocation will be forgotten.");
      return;
    }

  NS_LOG_DEBUG ("UE MAC RETX HARQ " << + m_ulDci->m_harqProcess);

  NS_ASSERT (pb->GetNPackets() > 0);

  for (std::list<Ptr<Packet> >::const_iterator j = pb->Begin (); j != pb->End (); ++j)
    {
      Ptr<Packet> pkt = (*j)->Copy ();
      LteRadioBearerTag bearerTag;
      if (!pkt->PeekPacketTag (bearerTag))
        {
          NS_FATAL_ERROR ("No radio bearer tag");
        }
      //MIMO is not supported for UL yet.
      //Therefore, there will be only
      //one stream with stream Id 0.
      uint8_t streamId = 0;
      m_phySapProvider->SendMacPdu (pkt, m_ulDciSfnsf, m_ulDci->m_symStart, streamId);
    }

  m_miUlHarqProcessesPacketTimer.at (m_ulDci->m_harqProcess) = GetNumHarqProcess();
}

void
NrUeMac::SendRetxData (uint32_t usefulTbs, uint32_t activeLcsRetx)
{
  NS_LOG_FUNCTION (this);

  if (activeLcsRetx == 0)
    {
      return;
    }

  uint32_t bytesPerLcId = usefulTbs / activeLcsRetx;

  for (auto & itBsr : m_ulBsrReceived)
    {
      auto &bsr = itBsr.second;

      if (m_ulDciTotalUsed + bytesPerLcId <= usefulTbs)
        {
          LteMacSapUser::TxOpportunityParameters txParams;
          txParams.lcid = bsr.lcid;
          txParams.rnti = m_rnti;
          txParams.bytes = bytesPerLcId;
          txParams.layer = 0;
          txParams.harqId = m_ulDci->m_harqProcess;
          txParams.componentCarrierId = GetBwpId ();

          NS_LOG_INFO ("Notifying RLC of LCID " << +bsr.lcid << " of a TxOpp "
                       "of " << bytesPerLcId << " B for a RETX PDU");

          m_lcInfoMap.at (bsr.lcid).macSapUser->NotifyTxOpportunity (txParams);
          // After this call, m_ulDciTotalUsed has been updated with the
          // correct amount of bytes... but it is up to us in updating the BSR
          // value, substracting the amount of bytes transmitted

          // We need to use std::min here because bytesPerLcId can be
          // greater than bsr.txQueueSize because scheduler can assign
          // more bytes than needed due to how TB size is computed.
          bsr.retxQueueSize -= std::min (bytesPerLcId, bsr.retxQueueSize);
        }
      else
        {
          NS_LOG_DEBUG ("Something wrong with the calculation of overhead."
                        "Active LCS Retx: " << activeLcsRetx << " assigned to this: " <<
                        bytesPerLcId << ", with TBS of " << m_ulDci->m_tbSize.at (0) <<
                        " usefulTbs " << usefulTbs << " and total used " << m_ulDciTotalUsed);
        }
    }
}

void
NrUeMac::SendTxData(uint32_t usefulTbs, uint32_t activeTx)
{
  NS_LOG_FUNCTION (this);

  if (activeTx == 0)
    {
      return;
    }

  uint32_t bytesPerLcId = usefulTbs / activeTx;

  for (auto & itBsr : m_ulBsrReceived)
    {
      auto &bsr = itBsr.second;

      if (m_ulDciTotalUsed + bytesPerLcId <= usefulTbs)
        {
          LteMacSapUser::TxOpportunityParameters txParams;
          txParams.lcid = bsr.lcid;
          txParams.rnti = m_rnti;
          txParams.bytes = bytesPerLcId;
          txParams.layer = 0;
          txParams.harqId = m_ulDci->m_harqProcess;
          txParams.componentCarrierId = GetBwpId ();

          NS_LOG_INFO ("Notifying RLC of LCID " << +bsr.lcid << " of a TxOpp "
                       "of " << bytesPerLcId << " B for a TX PDU");

          m_lcInfoMap.at (bsr.lcid).macSapUser->NotifyTxOpportunity (txParams);
          // After this call, m_ulDciTotalUsed has been updated with the
          // correct amount of bytes... but it is up to us in updating the BSR
          // value, substracting the amount of bytes transmitted

          // We need to use std::min here because bytesPerLcId can be
          // greater than bsr.txQueueSize because scheduler can assign
          // more bytes than needed due to how TB size is computed.
          bsr.txQueueSize -= std::min (bytesPerLcId, bsr.txQueueSize);
        }
      else
        {
          NS_LOG_DEBUG ("Something wrong with the calculation of overhead."
                        "Active LCS Retx: " << activeTx << " assigned to this: " <<
                        bytesPerLcId << ", with TBS of " << m_ulDci->m_tbSize.at (0) <<
                        " usefulTbs " << usefulTbs << " and total used " << m_ulDciTotalUsed);
        }
    }
}

void
NrUeMac::SendNewData ()
{
  NS_LOG_FUNCTION (this);
  // New transmission -> empty pkt buffer queue (for deleting eventual pkts not acked )
  Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
  m_miUlHarqProcessesPacket.at (m_ulDci->m_harqProcess).m_pktBurst = pb;
  m_miUlHarqProcessesPacket.at (m_ulDci->m_harqProcess).m_lcidList.clear ();
  NS_LOG_INFO ("Reset HARQP " << +m_ulDci->m_harqProcess);

  // Sending the status data has no boundary: let's try to send the ACK as
  // soon as possible, filling the TBS, if necessary.
  SendNewStatusData ();

  // Let's count how many LC we have, that are waiting with some data
  uint16_t activeLcsRetx = 0;
  uint16_t activeLcsTx = 0;
  for (const auto & itBsr : m_ulBsrReceived)
    {
      if (itBsr.second.retxQueueSize > 0)
        {
          activeLcsRetx++;
        }
      if (itBsr.second.txQueueSize > 0)
        {
          activeLcsTx++;
        }
    }

  // Of the TBS we received in the DCI, one part is gone for the status pdu,
  // where we didn't check much as it is the most important data, that has to go
  // out. For the rest that we have left, we can use only a part of it because of
  // the overhead of the SHORT_BSR, which is 5 bytes.
  NS_ASSERT_MSG (m_ulDciTotalUsed + 5 <= m_ulDci->m_tbSize.at (0),
                 "The StatusPDU used " << m_ulDciTotalUsed << " B, we don't have any for the SHORT_BSR.");
  uint32_t usefulTbs = m_ulDci->m_tbSize.at (0) - m_ulDciTotalUsed - 5;

  // Now, we have 3 bytes of overhead for each subPDU. Let's try to serve all
  // the queues with some RETX data.
  if (activeLcsRetx * 3 > usefulTbs)
    {
      NS_LOG_DEBUG ("The overhead for transmitting retx data is greater than the space for transmitting it."
                    "Ignore the TBS of " << usefulTbs << " B.");
    }
  else
    {
      usefulTbs -= activeLcsRetx * 3;
      SendRetxData (usefulTbs, activeLcsRetx);
    }

  // Now we have to update our useful TBS for the next transmission.
  // Remember that m_ulDciTotalUsed keep count of data and overhead that we
  // used till now.
  NS_ASSERT_MSG (m_ulDciTotalUsed + 5 <= m_ulDci->m_tbSize.at (0),
                 "The StatusPDU sending required all space, we don't have any for the SHORT_BSR.");
  usefulTbs = m_ulDci->m_tbSize.at (0) - m_ulDciTotalUsed - 5; // Update the usefulTbs.

  // The last part is for the queues with some non-RETX data. If there is no space left,
  // then nothing.
  if (activeLcsTx * 3 > usefulTbs)
    {
      NS_LOG_DEBUG ("The overhead for transmitting new data is greater than the space for transmitting it."
                    "Ignore the TBS of " << usefulTbs << " B.");
    }
  else
    {
      usefulTbs -= activeLcsTx * 3;
      SendTxData (usefulTbs, activeLcsTx);
    }

  // If we did not used the packet burst, explicitly signal it to the HARQ
  // retx, if any.
  if (m_ulDciTotalUsed == 0)
    {
      m_miUlHarqProcessesPacket.at (m_ulDci->m_harqProcess).m_pktBurst = nullptr;
      m_miUlHarqProcessesPacket.at (m_ulDci->m_harqProcess).m_lcidList.clear ();
    }
}


void
NrUeMac::SendNewStatusData()
{
  NS_LOG_FUNCTION (this);

  bool hasStatusPdu = false;
  bool sentOneStatusPdu = false;

  for (auto & bsrIt : m_ulBsrReceived)
    {
      auto & bsr = bsrIt.second;

      if (bsr.statusPduSize > 0)
        {
          hasStatusPdu = true;

          // Check if we have room to transmit the statusPdu
          if (m_ulDciTotalUsed + bsr.statusPduSize <= m_ulDci->m_tbSize.at (0))
            {
              LteMacSapUser::TxOpportunityParameters txParams;
              txParams.lcid = bsr.lcid;
              txParams.rnti = m_rnti;
              txParams.bytes = bsr.statusPduSize;
              txParams.layer = 0;
              txParams.harqId = m_ulDci->m_harqProcess;
              txParams.componentCarrierId = GetBwpId ();

              NS_LOG_INFO ("Notifying RLC of LCID " << +bsr.lcid << " of a TxOpp "
                           "of " << bsr.statusPduSize << " B for a status PDU");

              m_lcInfoMap.at (bsr.lcid).macSapUser->NotifyTxOpportunity (txParams);
              // After this call, m_ulDciTotalUsed has been updated with the
              // correct amount of bytes... but it is up to us in updating the BSR
              // value, substracting the amount of bytes transmitted
              bsr.statusPduSize = 0;
              sentOneStatusPdu = true;
            }
          else
            {
              NS_LOG_INFO ("Cannot send StatusPdu of " << bsr.statusPduSize <<
                           " B, we already used all the TBS");
            }
        }
    }

  NS_ABORT_MSG_IF (hasStatusPdu && !sentOneStatusPdu,
                   "The TBS of size " << m_ulDci->m_tbSize.at (0) << " doesn't allow us "
                   "to send one status PDU...");
}

void
NrUeMac::DoReceiveControlMessage  (Ptr<NrControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);

  switch (msg->GetMessageType ())
    {
    case (NrControlMessage::UL_DCI):
      {
        ProcessUlDci (DynamicCast<NrUlDciMessage> (msg));
        break;
      }
    case (NrControlMessage::RAR):
      {
        NS_LOG_INFO ("Received RAR in slot " << m_currentSlot);

        m_macRxedCtrlMsgsTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), msg);

        if (m_waitingForRaResponse == true)
          {
            Ptr<NrRarMessage> rarMsg = DynamicCast<NrRarMessage> (msg);
            NS_LOG_LOGIC ("got RAR with RA-RNTI " << +rarMsg->GetRaRnti () <<
                          ", expecting " << +m_raRnti);
            for (auto it = rarMsg->RarListBegin (); it != rarMsg->RarListEnd (); ++it)
              {
                if (it->rapId == m_raPreambleId)
                  {
                    RecvRaResponse (it->rarPayload);
                  }
              }
          }
        break;
      }

    default:
      NS_LOG_LOGIC ("Control message not supported/expected");
    }
}

NrUePhySapUser*
NrUeMac::GetPhySapUser ()
{
  return m_phySapUser;
}

void
NrUeMac::SetPhySapProvider (NrPhySapProvider* ptr)
{
  m_phySapProvider = ptr;
}

void
NrUeMac::DoConfigureRach ([[maybe_unused]] LteUeCmacSapProvider::RachConfig rc)
{
  NS_LOG_FUNCTION (this);
}

void
NrUeMac::DoStartContentionBasedRandomAccessProcedure ()
{
  NS_LOG_FUNCTION (this);
  RandomlySelectAndSendRaPreamble ();
}

void
NrUeMac::RandomlySelectAndSendRaPreamble ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (m_currentSlot << " Received System Information, send to PHY the RA preamble");
  SendRaPreamble (true);
}

void
NrUeMac::SendRaPreamble ([[maybe_unused]] bool contention)
{
  NS_LOG_INFO (this);
  //m_raPreambleId = m_raPreambleUniformVariable->GetInteger (0, 64 - 1);
  m_raPreambleId = g_raPreambleId++;
  /*raRnti should be subframeNo -1 */
  m_raRnti = 1;

  Ptr<NrRachPreambleMessage> rachMsg = Create<NrRachPreambleMessage> ();
  rachMsg->SetSourceBwp (GetBwpId ());
  m_macTxedCtrlMsgsTrace (m_currentSlot, GetCellId (), m_rnti, GetBwpId (), rachMsg);

  m_phySapProvider->SendRachPreamble (m_raPreambleId, m_raRnti);
}

void
NrUeMac::DoStartNonContentionBasedRandomAccessProcedure (uint16_t rnti, [[maybe_unused]] uint8_t preambleId, uint8_t prachMask)
{
  NS_LOG_FUNCTION (this << " rnti" << rnti);
  NS_ASSERT_MSG (prachMask == 0, "requested PRACH MASK = " << (uint32_t) prachMask << ", but only PRACH MASK = 0 is supported");
  m_rnti = rnti;
}

void
NrUeMac::AddLc (uint8_t lcId,  LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu)
{
  NS_LOG_FUNCTION (this << " lcId" << (uint32_t) lcId);
  NS_ASSERT_MSG (m_lcInfoMap.find (lcId) == m_lcInfoMap.end (), "cannot add channel because LCID " << lcId << " is already present");

  LcInfo lcInfo;
  lcInfo.lcConfig = lcConfig;
  lcInfo.macSapUser = msu;
  m_lcInfoMap[lcId] = lcInfo;
}

void
NrUeMac::DoRemoveLc (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << " lcId" << lcId);
}

LteMacSapProvider*
NrUeMac::GetUeMacSapProvider (void)
{
  return m_macSapProvider;
}

void
NrUeMac::DoReset ()
{
  NS_LOG_FUNCTION (this);
}

int64_t
NrUeMac::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_raPreambleUniformVariable ->SetStream (stream);
  m_ueSelectedUniformVariable->SetStream (stream + 1);
  return 2;
}
//NR SL

std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrUeMac::GetNrSlTxOpportunities (const SfnSf& sfn)
{
  NS_LOG_FUNCTION (this << sfn.GetFrame() << +sfn.GetSubframe() << sfn.GetSlot ());

  //NR module supported candSsResoA list
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> nrCandSsResoA;

  std::list <NrSlCommResourcePool::SlotInfo> candSsResoA;// S_A as per TS 38.214
  uint64_t absSlotIndex = sfn.Normalize ();
  uint8_t bwpId = GetBwpId ();
  uint16_t numerology = sfn.GetNumerology ();

  //Check the validity of the resource selection window configuration (T1 and T2) 
  //and the following parameters: numerology and reservation period.
  uint16_t nsMs = (m_t2-m_t1+1) * (1 / pow(2,numerology)); // number of slots mutiplied by the slot duration in ms
  uint16_t rsvpMs = static_cast <uint16_t> (m_pRsvpTx.GetMilliSeconds ()); // reservation period in ms
  NS_ABORT_MSG_IF (nsMs > rsvpMs, 
      "An error may be generated due to the fact that the resource selection window size is higher than the resource reservation period value. Make sure that (T2-T1+1) x (1/(2^numerology)) < reservation period. Modify the values of T1, T2, numerology, and reservation period accordingly.");

  //step 4 as per TS 38.214 sec 8.1.4
  auto allTxOpps = m_slTxPool->GetNrSlCommOpportunities (absSlotIndex, bwpId, numerology, GetSlActivePoolId (), m_t1, m_t2);
  if (allTxOpps.size () == 0)
    {
      //Since, all the parameters (i.e., T1, T2min, and T2) of the selection
      //window are in terms of physical slots, it may happen that there are no
      //slots available for Sidelink, which depends on the TDD pattern and the
      //Sidelink bitmap.
      return GetNrSupportedList (sfn, allTxOpps);
    }
  candSsResoA = allTxOpps;
  uint32_t mTotal = candSsResoA.size (); // total number of candidate single-slot resources
  int rsrpThrehold = GetSlThresPsschRsrp ();

  if (m_enableSensing)
    {
      if (m_sensingData.size () == 0)
        {
          //no sensing
          nrCandSsResoA = GetNrSupportedList (sfn, candSsResoA);
          return nrCandSsResoA;
        }

      //Copy the buffer so we can trim the buffer as per Tproc0.
      //Note, we do not need to delete the latest measurement
      //from the original buffer because it will be deleted
      //by UpdateSensingWindow method once it is outdated.

      auto sensedData = m_sensingData;

      //latest sensing data is at the end of the list
      //now remove the latest sensing data as per the value of Tproc0. This would
      //keep the size of the buffer equal to [n – T0 , n – Tproc0)

      auto rvIt = sensedData.crbegin ();
      if (rvIt != sensedData.crend ())
        {
          while (sfn.Normalize () - rvIt->sfn.Normalize () <= GetTproc0 ())
            {
              NS_LOG_DEBUG ("IMSI " << m_imsi << " ignoring sensed SCI at sfn " << sfn << " received at " << rvIt->sfn);
              sensedData.pop_back ();
              rvIt = sensedData.crbegin ();
            }
        }

      // calculate all possible transmissions of sensed data
      //using a vector of SlotSensingData, since we need to check all the SCIs
      //and their possible future transmission that are received during the
      //above trimmed sensing window. On each index of the vector, it is a
      //list that holds the info of each received SCI and its possible
      //future transmission.
      std::vector<std::list<SlotSensingData>> allSensingData;
      for (const auto &itSensedSlot:sensedData)
        {
          std::list<SlotSensingData> listFutureSensTx = GetFutSlotsBasedOnSens (itSensedSlot);
          allSensingData.push_back (listFutureSensTx);
        }

      NS_LOG_DEBUG ("Size of allSensingData " << allSensingData.size ());

      //step 5 point 1: We don't need to implement it since we only sense those
      //slots at which this UE does not transmit. This is due to the half
      //duplex nature of the PHY.
      //step 6
      do
        {
          //following assignment is needed since we might have to perform
          //multiple do-while over the same list by increasing the rsrpThrehold
          candSsResoA = allTxOpps;
          nrCandSsResoA = GetNrSupportedList (sfn, candSsResoA);
          auto itCandSsResoA = nrCandSsResoA.begin ();
          while (itCandSsResoA != nrCandSsResoA.end ())
            {
              bool erased = false;
              // calculate all proposed transmissions of current candidate resource
              std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> listFutureCands;
              uint16_t pPrimeRsvpTx = m_slTxPool->GetResvPeriodInSlots (GetBwpId (),
                                                                        m_poolId,
                                                                        m_pRsvpTx,
                                                                        m_nrSlUePhySapProvider->GetSlotPeriod ());
              for (uint16_t i = 0; i < m_cResel; i++)
                {
                  auto slAlloc = *itCandSsResoA;
                  slAlloc.sfn.Add (i * pPrimeRsvpTx);
                  listFutureCands.emplace_back (slAlloc);
                }
              // Traverse over all the possible transmissions of each sensed SCI
              for (const auto &itSensedData:allSensingData)
                {
                  // for all proposed transmissions of current candidate resource
                  for (auto &itFutureCand:listFutureCands)
                    {
                      for (const auto &itFutureSensTx:itSensedData)
                        {
                          if (itFutureCand.sfn.Normalize () == itFutureSensTx.sfn.Normalize ())
                            {
                              uint16_t lastSbChInPlusOne = itFutureSensTx.sbChStart + itFutureSensTx.sbChLength;
                              for (uint8_t i = itFutureSensTx.sbChStart; i < lastSbChInPlusOne; i++)
                                {
                                  NS_LOG_DEBUG (this << " Overlapped Slot " << itCandSsResoA->sfn.Normalize () << " occupied " << +itFutureSensTx.sbChLength << " subchannels index " << +itFutureSensTx.sbChStart);
                                  itCandSsResoA->occupiedSbCh.insert (i);
                                }
                              if (itCandSsResoA->occupiedSbCh.size () == GetTotalSubCh (m_poolId))
                                {
                                  if(itFutureSensTx.slRsrp > rsrpThrehold)
                                    {
                                      itCandSsResoA = nrCandSsResoA.erase (itCandSsResoA);
                                      erased = true;
                                      NS_LOG_DEBUG ("Absolute slot number " << itFutureCand.sfn.Normalize () << " erased. Its rsrp : " << itFutureSensTx.slRsrp << " Threshold : " << rsrpThrehold);
                                      //stop traversing over sensing data as we have
                                      //already found the slot to exclude.
                                      break; // break for (const auto &itFutureSensTx:listFutureSensTx)
                                    }
                                }
                            }
                        }
                      if (erased)
                        {
                          break; // break for (const auto &itFutureCand:listFutureCands)
                        }
                    }
                  if (erased)
                    {
                      break; // break for (const auto &itSensedSlot:allSensingData)
                    }
                }
              if (!erased)
                {
                  itCandSsResoA++;
                }
            }
          //step 7. If the following while will not break, start over do-while
          //loop with rsrpThreshold increased by 3dB
          rsrpThrehold += 3;
          if (rsrpThrehold > 0)
            {
              //0 dBm is the maximum RSRP threshold level so if we reach
              //it, that means all the available slots are overlapping
              //in time and frequency with the sensed slots, and the
              //RSRP of the sensed slots is very high.
              NS_LOG_DEBUG ("Reached maximum RSRP threshold, unable to select resources");
              nrCandSsResoA.erase (nrCandSsResoA.begin (), nrCandSsResoA.end ());
              break; //break do while
            }
        }
      while (nrCandSsResoA.size () < (GetResourcePercentage () / 100.0) * mTotal);

      NS_LOG_DEBUG (nrCandSsResoA.size () << " slots selected after sensing resource selection from " << mTotal << " slots");
    }
  else
    {
      //no sensing
      nrCandSsResoA = GetNrSupportedList (sfn, candSsResoA);
      NS_LOG_DEBUG ("No sensing: Total slots selected " << nrCandSsResoA.size ());
    }

  return nrCandSsResoA;
}

std::list<SlotSensingData>
NrUeMac::GetFutSlotsBasedOnSens (SensingData sensedData)
{
  NS_LOG_FUNCTION (this);
  std::list<SlotSensingData> listFutureSensTx;

  double slotLenMiSec = m_nrSlUePhySapProvider->GetSlotPeriod ().GetSeconds () * 1000.0;
  NS_ABORT_MSG_IF (slotLenMiSec > 1, "Slot length can not exceed 1 ms");
  uint16_t selecWindLen = (m_t2 - m_t1) + 1; //selection window length in physical slots
  double tScalMilSec = selecWindLen * slotLenMiSec;
  double pRsvpRxMilSec = static_cast<double> (sensedData.rsvp);
  uint16_t q = 0;
  //I am aware that two double variable are compared. I don't expect these two
  //numbers to be big floating-point numbers.
  if (pRsvpRxMilSec < tScalMilSec)
    {
      q = static_cast <uint16_t> (std::ceil (tScalMilSec / pRsvpRxMilSec));
    }
  else
    {
      q = 1;
    }
  uint16_t pPrimeRsvpRx = m_slTxPool->GetResvPeriodInSlots (GetBwpId (),
                                                            m_poolId,
                                                            MilliSeconds (sensedData.rsvp),
                                                            m_nrSlUePhySapProvider->GetSlotPeriod ());

  for (uint16_t i = 0; i <= q; i++)
    {
      SlotSensingData sensedSlotData (sensedData.sfn, sensedData.rsvp,
                                      sensedData.sbChLength, sensedData.sbChStart,
                                      sensedData.prio, sensedData.slRsrp);
      sensedSlotData.sfn.Add (i * pPrimeRsvpRx);
      listFutureSensTx.emplace_back (sensedSlotData);

      if (sensedData.gapReTx1 != std::numeric_limits <uint8_t>::max ())
        {
          auto reTx1Slot = sensedSlotData;
          reTx1Slot.sfn = sensedSlotData.sfn.GetFutureSfnSf (sensedData.gapReTx1);
          reTx1Slot.sbChLength = sensedData.sbChLength;
          reTx1Slot.sbChStart = sensedData.sbChStartReTx1;
          listFutureSensTx.emplace_back (reTx1Slot);
        }
      if (sensedData.gapReTx2 != std::numeric_limits <uint8_t>::max ())
        {
          auto reTx2Slot = sensedSlotData;
          reTx2Slot.sfn = sensedSlotData.sfn.GetFutureSfnSf (sensedData.gapReTx2);
          reTx2Slot.sbChLength = sensedData.sbChLength;
          reTx2Slot.sbChStart = sensedData.sbChStartReTx2;
          listFutureSensTx.emplace_back (reTx2Slot);
        }
    }

  return listFutureSensTx;
}


std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrUeMac::GetNrSupportedList (const SfnSf& sfn, std::list <NrSlCommResourcePool::SlotInfo> slotInfo)
{
  NS_LOG_FUNCTION (this);
  std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> nrSupportedList;
  for (const auto& it:slotInfo)
    {
      std::set <uint8_t> emptySet;
      NrSlUeMacSchedSapProvider::NrSlSlotInfo info (it.numSlPscchRbs, it.slPscchSymStart,
                                                    it.slPscchSymLength, it.slPsschSymStart,
                                                    it.slPsschSymLength, it.slSubchannelSize,
                                                    it.slMaxNumPerReserve,
                                                    sfn.GetFutureSfnSf (it.slotOffset),
                                                    emptySet);
      nrSupportedList.emplace_back (info);
    }

  return nrSupportedList;
}

void
NrUeMac::DoReceiveSensingData (SensingData sensingData)
{
  NS_LOG_FUNCTION (this);

  if (m_enableSensing)
    {
      //oldest data will be at the front of the queue
      m_sensingData.push_back (sensingData);
    }
}

void
NrUeMac::UpdateSensingWindow (const SfnSf& sfn)
{
  NS_LOG_FUNCTION (this << sfn);

  uint16_t sensWindLen = m_slTxPool->GetNrSlSensWindInSlots (GetBwpId (),
                                                             m_poolId,
                                                             m_nrSlUePhySapProvider->GetSlotPeriod ());
  //oldest sensing data is on the top of the list
  auto it = m_sensingData.cbegin();
  while (it != m_sensingData.cend ())
    {
      if (it->sfn.Normalize () < sfn.Normalize () - sensWindLen)
        {
          NS_LOG_DEBUG ("IMSI " << m_imsi << " erasing SCI at sfn " << sfn << " received at " << it->sfn);
          it = m_sensingData.erase (it);
        }
      else
        {
          //once we reached the sensing data, which lies in the
          //sensing window, we break. If the last entry lies in the sensing
          //window rest of the entries as well.
          break;
        }
      ++it;
    }

  //To keep the size of the buffer equal to [n – T0 , n – Tproc0)
  //the other end of sensing buffer is trimmed in GetNrSlTxOpportunities.
}


void
NrUeMac::DoReceivePsschPhyPdu (Ptr<PacketBurst> pdu)
{
  NS_LOG_FUNCTION (this << "Received Sidelink PDU from PHY");

  LteRadioBearerTag tag;
  NrSlSciF2aHeader sciF2a;
  //Separate SCI stage 2 packet from data packets
  std::list <Ptr<Packet> > dataPkts;
  bool foundSci2 = false;
  for (auto packet : pdu->GetPackets ())
    {
      LteRadioBearerTag tag;
      if (packet->PeekPacketTag (tag) == false)
        {
          //SCI stage 2 is the only packet in the packet burst, which does
          //not have the tag
          packet->RemoveHeader (sciF2a);
          foundSci2 = true;
        }
      else
        {
          dataPkts.push_back (packet);
        }
    }

  NS_ABORT_MSG_IF (foundSci2 == false, "Did not find SCI stage 2 in PSSCH packet burst");
  NS_ASSERT_MSG (dataPkts.size () > 0, "Received PHY PDU with not data packets");

  //Perform L2 filtering.
  //Remember, all the packets in the packet burst are for the same
  //destination, therefore it is safe to do the following.
  auto it = m_sidelinkRxDestinations.find (sciF2a.GetDstId ());
  if (it == m_sidelinkRxDestinations.end ())
    {
      //if we hit this assert that means SCI 1 reception code in NrUePhy
      //is not filtering the SCI 1 correctly.
      NS_FATAL_ERROR ("Received PHY PDU with unknown destination " << sciF2a.GetDstId ());
    }

  for (auto &pktIt : dataPkts)
    {
      pktIt->RemovePacketTag (tag);
      //Even though all the packets in the packet burst are for the same
      //destination, they can belong to different Logical Channels (LC),
      //therefore, we have to build the identifier and find the LC of the
      //packet.
      SidelinkLcIdentifier identifier;
      identifier.lcId = tag.GetLcid ();
      identifier.srcL2Id = sciF2a.GetSrcId ();
      identifier.dstL2Id = sciF2a.GetDstId ();

      std::map <SidelinkLcIdentifier, SlLcInfoUeMac>::iterator it = m_nrSlLcInfoMap.find (identifier);
      if (it == m_nrSlLcInfoMap.end ())
        {
          //notify RRC to setup bearer
          m_nrSlUeCmacSapUser->NotifySidelinkReception (tag.GetLcid (), identifier.srcL2Id, identifier.dstL2Id);

          //should be setup now
          it = m_nrSlLcInfoMap.find (identifier);
          if (it == m_nrSlLcInfoMap.end ())
            {
              NS_FATAL_ERROR ("Failure to setup Sidelink radio bearer for reception");
            }
        }
      NrSlMacSapUser::NrSlReceiveRlcPduParameters rxPduParams (pktIt, m_rnti, tag.GetLcid (),
                                                               identifier.srcL2Id, identifier.dstL2Id);

      FireTraceSlRlcRxPduWithTxRnti (pktIt->Copy (), tag.GetLcid ());

      it->second.macSapUser->ReceiveNrSlRlcPdu (rxPduParams);
    }
}

void
NrUeMac::DoNrSlSlotIndication (const SfnSf& sfn)
{
  NS_LOG_FUNCTION (this << " Frame " << sfn.GetFrame() << " Subframe " << +sfn.GetSubframe()
                        << " slot " << sfn.GetSlot () << " Normalized slot number " << sfn.Normalize ());

  UpdateSensingWindow (sfn);

  if (m_slTxPool->GetNrSlSchedulingType () == NrSlCommResourcePool::SCHEDULED)
    {

    }
  else if (m_slTxPool->GetNrSlSchedulingType () == NrSlCommResourcePool::UE_SELECTED)
    {
      //Do not ask for resources if no HARQ/Sidelink process is available
      if (m_nrSlHarq->GetNumAvaiableHarqIds () > 0)
        {
          for (const auto &itDst : m_sidelinkTxDestinations)
            {
              const auto itGrantInfo = m_grantInfo.find (itDst.first);
              bool foundDest = itGrantInfo != m_grantInfo.end () ? true : false;
              if (foundDest)
                {
                  //If the re-selection counter of the found destination is not zero,
                  //it means it already have resources assigned to it via semi-persistent
                  //scheduling, thus, we go to the next destination
                  if (itGrantInfo->second.slResoReselCounter != 0)
                    {
                      NS_LOG_INFO ("Destination " << itDst.first << " already have the allocation, scheduling the next destination, if any");
                      continue;
                    }

                  double randProb = m_ueSelectedUniformVariable->GetValue (0, 1);
                  if (itGrantInfo->second.cReselCounter > 0 &&
                      itGrantInfo->second.slotAllocations.size () > 0 && m_slProbResourceKeep > randProb)
                    {
                      NS_LOG_INFO ("IMSI " << m_imsi << " keeping the resource for " << itDst.first);
                      NS_ASSERT_MSG (itGrantInfo->second.slResoReselCounter == 0, "Sidelink resource re-selection counter must be zero before continuing with the same grant for dst " << itDst.first);
                      //keeping the resource, reassign the same sidelink resource re-selection
                      //counter we chose while creating the fresh grant
                      itGrantInfo->second.slResoReselCounter = itGrantInfo->second.prevSlResoReselCounter;
                      continue;
                    }
                  else
                    {
                      //we need to choose new resource so erase the previous allocations
                      NS_LOG_DEBUG ("Choosing new resources : ResoReselCounter "
                                    << +itGrantInfo->second.slResoReselCounter
                                    << " cResel " << itGrantInfo->second.cReselCounter
                                    << " remaining alloc " << itGrantInfo->second.slotAllocations.size ()
                                    << " slProbResourceKeep " << +m_slProbResourceKeep
                                    << " random prob " << randProb);
                      itGrantInfo->second.slotAllocations.erase (itGrantInfo->second.slotAllocations.begin (), itGrantInfo->second.slotAllocations.end ());
                    }
                }

              m_reselCounter = GetRndmReselectionCounter ();
              m_cResel = m_reselCounter * 10;
              NS_LOG_DEBUG ("Resel Counter " << +m_reselCounter << " cResel " << m_cResel);
              std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> availbleReso = GetNrSlTxOpportunities (sfn);
              //sensing or not, due to the semi-persistent scheduling, after
              //calling the GetNrSlTxOpportunities method, and before asking the
              //scheduler for resources, we need to remove those available slots,
              //which are already part of the existing grant. When sensing is
              //activated this step corresponds to step 2 in TS 38.214 sec 8.1.4
              //Remember, availbleReso itself can be an empty list, we do not need
              //another if here because FilterTxOpportunities will return an empty
              //list.
              auto filteredReso = FilterTxOpportunities (availbleReso);
              if (!filteredReso.empty ())
                {
                  //we ask the scheduler for resources only if the filtered list is not empty.
                  NS_LOG_INFO ("IMSI " << m_imsi << " scheduling the destination " << itDst.first);
                  m_nrSlUeMacSchedSapProvider->SchedUeNrSlTriggerReq (itDst.first, filteredReso);
                  m_reselCounter = 0;
                  m_cResel = 0;
                }
              else
                {
                  NS_LOG_DEBUG ("Do not have enough slots to allocate. Not calling the scheduler for dst " << itDst.first);
                  m_reselCounter = 0;
                  m_cResel = 0;
                }
            }
        }
    }
  else
    {
      NS_FATAL_ERROR ("Scheduling type " << m_slTxPool->GetNrSlSchedulingType () << " for NR Sidelink pools is unknown");
    }

  //check if we need to transmit PSCCH + PSSCH
  //We are starting with the transmission of data packets because if the buffer
  //at the RLC would be empty we just erase the grant of the current slot
  //without transmitting SCI 1 and SCI 2 message, and data. Therefore,
  //even we had the grant we will not put anything in the queues at the PHY.
  for (auto & itGrantInfo : m_grantInfo)
    {
      if (itGrantInfo.second.slResoReselCounter != 0 && itGrantInfo.second.slotAllocations.begin ()->sfn == sfn)
        {
          auto grantIt = itGrantInfo.second.slotAllocations.begin ();
          NrSlSlotAlloc currentGrant = *grantIt;
          //remove the allocation since we already used it
          itGrantInfo.second.slotAllocations.erase (grantIt);
          NS_LOG_INFO ("Grant at : Frame = " << currentGrant.sfn.GetFrame ()
                       << " SF = " << +currentGrant.sfn.GetSubframe ()
                       << " slot = " << currentGrant.sfn.GetSlot ());

          uint32_t tbSize = 0;
          //sum all the assigned bytes to each LC of this destination
          for (const auto & it : currentGrant.slRlcPduInfo)
            {
              NS_LOG_DEBUG ("LC " << static_cast <uint16_t> (it.lcid) << " was assigned " << it.size << "bytes");
              tbSize += it.size;
            }

          if (currentGrant.ndi)
            {
              itGrantInfo.second.tbTxCounter = 1;
              for (const auto & itLcRlcPduInfo : currentGrant.slRlcPduInfo)
                {
                  SidelinkLcIdentifier slLcId;
                  slLcId.lcId = itLcRlcPduInfo.lcid;
                  slLcId.srcL2Id = m_srcL2Id;
                  slLcId.dstL2Id = currentGrant.dstL2Id;
                  const auto & itLc = m_nrSlLcInfoMap.find (slLcId);
                  NS_ASSERT_MSG (itLc != m_nrSlLcInfoMap.end (), "No LC with id " << +itLcRlcPduInfo.lcid << " found for destination " << currentGrant.dstL2Id);
                  //Assign HARQ id and store it in the grant
                  //Side effect, if RLC buffer would be empty, the assigned
                  //HARQ id will be occupied until all the grants for this slot
                  //and it ReTxs are exhausted.
                  uint8_t nrSlHarqId {std::numeric_limits <uint8_t>::max ()};
                  nrSlHarqId = m_nrSlHarq->AssignNrSlHarqProcessId (currentGrant.dstL2Id);
                  itGrantInfo.second.nrSlHarqId = nrSlHarqId;
                  NS_ASSERT_MSG (itGrantInfo.second.nrSlHarqId != std::numeric_limits <uint8_t>::max (), "HARQ id was not assigned for destination " << currentGrant.dstL2Id);
                  NS_LOG_DEBUG ("Notifying NR SL RLC of TX opportunity for LC id " << +itLcRlcPduInfo.lcid << " for TB size " << itLcRlcPduInfo.size);
                  itLc->second.macSapUser->NotifyNrSlTxOpportunity (NrSlMacSapUser::NrSlTxOpportunityParameters (itLcRlcPduInfo.size, m_rnti, itLcRlcPduInfo.lcid,
                                                                                                                 0, itGrantInfo.second.nrSlHarqId, GetBwpId (),
                                                                                                                 m_srcL2Id, currentGrant.dstL2Id));
                }
              if (itGrantInfo.second.tbTxCounter == itGrantInfo.second.nSelected)
                {
                  //38.321 5.22.1.3.1a says: if this transmission corresponds
                  //to the last transmission of the MAC PDU, decrement
                  //SL_RESOURCE_RESELECTION_COUNTER by 1, if available.
                  --itGrantInfo.second.slResoReselCounter;
                  --itGrantInfo.second.cReselCounter;
                  //Clear the HARQ buffer since we assign the HARQ id
                  //and put the TB in HARQ buffer (if RLC buffer was not empty)
                  //even if the retxs are not configured.
                  m_nrSlHarq->RecvNrSlHarqFeedback (currentGrant.dstL2Id, itGrantInfo.second.nrSlHarqId);
                }
            }
          else
            {
              //retx from MAC HARQ buffer
              //we might want to match the LC ids in currentGrant.slRlcPduInfo and
              //the LC ids whose packets are in the packet burst in the HARQ
              //buffer. I am not doing it at the moment as it might slow down
              //the simulation.
              itGrantInfo.second.tbTxCounter++;
              Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
              pb = m_nrSlHarq->GetPacketBurst (currentGrant.dstL2Id, itGrantInfo.second.nrSlHarqId);
              if (m_enableBlindReTx)
                {
                  if (pb->GetNPackets () > 0)
                    {
                      m_nrSlMacPduTxed = true;
                      NS_ASSERT_MSG (pb->GetNPackets () > 0, "Packet burst for HARQ id " << +itGrantInfo.second.nrSlHarqId << " is empty");
                      for (const auto & itPkt : pb->GetPackets ())
                        {
                          m_nrSlUePhySapProvider->SendPsschMacPdu (itPkt);
                        }
                    }
                  else
                    {
                      NS_LOG_DEBUG ("Wasted Retx grant");
                    }
                  if (itGrantInfo.second.tbTxCounter == itGrantInfo.second.nSelected)
                    {
                      //38.321 5.22.1.3.1a says: if this transmission corresponds
                      //to the last transmission of the MAC PDU, decrement
                      //SL_RESOURCE_RESELECTION_COUNTER by 1, if available.
                      --itGrantInfo.second.slResoReselCounter;
                      --itGrantInfo.second.cReselCounter;
                      itGrantInfo.second.tbTxCounter = 0;
                      //generate fake feedback. It is important to clear the
                      //HARQ buffer, which make the HARQ id available again
                      //since we assign the HARQ id even in the end
                      //RLC buffer is empty. See the for loop above to trigger RLC.
                      NS_LOG_INFO ("sending fake HARQ feedback for HARQ id " << +itGrantInfo.second.nrSlHarqId);
                      m_nrSlHarq->RecvNrSlHarqFeedback (currentGrant.dstL2Id, itGrantInfo.second.nrSlHarqId);
                    }
                }
              else
                {
                  //we need to have a feedback to do the retx when blind retx
                  //are not enabled.
                  NS_FATAL_ERROR ("Feedback based retransmissions are not supported");
                }
            }

          if (!m_nrSlMacPduTxed)
            {
              //NR SL MAC PDU was not txed. It can happen if RLC buffer was empty
              NS_LOG_DEBUG ("Grant wasted at : Frame = " << currentGrant.sfn.GetFrame () << " SF = " << +currentGrant.sfn.GetSubframe () << " slot = " << currentGrant.sfn.GetSlot ());
              continue;
            }

          //prepare and send SCI format 2A message
          NrSlSciF2aHeader sciF2a;
          sciF2a.SetHarqId (itGrantInfo.second.nrSlHarqId);
          sciF2a.SetNdi (currentGrant.ndi);
          sciF2a.SetRv (currentGrant.rv);
          sciF2a.SetSrcId (m_srcL2Id);
          sciF2a.SetDstId (currentGrant.dstL2Id);
          //fields which are not used yet that is why we set them to 0
          sciF2a.SetCsiReq (0);
          sciF2a.SetHarqFbIndicator (0);
          sciF2a.SetCastType (NrSlSciF2aHeader::Broadcast);
          Ptr<Packet> pktSciF02 = Create<Packet> ();
          pktSciF02->AddHeader (sciF2a);
          //put SCI stage 2 in PSSCH queue
          m_nrSlUePhySapProvider->SendPsschMacPdu (pktSciF02);

          //set the VarTti allocation info for PSSCH
          NrSlVarTtiAllocInfo dataVarTtiInfo;
          dataVarTtiInfo.SlVarTtiType = NrSlVarTtiAllocInfo::DATA;
          dataVarTtiInfo.symStart = currentGrant.slPsschSymStart;
          dataVarTtiInfo.symLength = currentGrant.slPsschSymLength;
          dataVarTtiInfo.rbStart = currentGrant.slPsschSubChStart * m_slTxPool->GetNrSlSubChSize (GetBwpId (), m_poolId);
          dataVarTtiInfo.rbLength = currentGrant.slPsschSubChLength * m_slTxPool->GetNrSlSubChSize (GetBwpId (), m_poolId);
          m_nrSlUePhySapProvider->SetNrSlVarTtiAllocInfo (sfn, dataVarTtiInfo);

          // Collect statistics for NR SL PSCCH UE MAC scheduling trace
          SlPsschUeMacStatParameters psschStatsParams;
          psschStatsParams.timeMs = Simulator::Now ().GetSeconds () * 1000.0;
          psschStatsParams.imsi = m_imsi;
          psschStatsParams.rnti = m_rnti;
          psschStatsParams.frameNum = currentGrant.sfn.GetFrame ();
          psschStatsParams.subframeNum = currentGrant.sfn.GetSubframe ();
          psschStatsParams.slotNum = currentGrant.sfn.GetSlot ();
          psschStatsParams.symStart = currentGrant.slPsschSymStart;
          psschStatsParams.symLength = currentGrant.slPsschSymLength;
          psschStatsParams.rbStart = currentGrant.slPsschSubChStart * m_slTxPool->GetNrSlSubChSize (GetBwpId (), m_poolId);
          psschStatsParams.subChannelSize = m_slTxPool->GetNrSlSubChSize (GetBwpId (), m_poolId);
          psschStatsParams.rbLength = currentGrant.slPsschSubChLength * m_slTxPool->GetNrSlSubChSize (GetBwpId (), m_poolId);
          psschStatsParams.harqId = itGrantInfo.second.nrSlHarqId;
          psschStatsParams.ndi = currentGrant.ndi;
          psschStatsParams.rv = currentGrant.rv;
          psschStatsParams.srcL2Id = m_srcL2Id;
          psschStatsParams.dstL2Id = currentGrant.dstL2Id;
          psschStatsParams.csiReq = sciF2a.GetCsiReq ();
          psschStatsParams.castType = sciF2a.GetCastType ();
          psschStatsParams.resoReselCounter = itGrantInfo.second.slResoReselCounter;
          psschStatsParams.cReselCounter = itGrantInfo.second.cReselCounter;
          m_slPsschScheduling (psschStatsParams); //Trace

          if (currentGrant.txSci1A)
            {
              //prepare and send SCI format 1A message
              NrSlSciF1aHeader sciF1a;
              sciF1a.SetPriority (currentGrant.priority);
              sciF1a.SetMcs (currentGrant.mcs);
              sciF1a.SetSciStage2Format (NrSlSciF1aHeader::SciFormat2A);
              sciF1a.SetSlResourceReservePeriod (static_cast <uint16_t> (m_pRsvpTx.GetMilliSeconds ()));
              sciF1a.SetTotalSubChannels (GetTotalSubCh (m_poolId));
              sciF1a.SetIndexStartSubChannel (currentGrant.slPsschSubChStart);
              sciF1a.SetLengthSubChannel (currentGrant.slPsschSubChLength);
              sciF1a.SetSlMaxNumPerReserve (currentGrant.maxNumPerReserve);
              if (currentGrant.slotNumInd > 1)
                {
                  //itGrantInfo.second.slotAllocations.cbegin () points to
                  //the next slot allocation this slot has to indicate
                  std::vector<uint8_t> gaps = ComputeGaps (currentGrant.sfn,
                                                           itGrantInfo.second.slotAllocations.cbegin (),
                                                           currentGrant.slotNumInd);
                  std::vector<uint8_t> sbChIndex = GetStartSbChOfReTx (itGrantInfo.second.slotAllocations.cbegin (),
                                                                       currentGrant.slotNumInd);
                  sciF1a.SetGapReTx1 (gaps.at (0));
                  sciF1a.SetIndexStartSbChReTx1 (sbChIndex.at (0));
                  if (gaps.size () > 1)
                    {
                      sciF1a.SetGapReTx2 (gaps.at (1));
                      NS_ASSERT_MSG (gaps.at (0) < gaps.at (1), "Incorrect computation of ReTx slot gaps");
                      sciF1a.SetIndexStartSbChReTx2 (sbChIndex.at (1));
                    }
                }

              Ptr<Packet> pktSciF1a = Create<Packet> ();
              pktSciF1a->AddHeader (sciF1a);
              NrSlMacPduTag tag (m_rnti, currentGrant.sfn, currentGrant.slPsschSymStart, currentGrant.slPsschSymLength, tbSize, currentGrant.dstL2Id);
              pktSciF1a->AddPacketTag (tag);

              m_nrSlUePhySapProvider->SendPscchMacPdu (pktSciF1a);

              //set the VarTti allocation info for PSCCH
              NrSlVarTtiAllocInfo ctrlVarTtiInfo;
              ctrlVarTtiInfo.SlVarTtiType = NrSlVarTtiAllocInfo::CTRL;
              ctrlVarTtiInfo.symStart = currentGrant.slPscchSymStart;
              ctrlVarTtiInfo.symLength = currentGrant.slPscchSymLength;
              ctrlVarTtiInfo.rbStart = currentGrant.slPsschSubChStart * m_slTxPool->GetNrSlSubChSize (GetBwpId (), m_poolId);
              ctrlVarTtiInfo.rbLength = currentGrant.numSlPscchRbs;
              m_nrSlUePhySapProvider->SetNrSlVarTtiAllocInfo (sfn, ctrlVarTtiInfo);

              // Collect statistics for NR SL PSCCH UE MAC scheduling trace
              SlPscchUeMacStatParameters pscchStatsParams;
              pscchStatsParams.timeMs = Simulator::Now ().GetSeconds () * 1000.0;
              pscchStatsParams.imsi = m_imsi;
              pscchStatsParams.rnti = m_rnti;
              pscchStatsParams.frameNum = currentGrant.sfn.GetFrame ();
              pscchStatsParams.subframeNum = currentGrant.sfn.GetSubframe ();
              pscchStatsParams.slotNum = currentGrant.sfn.GetSlot ();
              pscchStatsParams.symStart = currentGrant.slPscchSymStart;
              pscchStatsParams.symLength = currentGrant.slPscchSymLength;
              pscchStatsParams.rbStart = currentGrant.slPsschSubChStart * m_slTxPool->GetNrSlSubChSize (GetBwpId (), m_poolId);
              pscchStatsParams.rbLength = currentGrant.numSlPscchRbs;
              pscchStatsParams.priority = currentGrant.priority;
              pscchStatsParams.mcs = currentGrant.mcs;
              pscchStatsParams.tbSize = tbSize;
              pscchStatsParams.slResourceReservePeriod = static_cast <uint16_t> (m_pRsvpTx.GetMilliSeconds ());
              pscchStatsParams.totalSubChannels = GetTotalSubCh (m_poolId);
              pscchStatsParams.slPsschSubChStart = currentGrant.slPsschSubChStart;
              pscchStatsParams.slPsschSubChLength = currentGrant.slPsschSubChLength;
              pscchStatsParams.slMaxNumPerReserve = currentGrant.maxNumPerReserve;
              pscchStatsParams.gapReTx1 = sciF1a.GetGapReTx1 ();
              pscchStatsParams.gapReTx2 = sciF1a.GetGapReTx2 ();
              m_slPscchScheduling (pscchStatsParams); //Trace
            }
        }
      else
        {
          //When there are no resources it may happen that the re-selection
          //counter of already existing destination remains zero. In this case,
          //we just go the next destination, if any.
        }
      //make this false before processing the grant for next destination
      m_nrSlMacPduTxed = false;
    }
}

std::vector<uint8_t>
NrUeMac::ComputeGaps (const SfnSf& sfn,
                      std::set <NrSlSlotAlloc>::const_iterator it, uint8_t slotNumInd)
{
  NS_LOG_FUNCTION (this);
  std::vector<uint8_t> gaps;
  //slotNumInd is the number including the first TX. Gaps are computed only for
  //the ReTxs
  for (uint8_t i = 0; i < slotNumInd - 1; i++)
    {
      std::advance (it, i);
      gaps.push_back (static_cast<uint8_t>(it->sfn.Normalize () - sfn.Normalize ()));
    }

  return gaps;
}

std::vector<uint8_t>
NrUeMac::GetStartSbChOfReTx (std::set <NrSlSlotAlloc>::const_iterator it, uint8_t slotNumInd)
{
  NS_LOG_FUNCTION (this);
  std::vector<uint8_t> startSbChIndex;
  //slotNumInd is the number including the first TX. Start sub-channel index or
  //indices are retrieved only for the ReTxs
  for (uint8_t i = 0; i < slotNumInd - 1; i++)
    {
      std::advance (it, i);
      startSbChIndex.push_back (it->slPsschSubChStart);
    }

  return startSbChIndex;
}

std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>
NrUeMac::FilterTxOpportunities (std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo> txOppr)
{
  NS_LOG_FUNCTION (this);

  if (txOppr.empty ())
    {
      return txOppr;
    }

  NrSlSlotAlloc dummyAlloc;

  for (const auto & itDst : m_grantInfo)
    {
      auto itTxOppr = txOppr.begin ();
      while (itTxOppr != txOppr.end ())
        {
          dummyAlloc.sfn = itTxOppr->sfn;
          auto itAlloc = itDst.second.slotAllocations.find (dummyAlloc);
          if (itAlloc != itDst.second.slotAllocations.end ())
            {
              itTxOppr = txOppr.erase (itTxOppr);
            }
          else
            {
              ++itTxOppr;
            }
        }
    }

  return txOppr;
}

void
NrUeMac::DoSchedUeNrSlConfigInd (const std::set<NrSlSlotAlloc>& slotAllocList)
{
  NS_LOG_FUNCTION (this);
  auto itGrantInfo = m_grantInfo.find (slotAllocList.begin ()->dstL2Id);

  if (itGrantInfo == m_grantInfo.end ())
    {
      NrSlGrantInfo grant = CreateGrantInfo (slotAllocList);
      itGrantInfo = m_grantInfo.emplace (std::make_pair (slotAllocList.begin ()->dstL2Id, grant)).first;
    }
  else
    {
      NS_ASSERT_MSG (itGrantInfo->second.slResoReselCounter == 0, "Sidelink resource counter must be zero before assigning new grant for dst " << slotAllocList.begin ()->dstL2Id);
      NrSlGrantInfo grant = CreateGrantInfo (slotAllocList);
      itGrantInfo->second = grant;
    }

  NS_ASSERT_MSG (itGrantInfo->second.slotAllocations.size () > 0, "CreateGrantInfo failed to create grants");
}

NrUeMac::NrSlGrantInfo
NrUeMac::CreateGrantInfo (const std::set<NrSlSlotAlloc>& slotAllocList)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG ((m_reselCounter != 0), "Can not create grants with 0 Resource selection counter");
  NS_ASSERT_MSG ((m_cResel != 0), "Can not create grants with 0 cResel counter");

  NS_LOG_DEBUG ("Creating grants with Resel Counter " << +m_reselCounter << " and cResel " << m_cResel);

  uint16_t resPeriodSlots = m_slTxPool->GetResvPeriodInSlots (GetBwpId (), m_poolId, m_pRsvpTx, m_nrSlUePhySapProvider->GetSlotPeriod ());
  NrSlGrantInfo grant;

  grant.cReselCounter = m_cResel;
  //save reselCounter to be used if probability of keeping the resource would
  //be higher than the configured one
  grant.prevSlResoReselCounter = m_reselCounter;
  grant.slResoReselCounter = m_reselCounter;

  grant.nSelected = static_cast<uint8_t>(slotAllocList.size ());
  NS_LOG_DEBUG ("nSelected = " << +grant.nSelected);

  for (uint16_t i = 0; i < m_cResel; i++)
    {
      for (const auto &it : slotAllocList)
        {
          auto slAlloc = it;
          slAlloc.sfn.Add (i * resPeriodSlots);
          NS_LOG_DEBUG ("First tx at : Frame = " << slAlloc.sfn.GetFrame ()
                        << " SF = " << +slAlloc.sfn.GetSubframe ()
                        << " slot = " << slAlloc.sfn.GetSlot ());
          bool insertStatus = grant.slotAllocations.emplace (slAlloc).second;
          NS_ASSERT_MSG (insertStatus, "slot allocation already exist");
        }
    }

  return grant;
}



NrSlMacSapProvider*
NrUeMac::GetNrSlMacSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_nrSlMacSapProvider;
}

void
NrUeMac::SetNrSlMacSapUser (NrSlMacSapUser* s)
{
  NS_LOG_FUNCTION (this);
  m_nrSlMacSapUser = s;
}

NrSlUeCmacSapProvider*
NrUeMac::GetNrSlUeCmacSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_nrSlUeCmacSapProvider;
}

 void
 NrUeMac::SetNrSlUeCmacSapUser (NrSlUeCmacSapUser* s)
 {
   NS_LOG_FUNCTION (this);
   m_nrSlUeCmacSapUser = s;
 }

 NrSlUePhySapUser*
 NrUeMac::GetNrSlUePhySapUser ()
 {
   NS_LOG_FUNCTION (this);
   return m_nrSlUePhySapUser;
 }

 void
 NrUeMac::SetNrSlUePhySapProvider (NrSlUePhySapProvider* s)
 {
   NS_LOG_FUNCTION (this);
   m_nrSlUePhySapProvider = s;
 }

 void
 NrUeMac::SetNrSlUeMacSchedSapProvider (NrSlUeMacSchedSapProvider* s)
 {
   NS_LOG_FUNCTION (this);
   m_nrSlUeMacSchedSapProvider = s;
 }

 NrSlUeMacSchedSapUser*
 NrUeMac::GetNrSlUeMacSchedSapUser ()
 {
   NS_LOG_FUNCTION (this);
   return m_nrSlUeMacSchedSapUser;
 }

 void
 NrUeMac::SetNrSlUeMacCschedSapProvider (NrSlUeMacCschedSapProvider* s)
 {
   NS_LOG_FUNCTION (this);
   m_nrSlUeMacCschedSapProvider = s;
 }

 NrSlUeMacCschedSapUser*
 NrUeMac::GetNrSlUeMacCschedSapUser ()
 {
   NS_LOG_FUNCTION (this);
   return m_nrSlUeMacCschedSapUser;
 }

void
NrUeMac::DoTransmitNrSlRlcPdu (const NrSlMacSapProvider::NrSlRlcPduParameters &params)
{
  NS_LOG_FUNCTION (this << +params.lcid << +params.harqProcessId);
  LteRadioBearerTag bearerTag (params.rnti, params.lcid, 0);
  params.pdu->AddPacketTag (bearerTag);
  NS_LOG_DEBUG ("Adding packet in HARQ buffer for HARQ id " << +params.harqProcessId << " pkt size " << params.pdu->GetSize ());
  m_nrSlHarq->AddPacket (params.dstL2Id, params.lcid, params.harqProcessId, params.pdu);
  m_nrSlUePhySapProvider->SendPsschMacPdu (params.pdu);
  m_nrSlMacPduTxed = true;
}

void
NrUeMac::DoReportNrSlBufferStatus (const NrSlMacSapProvider::NrSlReportBufferStatusParameters &params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Reporting for Sidelink. Tx Queue size = " << params.txQueueSize);
  //Sidelink BSR
  std::map <SidelinkLcIdentifier, NrSlMacSapProvider::NrSlReportBufferStatusParameters>::iterator it;

  SidelinkLcIdentifier slLcId;
  slLcId.lcId = params.lcid;
  slLcId.srcL2Id = params.srcL2Id;
  slLcId.dstL2Id = params.dstL2Id;

  it = m_nrSlBsrReceived.find (slLcId);
  if (it != m_nrSlBsrReceived.end ())
    {
      // update entry
      (*it).second = params;
    }
  else
    {
      m_nrSlBsrReceived.insert (std::make_pair (slLcId, params));
    }

  auto report = NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams (params.rnti, params.lcid,
                                                                                params.txQueueSize, params.txQueueHolDelay,
                                                                                params.retxQueueSize, params.retxQueueHolDelay,
                                                                                params.statusPduSize, params.srcL2Id, params.dstL2Id);

  m_nrSlUeMacSchedSapProvider->SchedUeNrSlRlcBufferReq (report);
}

void
NrUeMac::DoAddNrSlLc (const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo &slLcInfo, NrSlMacSapUser* msu)
{
  NS_LOG_FUNCTION (this << +slLcInfo.lcId << slLcInfo.srcL2Id << slLcInfo.dstL2Id);
  SidelinkLcIdentifier slLcIdentifier;
  slLcIdentifier.lcId = slLcInfo.lcId;
  slLcIdentifier.srcL2Id = slLcInfo.srcL2Id;
  slLcIdentifier.dstL2Id = slLcInfo.dstL2Id;

  NS_ASSERT_MSG (m_nrSlLcInfoMap.find (slLcIdentifier) == m_nrSlLcInfoMap.end (), "cannot add LCID " << +slLcInfo.lcId
                                                                    << ", srcL2Id " << slLcInfo.srcL2Id << ", dstL2Id " << slLcInfo.dstL2Id << " is already present");

  SlLcInfoUeMac slLcInfoUeMac;
  slLcInfoUeMac.lcInfo = slLcInfo;
  slLcInfoUeMac.macSapUser = msu;
  m_nrSlLcInfoMap.insert (std::make_pair (slLcIdentifier, slLcInfoUeMac));

  auto lcInfo = NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo (slLcInfo.dstL2Id, slLcInfo.lcId,
                                                                        slLcInfo.lcGroup, slLcInfo.pqi,
                                                                        slLcInfo.priority, slLcInfo.isGbr,
                                                                        slLcInfo.mbr, slLcInfo.gbr);

  //Following if is needed because this method is called for both
  //TX and RX LCs addition into m_nrSlLcInfoMap. In case of RX LC, the
  //destination is this UE MAC.
  if (slLcInfo.srcL2Id == m_srcL2Id)
    {
      NS_LOG_DEBUG ("UE MAC with src id " << m_srcL2Id << " giving info of LC to the scheduler");
      m_nrSlUeMacCschedSapProvider->CschedUeNrSlLcConfigReq (lcInfo);
      AddNrSlDstL2Id (slLcInfo.dstL2Id, slLcInfo.priority);
    }
}

void
NrUeMac::DoRemoveNrSlLc (uint8_t slLcId, uint32_t srcL2Id, uint32_t dstL2Id)
{
  NS_LOG_FUNCTION (this << +slLcId << srcL2Id << dstL2Id);
  NS_ASSERT_MSG (slLcId > 3, "Hey! I can delete only the LC for data radio bearers.");
  SidelinkLcIdentifier slLcIdentifier;
  slLcIdentifier.lcId = slLcId;
  slLcIdentifier.srcL2Id = srcL2Id;
  slLcIdentifier.dstL2Id = dstL2Id;
  NS_ASSERT_MSG (m_nrSlLcInfoMap.find (slLcIdentifier) != m_nrSlLcInfoMap.end (), "could not find Sidelink LCID " << slLcId);
  m_nrSlLcInfoMap.erase (slLcIdentifier);
}

void
NrUeMac::DoResetNrSlLcMap ()
{
  NS_LOG_FUNCTION (this);

  auto it = m_nrSlLcInfoMap.begin ();

  while (it != m_nrSlLcInfoMap.end ())
    {
      if (it->first.lcId > 3) //SL DRB LC starts from 4
        {
          m_nrSlLcInfoMap.erase (it);
        }
      else
        {
          it++;
        }
    }
}

void
NrUeMac::AddNrSlDstL2Id (uint32_t dstL2Id, uint8_t lcPriority)
{
  NS_LOG_FUNCTION (this << dstL2Id << lcPriority);
  bool foundDst = false;
  for (auto& it : m_sidelinkTxDestinations)
    {
      if (it.first == dstL2Id)
        {
          foundDst = true;
          if (lcPriority < it.second)
            {
              it.second = lcPriority;
            }
          break;
        }
    }

  if (!foundDst)
    {
      m_sidelinkTxDestinations.push_back (std::make_pair (dstL2Id, lcPriority));
    }

  std::sort (m_sidelinkTxDestinations.begin (), m_sidelinkTxDestinations.end (), CompareSecond);
}

bool
NrUeMac::CompareSecond (std::pair<uint32_t, uint8_t>& a, std::pair<uint32_t, uint8_t>& b)
{
  return a.second < b.second;
}

void
NrUeMac::DoAddNrSlCommTxPool (Ptr<const NrSlCommResourcePool> txPool)
{
  NS_LOG_FUNCTION (this << txPool);
  m_slTxPool = txPool;
  m_slTxPool->ValidateResvPeriod (GetBwpId (), m_poolId, m_pRsvpTx, m_nrSlUePhySapProvider->GetSlotPeriod ());
}

void
NrUeMac::DoAddNrSlCommRxPool (Ptr<const NrSlCommResourcePool> rxPool)
{
  NS_LOG_FUNCTION (this);
  m_slRxPool = rxPool;
}

void
NrUeMac::DoSetSlProbResoKeep (double prob)
{
  NS_LOG_FUNCTION (this << prob);
  NS_ASSERT_MSG (prob <= 1.0, "Probability value must be between 0 and 1");
  m_slProbResourceKeep = prob;
}

void
NrUeMac::DoSetSlMaxTxTransNumPssch (uint8_t maxTxPssch)
{
  NS_LOG_FUNCTION (this << +maxTxPssch);
  NS_ASSERT_MSG (maxTxPssch <= 32, "Number of PSSCH transmissions can not exceed 32");
  m_slMaxTxTransNumPssch = maxTxPssch;
}

void
NrUeMac::DoSetSourceL2Id (uint32_t srcL2Id)
{
  NS_LOG_FUNCTION (this << srcL2Id);
  m_srcL2Id = srcL2Id;
}

void
NrUeMac::DoAddNrSlRxDstL2Id (uint32_t dstL2Id)
{
  NS_LOG_FUNCTION (this << dstL2Id);
  m_sidelinkRxDestinations.insert (dstL2Id);
}

uint8_t
NrUeMac::DoGetSlActiveTxPoolId ()
{
  return GetSlActivePoolId ();
}

std::vector <std::pair<uint32_t, uint8_t> >
NrUeMac::DoGetSlTxDestinations ()
{
  return m_sidelinkTxDestinations;
}

std::unordered_set <uint32_t>
NrUeMac::DoGetSlRxDestinations ()
{
  return m_sidelinkRxDestinations;
}

uint8_t
NrUeMac::DoGetTotalSubCh () const
{
  NS_LOG_FUNCTION (this);
  return GetTotalSubCh (m_poolId);
}

uint8_t
NrUeMac::DoGetSlMaxTxTransNumPssch () const
{
  NS_LOG_FUNCTION (this);
  return m_slMaxTxTransNumPssch;
}
void
NrUeMac::DoCschedUeNrSlLcConfigCnf (uint8_t lcg, uint8_t lcId)
{
  NS_LOG_FUNCTION (this << +lcg << +lcId);
  NS_LOG_INFO ("SL UE scheduler successfully added LCG " << +lcg << " LC id " << +lcId);
}

void
NrUeMac::EnableSensing (bool enableSensing)
{
  NS_LOG_FUNCTION (this << enableSensing);
  NS_ASSERT_MSG (m_enableSensing == false, " Once the sensing is enabled, it can not be enabled or disabled again");
  m_enableSensing = enableSensing;
}

void
NrUeMac::EnableBlindReTx (bool enableBlindReTx)
{
  NS_LOG_FUNCTION (this << enableBlindReTx);
  NS_ASSERT_MSG (m_enableBlindReTx == false, " Once the blind re-transmission is enabled, it can not be enabled or disabled again");
  m_enableBlindReTx = enableBlindReTx;
}

void
NrUeMac::SetTproc0 (uint8_t tproc0)
{
  NS_LOG_FUNCTION (this << +tproc0);
  m_tproc0 = tproc0;
}

uint8_t
NrUeMac::GetTproc0 () const
{
  return m_tproc0;
}

uint8_t
NrUeMac::GetT1 () const
{
  return m_t1;
}

void
NrUeMac::SetT1 (uint8_t t1)
{
  NS_LOG_FUNCTION (this << +t1);
  m_t1 = t1;
}

uint16_t
NrUeMac::GetT2 () const
{
  return m_t2;
}

void
NrUeMac::SetT2 (uint16_t t2)
{
  NS_LOG_FUNCTION (this << t2);
  m_t2 = t2;
}

uint16_t
NrUeMac::GetSlActivePoolId () const
{
  return m_poolId;
}

void
NrUeMac::SetSlActivePoolId (uint16_t poolId)
{
  m_poolId =  poolId;
}

uint8_t
NrUeMac::GetTotalSubCh (uint16_t poolId) const
{
  NS_LOG_FUNCTION (this << poolId);

  uint16_t subChSize = m_slTxPool->GetNrSlSubChSize (static_cast <uint8_t> (GetBwpId ()), poolId);

  uint8_t totalSubChanels = static_cast <uint8_t> (std::floor (m_nrSlUePhySapProvider->GetBwInRbs () / subChSize));

  return totalSubChanels;
}

void
NrUeMac::SetReservationPeriod (const Time &rsvp)
{
  NS_LOG_FUNCTION (this << rsvp);
  m_pRsvpTx = rsvp;
}

Time
NrUeMac::GetReservationPeriod () const
{
  return m_pRsvpTx;
}

uint8_t
NrUeMac::GetRndmReselectionCounter () const
{
  uint8_t min;
  uint8_t max;
  uint16_t periodInt = static_cast <uint16_t> (m_pRsvpTx.GetMilliSeconds ());

  switch(periodInt)
  {
    case 100:
    case 150:
    case 200:
    case 250:
    case 300:
    case 350:
    case 400:
    case 450:
    case 500:
    case 550:
    case 600:
    case 700:
    case 750:
    case 800:
    case 850:
    case 900:
    case 950:
    case 1000:
      min = 5;
      max = 15;
      break;
    default:
      if (periodInt < 100)
        {
          min = GetLoBoundReselCounter (periodInt);
          max = GetUpBoundReselCounter (periodInt);
        }
      else
        {
          NS_FATAL_ERROR ("VALUE NOT SUPPORTED!");
        }
      break;
  }

  NS_LOG_DEBUG ("Range to choose random reselection counter. min: " << +min << " max: " << +max);
  return m_ueSelectedUniformVariable->GetInteger (min, max);
}

uint8_t
NrUeMac::GetLoBoundReselCounter (uint16_t pRsrv) const
{
  NS_LOG_FUNCTION (this << pRsrv);
  NS_ASSERT_MSG (pRsrv < 100, "Resource reservation must be less than 100 ms");
  uint8_t lBound = (5 * std::ceil (100 / (std::max (static_cast <uint16_t> (20), pRsrv))));
  return lBound;
}

uint8_t
NrUeMac::GetUpBoundReselCounter (uint16_t pRsrv) const
{
  NS_LOG_FUNCTION (this << pRsrv);
  NS_ASSERT_MSG (pRsrv < 100, "Resource reservation must be less than 100 ms");
  uint8_t uBound = (15 * std::ceil (100 / (std::max (static_cast <uint16_t> (20), pRsrv))));
  return uBound;
}

void
NrUeMac::SetNumSidelinkProcess (uint8_t numSidelinkProcess)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_grantInfo.size () == 0, "Can not reset the number of Sidelink processes. Scheduler already assigned grants");
  m_numSidelinkProcess = numSidelinkProcess;
  m_nrSlHarq->InitHarqBuffer (m_numSidelinkProcess);
}

uint8_t
NrUeMac::GetNumSidelinkProcess () const
{
  NS_LOG_FUNCTION (this);
  return m_numSidelinkProcess;
}

void
NrUeMac::SetSlThresPsschRsrp (int thresRsrp)
{
  NS_LOG_FUNCTION (this);
  m_thresRsrp = thresRsrp;
}

int
NrUeMac::GetSlThresPsschRsrp () const
{
  NS_LOG_FUNCTION (this);
  return m_thresRsrp;
}

void
NrUeMac::SetResourcePercentage (uint8_t percentage)
{
  NS_LOG_FUNCTION (this);
  m_resPercentage = percentage;
}

uint8_t
NrUeMac::GetResourcePercentage () const
{
  NS_LOG_FUNCTION (this);
  return m_resPercentage;
}

void
NrUeMac::FireTraceSlRlcRxPduWithTxRnti (const Ptr<Packet> p, uint8_t lcid)
{
  NS_LOG_FUNCTION (this);
  // Receiver timestamp
  RlcTag rlcTag;
  Time delay;

  bool ret = p->FindFirstMatchingByteTag (rlcTag);
  NS_ASSERT_MSG (ret, "RlcTag is missing for NR SL");

  delay = Simulator::Now () - rlcTag.GetSenderTimestamp ();
  m_rxRlcPduWithTxRnti (m_imsi, m_rnti, rlcTag.GetTxRnti (), lcid, p->GetSize (), delay.GetSeconds ());
}

//////////////////////////////////////////////

}

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

#include "nr-sl-ue-mac-scheduler-ns3.h"

#include <ns3/log.h>
#include <ns3/boolean.h>
#include <ns3/uinteger.h>
#include <ns3/pointer.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeMacMacSchedulerNs3");
NS_OBJECT_ENSURE_REGISTERED (NrSlUeMacSchedulerNs3);

TypeId
NrSlUeMacSchedulerNs3::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlUeMacSchedulerNs3")
    .SetParent<NrSlUeMacScheduler> ()
    .SetGroupName ("nr")
    .AddAttribute ("FixNrSlMcs",
                   "Fix MCS to value set in SetInitialNrSlMcs",
                   BooleanValue (false),
                   MakeBooleanAccessor (&NrSlUeMacSchedulerNs3::UseFixedNrSlMcs,
                                        &NrSlUeMacSchedulerNs3::IsNrSlMcsFixed),
                   MakeBooleanChecker ())
    .AddAttribute ("InitialNrSlMcs",
                   "The initial value of the MCS used for NR Sidelink",
                   UintegerValue (14),
                   MakeUintegerAccessor (&NrSlUeMacSchedulerNs3::SetInitialNrSlMcs,
                                         &NrSlUeMacSchedulerNs3::GetInitialNrSlMcs),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("NrSlAmc",
                   "The NR SL AMC of this scheduler",
                   PointerValue (),
                   MakePointerAccessor (&NrSlUeMacSchedulerNs3::m_nrSlAmc),
                   MakePointerChecker <NrAmc> ())
  ;
  return tid;
}

NrSlUeMacSchedulerNs3::NrSlUeMacSchedulerNs3 ()
{
  m_uniformVariable = CreateObject<UniformRandomVariable> ();
}

NrSlUeMacSchedulerNs3::~NrSlUeMacSchedulerNs3 ()
{
  //just to make sure
  m_dstMap.clear ();
}

void
NrSlUeMacSchedulerNs3::DoCschedUeNrSlLcConfigReq (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params)
{
  NS_LOG_FUNCTION (this << params.dstL2Id << +params.lcId);

  auto dstInfo = CreateDstInfo (params);
  const auto & lcgMap = dstInfo->GetNrSlLCG (); //Map of unique_ptr should not copy
  auto itLcg = lcgMap.find (params.lcGroup);
  auto itLcgEnd = lcgMap.end ();
  if (itLcg == itLcgEnd)
    {
      NS_LOG_DEBUG ("Created new NR SL LCG for destination " << dstInfo->GetDstL2Id () <<
                    " LCG ID =" << static_cast<uint32_t> (params.lcGroup));
      itLcg = dstInfo->Insert (CreateLCG (params.lcGroup));
    }

  itLcg->second->Insert (CreateLC (params));
  NS_LOG_INFO ("Added LC id " << +params.lcId << " in LCG " << +params.lcGroup);
  //send confirmation to UE MAC
  m_nrSlUeMacCschedSapUser->CschedUeNrSlLcConfigCnf (params.lcGroup, params.lcId);
}

std::shared_ptr<NrSlUeMacSchedulerDstInfo>
NrSlUeMacSchedulerNs3::CreateDstInfo (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params)
{
  std::shared_ptr<NrSlUeMacSchedulerDstInfo> dstInfo = nullptr;
  auto itDst = m_dstMap.find (params.dstL2Id);
  if (itDst == m_dstMap.end ())
    {
      NS_LOG_INFO ("Creating destination info. Destination L2 id " << params.dstL2Id);

      dstInfo = std::make_shared <NrSlUeMacSchedulerDstInfo> (params.dstL2Id);
      dstInfo->SetDstMcs (m_initialNrSlMcs);

      itDst = m_dstMap.insert (std::make_pair (params.dstL2Id, dstInfo)).first;
    }
  else
    {
      NS_LOG_LOGIC ("Doing nothing. You are seeing this because we are adding new LC " << +params.lcId << " for Dst " << params.dstL2Id);
      dstInfo = itDst->second;
    }

  return dstInfo;
}


NrSlLCGPtr
NrSlUeMacSchedulerNs3::CreateLCG (uint8_t lcGroup) const
{
  NS_LOG_FUNCTION (this);
  return std::unique_ptr<NrSlUeMacSchedulerLCG> (new NrSlUeMacSchedulerLCG (lcGroup));
}


NrSlLCPtr
NrSlUeMacSchedulerNs3::CreateLC (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) const
{
  NS_LOG_FUNCTION (this);
  return std::unique_ptr<NrSlUeMacSchedulerLC> (new NrSlUeMacSchedulerLC (params));
}


void
NrSlUeMacSchedulerNs3::DoSchedUeNrSlRlcBufferReq (const struct NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams& params)
{
  NS_LOG_FUNCTION (this << params.dstL2Id <<
                   static_cast<uint32_t> (params.lcid));

  GetSecond DstInfoOf;
  auto itDst = m_dstMap.find (params.dstL2Id);
  NS_ABORT_MSG_IF (itDst == m_dstMap.end (), "Destination " << params.dstL2Id << " info not found");

  for (const auto &lcg : DstInfoOf (*itDst)->GetNrSlLCG ())
    {
      if (lcg.second->Contains (params.lcid))
        {
          NS_LOG_INFO ("Updating NR SL LC Info: " << params <<
                       " in LCG: " << static_cast<uint32_t> (lcg.first));
          lcg.second->UpdateInfo (params);
          return;
        }
    }
  // Fail miserably because we didn't find any LC
  NS_FATAL_ERROR ("The LC does not exist. Can't update");
}

void
NrSlUeMacSchedulerNs3::DoSchedUeNrSlTriggerReq (uint32_t dstL2Id, const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& params)
{
  NS_LOG_FUNCTION (this << dstL2Id);

  const auto itDst = m_dstMap.find (dstL2Id);
  NS_ABORT_MSG_IF (itDst == m_dstMap.end (), "Destination " << dstL2Id << "info not found");

  std::set<NrSlSlotAlloc> allocList;

  bool allocated = DoNrSlAllocation (params, itDst->second, allocList);

  if (!allocated)
    {
      return;
    }
  m_nrSlUeMacSchedSapUser->SchedUeNrSlConfigInd (allocList);
}


uint8_t
NrSlUeMacSchedulerNs3::GetTotalSubCh () const
{
  return m_nrSlUeMacSchedSapUser->GetTotalSubCh ();
}

uint8_t
NrSlUeMacSchedulerNs3::GetSlMaxTxTransNumPssch () const
{
  return m_nrSlUeMacSchedSapUser->GetSlMaxTxTransNumPssch ();
}


void
NrSlUeMacSchedulerNs3::InstallNrSlAmc (const Ptr<NrAmc> &nrSlAmc)
{
  NS_LOG_FUNCTION (this);
  m_nrSlAmc = nrSlAmc;
  //In NR it does not have any impact
  m_nrSlAmc->SetUlMode ();
}

Ptr<const NrAmc>
NrSlUeMacSchedulerNs3::GetNrSlAmc () const
{
  NS_LOG_FUNCTION (this);
  return m_nrSlAmc;
}

void
NrSlUeMacSchedulerNs3::UseFixedNrSlMcs (bool fixMcs)
{
  NS_LOG_FUNCTION (this);
  m_fixedNrSlMcs = fixMcs;
}

bool
NrSlUeMacSchedulerNs3::IsNrSlMcsFixed () const
{
  NS_LOG_FUNCTION (this);
  return m_fixedNrSlMcs;
}

void
NrSlUeMacSchedulerNs3::SetInitialNrSlMcs (uint8_t mcs)
{
  NS_LOG_FUNCTION (this);
  m_initialNrSlMcs = mcs;
}

uint8_t
NrSlUeMacSchedulerNs3::GetInitialNrSlMcs () const
{
  NS_LOG_FUNCTION (this);
  return m_initialNrSlMcs;
}

uint8_t
NrSlUeMacSchedulerNs3::GetRv (uint8_t txNumTb) const
{
  NS_LOG_FUNCTION (this << +txNumTb);
  uint8_t modulo  = txNumTb % 4;
  //we assume rvid = 0, so RV would take 0, 2, 3, 1
  //see TS 38.21 table 6.1.2.1-2
  uint8_t rv = 0;
  switch (modulo)
  {
    case 0:
      rv = 0;
      break;
    case 1:
      rv = 2;
      break;
    case 2:
      rv = 3;
      break;
    case 3:
      rv = 1;
      break;
    default:
      NS_ABORT_MSG ("Wrong modulo result to deduce RV");
  }

  return rv;
}

int64_t
NrSlUeMacSchedulerNs3::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  m_uniformVariable->SetStream (stream );
  return 1;
}



} //namespace ns3

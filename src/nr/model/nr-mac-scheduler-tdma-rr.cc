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
#include "nr-mac-scheduler-tdma-rr.h"
#include "nr-mac-scheduler-ue-info-rr.h"
#include <ns3/log.h>
#include <algorithm>
#include <functional>

namespace ns3  {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerTdmaRR");
NS_OBJECT_ENSURE_REGISTERED (NrMacSchedulerTdmaRR);

TypeId
NrMacSchedulerTdmaRR::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMacSchedulerTdmaRR")
    .SetParent<NrMacSchedulerTdma> ()
    .AddConstructor<NrMacSchedulerTdmaRR> ()
  ;
  return tid;
}

NrMacSchedulerTdmaRR::NrMacSchedulerTdmaRR ()
  : NrMacSchedulerTdma ()
{
  NS_LOG_FUNCTION (this);
}

std::shared_ptr<NrMacSchedulerUeInfo>
NrMacSchedulerTdmaRR::CreateUeRepresentation (const NrMacCschedSapProvider::CschedUeConfigReqParameters &params) const
{
  NS_LOG_FUNCTION (this);
  return std::make_shared <NrMacSchedulerUeInfoRR> (params.m_rnti, params.m_beamConfId,
                                                        std::bind (&NrMacSchedulerTdmaRR::GetNumRbPerRbg, this));
}

void
NrMacSchedulerTdmaRR::AssignedDlResources (const UePtrAndBufferReq &ue,
                                           [[maybe_unused]] const FTResources &assigned,
                                           [[maybe_unused]] const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  GetFirst GetUe;
  GetUe (ue)->UpdateDlMetric (m_dlAmc);
}

void
NrMacSchedulerTdmaRR::AssignedUlResources (const UePtrAndBufferReq &ue,
                                           [[maybe_unused]] const FTResources &assigned,
                                           [[maybe_unused]] const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  GetFirst GetUe;
  GetUe (ue)->UpdateUlMetric (m_ulAmc);
}

std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const NrMacSchedulerNs3::UePtrAndBufferReq &rhs )>
NrMacSchedulerTdmaRR::GetUeCompareDlFn () const
{
  return NrMacSchedulerUeInfoRR::CompareUeWeightsDl;
}

std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const NrMacSchedulerNs3::UePtrAndBufferReq &rhs )>
NrMacSchedulerTdmaRR::GetUeCompareUlFn () const
{
  return NrMacSchedulerUeInfoRR::CompareUeWeightsUl;
}

} //namespace ns3

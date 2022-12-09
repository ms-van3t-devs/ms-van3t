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
#include "nr-mac-scheduler-ofdma-pf.h"
#include "nr-mac-scheduler-ue-info-pf.h"
#include <algorithm>
#include <ns3/double.h>
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerOfdmaPF");
NS_OBJECT_ENSURE_REGISTERED (NrMacSchedulerOfdmaPF);

TypeId
NrMacSchedulerOfdmaPF::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMacSchedulerOfdmaPF")
    .SetParent<NrMacSchedulerOfdmaRR> ()
    .AddConstructor<NrMacSchedulerOfdmaPF> ()
    .AddAttribute ("FairnessIndex",
                   "Value (between 0 and 1) that defines the PF metric (1 is the traditional 3GPP PF, 0 is RR in throughput",
                   DoubleValue (1),
                   MakeDoubleAccessor (&NrMacSchedulerOfdmaPF::SetFairnessIndex,
                                       &NrMacSchedulerOfdmaPF::GetFairnessIndex),
                   MakeDoubleChecker<float> (0, 1))
    .AddAttribute ("LastAvgTPutWeight",
                   "Weight of the last average throughput in the average throughput calculation",
                   DoubleValue (99),
                   MakeDoubleAccessor (&NrMacSchedulerOfdmaPF::SetTimeWindow,
                                       &NrMacSchedulerOfdmaPF::GetTimeWindow),
                   MakeDoubleChecker<float> (0))
  ;
  return tid;
}

NrMacSchedulerOfdmaPF::NrMacSchedulerOfdmaPF () : NrMacSchedulerOfdmaRR ()
{

}

void
NrMacSchedulerOfdmaPF::SetFairnessIndex (double v)
{
  NS_LOG_FUNCTION (this);
  m_alpha = v;
}

double
NrMacSchedulerOfdmaPF::GetFairnessIndex () const
{
  NS_LOG_FUNCTION (this);
  return m_alpha;
}

void
NrMacSchedulerOfdmaPF::SetTimeWindow (double v)
{
  NS_LOG_FUNCTION (this);
  m_timeWindow = v;
}

double
NrMacSchedulerOfdmaPF::GetTimeWindow () const
{
  NS_LOG_FUNCTION (this);
  return m_timeWindow;
}

std::shared_ptr<NrMacSchedulerUeInfo>
NrMacSchedulerOfdmaPF::CreateUeRepresentation (const NrMacCschedSapProvider::CschedUeConfigReqParameters &params) const
{
  NS_LOG_FUNCTION (this);
  return std::make_shared <NrMacSchedulerUeInfoPF> (m_alpha, params.m_rnti, params.m_beamConfId,
                                                        std::bind(&NrMacSchedulerOfdmaPF::GetNumRbPerRbg, this));
}

std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const NrMacSchedulerNs3::UePtrAndBufferReq &rhs )>
NrMacSchedulerOfdmaPF::GetUeCompareDlFn () const
{
  return NrMacSchedulerUeInfoPF::CompareUeWeightsDl;
}

std::function<bool (const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                    const NrMacSchedulerNs3::UePtrAndBufferReq &rhs)>
NrMacSchedulerOfdmaPF::GetUeCompareUlFn () const
{
  return NrMacSchedulerUeInfoPF::CompareUeWeightsUl;
}

void
NrMacSchedulerOfdmaPF::AssignedDlResources (const UePtrAndBufferReq &ue,
                                            [[maybe_unused]]const FTResources &assigned,
                                                  const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->UpdateDlPFMetric (totAssigned, m_timeWindow, m_dlAmc);
}

void
NrMacSchedulerOfdmaPF::NotAssignedDlResources (const NrMacSchedulerNs3::UePtrAndBufferReq &ue,
                                               [[maybe_unused]] const NrMacSchedulerNs3::FTResources &assigned,
                                                   const NrMacSchedulerNs3::FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->UpdateDlPFMetric (totAssigned, m_timeWindow, m_dlAmc);
}

void
NrMacSchedulerOfdmaPF::AssignedUlResources (const UePtrAndBufferReq &ue,
                                            [[maybe_unused]] const FTResources &assigned,
                                                const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->UpdateUlPFMetric (totAssigned, m_timeWindow, m_ulAmc);
}

void
NrMacSchedulerOfdmaPF::NotAssignedUlResources (const NrMacSchedulerNs3::UePtrAndBufferReq &ue,
                                               [[maybe_unused]] const NrMacSchedulerNs3::FTResources &assigned,
                                                   const NrMacSchedulerNs3::FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->UpdateUlPFMetric (totAssigned, m_timeWindow, m_ulAmc);
}

void
NrMacSchedulerOfdmaPF::BeforeDlSched (const UePtrAndBufferReq &ue,
                                          const FTResources &assignableInIteration) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->CalculatePotentialTPutDl (assignableInIteration, m_dlAmc);
}

void
NrMacSchedulerOfdmaPF::BeforeUlSched (const UePtrAndBufferReq &ue,
                                          const FTResources &assignableInIteration) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->CalculatePotentialTPutUl (assignableInIteration, m_ulAmc);
}

} // namespace ns3

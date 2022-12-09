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
#include "nr-mac-scheduler-tdma-pf.h"
#include "nr-mac-scheduler-ue-info-pf.h"
#include <ns3/log.h>
#include <ns3/double.h>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerTdmaPF");
NS_OBJECT_ENSURE_REGISTERED (NrMacSchedulerTdmaPF);

TypeId
NrMacSchedulerTdmaPF::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMacSchedulerTdmaPF")
    .SetParent<NrMacSchedulerTdmaRR> ()
    .AddConstructor<NrMacSchedulerTdmaPF> ()
    .AddAttribute ("FairnessIndex",
                   "Value (between 0 and 1) that defines the PF metric (1 is the traditional 3GPP PF, 0 is RR in throughput",
                   DoubleValue (1),
                   MakeDoubleAccessor (&NrMacSchedulerTdmaPF::SetFairnessIndex,
                                       &NrMacSchedulerTdmaPF::GetFairnessIndex),
                   MakeDoubleChecker<double> (0, 1))
    .AddAttribute ("LastAvgTPutWeight",
                   "Weight of the last average throughput in the average throughput calculation",
                   DoubleValue (99),
                   MakeDoubleAccessor (&NrMacSchedulerTdmaPF::SetTimeWindow,
                                       &NrMacSchedulerTdmaPF::GetTimeWindow),
                   MakeDoubleChecker<double> (0))
  ;
  return tid;
}

NrMacSchedulerTdmaPF::NrMacSchedulerTdmaPF () : NrMacSchedulerTdmaRR ()
{
  NS_LOG_FUNCTION (this);
}

void
NrMacSchedulerTdmaPF::SetFairnessIndex (double v)
{
  NS_LOG_FUNCTION (this);
  m_alpha = v;
}

double
NrMacSchedulerTdmaPF::GetFairnessIndex () const
{
  NS_LOG_FUNCTION (this);
  return m_alpha;
}

void
NrMacSchedulerTdmaPF::SetTimeWindow (double v)
{
  NS_LOG_FUNCTION (this);
  m_timeWindow = v;
}

double
NrMacSchedulerTdmaPF::GetTimeWindow () const
{
  NS_LOG_FUNCTION (this);
  return m_timeWindow;
}

std::shared_ptr<NrMacSchedulerUeInfo>
NrMacSchedulerTdmaPF::CreateUeRepresentation (const NrMacCschedSapProvider::CschedUeConfigReqParameters &params) const
{
  NS_LOG_FUNCTION (this);
  return std::make_shared <NrMacSchedulerUeInfoPF> (m_alpha, params.m_rnti, params.m_beamConfId,
                                                        std::bind (&NrMacSchedulerTdmaPF::GetNumRbPerRbg, this));
}

std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const NrMacSchedulerNs3::UePtrAndBufferReq &rhs )>
NrMacSchedulerTdmaPF::GetUeCompareDlFn () const
{
  return NrMacSchedulerUeInfoPF::CompareUeWeightsDl;
}

std::function<bool (const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                    const NrMacSchedulerNs3::UePtrAndBufferReq &rhs)>
NrMacSchedulerTdmaPF::GetUeCompareUlFn () const
{
  return NrMacSchedulerUeInfoPF::CompareUeWeightsUl;
}

void
NrMacSchedulerTdmaPF::AssignedDlResources (const UePtrAndBufferReq &ue,
                                           [[maybe_unused]] const FTResources &assigned,
                                           const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->UpdateDlPFMetric (totAssigned, m_timeWindow, m_dlAmc);
}

void
NrMacSchedulerTdmaPF::NotAssignedDlResources (const NrMacSchedulerNs3::UePtrAndBufferReq &ue,
                                              [[maybe_unused]] const NrMacSchedulerNs3::FTResources &notAssigned,
                                              const NrMacSchedulerNs3::FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->UpdateDlPFMetric (totAssigned, m_timeWindow, m_dlAmc);
}

void
NrMacSchedulerTdmaPF::AssignedUlResources (const UePtrAndBufferReq &ue,
                                           [[maybe_unused]] const FTResources &assigned,
                                           const FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->UpdateUlPFMetric (totAssigned, m_timeWindow, m_ulAmc);
}

void
NrMacSchedulerTdmaPF::NotAssignedUlResources (const NrMacSchedulerNs3::UePtrAndBufferReq &ue,
                                              [[maybe_unused]] const NrMacSchedulerNs3::FTResources &notAssigned,
                                              const NrMacSchedulerNs3::FTResources &totAssigned) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->UpdateUlPFMetric (totAssigned, m_timeWindow, m_ulAmc);
}

void
NrMacSchedulerTdmaPF::BeforeDlSched (const UePtrAndBufferReq &ue,
                                     const FTResources &assignableInIteration) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->CalculatePotentialTPutDl (assignableInIteration, m_dlAmc);
}

void
NrMacSchedulerTdmaPF::BeforeUlSched (const UePtrAndBufferReq &ue,
                                     const FTResources &assignableInIteration) const
{
  NS_LOG_FUNCTION (this);
  auto uePtr = std::dynamic_pointer_cast<NrMacSchedulerUeInfoPF> (ue.first);
  uePtr->CalculatePotentialTPutUl (assignableInIteration, m_ulAmc);
}

} //namespace ns3

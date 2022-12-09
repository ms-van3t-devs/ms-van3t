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
#include "nr-mac-scheduler-ofdma-mr.h"
#include "nr-mac-scheduler-ue-info-mr.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerOfdmaMR");
NS_OBJECT_ENSURE_REGISTERED (NrMacSchedulerOfdmaMR);

TypeId
NrMacSchedulerOfdmaMR::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMacSchedulerOfdmaMR")
    .SetParent<NrMacSchedulerOfdmaRR> ()
    .AddConstructor<NrMacSchedulerOfdmaMR> ()
  ;
  return tid;
}

NrMacSchedulerOfdmaMR::NrMacSchedulerOfdmaMR () : NrMacSchedulerOfdmaRR ()
{

}

std::shared_ptr<NrMacSchedulerUeInfo>
NrMacSchedulerOfdmaMR::CreateUeRepresentation (const NrMacCschedSapProvider::CschedUeConfigReqParameters &params) const
{
  NS_LOG_FUNCTION (this);
  return std::make_shared <NrMacSchedulerUeInfoMR> (params.m_rnti, params.m_beamConfId,
                                                        std::bind (&NrMacSchedulerOfdmaMR::GetNumRbPerRbg, this));
}

std::function<bool(const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                   const NrMacSchedulerNs3::UePtrAndBufferReq &rhs )>
NrMacSchedulerOfdmaMR::GetUeCompareDlFn () const
{
  return NrMacSchedulerUeInfoMR::CompareUeWeightsDl;
}

std::function<bool (const NrMacSchedulerNs3::UePtrAndBufferReq &lhs,
                    const NrMacSchedulerNs3::UePtrAndBufferReq &rhs)>
NrMacSchedulerOfdmaMR::GetUeCompareUlFn () const
{
  return NrMacSchedulerUeInfoMR::CompareUeWeightsUl;
}

} // namespace ns3

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

#include "nr-sl-ue-mac-scheduler-dst-info.h"
#include <ns3/log.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeMacSchedulerDstInfo");

NrSlUeMacSchedulerDstInfo::NrSlUeMacSchedulerDstInfo (uint32_t dstL2Id)
  : m_dstL2Id (dstL2Id)
{
}

NrSlUeMacSchedulerDstInfo::~NrSlUeMacSchedulerDstInfo ()
{
}

std::unordered_map<uint8_t, NrSlLCGPtr> &
NrSlUeMacSchedulerDstInfo::GetNrSlLCG ()
{
  return m_nrSlLCG;
}

NrSlLCGIt
NrSlUeMacSchedulerDstInfo::Insert (NrSlLCGPtr && lcg)
{
  std::pair<NrSlLCGIt, bool> ret;
  ret = m_nrSlLCG.emplace (std::make_pair (lcg->m_id, std::move (lcg)));
  bool insertStatus = ret.second;
  NS_ASSERT_MSG (insertStatus, "Destination " << m_dstL2Id << " already contains LCG ID " << +ret.first->second->m_id);
  return ret.first;
}

uint32_t
NrSlUeMacSchedulerDstInfo::GetDstL2Id () const
{
  return m_dstL2Id;
}

void
NrSlUeMacSchedulerDstInfo::SetDstMcs (uint8_t mcs)
{
  m_mcs = mcs;
}

uint8_t
NrSlUeMacSchedulerDstInfo::GetDstMcs () const
{
  return m_mcs;
}


} // namespace ns3

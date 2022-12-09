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

#include "nr-sl-ue-mac-scheduler.h"

#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeMacScheduler");
NS_OBJECT_ENSURE_REGISTERED (NrSlUeMacScheduler);

TypeId
NrSlUeMacScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlUeMacScheduler")
    .SetParent<Object> ()
    .SetGroupName ("nr")
  ;

  return tid;
}

NrSlUeMacScheduler::NrSlUeMacScheduler ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_nrSlUeMacSchedSapProvider = new NrSlUeMacGeneralSchedSapProvider (this);
  m_nrSlUeMacCschedSapProvider = new NrSlUeMacGeneralCschedSapProvider (this);
}

NrSlUeMacScheduler::~NrSlUeMacScheduler ()
{
  NS_LOG_FUNCTION_NOARGS ();
  delete m_nrSlUeMacSchedSapProvider;
  m_nrSlUeMacSchedSapProvider = nullptr;

  delete m_nrSlUeMacCschedSapProvider;
  m_nrSlUeMacCschedSapProvider = nullptr;
}


void
NrSlUeMacScheduler::SetNrSlUeMacSchedSapUser (NrSlUeMacSchedSapUser* sap)
{
  m_nrSlUeMacSchedSapUser = sap;
}


NrSlUeMacSchedSapProvider*
NrSlUeMacScheduler::GetNrSlUeMacSchedSapProvider ()
{
  return m_nrSlUeMacSchedSapProvider;
}

void
NrSlUeMacScheduler::SetNrSlUeMacCschedSapUser (NrSlUeMacCschedSapUser* sap)
{
  m_nrSlUeMacCschedSapUser = sap;
}

NrSlUeMacCschedSapProvider*
NrSlUeMacScheduler::GetNrSlUeMacCschedSapProvider ()
{
  return m_nrSlUeMacCschedSapProvider;
}



//CSCHED API primitives for NR Sidelink
NrSlUeMacGeneralCschedSapProvider::NrSlUeMacGeneralCschedSapProvider (NrSlUeMacScheduler* scheduler)
  : m_scheduler (scheduler)
{
}

void
NrSlUeMacGeneralCschedSapProvider::CschedUeNrSlLcConfigReq (const struct NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params)
{
  m_scheduler->DoCschedUeNrSlLcConfigReq (params);
}

//SCHED API primitives for NR Sidelink
NrSlUeMacGeneralSchedSapProvider::NrSlUeMacGeneralSchedSapProvider (NrSlUeMacScheduler* sched)
  : m_scheduler (sched)
{
}

void
NrSlUeMacGeneralSchedSapProvider::SchedUeNrSlRlcBufferReq (const struct NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams& params)
{
  m_scheduler->DoSchedUeNrSlRlcBufferReq (params);
}
void
NrSlUeMacGeneralSchedSapProvider::SchedUeNrSlTriggerReq (uint32_t dstL2Id, const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& params)
{
  m_scheduler->DoSchedUeNrSlTriggerReq (dstL2Id, params);
}

} // namespace ns3



/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "realistic-bf-manager.h"
#include <ns3/uinteger.h>
#include <ns3/enum.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RealisticBfManager");
NS_OBJECT_ENSURE_REGISTERED (RealisticBfManager);

RealisticBfManager::RealisticBfManager()
{
}

RealisticBfManager::~RealisticBfManager() {
  // TODO Auto-generated RealisticBfManager stub
}

TypeId
RealisticBfManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RealisticBfManager")
                     .SetParent<BeamManager> ()
                     .AddConstructor<RealisticBfManager> ()
                     .AddAttribute ("TriggerEvent",
                                    "Defines a beamforming trigger event",
                                    EnumValue (RealisticBfManager::SRS_COUNT),
                                    MakeEnumAccessor (&RealisticBfManager::SetTriggerEvent,
                                                      &RealisticBfManager::GetTriggerEvent),
                                    MakeEnumChecker (RealisticBfManager::SRS_COUNT, "SrsCount",
                                                     RealisticBfManager::DELAYED_UPDATE, "DelayedUpdate"))
                    .AddAttribute ("UpdatePeriodicity",
                                   "Interval between consecutive beamforming update method executions expressed in the "
                                   "number of SRS periodicities to wait before triggering the next beamforming update.",
                                   UintegerValue (1),
                                   MakeUintegerAccessor (&RealisticBfManager::SetUpdatePeriodicity,
                                                         &RealisticBfManager::GetUpdatePeriodicity),
                                   MakeUintegerChecker <uint16_t>())
                    .AddAttribute ("UpdateDelay",
                                   "Delay between SRS SINR report and the beamforming vectors update. Should be lower "
                                   "then SRS periodicity in slots, otherwise the SRS SINR being used will be the latest "
                                   "received.",
                                   TimeValue (MilliSeconds (10)),
                                   MakeTimeAccessor (&RealisticBfManager::SetUpdateDelay,
                                                     &RealisticBfManager::GetUpdateDelay),
                                   MakeTimeChecker());
  return tid;
}


void
RealisticBfManager::SetTriggerEvent (RealisticBfManager::TriggerEvent triggerEvent)
{
  m_triggerEvent = triggerEvent;
}

RealisticBfManager::TriggerEvent
RealisticBfManager::GetTriggerEvent () const
{
  return m_triggerEvent;
}

void
RealisticBfManager::SetUpdatePeriodicity (uint16_t periodicity)
{
  m_updatePeriodicity = periodicity;
}

uint16_t
RealisticBfManager::GetUpdatePeriodicity () const
{
  return m_updatePeriodicity;
}

void
RealisticBfManager::SetUpdateDelay (Time delay)
{
  m_updateDelay = delay;
}

Time
RealisticBfManager::GetUpdateDelay () const
{
  return m_updateDelay;
}

} /* namespace ns3 */

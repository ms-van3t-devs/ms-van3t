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

#include "ideal-beamforming-helper.h"
#include <ns3/object-factory.h>
#include <ns3/ideal-beamforming-algorithm.h>
#include <ns3/log.h>
#include <ns3/nr-gnb-net-device.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/nr-gnb-phy.h>
#include <ns3/nr-ue-phy.h>
#include <ns3/beam-manager.h>
#include <ns3/vector.h>

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("IdealBeamformingHelper");
NS_OBJECT_ENSURE_REGISTERED (IdealBeamformingHelper);


IdealBeamformingHelper::IdealBeamformingHelper ()
{
  NS_LOG_FUNCTION (this);
}

IdealBeamformingHelper::~IdealBeamformingHelper ()
{
  NS_LOG_FUNCTION (this);
  m_beamformingAlgorithm = nullptr;
}

void
IdealBeamformingHelper::DoInitialize ()
{
  m_beamformingTimer = Simulator::Schedule (m_beamformingPeriodicity,
                                              &IdealBeamformingHelper::ExpireBeamformingTimer, this);
  BeamformingHelperBase::DoInitialize ();
}

TypeId
IdealBeamformingHelper::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::IdealBeamformingHelper")
      .SetParent<BeamformingHelperBase> ()
      .AddConstructor<IdealBeamformingHelper> ()
      .AddAttribute ("BeamformingMethod",
                     "Type of the ideal beamforming method in the case that it is enabled, by default is \"cell scan\" method.",
                      TypeIdValue (CellScanBeamforming::GetTypeId ()),
                      MakeTypeIdAccessor (&IdealBeamformingHelper::SetBeamformingMethod),
                      MakeTypeIdChecker ())
      .AddAttribute ("BeamformingPeriodicity",
                     "Interval between consecutive beamforming method executions. If set to 0 it will not be updated.",
                      TimeValue (MilliSeconds (100)),
                      MakeTimeAccessor (&IdealBeamformingHelper::SetPeriodicity,
                                        &IdealBeamformingHelper::GetPeriodicity),
                      MakeTimeChecker())
      ;
    return tid;
}

void
IdealBeamformingHelper::AddBeamformingTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                            const Ptr<NrUeNetDevice>& ueDev)
{
  NS_LOG_FUNCTION (this);

  if (!m_beamformingAlgorithm)
    {
      m_beamformingAlgorithm = m_algorithmFactory.Create<IdealBeamformingAlgorithm> ();
    }

  BeamformingHelperBase::AddBeamformingTask (gNbDev, ueDev);

  for (uint8_t ccId = 0; ccId < gNbDev->GetCcMapSize () ; ccId++)
    {
      RunTask (gNbDev, ueDev, ccId); // run immediately the task, and next time will be according to configured periodicity
    }
}

void
IdealBeamformingHelper::Run () const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Running the beamforming method. There are :" <<
               m_beamformingTasks.size()<<" tasks.");

  for (const auto& task : m_beamformingTasks)
    {
      NS_LOG_INFO ("There are :" << task.first->GetCcMapSize ()<< " antennas per device.");
      Ptr<NrGnbNetDevice> gNbDev = task.first;
      Ptr<NrUeNetDevice> ueDev = task.second;

      for (uint8_t ccId = 0; ccId < gNbDev->GetCcMapSize () ; ccId++)
        {
          RunTask (gNbDev, ueDev, ccId);
        }
    }
}


void
IdealBeamformingHelper::GetBeamformingVectors (const Ptr<NrGnbNetDevice>& gnbDev,
                                              const Ptr<NrUeNetDevice>& ueDev,
                                              BeamformingVector* gnbBfv,
                                              BeamformingVector* ueBfv,
                                              uint16_t ccId) const
{
  NS_LOG_FUNCTION (this);
  m_beamformingAlgorithm->GetBeamformingVectors (gnbDev, ueDev, gnbBfv, ueBfv, ccId);
}

void
IdealBeamformingHelper::SetBeamformingMethod (const TypeId &beamformingMethod)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (beamformingMethod.IsChildOf (IdealBeamformingAlgorithm::GetTypeId ()));
  m_algorithmFactory.SetTypeId (beamformingMethod);
}

void
IdealBeamformingHelper::ExpireBeamformingTimer ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Beamforming timer expired; programming a beamforming");

  Run (); //Run beamforming tasks
  m_beamformingTimer.Cancel (); // Cancel any previous beamforming event
  m_beamformingTimer = Simulator::Schedule (m_beamformingPeriodicity,
                                            &IdealBeamformingHelper::ExpireBeamformingTimer, this);
}

void
IdealBeamformingHelper::SetPeriodicity (const Time &v)
{
  NS_LOG_FUNCTION (this);
  NS_ABORT_MSG_IF ( v == MilliSeconds (0), "Periodicity must be greater than 0 ms.");
  m_beamformingPeriodicity = v;
}

Time
IdealBeamformingHelper::GetPeriodicity () const
{
  NS_LOG_FUNCTION (this);
  return m_beamformingPeriodicity;
}


}

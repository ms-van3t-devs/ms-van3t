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

#include "realistic-beamforming-helper.h"
#include <ns3/log.h>
#include <ns3/nr-gnb-net-device.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/nr-gnb-phy.h>
#include <ns3/nr-ue-phy.h>
#include <ns3/beam-manager.h>
#include <ns3/vector.h>
#include <ns3/uinteger.h>
#include <ns3/lte-ue-rrc.h>
#include <ns3/nr-phy-mac-common.h>
#include <ns3/nr-spectrum-phy.h>
#include <ns3/nr-mac-scheduler.h>

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("RealisticBeamformingHelper");
NS_OBJECT_ENSURE_REGISTERED (RealisticBeamformingHelper);


TypeId
RealisticBeamformingHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RealisticBeamformingHelper")
                      .SetParent<BeamformingHelperBase> ()
                      .AddConstructor<RealisticBeamformingHelper> ();
  return tid;
}



void
RealisticBeamformingHelper::AddBeamformingTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                                const Ptr<NrUeNetDevice>& ueDev)
{
  NS_LOG_FUNCTION (this);
  for (uint8_t ccId = 0; ccId < gNbDev->GetCcMapSize () ; ccId++)
    {
      uint8_t gnbAntennaArrays = gNbDev->GetPhy (ccId)->GetNumberOfStreams ();
      uint8_t ueAntennaArrays = ueDev->GetPhy (ccId)->GetNumberOfStreams ();
      uint8_t arrays = std::min (gnbAntennaArrays, ueAntennaArrays);
      NS_ASSERT (arrays);

      //TODO add assert to check if they are of the same polarization

      for (uint8_t arrayIndex = 0; arrayIndex < arrays; arrayIndex++)
        {
          Ptr<NrSpectrumPhy> gnbSpectrumPhy = gNbDev->GetPhy (ccId)->GetSpectrumPhy (arrayIndex);
          Ptr<NrSpectrumPhy> ueSpectrumPhy = ueDev->GetPhy (ccId)->GetSpectrumPhy (arrayIndex);

          auto itAlgorithms = m_antennaPairToAlgorithm.find(std::make_pair (gnbSpectrumPhy, ueSpectrumPhy));
          NS_ABORT_MSG_IF ( itAlgorithms != m_antennaPairToAlgorithm.end(),
                            "Realistic beamforming task already created for the provided devices");

          // for each pair of antenna arrays of transmiter and receiver create an instance of beamforming algorithm
          Ptr<RealisticBeamformingAlgorithm> beamformingAlgorithm = m_algorithmFactory.Create<RealisticBeamformingAlgorithm> ();

          Ptr<NrMacScheduler> sched = gNbDev ->GetScheduler (ccId);

          beamformingAlgorithm->Install (gNbDev,
                                         ueDev,
                                         gnbSpectrumPhy,
                                         ueSpectrumPhy,
                                         sched
                                         );

          m_antennaPairToAlgorithm [std::make_pair(gnbSpectrumPhy, ueSpectrumPhy)] = beamformingAlgorithm;
          //connect trace of the corresponding gNB PHY to the RealisticBeamformingAlgorithm funcition
          gnbSpectrumPhy->AddSrsSinrReportCallback (MakeCallback (&RealisticBeamformingAlgorithm::NotifySrsSinrReport, beamformingAlgorithm));
          gnbSpectrumPhy->AddSrsSnrReportCallback (MakeCallback (&RealisticBeamformingAlgorithm::NotifySrsSnrReport, beamformingAlgorithm));
          beamformingAlgorithm->SetTriggerCallback (MakeCallback (&RealisticBeamformingHelper::RunTask, this));
        }
    }
}


BeamformingVectorPair
RealisticBeamformingHelper::GetBeamformingVectors (const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
                                                   const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const
{
  auto itAlgo = m_antennaPairToAlgorithm.find(std::make_pair(gnbSpectrumPhy, ueSpectrumPhy));
  NS_ABORT_MSG_IF (itAlgo == m_antennaPairToAlgorithm.end(), "There is no created task/algorithm for the specified pair of antenna arrays.");
  return itAlgo->second->GetBeamformingVectors ();
}

void
RealisticBeamformingHelper::SetBeamformingMethod (const TypeId &beamformingMethod)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (beamformingMethod == RealisticBeamformingAlgorithm::GetTypeId () ||
             beamformingMethod.IsChildOf (RealisticBeamformingAlgorithm::GetTypeId ()));

  m_algorithmFactory.SetTypeId (beamformingMethod);
}


}

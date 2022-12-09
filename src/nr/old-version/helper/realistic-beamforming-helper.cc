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

  auto itAlgorithms = m_devicePairToAlgorithmsPerCcId.find(std::make_pair(gNbDev, ueDev));
  NS_ABORT_MSG_IF ( itAlgorithms != m_devicePairToAlgorithmsPerCcId.end(), "Realistic beamforming task already created for the provided devices");

  // create new element in the map that
  m_devicePairToAlgorithmsPerCcId [std::make_pair(gNbDev, ueDev)] = CcIdToBeamformingAlgorithm ();

  for (uint8_t ccId = 0; ccId < gNbDev->GetCcMapSize () ; ccId++)
    {
      Ptr<RealisticBeamformingAlgorithm> beamformingAlgorithm = m_algorithmFactory.Create<RealisticBeamformingAlgorithm> ();
      beamformingAlgorithm->Install (gNbDev, ueDev, ccId);
      m_devicePairToAlgorithmsPerCcId [std::make_pair(gNbDev, ueDev)] [ccId] = beamformingAlgorithm;
      //connect trace of the corresponding gNB PHY to the RealisticBeamformingAlgorithm funcition
      gNbDev->GetPhy (ccId)->GetSpectrumPhy ()->AddSrsSinrReportCallback (MakeCallback (&RealisticBeamformingAlgorithm::NotifySrsSinrReport, beamformingAlgorithm));
      gNbDev->GetPhy (ccId)->GetSpectrumPhy ()->AddSrsSnrReportCallback (MakeCallback (&RealisticBeamformingAlgorithm::NotifySrsSnrReport, beamformingAlgorithm));
      beamformingAlgorithm->SetTriggerCallback (MakeCallback (&RealisticBeamformingHelper::RunTask, this));
    }

  BeamformingHelperBase ::AddBeamformingTask (gNbDev, ueDev);
}


void
RealisticBeamformingHelper::GetBeamformingVectors (const Ptr<NrGnbNetDevice>& gnbDev,
                                                   const Ptr<NrUeNetDevice>& ueDev,
                                                   BeamformingVector* gnbBfv,
                                                   BeamformingVector* ueBfv,
                                                   uint16_t ccId) const
{

  auto itDevPair = m_devicePairToAlgorithmsPerCcId.find(std::make_pair(gnbDev, ueDev));
  NS_ABORT_MSG_IF (itDevPair == m_devicePairToAlgorithmsPerCcId.end(), "There is no task for the specified pair of devices.");

  auto itAlgorithm = (itDevPair->second).find (ccId);;
  NS_ABORT_MSG_IF (itAlgorithm == (itDevPair->second).end (), "There is no BF task for the specified component carrier ID.");

  itAlgorithm->second->GetBeamformingVectors (gnbDev, ueDev, gnbBfv, ueBfv, ccId);
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

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

#include "beamforming-helper-base.h"
#include <ns3/log.h>
#include <ns3/nr-gnb-net-device.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/nr-gnb-phy.h>
#include <ns3/nr-ue-phy.h>
#include <ns3/beam-manager.h>
#include <ns3/beamforming-algorithm.h>
#include <ns3/vector.h>
#include <ns3/node.h>

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("BeamformingHelperBase");
NS_OBJECT_ENSURE_REGISTERED (BeamformingHelperBase);


BeamformingHelperBase::BeamformingHelperBase ()
{
  // TODO Auto-generated constructor stub
  NS_LOG_FUNCTION (this);
}

BeamformingHelperBase::~BeamformingHelperBase ()
{
  // TODO Auto-generated destructor stub
  NS_LOG_FUNCTION (this);
}

TypeId
BeamformingHelperBase::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::BeamformingHelperBase")
      .SetParent<Object> ()
      ;
  return tid;
}

void
BeamformingHelperBase::AddBeamformingTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                           const Ptr<NrUeNetDevice>& ueDev)
{
  NS_LOG_FUNCTION (this);
  m_beamformingTasks.push_back(std::make_pair(gNbDev, ueDev));
}


void
BeamformingHelperBase::RunTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                const Ptr<NrUeNetDevice>& ueDev, uint8_t ccId) const
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO (" Run beamforming task for gNB:" << gNbDev->GetNode() -> GetId() <<
                 " and UE:"<< ueDev->GetNode()->GetId () );
  BeamformingVector gnbBfv, ueBfv;
  GetBeamformingVectors (gNbDev, ueDev, &gnbBfv, &ueBfv, ccId);
  Ptr<NrGnbPhy> gNbPhy = gNbDev->GetPhy (ccId);
  Ptr<NrUePhy> uePhy = ueDev->GetPhy (ccId);
  NS_ABORT_IF (gNbPhy == nullptr || uePhy == nullptr);
  gNbPhy->GetBeamManager ()->SaveBeamformingVector (gnbBfv, ueDev);
  uePhy->GetBeamManager ()->SaveBeamformingVector (ueBfv, gNbDev);
  uePhy->GetBeamManager ()->ChangeBeamformingVector (gNbDev);
}

void
BeamformingHelperBase::SetBeamformingAlgorithmAttribute (const std::string &n, const AttributeValue &v)
{
  NS_LOG_FUNCTION (this);
  m_algorithmFactory.Set (n, v);
}


}

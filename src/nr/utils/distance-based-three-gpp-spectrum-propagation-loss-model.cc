/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "distance-based-three-gpp-spectrum-propagation-loss-model.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/simulator.h"
#include "ns3/pointer.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DistanceBasedThreeGppSpectrumPropagationLossModel");
NS_OBJECT_ENSURE_REGISTERED (DistanceBasedThreeGppSpectrumPropagationLossModel);

DistanceBasedThreeGppSpectrumPropagationLossModel::DistanceBasedThreeGppSpectrumPropagationLossModel ()
{
  NS_LOG_FUNCTION (this);
}

DistanceBasedThreeGppSpectrumPropagationLossModel::~DistanceBasedThreeGppSpectrumPropagationLossModel ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
DistanceBasedThreeGppSpectrumPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DistanceBasedThreeGppSpectrumPropagationLossModel")
    .SetParent<ThreeGppSpectrumPropagationLossModel> ()
    .SetGroupName ("Spectrum")
    .AddConstructor<DistanceBasedThreeGppSpectrumPropagationLossModel> ()
    .AddAttribute("MaxDistance",
                  "The maximum distance in meters between nodes in order to calculate fast fading and beamforming."
                  "For all signals for which nodes are at higher distance will be returned 0 PSD.",
                  DoubleValue (1000),
                  MakeDoubleAccessor (&DistanceBasedThreeGppSpectrumPropagationLossModel::SetMaxDistance,
                                      &DistanceBasedThreeGppSpectrumPropagationLossModel::GetMaxDistance),
                  MakeDoubleChecker<double> ())
    ;
  return tid;
}

void
DistanceBasedThreeGppSpectrumPropagationLossModel::SetMaxDistance (double maxDistance)
{
  m_maxDistance = maxDistance;
}

double
DistanceBasedThreeGppSpectrumPropagationLossModel::GetMaxDistance () const
{
  return m_maxDistance;
}


Ptr<SpectrumValue>
DistanceBasedThreeGppSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensity (Ptr<const SpectrumValue> txPsd,
                                                                                 Ptr<const MobilityModel> a,
                                                                                 Ptr<const MobilityModel> b,
                                                                                 Ptr<const PhasedArrayModel> aPhasedArrayModel,
                                                                                 Ptr<const PhasedArrayModel> bPhasedArrayModel) const
{
  NS_LOG_FUNCTION (this);
  uint32_t aId = a->GetObject<Node> ()->GetId (); // id of the node a
  uint32_t bId = b->GetObject<Node> ()->GetId (); // id of the node b

  Ptr<SpectrumValue> rxPsd = Copy<SpectrumValue> (txPsd);
  if (a->GetDistanceFrom (b) > m_maxDistance)
    {
      NS_LOG_LOGIC ("Distance between a: " << aId << "and  node b: "<<bId << " is higher than max allowed distance. Return 0 PSD.");
      *rxPsd = 0.0;
      return rxPsd;
    }
  else
    {
      return ThreeGppSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensity (txPsd, a, b, aPhasedArrayModel, bPhasedArrayModel);
    }

  return rxPsd;
}


}  // namespace ns3

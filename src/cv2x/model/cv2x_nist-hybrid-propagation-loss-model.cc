/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * 
 * We would appreciate acknowledgement if the software is used.
 * 
 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * 
 * Modified by: NIST
 * It was tested under ns-3.22
 */

#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/vector.h"
#include "ns3/box.h"
#include "ns3/mobility-model.h"
#include <cmath>
#include "cv2x_nist-hybrid-propagation-loss-model.h"
#include "ns3/mobility-building-info.h"
#include "ns3/node.h"
//#include <ns3/cv2x_lte-enb-net-device.h>
//#include <ns3/cv2x_lte-ue-net-device.h>

NS_LOG_COMPONENT_DEFINE ("cv2x_NistHybridPropagationLossModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_NistHybridPropagationLossModel);


TypeId
cv2x_NistHybridPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_NistHybridPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<cv2x_NistHybridPropagationLossModel> ()
    .AddAttribute ("Frequency",
                    "The propagation frequency in Hz",
                    DoubleValue (763e6),
                    MakeDoubleAccessor (&cv2x_NistHybridPropagationLossModel::m_frequency),
                    MakeDoubleChecker<double> ())
    ;

  return tid;
}

cv2x_NistHybridPropagationLossModel::cv2x_NistHybridPropagationLossModel ()
  : PropagationLossModel ()
{
  m_rand = CreateObject<UniformRandomVariable> ();
}

cv2x_NistHybridPropagationLossModel::~cv2x_NistHybridPropagationLossModel ()
{
}

double
cv2x_NistHybridPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{

  // Pathloss
  double loss = 0.0;
  // Frequency in GHz
  double fc = m_frequency / 1e9;  
  // Propagation velocity in free space
  double c = 3 * std::pow (10, 8);

  // Actual antenna heights (1.5 m for UEs)  
  double hms = a->GetPosition ().z;
  double hbs = b->GetPosition ().z;
  // Effective antenna heights (0.7 m for UEs)
  double hbs1 = hbs - 1;
  double hms1 = hms - 0.7;

  double d1 = 4 * hbs1 * hms1 * m_frequency * (1 / c);

  // Distance between the two nodes
  double dist = a->GetDistanceFrom (b);
  // Distance between the outdoor terminal and the point on the wall 
  // that is the nearest to the indoor terminal
  double dout = 0;
  // Distance from the wall to the indoor terminal
  double din = 0; 

  // Find the indoor node
  Ptr<MobilityModel> in;
  Ptr<MobilityModel> out;

  if (a->GetObject<MobilityBuildingInfo> ()->IsIndoor ())
  {
    in = a;
    out = b;
  }
  else 
  {
    in = b;
    out = a;
  }

  // Get the position vectors of the indoor and outdoor nodes
  Vector inPosition = in->GetPosition ();
  Vector outPosition = out->GetPosition ();
  // Get the boundaries of the building of the indoor node 
  Box inBox = in->GetObject<MobilityBuildingInfo> ()->GetBuilding ()->GetBoundaries ();
  // Calculate the din and the position of the nearest point on the wall to the indoor node
  Vector3D wallPosition;
  if (inBox.GetClosestSide (inPosition) == Box::LEFT)
  {
    din = std::abs (inPosition.x - inBox.xMin);
    Vector3D wallPosition (inBox.xMin, inPosition.y, inPosition.z);
  }
  else if (inBox.GetClosestSide (inPosition) == Box::RIGHT)
  {
    din = std::abs (inPosition.x - inBox.xMax);
    Vector3D wallPosition (inBox.xMax, inPosition.y, inPosition.z);
  }
  else if (inBox.GetClosestSide (inPosition) == Box::BOTTOM)
  {
    din = std::abs (inPosition.y - inBox.yMin);
    Vector3D wallPosition (inPosition.x, inBox.yMin, inPosition.z);
  }
  else if (inBox.GetClosestSide (inPosition) == Box::TOP)
  {
    din = std::abs (inPosition.y - inBox.yMax);
    Vector3D wallPosition (inPosition.x, inBox.yMax, inPosition.z);
  }
  else if (inBox.GetClosestSide (inPosition) == Box::DOWN)
  {
    din = std::abs (inPosition.z - inBox.zMin);
    Vector3D wallPosition (inPosition.x, inPosition.y, inBox.zMin);
  }
  else if (inBox.GetClosestSide (inPosition) == Box::UP)
  {
    din = std::abs (inPosition.z - inBox.zMax);
    Vector3D wallPosition (inPosition.x, inPosition.y, inBox.zMax);
  }
  // Calculate the dout
  dout = CalculateDistance (outPosition, wallPosition);

  // Calculate the pathloss based on 3GPP specifications : 3GPP TR 36.843 V12.0.1
  // WINNER II Channel Models, D1.1.2 V1.2., Equation (4.24) p.43, available at 
  // http://www.cept.org/files/1050/documents/winner2%20-%20final%20report.pdf
  double lossFree = 20 * std::log10 (dist) + 46.6 + 20 * std::log10 (fc / 5.0);

  // Calculate the LOS probability based on 3GPP specifications : 3GPP TR 36.843 V12.0.1
  // "Guidelines for evaluation of radio interface technologies for IMT-Advanced",
  // Draft new Report ITU-R M.[IMT.EVAL], Document 5/69-E.
  // for outdoor users only
  double plos = std::min ((18 / dist), 1.0) * (1 - std::exp (-dist / 36)) + std::exp (-dist / 36);
  
  // Generate a random number between 0 and 1 (if it doesn't already exist) to evaluate the LOS/NLOS situation
  double r = 0.0;

  MobilityDuo couple;
  couple.a = a;
  couple.b = b;
  std::map<MobilityDuo, double>::iterator it_a = m_randomMap.find (couple);
  if (it_a != m_randomMap.end ())
  {
    r = it_a->second;
  }
  else
  {
    couple.a = b;
    couple.b = a;
    std::map<MobilityDuo, double>::iterator it_b = m_randomMap.find (couple);
    if (it_b != m_randomMap.end ())
    {
      r = it_b->second;
    }
    else
    {
      m_randomMap[couple] = m_rand->GetValue (0,1);
      r = m_randomMap[couple];
    }
  }

  //std::cout << "hybrid = " << r << " - ";
  // LOS offset = LOS loss to add to the computed pathloss
  double los = 0;
  // NLOS offset = NLOS loss to add to the computed pathloss
  double nlos = 5;

  double lossOut = 0.0;

  if (r <= plos)
  {
    // LOS
    if (dist <= d1)
      {
        lossOut = 22.7 * std::log10 (din + dout) + 27.0 + 20.0 * std::log10 (fc) + los; 
      }
    else
      {
        lossOut = 40 * std::log10 (din + dout) + 7.56 - 17.3 * std::log10 (hbs1) - 17.3 * std::log10 (hms1) + 2.7 * std::log10 (fc) + los;  
      }

    lossOut = std::max (lossFree, lossOut);

    loss = lossOut + 20.0 + 0.5 * din;
    NS_LOG_INFO (this << "Hybrid LOS = " << loss);
  } 
  else
  {
    // NLOS
    if ((fc >= 0.758) and (fc <= 0.798))
    {
      // Frequency = 700 MHz for Public Safety
      lossOut = (44.9 - 6.55 * std::log10 (hbs)) * std::log10 (din + dout) + 5.83 * std::log10 (hbs) + 16.33 + 26.16 * std::log10 (fc) + nlos;    
    }
    if ((fc >= 1.92) and (fc <= 2.17))
    {
      // Frequency = 2 GHz for general scenarios
      lossOut = (44.9 - 6.55 * std::log10 (hbs)) * std::log10 (din + dout) + 5.83 * std::log10 (hbs) + 14.78 + 34.97 * std::log10 (fc) + nlos;
    }
  
    lossOut = std::max (lossFree, lossOut);

    loss = lossOut + 20.0 + 0.5 * din - 0.8 * hms;   
    NS_LOG_INFO (this << "Hybrid NLOS = " << loss);
  }

  return std::max (0.0, loss);
}

double 
cv2x_NistHybridPropagationLossModel::DoCalcRxPower (double txPowerDbm,
					       Ptr<MobilityModel> a,
					       Ptr<MobilityModel> b) const
{
  return (txPowerDbm - GetLoss (a, b));
}

int64_t
cv2x_NistHybridPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}


} // namespace ns3



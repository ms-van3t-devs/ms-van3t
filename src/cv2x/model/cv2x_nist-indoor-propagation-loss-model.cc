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
#include "ns3/mobility-model.h"
#include <cmath>
#include <ns3/mobility-building-info.h>
#include "cv2x_nist-indoor-propagation-loss-model.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE ("cv2x_NistIndoorPropagationLossModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_NistIndoorPropagationLossModel);

TypeId
cv2x_NistIndoorPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_NistIndoorPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<cv2x_NistIndoorPropagationLossModel> ()
    .AddAttribute ("Frequency",
                    "The propagation frequency in Hz",
                    DoubleValue (763e6),
                    MakeDoubleAccessor (&cv2x_NistIndoorPropagationLossModel::m_frequency),
                    MakeDoubleChecker<double> ())
    ;

  return tid;
}

cv2x_NistIndoorPropagationLossModel::cv2x_NistIndoorPropagationLossModel ()
  : PropagationLossModel ()
{ 
  m_rand = CreateObject<UniformRandomVariable> ();
}

cv2x_NistIndoorPropagationLossModel::~cv2x_NistIndoorPropagationLossModel ()
{
}

// Initialize the static member
//Ptr <UniformRandomVariable> RandomVariable::s_rand = CreateObject<UniformRandomVariable> ();

std::pair<double, bool> 
cv2x_NistIndoorPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  bool los = false;
  double loss = 0.0;
  double dist = a->GetDistanceFrom (b);
  // Calculate the pathloss based on 3GPP specifications : 3GPP TR 36.843 V12.0.1
  // The indoor model is defined by 3GPP TR 36.814 V9.0.0, Table A.2.1.1.5-1

  // Same building
  if (a->GetObject<MobilityBuildingInfo> ()->GetBuilding () == b->GetObject<MobilityBuildingInfo> ()->GetBuilding ())
  {
    // Computing the probability of line of sight (LOS)
    double plos = 0.0;
    if (dist <= 18)
    {
      plos = 1.0;
    }
    else if ((dist > 18) and (dist < 37))
    {
      plos = std::exp (-(dist - 18) / 27);
    }
    else 
    {
      plos = 0.5;
    }
  
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
    
    //std::cout << "indoor = " << r << " - ";
    // Computing the pathloss when the two nodes are in the same building
    
    if (r <= plos)
    {
      // LOS
      loss = 89.5 + 16.9 * std::log10 (dist * 1e-3);
      los = true;
      NS_LOG_INFO (this << "Indoor (Same building) LOS = " << loss);
    }
    else
    {
      // NLOS
      loss = 147.5 + 43.3 * std::log10 (dist * 1e-3);
      NS_LOG_INFO (this << "Indoor (Same building) NLOS = " << loss);
    }
  }

  // Different buildings
  else
  {
    // Computing the pathloss when the two nodes are in different buildings
    loss = std::max (131.1 + 42.8 * std::log10 (dist * 1e3), 147.4 + 43.3 * std::log10 (dist * 1e3));
    NS_LOG_INFO (this << "Indoor (Different buildings) = " << loss);
  }

  // Pathloss correction for Public Safety frequency 700 MHz
  if (((m_frequency / 1e9) >= 0.758) and ((m_frequency / 1e9) <= 0.798))
  {
    loss = loss + 20 * std::log10 (m_frequency / 2e9);
  }

  return std::make_pair(std::max (0.0, loss), los);
}

double 
cv2x_NistIndoorPropagationLossModel::DoCalcRxPower (double txPowerDbm,
					       Ptr<MobilityModel> a,
					       Ptr<MobilityModel> b) const
{
  return (txPowerDbm - GetLoss (a, b).first);
}

int64_t
cv2x_NistIndoorPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}


} // namespace ns3


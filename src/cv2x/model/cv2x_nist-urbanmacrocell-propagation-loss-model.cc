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
#include "cv2x_nist-urbanmacrocell-propagation-loss-model.h"
#include <ns3/node.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-ue-net-device.h>
#include <ns3/boolean.h>

NS_LOG_COMPONENT_DEFINE ("cv2x_NistUrbanmacrocellPropagationLossModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_NistUrbanmacrocellPropagationLossModel);


TypeId
cv2x_NistUrbanmacrocellPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_NistUrbanmacrocellPropagationLossModel")
    .SetParent<BuildingsPropagationLossModel> ()
    .AddConstructor<cv2x_NistUrbanmacrocellPropagationLossModel> ()
    .AddAttribute ("Frequency",
                    "The propagation frequency in Hz",
                    DoubleValue (700e6),
                    MakeDoubleAccessor (&cv2x_NistUrbanmacrocellPropagationLossModel::m_frequency),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("BuildingHeight",
                    "The average building heigth",
                    DoubleValue (20),
                    MakeDoubleAccessor (&cv2x_NistUrbanmacrocellPropagationLossModel::m_buildingHeight),
                    MakeDoubleChecker<double> ())
    .AddAttribute ("StreetWidth",
                    "The average street width",
                    DoubleValue (20),
                    MakeDoubleAccessor (&cv2x_NistUrbanmacrocellPropagationLossModel::m_streetWidth),
                    MakeDoubleChecker<double> ())    
    .AddAttribute ("ShadowingEnabled",
                   "Activate or deactivate the shadowing computation",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_NistUrbanmacrocellPropagationLossModel::m_isShadowingEnabled),
                   MakeBooleanChecker ())
    .AddAttribute ("LosEnabled",
                   "when Line-of-Sight is enabled, all UEs are considered in Los with eNB. Otherwise, use randomness",
                   BooleanValue (false),
                   MakeBooleanAccessor (&cv2x_NistUrbanmacrocellPropagationLossModel::m_isLosEnabled),
                   MakeBooleanChecker ())
    ;

  return tid;
}

cv2x_NistUrbanmacrocellPropagationLossModel::cv2x_NistUrbanmacrocellPropagationLossModel ()
  : BuildingsPropagationLossModel ()
{  
  m_rand = CreateObject<UniformRandomVariable> ();
  //BuildingsPropagationLossModel::m_isShadowingEnabled = m_isShadowingEnabled;
}

cv2x_NistUrbanmacrocellPropagationLossModel::~cv2x_NistUrbanmacrocellPropagationLossModel ()
{
}

double
cv2x_NistUrbanmacrocellPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  // Pathloss
  double loss = 0.0;
  // Frequency in GHz
  double fc = m_frequency / 1e9;
  // Distance between the two nodes in meter
  double dist = a->GetDistanceFrom (b);

  // Actual antenna heights
  double hms = 0;
  double hbs = 0;

  if (a->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0)
  {
    hms = a->GetPosition ().z;
  }
  else if (a->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteEnbNetDevice> () != 0)
  {
    hbs = a->GetPosition ().z;
  }
  else 
  {
    NS_FATAL_ERROR ("The node " << a->GetObject<Node> ()->GetId () << " has neither a cv2x_LteUeNetDevice nor a cv2x_LteEnbNetDevice ");
  }

  if (b->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0)
  {
    hms = b->GetPosition ().z;
  }
  else if (b->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteEnbNetDevice> () != 0)
  {
    hbs = b->GetPosition ().z;
  }
  else 
  {
    NS_FATAL_ERROR ("The node " << b->GetObject<Node> ()->GetId () << " has neither a cv2x_LteUeNetDevice nor a cv2x_LteEnbNetDevice ");
  }
  // Effective antenna heights
  double hbs1 = hbs - 1;
  double hms1 = hms - 1;
  // Propagation velocity in free space
  double c = 3 * std::pow (10, 8);

  double d1 = 4 * hbs1 * hms1 * m_frequency * (1 / c);

  // Calculate the LOS probability based on 3GPP specifications 
  // 3GPP TR 36.814 channel model for Urban Macrocell scenario (UMa) : Table B.1.2.1-2
  double plos = std::min ((18 / dist), 1.0) * (1 - std::exp (-dist / 63)) + std::exp (-dist / 63);

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

  /*
    std::map<Ptr<MobilityModel>, std::map<Ptr<MobilityModel>, double> >::iterator it = m_randomMap.find (a);
    if (it != m_randomMap.end ())
    {
      std::map<Ptr<MobilityModel>, double>::iterator it1 = it->second.find (b);
      if (it1 != it->second.end ())
      {
        r = it1->second;
      }
      else
      { 
        it->second[b] = m_rand->GetValue (0,1);
        r = it->second[b];
      }
    }
    else
    {
      m_randomMap[a][b] = m_rand->GetValue (0,1);
      r = m_randomMap[a][b];
    }
  */

  // Compute the pathloss based on 3GPP specifications
  // 3GPP TR 36.814 channel model for Urban Macrocell scenario (UMa) : Table B.1.2.1-1
  // This model is only valid to a minimum distance of 10 meters 
  if (dist >= 10)
  {
    if ((r <= plos) or (m_isLosEnabled)) 
    {
      // LOS
      if (dist <= d1)
      {
        loss = 22 * std::log10 (dist) + 28.0 + 20.0 * std::log10 (fc);
        NS_LOG_INFO (this << "Urban macrocell LOS (Distance <= " << d1 << ") : the pathloss = " << loss);
      }
      else
      {
        loss = 40.0 * std::log10 (dist) + 7.8 - 18.0 * std::log10 (hbs1) - 18.0 * std::log10 (hms1) + 2.0 * std::log10 (fc);
        NS_LOG_INFO (this << "Urban macrocell LOS (Distance > " << d1 << ") : the pathloss = " << loss);
      }
    }
    else 
    {
      // NLOS
      loss = 161.04 - 7.1 * std::log10 (m_streetWidth) + 7.5 * std::log10 (m_buildingHeight) - (24.37 - 3.7 * (std::pow (m_buildingHeight/hbs, 2)) * std::log10 (hbs)) + (43.42 + 3.1 * std::log10 (hbs))* (std::log10 (dist) - 3) + 20.0 * std::log10 (fc) - (3.2 * std::pow(std::log10 (11.75 * hms), 2) - 4.97);
      NS_LOG_INFO (this << "Urban macrocell NLOS, the pathloss = " << loss);
    }  
  }
  return std::max (0.0, loss);
}

cv2x_NistUrbanmacrocellPropagationLossModel::PathlossType
cv2x_NistUrbanmacrocellPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b, bool check)
{
  // Frequency in GHz
  double fc = m_frequency / 1e9;
  // Distance between the two nodes in meter
  double dist = a->GetDistanceFrom (b);

  // Actual antenna heights
  double hms = 0;
  double hbs = 0;

  if (a->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0)
  {
    hms = a->GetPosition ().z;
  }
  else if (a->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteEnbNetDevice> () != 0)
  {
    hbs = a->GetPosition ().z;
  }
  else 
  {
    NS_FATAL_ERROR ("The node " << a->GetObject<Node> ()->GetId () << " has neither a cv2x_LteUeNetDevice nor a cv2x_LteEnbNetDevice ");
  }

  if (b->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0)
  {
    hms = b->GetPosition ().z;
  }
  else if (b->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteEnbNetDevice> () != 0)
  {
    hbs = b->GetPosition ().z;
  }
  else 
  {
    NS_FATAL_ERROR ("The node " << b->GetObject<Node> ()->GetId () << " has neither a cv2x_LteUeNetDevice nor a cv2x_LteEnbNetDevice ");
  }
  // Effective antenna heights
  double hbs1 = hbs - 1;
  double hms1 = hms - 1;
  // Propagation velocity in free space
  double c = 3 * std::pow (10, 8);

  double d1 = 4 * hbs1 * hms1 * m_frequency * (1 / c);

  // Calculate the LOS probability based on 3GPP specifications 
  // 3GPP TR 36.814 channel model for Urban Macrocell scenario (UMa) : Table B.1.2.1-2
  double plos = std::min ((18 / dist), 1.0) * (1 - std::exp (-dist / 63)) + std::exp (-dist / 63);

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
  
  // Compute the pathloss based on 3GPP specifications
  // 3GPP TR 36.814 channel model for Urban Macrocell scenario (UMa) : Table B.1.2.1-1
  // This model is only valid to a minimum distance of 10 meters 
  
  // Pathloss
  double loss = 0.0;

  if (dist >= 10)
  {
    if ((r <= plos) or (m_isLosEnabled)) 
    {
      // LOS
      if (dist <= d1)
      {
        loss = 22 * std::log10 (dist) + 28.0 + 20.0 * std::log10 (fc);
        NS_LOG_INFO (this << "Urban macrocell LOS (Distance <= " << d1 << ") : the pathloss = " << loss);
      }
      else
      {
        loss = 40.0 * std::log10 (dist) + 7.8 - 18.0 * std::log10 (hbs1) - 18.0 * std::log10 (hms1) + 2.0 * std::log10 (fc);
        NS_LOG_INFO (this << "Urban macrocell LOS (Distance > " << d1 << ") : the pathloss = " << loss);
      }
    }
    else 
    {
      // NLOS
      loss = 161.04 - 7.1 * std::log10 (m_streetWidth) + 7.5 * std::log10 (m_buildingHeight) - (24.37 - 3.7 * (std::pow (m_buildingHeight/hbs, 2)) * std::log10 (hbs)) + (43.42 + 3.1 * std::log10 (hbs))* (std::log10 (dist) - 3) + 20.0 * std::log10 (fc) - (3.2 * std::pow(std::log10 (11.75 * hms), 2) - 4.97);
      NS_LOG_INFO (this << "Urban macrocell NLOS, the pathloss = " << loss);
    }  
  }
  
  PathlossType triplet;
  triplet.loss = std::max (0.0, loss);
  triplet.plos = plos;
  triplet.r = r;
  return triplet;
}


double
cv2x_NistUrbanmacrocellPropagationLossModel::GetShadowing (Ptr<MobilityModel> a, Ptr<MobilityModel> b)
const
{
    Ptr<MobilityBuildingInfo> a1 = a->GetObject <MobilityBuildingInfo> ();
    Ptr<MobilityBuildingInfo> b1 = b->GetObject <MobilityBuildingInfo> ();
    NS_ASSERT_MSG ((a1 != 0) && (b1 != 0), "BuildingsPropagationLossModel only works with MobilityBuildingInfo");
  
  std::map<Ptr<MobilityModel>,  std::map<Ptr<MobilityModel>, double> >::iterator ait = m_shadowingLossMap.find (a);
  if (ait != m_shadowingLossMap.end ())
    {
      std::map<Ptr<MobilityModel>, double>::iterator bit = ait->second.find (b);
      if (bit != ait->second.end ())
        {
          return (bit->second);
        }
      else
        {
          double sigma = EvaluateSigma (a1, b1);
          // side effect: will create new entry          
          // sigma is standard deviation, not variance
          double shadowingValue = m_randVariable->GetValue (0.0, (sigma*sigma));
          ait->second[b] = shadowingValue;
          m_shadowingLossMap[b][a] = shadowingValue;
          return (shadowingValue);
        }
    }
  else
    {
      double sigma = EvaluateSigma (a1, b1);
      // side effect: will create new entries in both maps
      // sigma is standard deviation, not variance
      double shadowingValue = m_randVariable->GetValue (0.0, (sigma*sigma));
      m_shadowingLossMap[a][b] = shadowingValue; 
      m_shadowingLossMap[b][a] = shadowingValue;
      return (shadowingValue);       
    }
}


double
cv2x_NistUrbanmacrocellPropagationLossModel::EvaluateSigma (Ptr<MobilityBuildingInfo> a, Ptr<MobilityBuildingInfo> b) const
{
  if (m_isShadowingEnabled)
  {
    Ptr<MobilityModel> a1 = a->GetObject<MobilityModel> ();
    Ptr<MobilityModel> b1 = b->GetObject<MobilityModel> ();
    double dist = a1->GetDistanceFrom (b1);
    double plos = std::min ((18 / dist), 1.0) * (1 - std::exp (-dist / 63)) + std::exp (-dist / 63);
    double r = 0.0;
  
    MobilityDuo couple;
    couple.a = a1;
    couple.b = b1;
    std::map<MobilityDuo, double>::iterator it_a = m_randomMap.find (couple);
    if (it_a != m_randomMap.end ())
    {
      r = it_a->second;
    }
    else
    {
      couple.a = b1;
      couple.b = a1;
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
    if ((r <= plos) or (m_isLosEnabled))
      {
        // LOS
        return 4.0;
      }
    else 
      {
        // NLOS
        return 6.0;
      }  
  }
  return 0.0;
}

int64_t
cv2x_NistUrbanmacrocellPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}


} // namespace ns3

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
#include <ns3/boolean.h>
#include "cv2x_nist-scm-urbanmacrocell-propagation-loss-model.h"
#include <ns3/node.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-ue-net-device.h>

NS_LOG_COMPONENT_DEFINE ("cv2x_NistScmUrbanmacrocellPropagationLossModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_NistScmUrbanmacrocellPropagationLossModel);


TypeId
cv2x_NistScmUrbanmacrocellPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_NistScmUrbanmacrocellPropagationLossModel")
    .SetParent<BuildingsPropagationLossModel> ()
    .AddConstructor<cv2x_NistScmUrbanmacrocellPropagationLossModel> ()
    .AddAttribute ("Frequency",
                   "The propagation frequency in Hz",
                   DoubleValue (700e6),
                   MakeDoubleAccessor (&cv2x_NistScmUrbanmacrocellPropagationLossModel::m_frequency),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("ShadowingEnabled",
                   "Activate or deactivate the shadowing computation",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_NistScmUrbanmacrocellPropagationLossModel::m_isShadowingEnabled),
                   MakeBooleanChecker ())
    ;
  return tid;
}

cv2x_NistScmUrbanmacrocellPropagationLossModel::cv2x_NistScmUrbanmacrocellPropagationLossModel ()
  : BuildingsPropagationLossModel ()
{  
  m_rand = CreateObject<UniformRandomVariable> ();
  //BuildingsPropagationLossModel::m_isShadowingEnabled = m_isShadowingEnabled;
}

cv2x_NistScmUrbanmacrocellPropagationLossModel::~cv2x_NistScmUrbanmacrocellPropagationLossModel ()
{
}

double
cv2x_NistScmUrbanmacrocellPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
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

  /*********************************************/
  //3GPP TR 25.996 v6.1.0
  //non-LOS condition (low probability of occurence)
  //hbs : BS antenna height in meter
  //hms : MS antenna height in meter
  //fc*1000 : frequency in MHz
  //dist : distance between BS and MS in meter
  loss = ((44.9 - 6.55 * std::log10 (hbs)) * std::log10 (dist/1000)) + 45.5 + ((35.46 - (1.1 * hms)) * std::log10 (fc * 1000)) - (13.82 * std::log10 (hbs)) + (0.7 * hms) + 3;
  /*********************************************/

  return std::max (0.0, loss);
}

cv2x_NistScmUrbanmacrocellPropagationLossModel::PathlossType
cv2x_NistScmUrbanmacrocellPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b, bool check)
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

  /*********************************************/
  //3GPP TR 25.996 v6.1.0
  //non-LOS condition (low probability of occurence)
  double plos = 1.0;
  double r = 0.0;
  //pathloss
  double loss = 0.0;
  //hbs : BS antenna height in meter
  //hms : MS antenna height in meter
  //fc*1000 : frequency in MHz
  //dist : distance between BS and MS in meter
  loss = ((44.9 - 6.55 * std::log10 (hbs)) * std::log10 (dist/1000)) + 45.5 + ((35.46 - (1.1 * hms)) * std::log10 (fc * 1000)) - (13.82 * std::log10 (hbs)) + (0.7 * hms) + 3;
  /*********************************************/
  
  PathlossType triplet;
  triplet.loss = std::max (0.0, loss);
  triplet.plos = plos;
  triplet.r = r;
  return triplet;
}


double
cv2x_NistScmUrbanmacrocellPropagationLossModel::GetShadowing (Ptr<MobilityModel> a, Ptr<MobilityModel> b)
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
cv2x_NistScmUrbanmacrocellPropagationLossModel::EvaluateSigma (Ptr<MobilityBuildingInfo> a, Ptr<MobilityBuildingInfo> b) const
{
  /*********************************************/
  //3GPP TR 25.996 v6.1.0
  return 8.0;
  /*********************************************/
}

int64_t
cv2x_NistScmUrbanmacrocellPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}


} // namespace ns3

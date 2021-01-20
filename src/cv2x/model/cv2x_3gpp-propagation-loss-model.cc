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
 * It was tested under ns-3.21
 */

#include "ns3/mobility-model.h"
#include "ns3/double.h"
#include "ns3/cv2x_nist-outdoor-propagation-loss-model.h"
#include "ns3/cv2x_nist-indoor-propagation-loss-model.h"
#include "ns3/cv2x_nist-hybrid-propagation-loss-model.h"
#include "ns3/cv2x_nist-urbanmacrocell-propagation-loss-model.h"
#include "ns3/cv2x_cni-urbanmicrocell-propagation-loss-model.h"
#include "ns3/mobility-building-info.h"
#include "ns3/enum.h"
#include "ns3/cv2x_3gpp-propagation-loss-model.h"
#include "ns3/building-list.h"
#include "ns3/cv2x_lte-enb-net-device.h"
#include "ns3/cv2x_lte-ue-net-device.h"
#include <ns3/boolean.h>

NS_LOG_COMPONENT_DEFINE ("cv2x_Lte3gppPropagationLossModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_Lte3gppPropagationLossModel);


cv2x_Lte3gppPropagationLossModel::cv2x_Lte3gppPropagationLossModel ()
{
  m_indoor = CreateObject<cv2x_NistIndoorPropagationLossModel> ();
  m_outdoor = CreateObject<cv2x_NistOutdoorPropagationLossModel> ();
  m_hybrid = CreateObject<cv2x_NistHybridPropagationLossModel> ();
  m_urbanmacrocell = CreateObject<cv2x_NistUrbanmacrocellPropagationLossModel> ();
  m_urbanmicrocell = CreateObject<cv2x_CniUrbanmicrocellPropagationLossModel> (); 
}

cv2x_Lte3gppPropagationLossModel::~cv2x_Lte3gppPropagationLossModel ()
{
}

TypeId
cv2x_Lte3gppPropagationLossModel::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_Lte3gppPropagationLossModel")
    .SetParent<BuildingsPropagationLossModel> ()
    .AddConstructor<cv2x_Lte3gppPropagationLossModel> ()
    .AddAttribute ("Frequency",
                   "The propagation frequency  (default : general cases 2100MHz band).",
                   DoubleValue (2106e6),
                   MakeDoubleAccessor (&cv2x_Lte3gppPropagationLossModel::SetFrequency),
                   MakeDoubleChecker<double> ())
    /*.AddAttribute ("LosCheck",
                   "LOS/NLOS condition",
                   BooleanValue (false),
                   MakeBooleanAccessor (&cv2x_Lte3gppPropagationLossModel::m_los),
                   MakeBooleanChecker ())*/
    .AddAttribute ("CacheLoss",
                   "Indicates if the loss value should be cached",
                   BooleanValue (false),
                   MakeBooleanAccessor (&cv2x_Lte3gppPropagationLossModel::m_cacheLoss),
                   MakeBooleanChecker ())    
    .AddTraceSource ("PathlossValue",
                     "Pathloss value to trace",
                     MakeTraceSourceAccessor (&cv2x_Lte3gppPropagationLossModel::m_pathlossTrace),
                     "ns3::cv2x_Lte3gppPropagationLossModel::PathlossValueTracedCallback")
  ;
  return tid;
}

void
cv2x_Lte3gppPropagationLossModel::SetFrequency (double freq)
{
  m_indoor->SetAttribute ("Frequency", DoubleValue (freq));
  m_outdoor->SetAttribute ("Frequency", DoubleValue (freq));
  m_hybrid->SetAttribute ("Frequency", DoubleValue (freq));
  m_urbanmacrocell->SetAttribute ("Frequency", DoubleValue (freq));
  m_urbanmicrocell->SetAttribute ("Frequency", DoubleValue (freq)); 
  m_frequency = freq;
}

double
cv2x_Lte3gppPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{

  NS_ASSERT_MSG ((a->GetPosition ().z >= 0) && (b->GetPosition ().z >= 0), "Lte3gppsPropagationLossModel does not support underground nodes (placed at z < 0)");

  //Check if loss value is cached
  double loss = 0.0;
  MobilityDuo couple;
  couple.a = a;
  couple.b = b;
  std::map<MobilityDuo, double>::iterator it_a = m_lossMap.find (couple);
  if (it_a != m_lossMap.end ())
    {
      loss = it_a->second;
    }
  else
    {
      couple.a = b;
      couple.b = a;
      std::map<MobilityDuo, double>::iterator it_b = m_lossMap.find (couple);
      if (it_b != m_lossMap.end ())
        {
          loss = it_b->second;
        }
      else
        {
          // Get the MobilityBuildingInfo pointers
          Ptr<MobilityBuildingInfo> a1 = a->GetObject<MobilityBuildingInfo> ();
          Ptr<MobilityBuildingInfo> b1 = b->GetObject<MobilityBuildingInfo> ();
          NS_ASSERT_MSG ((a1 != 0) && (b1 != 0), "Lte3gppsPropagationLossModel only works with MobilityBuildingInfo");

          //double loss = 0.0;
          Vector aPos = a->GetPosition ();
          Vector bPos = b->GetPosition ();

          bool aIndoor = false;
          bool bIndoor = false;

          // Go through the different buildings in the simulation 
          // and check if the nodes are outdoor or indoor
          for (BuildingList::Iterator bit = BuildingList::Begin (); bit != BuildingList::End (); ++bit)
            {
              NS_LOG_LOGIC ("checking building " << (*bit)->GetId () << " with boundaries " << (*bit)->GetBoundaries ());
              if ((*bit)->IsInside (aPos))
                {
                  aIndoor = true;
                  uint16_t floor = (*bit)->GetFloor (aPos);
                  uint16_t roomX = (*bit)->GetRoomX (aPos);
                  uint16_t roomY = (*bit)->GetRoomY (aPos);     
                  a1->SetIndoor (*bit, floor, roomX, roomY);       
                }
              if ((*bit)->IsInside (bPos))
                {
                  bIndoor = true;
                  uint16_t floor = (*bit)->GetFloor (bPos);
                  uint16_t roomX = (*bit)->GetRoomX (bPos);
                  uint16_t roomY = (*bit)->GetRoomY (bPos);     
                  b1->SetIndoor (*bit, floor, roomX, roomY);  
                }
            }

          // Verify if it is a D2D communication (UE-UE) / V2X communication (UE-UE) or LTE communication (eNB-UE)
          // LTE
          if (((a->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0) and (b->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteEnbNetDevice> () != 0)) or ((b->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0) and (a->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteEnbNetDevice> () != 0)))
            {
              loss = Urbanmacrocell (a,b);
            }
          //D2D or V2X
          else if ((a->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0) and (b->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0))
            {
              // Calculate the pathloss based on the position of the nodes (outdoor/indoor)
              // a outdoor
              if (!aIndoor)
                {
                  // b outddor
                  if (!bIndoor)
                    {
                      // Outdoor tranmission 
                      loss = Outdoor (a, b);
                      NS_LOG_INFO (this << " Outdoor : " << loss);
                    }

                  // b indoor
                  else
                    {
                      loss = Hybrid (a, b);
                      NS_LOG_INFO (this << " Hybrid : " << loss);
                    }
                } 

              // a is indoor
              else
                {
                  // b is indoor
                  if (bIndoor)
                    {
                      loss = Indoor (a, b).first;
                      //m_los = Indoor (a, b).second;
                      NS_LOG_INFO (this << " Indoor : " << loss );  
                    }

                  // b is outdoor
                  else
                    {
                      loss = Hybrid (a, b);
                      NS_LOG_INFO (this << " Hybrid : " << loss);
                    } 
                }
            }
          //Other cases (not nist nodes)
          else 
            {
              NS_FATAL_ERROR ("Non-valid nodes");
            }
          loss = std::max (loss, 0.0);
          if (m_cacheLoss)
            {
              m_lossMap[couple] = loss; //cache value
            }
          m_pathlossTrace (loss, a->GetObject<Node> (), b->GetObject<Node> (), a->GetDistanceFrom (b), aIndoor, bIndoor);
        }      
    }

  
  return loss;
}


double
cv2x_Lte3gppPropagationLossModel::GetShadowing (Ptr<MobilityModel> a, Ptr<MobilityModel> b)
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
cv2x_Lte3gppPropagationLossModel::EvaluateSigma (Ptr<MobilityBuildingInfo> a, Ptr<MobilityBuildingInfo> b) const
{
  //LTE
  if (((a->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0) and (b->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteEnbNetDevice> () != 0)) or ((b->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0) and (a->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteEnbNetDevice> () != 0)))
  {
    return m_urbanmacrocell->EvaluateSigma (a,b);
  }
  //D2D
  else if ((a->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0) and (b->GetObject<Node> ()->GetDevice (0)->GetObject<cv2x_LteUeNetDevice> () != 0))
  {
    if (!a->IsIndoor ())
    {
      if (!b->IsIndoor ())
      {
        // Outdoor
        return 7;
      }
      else
      {
        // Hybrid
        return 7;
      }
    }
    else
    if (b->IsIndoor ())
    {
      if (a->GetBuilding () == b->GetBuilding ())
      {
        //if (m_los)
        if (Indoor (a->GetObject<MobilityModel> (), b->GetObject<MobilityModel> ()).second)
        {
          // Indoor Same building LOS
          return 3; 
        }
        else
        {
          // Indoor Same building NLOS
          return 4;
        }
      }
      else
      {
        // Indoor Different Buildings
        return 10;
      }    
    }
    else
    {
      // Hybrid
      return 7;
    }
  }
  else 
  {
    NS_FATAL_ERROR ("Non-valid nodes"); 
  }
  return 0;
}

std::pair<double, bool>
cv2x_Lte3gppPropagationLossModel::Indoor (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  return m_indoor->GetLoss (a, b);
}

double
cv2x_Lte3gppPropagationLossModel::Outdoor (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  return m_outdoor->GetLoss (a, b);
}

double
cv2x_Lte3gppPropagationLossModel::Hybrid (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  return m_hybrid->GetLoss (a, b);
}

double
cv2x_Lte3gppPropagationLossModel::Urbanmacrocell (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  return m_urbanmacrocell->GetLoss (a, b);
}

double 
cv2x_Lte3gppPropagationLossModel::Urbanmicrocell (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  return m_urbanmicrocell->GetLoss (a,b); 
}

} // namespace ns3

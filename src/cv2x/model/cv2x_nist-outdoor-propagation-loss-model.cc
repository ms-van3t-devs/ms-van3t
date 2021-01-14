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
#include "cv2x_nist-outdoor-propagation-loss-model.h"
#include <ns3/node.h>
//#include <ns3/cv2x_lte-enb-net-device.h>
//#include <ns3/cv2x_lte-ue-net-device.h>

NS_LOG_COMPONENT_DEFINE ("cv2x_NistOutdoorPropagationLossModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_NistOutdoorPropagationLossModel);


TypeId
cv2x_NistOutdoorPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_NistOutdoorPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<cv2x_NistOutdoorPropagationLossModel> ()
    .AddAttribute ("Frequency",
                    "The propagation frequency in Hz",
                    DoubleValue (763e6),
                    MakeDoubleAccessor (&cv2x_NistOutdoorPropagationLossModel::m_frequency),
                    MakeDoubleChecker<double> ())
    ;

  return tid;
}

cv2x_NistOutdoorPropagationLossModel::cv2x_NistOutdoorPropagationLossModel ()
  : PropagationLossModel ()
{  
  m_rand = CreateObject<UniformRandomVariable> ();
}

cv2x_NistOutdoorPropagationLossModel::~cv2x_NistOutdoorPropagationLossModel ()
{
}

double
cv2x_NistOutdoorPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  // Free space pathloss
  double loss = 0.0;
  // Frequency in GHz
  double fc = m_frequency / 1e9;
  // Distance between the two nodes in meter
  double dist = a->GetDistanceFrom (b);

  // Calculate the pathloss based on 3GPP specifications : 3GPP TR 36.843 V12.0.1
  // WINNER II Channel Models, D1.1.2 V1.2., Equation (4.24) p.43, available at 
  // http://www.cept.org/files/1050/documents/winner2%20-%20final%20report.pdf
  loss = 20 * std::log10 (dist) + 46.6 + 20 * std::log10 (fc / 5.0);
  NS_LOG_INFO (this << "Outdoor , the free space loss = " << loss);
  
  // WINNER II channel model for Urban Microcell scenario (UMi) : B1
  double pl_b1 = 0.0;
  // Actual antenna heights (1.5 m for UEs)
  double hms = a->GetPosition ().z;
  double hbs = b->GetPosition ().z;
  // Effective antenna heights (0.8 m for UEs)
  double hbs1 = hbs - 1;
  double hms1 = hms - 0.7;
  // Propagation velocity in free space
  double c = 3 * std::pow (10, 8);
  // LOS offset = LOS loss to add to the computed pathloss
  double los = 0;
  // NLOS offset = NLOS loss to add to the computed pathloss
  double nlos = -5;

  double d1 = 4 * hbs1 * hms1 * m_frequency * (1 / c);

  // Calculate the LOS probability based on 3GPP specifications : 3GPP TR 36.843 V12.0.1
  // WINNER II Channel Models, D1.1.2 V1.2., Table 4-7 p.48, available at 
  // http://www.cept.org/files/1050/documents/winner2%20-%20final%20report.pdf
  double plos = std::min ((18 / dist), 1.0) * (1 - std::exp (-dist / 36)) + std::exp (-dist / 36);

  // Compute the WINNER II B1 pathloss based on 3GPP specifications : 3GPP TR 36.843 V12.0.1
  // D5.3: WINNER+ Final Channel Models, Table 4-1 p.74, available at 
  // http://projects.celtic-initiative.org/winner%2B/WINNER+%20Deliverables/D5.3_v1.0.pdf
    
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
  
  // This model is only valid to a minimum distance of 3 meters 
  if (dist >= 3)
  {
    if (r <= plos)
    {
      // LOS
      if (dist <= d1)
      {
        pl_b1 = 22.7 * std::log10 (dist) + 27.0 + 20.0 * std::log10 (fc) + los;
        NS_LOG_INFO (this << "Outdoor LOS (Distance <= " << d1 << ") : the WINNER B1 loss = " << pl_b1);
      }
      else
      {
        pl_b1 = 40 * std::log10 (dist) + 7.56 - 17.3 * std::log10 (hbs1) - 17.3 * std::log10 (hms1) + 2.7 * std::log10 (fc) + los;
        NS_LOG_INFO (this << "Outdoor LOS (Distance > " << d1 << ") : the WINNER B1 loss = " << pl_b1);
      }
    }
    else 
    {
      // NLOS
      if ((fc >= 0.758) and (fc <= 0.798))
      {
        // Frequency = 700 MHz for Public Safety
        pl_b1 = (44.9 - 6.55 * std::log10 (hbs)) * std::log10 (dist) + 5.83 * std::log10 (hbs) + 16.33 + 26.16 * std::log10 (fc) + nlos;
        NS_LOG_INFO (this << "Outdoor NLOS (Frequency 0.7 GHz) , the WINNER B1 loss = " << pl_b1);
      }
      if ((fc >= 1.92) and (fc <= 2.17))
      {
        // Frequency = 2 GHz for general scenarios
        pl_b1 = (44.9 - 6.55 * std::log10 (hbs)) * std::log10 (dist) + 5.83 * std::log10 (hbs) + 14.78 + 34.97 * std::log10 (fc) + nlos;
        NS_LOG_INFO (this << "Outdoor NLOS (Frequency 2 GHz) , the WINNER B1 loss = " << pl_b1);
      }
    }  
  }

  loss = std::max (loss, pl_b1);  
  return std::max (0.0, loss);
}

double 
cv2x_NistOutdoorPropagationLossModel::DoCalcRxPower (double txPowerDbm,
					       Ptr<MobilityModel> a,
					       Ptr<MobilityModel> b) const
{
  return (txPowerDbm - GetLoss (a, b));
}

int64_t
cv2x_NistOutdoorPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}


} // namespace ns3

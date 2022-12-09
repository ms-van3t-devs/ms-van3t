/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * Cni assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.
 * 
 * We would appreciate acknowledgement if the software is used.
 * 
 * Cni ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 * 
 * Modified by: CNI
 * It was tested under ns-3.28
 */

#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/enum.h"
#include "ns3/mobility-model.h"
#include <cmath>
#include "nr_cni-urbanmicrocell-propagation-loss-model.h"
#include <ns3/node.h>

NS_LOG_COMPONENT_DEFINE ("nr_CniUrbanmicrocellPropagationLossModel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (nr_CniUrbanmicrocellPropagationLossModel);

TypeId
nr_CniUrbanmicrocellPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::nr_CniUrbanmicrocellPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .AddConstructor<nr_CniUrbanmicrocellPropagationLossModel> ()
    .AddAttribute ("Frequency",
                    "The propagation frequency in Hz",
                    DoubleValue (5900e6),
                    MakeDoubleAccessor (&nr_CniUrbanmicrocellPropagationLossModel::m_frequency),
                    MakeDoubleChecker<double> ())
    ;

  return tid;
}

void
nr_CniUrbanmicrocellPropagationLossModel::SetFrequency (double freq)
{
  m_frequency = freq;
}


nr_CniUrbanmicrocellPropagationLossModel::nr_CniUrbanmicrocellPropagationLossModel ()
  : PropagationLossModel ()
{ 
  m_rand = CreateObject<UniformRandomVariable> ();
}

nr_CniUrbanmicrocellPropagationLossModel::~nr_CniUrbanmicrocellPropagationLossModel ()
{
}

double
nr_CniUrbanmicrocellPropagationLossModel::GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  // Pathloss
  double loss = 0.0;
  // Frequency in GHz
  double fc = m_frequency / 1e9;
  // Distance between the two nodes in meter
  double dist = a->GetDistanceFrom (b);

  // Actual antenna heights (1.5m for UEs)
  double hms = a->GetPosition().z;
  double hbs = b->GetPosition().z;

  // Effective antenna heights
  double hbs1 = hbs - 1;
  double hms1 = hms - 1;
  // Propagation velocity in free space
  double c = 3 * std::pow (10, 8);

  // NLOS offset = NLOS loss to add to the computed pathloss
  double nlos = -5;
  
  // Breakpoint distance
  double d_bp = 4 * hbs1 * hms1 * m_frequency * (1 / c);

  // Calculate the LOS probability based on 3GPP specifications 
  // https://www.cept.org/files/8339/winner2%20-%20final%20report.pdf Table 4-7
  double plos = std::min ((18 / dist), 1.0) * (1 - std::exp (-dist / 36)) + std::exp (-dist / 36);

  // Freespace pathloss
  double loss_free  = 20*std::log10 (dist) + 46.4 + 20*std::log10(fc/5.0); 
  NS_LOG_INFO (this << "Outdoor , the free space loss = " << loss_free);

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
  // This model is only valid to a minimum distance of 3 meters 
  if (dist >= 3)
  {
    if ((r <= plos) or (m_isLosEnabled)) 
    { 
      // LOS
      if (dist <= d_bp)
      {
        loss = 22.7 * std::log10 (dist) + 27.0 + 20.0 * std::log10 (fc);
        NS_LOG_INFO (this << "Urban microcell LOS (Distance <= " << d_bp << ") : the pathloss = " << loss);
      }
      else
      {
        loss = 40.0 * std::log10 (dist) + 7.56 - 17.3 * std::log10 (hbs1) - 17.3 * std::log10 (hms1) + 2.7 * std::log10 (fc);
        NS_LOG_INFO (this << "Urban microcell LOS (Distance > " << d_bp << ") : the pathloss = " << loss);
      }
    }
    else 
    { 
      // NLOS
      loss = (44.9 - 6.55 * std::log10 (hbs)) * std::log10 (dist) + 5.83 * std::log10 (hbs) + 18.38 + 23 * std::log10 (fc) + nlos;
      NS_LOG_INFO (this << "Urban microcell NLOS, the pathloss = " << loss);
    }  
  }
  loss = std::max (loss_free, loss);  
  return std::max (0.0, loss);
}

double 
nr_CniUrbanmicrocellPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                                      Ptr<MobilityModel> a,
                                                      Ptr<MobilityModel> b) const
{
  return (txPowerDbm - GetLoss (a, b));
}

int64_t
nr_CniUrbanmicrocellPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

} // namespace ns3

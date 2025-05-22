/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

* Created by:
    *  Diego Gasco, Politecnico di Torino (diego.gasco@polito.it, diego.gasco99@gmail.com)
*/

#include "DCC.h"

namespace ns3 {
NS_LOG_COMPONENT_DEFINE("DCC");

TypeId
DCC::GetTypeId ()
{
  static TypeId tid = TypeId("ns3::DCC")
                          .SetParent <Object>()
                          .AddConstructor <DCC>();
  return tid;
}

DCC::DCC ()
{
  m_reactive_parameters[ReactiveState::Relaxed] = {0.20, 24.0, 100, -95.0};
  m_reactive_parameters[ReactiveState::Active1] = {0.30, 18.0, 200, -95.0};
  m_reactive_parameters[ReactiveState::Active2] = {0.40, 12.0, 400, -95.0};
  m_reactive_parameters[ReactiveState::Active3] = {0.50, 6.0, 500, -95.0};
  m_reactive_parameters[ReactiveState::Restrictive] = {1.0, 2.0, 1000, -65.0};
}

DCC::~DCC()
= default;

void DCC::reactiveDCC()
{
  NS_LOG_INFO("Starting DCC check");
  NS_ASSERT_MSG (m_metric_supervisor != nullptr, "Metric Supervisor not set");
  NS_ASSERT_MSG (m_dcc_interval != Time(Seconds(-1.0)), "DCC interval not set");

  std::unordered_map<std::string, std::vector<double>> cbrs = m_metric_supervisor->getCBRValues();
  for (auto it = cbrs.begin(); it != cbrs.end(); ++it)
    {
      std::string id = it->first;
      double current_cbr = it->second.back();
      bool found = false;
      ReactiveState old_state;
      ReactiveState new_state;
      if (m_vehicle_state.find(id) != m_vehicle_state.end())
        {
          found = true;
          old_state = m_vehicle_state[id];
        }

      if (found)
        {
          bool relaxed_flag = true ? old_state == ReactiveState::Relaxed : false;
          if (current_cbr > m_reactive_parameters[old_state].cbr_threshold)
            {
              new_state = static_cast<ReactiveState> (old_state + 1);
            }
          else if (relaxed_flag)
            {
              new_state = ReactiveState::Relaxed;
            }
          else if (current_cbr <
                   m_reactive_parameters[static_cast<ReactiveState> (old_state - 1)].cbr_threshold)
            {
              new_state = static_cast<ReactiveState> (old_state - 1);
            }
          else
            {
              new_state = old_state;
            }
        }
      else
        {
          for (int foo_int = ReactiveState::Relaxed; foo_int != ReactiveState::Restrictive; foo_int++)
            {
              ReactiveState foo = static_cast<ReactiveState>(foo_int);
              if (current_cbr < m_reactive_parameters[foo].cbr_threshold)
                {
                  new_state = foo;
                  break;
                }
            }
        }

      // Get the NetDevice
      int nodeID_int = m_traci_client->get_NodeMap()[id].second->GetId();
      std::string nodeID_str = std::to_string (nodeID_int);
      Ptr<NetDevice> netDevice = m_traci_client->get_NodeMap()[id].second->GetDevice (0);
      Ptr<WifiNetDevice> wifiDevice;

      Ptr<WifiPhy> phy80211p = nullptr;

      // Get the WifiNetDevice
      wifiDevice = DynamicCast<WifiNetDevice> (netDevice);

      if (wifiDevice != nullptr)
        {
          // Get the PHY layer
          phy80211p = wifiDevice->GetPhy ();
        }

        if (phy80211p != nullptr)
          {
            phy80211p->SetTxPowerStart (m_reactive_parameters[new_state].tx_power);
            phy80211p->SetTxPowerEnd (m_reactive_parameters[new_state].tx_power);
            phy80211p->SetRxSensitivity (m_reactive_parameters[new_state].sensitivity);
          }

        if (m_caService.find(nodeID_str) != m_caService.end())
          {
            m_caService[nodeID_str]->setCheckCamGenMs (m_reactive_parameters[new_state].tx_inter_packet_time);
          }
        if (m_caServiceV1.find(nodeID_str) != m_caServiceV1.end())
          {
            m_caServiceV1[nodeID_str]->setCheckCamGenMs (m_reactive_parameters[new_state].tx_inter_packet_time);
          }
        if (m_cpService.find(nodeID_str) != m_cpService.end())
          {
            m_cpService[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters[new_state].tx_inter_packet_time);
          }
        if (m_cpServiceV1.find(nodeID_str) != m_cpServiceV1.end())
          {
            m_cpServiceV1[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters[new_state].tx_inter_packet_time);
          }
        if(m_vruService.find(nodeID_str) != m_vruService.end())
          {
            m_vruService[nodeID_str]->setCheckVamGenMs (m_reactive_parameters[new_state].tx_inter_packet_time);
          }

      m_vehicle_state[id] = new_state;
    }

  Simulator::Schedule(m_dcc_interval, &DCC::reactiveDCC, this);
}

void DCC::adaptiveDCC()
{
  NS_LOG_INFO ("Starting DCC check");
  NS_ASSERT_MSG (m_traci_client != nullptr, "TraCI client not set");
  NS_ASSERT_MSG (m_metric_supervisor != nullptr, "Metric Supervisor not set");
  NS_ASSERT_MSG (m_dcc_interval != Time (Seconds (-1.0)), "DCC interval not set");

  std::unordered_map<std::string, std::vector<double>> cbrs = m_metric_supervisor->getCBRValues ();

  for (auto it = cbrs.begin (); it != cbrs.end (); ++it)
    {
      std::string id = it->first;
      int nodeID_int = m_traci_client->get_NodeMap ()[id].second->GetId ();
      std::string nodeID_str = std::to_string (nodeID_int);
      double current_cbr = it->second.back ();
      double previous_cbr;
      if (it->second.size() < 2)
        {
          previous_cbr = 0;
        }
      else
        {
          previous_cbr = it->second[it->second.size () - 2];
        }
      double delta_offset;

      // Step 1
      if (m_CBR_its.find(id) != m_CBR_its.end())
        {
          m_CBR_its[id] = 0.5 * m_CBR_its[id] + 0.25 * ((current_cbr + previous_cbr) / 2);
        }
      else
        {
          m_CBR_its[id] = (current_cbr + previous_cbr) / 2;
        }

      if ((m_CBR_target - m_CBR_its[id]) > 0) // Step 2
        {
          delta_offset = std::min (m_beta * (m_CBR_target - m_CBR_its[id]), m_Gmax);
        }
      else
        {
          delta_offset = std::max (m_beta * (m_CBR_target - m_CBR_its[id]), m_Gmin);
        }

      m_delta = (1 - m_alpha) * m_delta + delta_offset; // Step 3

      if (m_delta > m_delta_max) // Step 4
        {
          m_delta = m_delta_max;
        }

      if (m_delta < m_delta_min) // Step 5
        {
          m_delta = m_delta_min;
        }

      if (m_caService.find (nodeID_str) != m_caService.end ())
        {
          m_caService[nodeID_str]->toffUpdateAfterDeltaUpdate (m_delta);
        }
      if (m_caServiceV1.find (nodeID_str) != m_caServiceV1.end ())
        {
          m_caServiceV1[nodeID_str]->toffUpdateAfterDeltaUpdate (m_delta);
        }
      if (m_cpService.find (nodeID_str) != m_cpService.end ())
        {
          m_cpService[nodeID_str]->toffUpdateAfterDeltaUpdate(m_delta);
        }
      if (m_cpServiceV1.find (nodeID_str) != m_cpServiceV1.end ())
        {
          m_cpServiceV1[nodeID_str]->toffUpdateAfterDeltaUpdate(m_delta);
        }
      if (m_vruService.find (nodeID_str) != m_vruService.end ())
        {
          m_vruService[nodeID_str]->toffUpdateAfterDeltaUpdate(m_delta);
        }
    }

  Simulator::Schedule(m_dcc_interval, &DCC::adaptiveDCC, this);
}

}


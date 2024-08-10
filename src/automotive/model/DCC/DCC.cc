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
= default;

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
      ReactiveState oldState = ReactiveState::Undefined;
      ReactiveState state;
      if (m_states.find(id) != m_states.end())
        {
          oldState = m_states[id];
        }

      if (current_cbr < m_reactive_parameters_relaxed.m_cbr_threshold)
        {
          if (oldState == ReactiveState::Undefined)
            {
              state = ReactiveState::Relaxed;
            }
          else if (oldState == ReactiveState::Relaxed)
            {
              continue;
            }
          else
            {
              state = static_cast<ReactiveState> (oldState - 1);
            }
        }
      else if (current_cbr < m_reactive_parameters_active1.m_cbr_threshold)
        {
          if (oldState == ReactiveState::Undefined)
            {
              state = ReactiveState::Active1;
            }
          else if (oldState == ReactiveState::Active1)
            {
              continue;
            }
          else if (oldState - ReactiveState::Active1 > 0)
            {
              state = static_cast<ReactiveState> (oldState - 1);
            }
          else
            {
              state = static_cast<ReactiveState> (oldState + 1);
            }
        }
      else if (current_cbr < m_reactive_parameters_active2.m_cbr_threshold)
        {
          if (oldState == ReactiveState::Undefined)
            {
              state = ReactiveState::Active2;
            }
          else if (oldState == ReactiveState::Active2)
            {
              continue;
            }
          else if (oldState - ReactiveState::Active2 > 0)
            {
              state = static_cast<ReactiveState> (oldState - 1);
            }
          else
            {
              state = static_cast<ReactiveState> (oldState + 1);
            }
        }
      else if (current_cbr < m_reactive_parameters_active3.m_cbr_threshold)
        {
          if (oldState == ReactiveState::Undefined)
            {
              state = ReactiveState::Active3;
            }
          else if (oldState == ReactiveState::Active3)
            {
              continue;
            }
          else if (oldState - ReactiveState::Active3 > 0)
            {
              state = static_cast<ReactiveState> (oldState - 1);
            }
          else
            {
              state = static_cast<ReactiveState> (oldState + 1);
            }
        }
      else
        {
          if (oldState == ReactiveState::Undefined)
            {
              state = ReactiveState::Restrictive;
            }
          else if (oldState == ReactiveState::Restrictive)
            {
              continue;
            }
          else
            {
              state = static_cast<ReactiveState> (oldState + 1);
            }
        }

      // Get the NetDevice
      int nodeID_int = m_traci_client->get_NodeMap()[id].second->GetId();
      std::string nodeID_str = std::to_string (nodeID_int);
      Ptr<NetDevice> netDevice = m_traci_client->get_NodeMap()[id].second->GetDevice (0);
      Ptr<WifiNetDevice> wifiDevice;

      Ptr<WifiPhy> phy80211p = nullptr;

      if (m_metric_supervisor->getChannelTechnology() == "80211p")
        {
          // Get the WifiNetDevice
          wifiDevice = DynamicCast<WifiNetDevice> (netDevice);

          if (wifiDevice != nullptr)
            {
              // Get the PHY layer
              phy80211p = wifiDevice->GetPhy ();
            }
        }

        switch (state)
          {
          case ReactiveState::Relaxed:
            if (m_metric_supervisor->getChannelTechnology() == "80211p" && phy80211p != nullptr)
              {
                phy80211p->SetTxPowerStart (m_reactive_parameters_relaxed.m_tx_power);
                phy80211p->SetTxPowerEnd (m_reactive_parameters_relaxed.m_tx_power);
                phy80211p->SetRxSensitivity (m_reactive_parameters_relaxed.m_sensitivity);
              }
            // else if (m_metric_supervisor->getChannelTechnology() == "Nr" && m_nr_helper != nullptr)
            //   {
            //     m_nr_helper->GetUePhy (netDevice, 0)->SetTxPower (m_reactive_parameters_relaxed.m_tx_power);
            //   }

            if (m_caService.find(nodeID_str) != m_caService.end())
              {
                m_caService[nodeID_str]->setCheckCamGenMs (m_reactive_parameters_relaxed.m_tx_inter_packet_time);
              }
            if (m_caServiceV1.find(nodeID_str) != m_caServiceV1.end())
              {
                m_caServiceV1[nodeID_str]->setCheckCamGenMs (m_reactive_parameters_relaxed.m_tx_inter_packet_time);
              }
            if (m_cpService.find(nodeID_str) != m_cpService.end())
              {
                m_cpService[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters_relaxed.m_tx_inter_packet_time);
              }
            if (m_cpServiceV1.find(nodeID_str) != m_cpServiceV1.end())
              {
                m_cpServiceV1[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters_relaxed.m_tx_inter_packet_time);
              }
            if(m_vruService.find(nodeID_str) != m_vruService.end())
              {
                m_vruService[nodeID_str]->setCheckVamGenMs (m_reactive_parameters_relaxed.m_tx_inter_packet_time);
              }

            break;

          case Active1:
            if (m_metric_supervisor->getChannelTechnology() == "80211p" && phy80211p != nullptr)
              {
                phy80211p->SetTxPowerStart (m_reactive_parameters_active1.m_tx_power);
                phy80211p->SetTxPowerEnd (m_reactive_parameters_active1.m_tx_power);
                phy80211p->SetRxSensitivity (m_reactive_parameters_active1.m_sensitivity);
              }
            // else if (m_metric_supervisor->getChannelTechnology() == "Nr" && m_nr_helper != nullptr)
            //   {
            //     m_nr_helper->GetUePhy (netDevice, 0)->SetTxPower (m_reactive_parameters_active1.m_tx_power);
            //   }

            if (m_caService.find(nodeID_str) != m_caService.end())
              {
                m_caService[nodeID_str]->setCheckCamGenMs (m_reactive_parameters_active1.m_tx_inter_packet_time);
              }
            if (m_caServiceV1.find(nodeID_str) != m_caServiceV1.end())
              {
                m_caServiceV1[nodeID_str]->setCheckCamGenMs (m_reactive_parameters_active1.m_tx_inter_packet_time);
              }
            if (m_cpService.find(nodeID_str) != m_cpService.end())
              {
                m_cpService[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters_active1.m_tx_inter_packet_time);
              }
            if (m_cpServiceV1.find(nodeID_str) != m_cpServiceV1.end())
              {
                m_cpServiceV1[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters_active1.m_tx_inter_packet_time);
              }
            if(m_vruService.find(nodeID_str) != m_vruService.end())
              {
                m_vruService[nodeID_str]->setCheckVamGenMs (m_reactive_parameters_active1.m_tx_inter_packet_time);
              }
            break;

          case Active2:
            if (m_metric_supervisor->getChannelTechnology() == "80211p" && phy80211p != nullptr)
              {
                phy80211p->SetTxPowerStart (m_reactive_parameters_active2.m_tx_power);
                phy80211p->SetTxPowerEnd (m_reactive_parameters_active2.m_tx_power);
                phy80211p->SetRxSensitivity (m_reactive_parameters_active2.m_sensitivity);
              }
            // else if (m_metric_supervisor->getChannelTechnology() == "Nr" && m_nr_helper != nullptr)
            //   {
            //     m_nr_helper->GetUePhy (netDevice, 0)->SetTxPower (m_reactive_parameters_active2.m_tx_power);
            //   }

            if (m_caService.find(nodeID_str) != m_caService.end())
              {
                m_caService[nodeID_str]->setCheckCamGenMs (m_reactive_parameters_active2.m_tx_inter_packet_time);
              }
            if (m_caServiceV1.find(nodeID_str) != m_caServiceV1.end())
              {
                m_caServiceV1[nodeID_str]->setCheckCamGenMs (m_reactive_parameters_active2.m_tx_inter_packet_time);
              }
            if (m_cpService.find(nodeID_str) != m_cpService.end())
              {
                m_cpService[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters_active2.m_tx_inter_packet_time);
              }
            if (m_cpServiceV1.find(nodeID_str) != m_cpServiceV1.end())
              {
                m_cpServiceV1[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters_active2.m_tx_inter_packet_time);
              }
            if(m_vruService.find(nodeID_str) != m_vruService.end())
              {
                m_vruService[nodeID_str]->setCheckVamGenMs (m_reactive_parameters_active2.m_tx_inter_packet_time);
              }
            break;

          case Active3:
            if (m_metric_supervisor->getChannelTechnology() == "80211p" && phy80211p != nullptr)
              {
                phy80211p->SetTxPowerStart (m_reactive_parameters_active3.m_tx_power);
                phy80211p->SetTxPowerEnd (m_reactive_parameters_active3.m_tx_power);
                phy80211p->SetRxSensitivity (m_reactive_parameters_active3.m_sensitivity);
              }
            // else if (m_metric_supervisor->getChannelTechnology() == "Nr" && m_nr_helper != nullptr)
            //   {
            //     m_nr_helper->GetUePhy (netDevice, 0)->SetTxPower (m_reactive_parameters_active3.m_tx_power);
            //   }

            if (m_caService.find(nodeID_str) != m_caService.end())
              {
                m_caService[nodeID_str]->setCheckCamGenMs (m_reactive_parameters_active3.m_tx_inter_packet_time);
              }
            if (m_caServiceV1.find(nodeID_str) != m_caServiceV1.end())
              {
                m_caServiceV1[nodeID_str]->setCheckCamGenMs (m_reactive_parameters_active3.m_tx_inter_packet_time);
              }
            if (m_cpService.find(nodeID_str) != m_cpService.end())
              {
                m_cpService[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters_active3.m_tx_inter_packet_time);
              }
            if (m_cpServiceV1.find(nodeID_str) != m_cpServiceV1.end())
              {
                m_cpServiceV1[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters_active3.m_tx_inter_packet_time);
              }
            if(m_vruService.find(nodeID_str) != m_vruService.end())
              {
                m_vruService[nodeID_str]->setCheckVamGenMs (m_reactive_parameters_active3.m_tx_inter_packet_time);
              }
            break;

          case Restrictive:
            if (m_metric_supervisor->getChannelTechnology() == "80211p" && phy80211p != nullptr)
              {
                phy80211p->SetTxPowerStart (m_reactive_parameters_restricted.m_tx_power);
                phy80211p->SetTxPowerEnd (m_reactive_parameters_restricted.m_tx_power);
                phy80211p->SetRxSensitivity (m_reactive_parameters_restricted.m_sensitivity);
              }
            // else if (m_metric_supervisor->getChannelTechnology() == "Nr" && m_nr_helper != nullptr)
            //   {
            //     m_nr_helper->GetUePhy (netDevice, 0)->SetTxPower (m_reactive_parameters_restricted.m_tx_power);
            //   }

            if (m_caService.find(nodeID_str) != m_caService.end())
              {
                m_caService[nodeID_str]->setCheckCamGenMs (m_reactive_parameters_restricted.m_tx_inter_packet_time);
              }
            if (m_caServiceV1.find(nodeID_str) != m_caServiceV1.end())
              {
                m_caServiceV1[nodeID_str]->setCheckCamGenMs (m_reactive_parameters_restricted.m_tx_inter_packet_time);
              }
            if (m_cpService.find(nodeID_str) != m_cpService.end())
              {
                m_cpService[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters_restricted.m_tx_inter_packet_time);
              }
            if (m_cpServiceV1.find(nodeID_str) != m_cpServiceV1.end())
              {
                m_cpServiceV1[nodeID_str]->setCheckCpmGenMs (m_reactive_parameters_restricted.m_tx_inter_packet_time);
              }
            if(m_vruService.find(nodeID_str) != m_vruService.end())
              {
                m_vruService[nodeID_str]->setCheckVamGenMs (m_reactive_parameters_restricted.m_tx_inter_packet_time);
              }
            break;
          }
      m_states[id] = state;
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


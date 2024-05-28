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

  NS_ASSERT_MSG (m_bs_map != nullptr, "BS Map not set");
  NS_ASSERT_MSG (m_traci_client != nullptr, "TraCI client not set");
  NS_ASSERT_MSG (m_metric_supervisor != nullptr, "Metric Supervisor not set");
  NS_ASSERT_MSG (m_dcc_interval != Time(Seconds(-1.0)), "DCC interval not set");

  std::mutex& mutex = m_metric_supervisor->getCBRMutex();
  mutex.lock();

  std::unordered_map<std::string, std::vector<double>> cbrs = m_metric_supervisor->getCBRValues();
  for (auto it = cbrs.begin(); it != cbrs.end(); ++it)
    {
      double current_cbr = it->second.back();
      ReactiveState state;
      if (current_cbr < m_cbr_thresholds.m_cbr_threshold_relaxed)
        {
          state = ReactiveState::Relaxed;
        }
      else if (current_cbr < m_cbr_thresholds.m_cbr_threshold_active1)
        {
          state = ReactiveState::Active1;
        }
      else if (current_cbr < m_cbr_thresholds.m_cbr_threshold_active2)
        {
          state = ReactiveState::Active2;
        }
      else if (current_cbr < m_cbr_thresholds.m_cbr_threshold_active3)
        {
          state = ReactiveState::Active3;
        }
      else
        {
          state = ReactiveState::Restrictive;
        }

      std::string id = it->first;
      std::string numericPart = id.substr (3);
      long stationId = std::stol (numericPart);
      ReactiveState oldState;
      if (m_veh_states.find(stationId) != m_veh_states.end())
        {
          oldState = m_veh_states[stationId];
          
          if (oldState == state)
            {
              // Nothing to change in the node setup
              continue;
            }

          if (state - oldState > 1)
            {
              // If the new state is much more restrictive than the old one
              state = static_cast<ReactiveState> (oldState + 1);
            }
          else if (oldState - state > 1)
            {
              // If the new state is much less restrictive than the old one
              state = static_cast<ReactiveState> (oldState - 1);
            }
        }
      // Get the NetDevice
      Ptr<NetDevice> netDevice = m_traci_client->get_NodeMap()[id].second->GetDevice (0);

      if (m_metric_supervisor->getChannelTechnology() == "80211p")
        {
          // Get the WifiNetDevice
          Ptr<WifiNetDevice> wifiDevice = DynamicCast<WifiNetDevice>(netDevice);

          if (wifiDevice != nullptr)
            {
              // Get the PHY layer
              Ptr<WifiPhy> phy = wifiDevice->GetPhy();
              Ptr<WifiMac> mac = wifiDevice->GetMac();
              Ptr<BSContainer> bs = m_bs_map->get (stationId);
              if (bs == nullptr)
                {
                  continue;
                }
              Ptr<CABasicService> caService = m_bs_map->get (stationId)->getCABasicService();
              Ptr<VRUBasicService> vruService = m_bs_map->get (stationId)->getVRUBasicService();
              Ptr<CPBasicService> cpService = m_bs_map->get (stationId)->getCPBasicService();

              switch (state)
                {
                  case ReactiveState::Relaxed:
                    phy->SetTxPowerStart(m_reactive_parameters_relaxed.m_tx_power);
                    phy->SetTxPowerEnd(m_reactive_parameters_relaxed.m_tx_power);
                    phy->SetRxSensitivity (m_reactive_parameters_relaxed.m_sensitivity);
                    caService->setCheckCamGenMs(m_reactive_parameters_relaxed.m_tx_inter_packet_time);
                    vruService->setCheckVamGenMs(m_reactive_parameters_relaxed.m_tx_inter_packet_time);
                    cpService->setCheckCpmGenMs(m_reactive_parameters_relaxed.m_tx_inter_packet_time);
                    break;

                  case Active1:
                    phy->SetTxPowerStart(m_reactive_parameters_active1.m_tx_power);
                    phy->SetTxPowerEnd(m_reactive_parameters_active1.m_tx_power);
                    phy->SetRxSensitivity (m_reactive_parameters_active1.m_sensitivity);
                    caService->setCheckCamGenMs(m_reactive_parameters_active1.m_tx_inter_packet_time);
                    vruService->setCheckVamGenMs(m_reactive_parameters_active1.m_tx_inter_packet_time);
                    cpService->setCheckCpmGenMs(m_reactive_parameters_active1.m_tx_inter_packet_time);
                    break;

                  case Active2:
                    phy->SetTxPowerStart(m_reactive_parameters_active2.m_tx_power);
                    phy->SetTxPowerEnd(m_reactive_parameters_active2.m_tx_power);
                    phy->SetRxSensitivity (m_reactive_parameters_active2.m_sensitivity);
                    caService->setCheckCamGenMs(m_reactive_parameters_active2.m_tx_inter_packet_time);
                    vruService->setCheckVamGenMs(m_reactive_parameters_active2.m_tx_inter_packet_time);
                    cpService->setCheckCpmGenMs(m_reactive_parameters_active2.m_tx_inter_packet_time);
                    break;

                  case Active3:
                    phy->SetTxPowerStart(m_reactive_parameters_active3.m_tx_power);
                    phy->SetTxPowerEnd(m_reactive_parameters_active3.m_tx_power);
                    phy->SetRxSensitivity (m_reactive_parameters_active3.m_sensitivity);
                    caService->setCheckCamGenMs(m_reactive_parameters_active3.m_tx_inter_packet_time);
                    vruService->setCheckVamGenMs(m_reactive_parameters_active3.m_tx_inter_packet_time);
                    cpService->setCheckCpmGenMs(m_reactive_parameters_active3.m_tx_inter_packet_time);
                    break;

                  case Restrictive:
                    phy->SetTxPowerStart(m_reactive_parameters_restricted.m_tx_power);
                    phy->SetTxPowerEnd(m_reactive_parameters_restricted.m_tx_power);
                    phy->SetRxSensitivity (m_reactive_parameters_restricted.m_sensitivity);
                    caService->setCheckCamGenMs(m_reactive_parameters_restricted.m_tx_inter_packet_time);
                    vruService->setCheckVamGenMs(m_reactive_parameters_restricted.m_tx_inter_packet_time);
                    cpService->setCheckCpmGenMs(m_reactive_parameters_restricted.m_tx_inter_packet_time);
                    break;
                }
            }
          else
            {
              NS_FATAL_ERROR ("WifiNetdevice not found");
            }
        }
      else if (m_metric_supervisor->getChannelTechnology() == "Nr")
        {
          // TODO
        }
      m_veh_states[stationId] = state;
    }

  mutex.unlock();

  Simulator::Schedule(m_dcc_interval, &DCC::reactiveDCC, this);
}
}


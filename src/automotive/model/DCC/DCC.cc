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
      std::string id = it->first;
      std::string numericPart = id.substr (3);
      long stationId = std::stol (numericPart);
      double current_cbr = it->second.back();
      ReactiveState oldState = ReactiveState::Undefined;
      ReactiveState state;
      if (m_veh_states.find(stationId) != m_veh_states.end())
        {
          oldState = m_veh_states[stationId];
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

/*

Work in progress

void DCC::ToffUpdateAfterCBRupdate(){

  // Necesito el Toff y el waiting
  double aux;
  double waiting;


  waiting = Simulator::Now().GetDouble() - (LastTx);

  aux = Tonpp.GetDouble()/m_delta * (Toff.GetDouble() - waiting) / Toff.GetDouble() + waiting;

  if(aux < 25*1000000){
      aux = 25*1000000;
    }

  if(aux > 1000000000){
      Toff = Seconds(1);
    }
  else{
      Toff = NanoSeconds(aux);
    }
}


void DCC::adaptiveDCC()
{
  NS_LOG_INFO ("Starting DCC check");

  NS_ASSERT_MSG (m_bs_map != nullptr, "BS Map not set");
  NS_ASSERT_MSG (m_traci_client != nullptr, "TraCI client not set");
  NS_ASSERT_MSG (m_metric_supervisor != nullptr, "Metric Supervisor not set");
  NS_ASSERT_MSG (m_dcc_interval != Time (Seconds (-1.0)), "DCC interval not set");

  std::mutex &mutex = m_metric_supervisor->getCBRMutex ();
  mutex.lock ();

  std::unordered_map<std::string, std::vector<double>> cbrs = m_metric_supervisor->getCBRValues ();

  double delta_offset = 0;
  double aux = 0;
  double CBR = 0; // To retrieve from each node
  double CBR_Before = 0; // To retrieve from each node

  m_CBR_its = 0.5*m_CBR_its + 0.25* (CBR_Before + CBR); //Step 1

  aux = m_beta*(m_CBR_target - m_CBR_its);

  if((m_CBR_target - m_CBR_its) > 0){ // step 2

      if(aux > m_Gmax){
          delta_offset = m_Gmax;
        }else {
          delta_offset = aux;
        }

    }
  else{

      if(aux > m_Gmin){
          delta_offset = aux;
        }else{
          delta_offset = m_Gmin;
        }
    }

  m_delta = (1-m_alpha) * m_delta + delta_offset; //step 3

  if(m_delta > m_delta_max) //step 4
    m_delta = m_delta_max;

  if(m_delta < m_delta_min) // step 5
    m_delta = m_delta_min;

  ToffUpdateAfterCBRupdate();
}
*/
}


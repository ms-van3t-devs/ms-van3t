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

#include "CBRSupervisor.h"
#include <numeric>

namespace ns3 {
  NS_LOG_COMPONENT_DEFINE ("CBRSupervisor");

  std::unordered_map<std::string, std::vector<float>> currentBusyCBR;
  std::unordered_map<std::string, std::vector<float>> currentIdleCBR;
  std::unordered_map<std::string, std::vector<float>> averageCbr; //!< The exponential moving average CBR for each node

  CBRSupervisor::~CBRSupervisor ()
  {
    NS_LOG_FUNCTION (this);
  }

  void
  computeCBR (std::string context, Time start, Time duration, WifiPhyState state)
  {
    // End and start are expressed in ns
    std::size_t first = context.find ("/NodeList/") + 10; // 10 is the length of "/NodeList/"
    std::size_t last = context.find ("/", first);
    std::string node = context.substr (first, last - first);

    float duration_ns = duration.GetNanoSeconds ();
    if (state == WifiPhyState::IDLE || state == WifiPhyState::SLEEP)
      {
        currentIdleCBR[node].push_back (duration_ns);
      }
    else
      {
        currentBusyCBR[node].push_back (duration_ns);
      }
  }

  void
  CBRSupervisor::logLastCBRs ()
  {
    if (m_verbose_stdout)
      {
        std::ofstream file;
        std::cout << "CBR last values for each node:" << std::endl;
        if (m_write_to_file)
          {
            file.open ("cbr_values.txt", std::ios_base::out);
            file << "CBR last values for each node:" << std::endl;
          }
        for (auto it = averageCbr.begin (); it != averageCbr.end (); ++it)
          {
            std::string node = it->first;
            if (it->second.empty ())
              {
                continue;
              }
            float cbr = it->second.back();
            std::cout << "Node " << node << ": " << std::fixed << std::setprecision(2) << cbr * 100 << "%" << std::endl;
            if (m_write_to_file)
              {
                file << "Node " << node << ": " << std::fixed << std::setprecision(2) << cbr * 100 << "%" << std::endl;
              }
          }
        if (m_write_to_file)
          {
            file.close ();
          }
      }
  }

  void
  CBRSupervisor::checkCBR ()
  {
    std::vector<std::string> visitedNodes;

    for (auto it = currentBusyCBR.begin (); it != currentBusyCBR.end (); ++it)
      {
        std::string node = it->first;
        visitedNodes.push_back (node);
        std::vector<float> busyCbr = it->second;
        std::vector<float> idleCbr = currentIdleCBR[node]; // Assuming you ensure node exists in currentIdleCBR
        float idleSum = 0;
        if (idleCbr.empty ())
          {
            idleSum = 0;
          }
        else
          {
            idleSum = std::accumulate (idleCbr.begin (), idleCbr.end (), 0.0f);
          }

        float busySum = std::accumulate (busyCbr.begin (), busyCbr.end (), 0.0f);

        float current_cbr = busySum / (busySum + idleSum);
        if (averageCbr.find (node) != averageCbr.end ())
          {
            // Exponential moving average
            float new_cbr = m_alpha * averageCbr[node].back () + (1 - m_alpha) * current_cbr;
            averageCbr[node].push_back (new_cbr);
          }
        else
          {
            averageCbr[node].push_back (current_cbr);
          }
      }

    // Check for the nodes that hasn't listened the channel busy
    for (auto it = currentIdleCBR.begin (); it != currentIdleCBR.end (); ++it)
      {
        std::string node = it->first;
        if (std::find (visitedNodes.begin (), visitedNodes.end (), node) == visitedNodes.end ())
          {
            // The node has not been considered for the busy channel
            if (averageCbr.find (node) != averageCbr.end ())
              {
                // Exponential moving average with current CBR = 0
                float new_cbr = m_alpha * averageCbr[node].back () + (1 - m_alpha) * 0;
                averageCbr[node].push_back (new_cbr);
              }
            else
              {
                averageCbr[node].push_back (0.0);
              }
          }
      }

    currentIdleCBR.clear ();
    currentBusyCBR.clear ();

    // Schedule the next CBR check
    Simulator::Schedule (MilliSeconds (m_window), &CBRSupervisor::checkCBR, this);
  }

  void
  CBRSupervisor::startCheckCBR ()
  {
    Time current = Simulator::Now ();
    Config::Connect ("/NodeList/*/DeviceList/*/Phy/State/State", MakeCallback (&computeCBR));
    Simulator::Schedule (MilliSeconds(m_window), &CBRSupervisor::checkCBR, this);
    Simulator::Schedule (Seconds (m_simulation_time), &CBRSupervisor::logLastCBRs, this);
  }

}

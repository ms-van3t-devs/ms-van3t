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

  std::unordered_map<std::string, float> currentBusyCBR;
  std::unordered_map<std::string, std::vector<float>> averageCbr; //!< The exponential moving average CBR for each node
  std::mutex mutex;

  CBRSupervisor::~CBRSupervisor ()
  {
    NS_LOG_FUNCTION (this);
  }

  void
  storeCBR80211p (std::string context, Time start, Time duration, WifiPhyState state)
  {
    // End and start are expressed in ns
    std::size_t first = context.find ("/NodeList/") + 10; // 10 is the length of "/NodeList/"
    std::size_t last = context.find ("/", first);
    std::string node = context.substr (first, last - first);

    float durationNs = duration.GetNanoSeconds ();
    if (state != WifiPhyState::IDLE && state != WifiPhyState::SLEEP)
      {
        mutex.lock();
        if (currentBusyCBR[node] == -1.0 || currentBusyCBR.find(node) == currentBusyCBR.end())
          {
            currentBusyCBR[node] = durationNs;
          } else
          {
            currentBusyCBR[node] += durationNs;
          }
        mutex.unlock();
      }
  }

  void
  storeCBRNr(std::string context, Time duration)
  {
    std::size_t first = context.find ("/NodeList/") + 10; // 10 is the length of "/NodeList/"
    std::size_t last = context.find ("/", first);
    std::string node = context.substr (first, last - first);
    float durationNs = duration.GetNanoSeconds ();
    mutex.lock();
    if (currentBusyCBR[node] == -1.0 || currentBusyCBR.find(node) == currentBusyCBR.end())
      {
        currentBusyCBR[node] = durationNs;
      } else
      {
        currentBusyCBR[node] += durationNs;
      }
    mutex.unlock();
  }

  void
  CBRSupervisor::checkCBR ()
  {
    mutex.lock();

    for (auto it = currentBusyCBR.begin (); it != currentBusyCBR.end (); ++it)
      {
        std::string node = it->first;
        float busyCbr = it->second;

        if (busyCbr == -1.0)
          {
            continue;
          }

        float currentCbr = busyCbr / (m_window * 1e6);
        if (averageCbr.find (node) != averageCbr.end ())
          {
            // Exponential moving average
            float new_cbr = m_alpha * averageCbr[node].back () + (1 - m_alpha) * currentCbr;
            averageCbr[node].push_back (new_cbr);
          }
        else
          {
            averageCbr[node].push_back (currentCbr);
          }
        it->second = -1.0;
      }

    mutex.unlock();

    Simulator::Schedule (MilliSeconds (m_window), &CBRSupervisor::checkCBR, this);
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
  CBRSupervisor::startCheckCBR ()
  {

    if (m_channel_technology == "80211p")
      {
        Config::Connect ("/NodeList/*/DeviceList/*/Phy/State/State", MakeCallback (&storeCBR80211p));
      } else if (m_channel_technology == "Nr")
      {
        Config::Connect("/NodeList/*/DeviceList/*/$ns3::NrUeNetDevice/ComponentCarrierMapUe/*/NrUePhy/NrSpectrumPhyList/*/ChannelOccupied", MakeCallback(&storeCBRNr));
      }
    Simulator::Schedule (MilliSeconds(m_window), &CBRSupervisor::checkCBR, this);
    Simulator::Schedule (Seconds (m_simulation_time), &CBRSupervisor::logLastCBRs, this);

  }

  std::tuple<std::string, float>
  CBRSupervisor::getCBRForNode (std::string node)
  {
    if (averageCbr.find (node) != averageCbr.end ())
      {
        return std::make_tuple (node, averageCbr[node].back ());
      } else {
        return std::make_tuple (node, -1.0);
      }
  }

}

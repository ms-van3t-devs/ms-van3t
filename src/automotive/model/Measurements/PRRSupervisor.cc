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
 *  Marco Malinverno, Politecnico di Torino (marco.malinverno1@gmail.com)
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/

#include "PRRSupervisor.h"
#include <sstream>
#include <cfloat>

#define DEG_2_RAD(val) ((val)*M_PI/180.0)

namespace {
  double PRRSupervisor_haversineDist(double lat_a, double lon_a, double lat_b, double lon_b) {
      // 12742000 is the mean Earth radius (6371 km) * 2 * 1000 (to convert from km to m)
      return 12742000.0*asin(sqrt(sin(DEG_2_RAD(lat_b-lat_a)/2)*sin(DEG_2_RAD(lat_b-lat_a)/2)+cos(DEG_2_RAD(lat_a))*cos(DEG_2_RAD(lat_b))*sin(DEG_2_RAD(lon_b-lon_a)/2)*sin(DEG_2_RAD(lon_b-lon_a)/2)));
  }
}

namespace ns3 {
  NS_LOG_COMPONENT_DEFINE("PRRSupervisor");

  TypeId
  PRRSupervisor::GetTypeId ()
  {
    static TypeId tid = TypeId("ns3::PRRSupervisor")
        .SetParent <Object>()
        .AddConstructor <PRRSupervisor>();
    return tid;
  }

  PRRSupervisor::~PRRSupervisor ()
  {
    NS_LOG_FUNCTION(this);

    std::list<EventId>::iterator eventit = eventList.begin();
    while(eventit != eventList.end())
    {
      if(eventit->IsExpired() == false)
      {
        Simulator::Cancel(*eventit);
      }
      eventit = eventList.erase(eventit);
    }
  }

  std::string
  PRRSupervisor::bufToString(uint8_t *buf, uint32_t bufsize)
  {
    std::stringstream bufss;

    bufss << std::hex << std::setfill('0');

    for(size_t i=0;i<bufsize;++i)
    {
      bufss << std::setw(2) << static_cast<unsigned>(buf[i]);
    }

    return bufss.str();
  }

  void
  PRRSupervisor::signalSentPacket(std::string buf, double lat, double lon, uint64_t vehicleID)
  {
    EventId computePRR_id;

    if(m_traci_ptr == nullptr)
    {
      NS_FATAL_ERROR("Fatal error: TraCI client not set in PRR Supervisor.");
    }

    std::vector<std::string> ids = m_traci_ptr->TraCIAPI::vehicle.getIDList ();

    for(std::vector<std::string>::iterator it=ids.begin();it!=ids.end();++it)
    {
      uint64_t stationID = std::stol(it->substr (3));
      libsumo::TraCIPosition pos = m_traci_ptr->TraCIAPI::vehicle.getPosition (*it);
      pos=m_traci_ptr->TraCIAPI::simulation.convertXYtoLonLat (pos.x,pos.y);

      if(PRRSupervisor_haversineDist(lat,lon,pos.y,pos.x)<=m_baseline_m)
      {
          m_packetbuff_map[buf].vehList.push_back(stationID);
      }
    }

    computePRR_id = Simulator::Schedule(Seconds (3), &PRRSupervisor::computePRR, this, buf);
    eventList.push_back (computePRR_id);

    m_latency_map[buf] = Simulator::Now ().GetNanoSeconds ();

    m_vehicleid_map[buf] = vehicleID;
  }

  void
  PRRSupervisor::signalReceivedPacket(std::string buf, uint64_t vehicleID)
  {
    baselineVehicleData_t currBaselineData;
    double curr_latency_ms = DBL_MAX;

    if(m_traci_ptr == nullptr)
    {
      NS_FATAL_ERROR("Fatal error: TraCI client not set in PRR Supervisor.");
    }

    if(m_packetbuff_map.count(buf)>0)
    {
      currBaselineData = m_packetbuff_map[buf];

      if(std::find(currBaselineData.vehList.begin(), currBaselineData.vehList.end(), vehicleID) != currBaselineData.vehList.end())
      {
        (m_packetbuff_map[buf].x)++;
      }
    }

    // Compute latency in ms
    if(m_latency_map.count(buf)>0)
    {
        uint64_t senderID = m_vehicleid_map[buf];

        curr_latency_ms = static_cast<double>(Simulator::Now ().GetNanoSeconds () - m_latency_map[buf])/1000000.0;
        m_count_latency++;

        m_avg_latency_ms += (curr_latency_ms-m_avg_latency_ms)/m_count_latency;

        if(m_count_latency_per_veh.count(senderID)<=0) {
            m_count_latency_per_veh[senderID]=0;
        }

        if(m_avg_latency_ms_per_veh.count(senderID)<=0) {
            m_avg_latency_ms_per_veh[senderID]=0;
        }

        m_count_latency_per_veh[senderID]++;
        m_avg_latency_ms_per_veh[senderID] += (curr_latency_ms - m_avg_latency_ms_per_veh[senderID])/m_count_latency_per_veh[senderID];
    }
  }

  void
  PRRSupervisor::computePRR(std::string buf)
  {
    double PRR = 0.0;

    if(m_packetbuff_map[buf].vehList.size()>1)
    {
      uint64_t senderID = m_vehicleid_map[buf];

      PRR = (double) m_packetbuff_map[buf].x/(double) (m_packetbuff_map[buf].vehList.size()-1.0);
      m_count++;
      m_avg_PRR += (PRR-m_avg_PRR)/m_count;

      if(m_count_per_veh.count(senderID)<=0) {
          m_count_per_veh[senderID]=0;
      }

      if(m_avg_PRR_per_veh.count(senderID)<=0) {
          m_avg_PRR_per_veh[senderID]=0;
      }

      m_count_per_veh[senderID]++;
      m_avg_PRR_per_veh[senderID] += (PRR-m_avg_PRR_per_veh[senderID])/m_count_per_veh[senderID];

      m_packetbuff_map.erase(buf);
      m_vehicleid_map.erase(buf);
    }

    // Some time has passed -> remove the packet from the latency map too
    m_latency_map.erase(buf);
  }
}

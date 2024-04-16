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
PRRSupervisor::signalSentPacket(std::string buf, double lat, double lon, uint64_t nodeID, messageType_e messagetype)
{
  EventId computePRR_id;

  if(m_traci_ptr == nullptr)
    {
      NS_FATAL_ERROR("Fatal error: TraCI client not set in PRR Supervisor.");
    }

  // If the packet is sent by an excluded vehicle due to a problem in the configuration of the simulation, ignore it
  if(m_excluded_vehID_enabled==true && (m_excluded_vehID_list.find(nodeID)!=m_excluded_vehID_list.end()))
    {
      return;
    }

  //std::vector<std::string> ids = m_traci_ptr->TraCIAPI::vehicle.getIDList ();
  std::map< std::string, std::pair< StationType_t, Ptr<Node> > > node_map = m_traci_ptr->get_NodeMap ();

  for(std::map< std::string, std::pair< StationType_t, Ptr<Node> > >::iterator it=node_map.begin();it!=node_map.end();++it)
    {
      uint64_t stationID = std::stol(it->first.substr (3));
      StationType_t station_type = it->second.first;

      libsumo::TraCIPosition pos;
      if(station_type == StationType_pedestrian)
        pos = m_traci_ptr->TraCIAPI::person.getPosition (it->first);
      else
        pos = m_traci_ptr->TraCIAPI::vehicle.getPosition (it->first);
      pos=m_traci_ptr->TraCIAPI::simulation.convertXYtoLonLat (pos.x,pos.y);

      if(stationID == nodeID)
        m_stationtype_map[buf] = station_type;

      if(m_excluded_vehID_enabled==false || (m_excluded_vehID_list.find(stationID)==m_excluded_vehID_list.end())) {
          if(PRRSupervisor_haversineDist(lat,lon,pos.y,pos.x)<=m_baseline_m)
            {
              m_packetbuff_map[buf].nodeList.push_back(stationID);
            }
        }
    }

  computePRR_id = Simulator::Schedule(MilliSeconds (m_pprcomp_timeout*1000.0), &PRRSupervisor::computePRR, this, buf);
  eventList.push_back (computePRR_id);

  m_latency_map[buf] = Simulator::Now ().GetNanoSeconds ();

  m_vehicleid_map[buf] = nodeID;

  m_messagetype_map[buf] = messagetype;
}

void
PRRSupervisor::signalReceivedPacket(std::string buf, uint64_t nodeID)
{
  baselineVehicleData_t currBaselineData;
  double curr_latency_ms = DBL_MAX;

  if(m_traci_ptr == nullptr)
    {
      NS_FATAL_ERROR("Fatal error: TraCI client not set in PRR Supervisor.");
    }

  // If the packet was received by an excluded vehicle due to a problem in the configuration of the simulation, ignore it
  if(m_excluded_vehID_enabled==true && (m_excluded_vehID_list.find(nodeID)!=m_excluded_vehID_list.end()))
    {
      return;
    }

  // If the packet was sent by an excluded vehicle due to a problem in the configuration of the simulation, it will be automatically
  // ignored as it will not be in the m_packetbuff_map and m_packetbuff_map.count(buf)==0
  if(m_packetbuff_map.count(buf)>0)
    {
      currBaselineData = m_packetbuff_map[buf];

      if(std::find(currBaselineData.nodeList.begin(), currBaselineData.nodeList.end(), nodeID) != currBaselineData.nodeList.end())
        {
          (m_packetbuff_map[buf].x)++;
        }
    }

  // Compute latency in ms
  if(m_latency_map.count(buf)>0)
    {
      uint64_t senderID = m_vehicleid_map[buf];
      messageType_e messagetype = m_messagetype_map[buf];

      curr_latency_ms = static_cast<double>(Simulator::Now ().GetNanoSeconds () - m_latency_map[buf])/1000000.0;
      m_count_latency++;

      m_avg_latency_ms += (curr_latency_ms-m_avg_latency_ms)/m_count_latency;

      StationType_t station_type = m_stationtype_map[buf];
      if(station_type == StationType_pedestrian){

          if(m_count_latency_per_ped.count(senderID)<=0) {
              m_count_latency_per_ped[senderID]=0;
            }

          if(m_avg_latency_ms_per_ped.count(senderID)<=0) {
              m_avg_latency_ms_per_ped[senderID]=0;
            }

          m_count_latency_per_ped[senderID]++;
          m_avg_latency_ms_per_ped[senderID] += (curr_latency_ms - m_avg_latency_ms_per_ped[senderID])/m_count_latency_per_ped[senderID];

        }
      else{
          if(m_count_latency_per_veh.count(senderID)<=0) {
              m_count_latency_per_veh[senderID]=0;
            }

          if(m_avg_latency_ms_per_veh.count(senderID)<=0) {
              m_avg_latency_ms_per_veh[senderID]=0;
            }

          m_count_latency_per_veh[senderID]++;
          m_avg_latency_ms_per_veh[senderID] += (curr_latency_ms - m_avg_latency_ms_per_veh[senderID])/m_count_latency_per_veh[senderID];
        }

      if(m_count_latency_per_messagetype.count(messagetype)<=0) {
          m_count_latency_per_messagetype[messagetype]=0;
        }

      if(m_avg_latency_ms_per_messagetype.count(messagetype)<=0) {
          m_avg_latency_ms_per_messagetype[messagetype]=0;
        }

      m_count_latency_per_messagetype[messagetype]++;
      m_avg_latency_ms_per_messagetype[messagetype] += (curr_latency_ms - m_avg_latency_ms_per_messagetype[messagetype])/m_count_latency_per_messagetype[messagetype];

      if(m_verbose_stdout == true) {
          std::cout << "|Latency| ID: " << nodeID << " Current: " << curr_latency_ms << " - Average: " << m_avg_latency_ms << std::endl;
        }
    }
}

void
PRRSupervisor::computePRR(std::string buf)
{
  double PRR = 0.0;

  if(m_packetbuff_map[buf].nodeList.size()>1)
    {
      uint64_t senderID = m_vehicleid_map[buf];
      messageType_e messagetype = m_messagetype_map[buf];
      StationType_t station_type = m_stationtype_map[buf];

      PRR = (double) m_packetbuff_map[buf].x/(double) (m_packetbuff_map[buf].nodeList.size()-1.0);

      if(PRR>1) {
          std::cerr << "Value of X: " << (double) m_packetbuff_map[buf].x << " - value of Y: " << (double) (m_packetbuff_map[buf].nodeList.size()-1.0) << std::endl;
          NS_FATAL_ERROR ("Error. Computed a PRR greater than 1. This is not possible. Please check how you configured your simulation and the PRRSupervisor.");
        }

      m_count++;
      m_avg_PRR += (PRR-m_avg_PRR)/m_count;

      if(m_verbose_stdout == true) {
          std::cout << "|PRR| Current: " << PRR << " - Average: " << m_avg_PRR << std::endl;
        }

      if (station_type == StationType_pedestrian){
          if(m_count_per_ped.count(senderID)<=0) {
              m_count_per_ped[senderID]=0;
            }

          if(m_avg_PRR_per_ped.count(senderID)<=0) {
              m_avg_PRR_per_ped[senderID]=0;
            }

          m_count_per_ped[senderID]++;
          m_avg_PRR_per_ped[senderID] += (PRR-m_avg_PRR_per_ped[senderID])/m_count_per_ped[senderID];
        }
      else
        {
          if(m_count_per_veh.count(senderID)<=0) {
              m_count_per_veh[senderID]=0;
            }

          if(m_avg_PRR_per_veh.count(senderID)<=0) {
              m_avg_PRR_per_veh[senderID]=0;
            }

          m_count_per_veh[senderID]++;
          m_avg_PRR_per_veh[senderID] += (PRR-m_avg_PRR_per_veh[senderID])/m_count_per_veh[senderID];
        }

      if(m_count_per_messagetype.count(messagetype)<=0) {
          m_count_per_messagetype[messagetype]=0;
        }

      if(m_avg_PRR_per_messagetype.count(messagetype)<=0) {
          m_avg_PRR_per_messagetype[messagetype]=0;
        }

      m_count_per_messagetype[messagetype]++;
      m_avg_PRR_per_messagetype[messagetype] += (PRR-m_avg_PRR_per_messagetype[messagetype])/m_count_per_messagetype[messagetype];

      m_packetbuff_map.erase(buf);
      m_vehicleid_map.erase(buf);
      m_messagetype_map.erase(buf);
    }

  // Some time has passed -> remove the packet from the latency map too
  m_latency_map.erase(buf);
}
}
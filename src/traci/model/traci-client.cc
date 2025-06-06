/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
 * Copyright (c) 2018 TU Dresden
 *
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
 *
 * Authors: Patrick Schmager <patrick.schmager@tu-dresden.de>
 *          Sebastian Kuehlmorgen <sebastian.kuehlmorgen@tu-dresden.de>
 */

#include <exception>
#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

#include "traci-client.h"

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE("TraciClient");

  TypeId
  TraciClient::GetTypeId(void)
  {
    static TypeId tid =
        TypeId("ns3::TraciClient").SetParent<Object>()
    .SetGroupName ("TraciClient")
    .AddAttribute ("SumoConfigPath",
                  "Path to SUMO configuration file.",
                  StringValue (""),
                  MakeStringAccessor (&TraciClient::m_sumoConfigPath),
                  MakeStringChecker ())
    .AddAttribute ("SumoBinaryPath",
                  "Path to SUMO binary file.",
                  StringValue (""),
                  MakeStringAccessor (&TraciClient::m_sumoBinaryPath),
                  MakeStringChecker ())
    .AddAttribute ("SumoPort",
                  "Port on which SUMO/Traci is listening for connection.",
                  UintegerValue (1338),
                  MakeUintegerAccessor (&TraciClient::m_sumoPort),
                  MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("SumoWaitForSocket",
                  "Wait XX sec (=1e6 microsec) until sumo opens socket for traci connection.",
                  TimeValue (ns3::Seconds(1.0)),
                  MakeTimeAccessor (&TraciClient::m_sumoWaitForSocket),
                  MakeTimeChecker ())
    .AddAttribute ("SumoGUI",
                  "Turn SUMO GUI on/off.",
                  BooleanValue (false),
                  MakeBooleanAccessor (&TraciClient::m_sumoGUI),
                  MakeBooleanChecker ())
    .AddAttribute ("SumoAdditionalCmdOptions",
                  "Additional commandline options for SUMO start-up.",
                  StringValue (""),
                  MakeStringAccessor (&TraciClient::m_sumoAddCmdOpt),
                  MakeStringChecker ())
    .AddAttribute ("SumoSeed",
                  "Random Seed for SUMO.",
                  IntegerValue (0),
                  MakeIntegerAccessor (&TraciClient::m_sumoSeed),
                  MakeIntegerChecker<int> ())
    .AddAttribute ("SumoLogFile",
                  "Creates a SUMO error log file when set to true.",
                  BooleanValue (false),
                  MakeBooleanAccessor (&TraciClient::m_sumoLogFile),
                  MakeBooleanChecker ())
    .AddAttribute ("SumoStepLog", "Turns SUMO commandline step output on, if gui is turned off.",
                  BooleanValue (false),
                  MakeBooleanAccessor (&TraciClient::m_sumoStepLog),
                  MakeBooleanChecker ())
    .AddAttribute ("SynchInterval",
                  "Time interval for synchronizing the two simulators.",
                  TimeValue (ns3::Seconds(1.0)),
                  MakeTimeAccessor (&TraciClient::m_synchInterval),
                  MakeTimeChecker ())
    .AddAttribute ("StartTime",
                  "Start time of SUMO simulator; Offset time between ns3 and sumo simulation.",
                  TimeValue (ns3::Seconds(0.0)),
                  MakeTimeAccessor (&TraciClient::m_startTime),
                  MakeTimeChecker ())
    .AddAttribute ("PenetrationRate", "Rate of vehicles, equipped with wireless communication devices",
                  DoubleValue (1.0),
                  MakeDoubleAccessor (&TraciClient::m_penetrationRate),
                  MakeDoubleChecker<double> ())
    .AddAttribute ("Altitude",
                  "Altitude of nodes in meter",
                  DoubleValue (1.5),
                  MakeDoubleAccessor (&TraciClient::m_altitude),
                  MakeDoubleChecker<double> ())
    .AddAttribute ("VehicleVisualizer",
                  "Vehicle visualizer client",
                  PointerValue (0),
                  MakePointerAccessor (&TraciClient::m_vehicle_visualizer),
                  MakePointerChecker<vehicleVisualizer> ())
    .AddAttribute ("UseNetworkNamespace",
                  "Name of the network namespace to be used to launch SUMO",
                   StringValue (""),
                   MakeStringAccessor (&TraciClient::m_netns_name),
                   MakeStringChecker ());
  ;
    return tid;
  }

  TraciClient::TraciClient(void)
  {
    NS_LOG_FUNCTION(this);

    m_sumoSeed = 0;
    m_altitude = 1.5;
    m_sumoPort = 1338;
    m_sumoGUI = false;
    m_penetrationRate = 1.0;
    m_sumoLogFile = false;
    m_sumoStepLog = false;
    m_sumoWaitForSocket = ns3::Seconds(1.0);
    m_vehicle_visualizer = nullptr;
    m_netns_name = "";
  }

  TraciClient::~TraciClient(void)
  {
    NS_LOG_FUNCTION(this);
    SumoStop();
  }

  void
  TraciClient::SumoStop()
  {
    NS_LOG_FUNCTION(this);

    try
      {
        this->TraCIAPI::close();
      }
    catch (std::exception& e)
      {
        terminateVehicleVisualizer();
        NS_FATAL_ERROR("Problem while closing traci socket: " << e.what());
      }
  }

  std::string
  TraciClient::GetVehicleId(Ptr<Node> node)
  {
    NS_LOG_FUNCTION(this);

    std::string foundNode("");

    // search map for corresponding node
    for (std::map<std::string, std::pair< StationType_t, Ptr<Node> > >::iterator it = m_NodeMap.begin(); it != m_NodeMap.end(); ++it)
      {
        if (it->second.second == node)
          {
            foundNode = it->first;
            break;
          }
      }

    return foundNode;
  }

  std::string
  TraciClient::GetSumoCmdString(void)
  {
    NS_LOG_FUNCTION(this);

    if (m_sumoConfigPath == "")
      {
        terminateVehicleVisualizer();
        NS_FATAL_ERROR("Error: No path specified for sumo configuration! Use .SetAttribute('m_sumoConfigPath', ...) before calling .SetupSUMO");
      }

    // sumo gui
    if (m_sumoGUI)
      {
        // m_sumoCommand = m_sumoBinaryPath + "sumo-gui.exe -c"; // <- to connect to the Windows version of SUMO under WSL2
        m_sumoCommand = m_sumoBinaryPath + "sumo-gui -c";
      }
    else
      {
        // m_sumoCommand = m_sumoBinaryPath + "sumo.exe -c"; // <- to connect to the Windows version of SUMO under WSL2
        m_sumoCommand = m_sumoBinaryPath + "sumo -c";
      }

    // sumo path
    m_sumoCommand += " " + m_sumoConfigPath;

    // remote port
    m_sumoCommand += " --remote-port " + std::to_string(m_sumoPort);

    // synchronisation interval
    m_sumoCommand += " --step-length " + std::to_string(m_synchInterval.GetSeconds());

    // sumo log file
    if (m_sumoLogFile)
      {
        int pos = m_sumoConfigPath.find_last_of("/\\");
        std::string sumoDir = m_sumoConfigPath.substr(0, pos);
        m_sumoCommand += " --error-log " + sumoDir + "/SumoError.log";
      }

    // sumo step log
    if (m_sumoStepLog)
      {
        m_sumoCommand += " --no-step-log false";
      }
    else
      {
        m_sumoCommand += " --no-step-log true";
      }

    // sumo random seed
    if (m_sumoSeed)
      {
        m_sumoCommand += " --seed " + std::to_string(m_sumoSeed);
      }

    // sumo additional command line options
    m_sumoCommand += " " + m_sumoAddCmdOpt;
    m_sumoCommand += " --start --quit-on-end &";

    return m_sumoCommand;
  }

  void
  TraciClient::SumoSetup(STARTUP_FCN includeNode, SHUTDOWN_FCN excludeNode)
  {
    NS_LOG_FUNCTION(this);

    m_sumoPort = GetFreePort(m_sumoPort);

    m_includeNode = includeNode;
    m_excludeNode = excludeNode;
    m_sumoCommand = GetSumoCmdString();

    if(m_netns_name != "")
    {
        if(geteuid() != 0)
        {
            NS_FATAL_ERROR("Error. Setting a network namespace for SUMO requires root privileges or 'sudo'");
        }
        m_sumoCommand = "sudo ip netns exec " + m_netns_name + " " + m_sumoCommand;
        NS_LOG_INFO("SUMO will be launched on Network namespace: " + m_netns_name);
    }

    // start up sumo
    int startCmd = std::system(m_sumoCommand.c_str());
    if (startCmd)
      {
        NS_LOG_INFO("Used the following command to start up sumo: " << m_sumoCommand);
      }

    // wait 1 sec (=1e6 microsec) until sumo opens socket for traci connection
    std::cout << "Sumo: wait for socket: " << m_sumoWaitForSocket.GetSeconds() << "s" << std::endl;
    usleep(m_sumoWaitForSocket.GetMicroSeconds());

    // connect to sumo via traci
    try
      {
        // this->TraCIAPI::connect("172.23.208.1", m_sumoPort); // <- to connect to the Windows version of SUMO under WSL2 (Windows "host IP" needs to be customized)
        this->TraCIAPI::connect("localhost", m_sumoPort);
      }
    catch (std::exception& e)
      {
        terminateVehicleVisualizer();
        NS_FATAL_ERROR("Can not connect to sumo via traci: " << e.what());
      }

    if (m_vehicle_visualizer!=nullptr && m_vehicle_visualizer->isConnected())
    {
        /* Compute central position of the map to be sent to the web visualizer */
        libsumo::TraCIPositionVector net_boundaries = this->TraCIAPI::simulation.getNetBoundary ();
        libsumo::TraCIPosition pos1;
        libsumo::TraCIPosition pos2;
        double lon,lat;
        /* Convert (x,y) to (long,lat) */
        // Long = x, Lat = y
        pos1 = this->TraCIAPI::simulation.convertXYtoLonLat (net_boundaries[0].x,net_boundaries[0].y);
        pos2 = this->TraCIAPI::simulation.convertXYtoLonLat (net_boundaries[1].x,net_boundaries[1].y);
        /* Check the center of the map */
        lon = (pos1.x + pos2.x)/2;
        lat = (pos1.y + pos2.y)/2;
        int rval = m_vehicle_visualizer->sendMapDraw(lat,lon);
        if (rval<0)
        {
            NS_FATAL_ERROR("Error: cannot send the map coordinates to the vehicle visualizer.");
        }
    }


    // start sumo and simulate until the specified time
    this->TraCIAPI::simulationStep(m_startTime.GetSeconds());

    // synchronise sumo vehicles with ns3 nodes
    SynchroniseNodeMap();

    // get current positions from sumo and uptdate positions
    UpdatePositions();

    // schedule event to command sumo the next simulation step
    Simulator::Schedule(m_synchInterval, &TraciClient::SumoSimulationStep, this);
  }

  void
  TraciClient::SumoSimulationStep()
  {
    NS_LOG_FUNCTION(this);

    try
      {
        // get current simulation time
        auto nextTime = Simulator::Now().GetSeconds() + m_synchInterval.GetSeconds() + m_startTime.GetSeconds();

        // command sumo to simulate next time step
        this->TraCIAPI::simulationStep(nextTime);

        // include a ns3 node for every new sumo vehicle/pedestrian and exclude arrived vehicles/pedestrians
        SynchroniseNodeMap();

        // ask sumo for new vehicle/pedestrian positions and update node positions
        UpdatePositions();

        // schedule next event to simulate next time step in sumo
        Simulator::Schedule(m_synchInterval, &TraciClient::SumoSimulationStep, this);
      }
    catch (std::exception& e)
      {
        terminateVehicleVisualizer();
        NS_FATAL_ERROR("Sumo was closed unexpectedly during simulation: " << e.what());
      }
  }

  void
  TraciClient::UpdatePositions()
  {
    NS_LOG_FUNCTION(this);

    try
      {
        // iterate over all nodes in the map
        for (std::map<std::string, std::pair< StationType_t, Ptr<Node> > >::iterator it = m_NodeMap.begin(); it != m_NodeMap.end(); ++it)
          {
            // get current vehicle/pedestrian from the map
            std::string node_ID(it->first);

            // get vehicle/pedestrian position from sumo
            libsumo::TraCIPosition pos;
            if(it->second.first == StationType_pedestrian)
               pos = this->TraCIAPI::person.getPosition(node_ID);
            else if (it->second.first == StationType_roadSideUnit)
              continue;
            else
               pos = this->TraCIAPI::vehicle.getPosition(node_ID);

            // get corresponding ns3 node from map
            Ptr<MobilityModel> mob = m_NodeMap.at(node_ID).second->GetObject<MobilityModel>();
            // set ns3 node position with user defined altitude
            mob->SetPosition(Vector(pos.x, pos.y, m_altitude));

            if (m_sionna == true)
            {
              Vector pos_for_sionna = Vector(pos.x, pos.y, m_altitude);
              double angle_for_sionna = this->TraCIAPI::vehicle.getAngle(node_ID);
              double speed = this->TraCIAPI::vehicle.getSpeed(node_ID);
              Vector vel_for_sionna = Vector(speed * cos(angle_for_sionna), speed * sin(angle_for_sionna), 0.0);
              updateLocationInSionna(node_ID, pos_for_sionna, angle_for_sionna, vel_for_sionna);
            }
            
            if (m_vehicle_visualizer!=nullptr && m_vehicle_visualizer->isConnected() && it->second.first != StationType_pedestrian)
            {
                libsumo::TraCIPosition lonlat = this->TraCIAPI::simulation.convertXYtoLonLat (pos.x,pos.y);
                int rval = m_vehicle_visualizer->sendObjectUpdate (node_ID,lonlat.y,lonlat.x,this->TraCIAPI::vehicle.getAngle (node_ID));
                if (rval<0)
                {
                    NS_FATAL_ERROR("Error: cannot send the object update to the vehicle visualizer for vehicle: "<<node_ID);
                }
            }
          }
      }
    catch (std::exception& e)
      {
        terminateVehicleVisualizer();
        NS_FATAL_ERROR("SUMO was closed unexpectedly while asking for vehicle positions: " << e.what());
      }
  }

  void
  TraciClient::GetSumoVehicles(std::vector<std::string>& sumoVehicles)
  {
    NS_LOG_FUNCTION(this);

    // initialize uniform random distribution for penetration rate
    Ptr<UniformRandomVariable> randVar = CreateObject<UniformRandomVariable>();
    randVar->SetAttribute("Min", DoubleValue(0.0));
    randVar->SetAttribute("Max", DoubleValue(1.0));
    sumoVehicles.clear();

    try
      {
        // ask sumo for all (new) departed vehicles SINCE last simulation step (=one synch interval)
        std::vector<std::string> departedVehicles = this->TraCIAPI::simulation.getDepartedIDList();

        // ask sumo for all (new) arrived vehicles SINCE last simulation step (=one synch interval)
        std::vector<std::string> arrivedVehicles = this->TraCIAPI::simulation.getArrivedIDList();

        // iterate over departed vehicles
        for (std::vector<std::string>::iterator it = departedVehicles.begin(); it != departedVehicles.end(); ++it)
          {
            // get departed vehicle
            std::string veh(*it);

            // search for same vehicle in arrived vehicles
            std::vector<std::string>::iterator pos = std::find(arrivedVehicles.begin(), arrivedVehicles.end(), veh);

            // if vehicle is found in both lists, ignore it; all others are considered as relevant vehicles for simulation
            if (pos != arrivedVehicles.end())
              {
                arrivedVehicles.erase(pos);
              }
            else
              {
                // penetration rate determines number of included nodes
                if (randVar->GetValue() <= m_penetrationRate)
                  {
                    sumoVehicles.push_back(veh);
                  }
              }
          }

        // iterate over arrived vehicles
        for (std::vector<std::string>::iterator it = arrivedVehicles.begin(); it != arrivedVehicles.end(); ++it)
          {
            // get arrived vehicle
            std::string veh(*it);

            // search for arrived vehicle in vehicleNodeMap
            std::map<std::string, std::pair< StationType_t, Ptr<Node> > >::iterator pos = m_NodeMap.find(veh);

            // if node is in map, exclude it, otherwise is was not simulated in ns3 because of the penetration rate
            if (pos != m_NodeMap.end())
              {
                sumoVehicles.push_back (veh);
              }
          }
      }
    catch (std::exception& e)
      {
        terminateVehicleVisualizer();
        NS_FATAL_ERROR("SUMO was closed unexpectedly while asking for arrived/departed vehicles: " << e.what());
      }
  }

  void
  TraciClient::SynchroniseNodeMap()
  {
    NS_LOG_FUNCTION(this);

    try
      {
        // get departed and arrived sumo vehicles since last simulation step
        std::vector<std::string> sumoVehicles;
        GetSumoVehicles(sumoVehicles);

        // iterate over all sumo vehicles with changes; include departed vehicles, exclude arrived vehicles
        for (std::vector<std::string>::iterator it = sumoVehicles.begin(); it != sumoVehicles.end(); ++it)
          {
            // get current vehicle
            std::string veh(*it);

            // search for vehicle in vehicleNodeMap
            std::map<std::string,std::pair< StationType_t, Ptr<Node> >>::iterator pos = m_NodeMap.find(veh);

            // if it is already in the map, remove it and exclude node
            if (pos != m_NodeMap.end())
              {
                // get corresponding ns3 node
                Ptr<ns3::Node> exNode = m_NodeMap.at(veh).second;

                // call exclude function for this node
                m_excludeNode(exNode,veh);

                // unregister in map
                m_NodeMap.erase(veh);
              }
            else // if it is not in the map, create a new ns3 node for it
              {
                // create new node by calling the include function
                std::pair<StationType_t, Ptr<ns3::Node>> inNode;
                inNode.first = StationType_passengerCar;
                inNode.second = m_includeNode(veh,StationTypeTraci_vehicle);

                // register in the map (link vehicle to node!)
                m_NodeMap.insert(std::pair<std::string, std::pair<StationType_t, Ptr<ns3::Node>>>(veh, inNode));
              }
          }

        // Get all pedestrians present in the simulation
        std::vector<std::string> sumoPed = this->TraCIAPI::simulation.getPedList ();
        if(!sumoPed.empty()){
            m_pedlist_empty = false;
          }

        if(!m_pedlist_empty){
            // Iterate over all pedestrians present in the simulation
            for (std::vector<std::string>::iterator it = sumoPed.begin(); it != sumoPed.end(); ++it){
                // Get current pedestrian
                std::string ped(*it);

                // Search for pedestrian in the node map
                std::map<std::string, std::pair< StationType_t, Ptr<Node> > >::iterator pos = m_NodeMap.find(ped);

                // If the pedestrian is not present in the node map yet, include it
                if (pos == m_NodeMap.end()){
                    // Create the new node by calling the include function
                    std::pair<StationType_t, Ptr<ns3::Node>> inNode_ped;
                    inNode_ped.first = StationType_pedestrian;
                    inNode_ped.second = m_includeNode(ped,StationTypeTraci_pedestrian);

                    // Register the new node in the map
                    m_NodeMap.insert(std::pair<std::string, std::pair<StationType_t, Ptr<ns3::Node>>>(ped, inNode_ped));
                  }
              }

            // Iterate over all nodes present in the node map
            for (auto it = m_NodeMap.begin(); it != m_NodeMap.end(); /* no increment here */){
                std::string node_ID = it->first;

                // Check if the extracted node corresponds to a pedestrian
                if(it->second.first == StationType_pedestrian){
                    // Search for the given node among the pedestrians present in the simulation
                    auto iter = std::find(sumoPed.begin(), sumoPed.end(), node_ID);

                    // If the node is not currently present in the simulation, remove it
                    if(iter == sumoPed.end()){
                        // get corresponding ns3 node
                        Ptr<ns3::Node> exNode_ped = it->second.second;

                        // Call exclude function for this node
                        m_excludeNode(exNode_ped,node_ID);

                        // Unregister in map and update iterator
                        it = m_NodeMap.erase(it);
                      } else {
                        ++it;
                      }
                  } else {
                    ++it;
                  }
              }
          }
      }
    catch (std::exception& e)
      {
        terminateVehicleVisualizer();
        NS_FATAL_ERROR("SUMO was closed unexpectedly while updating the node map: " << e.what());
      }
  }

uint32_t
TraciClient::GetVehicleMapSize()
{
return m_NodeMap.size();
}

void
TraciClient::terminateVehicleVisualizer(void)
{
  if (m_vehicle_visualizer!=nullptr && m_vehicle_visualizer->isConnected())
  {
      m_vehicle_visualizer->terminateServer ();
  }
}

bool
TraciClient::PortFreeCheck (uint32_t portNum)
{
    int socketFd;
    struct sockaddr_in address;

    // Creating socket file descriptor
    if ((socketFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
      perror("socket failed");
      exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( portNum );

    // Forcefully attaching socket to the specified port
    if (bind(socketFd, (struct sockaddr *)&address, sizeof(address))<0)
    {
      // port not available
      return false;
    }
    else
    {
      // port available
      ::close(socketFd); // goto to top empty namespace to avoid conclict with TraCIAPI::close()
      return true;
    }
}

uint32_t
TraciClient::GetFreePort (uint32_t portNum)
{
    uint32_t port = portNum;
    while (!PortFreeCheck(port))
    {
      ++port;
    }

return port;
}

std::vector<std::string>
TraciClient::getVehicleNodeMapIds()
{
    std::vector<std::string> ids;
    for (auto & it : m_NodeMap)
    {
      ids.push_back(it.first);
    }
    return ids;
}

void TraciClient::AddStation(std::string id, float x, float y, float z, Ptr<Node> node)
{
  // Add RSU to the map
  std::pair<StationType_t, Ptr<ns3::Node>> inNode;
  inNode.first = StationType_roadSideUnit;
  inNode.second = node;

  // register in the map (link vehicle to node!)
  m_NodeMap.insert(std::pair<std::string, std::pair<StationType_t, Ptr<ns3::Node>>>(id, inNode));

  // Set the position of the Station
  Ptr<MobilityModel> mob = node->GetObject<MobilityModel>();
  mob->SetPosition(Vector(x, y, z));
}

std::string TraciClient::GetStationId(Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);

  std::string foundNode ("");

  // search map for corresponding node
  for (auto it = m_NodeMap.begin (); it != m_NodeMap.end (); ++it)
    {
      if (it->second.second == node)
        {
          foundNode = it->first;
          break;
        }
    }

  return foundNode;
}

} // namespace ns3
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
 *  Carlos Mateo Risma Carletti, Politecnico di Torino (carlosrisma@gmail.com)
*/

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/sumo_xml_parser.h"
#include "ns3/mobility-module.h"
#include "ns3/traci-module.h"
#include "ns3/v2xEmulator-helper.h"
#include "ns3/emu-fd-net-device-helper.h"
#include "ns3/v2xEmulator.h"
#include "ns3/packet-socket-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("v2x-emulator");

int
main (int argc, char *argv[])
{
  /* In this example the vehicles generated will redirect all their V2X messages toward the physical
   * interface specified in the --interface option. By default, in this example the vehicles generate CAMs with a variable
   * frequency (see ETSI CAM generation standard) and 1 DENM every second.
   * Both CAM and DENM dissemination che be stopped by specifying
   * --send-cam = false
   * --send-denm = false
   *
   * Normally each message is encapsulated into BTP+GeoNetworking and broadcasted. However, you can also specify:
   * --udp = <ip address>:<port>
   * to encapsulate all the messages inside BTP+GeoNetworking+UDP+IPv4 and send them in an unicast fashion (UDP mode).
   * Additionally, in UDP mode, you can also configure subnet, netmask and gateway in the emulated nodes.
   *
   * To start this application, you need to set the interface where the V2X messages are relayed into promiscuous mode!
   * command -> sudo ip link set <interface name> promisc on
   */

  // Physical interface parameters
  std::string deviceName ("eth1");
  std::string encapMode ("Dix");

  // UDP mode parameters
  std::string udpIp = "";
  std::string gwstr = "192.168.1.1";
  std::string subnet = "192.168.1.0";
  std::string netmask = "255.255.255.0";
  int destPort=0;
  Ipv4Address destAddr;
  Ipv4AddressHelper ipv4helper;

  // Simulation parameters
  std::string sumo_folder = "src/automotive/examples/sumo_files_v2x_map/";
  std::string mob_trace = "cars.rou.xml";
  std::string sumo_config = "src/automotive/examples/sumo_files_v2x_map/map.sumo.cfg";
  std::string sumo_netns = "";
  bool sendCam = true;
  bool sendDenm = true;
  bool sendCpm = true;

  bool verbose = true;
  bool sumo_gui = true;
  double sumo_updates = 0.01;
  bool print_summary = false;

  double emuTime = 100;

  int numberOfNodes = 0;
  uint32_t nodeCounter = 0;

  CommandLine cmd;
  xmlDocPtr rou_xml_file;

  /* Cmd Line option for vehicular application */
  cmd.AddValue ("sumo-gui", "Use SUMO gui or not", sumo_gui);
  cmd.AddValue ("sumo-updates", "SUMO granularity", sumo_updates);
  cmd.AddValue ("sumo-folder","Position of sumo config files",sumo_folder);
  cmd.AddValue ("mob-trace", "Name of the mobility trace file", mob_trace);
  cmd.AddValue ("sumo-config", "Location and name of SUMO configuration file", sumo_config);
  cmd.AddValue ("sumo-netns","[Advanced users] Name of the network namespace SUMO shall be launched in. If not specified, SUMO is launched normally.",sumo_netns);
  cmd.AddValue ("summary", "Print a summary for each vehicle at the end of the simulation", print_summary);
  cmd.AddValue ("verbose", "Enable verbose printing on stdout", verbose);
  cmd.AddValue ("interface", "Name of the physical interface to send V2X messages to", deviceName);
  cmd.AddValue ("sim-time", "Total duration of the emulation [s]", emuTime);
  cmd.AddValue ("send-cam", "To trigger the CAM dissemination", sendCam);
  cmd.AddValue ("send-cpm", "To trigger the CPM dissemination", sendCpm);
  cmd.AddValue ("send-denm", "To trigger the DENM dissemination", sendDenm);
  cmd.AddValue ("udp", "[UDP mode] To enable UDP mode and specify UDP port and IP address where the V2X messages are redirected (format: <IP>:<port>)", udpIp);
  cmd.AddValue ("gateway", "[UDP mode] To specify the gateway at which the UDP/IP packets will be sent", gwstr);
  cmd.AddValue ("subnet", "[UDP mode] To specify the subnet which will be used to assign the IP addresses of emulated nodes (the .1 address is automatically excluded)", subnet);
  cmd.AddValue ("netmask", "[UDP mode] To specify the netmask of the network", netmask);

  cmd.Parse (argc, argv);

  /* If verbose is true, enable some additional logging */
  if (verbose)
    {
      LogComponentEnable ("v2x-emulator", LOG_LEVEL_INFO);
      LogComponentEnable ("CABasicService", LOG_LEVEL_INFO);
      LogComponentEnable ("DENBasicService", LOG_LEVEL_INFO);
    }

  /* Using the real-time scheduler is mandatory when emulating vehicles */
  /* WARNING: you must be sure that the computer is capable enough to simulate
   * the desired number of vehicles, otherwise the simulation will slow down
   * and the real-time constraint will not be respected anymore (causing, for
   * instance, the vehicles to look as if they were moving more slowly and
   * sending CAMs with a lower frequency) */
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  /* Read the SUMO mobility information and map from the .rou XML file */
  NS_LOG_INFO("Reading the .rou file...");
  std::string path = sumo_folder + mob_trace;

  /* Load the .rou.xml document */
  xmlInitParser();
  rou_xml_file = xmlParseFile(path.c_str ());
  if (rou_xml_file == NULL)
    {
      NS_FATAL_ERROR("Error: unable to parse the specified XML file: "<<path);
    }
  numberOfNodes = XML_rou_count_vehicles(rou_xml_file);

  xmlFreeDoc(rou_xml_file);
  xmlCleanupParser();

  if(numberOfNodes==0)
    {
      NS_FATAL_ERROR("Fatal error: cannot gather the number of vehicles from the specified XML file: "<<path<<". Please check if it is a correct SUMO file.");
    }
  NS_LOG_INFO("The .rou file has been read: " << numberOfNodes << " vehicles will be present in the simulation.");

  // When --udp is specified, parse the IPv4 and port parameters
  if (udpIp != "")
    {
      std::istringstream udpIpStream(udpIp);
      std::string curr_str;
      // IP address
      std::getline(udpIpStream, curr_str, ':');

      destAddr = Ipv4Address(curr_str.c_str());

      // Port
      std::getline(udpIpStream, curr_str, ':');
      destPort=std::stoi(curr_str);

      if(destPort<=0 || destPort > 65535)
        {
          NS_FATAL_ERROR("Error: "<<destPort<<" is not a valid port for UDP operations.");
        }

      std::cout<< "V2X Messages will be sent to:"<<destAddr<<":"<<destPort << std::endl;
    }

  /* Set the emulation total time (in seconds) */
  NS_LOG_INFO("Simulation will last " << emuTime << " seconds");
  ns3::Time simulationTime (ns3::Seconds(emuTime));

  /* Create containers for OBUs */
  NodeContainer obuNodes;
  obuNodes.Create(numberOfNodes);

  if (udpIp != "")
    {
      InternetStackHelper internet;
      internet.SetIpv4StackInstall (true);
      internet.Install (obuNodes);
      ipv4helper.SetBase (subnet.c_str(), netmask.c_str());
      ipv4helper.NewAddress (); // "Burn" the first IP as it may be assigned to a router in the local network
    }

  /* Setup Mobility and position node pool */
  MobilityHelper mobility;
  mobility.Install (obuNodes);

  /* Setup Traci and start SUMO */
  Ptr<TraciClient> sumoClient = CreateObject<TraciClient> ();
  sumoClient->SetAttribute ("SumoConfigPath", StringValue (sumo_config));
  sumoClient->SetAttribute ("SumoBinaryPath", StringValue (""));    // use system installation of sumo
  sumoClient->SetAttribute ("SynchInterval", TimeValue (Seconds (sumo_updates)));
  sumoClient->SetAttribute ("StartTime", TimeValue (Seconds (0.0)));
  sumoClient->SetAttribute ("SumoGUI", (BooleanValue) sumo_gui);
  sumoClient->SetAttribute ("SumoPort", UintegerValue (3400));
  sumoClient->SetAttribute ("PenetrationRate", DoubleValue (1.0));
  sumoClient->SetAttribute ("SumoLogFile", BooleanValue (false));
  sumoClient->SetAttribute ("SumoStepLog", BooleanValue (false));
  sumoClient->SetAttribute ("SumoSeed", IntegerValue (10));
  sumoClient->SetAttribute ("SumoAdditionalCmdOptions", StringValue ("--verbose true"));
  sumoClient->SetAttribute ("SumoWaitForSocket", TimeValue (Seconds (1.0)));
  sumoClient->SetAttribute ("SumoAdditionalCmdOptions", StringValue ("--collision.action warn --collision.check-junctions --error-log=sumo-errors-or-collisions.xml"));
  sumoClient->SetAttribute ("UseNetworkNamespace", StringValue(sumo_netns));

  /* Create the OBU application (v2xEmulator, see also model/v2xEmulator.cc/.h) */
  v2xEmulatorHelper emuHelper;
  emuHelper.SetAttribute ("Client", (PointerValue) sumoClient); // pass TraciClient object for accessing sumo in application
  emuHelper.SetAttribute ("SendCAM", (BooleanValue) sendCam);
  emuHelper.SetAttribute ("SendDENM", (BooleanValue) sendDenm);
  emuHelper.SetAttribute ("SendCPM", (BooleanValue) sendCpm);
  if(udpIp!="")
  {
    emuHelper.SetAttribute ("DestinationIPv4", Ipv4AddressValue(destAddr));
    emuHelper.SetAttribute ("DestinationPort", IntegerValue(destPort));
    emuHelper.SetAttribute ("UDPmode",(BooleanValue) true);
  }

  /* Create the FdNetDevice to send packets over a physical interface */
  EmuFdNetDeviceHelper emuDev;
  emuDev.SetDeviceName (deviceName);
  emuDev.SetAttribute ("EncapsulationMode", StringValue (encapMode));

  /* Give packet socket powers to nodes (otherwise, if the app tries to create a PacketSocket, CreateSocket will end up with a segmentation fault */
  if (udpIp=="")
  {
    PacketSocketHelper packetSocket;
    packetSocket.Install (obuNodes);
  }

  NetDeviceContainer fdnetContainer;
  Ipv4InterfaceContainer ipv4ic;
  Ipv4Address gateway (gwstr.c_str());
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  /* Callback function for node creation */
  STARTUP_FCN setupNewEmuNode = [&] (std::string vehicleID) -> Ptr<Node>
    {
      if (nodeCounter >= obuNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      std::cout<<"Creating node: "<<nodeCounter<<std::endl;

      /* don't create and install the protocol stack of the node at simulation time -> take from "node pool" */
      Ptr<Node> includedNode = obuNodes.Get(nodeCounter);
      ++nodeCounter; //increment counter for next node

      /* Install FdNetDevice and set MAC address (using 00:00:00:00:00:01, 00:00:00:00:00:02, and so on, for each vehicle) */
      std::ostringstream veh_mac;
      veh_mac << "00:00:00:00:00:" << std::setfill('0') << std::setw(2) << nodeCounter;
      fdnetContainer = emuDev.Install(includedNode);
      Ptr<FdNetDevice> dev = fdnetContainer.Get (0)->GetObject<FdNetDevice> ();
      dev->SetAddress (Mac48Address (veh_mac.str().c_str ()));

      std::cout<<"MAC of node "<<nodeCounter-1<<": "<<veh_mac.str()<<std::endl;

      /* When in UDP mode, configure the IP stack of the nodes/vehicles */
      if(udpIp!="")
      {
         Ptr<Ipv4> ipv4 = includedNode->GetObject<Ipv4> ();
         uint32_t interface = ipv4->AddInterface (dev);
         ipv4ic = ipv4helper.Assign (fdnetContainer);
         Ipv4InterfaceAddress address = Ipv4InterfaceAddress (ipv4ic.GetAddress (0,0), netmask.c_str());
         ipv4->AddAddress (interface, address);
         ipv4->SetMetric (interface, 1);
         ipv4->SetUp (interface);

         Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting (ipv4);
         staticRouting->SetDefaultRoute (gateway, interface);
         std::cout<<"IPv4 of node "<<nodeCounter-1<<": "<<ipv4ic.GetAddress (0,0)<<std::endl;
      }

      /* Install Application */
      ApplicationContainer v2xEmulatorApp = emuHelper.Install (includedNode);
      v2xEmulatorApp.Start (Seconds (0.0));
      v2xEmulatorApp.Stop (simulationTime - Simulator::Now () - Seconds (0.1));

      std::cout<<"New node: "<<nodeCounter-1<<std::endl;

      return includedNode;
    };

  /* Callback function for node shutdown */
  SHUTDOWN_FCN shutdownEmuNode = [] (Ptr<Node> exNode,std::string vehicleID)
    {
      /* stop all applications */
      Ptr<v2xEmulator> v2xEmulatorApp_ = exNode->GetApplication(0)->GetObject<v2xEmulator>();
      if(v2xEmulatorApp_)
        v2xEmulatorApp_->StopApplicationNow ();

       /* set position outside communication range */
      Ptr<ConstantPositionMobilityModel> mob = exNode->GetObject<ConstantPositionMobilityModel>();
      mob->SetPosition(Vector(-1000.0+(rand()%25),320.0+(rand()%25),250.0));// rand() for visualization purposes

      /* NOTE: further actions could be required for a safe shut down! */
    };

  /* Start traci client with the given function pointers */
  sumoClient->SumoSetup (setupNewEmuNode, shutdownEmuNode);

  /* Start Emulation */
  Simulator::Stop (simulationTime);

  Simulator::Run ();
  Simulator::Destroy ();
}

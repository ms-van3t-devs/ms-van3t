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

#include "ns3/automotive-module.h"
#include "ns3/traci-module.h"
#include "ns3/internet-module.h"
#include "ns3/wave-module.h"
#include "ns3/mobility-module.h"
#include "ns3/sumo_xml_parser.h"
#include "ns3/packet-socket-helper.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("v2i-80211p");

int
main (int argc, char *argv[])
{
  /*
   * In this example the generated vehicles will broadcast their CAMs, that will be also received by a RSU placed in the middle
   * of the simulated scenario. Both vehicles and RSU transmit V2X messages through a 802.11p interface.
   * The RSU broadcasts DENM messages with a frequency of 1 Hz, using the ETSI ITS-G5 stack, and the GeoNet
   * dissemination area is set as a circular area around the RSU (90 meters of diameter).
   * The dissemination of DENMs starts only after the RSU receives a CAM. If for 5 seconds
   * the RSU does not receive any CAM, the DENM dissemination stops.
   * Each DENM includes an optional container (à la carte), where there is a field named RoadWorks->SpeedLimit, in which
   * the maximum speed allowed in the dissemination area is specified (25km/h).
   * Whenever a vehicle receives a DENM at application layer (meaning that the information traverses GeoNet without being
   * filtered), it reads the information inside the à la carte container and corrects its speed accordingly.
   * If a vehicle doesn't receive any DENM for more than 1.5 seconds, it resumes its old speed.
   */

  // Admitted data rates for 802.11p
  std::vector<float> rate_admitted_values{3,4.5,6,9,12,18,24,27};
  std::string datarate_config;

  /*** 0.a App Options ***/
  std::string sumo_folder = "src/automotive/examples/sumo_files_v2i_map/";
  std::string mob_trace = "cars.rou.xml";
  std::string sumo_config ="src/automotive/examples/sumo_files_v2i_map/map.sumo.cfg";

  bool verbose = true;
  bool realtime = false;
  bool sumo_gui = true;
  bool aggregate_out = false;
  double sumo_updates = 0.01;
  std::string csv_name;
  bool print_summary = false;
  int txPower=26;
  float datarate=12;

  double simTime = 100;

  int numberOfNodes;
  uint32_t nodeCounter = 0;

  CommandLine cmd;

  xmlDocPtr rou_xml_file;

  /* Cmd Line option for vehicular application */
  cmd.AddValue ("realtime", "Use the realtime scheduler or not", realtime);
  cmd.AddValue ("sumo-gui", "Use SUMO gui or not", sumo_gui);
  cmd.AddValue ("server-aggregate-output", "Print an aggregate output for server", aggregate_out);
  cmd.AddValue ("sumo-updates", "SUMO granularity", sumo_updates);
  cmd.AddValue ("sumo-folder","Position of sumo config files",sumo_folder);
  cmd.AddValue ("mob-trace", "Name of the mobility trace file", mob_trace);
  cmd.AddValue ("sumo-config", "Location and name of SUMO configuration file", sumo_config);
  cmd.AddValue ("csv-log", "Name of the CSV log file", csv_name);
  cmd.AddValue ("summary", "Print a summary for each vehicle at the end of the simulation", print_summary);

  /* Cmd Line option for 802.11p */
  cmd.AddValue ("tx-power", "OBUs transmission power [dBm]", txPower);
  cmd.AddValue ("datarate", "802.11p channel data rate [Mbit/s]", datarate);

  cmd.AddValue ("sim-time", "Total duration of the simulation [s]", simTime);

  cmd.Parse (argc, argv);

  if(std::find(rate_admitted_values.begin(), rate_admitted_values.end(), datarate) == rate_admitted_values.end())
    {
      NS_FATAL_ERROR("Fatal error: invalid 802.11p data rate" << datarate << "Mbit/s. Valid rates are: 3, 4.5, 6, 9, 12, 18, 24, 27 Mbit/s.");
    }
  else
    {
      if(datarate==4.5)
        {
          datarate_config = "OfdmRate4_5MbpsBW10MHz";
        }
      else
        {
          datarate_config = "OfdmRate" + std::to_string((int)datarate) + "MbpsBW10MHz";
        }
    }

  if (verbose)
    {
      LogComponentEnable ("v2i-80211p", LOG_LEVEL_INFO);
      LogComponentEnable ("CABasicService", LOG_LEVEL_INFO);
      LogComponentEnable ("DENBasicService", LOG_LEVEL_INFO);
      LogComponentEnable ("areaSpeedAdvisorServer80211p", LOG_LEVEL_INFO);
    }

  /* Use the realtime scheduler of ns3 */
  if(realtime)
      GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  /*** 0.b Read from the mob_trace the number of vehicles that will be created.
   *       The number of vehicles is directly parsed from the rou.xml file, looking at all
   *       the valid XML elements of type <vehicle>
  ***/
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

  if(numberOfNodes==-1)
    {
      NS_FATAL_ERROR("Fatal error: cannot gather the number of vehicles from the specified XML file: "<<path<<". Please check if it is a correct SUMO file.");
    }
  NS_LOG_INFO("The .rou file has been read: " << numberOfNodes << " vehicles will be present in the simulation.");

  /* Set the simulation time (in seconds) */
  NS_LOG_INFO("Simulation will last " << simTime << " seconds");
  ns3::Time simulationTime (ns3::Seconds(simTime));

  /*** 1. Create containers for OBUs ***/
  NodeContainer obuNodes;
  obuNodes.Create(numberOfNodes+1); //+1 for the server

  /*** 2. Create and setup channel objects
       the network topology created is the following:

       OBUs->(WIFI CHANNEL)->RSU->RemoteHost

   ***/
  /*** 2. Create and setup channel   ***/
  YansWifiPhyHelper wifiPhy;
  wifiPhy.Set ("TxPowerStart", DoubleValue (txPower));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (txPower));
  NS_LOG_INFO("Setting up the 802.11p channel @ " << datarate << " Mbit/s, 10 MHz, and tx power " << (int)txPower << " dBm.");

  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);
  
  /*** 3. Create and setup MAC ***/
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (datarate_config), "ControlMode", StringValue (datarate_config));
  NetDeviceContainer netDevices = wifi80211p.Install (wifiPhy, wifi80211pMac, obuNodes);

  //wifi80211p.EnableLogComponents ();

  //wifiPhy.EnablePcap ("v2i-test",netDevices);
  /* Give packet socket powers to nodes (otherwise, if the app tries to create a PacketSocket, CreateSocket will end up with a segmentation fault */
  PacketSocketHelper packetSocket;
  packetSocket.Install (obuNodes);

  /*** 6. Setup Mobility and position node pool ***/
  MobilityHelper mobility;
  mobility.Install (obuNodes);

  /* Set the RSU to a fixed position (i.e. on the center of the map, in this case) */
  Ptr<MobilityModel> mobilityRSU = obuNodes.Get (0)->GetObject<MobilityModel> ();
  mobilityRSU->SetPosition (Vector (0, 0, 20.0)); // Normally, in SUMO, (0,0) is the center of the map

  /*** 5. Setup Traci and start SUMO ***/
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

  /*** 6. Create and Setup application for the server ***/
  areaSpeedAdvisorServer80211pHelper AreaSpeedAdvisorServer80211pHelper;
  AreaSpeedAdvisorServer80211pHelper.SetAttribute ("Client", (PointerValue) sumoClient);
  AreaSpeedAdvisorServer80211pHelper.SetAttribute ("RealTime", BooleanValue(realtime));
  AreaSpeedAdvisorServer80211pHelper.SetAttribute ("AggregateOutput", BooleanValue(aggregate_out));
  AreaSpeedAdvisorServer80211pHelper.SetAttribute ("CSV", StringValue(csv_name));

  ApplicationContainer AppServer = AreaSpeedAdvisorServer80211pHelper.Install (obuNodes.Get (0));

  AppServer.Start (Seconds (0.0));
  AppServer.Stop (simulationTime - Seconds (0.1));
  ++nodeCounter;

  /*** 7. Setup interface and application for dynamic nodes ***/
  areaSpeedAdvisorClient80211pHelper AreaSpeedAdvisorClient80211pHelper;
  Ipv4Address remoteHostAddr;

  AreaSpeedAdvisorClient80211pHelper.SetAttribute ("ServerAddr", Ipv4AddressValue(remoteHostAddr));
  AreaSpeedAdvisorClient80211pHelper.SetAttribute ("Client", (PointerValue) sumoClient); // pass TraciClient object for accessing sumo in application
  AreaSpeedAdvisorClient80211pHelper.SetAttribute ("PrintSummary", BooleanValue(print_summary));
  AreaSpeedAdvisorClient80211pHelper.SetAttribute ("RealTime", BooleanValue(realtime));
  AreaSpeedAdvisorClient80211pHelper.SetAttribute ("CSV", StringValue(csv_name));

  /* callback function for node creation */
  std::function<Ptr<Node> ()> setupNewWifiNode = [&] () -> Ptr<Node>
    {
      if (nodeCounter >= obuNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      /* Don't create and install the protocol stack of the node at simulation time -> take from "node pool" */
      Ptr<Node> includedNode = obuNodes.Get(nodeCounter);
      ++nodeCounter; //increment counter for next node

      /* Install Application */
      ApplicationContainer ClientApp = AreaSpeedAdvisorClient80211pHelper.Install (includedNode);
      ClientApp.Start (Seconds (0.0));
      ClientApp.Stop (simulationTime - Simulator::Now () - Seconds (0.1));

      return includedNode;
    };

  /* callback function for node shutdown */
  std::function<void (Ptr<Node>)> shutdownWifiNode = [] (Ptr<Node> exNode)
    {
      /* Stop all applications */
      Ptr<areaSpeedAdvisorClient80211p> areaSpeedAdvisorClient80211p_ = exNode->GetApplication(0)->GetObject<areaSpeedAdvisorClient80211p>();
      if(areaSpeedAdvisorClient80211p_)
        areaSpeedAdvisorClient80211p_->StopApplicationNow ();

       /* Set position outside communication range */
      Ptr<ConstantPositionMobilityModel> mob = exNode->GetObject<ConstantPositionMobilityModel>();
      mob->SetPosition(Vector(-1000.0+(rand()%25),320.0+(rand()%25),250.0));// rand() for visualization purposes

      /* NOTE: further actions could be required for a safe shut down! */
    };

  /* Start traci client with given function pointers */
  sumoClient->SumoSetup (setupNewWifiNode, shutdownWifiNode);

  /*** 8. Start Simulation ***/
  Simulator::Stop (simulationTime);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

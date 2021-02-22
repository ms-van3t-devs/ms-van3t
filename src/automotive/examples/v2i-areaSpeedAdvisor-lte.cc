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
#include "ns3/lte-helper.h"
#include "ns3/config-store.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/sumo_xml_parser.h"
#include "ns3/point-to-point-helper.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("v2i-lte");

int
main (int argc, char *argv[])
{
  /*
   * In this example the generated vehicles will send their CAMs toward their LTE Uu interface. The messages will be directed
   * to a server connected to the EPC of the LTE network. In this case, as opposed to 802.11p, it is not possible
   * to broadcast the V2X messages, that will instead be encapsulated inside UDP-IPv4. Thus, each message will have the
   * following encapsulation: BTP+GeoNet+UDP+IPv4+{LTE}. Due to the impossibility of broadcasting messages, also the
   * application logic changes a bit (with respect to 802.11p).
   * In this case, the server computes (at the beginning of the simulation) two areas: the inner area, where the maximum
   * speed is meant to be 25 km/h, and an outer area, where the maximum speed is meant to be 75 km/h. The server monitors
   * the position of the vehicles by reading the CAMs information. As soon as a vehicle performs a transition between the
   * two areas, the server will generate a DENM including an optional container (à la carte), where there is a field named
   * RoadWorks->SpeedLimit that is used to tell the vehicle the maximum allowed speed.
   */

  std::string sumo_folder = "src/automotive/examples/sumo_files_v2i_map/";
  std::string mob_trace = "cars.rou.xml";
  std::string sumo_config ="src/automotive/examples/sumo_files_v2i_map/map.sumo.cfg";

  /*** 0.a App Options ***/
  bool verbose = true;
  bool realtime = false;
  bool sumo_gui = true;
  bool aggregate_out = false;
  double sumo_updates = 0.01;
  std::string csv_name;
  bool print_summary = false;

  /*** 0.b LENA Options ***/
  double interPacketInterval = 100;
  bool useCa = false;

  int numberOfNodes;
  uint32_t nodeCounter = 0;

  xmlDocPtr rou_xml_file;

  double simTime = 100;

  CommandLine cmd;

  /* Cmd Line option for vehicular application */
  cmd.AddValue ("realtime", "Use the realtime scheduler or not", realtime);
  cmd.AddValue ("sumo-gui", "Use SUMO gui or not", sumo_gui);
  cmd.AddValue ("server-aggregate-output", "Print an aggregate output for server", aggregate_out);
  cmd.AddValue ("sumo-updates", "SUMO granularity", sumo_updates);
  cmd.AddValue ("sumo-folder","Position of sumo config files",sumo_folder);
  cmd.AddValue ("mob-trace", "Name of the mobility trace file", mob_trace);
  cmd.AddValue ("sumo-config", "Location and name of SUMO configuration file", sumo_config);
  cmd.AddValue ("csv-log", "Name of the CSV log file", csv_name);

  /* Cmd Line option for Lena */
  cmd.AddValue("interPacketInterval", "Inter packet interval [ms]", interPacketInterval);
  cmd.AddValue("useCa", "Whether to use carrier aggregation", useCa);

  cmd.AddValue("sim-time", "Total duration of the simulation [s]", simTime);

  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("v2i-lte", LOG_LEVEL_INFO);
      LogComponentEnable ("CABasicService", LOG_LEVEL_INFO);
      LogComponentEnable ("DENBasicService", LOG_LEVEL_INFO);
    }

  /* Carrier aggregation for LTE */
  if (useCa)
   {
     Config::SetDefault ("ns3::LteHelper::UseCa", BooleanValue (useCa));
     Config::SetDefault ("ns3::LteHelper::NumberOfComponentCarriers", UintegerValue (2));
     Config::SetDefault ("ns3::LteHelper::EnbComponentCarrierManager", StringValue ("ns3::RrComponentCarrierManager"));
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

  /*** 1. Create LTE objects
       the network topology created is the following:

       UEs->(LTE CHANNEL)->enB->(SGW->PGW)->RemoteHost
                                  ^EPC^
   ***/
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  /*** 2. Create (and configure) the remote host that will gather the CAM and send the DENM ***/
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  /* Connect the remote host to the packet gateway and create the Internet */
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("10Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.005)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  /* interface 0 is localhost, 1 is the p2p device */
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  /*** 3. Create containers for UEs and eNB ***/
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(1);
  ueNodes.Create(numberOfNodes);

  /*** 4. Create and install mobility (SUMO will be attached later) ***/
  MobilityHelper mobility;
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);

  /* Set the eNB to a fixed position */
  Ptr<MobilityModel> mobilityeNBn = enbNodes.Get (0)->GetObject<MobilityModel> ();
  mobilityeNBn->SetPosition (Vector (0, 0, 20.0)); // Normally, in SUMO, (0,0) is the center of the map

  /*** 5. Install LTE Devices to the nodes + assign IP to UE***/
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  /* Install the IP stack on the UEs */
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;

  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

  /* Assign IP address to UEs */
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      /* Set the default gateway for the UE */
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  for (uint16_t i = 0; i < numberOfNodes; i++)
      {
        lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(0));
        /* side effect: the default EPS bearer will be activated */
      }

  /*** 6. Setup Traci and start SUMO ***/
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

  /*** 7. Create and Setup application for the server ***/
  areaSpeedAdvisorServerLTEHelper AreaSpeedAdvisorServerLTEHelper;
  AreaSpeedAdvisorServerLTEHelper.SetAttribute ("Client", (PointerValue) sumoClient);
  AreaSpeedAdvisorServerLTEHelper.SetAttribute ("RealTime", BooleanValue(realtime));
  AreaSpeedAdvisorServerLTEHelper.SetAttribute ("AggregateOutput", BooleanValue(aggregate_out));
  AreaSpeedAdvisorServerLTEHelper.SetAttribute ("CSV", StringValue(csv_name));

  ApplicationContainer AppServer = AreaSpeedAdvisorServerLTEHelper.Install (remoteHostContainer.Get (0));

  AppServer.Start (Seconds (0.0));
  AppServer.Stop (simulationTime - Seconds (0.1));

  /*** 8. Setup interface and application for dynamic nodes ***/
  areaSpeedAdvisorClientLTEHelper AreaSpeedAdvisorClientLTEHelper;
  AreaSpeedAdvisorClientLTEHelper.SetAttribute ("ServerAddr", Ipv4AddressValue(remoteHostAddr));
  AreaSpeedAdvisorClientLTEHelper.SetAttribute ("Client", (PointerValue) sumoClient); // pass TraciClient object for accessing sumo in application
  AreaSpeedAdvisorClientLTEHelper.SetAttribute ("PrintSummary", BooleanValue(print_summary));
  AreaSpeedAdvisorClientLTEHelper.SetAttribute ("RealTime", BooleanValue(realtime));
  AreaSpeedAdvisorClientLTEHelper.SetAttribute ("CSV", StringValue(csv_name));

  /* callback function for node creation */
  std::function<Ptr<Node> ()> setupNewWifiNode = [&] () -> Ptr<Node>
    {
      if (nodeCounter >= ueNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      /* Don't create and install the protocol stack of the node at simulation time -> take from "node pool" */
      Ptr<Node> includedNode = ueNodes.Get(nodeCounter);
      ++nodeCounter; // increment counter for next node

      /* Install Application */
      ApplicationContainer ClientApp = AreaSpeedAdvisorClientLTEHelper.Install (includedNode);
      ClientApp.Start (Seconds (0.0));
      ClientApp.Stop (simulationTime - Simulator::Now () - Seconds (0.1));

      return includedNode;
    };

  /* Callback function for node shutdown */
  std::function<void (Ptr<Node>)> shutdownWifiNode = [] (Ptr<Node> exNode)
    {
      /* Stop all applications */
      Ptr<areaSpeedAdvisorClientLTE> appClient_ = exNode->GetApplication(0)->GetObject<areaSpeedAdvisorClientLTE>();

      if(appClient_)
        appClient_->StopApplicationNow ();

       /* Set position outside communication range */
      Ptr<ConstantPositionMobilityModel> mob = exNode->GetObject<ConstantPositionMobilityModel>();
      mob->SetPosition(Vector(-1000.0+(rand()%25),320.0+(rand()%25),250.0));// rand() for visualization purposes

      /* NOTE: further actions could be required for a safe shut down! */
    };

  /* start traci client with given function pointers */
  sumoClient->SumoSetup (setupNewWifiNode, shutdownWifiNode);

  /* To enable statistics collection of LTE module */
  //lteHelper->EnableTraces ();

  /*** 9. Start Simulation ***/
  Simulator::Stop (simulationTime);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

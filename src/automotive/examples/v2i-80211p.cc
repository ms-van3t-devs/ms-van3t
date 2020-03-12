#include "ns3/automotive-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/traci-applications-module.h"
#include "ns3/network-module.h"
#include "ns3/traci-module.h"
#include "ns3/wave-module.h"
#include "ns3/ocb-wifi-mac.h"
#include "ns3/wifi-80211p-helper.h"
#include "ns3/wave-mac-helper.h"
#include "ns3/netanim-module.h"
#include <functional>
#include <stdlib.h>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("v2i-80211p-sandbox");

int
main (int argc, char *argv[])
{
  /*** 0.a App Options ***/
  bool verbose = true;
  bool realtime = false;
  bool sumo_gui = true;
  bool aggregate_out = true;
  double sumo_updates = 0.01;
  bool send_cam = true;
  bool asn = false;
  double cam_intertime = 0.1;
  std::string sumo_folder = "src/automotive/examples/sumo-files/";
  std::string mob_trace = "cars.rou.xml";
  std::string sumo_config ="src/automotive/examples/sumo-files/map.sumo.cfg";

  double simTime = 100;

  uint16_t numberOfNodes;
  uint32_t nodeCounter = 0;

  CommandLine cmd;

  /* Cmd Line option for vehicular application */
  cmd.AddValue ("realtime", "Use the realtime scheduler or not", realtime);
  cmd.AddValue ("sumo-gui", "Use SUMO gui or not", sumo_gui);
  cmd.AddValue ("server-aggregate-output", "Print an aggregate output for server", aggregate_out);
  cmd.AddValue ("sumo-updates", "SUMO granularity", sumo_updates);
  cmd.AddValue ("send-cam", "Enable car to send cam", send_cam);
  cmd.AddValue ("sumo-folder","Position of sumo config files",sumo_folder);
  cmd.AddValue ("asn", "Use ASN.1 or plain-text to send message", asn);
  cmd.AddValue ("mob-trace", "Name of the mobility trace file", mob_trace);
  cmd.AddValue ("sumo-config", "Location and name of SUMO configuration file", sumo_config);
  cmd.AddValue ("cam-intertime", "CAM dissemination inter-time [s]", cam_intertime);

  cmd.AddValue("sim-time", "Total duration of the simulation [s])", simTime);

  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("TraciClient", LOG_LEVEL_INFO);
      LogComponentEnable ("v2i-CAM-sender", LOG_LEVEL_INFO);
      LogComponentEnable ("v2i-DENM-sender", LOG_LEVEL_INFO);
    }

  /* Use the realtime scheduler of ns3 */
  if(realtime)
      GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  /*** 0.b Read from the mob_trace the number of vehicles that will be created.
   *      The file should begin with something like:
   *      <!-- number of vehicles:2 -->
   *      The file must be included in the sumo-folder
  ***/
  std::string path = sumo_folder + mob_trace;
  std::ifstream infile (path);
  std::string num_client;
  /* Read the file*/
  if (infile.good())
    {
      getline(infile, num_client);
    }
  infile.close();
  /* Manipulate the string to get only the number of vehicles present */
  num_client.erase (0,24);
  num_client.erase (num_client.end ()-4,num_client.end ());
  numberOfNodes = std::stoi (num_client)+1;

  ns3::Time simulationTime (ns3::Seconds(simTime));

  /*** 1. Create containers for OBUs and RSU ***/
  NodeContainer wifiNodes;
  wifiNodes.Create(numberOfNodes+1);

  /*** 2. Create and setup channel objects
       the network topology created is the following:

       OBUs->(WIFI CHANNEL)->RSU->RemoteHost

   ***/
  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("TxPowerStart", DoubleValue (30));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (30));
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);

  /*** 3. Create and setup MAC ***/
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode), "ControlMode", StringValue (phyMode));
  NetDeviceContainer netDevices = wifi80211p.Install (wifiPhy, wifi80211pMac, wifiNodes);

  /*** 4. Add Internet layers stack and routing ***/
  InternetStackHelper internet;
  internet.Install (wifiNodes);

  /*** 5. Assign IP address to each device ***/
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer ipv4Interfaces;
  ipv4Interfaces = address.Assign (netDevices);

  /*** 4. Create and install mobility (SUMO will be attached later) ***/
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifiNodes);
  /* Set the RSU to a fixed position */
  Ptr<MobilityModel> mobilityRSU = wifiNodes.Get (0)->GetObject<MobilityModel> ();
  mobilityRSU->SetPosition (Vector (0, 0, 20.0)); // set RSU to fixed position

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

  /*** 6. Create and Setup application for the server ***/
  DENMSenderHelper DenmSenderHelper (9); // Port #9
  DenmSenderHelper.SetAttribute ("AggregateOutput", BooleanValue(aggregate_out));
  DenmSenderHelper.SetAttribute ("Client", (PointerValue) sumoClient); // pass TraciClient object for accessing sumo in application
  DenmSenderHelper.SetAttribute ("RealTime", BooleanValue(realtime));
  DenmSenderHelper.SetAttribute ("ASN", BooleanValue(asn));

  ApplicationContainer DENMSenderApp = DenmSenderHelper.Install (wifiNodes.Get (0));
  DENMSenderApp.Start (Seconds (0.0));
  DENMSenderApp.Stop (simulationTime - Seconds (0.1));
  ++nodeCounter;

  /*** 7. Setup interface and application for dynamic nodes ***/
  CAMSenderHelper CamSenderHelper (9);
  CamSenderHelper.SetAttribute ("Client", (PointerValue) sumoClient); // pass TraciClient object for accessing sumo in application

  /* Extract the server address */
  Ptr<Ipv4> ipv4 = wifiNodes.Get (0)->GetObject<Ipv4> ();
  Ipv4InterfaceAddress iaddr = ipv4->GetAddress (1, 0);
  Ipv4Address remoteHostAddr = iaddr.GetLocal ();

  /* callback function for node creation */
  std::function<Ptr<Node> ()> setupNewWifiNode = [&] () -> Ptr<Node>
    {
      if (nodeCounter >= wifiNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      /* don't create and install the protocol stack of the node at simulation time -> take from "node pool" */
      Ptr<Node> includedNode = wifiNodes.Get(nodeCounter);
      ++nodeCounter; //increment counter for next node

      /* Install Application */
      CamSenderHelper.SetAttribute ("Index", IntegerValue(nodeCounter));
      CamSenderHelper.SetAttribute ("SendCam", BooleanValue(send_cam));
      CamSenderHelper.SetAttribute ("RealTime", BooleanValue(realtime));
      CamSenderHelper.SetAttribute ("PrintSummary", BooleanValue(true));
      CamSenderHelper.SetAttribute ("ASN", BooleanValue(asn));
      CamSenderHelper.SetAttribute ("CAMIntertime", DoubleValue(cam_intertime));

      std::ostringstream oss;
      std::ostream &os = oss;
      remoteHostAddr.Print (os);
      std::string serv_addr = oss.str();
      CamSenderHelper.SetAttribute ("ServerAddr", StringValue(serv_addr));

      ApplicationContainer CAMSenderApp = CamSenderHelper.Install (includedNode);
      CAMSenderApp.Start (Seconds (0.0));
      CAMSenderApp.Stop (simulationTime - Simulator::Now () - Seconds (0.1));

      return includedNode;
    };

  /* callback function for node shutdown */
  std::function<void (Ptr<Node>)> shutdownWifiNode = [] (Ptr<Node> exNode)
    {
      /* stop all applications */
      Ptr<CAMSender> CAMSender_ = exNode->GetApplication(0)->GetObject<CAMSender>();
      if(CAMSender_)
        CAMSender_->StopApplicationNow();

       /* set position outside communication range */
      Ptr<ConstantPositionMobilityModel> mob = exNode->GetObject<ConstantPositionMobilityModel>();
      mob->SetPosition(Vector(-1000.0+(rand()%25),320.0+(rand()%25),250.0));// rand() for visualization purposes

      /* NOTE: further actions could be required for a save shut down! */
    };

  /* start traci client with given function pointers */
  sumoClient->SumoSetup (setupNewWifiNode, shutdownWifiNode);

  /*** 8. Start Simulation ***/
  Simulator::Stop (simulationTime);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

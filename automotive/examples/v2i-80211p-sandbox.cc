#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/automotive-module.h"
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
  bool aggregate_out = false;
  double sumo_updates = 0.01;
  bool send_cam = true;
  bool asn = false;
  std::string sumo_folder = "src/automotive/examples/sumo-files/";
  std::string mob_trace = "cars.rou.xml";
  std::string sumo_config ="src/automotive/examples/sumo-files/map.sumo.cfg";

  double simTime = 300;

  CommandLine cmd;
  cmd.AddValue ("sim-time","Simulation time",simTime);
  cmd.AddValue ("realtime", "Use the realtime scheduler or not", realtime);
  cmd.AddValue ("sumo-gui", "Use SUMO gui or not", sumo_gui);
  cmd.AddValue ("server-aggregate-output", "Print an aggregate output for server", aggregate_out);
  cmd.AddValue ("sumo-updates", "SUMO granularity", sumo_updates);
  cmd.AddValue ("send-cam", "Enable car to send cam", send_cam);
  cmd.AddValue ("asn", "Use ASN.1 or plain-text to send message", asn);

  cmd.Parse (argc, argv);
  if (verbose)
    {
      LogComponentEnable ("TraciClient", LOG_LEVEL_INFO);
      LogComponentEnable ("v2i-CAM-sender", LOG_LEVEL_INFO);
      LogComponentEnable ("v2i-DENM-sender", LOG_LEVEL_INFO);    }

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
  uint16_t numberOfNodes = std::stoi (num_client)+1;

  ns3::Time simulationTime (ns3::Seconds(simTime));

  /***
       the network topology created is the following:

       OBUs->(802.11p CHANNEL)->RSU
       ^CAM ->              <- DENM^

   ***/

  /*** 1. Create node pool and counter; large enough to cover all sumo vehicles ***/
  NodeContainer nodePool;
  nodePool.Create (numberOfNodes+1);
  uint32_t nodeCounter (0);

  /*** 2. Create and setup channel ***/
  std::string phyMode ("OfdmRate6MbpsBW10MHz");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.Set ("TxPowerStart", DoubleValue (20));
  wifiPhy.Set ("TxPowerEnd", DoubleValue (20));
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  Ptr<YansWifiChannel> channel = wifiChannel.Create ();
  wifiPhy.SetChannel (channel);

  /*** 3. Create and setup MAC ***/
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11);
  NqosWaveMacHelper wifi80211pMac = NqosWaveMacHelper::Default ();
  Wifi80211pHelper wifi80211p = Wifi80211pHelper::Default ();
  wifi80211p.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue (phyMode), "ControlMode", StringValue (phyMode));
  NetDeviceContainer netDevices = wifi80211p.Install (wifiPhy, wifi80211pMac, nodePool);

  /*** 4. Add Internet layers stack and routing ***/
  InternetStackHelper stack;
  stack.Install (nodePool);

  /*** 5. Assign IP address to each device ***/
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer ipv4Interfaces;
  ipv4Interfaces = address.Assign (netDevices);

  /*** 6. Setup Mobility and position node pool ***/
  MobilityHelper mobility;
  Ptr<UniformDiscPositionAllocator> positionAlloc = CreateObject<UniformDiscPositionAllocator> ();
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodePool);

  /*** 7. Setup Traci and start SUMO ***/
  Ptr<TraciClient> sumoClient = CreateObject<TraciClient> ();
  sumoClient->SetAttribute ("SumoConfigPath", StringValue (sumo_config));
  sumoClient->SetAttribute ("SumoBinaryPath", StringValue (""));    // use system installation of sumo
  sumoClient->SetAttribute ("SynchInterval", TimeValue (Seconds (sumo_updates)));
  sumoClient->SetAttribute ("StartTime", TimeValue (Seconds (0.0)));
  sumoClient->SetAttribute ("SumoGUI", (BooleanValue) sumo_gui);
  sumoClient->SetAttribute ("SumoPort", UintegerValue (3400));
  sumoClient->SetAttribute ("PenetrationRate", DoubleValue (1.0));  // portion of vehicles equipped with wifi
  sumoClient->SetAttribute ("SumoLogFile", BooleanValue (true));
  sumoClient->SetAttribute ("SumoStepLog", BooleanValue (false));
  sumoClient->SetAttribute ("SumoSeed", IntegerValue (10));
  sumoClient->SetAttribute ("SumoAdditionalCmdOptions", StringValue ("--verbose true"));
  sumoClient->SetAttribute ("SumoWaitForSocket", TimeValue (Seconds (1.0)));

  /*** 8. Create and Setup Applications for the RSU node and set position ***/
  DENMSenderHelper DenmSenderHelper (9); // Port #9
  DenmSenderHelper.SetAttribute ("AggregateOutput", BooleanValue(aggregate_out));
  DenmSenderHelper.SetAttribute ("Client", (PointerValue) sumoClient); // pass TraciClient object for accessing sumo in application
  DenmSenderHelper.SetAttribute ("RealTime", BooleanValue(realtime));
  DenmSenderHelper.SetAttribute ("ASN", BooleanValue(asn));

  ApplicationContainer DENMSenderApp = DenmSenderHelper.Install (nodePool.Get (0));
  DENMSenderApp.Start (Seconds (0.0));
  DENMSenderApp.Stop (simulationTime - Seconds (0.1));

  Ptr<MobilityModel> mobilityRsuNode = nodePool.Get (0)->GetObject<MobilityModel> ();

  mobilityRsuNode->SetPosition (Vector (0, 0, 3.0)); // set RSU to fixed position
  nodeCounter++;    // one node (RSU) consumed from "node pool"

  /*** 9. Setup interface and application for dynamic nodes ***/
  CAMSenderHelper CamSenderHelper (9);
  CamSenderHelper.SetAttribute ("Client", (PointerValue) sumoClient); // pass TraciClient object for accessing sumo in application

  // callback function for node creation
  std::function<Ptr<Node> ()> setupNewWifiNode = [&] () -> Ptr<Node>
    {
      if (nodeCounter >= nodePool.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      // don't create and install the protocol stack of the node at simulation time -> take from "node pool"
      Ptr<Node> includedNode = nodePool.Get(nodeCounter);
      nodeCounter++;// increment counter for next node

      // Install Application
      CamSenderHelper.SetAttribute ("Index", IntegerValue(nodeCounter));
      CamSenderHelper.SetAttribute ("SendCam", BooleanValue(send_cam));
      CamSenderHelper.SetAttribute ("RealTime", BooleanValue(realtime));
      CamSenderHelper.SetAttribute ("PrintSummary", BooleanValue(false));
      CamSenderHelper.SetAttribute ("ASN", BooleanValue(asn));

//      std::ostringstream oss;
//      std::ostream &os = oss;
//      remoteHostAddr.Print (os);
//      std::string serv_addr = oss.str();
//      CamSenderHelper.SetAttribute ("ServerAddr", StringValue(serv_addr));

      ApplicationContainer CAMSenderApp = CamSenderHelper.Install (includedNode);
      CAMSenderApp.Start (Seconds (0.0));
      CAMSenderApp.Stop (simulationTime - Simulator::Now () - Seconds (0.1));

      return includedNode;
    };

  // callback function for node shutdown
  std::function<void (Ptr<Node>)> shutdownWifiNode = [] (Ptr<Node> exNode)
    {
      // stop all applications
      Ptr<CAMSender> CAMSender_ = exNode->GetApplication(0)->GetObject<CAMSender>();
      if(CAMSender_)
        CAMSender_->StopApplicationNow();

       // set position outside communication range
      Ptr<ConstantPositionMobilityModel> mob = exNode->GetObject<ConstantPositionMobilityModel>();
      mob->SetPosition(Vector(-1000.0+(rand()%25),320.0+(rand()%25),250.0));// rand() for visualization purposes

      // NOTE: further actions could be required for a save shut down!
    };

  // start traci client with given function pointers
  sumoClient->SumoSetup (setupNewWifiNode, shutdownWifiNode);

  /*** 10. Setup and Start Simulation + Animation ***/
  //AnimationInterface anim ("src/automotive/examples/ns3-sumo-sim.xml"); // Mandatory
  Simulator::Stop (simulationTime);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

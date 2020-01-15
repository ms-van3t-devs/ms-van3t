#include "ns3/automotive-module.h"
#include "ns3/traci-module.h"
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/phy-stats-calculator.h"
#include <functional>
#include <stdlib.h>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("v2i-lte-sandbox");

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

  /*** 0.b LENA Options ***/
  uint16_t numberOfNodes;
  double interPacketInterval = 100;
  bool useCa = false;
  uint32_t nodeCounter = 0;

  double simTime = 300;

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

  /* Cmd Line option for Lena */
  cmd.AddValue("interPacketInterval", "Inter packet interval [ms])", interPacketInterval);
  cmd.AddValue("useCa", "Whether to use carrier aggregation.", useCa);

  cmd.AddValue("sim-time", "Total duration of the simulation [s])", simTime);

  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("TraciClient", LOG_LEVEL_INFO);
      LogComponentEnable ("v2i-CAM-sender", LOG_LEVEL_INFO);
      LogComponentEnable ("v2i-DENM-sender", LOG_LEVEL_INFO);
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

  /*** 0.c Read from the mob_trace the number of vehicles that will be created.
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
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("10.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
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
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);
  /* Set the eNB to a fixed position */
  Ptr<MobilityModel> mobilityeNBn = enbNodes.Get (0)->GetObject<MobilityModel> ();
  mobilityeNBn->SetPosition (Vector (0, 0, 20.0)); // set eNB to fixed position

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
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  for (uint16_t i = 0; i < numberOfNodes; i++)
      {
        lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(0));
        // side effect: the default EPS bearer will be activated
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

  /*** 7. Create and Setup application for the server ***/
  DENMSenderHelper DenmSenderHelper (9); // Port #9
  DenmSenderHelper.SetAttribute ("AggregateOutput", BooleanValue(aggregate_out));
  DenmSenderHelper.SetAttribute ("Client", (PointerValue) sumoClient); // pass TraciClient object for accessing sumo in application
  DenmSenderHelper.SetAttribute ("RealTime", BooleanValue(realtime));
  DenmSenderHelper.SetAttribute ("ASN", BooleanValue(asn));

  ApplicationContainer DENMSenderApp = DenmSenderHelper.Install (remoteHostContainer.Get (0));
  DENMSenderApp.Start (Seconds (0.0));
  DENMSenderApp.Stop (simulationTime - Seconds (0.1));
  ++nodeCounter;

  /*** 8. Setup interface and application for dynamic nodes ***/
  CAMSenderHelper CamSenderHelper (9);
  CamSenderHelper.SetAttribute ("Client", (PointerValue) sumoClient); // pass TraciClient object for accessing sumo in application

  // callback function for node creation
  std::function<Ptr<Node> ()> setupNewWifiNode = [&] () -> Ptr<Node>
    {
      if (nodeCounter >= ueNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      // don't create and install the protocol stack of the node at simulation time -> take from "node pool"
      Ptr<Node> includedNode = ueNodes.Get(nodeCounter);
      ++nodeCounter;// increment counter for next node

      // Install Application
      CamSenderHelper.SetAttribute ("Index", IntegerValue(nodeCounter));
      CamSenderHelper.SetAttribute ("SendCam", BooleanValue(send_cam));
      CamSenderHelper.SetAttribute ("RealTime", BooleanValue(realtime));
      CamSenderHelper.SetAttribute ("PrintSummary", BooleanValue(false));
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

  /* To enable statistics collection of LTE module */
  //lteHelper->EnableTraces ();

  /*** 9. Start Simulation ***/
  Simulator::Stop (simulationTime);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

#include "ns3/automotive-module.h"
#include "ns3/traci-module.h"
#include "ns3/cv2x_lte-v2x-helper.h"
#include "ns3/config-store.h"
#include "ns3/internet-module.h"
#include "ns3/cv2x-module.h"
#include "ns3/sumo_xml_parser.h"
#include <ns3/node-list.h>

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("v2v-cv2x");

int
main (int argc, char *argv[])
{
  double baseline= 320.0;                  // Baseline distance in meter (150m for urban, 320m for freeway)

  std::string sumo_folder = "src/automotive/examples/sumo_files_v2v_map/";
  std::string mob_trace = "cars.rou.xml";
  std::string sumo_config ="src/automotive/examples/sumo_files_v2v_map/map.sumo.cfg";

  /*** 0.a App Options ***/
  bool verbose = true;
  bool realtime = false;
  bool sumo_gui = true;
  double sumo_updates = 0.01;
  bool send_cam = true;
  bool send_denm = true;
  std::string csv_name;

  /*** 0.b LENA + V2X Options ***/
  double ueTxPower = 23.0;                // Transmission power in dBm
  double probResourceKeep = 0.0;          // Probability to select the previous resource again [0.0-0.8]
  uint32_t mcs = 20;                      // Modulation and Coding Scheme
  bool harqEnabled = false;               // Retransmission enabled (harq not available yet)
  bool adjacencyPscchPssch = true;        // Subchannelization scheme
  bool partialSensing = false;            // Partial sensing enabled (actual only partialSensing is false supported)
  uint16_t sizeSubchannel = 10;           // Number of RBs per subchannel
  uint16_t numSubchannel = 3;             // Number of subchannels per subframe
  uint16_t startRbSubchannel = 0;         // Index of first RB corresponding to subchannelization
  uint16_t pRsvp = 20;                    // Resource reservation interval
  uint16_t t1 = 4;                        // T1 value of selection window
  uint16_t t2 = 100;                      // T2 value of selection window
  uint16_t slBandwidth;                   // Sidelink bandwidth

  double simTime = 100;

  uint16_t numberOfNodes;
  uint32_t nodeCounter = 0;

  xmlDocPtr rou_xml_file;

  CommandLine cmd;

  /* Cmd Line option for application */
  cmd.AddValue ("realtime", "Use the realtime scheduler or not", realtime);
  cmd.AddValue ("sumo-gui", "Use SUMO gui or not", sumo_gui);
  cmd.AddValue ("sumo-updates", "SUMO granularity", sumo_updates);
  cmd.AddValue ("send-cam", "Enable car to send cam", send_cam);
  cmd.AddValue ("send-denm", "Enable car to send cam", send_denm);
  cmd.AddValue ("sumo-folder","Position of sumo config files",sumo_folder);
  cmd.AddValue ("mob-trace", "Name of the mobility trace file", mob_trace);
  cmd.AddValue ("sumo-config", "Location and name of SUMO configuration file", sumo_config);
  cmd.AddValue ("csv-log", "Name of the CSV log file", csv_name);

  /* Cmd Line option for v2x */
  cmd.AddValue ("tx-power", "UEs transmission power [dBm]", ueTxPower);
  cmd.AddValue ("adjacencyPscchPssch", "Scheme for subchannelization", adjacencyPscchPssch);
  cmd.AddValue ("sizeSubchannel", "Number of RBs per Subchannel", sizeSubchannel);
  cmd.AddValue ("numSubchannel", "Number of Subchannels", numSubchannel);
  cmd.AddValue ("startRbSubchannel", "Index of first subchannel index", startRbSubchannel);
  cmd.AddValue ("T1", "T1 Value of Selection Window", t1);
  cmd.AddValue ("T2", "T2 Value of Selection Window", t2);
  cmd.AddValue ("mcs", "Modulation and Coding Scheme", mcs);
  cmd.AddValue ("pRsvp", "Resource Reservation Interval", pRsvp);
  cmd.AddValue ("probResourceKeep", "Probability for selecting previous resource again", probResourceKeep);
  cmd.AddValue ("baseline", "Distance in which messages are transmitted and must be received", baseline);

  cmd.AddValue("sim-time", "Total duration of the simulation [s])", simTime);

  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("v2v-cv2x", LOG_LEVEL_INFO);
      LogComponentEnable ("CABasicService", LOG_LEVEL_INFO);
      LogComponentEnable ("DENBasicService", LOG_LEVEL_INFO);
    }

  NS_LOG_INFO("Configuring C-V2X channel...");
  /*** 0.c V2X Configurations ***/
  /* Set the UEs power in dBm */
  Config::SetDefault ("ns3::cv2x_LteUePhy::TxPower", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::cv2x_LteUePhy::RsrpUeMeasThreshold", DoubleValue (-10.0));
  /* Enable V2X communication on PHY layer */
  Config::SetDefault ("ns3::cv2x_LteUePhy::EnableV2x", BooleanValue (true));

  /* Set power */
  Config::SetDefault ("ns3::cv2x_LteUePowerControl::Pcmax", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::cv2x_LteUePowerControl::PsschTxPower", DoubleValue (ueTxPower));
  Config::SetDefault ("ns3::cv2x_LteUePowerControl::PscchTxPower", DoubleValue (ueTxPower));

  if (adjacencyPscchPssch)
  {
      slBandwidth = sizeSubchannel * numSubchannel;
  }
  else
  {
      slBandwidth = (sizeSubchannel+2) * numSubchannel;
  }

  /* Configure for UE selected */
  Config::SetDefault ("ns3::cv2x_LteUeMac::UlBandwidth", UintegerValue (slBandwidth));
  Config::SetDefault ("ns3::cv2x_LteUeMac::EnableV2xHarq", BooleanValue (harqEnabled));
  Config::SetDefault ("ns3::cv2x_LteUeMac::EnableAdjacencyPscchPssch", BooleanValue (adjacencyPscchPssch));
  Config::SetDefault ("ns3::cv2x_LteUeMac::EnablePartialSensing", BooleanValue (partialSensing));
  Config::SetDefault ("ns3::cv2x_LteUeMac::SlGrantMcs", UintegerValue (mcs));
  Config::SetDefault ("ns3::cv2x_LteUeMac::SlSubchannelSize", UintegerValue (sizeSubchannel));
  Config::SetDefault ("ns3::cv2x_LteUeMac::SlSubchannelNum", UintegerValue (numSubchannel));
  Config::SetDefault ("ns3::cv2x_LteUeMac::SlStartRbSubchannel", UintegerValue (startRbSubchannel));
  Config::SetDefault ("ns3::cv2x_LteUeMac::SlPrsvp", UintegerValue (pRsvp));
  Config::SetDefault ("ns3::cv2x_LteUeMac::SlProbResourceKeep", DoubleValue (probResourceKeep));
  Config::SetDefault ("ns3::cv2x_LteUeMac::SelectionWindowT1", UintegerValue (t1));
  Config::SetDefault ("ns3::cv2x_LteUeMac::SelectionWindowT2", UintegerValue (t2));

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  /* Use the realtime scheduler of ns3 */
  if(realtime)
      GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

  /*** 0.d Read from the mob_trace the number of vehicles that will be created.
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

  /*** 1. Create LTE objects   ***/
  Ptr<cv2x_PointToPointEpcHelper>  epcHelper = CreateObject<cv2x_PointToPointEpcHelper> ();
  Ptr<cv2x_LteHelper> lteHelper = CreateObject<cv2x_LteHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  // Disable eNBs for out-of-coverage modelling
  lteHelper->DisableNewEnbPhy();

  /* V2X */
  Ptr<cv2x_LteV2xHelper> lteV2xHelper = CreateObject<cv2x_LteV2xHelper> ();
  lteV2xHelper->SetLteHelper (lteHelper);

  /* Configure eNBs' antenna parameters before deploying them. */
  lteHelper->SetEnbAntennaModelType ("ns3::cv2x_NistParabolic3dAntennaModel");
  lteHelper->SetAttribute ("UseSameUlDlPropagationCondition", BooleanValue(true));
  Config::SetDefault ("ns3::cv2x_LteEnbNetDevice::UlEarfcn", StringValue ("54990")); // EARFCN 54990 -> 5855-5890-5925 MHz
  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::cv2x_CniUrbanmicrocellPropagationLossModel"));
  NS_LOG_INFO("Antenna parameters set. Current EARFCN: 54990, current frequency: 5.89 GHz");

  /*** 2. Create Internet and ipv4 helpers ***/
  InternetStackHelper internet;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  /*** 3. Create containers for UEs and eNB ***/
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create(1);
  ueNodes.Create(numberOfNodes);

  /*** 4. Create and install mobility (SUMO will be attached later) ***/
  MobilityHelper mobility;
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);
  /* Set the eNB to a fixed position */
  Ptr<MobilityModel> mobilityeNBn = enbNodes.Get (0)->GetObject<MobilityModel> ();
  mobilityeNBn->SetPosition (Vector (0, 0, 20.0)); // set eNB to fixed position - it is still disabled

  /*** 5. Install LTE Devices to the nodes + assign IP to UEs + manage buildings ***/
  lteHelper->InstallEnbDevice (enbNodes); // If you don't do it, the simulation crashes

  /* Required to use NIST 3GPP model */
  BuildingsHelper::Install (ueNodes);
  BuildingsHelper::Install (enbNodes);
  // BuildingsHelper::MakeMobilityModelConsistent (); Removed because DEPRECATED from 3.31
  for (NodeList::Iterator nit = NodeList::Begin (); nit != NodeList::End (); ++nit)
    {
      Ptr<MobilityModel> mm = (*nit)->GetObject<MobilityModel> ();
      if (mm != 0)
        {
          Ptr<MobilityBuildingInfo> bmm = mm->GetObject<MobilityBuildingInfo> ();
          NS_ABORT_MSG_UNLESS (0 != bmm, "node " << (*nit)->GetId () << " has a MobilityModel that does not have a MobilityBuildingInfo");
          bmm->MakeConsistent (mm);
        }
    }

  lteHelper->SetAttribute("UseSidelink", BooleanValue (true));
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  /* Install the IP stack on the UEs */
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;

  /* Assign IP address to UEs */
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      /* Set the default gateway for the UE */
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      NS_LOG_INFO("Node "<< ueNode->GetId () << " has been assigned an IP address: " << ueNode->GetObject<Ipv4> ()->GetAddress(1,0).GetLocal());
    }

  NS_LOG_INFO("Configuring sidelink...");

  /* Create sidelink groups */
  std::vector<NetDeviceContainer> txGroups;
  txGroups = lteV2xHelper->AssociateForV2xBroadcast(ueLteDevs, numberOfNodes);

  /* Compute average number of receivers associated per transmitter and vice versa */
  std::map<uint32_t, uint32_t> txPerUeMap;
  std::map<uint32_t, uint32_t> groupsPerUe;
  std::vector<NetDeviceContainer>::iterator gIt;
  for(gIt=txGroups.begin(); gIt != txGroups.end(); gIt++)
      {
          uint32_t numDevs = gIt->GetN();

          uint32_t nId;

          for(uint32_t i=1; i< numDevs; i++)
              {
                  nId = gIt->Get(i)->GetNode()->GetId();
                  txPerUeMap[nId]++;
              }
      }

  std::map<uint32_t, uint32_t>::iterator mIt;
  for(mIt=txPerUeMap.begin(); mIt != txPerUeMap.end(); mIt++)
      {
          groupsPerUe [mIt->second]++;
      }

  std::vector<uint32_t> groupL2Addresses;
  uint32_t groupL2Address = 0x00;
  std::vector<Ipv4Address> ipAddresses;
  Ipv4AddressGenerator::Init(Ipv4Address ("225.0.0.0"), Ipv4Mask ("255.0.0.0"));
  Ipv4Address clientRespondersAddress = Ipv4AddressGenerator::NextAddress (Ipv4Mask ("255.0.0.0"));
  NetDeviceContainer activeTxUes;

  for(gIt=txGroups.begin(); gIt != txGroups.end(); gIt++)
      {
          /* Create Sidelink bearers */
          NetDeviceContainer txUe ((*gIt).Get(0));
          activeTxUes.Add(txUe);
          NetDeviceContainer rxUes = lteV2xHelper->RemoveNetDevice ((*gIt), txUe.Get (0));
          Ptr<cv2x_LteSlTft> tft = Create<cv2x_LteSlTft> (cv2x_LteSlTft::TRANSMIT, clientRespondersAddress, groupL2Address);
          lteV2xHelper->ActivateSidelinkBearer (Seconds(0.0), txUe, tft);
          tft = Create<cv2x_LteSlTft> (cv2x_LteSlTft::RECEIVE, clientRespondersAddress, groupL2Address);
          lteV2xHelper->ActivateSidelinkBearer (Seconds(0.0), rxUes, tft);

          /* store and increment addresses */
          groupL2Addresses.push_back (groupL2Address);
          ipAddresses.push_back (clientRespondersAddress);
          groupL2Address++;
          clientRespondersAddress = Ipv4AddressGenerator::NextAddress (Ipv4Mask ("255.0.0.0"));
      }

  /* Creating sidelink configuration */
  Ptr<cv2x_LteUeRrcSl> ueSidelinkConfiguration = CreateObject<cv2x_LteUeRrcSl>();
  ueSidelinkConfiguration->SetSlEnabled(true);
  ueSidelinkConfiguration->SetV2xEnabled(true);

  cv2x_LteRrcSap::SlV2xPreconfiguration preconfiguration;
  preconfiguration.v2xPreconfigFreqList.freq[0].v2xCommPreconfigGeneral.carrierFreq = 54890;
  preconfiguration.v2xPreconfigFreqList.freq[0].v2xCommPreconfigGeneral.slBandwidth = slBandwidth;

  preconfiguration.v2xPreconfigFreqList.freq[0].v2xCommTxPoolList.nbPools = 1;
  preconfiguration.v2xPreconfigFreqList.freq[0].v2xCommRxPoolList.nbPools = 1;

  cv2x_SlV2xPreconfigPoolFactory pFactory;
  pFactory.SetHaveUeSelectedResourceConfig (true);
  pFactory.SetSlSubframe (std::bitset<20> (0xFFFFF));
  pFactory.SetAdjacencyPscchPssch (adjacencyPscchPssch);
  pFactory.SetSizeSubchannel (sizeSubchannel);
  pFactory.SetNumSubchannel (numSubchannel);
  pFactory.SetStartRbSubchannel (startRbSubchannel);
  pFactory.SetStartRbPscchPool (0);
  pFactory.SetDataTxP0 (-4);
  pFactory.SetDataTxAlpha (0.9);

  preconfiguration.v2xPreconfigFreqList.freq[0].v2xCommTxPoolList.pools[0] = pFactory.CreatePool ();
  preconfiguration.v2xPreconfigFreqList.freq[0].v2xCommRxPoolList.pools[0] = pFactory.CreatePool ();
  ueSidelinkConfiguration->SetSlV2xPreconfiguration (preconfiguration);

  lteHelper->InstallSidelinkV2xConfiguration (ueLteDevs, ueSidelinkConfiguration);

  /*** 6. Setup Traci and start SUMO ***/
  Ptr<TraciClient> sumoClient = CreateObject<TraciClient> ();
  sumoClient->SetAttribute ("SumoConfigPath", StringValue (sumo_config));
  sumoClient->SetAttribute ("SumoBinaryPath", StringValue (""));    // use system installation of sumo
  sumoClient->SetAttribute ("SynchInterval", TimeValue (Seconds (sumo_updates)));
  sumoClient->SetAttribute ("StartTime", TimeValue (Seconds (0.0)));
  sumoClient->SetAttribute ("SumoGUI", BooleanValue (sumo_gui));
  sumoClient->SetAttribute ("SumoPort", UintegerValue (3400));
  sumoClient->SetAttribute ("PenetrationRate", DoubleValue (1.0));
  sumoClient->SetAttribute ("SumoLogFile", BooleanValue (false));
  sumoClient->SetAttribute ("SumoStepLog", BooleanValue (false));
  sumoClient->SetAttribute ("SumoSeed", IntegerValue (10));
  sumoClient->SetAttribute ("SumoAdditionalCmdOptions", StringValue ("--verbose true"));
  sumoClient->SetAttribute ("SumoWaitForSocket", TimeValue (Seconds (1.0)));

  /*** 7. Setup interface and application for dynamic nodes ***/
  appSampleHelper AppSampleHelper;
  AppSampleHelper.SetAttribute ("Client", PointerValue (sumoClient));
  AppSampleHelper.SetAttribute ("RealTime", BooleanValue(realtime));
  AppSampleHelper.SetAttribute ("SendDenm", BooleanValue (send_denm));
  AppSampleHelper.SetAttribute ("SendCam", BooleanValue (send_cam));
  AppSampleHelper.SetAttribute ("PrintSummary", BooleanValue (true));
  AppSampleHelper.SetAttribute ("CSV", StringValue(csv_name));
  AppSampleHelper.SetAttribute ("Model", StringValue ("cv2x"));

  /* callback function for node creation */
  int i=0;
  std::function<Ptr<Node> ()> setupNewWifiNode = [&] () -> Ptr<Node>
    {
      if (nodeCounter >= ueNodes.GetN())
        NS_FATAL_ERROR("Node Pool empty!: " << nodeCounter << " nodes created.");

      Ptr<Node> includedNode = ueNodes.Get(nodeCounter);
      ++nodeCounter; // increment counter for next node

      /* Install Application */
      AppSampleHelper.SetAttribute ("IpAddr", Ipv4AddressValue(ipAddresses[i]));
      i++;

      //ApplicationContainer CAMSenderApp = CamSenderHelper.Install (includedNode);
      ApplicationContainer AppSample = AppSampleHelper.Install (includedNode);

      AppSample.Start (Seconds (0.0));
      AppSample.Stop (simulationTime - Simulator::Now () - Seconds (0.1));

      return includedNode;
    };

  /* callback function for node shutdown */
  std::function<void (Ptr<Node>)> shutdownWifiNode = [] (Ptr<Node> exNode)
    {
      /* stop all applications */
      Ptr<appSample> appSample_ = exNode->GetApplication(0)->GetObject<appSample>();

      if(appSample_)
        appSample_->StopApplicationNow();

       /* set position outside communication range */
      Ptr<ConstantPositionMobilityModel> mob = exNode->GetObject<ConstantPositionMobilityModel>();
      mob->SetPosition(Vector(-1000.0+(rand()%25),320.0+(rand()%25),250.0)); // rand() for visualization purposes

      /* NOTE: further actions could be required for a safe shut down! */
    };

  /* start traci client with given function pointers */
  sumoClient->SumoSetup (setupNewWifiNode, shutdownWifiNode);

  /*** 8. Start Simulation ***/
  Simulator::Stop (simulationTime);

  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

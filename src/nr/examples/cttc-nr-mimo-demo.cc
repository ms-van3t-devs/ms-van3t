/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
 * \ingroup examples
 * \file cttc-nr-mimo-demo.cc
 * \brief A cozy, simple, NR MIMO demo (in a tutorial style)
 *
 * This example describes how to setup a MIMO simulation using the 3GPP channel model
 * from TR 38.900. This example consists of a simple topology, in which there
 * is only one gNB and one UE. Have a look at the possible parameters
 * to know what you can configure through the command line.
 *
 * With the default configuration, the example will create one DL flow that will
 * go through only one BWP.
 *
 * The example will print on-screen the end-to-end result of the flow,
 * as well as writing them in a file.
 *
 * \code{.unparsed}
$ ./ns3 run "cttc-nr-mimo-demo --PrintHelp"
    \endcode
 *
 */

/*
 * Include part. Often, you will have to include the headers for an entire module;
 * do that by including the name of the module you need with the suffix "-module.h".
 */

#include "ns3/core-module.h"
#include "ns3/config-store-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/nr-module.h"
#include "ns3/antenna-module.h"

/*
 * Use, always, the namespace ns3. All the NR classes are inside such namespace.
 */
using namespace ns3;

/*
 * With this line, we will be able to see the logs of the file by enabling the
 * component "CttcNrMimoDemo"
 */
NS_LOG_COMPONENT_DEFINE ("CttcNrMimoDemo");

int
main (int argc, char *argv[])
{
  /*
   * Variables that represent the parameters we will accept as input by the
   * command line. Each of them is initialized with a default value, and
   * possibly overridden below when command-line arguments are parsed.
   */
  // Scenario parameters (that we will use inside this script):
  bool logging = false;

  //Whether gNB and UE antenna arrays support
  bool crossPolarizedGnb = true;
  bool crossPolarizedUe = true;

  //Number of rows and columns of antennas of Gnb and Ue
  uint16_t numRowsGnb = 2;
  uint16_t numColumnsGnb = 2;
  uint16_t numRowsUe = 1;
  uint16_t numColumnsUe = 1;

  // Traffic parameters (that we will use inside this script):
  uint32_t udpPacketSize = 1000;
  // For 2x2 MIMO and NR MCS table 2, packet interval is 40000 ns to
  // reach 200 mb/s
  Time packetInterval = NanoSeconds (40000);

  //distance between the gNB and the UE
  uint16_t gnbUeDistance = 20; //meters

  // Simulation parameters. Please don't use double to indicate seconds; use
  // ns-3 Time values which use integers to avoid portability issues.
  Time simTime = MilliSeconds (1000);
  Time udpAppStartTime = MilliSeconds (400);

  // NR parameters. We will take the input from the command line, and then we
  // will pass them inside the NR module.
  uint16_t numerology = 0;
  double centralFrequency = 3.5e9;
  double bandwidth = 20e6;
  double gnbTxPower = 30; //dBm
  double ueTxPower = 23; //dBm
  uint16_t fixedRankIndicator = 2;
  bool useFixedRi = true;
  double ro = 0;
  uint16_t updatePeriodMs = 100;
  uint16_t mcsTable = 2;

  // The polarization slant angle of first and second subarray in degrees
  double polSlantAngle1 = 0.0;
  double polSlantAngle2 = 90.0;

  // whether the cross polarization correlation is parameterized, when set to false correlation is calculated as per 3gpp channel model
  bool parametrizedCorrelation = true;

  // Where we will store the output files.
  std::string simTag = "default";
  std::string outputDir = "./";

  /*
   * From here, we instruct the ns3::CommandLine class of all the input parameters
   * that we may accept as input, as well as their description, and the storage
   * variable.
   */
  CommandLine cmd (__FILE__);

  cmd.AddValue ("logging",
                "Enable logging",
                logging);
  cmd.AddValue ("crossPolarizedGnb",
                "Whether the gNB antenna array has the cross polarized antenna "
                "elements. If yes, gNB supports 2 streams, otherwise only 1 stream",
                crossPolarizedGnb);
  cmd.AddValue ("crossPolarizedUe",
                "Whether the UE antenna array has the cross polarized antenna "
                "elements. If yes, UE supports 2 streams, otherwise only 1 stream",
                crossPolarizedUe);
  cmd.AddValue ("numRowsGnb",
                "Number of antenna rows at the gNB",
                numRowsGnb);
  cmd.AddValue ("numRowsUe",
                "Number of antenna rows at the UE",
                numRowsUe);
  cmd.AddValue ("numColumnsGnb",
                "Number of antenna columns at the gNB",
                numColumnsGnb);
  cmd.AddValue ("numColumnsUe",
                "Number of antenna columns at the UE",
                numColumnsUe);
  cmd.AddValue ("ro",
                "The channel correlation parameter",
                 ro);
  cmd.AddValue ("packetSize",
                "packet size in bytes to be used by best effort traffic",
                udpPacketSize);
  cmd.AddValue ("packetInterval",
                "Inter packet interval for CBR traffic",
                packetInterval);
  cmd.AddValue ("simTime",
                "Simulation time",
                simTime);
  cmd.AddValue ("numerology",
                "The numerology to be used",
                numerology);
  cmd.AddValue ("centralFrequency",
                "The system frequency to be used in band 1",
                centralFrequency);
  cmd.AddValue ("bandwidth",
                "The system bandwidth to be used",
                bandwidth);
  cmd.AddValue ("gnbTxPower",
                "gNB TX power",
                gnbTxPower);
  cmd.AddValue ("ueTxPower",
                "UE TX power",
                ueTxPower);
  cmd.AddValue ("gnbUeDistance",
                "The distance between the gNB and the UE in the scenario",
                gnbUeDistance);
  cmd.AddValue ("fixedRankIndicator",
                "The rank indicator used by the UE",
                fixedRankIndicator);
  cmd.AddValue ("useFixedRi",
                "If true, UE will use a fixed configured RI value; otherwise, "
                "it will use an adaptive RI value based on the SINR of the streams",
                useFixedRi);
  cmd.AddValue ("updatePeriodMs",
                "Channel update period in ms. If set to 0 then the channel update will be disabled",
                updatePeriodMs);
  cmd.AddValue ("polSlantAngle1",
                "Polarization slant angle of the first antenna sub-array/partition in degrees",
                polSlantAngle1);
  cmd.AddValue ("polSlantAngle2",
                "Polarization slant angle of the second antenna sub-array/partition in degrees",
                polSlantAngle2);
  cmd.AddValue ("parametrizedCorrelation",
                "Whether the cross polarization correlation is parameterized or "
                "calculated according to 3gpp channel model. When set to true "
                "it is parameterized, otherwise it is per 3gpp channel model",
                parametrizedCorrelation);
  cmd.AddValue ("mcsTable",
                "The NR MCS table to be used",
                mcsTable);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                outputDir);

  // Parse the command line
  cmd.Parse (argc, argv);

  /*
   * Check if the frequency is in the allowed range.
   * If you need to add other checks, here is the best position to put them.
   */
  NS_ABORT_IF (centralFrequency > 100e9);

  /*
   * If the logging variable is set to true, enable the log of some components
   * through the code. The same effect can be obtained through the use
   * of the NS_LOG environment variable:
   *
   * export NS_LOG="UdpClient=level_info|prefix_time|prefix_func|prefix_node:UdpServer=..."
   *
   * Usually, the environment variable way is preferred, as it is more customizable,
   * and more expressive.
   */
  if (logging)
    {
      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
    }

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

  /*
   * Attributes of ThreeGppChannelModel still cannot be set in our way.
   */
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (updatePeriodMs)));
  Config::SetDefault ("ns3::ThreeGppChannelModelParam::Ro", DoubleValue (ro));
  Config::SetDefault ("ns3::ThreeGppChannelModelParam::ParametrizedCorrelation", BooleanValue (parametrizedCorrelation));
  Config::SetDefault ("ns3::ThreeGppSpectrumPropagationLossModel::ChannelModel", StringValue ("ns3::ThreeGppChannelModelParam"));

  /*
   * Create the scenario. In our examples, we heavily use helpers that setup
   * the gnbs and ue following a pre-defined pattern. Please have a look at the
   * GridScenarioHelper documentation to see how the nodes will be distributed.
   */
  int64_t randomStream = 1;

  /*
   * Assign mobility to the gNB and UEs.
   *  1. Set mobility model type.
   *  2. Store the positions in ListPositionAllocator for the gNB and UE
   *  3. Install mobility model
   */

  /*
   * Create NodeContainer for gNB and UE
   */
  NodeContainer gnbContainer;
  gnbContainer.Create (1);
  NodeContainer ueContainer;
  ueContainer.Create (1);

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator> positionAllocUe = CreateObject<ListPositionAllocator> ();
  positionAllocUe->Add (Vector (0.0, 0.0, 10.0));
  positionAllocUe->Add (Vector (gnbUeDistance, 0.0, 1.5));
  mobility.SetPositionAllocator (positionAllocUe);
  mobility.Install (gnbContainer);
  mobility.Install (ueContainer);

  /* The default topology is the following:
   *
   *         gNB..........(20 m)..........UE
   *   (0.0, 0.0, 10.0)               (20, 0.0, 1.5)
   */

  /*
   * Setup the NR module. We create the various helpers needed for the
   * NR simulation:
   * - EpcHelper, which will setup the core network
   * - IdealBeamformingHelper, which takes care of the beamforming part
   * - NrHelper, which takes care of creating and connecting the various
   * part of the NR stack
   */
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  // Put the pointers inside nrHelper
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  /*
   * Spectrum division. We create one operational band, containing
   * one component carrier, and a single bandwidth part
   * centered at the frequency specified by the input parameters.
   * We will use the StreetCanyon channel modeling.
   */
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;  // single CC

  // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
  // a single BWP per CC
  CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequency, bandwidth, numCcPerBand, BandwidthPartInfo::UMi_StreetCanyon);

  // By using the configuration created, it is time to make the operation bands
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

  /*
   * The configured spectrum division is:
   * ------------Band--------------
   * ------------CC1----------------
   * ------------BWP1---------------
   */

  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (updatePeriodMs)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  /*
   * Initialize channel and pathloss, plus other things inside band. If needed,
   * the band configuration can be done manually, but we leave it for more
   * sophisticated examples. For the moment, this method will take care
   * of all the spectrum initialization needs.
   */
  nrHelper->InitializeOperationBand (&band);

  /*
   * allBwps contains all the spectrum configuration needed by the nrHelper
   * to install a device.
   */
  BandwidthPartInfoPtrVector allBwps;
  allBwps = CcBwpCreator::GetAllBwps ({band});

  /*
   * allBwps contains all the spectrum configuration needed for the nrHelper.
   */

  Packet::EnableChecking ();
  Packet::EnablePrinting ();

  /*
   *  Attributes valid for all the nodes
   */
  // Error Model: gNB and UE with same spectrum error model.
  std::string errorModel = "ns3::NrEesmIrT" + std::to_string (mcsTable);
  nrHelper->SetDlErrorModel (errorModel);
  nrHelper->SetUlErrorModel (errorModel);

  // Both DL and UL AMC will have the same model behind.
  nrHelper->SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel));
  nrHelper->SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel));

  // Beamforming method
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  // Core latency
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (numRowsUe));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (numColumnsUe));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // UE rank indicator
  if (useFixedRi)
    {
      // it makes more sense to configure the rank indicator value
      // if useFixedRi is true.
      nrHelper->SetUePhyAttribute ("UseFixedRi", BooleanValue (useFixedRi));
      nrHelper->SetUePhyAttribute ("FixedRankIndicator", UintegerValue (fixedRankIndicator));
    }
  else
    {
      nrHelper->SetUePhyAttribute ("UseFixedRi", BooleanValue (useFixedRi));
    }

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (numRowsGnb));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (numColumnsGnb));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));

  uint32_t bwpId = 0;

  // gNb routing between Bearer and bandwidth part
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpId));

  // UE routing between Bearer and bandwidth part
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpId));


  /*
   * We miss many other parameters. By default, not configuring them is equivalent
   * to use the default values. Please, have a look at the documentation to see
   * what are the default values for all the attributes you are not seeing here.
   */

  /*
   * We have configured the attributes we needed. Now, install and get the pointers
   * to the NetDevices, which contains all the NR stack:
   */

  uint8_t subArraysGnb = 0, subArraysUe = 0;

  if (crossPolarizedGnb)
    {
      subArraysGnb = 2;
    }
  else
    {
      subArraysGnb = 1;
    }

  if (crossPolarizedUe)
    {
      subArraysUe = 2;
    }
  else
    {
      subArraysUe = 1;
    }

  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gnbContainer, allBwps, subArraysGnb);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueContainer, allBwps, subArraysUe);

  /*
   * Fix the random stream throughout the nr, propagation, and spectrum
   * modules classes. This configuration is extremely important for the
   * reproducibility of the results.
   */
  randomStream += nrHelper->AssignStreams (enbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);

  /*
   * Per Node attribute configuration. Get the node and change the attributes we
   * have to setup.
   */

  // Get the first netdevice (enbNetDev.Get (0)) and the first bandwidth part (0)
  // and set the attribute.
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerology));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue (gnbTxPower));
  double polarizationFirstSubArray = (polSlantAngle1 * M_PI) / 180.0; // converting to radians
  double polarizationSecondSubArray = (polSlantAngle2 * M_PI) / 180.0; // converting to radians
  ObjectVectorValue gnbSpectrumPhys;
  Ptr<NrSpectrumPhy> nrSpectrumPhy;
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->GetAttribute ("NrSpectrumPhyList", gnbSpectrumPhys);
  nrSpectrumPhy = gnbSpectrumPhys.Get (0)->GetObject <NrSpectrumPhy> ();
  nrSpectrumPhy->GetAntenna ()->GetObject<UniformPlanarArray> ()->SetAttribute ("PolSlantAngle", DoubleValue (polarizationFirstSubArray));
  if (gnbSpectrumPhys.GetN () == 2)
    {
      nrSpectrumPhy = gnbSpectrumPhys.Get (1)->GetObject <NrSpectrumPhy> ();
      nrSpectrumPhy->GetAntenna ()->GetObject<UniformPlanarArray> ()->SetAttribute ("PolSlantAngle", DoubleValue (polarizationSecondSubArray));
    }

  // for UE
  nrHelper->GetUePhy (ueNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue (ueTxPower));
  ObjectVectorValue ueSpectrumPhys;
  nrHelper->GetUePhy (ueNetDev.Get (0), 0)->GetAttribute ("NrSpectrumPhyList", ueSpectrumPhys);
  nrSpectrumPhy = ueSpectrumPhys.Get (0)->GetObject <NrSpectrumPhy> ();
  nrSpectrumPhy->GetAntenna ()->GetObject<UniformPlanarArray> ()->SetAttribute ("PolSlantAngle", DoubleValue (polarizationFirstSubArray));
  if (ueSpectrumPhys.GetN () == 2)
    {
      nrSpectrumPhy = ueSpectrumPhys.Get (1)->GetObject <NrSpectrumPhy> ();
      nrSpectrumPhy->GetAntenna ()->GetObject<UniformPlanarArray> ()->SetAttribute ("PolSlantAngle", DoubleValue (polarizationSecondSubArray));
    }


  // When all the configuration is done, explicitly call UpdateConfig ()

  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }


  // From here, it is standard NS3. In the future, we will create helpers
  // for this part as well.

  // create the Internet and install the IP stack on the UEs
  // get SGW/PGW and create a single RemoteHost
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // connect a remoteHost to pgw. Setup routing too
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueContainer);


  Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
  // Set the default gateway for the UE
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueContainer.Get (0)->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  // attach UE to the closest eNB
  nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  /*
   * Install DL traffic part.
   */

  uint16_t dlPort = 1234;

  ApplicationContainer serverApps;

  // The sink will always listen to the specified ports
  UdpServerHelper dlPacketSink (dlPort);

  // The server, that is the application which is listening, is installed in the UE
  serverApps.Add (dlPacketSink.Install (ueContainer));

  /*
   * Configure attributes for the CBR traffic generator, using user-provided
   * parameters
   */
  UdpClientHelper dlClient;
  dlClient.SetAttribute ("RemotePort", UintegerValue (dlPort));
  dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  dlClient.SetAttribute ("PacketSize", UintegerValue (udpPacketSize));
  dlClient.SetAttribute ("Interval", TimeValue (packetInterval));

  // The bearer that will carry the traffic
  EpsBearer epsBearer (EpsBearer::NGBR_LOW_LAT_EMBB);

  // The filter for the traffic
  Ptr<EpcTft> dlTft = Create<EpcTft> ();
  EpcTft::PacketFilter dlPktFilter;
  dlPktFilter.localPortStart = dlPort;
  dlPktFilter.localPortEnd = dlPort;
  dlTft->Add (dlPktFilter);

  /*
   * Let's install the applications!
   */
  ApplicationContainer clientApps;

  for (uint32_t i = 0; i < ueContainer.GetN (); ++i)
    {
      Ptr<Node> ue = ueContainer.Get (i);
      Ptr<NetDevice> ueDevice = ueNetDev.Get (i);
      Address ueAddress = ueIpIface.GetAddress (i);

      // The client, who is transmitting, is installed in the remote host,
      // with destination address set to the address of the UE
      dlClient.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
      clientApps.Add (dlClient.Install (remoteHost));

      // Activate a dedicated bearer for the traffic
      nrHelper->ActivateDedicatedEpsBearer (ueDevice, epsBearer, dlTft);
    }

  // start UDP server and client apps
  serverApps.Start (udpAppStartTime);
  clientApps.Start (udpAppStartTime);
  serverApps.Stop (simTime);
  clientApps.Stop (simTime);

  // enable the traces provided by the nr module
  nrHelper->EnableTraces ();


  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (ueContainer);

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  Simulator::Stop (simTime);
  Simulator::Run ();

  /*
   * To check what was installed in the memory, i.e., BWPs of eNb Device, and its configuration.
   * Example is: Node 1 -> Device 0 -> BandwidthPartMap -> {0,1} BWPs -> NrGnbPhy -> Numerology,
  GtkConfigStore config;
  config.ConfigureAttributes ();
  */

  // Print per-flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  double averageFlowThroughput = 0.0;
  double averageFlowDelay = 0.0;

  std::ofstream outFile;
  std::string filename = outputDir + "/" + simTag;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  if (!outFile.is_open ())
    {
      std::cerr << "Can't open file " << filename << std::endl;
      return 1;
    }

  outFile.setf (std::ios_base::fixed);

  double flowDuration = (simTime - udpAppStartTime).GetSeconds ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::stringstream protoStream;
      protoStream << (uint16_t) t.protocol;
      if (t.protocol == 6)
        {
          protoStream.str ("TCP");
        }
      if (t.protocol == 17)
        {
          protoStream.str ("UDP");
        }
      outFile << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> " << t.destinationAddress << ":" << t.destinationPort << ") proto " << protoStream.str () << "\n";
      outFile << "  Tx Packets: " << i->second.txPackets << "\n";
      outFile << "  Tx Bytes:   " << i->second.txBytes << "\n";
      outFile << "  TxOffered:  " << i->second.txBytes * 8.0 / flowDuration / 1000.0 / 1000.0  << " Mbps\n";
      outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      if (i->second.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          averageFlowThroughput += i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000;
          averageFlowDelay += 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;

          outFile << "  Throughput: " << i->second.rxBytes * 8.0 / flowDuration / 1000 / 1000  << " Mbps\n";
          outFile << "  Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
          //outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << " Mbps \n";
          outFile << "  Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms\n";
        }
      else
        {
          outFile << "  Throughput:  0 Mbps\n";
          outFile << "  Mean delay:  0 ms\n";
          outFile << "  Mean jitter: 0 ms\n";
        }
      outFile << "  Rx Packets: " << i->second.rxPackets << "\n";
    }

  outFile << "\n\n  Mean flow throughput: " << averageFlowThroughput / stats.size () << "\n";
  outFile << "  Mean flow delay: " << averageFlowDelay / stats.size () << "\n";

  outFile.close ();

  std::ifstream f (filename.c_str ());

  if (f.is_open ())
    {
      std::cout << f.rdbuf ();
    }

  Simulator::Destroy ();
  return 0;
}



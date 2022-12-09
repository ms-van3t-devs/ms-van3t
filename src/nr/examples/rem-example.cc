/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * \file rem-example.cc
 * \ingroup examples
 * \brief REM Creation Example
 *
 * This example describes how to setup a simulation using NrRadioEnvironmentMapHelper.
 *
 * We provide a number of simulation parameters that can be configured in the
 * command line, such as the number of UEs per cell or the number of rows and
 * columns of the gNB and Ue antennas.
 * Please have a look at the possible parameters to know what you can configure
 * through the command line.
 * The user can also specify the type of REM map (BeamShape/CoverageArea/UeCoverage)
 * he wishes to generate with some of the following commands:
 * \code{.unparsed}
 *  $ ./ns3 run "rem-example --simTag=d --remMode=CoverageArea"
 *  $ ./ns3 run "rem-example --simTag=u --remMode=UeCoverage"
 *  $ ./ns3 run "rem-example --simTag=b1 --remMode=BeamShape --typeOfRem=DlRem"
 *  $ ./ns3 run "rem-example --simTag=b2 --remMode=BeamShape --typeOfRem=UlRem"
 * \endcode
 *
 * DL or UL REM map can be selected by passing to the rem helper the desired
 * transmitting device(s) (RTD(s)) and receiving device (RRD), which for the DL
 * case correspond to gNB(s) and UE and for the UL case to UE(s) and gNB.
 *
 * The output of the REM includes a map with the SNR values, a map with the SINR
 * and a map with IPSD values (aggregated rx Power in each rem point).
 * Note that in case there is only one gNB configured, the SNR/SINR maps will be the same.
 *
 * The output of this example are REM csv files from which can be generated REM
 * figures with the following command:
 * \code{.unparsed}
 * $ gnuplot -p nr-rem-{simTag}-gnbs.txt nr-rem-{simTag}-ues.txt nr-rem-{simTag}-buildings.txt nr-rem-{simTag}-plot-rem.gnuplot
 * \endcode
 *
 * If no simTag is specified then to plot run the following command:
 *
 * \code{.unparsed}
 * $ gnuplot -p nr-rem--gnbs.txt nr-rem--ues.txt nr-rem--buildings.txt nr-rem--plot-rem.gnuplot
 * \endcode
 *
 * And the following files will be generated (in the root project folder if not specified
 * differently): nr-rem--sinr.png, nr-rem--snr.png and nr-rem--ipsd.png
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/nr-helper.h"
#include "ns3/log.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/nr-mac-scheduler-tdma-rr.h"
#include "ns3/nr-module.h"
#include <ns3/buildings-module.h>
#include "ns3/antenna-module.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  std::string remMode = "CoverageArea";
  std::string simTag = "";

  std::string scenario = "UMa"; //scenario
  std::string beamforming = "dir-dir"; //beamforming at gNB and UE, the first is gNB and the second is UE
  enum BandwidthPartInfo::Scenario scenarioEnum = BandwidthPartInfo::UMa;

  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 1;
  std::string deploymentScenario = "SingleGnb";
  std::string typeOfRem = "DlRem";

  double gNB1x = 0.0;
  double gNB1y = 0.0;
  double gNB2x = -10.0;
  double gNB2y = -15.0;

  double ue1x = 10.0;
  double ue1y = 10.0;
  double ue2x = 50.0;
  double ue2y = -40.0;

  double frequency = 2e9;//28e9
  double bandwidth = 20e6; //100e6
  uint16_t numerology = 0;
  double txPower = 1; //4

  //Antenna Parameters
  double hBS;   //Depend on the scenario (no input parameters)
  double hUT;
  uint32_t numRowsUe = 1; //2
  uint32_t numColumnsUe = 1; //2
  uint32_t numRowsGnb = 1; //4
  uint32_t numColumnsGnb = 1; //4
  bool isoUe = true;
  bool isoGnb = false; //false

  double simTime = 1; // in seconds
  bool logging = false;
  bool enableTraces = false;

  //building parameters in case of buildings addition
  bool enableBuildings = false; //Depends on the scenario (no input parameter)
  uint32_t numOfBuildings = 1;
  uint32_t apartmentsX = 2;
  uint32_t nFloors = 1;

  //Rem parameters
  double xMin = -40.0;
  double xMax = 80.0;
  uint16_t xRes = 50;
  double yMin = -70.0;
  double yMax = 50.0;
  uint16_t yRes = 50;
  double z = 1.5;

  CommandLine cmd;
  cmd.AddValue ("remMode",
                "What type of REM map to use: BeamShape, CoverageArea, UeCoverage."
                "BeamShape shows beams that are configured in a user's script. "
                "Coverage area is used to show worst-case SINR and best-case SNR maps "
                "considering that at each point of the map the best beam is used "
                "towards that point from the serving gNB and also of all the interfering"
                "gNBs in the case of worst-case SINR."
                "UeCoverage is similar to the previous, just that it is showing the "
                "uplink coverage.",
                remMode);
  cmd.AddValue ("simTag",
                "Simulation string tag that will be concatenated to output file names",
                simTag);
  cmd.AddValue ("scenario",
                "The scenario for the simulation. Choose among 'RMa', 'UMa', "
                "'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen'"
                "'UMa-Buildings', 'UMi-Buildings'.",
                scenario);
  cmd.AddValue ("gNbNum",
                "The number of gNbs in multiple-ue topology",
                gNbNum);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per gNb in multiple-ue topology",
                ueNumPergNb);
  cmd.AddValue ("gNB1x",
                "gNb 1 x position",
                gNB1x);
  cmd.AddValue ("gNB1y",
                "gNb 1 y position",
                gNB1y);
  cmd.AddValue ("gNB2x",
                "gNb 2 x position",
                gNB2x);
  cmd.AddValue ("gNB2y",
                "gNb 2 y position",
                gNB2y);
  cmd.AddValue ("ue1x",
                "ue 1 x position",
                ue1x);
  cmd.AddValue ("ue1y",
                "ue 1 y position",
                ue1y);
  cmd.AddValue ("ue2x",
                "ue 2 x position",
                ue2x);
  cmd.AddValue ("ue2y",
                "ue 2 y position",
                ue2y);
  cmd.AddValue ("deploymentScenario",
                "The deployment scenario for the simulation. Choose among "
                "'SingleGnb', 'TwoGnbs'.",
                deploymentScenario);
  cmd.AddValue ("typeOfRem",
                "The type of Rem to generate (DL or UL) in the case of BeamShape option. Choose among "
                "'DlRem', 'UlRem'.",
                typeOfRem);
  cmd.AddValue ("frequency",
                "The central carrier frequency in Hz.",
                frequency);
  cmd.AddValue ("bandwidth",
                "The system bandwidth to be used",
                bandwidth);
  cmd.AddValue ("numerology",
                "The numerology to be used",
                numerology);
  cmd.AddValue ("txPower",
                "total tx power that will be proportionally assigned to"
                " bands, CCs and bandwidth parts depending on each BWP bandwidth ",
                txPower);
  cmd.AddValue ("numRowsUe",
                "Number of rows for the UE antenna",
                numRowsUe);
  cmd.AddValue ("numColumnsUe",
                "Number of columns for the UE antenna",
                numColumnsUe);
  cmd.AddValue ("isoUe",
                "If true (set to 1), use an isotropic radiation pattern in the Ue ",
                isoUe);
  cmd.AddValue ("numRowsGnb",
                "Number of rows for the gNB antenna",
                numRowsGnb);
  cmd.AddValue ("numColumnsGnb",
                "Number of columns for the gNB antenna",
                numColumnsGnb);
  cmd.AddValue ("isoGnb",
                "If true (set to 1), use an isotropic radiation pattern in the gNB ",
                isoGnb);
  cmd.AddValue ("numOfBuildings",
                "The number of Buildings to deploy in the scenario",
                numOfBuildings);
  cmd.AddValue ("apartmentsX",
                "The number of apartments inside a building",
                apartmentsX);
  cmd.AddValue ("nFloors",
                "The number of floors of a building",
                nFloors);
  cmd.AddValue ("beamforming",
                "If dir-dir configure direct-path at both gNB and UE; "
                "if dir-omni configure direct-path at gNB and quasi-omni at UE;"
                "if omni-dir configure quasi-omni at gNB and direct-path at UE;",
                beamforming);
  cmd.AddValue ("logging",
                "Enable logging"
                "another option is by exporting the NS_LOG environment variable",
                logging);
  cmd.AddValue ("xMin",
                "The min x coordinate of the rem map",
                xMin);
  cmd.AddValue ("xMax",
                "The max x coordinate of the rem map",
                xMax);
  cmd.AddValue ("xRes",
                "The resolution on the x axis of the rem map",
                xRes);
  cmd.AddValue ("yMin",
                "The min y coordinate of the rem map",
                yMin);
  cmd.AddValue ("yMax",
                "The max y coordinate of the rem map",
                yMax);
  cmd.AddValue ("yRes",
                "The resolution on the y axis of the rem map",
                yRes);
  cmd.AddValue ("z",
                "The z coordinate of the rem map",
                z);

  cmd.Parse (argc, argv);

  // enable logging
  if (logging)
    {
      //LogComponentEnable ("ThreeGppSpectrumPropagationLossModel", LOG_LEVEL_ALL);
      LogComponentEnable ("ThreeGppPropagationLossModel", LOG_LEVEL_ALL);
      //LogComponentEnable ("ThreeGppChannelModel", LOG_LEVEL_ALL);
      //LogComponentEnable ("ChannelConditionModel", LOG_LEVEL_ALL);
      //LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      //LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      //LogComponentEnable ("LteRlcUm", LOG_LEVEL_LOGIC);
      //LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
    }

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

  // set mobile device and base station antenna heights in meters, according to the chosen scenario
  if (scenario.compare ("RMa") == 0)
    {
      hBS = 35;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::RMa;
    }
  else if (scenario.compare ("UMa") == 0)
    {
      //hBS = 25;
      hBS = 1.5;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMa;
    }
  else if (scenario.compare ("UMa-Buildings") == 0)
    {
      hBS = 1.5; // 25
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMa_Buildings;
      enableBuildings = true;
    }
  else if (scenario.compare ("UMi-StreetCanyon") == 0)
    {
      hBS = 10;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMi_StreetCanyon;
    }
  else if (scenario.compare ("UMi-Buildings") == 0)
    {
      hBS = 10;
      hUT = 1.5;
      scenarioEnum = BandwidthPartInfo::UMi_Buildings;
      enableBuildings = true;
    }
  else if (scenario.compare ("InH-OfficeMixed") == 0)
    {
      hBS = 3;
      hUT = 1;
      scenarioEnum = BandwidthPartInfo::InH_OfficeMixed;
    }
  else if (scenario.compare ("InH-OfficeOpen") == 0)
    {
      hBS = 3;
      hUT = 1;
      scenarioEnum = BandwidthPartInfo::InH_OfficeOpen;
    }
  else
    {
      NS_ABORT_MSG ("Scenario not supported. Choose among 'RMa', 'UMa', "
                    "'UMi-StreetCanyon', 'InH-OfficeMixed', 'InH-OfficeOpen',"
                    "'UMa-Buildings', and 'UMi-Buildings'.");
    }

  if (deploymentScenario.compare ("SingleGnb") == 0)
    {
      gNbNum = 1;
      ueNumPergNb = 1;
    }
  else if (deploymentScenario.compare ("TwoGnbs") == 0)
    {
      gNbNum = 2;
      ueNumPergNb = 1;
    }
  else if (deploymentScenario.compare ("FourGnbs") == 0)
    {
      gNbNum = 4;
      ueNumPergNb = 1;
    }
  else
    {
      NS_ABORT_MSG ("Deployment scenario not supported. "
                    "Choose among 'SingleGnb', 'TwoGnbs'.");
    }

  double offset = 80;

  // create base stations and mobile terminals
  NodeContainer gnbNodes;
  NodeContainer ueNodes;
  gnbNodes.Create (gNbNum);
  ueNodes.Create (ueNumPergNb * gNbNum);

  // position the base stations
  Ptr<ListPositionAllocator> gnbPositionAlloc = CreateObject<ListPositionAllocator> ();
  gnbPositionAlloc->Add (Vector (gNB1x, gNB1y, hBS));
  if (deploymentScenario.compare ("TwoGnbs") == 0)
    {
      gnbPositionAlloc->Add (Vector (gNB2x, gNB2y, hBS));
    }
  if (deploymentScenario.compare ("FourGnbs") == 0)
    {
      gnbPositionAlloc->Add (Vector (gNB2x, gNB2y, hBS));
      gnbPositionAlloc->Add (Vector (gNB1x + offset, gNB1y, hBS));
      gnbPositionAlloc->Add (Vector (gNB2x + offset, gNB2y, hBS));
    }

  MobilityHelper gnbMobility;
  gnbMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  gnbMobility.SetPositionAllocator (gnbPositionAlloc);
  gnbMobility.Install (gnbNodes);

  // position the mobile terminals
  MobilityHelper ueMobility;
  ueMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  ueMobility.Install (ueNodes);

  ueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (ue1x, ue1y, hUT));
  if (deploymentScenario.compare ("TwoGnbs") == 0)
    {
      ueNodes.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (ue2x, ue2y, hUT));
    }

  if (deploymentScenario.compare ("FourGnbs") == 0)
    {
      ueNodes.Get (1)->GetObject<MobilityModel> ()->SetPosition (Vector (ue2x, ue2y, hUT));
      ueNodes.Get (2)->GetObject<MobilityModel> ()->SetPosition (Vector (ue1x + offset, ue1y, hUT));
      ueNodes.Get (3)->GetObject<MobilityModel> ()->SetPosition (Vector (ue2x + offset, ue2y, hUT));
    }

  if (enableBuildings)
    {
      Ptr<GridBuildingAllocator> gridBuildingAllocator;
      gridBuildingAllocator = CreateObject<GridBuildingAllocator> ();
      gridBuildingAllocator->SetAttribute ("GridWidth", UintegerValue (numOfBuildings));
      gridBuildingAllocator->SetAttribute ("LengthX", DoubleValue (2 * apartmentsX));
      gridBuildingAllocator->SetAttribute ("LengthY", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("DeltaX", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("DeltaY", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("Height", DoubleValue (3 * nFloors));
      gridBuildingAllocator->SetBuildingAttribute ("NRoomsX", UintegerValue (apartmentsX));
      gridBuildingAllocator->SetBuildingAttribute ("NRoomsY", UintegerValue (2));
      gridBuildingAllocator->SetBuildingAttribute ("NFloors", UintegerValue (nFloors));
      gridBuildingAllocator->SetAttribute ("MinX", DoubleValue (10));
      gridBuildingAllocator->SetAttribute ("MinY", DoubleValue (10));
      gridBuildingAllocator->Create (numOfBuildings);

      BuildingsHelper::Install (gnbNodes);
      BuildingsHelper::Install (ueNodes);
    }

  /*
   * Create NR simulation helpers
   */
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject <IdealBeamformingHelper> ();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  /*
   * Spectrum configuration:
   * We create a single operational band with 1 CC and 1 BWP.
   *
   * |---------------Band---------------|
   * |---------------CC-----------------|
   * |---------------BWP----------------|
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;


  CcBwpCreator::SimpleOperationBandConf bandConf (frequency, bandwidth, numCcPerBand, scenarioEnum);
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

  //Initialize channel and pathloss, plus other things inside band.
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds (0)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  nrHelper->InitializeOperationBand (&band);
  allBwps = CcBwpCreator::GetAllBwps ({band});

  // Configure beamforming method
  if (beamforming.compare ("dir-dir") == 0)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ())); // dir at gNB, dir at UE
    }
  else if (beamforming.compare ("dir-omni") == 0)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathQuasiOmniBeamforming::GetTypeId ())); // dir at gNB, q-omni at UE
    }
  else if (beamforming.compare ("omni-dir") == 0)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (QuasiOmniDirectPathBeamforming::GetTypeId ())); // q-omni at gNB, dir at UE
    }
  else if (beamforming.compare ("search-omni") == 0)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (CellScanQuasiOmniBeamforming::GetTypeId ())); // q-omni at gNB, dir at UE
    }
  else
    {
      NS_FATAL_ERROR ("Beamforming does not exist:" << beamforming);
    }

  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  // Antennas for the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (numRowsUe));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (numColumnsUe));
  //Antenna element type for UEs
  if (isoUe)
    {
      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
    }
  else
    {
      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
    }

  // Antennas for the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (numRowsGnb));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (numColumnsGnb));
  //Antenna element type for gNBs
  if (isoGnb)
    {
      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
    }
  else
    {
      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
    }


  // install nr net devices
  NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice (gnbNodes, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);

  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (gnbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);

  for (uint32_t i = 0; i < gNbNum; ++i)
    {
      nrHelper->GetGnbPhy (gnbNetDev.Get (i), 0)->SetTxPower (txPower);
      nrHelper->GetGnbPhy (gnbNetDev.Get (i), 0)->SetAttribute ("Numerology", UintegerValue (numerology));
    }

  // When all the configuration is done, explicitly call UpdateConfig ()
  for (auto it = gnbNetDev.Begin (); it != gnbNetDev.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  // create the internet and install the IP stack on the UEs
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
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);

  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      UdpServerHelper dlPacketSinkHelper (dlPort);
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u)));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds (1)));
      //dlClient.SetAttribute ("MaxPackets", UintegerValue(0xFFFFFFFF));
      dlClient.SetAttribute ("MaxPackets", UintegerValue (10));
      dlClient.SetAttribute ("PacketSize", UintegerValue (1500));
      clientApps.Add (dlClient.Install (remoteHost));
    }

  // attach UEs to the closest gNB
  nrHelper->AttachToEnb (ueNetDev.Get (0), gnbNetDev.Get (0));

  if (deploymentScenario.compare ("TwoGnbs") == 0)
    {
      nrHelper->AttachToEnb (ueNetDev.Get (1), gnbNetDev.Get (1));
    }

  if (deploymentScenario.compare ("FourGnbs") == 0)
    {
      nrHelper->AttachToEnb (ueNetDev.Get (1), gnbNetDev.Get (1));
      nrHelper->AttachToEnb (ueNetDev.Get (2), gnbNetDev.Get (2));
      nrHelper->AttachToEnb (ueNetDev.Get (3), gnbNetDev.Get (3));
    }


  // start server and client apps
  serverApps.Start (Seconds (0.4));
  clientApps.Start (Seconds (0.4));
  serverApps.Stop (Seconds (simTime));
  clientApps.Stop (Seconds (simTime - 0.2));

  // enable the traces provided by the nr module
  if (enableTraces)
    {
      nrHelper->EnableTraces ();
    }

  uint16_t remBwpId = 0;
  //Radio Environment Map Generation for ccId 0
  Ptr<NrRadioEnvironmentMapHelper> remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();
  remHelper->SetMinX (xMin);
  remHelper->SetMaxX (xMax);
  remHelper->SetResX (xRes);
  remHelper->SetMinY (yMin);
  remHelper->SetMaxY (yMax);
  remHelper->SetResY (yRes);
  remHelper->SetZ (z);
  remHelper->SetSimTag (simTag);

  gnbNetDev.Get (0)->GetObject<NrGnbNetDevice> ()->GetPhy (remBwpId)->GetSpectrumPhy(0)->GetBeamManager ()->ChangeBeamformingVector (ueNetDev.Get (0));

  if (deploymentScenario.compare ("TwoGnbs") == 0)
    {
      gnbNetDev.Get (1)->GetObject<NrGnbNetDevice> ()->GetPhy (remBwpId)->GetSpectrumPhy(0)->GetBeamManager ()->ChangeBeamformingVector (ueNetDev.Get (1));
    }

  if (deploymentScenario.compare ("FourGnbs") == 0)
    {
      gnbNetDev.Get (1)->GetObject<NrGnbNetDevice> ()->GetPhy (remBwpId)->GetSpectrumPhy(0)->GetBeamManager ()->ChangeBeamformingVector (ueNetDev.Get (1));
      gnbNetDev.Get (2)->GetObject<NrGnbNetDevice> ()->GetPhy (remBwpId)->GetSpectrumPhy(0)->GetBeamManager ()->ChangeBeamformingVector (ueNetDev.Get (2));
      gnbNetDev.Get (3)->GetObject<NrGnbNetDevice> ()->GetPhy (remBwpId)->GetSpectrumPhy(0)->GetBeamManager ()->ChangeBeamformingVector (ueNetDev.Get (3));
    }

  if (remMode == "BeamShape")
    {
      remHelper->SetRemMode (NrRadioEnvironmentMapHelper::BEAM_SHAPE);

      if (typeOfRem.compare ("DlRem") == 0)
        {
          remHelper->CreateRem (gnbNetDev, ueNetDev.Get (0), remBwpId);
        }
      else if (typeOfRem.compare ("UlRem") == 0)
        {
          remHelper->CreateRem (ueNetDev, gnbNetDev.Get (0), remBwpId);
        }
      else
        {
          NS_ABORT_MSG ("typeOfRem not supported. "
                        "Choose among 'DlRem', 'UlRem'.");
        }
    }
  else if (remMode == "CoverageArea")
    {
      remHelper->SetRemMode (NrRadioEnvironmentMapHelper::COVERAGE_AREA);
      remHelper->CreateRem (gnbNetDev, ueNetDev.Get (0), remBwpId);
    }
  else if (remMode == "UeCoverage")
    {
      remHelper->SetRemMode (NrRadioEnvironmentMapHelper::UE_COVERAGE);
      remHelper->CreateRem (ueNetDev, gnbNetDev.Get (0), remBwpId);
    }
  else
    {
      NS_ABORT_MSG ("Not supported remMode.");
    }

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}



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

#include "lena-lte-comparison.h"

#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/antenna-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/flow-monitor-module.h"

#include "ns3/lte-module.h"
#include <ns3/radio-environment-map-helper.h>
#include "ns3/config-store-module.h"
#include <ns3/sqlite-output.h>
#include "sinr-output-stats.h"
#include "flow-monitor-output-stats.h"
#include "power-output-stats.h"
#include "slot-output-stats.h"
#include "rb-output-stats.h"
#include "lena-v1-utils.h"
#include "lena-v2-utils.h"

#include <iomanip>

/*
 * To be able to use LOG_* functions.
 */
#include "ns3/log.h"

/*
 * With this line, we will be able to see the logs of the file by enabling the
 * component "LenaLteComparison", in this way:
 *
 * $ export NS_LOG="LenaLteComparison=level_info|prefix_func|prefix_time"
 */
NS_LOG_COMPONENT_DEFINE ("LenaLteComparison");

namespace ns3 {

const Time appStartWindow = MilliSeconds (50);

static std::pair<ApplicationContainer, Time>
InstallApps (const Ptr<Node> &ue, const Ptr<NetDevice> &ueDevice,
             const Address &ueAddress, const std::string &direction,
             UdpClientHelper *dlClientLowLat, const Ptr<Node> &remoteHost,
             const Ipv4Address &remoteHostAddr, Time udpAppStartTime,
             uint16_t dlPortLowLat, const Ptr<UniformRandomVariable> &x,
             Time appGenerationTime,
             const Ptr<LteHelper> &lteHelper, const Ptr<NrHelper> &nrHelper)
{
  ApplicationContainer app;

  // The bearer that will carry low latency traffic
  EpsBearer lowLatBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);

  // The filter for the low-latency traffic
  Ptr<EpcTft> lowLatTft = Create<EpcTft> ();
  EpcTft::PacketFilter dlpfLowLat;
  if (direction == "DL")
    {
      dlpfLowLat.localPortStart = dlPortLowLat;
      dlpfLowLat.localPortEnd = dlPortLowLat;
      dlpfLowLat.direction = EpcTft::DOWNLINK;
    }
  else
    {
      dlpfLowLat.remotePortStart = dlPortLowLat;
      dlpfLowLat.remotePortEnd = dlPortLowLat;
      dlpfLowLat.direction = EpcTft::UPLINK;
    }
  lowLatTft->Add (dlpfLowLat);

  // The client, who is transmitting, is installed in the remote host,
  // with destination address set to the address of the UE
  if (direction == "DL")
    {
      dlClientLowLat->SetAttribute ("RemoteAddress", AddressValue (ueAddress));
      app = dlClientLowLat->Install (remoteHost);
    }
  else
    {
      dlClientLowLat->SetAttribute ("RemoteAddress", AddressValue (remoteHostAddr));
      app = dlClientLowLat->Install (ue);
    }

  double start = x->GetValue (udpAppStartTime.GetMilliSeconds (),
                              (udpAppStartTime + appStartWindow).GetMilliSeconds ());
  Time startTime = MilliSeconds (start);
  app.Start (startTime);
  app.Stop (startTime + appGenerationTime);

  std::cout << "\tStarts at time " << startTime.As (Time::MS) << " and ends at "
            << (startTime + appGenerationTime).As (Time::MS) << std::endl;

  // Activate a dedicated bearer for the traffic type
  if (lteHelper != nullptr)
    {
      lteHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
    }
  else if (nrHelper != nullptr)
    {
      nrHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
    }

  return std::make_pair (app, startTime);
}


bool
Parameters::Validate (void) const
{

  NS_ABORT_MSG_IF (bandwidthMHz != 20 && bandwidthMHz != 10 && bandwidthMHz != 5,
                   "Valid bandwidth values are 20, 10, 5, you set " << bandwidthMHz);

  NS_ABORT_MSG_IF (trafficScenario > 3,
                   "Traffic scenario " << trafficScenario << " not valid. Valid values are 0 1 2 3");

  NS_ABORT_MSG_IF (numerologyBwp > 4,
                   "At most 4 bandwidth parts supported.");

  NS_ABORT_MSG_IF (direction != "DL" && direction != "UL",
                   "Flow direction can only be DL or UL: " << direction);
  NS_ABORT_MSG_IF (operationMode != "TDD" && operationMode != "FDD",
                   "Operation mode can only be TDD or FDD: " << operationMode);
  NS_ABORT_MSG_IF (radioNetwork != "LTE" && radioNetwork != "NR",
                   "Unrecognized radio network technology: " << radioNetwork);
  NS_ABORT_MSG_IF (radioNetwork == "LTE" && operationMode != "FDD",
                   "Operation mode must be FDD in a 4G LTE network: " << operationMode);
  NS_ABORT_MSG_IF (simulator != "LENA" && simulator != "5GLENA",
                   "Unrecognized simulator: " << simulator);
  NS_ABORT_MSG_IF (scheduler != "PF" && scheduler != "RR",
                   "Unrecognized scheduler: " << scheduler);

  if (dlRem || ulRem)
    {
      NS_ABORT_MSG_IF (simulator != "5GLENA",
                       "Cannot do the REM with the simulator " << simulator);
      NS_ABORT_MSG_IF (dlRem && ulRem,
                       "You selected both DL and UL REM, that is not supported");
      NS_ABORT_MSG_IF (remSector > 3,
                       "Only three sectors supported for REM");

      NS_ABORT_MSG_IF (remSector == 0 && freqScenario != 1,
                       "RemSector == 0 makes sense only in a OVERLAPPING scenario");
    }

  return true;
}


void
LenaLteComparison (const Parameters &params)
{
  params.Validate ();

  // Traffic parameters (that we will use inside this script:)
  uint32_t udpPacketSize = 1000;
  uint32_t lambda;
  uint32_t packetCount;

  std::cout << "\n----------------------------------------\n"
            << "Configuring scenario"
            << std::endl;

  std::cout << "  traffic parameters\n";
  switch (params.trafficScenario)
    {
      case 0: // let's put 80 Mbps with 20 MHz of bandwidth. Everything else is scaled
        packetCount = 0xFFFFFFFF;
        switch (params.bandwidthMHz)
          {
            case 20:
              udpPacketSize = 1000;
              break;
            case 10:
              udpPacketSize = 500;
              break;
            case 5:
              udpPacketSize = 250;
              break;
            default:
              udpPacketSize = 1000;
          }
        lambda = 10000 / params.ueNumPergNb;
        break;
      case 1:
        packetCount = 1;
        udpPacketSize = 12;
        lambda = 1;
        break;
      case 2: // 1 Mbps == 0.125 MB/s in case of 20 MHz, everything else is scaled
        packetCount = 0xFFFFFFFF;
        switch (params.bandwidthMHz)
          {
            case 20:
              udpPacketSize = 125;
              break;
            case 10:
              udpPacketSize = 63;
              break;
            case 5:
              udpPacketSize = 32;
              break;
            default:
              udpPacketSize = 125;
          }
        lambda = 1000 / params.ueNumPergNb;
        break;
      case 3: // 20 Mbps == 2.5 MB/s in case of 20 MHz, everything else is scaled
        packetCount = 0xFFFFFFFF;
        switch (params.bandwidthMHz)
          {
            case 20:
              udpPacketSize = 250;
              break;
            case 10:
              udpPacketSize = 125;
              break;
            case 5:
              udpPacketSize = 75;
              break;
            default:
              udpPacketSize = 250;
          }
        lambda = 10000 / params.ueNumPergNb;
        break;
      default:
        NS_FATAL_ERROR ("Traffic scenario " << params.trafficScenario << " not valid. Valid values are 0 1 2 3");
    }

  std::cout << "  statistics\n";
  SQLiteOutput db (params.outputDir + "/" + params.simTag + ".db");
  SinrOutputStats sinrStats;
  PowerOutputStats ueTxPowerStats;
  PowerOutputStats gnbRxPowerStats;
  SlotOutputStats slotStats;
  RbOutputStats rbStats;

  sinrStats.SetDb (&db);
  ueTxPowerStats.SetDb (&db, "ueTxPower");
  slotStats.SetDb (&db);
  rbStats.SetDb (&db);
  gnbRxPowerStats.SetDb (&db, "gnbRxPower");

  /*
   * Check if the frequency and numerology are in the allowed range.
   * If you need to add other checks, here is the best position to put them.
   */
  std::cout << "  checking frequency and numerology\n";

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
  std::cout << "  logging\n";
  if (params.logging)
    {
      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
//      LogComponentEnable ("NrMacSchedulerOfdma", LOG_LEVEL_ALL);
    }

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  std::cout << "  max tx buffer size\n";
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

  /*
   * Create the scenario. In our examples, we heavily use helpers that setup
   * the gnbs and ue following a pre-defined pattern. Please have a look at the
   * HexagonalGridScenarioHelper documentation to see how the nodes will be distributed.
   */

  ScenarioParameters scenarioParams;
  scenarioParams.SetScenarioParameters (params.scenario);
  // Customize parameters here
  //  scenarioParams.isd = ...

  // The essentials describing a laydown
  uint32_t gnbSites = 0;
  NodeContainer gnbNodes;
  NodeContainer ueNodes;
  double sector0AngleRad = 0;
  const uint32_t sectors = 3;

  //
  NodeDistributionScenarioInterface * scenario {NULL};
  FileScenarioHelper fileScenario;
  HexagonalGridScenarioHelper gridScenario;

  if (params.baseStationFile != "" and params.useSiteFile)
    {
      std::cout << "  using tower positions from " << params.baseStationFile
                << std::endl;
      std::cout << "    setting sectorization" << std::endl;
      fileScenario.SetSectorization (sectors);
      std::cout << "    adding site file" << std::endl;
      fileScenario.Add (params.baseStationFile);
      std::cout << "    setting scenario" << std::endl;
      fileScenario.SetScenarioParameters (params.scenario);
      std::cout << "    getting number of sites..." << std::flush;
      gnbSites = fileScenario.GetNumSites ();
      std::cout << gnbSites << std::endl;
      std::cout << "    getting number of cells..." << std::flush;
      uint32_t gnbNum = fileScenario.GetNumCells ();
      std::cout << gnbNum << std::endl;
      uint32_t ueNum = params.ueNumPergNb * gnbNum;
      std::cout << "    setting number of UEs..." << ueNum << std::endl;
      fileScenario.SetUtNumber (ueNum);
      std::cout << "    getting sector 0 angle..." << std::flush;
      sector0AngleRad = fileScenario.GetAntennaOrientationRadians (0);
      std::cout << sector0AngleRad << std::endl;

      // Creates and plots the network deployment
      std::cout << "    creating scenario" << std::endl;
      fileScenario.CreateScenario ();
      std::cout << "    getting gnbNodes..." << std::flush;
      gnbNodes = fileScenario.GetBaseStations ();
      std::cout << gnbNodes.GetN () << std::endl;
      std::cout << "    getting ueNodes..." << std::flush;
      ueNodes = fileScenario.GetUserTerminals ();
      std::cout << ueNodes.GetN () << std::endl;
      std::cout << "    setting scenario pointer..." << std::flush;
      scenario = &fileScenario;
      std::cout << "0x" << std::hex << scenario << std::dec << std::endl;
    }
  else
    {
      std::cout << "  hexagonal grid: ";
      gridScenario.SetScenarioParameters (scenarioParams);
      gridScenario.SetNumRings (params.numOuterRings);
      gnbSites = gridScenario.GetNumSites ();
      uint32_t ueNum = params.ueNumPergNb * gnbSites * sectors;
      gridScenario.SetUtNumber (ueNum);
      sector0AngleRad = gridScenario.GetAntennaOrientationRadians (0);
      std::cout << sector0AngleRad << std::endl;

      // Creates and plots the network deployment
      gridScenario.CreateScenario ();
      gnbNodes = gridScenario.GetBaseStations ();
      ueNodes = gridScenario.GetUserTerminals ();
      scenario = &gridScenario;
    }

  // Log the configuration
  std::cout << "\n    Topology configuration: " << gnbSites << " sites, "
            << sectors << " sectors/site, "
            << gnbNodes.GetN ()   << " cells, "
            << ueNodes.GetN ()    << " UEs\n";

  /*
   * Create different gNB NodeContainer for the different sectors.
   *
   * Relationships between ueId, cellId, sectorId and siteId:
   * ~~~{.cc}
   *   cellId = scenario->GetCellIndex (ueId);
   *   sector = scenario->GetSectorIndex (cellId);
   *   siteId = scenario->GetSiteIndex (cellId);
   * ~~~{.cc}
   *
   * Iterate/index gnbNodes, gnbNetDevs by `cellId`.
   * Iterate/index gnbSector<N>Container, gnbNodesBySector[sector],
   *   gnbSector<N>NetDev, gnbNdBySector[sector] by `siteId`
   */
  NodeContainer gnbSector1Container, gnbSector2Container, gnbSector3Container;
  std::vector<NodeContainer*> gnbNodesBySector{&gnbSector1Container, &gnbSector2Container, &gnbSector3Container};
  for (uint32_t cellId = 0; cellId < gnbNodes.GetN (); ++cellId)
    {
      Ptr<Node> gnb = gnbNodes.Get (cellId);
      auto sector = scenario->GetSectorIndex (cellId);
      gnbNodesBySector[sector]->Add (gnb);
    }
  std::cout << "    gNb containers: "
            << gnbSector1Container.GetN () << ", "
            << gnbSector2Container.GetN () << ", "
            << gnbSector3Container.GetN ()
            << std::endl;

  /*
   * Create different UE NodeContainer for the different sectors.
   *
   * Multiple UEs per sector!
   * Iterate/index ueNodes, ueNetDevs, ueIpIfaces by `ueId`.
   * Iterate/Index ueSector<N>Container, ueNodesBySector[sector],
   *   ueSector<N>NetDev, ueNdBySector[sector] with i % gnbSites
   */
  NodeContainer ueSector1Container, ueSector2Container, ueSector3Container;
  std::vector<NodeContainer*> ueNodesBySector {&ueSector1Container, &ueSector2Container, &ueSector3Container};
  for (uint32_t ueId = 0; ueId < ueNodes.GetN (); ++ueId)
    {
      Ptr<Node> ue = ueNodes.Get (ueId);
      auto cellId = scenario->GetCellIndex (ueId);
      auto sector = scenario->GetSectorIndex (cellId);
      ueNodesBySector[sector]->Add (ue);
    }
  std::cout << "    UE containers: "
            << ueSector1Container.GetN () << ", "
            << ueSector2Container.GetN () << ", "
            << ueSector3Container.GetN ()
            << std::endl;

  /*
   * Setup the LTE or NR module. We create the various helpers needed inside
   * their respective configuration functions
   */
  std::cout << "  helpers\n";
  Ptr<PointToPointEpcHelper> epcHelper;

  NetDeviceContainer gnbSector1NetDev, gnbSector2NetDev, gnbSector3NetDev;
  std::vector<NetDeviceContainer *> gnbNdBySector {&gnbSector1NetDev, &gnbSector2NetDev, &gnbSector3NetDev};
  NetDeviceContainer ueSector1NetDev, ueSector2NetDev,ueSector3NetDev;
  std::vector<NetDeviceContainer *> ueNdBySector {&ueSector1NetDev, &ueSector2NetDev, &ueSector3NetDev};

  Ptr <LteHelper> lteHelper = nullptr;
  Ptr <NrHelper> nrHelper = nullptr;

  if (params.simulator == "LENA")
    {
      epcHelper = CreateObject<PointToPointEpcHelper> ();
      LenaV1Utils::SetLenaV1SimulatorParameters (sector0AngleRad,
                                                 params.scenario,
                                                 gnbSector1Container,
                                                 gnbSector2Container,
                                                 gnbSector3Container,
                                                 ueSector1Container,
                                                 ueSector2Container,
                                                 ueSector3Container,
                                                 epcHelper,
                                                 lteHelper,
                                                 gnbSector1NetDev,
                                                 gnbSector2NetDev,
                                                 gnbSector3NetDev,
                                                 ueSector1NetDev,
                                                 ueSector2NetDev,
                                                 ueSector3NetDev,
                                                 params.calibration,
                                                 params.enableUlPc,
                                                 &sinrStats,
                                                 &ueTxPowerStats,
                                                 params.scheduler,
                                                 params.bandwidthMHz,
                                                 params.freqScenario,
                                                 params.downtiltAngle);
    }
  else if (params.simulator == "5GLENA")
    {
      epcHelper = CreateObject<NrPointToPointEpcHelper> ();
      LenaV2Utils::SetLenaV2SimulatorParameters (sector0AngleRad,
                                                 params.scenario,
                                                 params.radioNetwork,
                                                 params.errorModel,
                                                 params.operationMode,
                                                 params.direction,
                                                 params.numerologyBwp,
                                                 params.pattern,
                                                 gnbSector1Container,
                                                 gnbSector2Container,
                                                 gnbSector3Container,
                                                 ueSector1Container,
                                                 ueSector2Container,
                                                 ueSector3Container,
                                                 epcHelper,
                                                 nrHelper,
                                                 gnbSector1NetDev,
                                                 gnbSector2NetDev,
                                                 gnbSector3NetDev,
                                                 ueSector1NetDev,
                                                 ueSector2NetDev,
                                                 ueSector3NetDev,
                                                 params.calibration,
                                                 params.enableUlPc,
                                                 params.powerAllocation,
                                                 &sinrStats,
                                                 &ueTxPowerStats,
                                                 &gnbRxPowerStats,
                                                 &slotStats,
                                                 &rbStats,
                                                 params.scheduler,
                                                 params.bandwidthMHz,
                                                 params.freqScenario,
                                                 params.downtiltAngle);
    }

  // Check we got one valid helper
  if ( (lteHelper == nullptr) && (nrHelper == nullptr) )
    {
      NS_ABORT_MSG ("Programming error: no valid helper");
    }


  // From here, it is standard NS3. In the future, we will create helpers
  // for this part as well.

  // create the internet and install the IP stack on the UEs
  // get SGW/PGW and create a single RemoteHost
  std::cout << "  pgw and internet\n";
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
  internet.Install (ueNodes);

  NetDeviceContainer gnbNetDevs (gnbSector1NetDev, gnbSector2NetDev);
  gnbNetDevs.Add (gnbSector3NetDev);
  NetDeviceContainer ueNetDevs (ueSector1NetDev, ueSector2NetDev);
  ueNetDevs.Add (ueSector3NetDev);
  Ipv4InterfaceContainer ueIpIfaces {epcHelper->AssignUeIpv4Address (ueNetDevs)};

  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  // Set the default gateway for the UEs
  std::cout << "  default gateway\n";
  for (auto ue = ueNodes.Begin (); ue != ueNodes.End (); ++ue)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting ((*ue)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // attach UEs to their gNB. Try to attach them per cellId order
  std::cout << "  attach UEs to gNBs\n" << std::endl;
  for (uint32_t ueId = 0; ueId < ueNodes.GetN (); ++ueId)
    {
      auto cellId = scenario->GetCellIndex (ueId);
      Ptr<NetDevice> gnbNetDev = gnbNodes.Get (cellId)->GetDevice(0);
      Ptr<NetDevice> ueNetDev = ueNodes.Get (ueId)->GetDevice(0);
      if (lteHelper != nullptr)
        {
          lteHelper->Attach (ueNetDev, gnbNetDev);
        }
      else if (nrHelper != nullptr)
        {
          nrHelper->AttachToEnb (ueNetDev, gnbNetDev);
          auto uePhyBwp0 {nrHelper->GetUePhy (ueNetDev, 0)};
          auto gnbPhyBwp0 {nrHelper->GetGnbPhy (gnbNetDev, 0)};
          Vector gnbpos = gnbNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
          Vector uepos = ueNetDev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();
          double distance = CalculateDistance (gnbpos, uepos);
          std::cout <<"ueId "<< ueId
                    <<", cellIndex "<< cellId
                    <<", ue freq "<< uePhyBwp0->GetCentralFrequency () / 1e9
                    <<", gnb freq "<< gnbPhyBwp0->GetCentralFrequency () / 1e9
                    <<", sector "<< scenario->GetSectorIndex (cellId)
                    <<", distance "<< distance
                    <<", azimuth gnb->ue:"<< RadiansToDegrees (Angles (gnbpos, uepos).GetAzimuth())
                    << std::endl;
        }
    }

  /*
   * Traffic part. Install two kind of traffic: low-latency and voice, each
   * identified by a particular source port.
   */
  std::cout << "  server factory\n";
  uint16_t dlPortLowLat = 1234;

  ApplicationContainer serverApps;

  // The sink will always listen to the specified ports
  UdpServerHelper dlPacketSinkLowLat (dlPortLowLat);

  // The server, that is the application which is listening, is installed in the UE
  if (params.direction == "DL")
    {
      serverApps.Add (dlPacketSinkLowLat.Install (ueNodes));
    }
  else
    {
      serverApps.Add (dlPacketSinkLowLat.Install (remoteHost));
    }

  // start UDP server
  serverApps.Start (params.udpAppStartTime);

  /*
   * Configure attributes for the different generators, using user-provided
   * parameters for generating a CBR traffic
   *
   * Low-Latency configuration and object creation:
   */
  Time interval = Seconds (1.0 / lambda);
  std::cout << "  client factory:"
            << "\n    packet size: " << udpPacketSize
            << "\n    interval:    " << interval
            << "\n    max packets: " << packetCount
            << std::endl;

  UdpClientHelper dlClientLowLat;
  dlClientLowLat.SetAttribute ("RemotePort", UintegerValue (dlPortLowLat));
  dlClientLowLat.SetAttribute ("MaxPackets", UintegerValue (packetCount));
  dlClientLowLat.SetAttribute ("PacketSize", UintegerValue (udpPacketSize));
  dlClientLowLat.SetAttribute ("Interval", TimeValue (interval));

  /*
   * Let's install the applications!
   */
  std::cout << "  applications\n";
  ApplicationContainer clientApps;
  Ptr<UniformRandomVariable> startRng = CreateObject<UniformRandomVariable> ();
  startRng->SetStream (RngSeedManager::GetRun ());
  Time maxStartTime;

  for (uint32_t ueId = 0; ueId < ueNodes.GetN (); ++ueId)
    {
      auto cellId = scenario->GetCellIndex (ueId);
      auto sector = scenario->GetSectorIndex (cellId);
      auto siteId = scenario->GetSiteIndex (cellId);
      Ptr<Node> node = ueNodes.Get (ueId);
      Ptr<NetDevice> dev = ueNetDevs.Get (ueId);
      Address addr = ueIpIfaces.GetAddress (ueId);

      std::cout << "app for ue " << ueId
                << ", cellId " << cellId
                << ", sector " << sector
                << ", siteId " << siteId;
      // << ":" << std::endl;

      auto app = InstallApps (node, dev, addr,
                              params.direction, &dlClientLowLat,
                              remoteHost, remoteHostAddr,
                              params.udpAppStartTime, dlPortLowLat,
                              startRng, params.appGenerationTime,
                              lteHelper, nrHelper);
      maxStartTime = std::max (app.second, maxStartTime);
      clientApps.Add (app.first);
    }
  std::cout << clientApps.GetN () << " apps\n";

  // enable the traces provided by the nr module
  std::cout << "  tracing\n";
  if (params.traces == true)
    {
      if (lteHelper != nullptr)
        {
          lteHelper->EnableTraces ();
        }
      else if (nrHelper != nullptr)
        {
          nrHelper->EnableTraces ();
        }
    }


  std::cout << "  flowmon\n";
  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (ueNodes);

  Ptr<FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  std::string tableName = "e2e";


  Ptr<NrRadioEnvironmentMapHelper> remHelper; // Must be placed outside of block "if (generateRem)" because otherwise it gets destroyed,
                                              // and when simulation starts the object does not exist anymore, but the scheduled REM events do (exist).
                                              // So, REM events would be called with invalid pointer to remHelper ...

  if (params.dlRem || params.ulRem)
    {
      std::cout << "  rem helper\n";

      uint16_t remPhyIndex = 0;
      if (params.operationMode == "FDD" && params.direction == "UL")
        {
          remPhyIndex = 1;
        }

      NetDeviceContainer remNd;
      Ptr<NetDevice> remDevice;

      // params.ulRem:
      std::vector<NetDeviceContainer *> remNdBySector {ueNdBySector};
      std::vector<NetDeviceContainer *> remDevBySector {gnbNdBySector};

      if (params.dlRem)
        {
          remNdBySector = gnbNdBySector;
          remDevBySector = ueNdBySector;
        }

      uint32_t sectorIndex = 0;
      // Reverse order so we get sector 1 for the remSector == 0 case
      for (uint32_t sector = sectors; sector > 0; --sector)
        {
          if (params.remSector == sector || params.remSector == 0 )
            {
              sectorIndex = sector-1;
              remNd.Add (*remNdBySector[sectorIndex]);
              remDevice = remDevBySector[sectorIndex]->Get (0);
            }
        }

      if (params.ulRem)
        {
          auto antArray = DynamicCast<NrGnbNetDevice> (remDevice)->GetPhy (0)->GetSpectrumPhy ()->GetAntenna ();
          auto antenna = DynamicCast<UniformPlanarArray> (antArray);
          antenna->SetAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
        }

      //Radio Environment Map Generation for ccId 0
      remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();
      remHelper->SetMinX (params.xMinRem);
      remHelper->SetMaxX (params.xMaxRem);
      remHelper->SetResX (params.xResRem);
      remHelper->SetMinY (params.yMinRem);
      remHelper->SetMaxY (params.yMaxRem);
      remHelper->SetResY (params.yResRem);
      remHelper->SetZ (params.zRem);

      //save beamforming vectors, one per site (?)
      for (uint32_t sector = sectors; sector > 0; --sector)
        {
          if ( (params.remSector == sector) || (params.remSector == 0) )
            {
              sectorIndex = sector-1;
              for (uint32_t siteId = 0; siteId < gnbSites; ++siteId)
                {
                  gnbNdBySector[sectorIndex]->Get (siteId)
                  ->GetObject<NrGnbNetDevice>()
                  ->GetPhy (remPhyIndex)
                  ->ChangeBeamformingVector (ueNdBySector[sectorIndex]->Get (siteId));
                }
            }
        }

      remHelper->CreateRem (remNd, remDevice, remPhyIndex);
    }

  std::cout << "\n----------------------------------------\n"
            << "Start simulation"
            << std::endl;
  // Add some extra time for the last generated packets to be received
  const Time appStopWindow = MilliSeconds (50);
  Time stopTime = maxStartTime + params.appGenerationTime + appStopWindow;
  Simulator::Stop (stopTime);
  Simulator::Run ();

  sinrStats.EmptyCache ();
  ueTxPowerStats.EmptyCache ();
  gnbRxPowerStats.EmptyCache ();
  slotStats.EmptyCache ();
  rbStats.EmptyCache ();

  /*
   * To check what was installed in the memory, i.e., BWPs of eNb Device, and its configuration.
   * Example is: Node 1 -> Device 0 -> BandwidthPartMap -> {0,1} BWPs -> NrGnbPhy -> Numerology,
  GtkConfigStore config;
  config.ConfigureAttributes ();
  */

  FlowMonitorOutputStats flowMonStats;
  flowMonStats.SetDb (&db, tableName);
  flowMonStats.Save (monitor, flowmonHelper, params.outputDir + "/" + params.simTag);

  std::cout << "\n----------------------------------------\n"
            << "End simulation"
            << std::endl;

  Simulator::Destroy ();
}

std::ostream &
operator << (std::ostream & os, const Parameters & parameters)
{
  // Use p as shorthand for arg parametersx
  auto p {parameters};

#define MSG(m) \
  os << "\n" << m << std::left \
     << std::setw (40 - strlen (m)) << (strlen (m) > 0 ? ":" : "")


  MSG ("LENA LTE Scenario Parameters");
  MSG ("");
  MSG ("Model version")
    << p.simulator << (p.simulator == "LENA" ? " (v1)" : " (v2)");
  if (p.simulator == "5GLENA")
    {
      MSG ("LTE Standard")
        << p.radioNetwork << (p.radioNetwork == "LTE" ? " (4G)" : " (5G NR)");
      MSG ("4G-NR calibration mode") << (p.calibration ? "ON" : "off");
      MSG ("4G-NR ULPC mode") << (p.enableUlPc ? "Enabled" : "Disabled");
      MSG ("Operation mode") << p.operationMode;
      if (p.operationMode == "TDD")
        {
          MSG ("Numerology") << p.numerologyBwp;
          MSG ("TDD pattern") << p.pattern;
        }
      if (p.errorModel != "")
        {
          MSG ("Error model") << p.errorModel;
        }
      else if (p.radioNetwork == "LTE")
        {
          MSG ("Error model") << "ns3::LenaErrorModel";
        }
      else if (p.radioNetwork == "NR")
        {
          MSG ("Error model") << "ns3::NrEesmCcT2";
        }

    }
  else
    {
      // LENA v1
      p.operationMode = "FDD";
      MSG ("LTE Standard") << "4G";
      MSG ("Calibration mode") << (p.calibration ? "ON" : "off");
      MSG ("LTE ULPC mode") << (p.enableUlPc ? "Enabled" : "Disabled");
      MSG ("Operation mode") << p.operationMode;
    }

  if (p.baseStationFile != "" and p.useSiteFile)
    {
      MSG ("Base station positions") << "read from file " << p.baseStationFile;
    }
  else
    {
      MSG ("Base station positions") << "regular hexaonal lay down";
      MSG ("Number of rings") << p.numOuterRings;
    }

  MSG ("");
  MSG ("Channel bandwidth") << p.bandwidthMHz << " MHz";
  MSG ("Spectrum configuration")
    <<    (p.freqScenario == 0 ? "non-" : "") << "overlapping";
  MSG ("LTE Scheduler") << p.scheduler;

  MSG ("");
  MSG ("Basic scenario") << p.scenario;
  if (p.scenario == "UMa")
    {
      os << "\n  (ISD: 1.7 km, BS: 30 m, UE: 1.5 m, UE-BS min: 30.2 m)";
    }
  else if (p.scenario == "UMi")
    {
      os << "\n  (ISD: 0.5 km, BS: 10 m, UE: 1.5 m, UE-BS min: 10 m)";
    }
  else if (p.scenario == "RMa")
    {
      os << "\n  (ISD: 7.0 km, BS: 45 m, UE: 1.5 m, UE-BS min: 44.6 m)";
    }
  else
    {
      os << "\n  (unknown configuration)";
    }
  if (p.baseStationFile == "" and p.useSiteFile)
    {
      MSG ("Number of outer rings") << p.numOuterRings;
    }
  MSG ("Number of UEs per sector") << p.ueNumPergNb;
  MSG ("Antenna down tilt angle (deg)") << p.downtiltAngle;

  MSG ("");
  MSG ("Network loading") << p.trafficScenario;
  switch (p.trafficScenario)
    {
      case 0:
        MSG ("  Max loading (80 Mbps/20 MHz)");
        MSG ("  Number of packets") << "infinite";
        MSG ("  Packet size");
        switch (p.bandwidthMHz)
          {
            case 20:
              os << "1000 bytes";
              break;
            case 10:
              os << "500 bytes";
              break;
            case 5:
              os << "250 bytes";
              break;
            default:
              os << "1000 bytes";
          }
        // 1 s / (10000 / nUes)
        MSG ("  Inter-packet interval (per UE)") << p.ueNumPergNb / 10.0 << " ms";
        break;

      case 1:
        MSG ("  Latency");
        MSG ("  Number of packets") << 1;
        MSG ("  Packet size") << "12 bytes";
        MSG ("  Inter-packet interval (per UE)") << "1 s";
        break;

      case 2:
        MSG ("  Moderate loading");
        MSG ("  Number of packets") << "infinite";
        MSG ("  Packet size");
        switch (p.bandwidthMHz)
          {
            case 20:
              os << "125 bytes";
              break;
            case 10:
              os << "63 bytes";
              break;
            case 5:
              os << "32 bytes";
              break;
            default:
              os << "125 bytes";
          }
        // 1 s / (1000 / nUes)
        MSG ("  Inter-packet interval (per UE)") << 1 / (1000 / p.ueNumPergNb) << " s";

        break;

      case 3:
        MSG ("  Moderate-high loading");
        MSG ("  Number of packets") << "infinite";
        MSG ("  Packet size");
        switch (p.bandwidthMHz)
          {
            case 20:
              os << "250 bytes";
              break;
            case 10:
              os << "125 bytes";
              break;
            case 5:
              os << "75 bytes";
              break;
            default:
              os << "250 bytes";
          }
        // 1 s / (10000 / nUes)
        MSG ("  Inter-packet interval (per UE)") << 1 / (10000.0 / p.ueNumPergNb) << " s";

        break;
      default:
        os << "\n  (Unknown configuration)";
    }

  MSG ("Application start window") << p.udpAppStartTime.As (Time::MS) << " + " << appStartWindow.As (Time::MS);
  MSG ("Application on duration") << p.appGenerationTime.As (Time::MS);
  MSG ("Traffic direction") << p.direction;

  MSG ("");
  MSG ("Output file name") << p.simTag;
  MSG ("Output directory") << p.outputDir;
  MSG ("Logging") << (p.logging ? "ON" : "off");
  MSG ("Trace file generation") << (p.traces ? "ON" : "off");
  MSG ("");
  MSG ("Radio environment map") << (p.dlRem ? "DL" : (p.ulRem ? "UL" : "off"));
  if (p.dlRem || p.ulRem)
    {
      MSG ("  Sector to sample");
      if (p.remSector == 0)
        {
          os << "all";
        }
      else
        {
          os << p.remSector;
        }
      MSG ("  X range") << p.xMinRem << " - " << p.xMaxRem << ", in " << p.xResRem << " m steps";
      MSG ("  Y range") << p.yMinRem << " - " << p.yMaxRem << ", in " << p.yResRem << " m steps";
      MSG ("  Altitude (Z)") << p.zRem << " m";
    }

  os << std::endl;
  return os;
}


} // namespace ns3


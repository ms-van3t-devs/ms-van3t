/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/nr-mac-scheduler-tdma-rr.h"
#include "ns3/nr-mac-scheduler-ofdma-rr.h"
#include "ns3/nr-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ideal-beamforming-algorithm.h"
#include "ns3/antenna-module.h"


/**
 * \file cttc-nr-notching.cc
 * \ingroup examples
 *
 * \brief Creates a configurable NR TDD/FDD deployment with up to 2 gNBs, for
 * testing a notching mask.
 *
 * This example is used to study notching. By default the notching mask (mask with
 * 0s and 1s, where 0s denote the RBs to be notched) is configured to all ones,
 * meaning that no notched RBs are defined.
 *
 * The user can select through the command line the number of RBs to be notched,
 * as well as the starting RB.
 * If for example we want to mute RBs from 12 to 17, then the starting RB has to
 * be set to 12 and the number of notched RBs to 6.
 * All gNBs then, will be assigned the same notched mask.
 * The user can see the impact of the mask in the user throughput. As the notched
 * resources increase, it can be seen how the throughput is decreased and vice versa.
 *
 * TDD or FDD mode can be selected through the command line, as well as TDMA/OFDMA
 * and DL or/and UL traffic.
 * In case of TDD only 1 BWP is created, while for the FDD case a pair of DL/UL
 * BWPs are created.
 *
 * Please notice that the size of the mask depends on the bandwidth (BW). In this
 * example we support only BWs of 5, 10 and 20 MHz. If a user wants to test different
 * BWs or different mask sizes, will have to modify the example accordingly.
 *
 * Also notice that the number of notched RBs must be choosen wisely. If for example
 * someone sets the number of notched RBs too high and executes the example for
 * a high number of UEs, then it will result in error since there will not be
 * sufficient resources for the UE transmissions. In such case, either reduce the
 * number of UEs or the number of notched RBs.
 *
 * The example will print on-screen the end-to-end result of DL (and/or UL) flows,
 * as well as writing them on a file.
 *
 * \code{.unparsed}
$ ./ns3 run "cttc-nr-notching --PrintHelp"
    \endcode
 */
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CttcNrNotching");

int
main (int argc, char *argv[])
{
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 2;

  uint8_t numBands = 1;
  double centralFrequencyBand = 2e9;

  double bandwidth = 10e6; //BW changes later on, depending on whether TDD or FDD is selected

  uint16_t numerology = 0; //Only numerology of 0 is supported in this example

  double totalTxPower = 43;

  std::string operationMode = "TDD";  // TDD or FDD
  std::string pattern = "DL|S|UL|UL|DL|DL|S|UL|UL|DL|";

  bool udpFullBuffer = false;
  uint32_t udpPacketSizeUll = 100;
  uint32_t udpPacketSizeBe = 1252;
  uint32_t lambdaUll = 10000;
  uint32_t lambdaBe = 1000;

  bool logging = false;

  bool enableDl = false;
  bool enableUl = true;

  bool enableOfdma = false;

  int notchedRbStartDl = 0;
  int numOfNotchedRbsDl = 0;

  int notchedRbStartUl = 0;
  int numOfNotchedRbsUl = 0;

  std::string simTag = "default";
  std::string outputDir = "./";

  double simTime = 1; // seconds
  double udpAppStartTime = 0.4; //seconds

  CommandLine cmd;

  cmd.AddValue ("simTime",
                "Simulation time",
                simTime);
  cmd.AddValue ("gNbNum",
                "The number of gNbs. In this example up to 2 gNBs are supported",
                gNbNum);
  cmd.AddValue ("ueNumPergNb",
                "The number of UEs per gNb",
                ueNumPergNb);
  cmd.AddValue ("bandwidth",
                "The system bandwidth. Choose among 5, 10 and 20 MHz",
                bandwidth);
  cmd.AddValue ("totalTxPower",
                "total tx power that will be proportionally assigned to"
                " bandwidth parts depending on each BWP bandwidth ",
                totalTxPower);
  cmd.AddValue ("operationMode",
                "The network operation mode can be TDD or FDD",
                operationMode);
  cmd.AddValue ("tddPattern",
                "LTE TDD pattern to use (e.g. --tddPattern=DL|S|UL|UL|UL|DL|S|UL|UL|UL|)",
                pattern);
  cmd.AddValue ("udpFullBuffer",
                "Whether to set the full buffer traffic; if this parameter is "
                "set then the udpInterval parameter will be neglected.",
                udpFullBuffer);
  cmd.AddValue ("packetSizeUll",
                "packet size in bytes to be used by ultra low latency traffic",
                udpPacketSizeUll);
  cmd.AddValue ("packetSizeBe",
                "packet size in bytes to be used by best effort traffic",
                udpPacketSizeBe);
  cmd.AddValue ("lambdaUll",
                "Number of UDP packets in one second for ultra low latency traffic",
                lambdaUll);
  cmd.AddValue ("lambdaBe",
                "Number of UDP packets in one second for best effor traffic",
                lambdaBe);
  cmd.AddValue ("logging",
                "Enable logging",
                logging);
  cmd.AddValue ("enableDl",
                "Enable DL flow",
                enableDl);
  cmd.AddValue ("enableUl",
                "Enable UL flow",
                enableUl);
  cmd.AddValue ("enableOfdma",
                "enable Ofdma scheduler. If false (default) Tdma is enabled",
                enableOfdma);
  cmd.AddValue ("notchedRbStartDl",
                "starting point of notched RBs (choose among RBs 0-52 for BW of 10 MHz)"
                "for the DL",
                notchedRbStartDl);
  cmd.AddValue ("numOfNotchedRbsDl",
                "Number of notched RBs for the DL. "
                "Please be sure that the number of 'normal' RBs is sufficient to "
                "perform transmissions of the UEs. If an error occurs, please try "
                "to reduce either the number of UEs or the number of the RBs notched.",
                numOfNotchedRbsDl);
  cmd.AddValue ("notchedRbStartUl",
                "starting point of notched RBs (choose among RBs 0-52 for BW of 10 MHz)"
                "for the UL",
                notchedRbStartUl);
  cmd.AddValue ("numOfNotchedRbsUl",
                "Number of notched RBs for the UL. "
                "Please be sure that the number of 'normal' RBs is sufficient to "
                "perform transmissions of the UEs. If an error occurs, please try "
                "to reduce either the number of UEs or the number of the RBs notched.",
                numOfNotchedRbsUl);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                outputDir);

  cmd.Parse (argc, argv);

  NS_ABORT_IF (numBands < 1);
  NS_ABORT_MSG_IF (enableDl == false && enableUl == false, "Enable at least one of "
                   "the flows (DL/UL)");
  NS_ABORT_MSG_IF (numerology != 0, "Only numerology 0 is supported in this example");

  //Set the size of the mask according to the supported BW
  int size = 0;
  if (bandwidth == 5e6)
    {
      size = 26;
      NS_ABORT_MSG_IF (notchedRbStartDl < 0 || notchedRbStartDl > 25, "The starting point "
                       "of the DL notched RBs must be between 0 and 25 for BW of 5MHz");
      NS_ABORT_MSG_IF ((notchedRbStartDl + numOfNotchedRbsDl) > 25, "The available RBs "
                       "in DL are 26 (from 0 to 25) for BW of 5MHz");
      NS_ABORT_MSG_IF (notchedRbStartUl < 0 || notchedRbStartUl > 25, "The starting point "
                       "of the UL notched RBs must be between 0 and 25 for BW of 5MHz");
      NS_ABORT_MSG_IF ((notchedRbStartUl + numOfNotchedRbsUl) > 25, "The available RBs "
                       "in UL are 26 (from 0 to 25) for BW of 5MHz");
    }
  else if (bandwidth == 10e6)
    {
      size = 53;
      NS_ABORT_MSG_IF (notchedRbStartDl < 0 || notchedRbStartDl > 52, "The starting point "
                       "of the DL notched RBs must be between 0 and 52 for BW of 10MHz");
      NS_ABORT_MSG_IF ((notchedRbStartDl + numOfNotchedRbsDl) > 52, "The available RBs "
                       "in DL are 53 (from 0 to 52) for BW of 10MHz");
      NS_ABORT_MSG_IF (notchedRbStartUl < 0 || notchedRbStartUl > 52, "The starting point "
                       "of the UL notched RBs must be between 0 and 52 for BW of 10MHz");
      NS_ABORT_MSG_IF ((notchedRbStartUl + numOfNotchedRbsUl) > 52, "The available RBs "
                       "in UL are 53 (from 0 to 52) for BW of 10MHz");
    }
  else if (bandwidth == 20e6)
    {
      size = 106;
      NS_ABORT_MSG_IF (notchedRbStartDl < 0 || notchedRbStartDl > 105, "The starting point "
                       "of the DL notched RBs must be between 0 and 105 for BW of 20MHz");
      NS_ABORT_MSG_IF ((notchedRbStartDl + numOfNotchedRbsDl) > 105, "The available RBs "
                       "in DL are 106 (from 0 to 105) for BW of 20MHz");
      NS_ABORT_MSG_IF (notchedRbStartUl < 0 || notchedRbStartUl > 105, "The starting point "
                       "of the UL notched RBs must be between 0 and 105 for BW of 20MHz");
      NS_ABORT_MSG_IF ((notchedRbStartUl + numOfNotchedRbsUl) > 105, "The available RBs "
                       "in UL are 106 (from 0 to 105) for BW of 20MHz");
    }
  else
    {
      NS_ABORT_MSG ("This bandwidth is not supported in this example. "
                    "Please choose among 5MHz - 10MHz - 20MHz.");
    }

  // Default mask (all 1s)
  std::vector<uint8_t> notchedMaskDl (size, 1);
  std::vector<uint8_t> notchedMaskUl (size, 1);

  //mute RBs from notchedRbStart to (notchedRbStart + numOfNotchedRbs)
  for (int i = notchedRbStartDl; i < (notchedRbStartDl + numOfNotchedRbsDl); i++)
    {
      notchedMaskDl[i] = 0;
    }

  for (int i = notchedRbStartUl; i < (notchedRbStartUl + numOfNotchedRbsUl); i++)
    {
      notchedMaskUl[i] = 0;
    }

  std::cout << "DL notched Mask: ";
  for (int x : notchedMaskDl)
    {
      std::cout << x << " ";
    }
  std::cout << std::endl;
  std::cout << "UL notched Mask: ";
  for (int x : notchedMaskUl)
    {
      std::cout << x << " ";
    }
  std::cout << std::endl;
  std::cout << "Warning: Please be sure that the number of 'normal' RBs is " <<
    "sufficient to perform transmissions of the UEs.\n If an error " <<
    "occurs, please try to reduce either the number of notched RBs " <<
    "or the number of UEs." << std::endl;

  // enable logging or not
  if (logging)
    {
      LogLevel logLevel1 = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME |
                                      LOG_PREFIX_NODE | LOG_LEVEL_INFO);
      LogLevel logLevel2 = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME |
                                      LOG_PREFIX_NODE | LOG_LEVEL_DEBUG);
      LogComponentEnable ("NrMacSchedulerNs3", logLevel1);
      LogComponentEnable ("NrMacSchedulerTdma", logLevel1);
      //LogComponentEnable ("NrMacSchedulerTdmaRR", logLevel1);
      LogComponentEnable ("NrMacSchedulerOfdma", logLevel1);
      //LogComponentEnable ("NrMacSchedulerOfdmaRR", logLevel1);
      LogComponentEnable ("CcBwpHelper", logLevel2);
    }

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

  int64_t randomStream = 1;

  GridScenarioHelper gridScenario;
  gridScenario.SetRows (1);
  gridScenario.SetColumns (gNbNum);
  gridScenario.SetHorizontalBsDistance (5.0);
  gridScenario.SetVerticalBsDistance (5.0);
  gridScenario.SetBsHeight (1.5);
  gridScenario.SetUtHeight (1.5);
  gridScenario.SetSectorization (GridScenarioHelper::SINGLE);
  gridScenario.SetBsNumber (gNbNum);
  gridScenario.SetUtNumber (ueNumPergNb * gNbNum);
  gridScenario.SetScenarioHeight (3); // Create a 3x3 scenario where the UE will
  gridScenario.SetScenarioLength (3); // be distribuited.
  randomStream += gridScenario.AssignStreams (randomStream);
  gridScenario.CreateScenario ();

  // setup the nr simulation
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper> ();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));
  if (enableOfdma)
    {
      nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaRR"));
    }
  else
    {
      nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerTdmaRR"));
    }

  // Beamforming method
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  /*
   * Setup the configuration of the spectrum. One operation band is deployed
   * with 1 component carrier (CC), automatically generated by the ccBwpManager
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;

  OperationBandInfo band;

  /*
   * The configured spectrum division for TDD is:
   *
   * |----Band1----|
   * |-----CC1-----|
   * |-----BWP1----|
   *
   * And the configured spectrum division for FDD operation is:
   * |---------Band1---------|
   * |----------CC1----------|
   * |----BWP1---|----BWP2---|
   */

  const uint8_t numOfCcs = 1;

  if (operationMode == "FDD")
    {
      //For FDD we have 2 BWPs so the BW must be doubled (e.g. for BW of 10MHz we
      //need 20MHz --> 10MHz for the DL BWP and 10MHz for the UL BWP)
      bandwidth = bandwidth * 2;
    }

  // Create the configuration for the CcBwpHelper
  CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequencyBand, bandwidth,
                                                  numOfCcs, BandwidthPartInfo::UMi_StreetCanyon_LoS);

  bandConf.m_numBwp = operationMode == "FDD" ? 2 : 1; // FDD will have 2 BWPs per CC

  // By using the configuration created, it is time to make the operation band
  band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

  nrHelper->InitializeOperationBand (&band);
  allBwps = CcBwpCreator::GetAllBwps ({band});

  double x = pow (10, totalTxPower / 10);

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));


  uint32_t bwpIdForLowLat = 0;
  uint32_t bwpIdForVideo = 0;

  // gNb and UE routing between Bearer and bandwidh part
  if (operationMode == "TDD")
    {
      nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
      nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_DEFAULT", UintegerValue (bwpIdForVideo));
    }
  else
    {
      bwpIdForVideo = 1;
      nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
      nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_DEFAULT", UintegerValue (bwpIdForVideo));

      nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
      nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_DEFAULT", UintegerValue (bwpIdForVideo));
    }

  //Install and get the pointers to the NetDevices
  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gridScenario.GetBaseStations (), allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (gridScenario.GetUserTerminals (), allBwps);

  randomStream += nrHelper->AssignStreams (enbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);


  for (uint32_t i = 0; i < gNbNum; ++i)
    {
      // Manually set the attribute of the netdevice (enbNetDev.Get (0)) and bandwidth part (0), (1), ...
      nrHelper->GetGnbPhy (enbNetDev.Get (i), 0)->SetAttribute ("Numerology", UintegerValue (numerology));
      nrHelper->GetGnbPhy (enbNetDev.Get (i), 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (x)));

      //Set the mask
      Ptr<NrMacSchedulerNs3> schedulerBwp1 = DynamicCast<NrMacSchedulerNs3> (nrHelper->GetScheduler (enbNetDev.Get (i), 0));
      schedulerBwp1->SetDlNotchedRbgMask (notchedMaskDl);

      if (operationMode == "TDD")
        {
          nrHelper->GetGnbPhy (enbNetDev.Get (i), 0)->SetAttribute ("Pattern", StringValue (pattern));
          schedulerBwp1->SetUlNotchedRbgMask (notchedMaskUl);
        }
      else
        {
          nrHelper->GetGnbPhy (enbNetDev.Get (i), 0)->SetAttribute ("Pattern", StringValue ("DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|"));

          nrHelper->GetGnbPhy (enbNetDev.Get (i), 1)->SetAttribute ("Numerology", UintegerValue (numerology));
          nrHelper->GetGnbPhy (enbNetDev.Get (i), 1)->SetAttribute ("TxPower", DoubleValue (-30.0));
          nrHelper->GetGnbPhy (enbNetDev.Get (i), 1)->SetAttribute ("Pattern", StringValue ("UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|"));

          Ptr<NrMacSchedulerNs3> schedulerBwp2 = DynamicCast<NrMacSchedulerNs3> (nrHelper->GetScheduler (enbNetDev.Get (i), 1));
          schedulerBwp2->SetUlNotchedRbgMask (notchedMaskUl);

          // Link the two FDD BWPs:
          nrHelper->GetBwpManagerGnb (enbNetDev.Get (i))->SetOutputLink (1, 0);
        }
    }

  if (operationMode == "FDD")
    {
      // Set the UE routing:
      for (uint32_t i = 0; i < ueNetDev.GetN (); i++)
        {
          nrHelper->GetBwpManagerUe (ueNetDev.Get (i))->SetOutputLink (0, 1);
        }
    }

  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
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
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (gridScenario.GetUserTerminals ());
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < gridScenario.GetUserTerminals ().GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting =
        ipv4RoutingHelper.GetStaticRouting (gridScenario.GetUserTerminals ().Get (j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Attach to GNB
  nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  // install UDP applications
  uint16_t dlPortLowLat = 1234;
  uint16_t ulPortVoice = 1235;

  ApplicationContainer serverApps;

  // The sink will always listen to the specified ports
  UdpServerHelper dlPacketSinkLowLat (dlPortLowLat);
  UdpServerHelper ulPacketSinkVoice (ulPortVoice);

  // The server, that is the application which is listening, is installed in the UE
  // for the DL traffic, and in the remote host for the UL traffic
  serverApps.Add (dlPacketSinkLowLat.Install (gridScenario.GetUserTerminals ()));
  serverApps.Add (ulPacketSinkVoice.Install (remoteHost));

  /*
   * Configure attributes for the different generators, using user-provided
   * parameters for generating a CBR traffic
   *
   * Low-Latency configuration and object creation:
   */
  UdpClientHelper dlClientLowLat;
  dlClientLowLat.SetAttribute ("RemotePort", UintegerValue (dlPortLowLat));
  dlClientLowLat.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  dlClientLowLat.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
  dlClientLowLat.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaBe)));

  // The bearer that will carry low latency traffic
  EpsBearer lowLatBearer (EpsBearer::NGBR_LOW_LAT_EMBB);

  // The filter for the low-latency traffic
  Ptr<EpcTft> lowLatTft = Create<EpcTft> ();
  EpcTft::PacketFilter dlpfLowLat;
  dlpfLowLat.localPortStart = dlPortLowLat;
  dlpfLowLat.localPortEnd = dlPortLowLat;
  lowLatTft->Add (dlpfLowLat);

  // Voice configuration and object creation:
  UdpClientHelper ulClientVoice;
  ulClientVoice.SetAttribute ("RemotePort", UintegerValue (ulPortVoice));
  ulClientVoice.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  ulClientVoice.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
  ulClientVoice.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaBe)));

  // The bearer that will carry voice traffic
  EpsBearer videoBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT);

  // The filter for the voice traffic
  Ptr<EpcTft> voiceTft = Create<EpcTft> ();
  EpcTft::PacketFilter ulpfVoice;
  ulpfVoice.remotePortStart = ulPortVoice;
  ulpfVoice.remotePortEnd = ulPortVoice;
  ulpfVoice.direction = EpcTft::UPLINK;
  voiceTft->Add (ulpfVoice);

  //  Install the applications
  ApplicationContainer clientApps;

  for (uint32_t i = 0; i < gridScenario.GetUserTerminals ().GetN (); ++i)
    {
      Ptr<Node> ue = gridScenario.GetUserTerminals ().Get (i);
      Ptr<NetDevice> ueDevice = ueNetDev.Get (i);
      Address ueAddress = ueIpIface.GetAddress (i);

      // The client, who is transmitting, is installed in the remote host,
      // with destination address set to the address of the UE
      if (enableDl)
        {
          dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
          clientApps.Add (dlClientLowLat.Install (remoteHost));

          nrHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
        }
      // For the uplink, the installation happens in the UE, and the remote address
      // is the one of the remote host
      if (enableUl)
        {
          ulClientVoice.SetAttribute ("RemoteAddress", AddressValue (internetIpIfaces.GetAddress (1)));
          clientApps.Add (ulClientVoice.Install (ue));

          nrHelper->ActivateDedicatedEpsBearer (ueDevice, videoBearer, voiceTft);
        }
    }

  // start UDP server and client apps
  serverApps.Start (Seconds (udpAppStartTime));
  clientApps.Start (Seconds (udpAppStartTime));
  serverApps.Stop (Seconds (simTime));
  clientApps.Stop (Seconds (simTime));

  // enable the traces provided by the nr module
  nrHelper->EnableTraces ();


  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (gridScenario.GetUserTerminals ());

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  /*
   * To check what was installed in the memory, i.e., BWPs of eNb Device, and its configuration.
   * Example is: Node 1 -> Device 0 -> BandwidthPartMap -> {0,1} BWPs -> NrGnbPhy -> NrPhyMacCommong-> Numerology, Bandwidth, ...
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
      outFile << "  TxOffered:  " << i->second.txBytes * 8.0 / (simTime - udpAppStartTime) / 1000 / 1000  << " Mbps\n";
      outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      if (i->second.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          //double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
          double rxDuration = (simTime - udpAppStartTime);

          averageFlowThroughput += i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000;
          averageFlowDelay += 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;

          outFile << "  Throughput: " << i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000  << " Mbps\n";
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



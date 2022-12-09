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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/nr-module.h"
#include <chrono>
#include <ns3/antenna-module.h>

/**
 * \file cttc-error-model.cc
 * \ingroup examples
 * \brief Error model example with fixed MCS: 1 gNB and 1 UE, multiple packets with varying fading conditions.
 *
 * This example allows the user to test the end-to-end performance with the new
 * NR PHY abstraction model for error modeling by using a fixed MCS. It allows the user to set the MCS, the
 * gNB-UE distance, the MCS table, the error model type, and the HARQ method.
 *
 * The NR error model can be set as "--errorModel=ns3::NrEesmCcT1", for HARQ-CC and MCS Table1,
 * while "--errorModel=ns3::NrLteMiErrorModel" configures the LTE error model.
 * For NR, you can choose between different types of error model, which use
 * different tables and different methods to process the HARQ history, e.g.,
 * "--errorModel=ns3::NrEesmIrT1", for HARQ-IR and MCS Table2.
 * You can fix also the MCS index to use with "--mcs=7" (7 in this case), which refers
 * to the configured MCS table.
 *
 * The scenario consists of a single gNB and a single UE, placed at positions (0.0, 0.0, 10), and
 * (0.0, ueY, 1.5), respectively. ueY can be configured by the user, e.g. "ueY=20", and defaults
 * to 30 m.
 *
 * By default, the program uses the 3GPP channel model, Urban Micro scenario, without shadowing and with
 * probabilistic line of sight / non-line of sight option. The program runs for
 * 50 seconds and one packet is transmitted every 200 ms from gNB to UE (donwlink direction).
 * The packet size can be configured by using the following parameter: "--packetSize=1000".
 * The channel update period is 150 ms, so that every packet encounters a different
 * fading condition.
 *
 * This simulation prints the output to the terminal. The output statistics are
 * averaged among all the transmitted packets.
 *
 * To run the simulation with the default configuration one shall run the
 * following in the command line:
 *
 * ./ns3 run cttc-error-model
 *
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CttcErrorModelExample");


static Ptr<ListPositionAllocator>
GetGnbPositions (double gNbHeight = 10.0)
{
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator> ();
  pos->Add (Vector (0.0, 0.0, gNbHeight));

  return pos;
}

static Ptr<ListPositionAllocator>
GetUePositions (double ueY, double ueHeight = 1.5)
{
  Ptr<ListPositionAllocator> pos = CreateObject<ListPositionAllocator> ();
  pos->Add (Vector (0.0, ueY, ueHeight));

  return pos;
}

static std::vector<uint64_t> packetsTime;

static void
PrintRxPkt ([[maybe_unused]] std::string context, Ptr<const Packet> pkt)
{
  // ASSUMING ONE UE
  SeqTsHeader seqTs;
  pkt->PeekHeader (seqTs);
  packetsTime.push_back ((Simulator::Now () - seqTs.GetTs ()).GetMicroSeconds ());
}

int
main (int argc, char *argv[])
{
  uint32_t mcs = 13;
  const uint8_t gNbNum = 1;
  const uint8_t ueNum = 1;
  double totalTxPower = 4;
  uint16_t numerologyBwp = 4;
  double centralFrequencyBand = 28e9;
  double bandwidthBand = 100e6;
  double ueY = 30.0;

  double simTime = 10.0; // 50 seconds: to take statistics
  uint32_t pktSize = 500;
  Time udpAppStartTime = MilliSeconds (1000);
  Time packetInterval = MilliSeconds (200);
  Time updateChannelInterval = MilliSeconds (150);
  bool isUl = false;

  std::string errorModel = "ns3::NrEesmCcT1";

  CommandLine cmd;

  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue ("mcs",
                "The MCS that will be used in this example",
                mcs);
  cmd.AddValue ("errorModelType",
                "Error model type: ns3::NrEesmCcT1, ns3::NrEesmCcT2, ns3::NrEesmIrT1, ns3::NrEesmIrT2, ns3::NrLteMiErrorModel",
                errorModel);
  cmd.AddValue ("ueY",
                "Y position of any UE",
                ueY);
  cmd.AddValue ("pktSize",
                "Packet Size",
                pktSize);
  cmd.AddValue ("isUl",
                "Is this an UL transmission?",
                isUl);

  cmd.Parse (argc, argv);

  uint32_t packets = (simTime - udpAppStartTime.GetSeconds ()) / packetInterval.GetSeconds ();
  NS_ABORT_IF (packets == 0);

  /*
   * Default values for the simulation. We are progressively removing all
   * the instances of SetDefault, but we need it for legacy code (LTE)
   */
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize",
                      UintegerValue (999999999));

  Config::SetDefault ("ns3::NrAmc::ErrorModelType", TypeIdValue (TypeId::LookupByName (errorModel)));
  Config::SetDefault ("ns3::NrAmc::AmcModel", EnumValue (NrAmc::ShannonModel));  // NOT USED in this example. MCS is fixed.

  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 10.0;
  double ueHeight = 1.5;

  gNbNodes.Create (gNbNum);
  ueNodes.Create (ueNum);

  Ptr<ListPositionAllocator> gNbPositionAlloc = GetGnbPositions (gNbHeight);
  Ptr<ListPositionAllocator> uePositionAlloc = GetUePositions (ueY, ueHeight);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (gNbPositionAlloc);
  mobility.Install (gNbNodes);

  mobility.SetPositionAllocator (uePositionAlloc);
  mobility.Install (ueNodes);

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
   * Spectrum division. We create one operational band, with one CC, and the CC with a single bandwidth part.
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;

  CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequencyBand, bandwidthBand, numCcPerBand, BandwidthPartInfo::UMi_StreetCanyon);
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

  /*
   * Attributes of ThreeGppChannelModel still cannot be set in our way.
   * TODO: Coordinate with Tommaso
   */
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (updateChannelInterval));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  /*
   * Initialize channel and pathloss, plus other things inside band.
   */
  nrHelper->InitializeOperationBand (&band);
  allBwps = CcBwpCreator::GetAllBwps ({band});

  Packet::EnableChecking ();
  Packet::EnablePrinting ();

  /*
   *  Case (i): Attributes valid for all the nodes
   */
  // Beamforming method
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  // Core latency
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Scheduler
  nrHelper->SetSchedulerAttribute ("FixedMcsDl", BooleanValue (true));
  nrHelper->SetSchedulerAttribute ("FixedMcsUl", BooleanValue (true));
  nrHelper->SetSchedulerAttribute ("StartingMcsDl", UintegerValue (mcs));
  nrHelper->SetSchedulerAttribute ("StartingMcsUl", UintegerValue (mcs));

  // Error Model: UE and GNB with same spectrum error model.
  nrHelper->SetUlErrorModel (errorModel);
  nrHelper->SetDlErrorModel (errorModel);

  // Both DL and UL AMC will have the same model behind.
  // Note: NOT USED in this example. MCS is fixed.
  nrHelper->SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc::ShannonModel));
  nrHelper->SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc::ShannonModel));

  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (totalTxPower));

  uint32_t bwpId = 0;

  // gNb routing between Bearer and bandwidh part
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpId));

  // Ue routing between Bearer and bandwidth part
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpId));

  NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice (gNbNodes, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);

  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (gnbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);

  /*
   * Case (iii): Go node for node and change the attributes we have to setup
   * per-node.
   */

  // Get the first netdevice (enbNetDev.Get (0)) and the first bandwidth part (0)
  // and set the attribute.
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp));
  nrHelper->GetGnbPhy (gnbNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue (totalTxPower));

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
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  UdpServerHelper dlPacketSinkHelper (dlPort);
  ApplicationContainer txApps, sinkApps;
  NodeContainer txNodes, sinkNodes;
  Ipv4InterfaceContainer sinkIps;

  if (isUl)
    {
      sinkIps.Add (internetIpIfaces.Get (1));
      sinkNodes = remoteHostContainer;
      txNodes = ueNodes;
    }
  else
    {
      sinkIps = ueIpIface;
      sinkNodes = ueNodes;
      txNodes = remoteHostContainer;
    }

  // configure here UDP traffic
  for (uint32_t i = 0; i < txNodes.GetN (); ++i)
    {
      for (uint32_t j = 0; j < sinkNodes.GetN (); ++j)
        {
          UdpClientHelper dlClient (sinkIps.GetAddress (j), dlPort);
          dlClient.SetAttribute ("MaxPackets", UintegerValue (packets));
          dlClient.SetAttribute ("PacketSize", UintegerValue (pktSize));
          dlClient.SetAttribute ("Interval", TimeValue (packetInterval));

          txApps.Add (dlClient.Install (txNodes.Get (i)));
        }
    }

  sinkApps.Add (dlPacketSinkHelper.Install (sinkNodes));
  for (uint32_t j = 0; j < sinkApps.GetN (); ++j)
    {
      Ptr<UdpServer> client = DynamicCast<UdpServer> (sinkApps.Get (j));
      NS_ASSERT (client != nullptr);
      std::stringstream ss;
      ss << j;
      client->TraceConnect ("Rx", ss.str (), MakeCallback (&PrintRxPkt));
    }

  // start UDP server and client apps
  sinkApps.Start (udpAppStartTime);
  txApps.Start (udpAppStartTime);
  sinkApps.Stop (Seconds (simTime));
  txApps.Stop (Seconds (simTime));

  // attach UEs to the closest eNB
  nrHelper->AttachToClosestEnb (ueNetDev, gnbNetDev);

  // enable the traces provided by the nr module
  //nrHelper->EnableTraces();

  Simulator::Stop (Seconds (simTime));

  auto start = std::chrono::steady_clock::now ();

  Simulator::Run ();

  auto end = std::chrono::steady_clock::now ();


  uint64_t sum = 0;
  uint32_t cont = 0;
  for (auto & v : packetsTime)
    {
      if ( v < 100000 )
        {
          sum += v;
          cont++;
        }
    }
  std::cout << "Packets received: " << packetsTime.size () << std::endl;
  std::cout << "Counter (packets not affected by reordering): " << +cont << std::endl;

  if (packetsTime.size () > 0 && cont > 0)
    {
      std::cout << "Average e2e latency (over all received packets): " << sum / packetsTime.size () << " us" << std::endl;
      std::cout << "Average e2e latency (over counter): " << sum / cont << " us" << std::endl;
    }
  else
    {
      std::cout << "Average e2e latency: Not Available" << std::endl;
    }


  for (auto it = sinkApps.Begin (); it != sinkApps.End (); ++it)
    {
      uint64_t recv = DynamicCast<UdpServer> (*it)->GetReceived ();
      std::cout << "Sent: " << packets << " Recv: " << recv << " Lost: "
                << packets - recv << " pkts, ( "
                << (static_cast<double> (packets - recv) / packets) * 100.0
                << " % )" << std::endl;
    }


  Simulator::Destroy ();

  std::cout << "Running time: " << std::chrono::duration_cast<std::chrono::seconds>(end - start).count()
            << " s." << std::endl;
  return 0;
}



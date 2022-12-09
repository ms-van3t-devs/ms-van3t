/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#include "system-scheduler-test.h"
#include <ns3/packet.h>
#include <ns3/simulator.h>
#include <ns3/nr-module.h>
#include <ns3/internet-module.h>
#include <ns3/applications-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/antenna-module.h>

namespace ns3 {

void
SystemSchedulerTest::CountPkts ([[maybe_unused]] Ptr<const Packet> pkt)
{
  m_packets++;
  if (m_packets == m_limit)
    {
      Simulator::Stop ();
    }
}

SystemSchedulerTest::SystemSchedulerTest (const std::string & name, uint32_t usersPerBeamNum,
                                          uint32_t beamsNum, uint32_t numerology,
                                          double bw1, bool isDownlnk, bool isUplink,
                                          const std::string & schedulerType)
  : TestCase (name)
{
  m_numerology = numerology;
  m_bw1 = bw1;
  m_isDownlink = isDownlnk;
  m_isUplink = isUplink;
  m_usersPerBeamNum = usersPerBeamNum;
  NS_ABORT_MSG_UNLESS (beamsNum <= 4, "Test program is designed to support up to 4 beams per gNB" );
  m_beamsNum = beamsNum;
  m_schedulerType = schedulerType;
  m_name = name;
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
SystemSchedulerTest::~SystemSchedulerTest ()
{}


void
SystemSchedulerTest::DoRun (void)
{
  NS_ABORT_IF (!m_isUplink && !m_isDownlink);

  // set simulation time and mobility
  Time simTime = MilliSeconds (1500);
  Time udpAppStartTimeDl = MilliSeconds (500);
  Time udpAppStartTimeUl = MilliSeconds (500);
  Time udpAppStopTimeDl = MilliSeconds (1500);   // Let's give 1s to end the tx
  Time udpAppStopTimeUl = MilliSeconds (1500);   // Let's give 1 to end the tx
  uint16_t gNbNum = 1;
  uint32_t packetSize = 100;
  uint32_t maxPackets = 400;
  DataRate udpRate = DataRate ("320kbps");   // 400 packets of 800 bits

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));
  Config::SetDefault ("ns3::LteRlcUm::ReorderingTimer", TimeValue (Seconds (1)));
  Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));

  Config::SetDefault ("ns3::NrUePhy::EnableUplinkPowerControl", BooleanValue (false));   // scheduler tests are designed to expect the maximum transmit power, maximum MCS,
                                                                                         // thus uplink power control is not compatible, because it will adjust

  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 10;
  double ueHeight = 1.5;
  gNbNodes.Create (gNbNum);
  ueNodes.Create (m_usersPerBeamNum * m_beamsNum * gNbNum);

  Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> staPositionAlloc = CreateObject<ListPositionAllocator> ();

  double gNbx = 0;
  double gNby = 0;

  for (uint32_t gNb = 0; gNb < gNbNum; gNb++)
    {
      apPositionAlloc->Add (Vector (gNbx, gNby, gNbHeight));

      for (uint32_t beam = 1; beam <= m_beamsNum; beam++)
        {
          for (uint32_t uePerBeamIndex = 0; uePerBeamIndex < m_usersPerBeamNum; uePerBeamIndex++)
            {
              if (beam == 1)
                {
                  staPositionAlloc->Add (Vector (gNbx + 1 + 0.1 * uePerBeamIndex, gNby + 10 + 0.1 * uePerBeamIndex, ueHeight));
                }
              else if (beam == 2)
                {
                  staPositionAlloc->Add (Vector (gNbx + 10 + 0.1 * uePerBeamIndex, gNby - 1  + 0.1 * uePerBeamIndex, ueHeight));
                }
              else if (beam == 3)
                {
                  staPositionAlloc->Add (Vector (gNbx - 1 + 0.1 * uePerBeamIndex, gNby - 10  + 0.1 * uePerBeamIndex, ueHeight));
                }
              else if (beam == 4 )
                {
                  staPositionAlloc->Add (Vector (gNbx - 10  + 0.1 * uePerBeamIndex, gNby + 1 + 0.1 * uePerBeamIndex, ueHeight));
                }
            }

        }

      // position of next gNB and its UE is shiftened for 20, 20
      gNbx += 1;
      gNby += 1;
    }
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (apPositionAlloc);
  mobility.Install (gNbNodes);
  mobility.SetPositionAllocator (staPositionAlloc);
  mobility.Install (ueNodes);


  // setup the mmWave simulation
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();

  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (CellScanBeamforming::GetTypeId ()));
  idealBeamformingHelper->SetBeamformingAlgorithmAttribute ("BeamSearchAngleStep", DoubleValue (10.0));

  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);

  // set the number of antenna elements of UE
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // UE transmit power
  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (20.0));

  // set the number of antenna elements of gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));

  // gNB transmit power
  nrHelper->SetGnbPhyAttribute ("TxPower", DoubleValue (44.0));

  // gNB numerology
  nrHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (m_numerology));

  // Set the scheduler type
  nrHelper->SetSchedulerTypeId (TypeId::LookupByName (m_schedulerType));
  Config::SetDefault ("ns3::NrAmc::ErrorModelType", TypeIdValue (TypeId::LookupByName ("ns3::NrEesmCcT1")));
  nrHelper->SetSchedulerAttribute ("FixedMcsDl", BooleanValue (true));
  nrHelper->SetSchedulerAttribute ("FixedMcsUl", BooleanValue (true));
  nrHelper->SetSchedulerAttribute ("StartingMcsDl", UintegerValue (28));
  nrHelper->SetSchedulerAttribute ("StartingMcsUl", UintegerValue (28));


  nrHelper->SetEpcHelper (epcHelper);

  /*
  * Spectrum division. We create two operational bands, each of them containing
  * one component carrier, and each CC containing a single bandwidth part
  * centered at the frequency specified by the input parameters.
  * Each spectrum part length is, as well, specified by the input parameters.
  * Both operational bands will use the StreetCanyon channel modeling.
  */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  double centralFrequency = 28e9;
  double bandwidth = m_bw1;
  const uint8_t numCcPerBand = 1;
  BandwidthPartInfo::Scenario scenario = BandwidthPartInfo::UMi_StreetCanyon_LoS;
  CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequency, bandwidth, numCcPerBand, scenario);

  // By using the configuration created, it is time to make the operation bands
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod", TimeValue (MilliSeconds (0)));

  // Shadowing
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  /*
   * Initialize channel and pathloss, plus other things inside band1. If needed,
   * the band configuration can be done manually, but we leave it for more
   * sophisticated examples. For the moment, this method will take care
   * of all the spectrum initialization needs.
   */
  nrHelper->InitializeOperationBand (&band);
  allBwps = CcBwpCreator::GetAllBwps ({band});


  uint32_t bwpIdForLowLat = 0;
  // gNb routing between Bearer and bandwidh part
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  // UE routing between Bearer and bandwidh part
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));


  // install mmWave net devices
  NetDeviceContainer gNbNetDevs = nrHelper->InstallGnbDevice (gNbNodes, allBwps);
  NetDeviceContainer ueNetDevs = nrHelper->InstallUeDevice (ueNodes, allBwps);

  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (gNbNetDevs, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDevs, randomStream);

  for (auto it = gNbNetDevs.Begin (); it != gNbNetDevs.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDevs.Begin (); it != ueNetDevs.End (); ++it)
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
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // in this container, interface 0 is the pgw, 1 is the remoteHost
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDevs));


  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // attach UEs to the closest eNB
  nrHelper->AttachToClosestEnb (ueNetDevs, gNbNetDevs);


  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  ApplicationContainer clientAppsDl;
  ApplicationContainer serverAppsDl;
  ApplicationContainer clientAppsUl;
  ApplicationContainer serverAppsUl;
//    ObjectMapValue objectMapValue;

  Time udpInterval = NanoSeconds (1);

  if (m_isUplink)
    {
      UdpServerHelper ulPacketSinkHelper (ulPort);
      serverAppsUl.Add (ulPacketSinkHelper.Install (remoteHost));

      // configure here UDP traffic flows
      for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
        {

          UdpClientHelper ulClient (remoteHostAddr, ulPort);
          ulClient.SetAttribute ("MaxPackets", UintegerValue (maxPackets));
          ulClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
          ulClient.SetAttribute ("Interval", TimeValue (udpInterval));   // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
          clientAppsUl.Add (ulClient.Install (ueNodes.Get (j)));

          Ptr<EpcTft> tft = Create<EpcTft> ();
          EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          ulpf.direction = EpcTft::UPLINK;
          tft->Add (ulpf);

          EpsBearer bearer (EpsBearer::NGBR_LOW_LAT_EMBB);
          nrHelper->ActivateDedicatedEpsBearer (ueNetDevs.Get (j), bearer, tft);
        }

      serverAppsUl.Start (udpAppStartTimeUl);
      clientAppsUl.Start (udpAppStartTimeUl);
      serverAppsUl.Stop (udpAppStopTimeUl);
      clientAppsUl.Stop (udpAppStopTimeUl);
    }


  if (m_isDownlink)
    {
      UdpServerHelper dlPacketSinkHelper (dlPort);
      serverAppsDl.Add (dlPacketSinkHelper.Install (ueNodes));

      // configure here UDP traffic flows
      for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
        {
          UdpClientHelper dlClient (ueIpIface.GetAddress (j), dlPort);
          dlClient.SetAttribute ("MaxPackets", UintegerValue (maxPackets));
          dlClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
          dlClient.SetAttribute ("Interval", TimeValue (udpInterval));   // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
          clientAppsDl.Add (dlClient.Install (remoteHost));

          Ptr<EpcTft> tft = Create<EpcTft> ();
          EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          dlpf.direction = EpcTft::DOWNLINK;
          tft->Add (dlpf);

          EpsBearer bearer (EpsBearer::NGBR_LOW_LAT_EMBB);
          nrHelper->ActivateDedicatedEpsBearer (ueNetDevs.Get (j), bearer, tft);
        }
      // start UDP server and client apps
      serverAppsDl.Start (udpAppStartTimeDl);
      clientAppsDl.Start (udpAppStartTimeDl);
      serverAppsDl.Stop (udpAppStopTimeDl);
      clientAppsDl.Stop (udpAppStopTimeDl);
    }

  m_limit = ueNodes.GetN () * maxPackets * ((m_isUplink && m_isDownlink) ? 2 : 1);

  for (auto it = serverAppsDl.Begin (); it != serverAppsDl.End (); ++it)
    {
      (*it)->TraceConnectWithoutContext ("Rx", MakeCallback (&SystemSchedulerTest::CountPkts, this));
    }

  for (auto it = serverAppsUl.Begin (); it != serverAppsUl.End (); ++it)
    {
      (*it)->TraceConnectWithoutContext ("Rx", MakeCallback (&SystemSchedulerTest::CountPkts, this));
    }

  //nrHelper->EnableTraces();
  Simulator::Stop (simTime);
  Simulator::Run ();

  double dataRecvDl = 0;
  double dataRecvUl = 0;

  if (m_isDownlink)
    {
      for ( uint32_t i = 0; i < serverAppsDl.GetN (); i++)
        {
          Ptr<UdpServer> serverApp = serverAppsDl.Get (i)->GetObject<UdpServer> ();
          double data = (serverApp->GetReceived () * packetSize * 8);
          dataRecvDl += data;
        }
    }
  if (m_isUplink)
    {
      for ( uint32_t i = 0; i < serverAppsUl.GetN (); i++)
        {
          Ptr<UdpServer> serverApp = serverAppsUl.Get (i)->GetObject<UdpServer>();
          double data = (serverApp->GetReceived () * packetSize * 8);
          dataRecvUl += data;
        }
    }

  double expectedBitRate = udpRate.GetBitRate () * ueNodes.GetN () * ((m_isUplink && m_isDownlink) ? 2 : 1);
  NS_TEST_ASSERT_MSG_EQ_TOL (dataRecvDl + dataRecvUl, expectedBitRate, expectedBitRate * 0.05, "Wrong total DL + UL throughput");

  Simulator::Destroy ();
}

} // namespace ns3

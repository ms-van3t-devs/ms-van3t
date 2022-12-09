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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/nr-module.h"
#include "ns3/eps-bearer-tag.h"
#include "ns3/three-gpp-channel-model.h"
#include "ns3/antenna-module.h"
//#include "ns3/component-carrier-gnb.h"
//#include "ns3/component-carrier-nr-ue.h"
using namespace ns3;

/**
 * \file nr-test-fdm-of-numerologies.cc
 * \ingroup test
 *
 * \brief This test case checks if the throughput achieved over certain bandwidth part
 * is proportional to the bandwidth of that bandwidth part.
 * The test scenario consists of a scenario in which two UEs are attached to a
 * gNB, and perform UDP full buffer downlink traffic.
 * gNB is configured to have 2 bandwidth parts, which are configured with the
 * same numerology, but can have different bandwidth.
 * Bandwidth part manager is configured to forward first flow over the first
 * bandwidth part, and the second flow over the second bandwidth part.
 * Since the traffic is full buffer traffic, it is expected that when more
 * bandwidth is provided, more throughput will be achieved and vice versa.
 */
class NrTestFdmOfNumerologiesCase1 : public TestCase
{
public:
  NrTestFdmOfNumerologiesCase1 (std::string name, uint32_t numerology, double bw1, double bw2, bool isDownlink, bool isUplink);
  virtual ~NrTestFdmOfNumerologiesCase1 ();

private:
  virtual void DoRun (void);

  uint32_t m_numerology; // the numerology to be used
  double m_bw1; // bandwidth of bandwidth part 1
  double m_bw2; // bandwidth of bandwidth part 2
  bool m_isDownlink; // whether to generate the downlink traffic
  bool m_isUplink; // whether to generate the uplink traffic
};

// Add some help text to this case to describe what it is intended to test
NrTestFdmOfNumerologiesCase1::NrTestFdmOfNumerologiesCase1 (std::string name, uint32_t numerology, double bw1, double bw2, bool isDownlnk, bool isUplink)
  : TestCase (name)
{
  m_numerology = numerology;
  m_bw1 = bw1;
  m_bw2 = bw2;
  m_isDownlink = isDownlnk;
  m_isUplink = isUplink;
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
NrTestFdmOfNumerologiesCase1::~NrTestFdmOfNumerologiesCase1 ()
{}


void
NrTestFdmOfNumerologiesCase1::DoRun (void)
{
  // set simulation time and mobility
  double simTime = 0.2;   // seconds
  double udpAppStartTime = 0.1;   //seconds
  double totalTxPower = 4;
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 2;
  uint32_t packetSize = 1000;

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));
  Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));

  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);

  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 10;
  double ueHeight = 1.5;

  gNbNodes.Create (gNbNum);
  ueNodes.Create (ueNumPergNb * gNbNum);

  Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> staPositionAlloc = CreateObject<ListPositionAllocator> ();
  apPositionAlloc->Add (Vector (0.0, 20, gNbHeight));
  staPositionAlloc->Add (Vector (1, 1, ueHeight));
  staPositionAlloc->Add (Vector (-1, 1, ueHeight));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (apPositionAlloc);
  mobility.Install (gNbNodes);
  mobility.SetPositionAllocator (staPositionAlloc);
  mobility.Install (ueNodes);

  double totalBandwidth = 0;
  totalBandwidth = m_bw1 + m_bw2;

  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  // Put the pointers inside nrHelper
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  nrHelper->SetEpcHelper (epcHelper);


  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;

  const uint8_t numCcPerBand = 2;

  CcBwpCreator::SimpleOperationBandConf bandConf1 (28e9, totalBandwidth, numCcPerBand,
                                                   BandwidthPartInfo::UMi_StreetCanyon_LoS);
  OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);

  //Set BW of each BWP
  band1.m_cc[0]->m_bwp[0]->m_channelBandwidth = m_bw1;
  band1.m_cc[1]->m_bwp[0]->m_channelBandwidth = m_bw2;


  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  nrHelper->InitializeOperationBand (&band1);

  allBwps = CcBwpCreator::GetAllBwps ({band1});


  // gNb routing between Bearer and bandwidh part
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (0));
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (1));
  // Ue routing between Bearer and bandwidth part
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (0));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (1));


  // install nr net devices
  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gNbNodes, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);

  double x = pow (10, totalTxPower / 10);

  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (m_numerology));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Numerology", UintegerValue (m_numerology));

  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue ( 10 * log10 ((m_bw1 / totalBandwidth) * x)));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("TxPower", DoubleValue ( 10 * log10 ((m_bw2 / totalBandwidth) * x)));

  nrHelper->GetUePhy (ueNetDev.Get (0), 0)->SetAttribute ("TxPower", DoubleValue ( 10 * log10 ((m_bw1 / totalBandwidth) * x)));
  nrHelper->GetUePhy (ueNetDev.Get (0), 1)->SetAttribute ("TxPower", DoubleValue ( 10 * log10 ((m_bw2 / totalBandwidth) * x)));

  nrHelper->GetUePhy (ueNetDev.Get (1), 0)->SetAttribute ("TxPower", DoubleValue ( 10 * log10 ((m_bw1 / totalBandwidth) * x)));
  nrHelper->GetUePhy (ueNetDev.Get (1), 1)->SetAttribute ("TxPower", DoubleValue ( 10 * log10 ((m_bw2 / totalBandwidth) * x)));

  for (uint32_t j = 0; j < gNbNodes.GetN (); ++j)
    {
      //We test 2 BWP in this test
      for (uint8_t bwpId = 0; bwpId < 2; bwpId++)
        {
          Ptr<const NrSpectrumPhy> txSpectrumPhy = nrHelper->GetGnbPhy (enbNetDev.Get (j), bwpId)->GetSpectrumPhy ();
          Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel ();
          Ptr<ThreeGppPropagationLossModel> propagationLossModel =  DynamicCast<ThreeGppPropagationLossModel> (txSpectrumChannel->GetPropagationLossModel ());
          NS_ASSERT (propagationLossModel != nullptr);
          propagationLossModel->AssignStreams (1);
          Ptr<ChannelConditionModel> channelConditionModel = propagationLossModel->GetChannelConditionModel ();
          channelConditionModel->AssignStreams (1);
          Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel = DynamicCast<ThreeGppSpectrumPropagationLossModel> (txSpectrumChannel->GetPhasedArraySpectrumPropagationLossModel());
          NS_ASSERT (spectrumLossModel != nullptr);
          Ptr <ThreeGppChannelModel> channel = DynamicCast<ThreeGppChannelModel> (spectrumLossModel->GetChannelModel ());
          channel->AssignStreams (1);
        }
    }


  for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
    {
      //We test 2 BWP in this test
      for (uint8_t bwpId = 0; bwpId < 2; bwpId++)
        {
          Ptr<const NrSpectrumPhy> txSpectrumPhy = nrHelper->GetUePhy (ueNetDev.Get (j), bwpId)->GetSpectrumPhy ();
          Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel ();
          Ptr<ThreeGppPropagationLossModel> propagationLossModel =  DynamicCast<ThreeGppPropagationLossModel> (txSpectrumChannel->GetPropagationLossModel ());
          NS_ASSERT (propagationLossModel != nullptr);
          propagationLossModel->AssignStreams (1);
          Ptr<ChannelConditionModel> channelConditionModel = propagationLossModel->GetChannelConditionModel ();
          channelConditionModel->AssignStreams (1);
          Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel = DynamicCast<ThreeGppSpectrumPropagationLossModel> (txSpectrumChannel->GetPhasedArraySpectrumPropagationLossModel ());
          NS_ASSERT (spectrumLossModel != nullptr);
          Ptr <ThreeGppChannelModel> channel = DynamicCast<ThreeGppChannelModel> (spectrumLossModel->GetChannelModel ());
          channel->AssignStreams (1);
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
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // in this container, interface 0 is the pgw, 1 is the remoteHost
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
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

  // attach UEs to the closest eNB
  nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  ApplicationContainer clientAppsDl;
  ApplicationContainer serverAppsDl;
  ApplicationContainer clientAppsUl;
  ApplicationContainer serverAppsUl;
  //ObjectMapValue objectMapValue;


  if (m_isUplink)
    {
      // configure here UDP traffic
      for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
        {
          UdpServerHelper ulPacketSinkHelper (ulPort);
          serverAppsUl.Add (ulPacketSinkHelper.Install (remoteHost));

          UdpClientHelper ulClient (remoteHostAddr, ulPort);
          ulClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
          ulClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
          ulClient.SetAttribute ("Interval", TimeValue (Seconds (0.00001)));   // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
          clientAppsUl.Add (ulClient.Install (ueNodes.Get (j)));

          Ptr<EpcTft> tft = Create<EpcTft> ();
          EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          tft->Add (ulpf);

          enum EpsBearer::Qci q;

          if (j == 0)
            {
              q = EpsBearer::NGBR_LOW_LAT_EMBB;
            }
          else
            {
              q = EpsBearer::GBR_CONV_VOICE;
            }

          EpsBearer bearer (q);
          nrHelper->ActivateDedicatedEpsBearer (ueNetDev.Get (j), bearer, tft);

          ulPort++;
        }

      serverAppsUl.Start (Seconds (udpAppStartTime));
      clientAppsUl.Start (Seconds (udpAppStartTime));
      serverAppsUl.Stop (Seconds (simTime));
      clientAppsUl.Stop (Seconds (simTime));
    }


  if (m_isDownlink)
    {
      UdpServerHelper dlPacketSinkHelper (dlPort);
      serverAppsDl.Add (dlPacketSinkHelper.Install (ueNodes));

      // configure here UDP traffic
      for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
        {
          UdpClientHelper dlClient (ueIpIface.GetAddress (j), dlPort);
          dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
          dlClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
          dlClient.SetAttribute ("Interval", TimeValue (Seconds (0.00001)));   // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
          clientAppsDl.Add (dlClient.Install (remoteHost));

          Ptr<EpcTft> tft = Create<EpcTft> ();
          EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          tft->Add (dlpf);

          enum EpsBearer::Qci q;

          if (j == 0)
            {
              q = EpsBearer::NGBR_LOW_LAT_EMBB;
            }
          else
            {
              q = EpsBearer::GBR_CONV_VOICE;
            }

          EpsBearer bearer (q);
          nrHelper->ActivateDedicatedEpsBearer (ueNetDev.Get (j), bearer, tft);
        }


      // start UDP server and client apps
      serverAppsDl.Start (Seconds (udpAppStartTime));
      clientAppsDl.Start (Seconds (udpAppStartTime));
      serverAppsDl.Stop (Seconds (simTime));
      clientAppsDl.Stop (Seconds (simTime));
    }

  //nrHelper->EnableTraces();
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  if (m_isDownlink)
    {
      Ptr<UdpServer> serverApp1 = serverAppsDl.Get (0)->GetObject<UdpServer> ();
      Ptr<UdpServer> serverApp2 = serverAppsDl.Get (1)->GetObject<UdpServer> ();
      double throuhgput1 = (serverApp1->GetReceived () * (packetSize + 28) * 8) / (simTime - udpAppStartTime);
      double throuhgput2 = (serverApp2->GetReceived () * (packetSize + 28) * 8) / (simTime - udpAppStartTime);

      NS_TEST_ASSERT_MSG_EQ_TOL (throuhgput2,
                                 throuhgput1 * m_bw2 / m_bw1,
                                 std::max (throuhgput1, throuhgput2) * 0.2, "Throughputs are not equal within tolerance");
      NS_TEST_ASSERT_MSG_NE (throuhgput1, 0, "Throughput should be a non-zero value");
      std::cout << "Total DL UDP throughput 1 (bps):" << throuhgput1 / 10e6 << "Mbps" << std::endl;
      std::cout << "Total DL UDP throughput 2 (bps):" << throuhgput2 / 10e6 << "Mbps" << std::endl;
      std::cout << "\n Test value throughput 1: " << (throuhgput2 * m_bw1 / m_bw2) / 10e6 << "Mbps" << std::endl;
      std::cout << "\n Test value throughput 2: " << (throuhgput1 * m_bw2 / m_bw1) / 10e6 << "Mbps" << std::endl;
    }
  if (m_isUplink)
    {
      Ptr<UdpServer> serverApp1 = serverAppsUl.Get (0)->GetObject<UdpServer> ();
      Ptr<UdpServer> serverApp2 = serverAppsUl.Get (1)->GetObject<UdpServer> ();
      double throughput1 = (serverApp1->GetReceived () * (packetSize + 28) * 8) / (simTime - udpAppStartTime);
      double throughput2 = (serverApp2->GetReceived () * (packetSize + 28) * 8) / (simTime - udpAppStartTime);

      std::cout << "\nBw1:" << m_bw1 << ", bwp2:" << m_bw2 << std::endl;
      std::cout << "Total UL UDP throughput 1 (bps):" << throughput1 / 10e6 << "Mbps" << std::endl;
      std::cout << "Total UL UDP throughput 2 (bps):" << throughput2 / 10e6 << "Mbps" << std::endl;
      std::cout << "Test expected throughput 1: " << (throughput2 * m_bw1 / m_bw2) / 10e6 << "Mbps" << std::endl;
      std::cout << "Test expected throughput 2: " << (throughput1 * m_bw2 / m_bw1) / 10e6 << "Mbps" << std::endl;

      NS_TEST_ASSERT_MSG_EQ_TOL (throughput2, throughput1 * m_bw2 / m_bw1,
                                 std::max (throughput1, throughput2) * 0.5,
                                 "Throughputs are not equal within tolerance");

      NS_TEST_ASSERT_MSG_NE (throughput1, 0, "Throughput should be a non-zero value");

    }

  Simulator::Destroy ();
}

// The TestSuite class names the TestNrTestFdmOfNumerologiesTestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined
//
class NrTestFdmOfNumerologiesTestSuiteDlFour : public TestSuite
{
public:
  NrTestFdmOfNumerologiesTestSuiteDlFour ();
};

NrTestFdmOfNumerologiesTestSuiteDlFour::NrTestFdmOfNumerologiesTestSuiteDlFour ()
  : TestSuite ("nr-test-fdm-of-numerologies-dl-4", SYSTEM)
{
  // downlink test cases
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm dl 4, 50e6, 150e6", 4, 50e6, 150e6, true, false), TestCase::QUICK);
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm dl 4, 100e6, 100e6", 4, 100e6, 100e6, true, false), TestCase::QUICK);
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm dl 4, 80e6, 120e6", 4, 80e6, 120e6, true, false), TestCase::QUICK);
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm dl 4 60e6, 140e6", 4, 60e6, 140e6, true, false), TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NrTestFdmOfNumerologiesTestSuiteDlFour nrTestFdmOfNumerologiesTestSuiteDlFour;


// ----------------------------------------------------------------------------

class NrTestFdmOfNumerologiesTestSuiteDlTwo : public TestSuite
{
public:
  NrTestFdmOfNumerologiesTestSuiteDlTwo ();
};

NrTestFdmOfNumerologiesTestSuiteDlTwo::NrTestFdmOfNumerologiesTestSuiteDlTwo ()
  : TestSuite ("nr-test-fdm-of-numerologies-dl-2", SYSTEM)
{
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm dl 2 50e6 150e6", 2, 50e6, 150e6, true, false), TestCase::QUICK);
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm dl 2 100e6 100e6", 2, 100e6, 100e6, true, false), TestCase::QUICK);
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm dl 2 80e6 120e6", 2, 80e6, 120e6, true, false), TestCase::QUICK);
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm dl 2 60e6 140e6", 2, 60e6, 140e6, true, false), TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NrTestFdmOfNumerologiesTestSuiteDlTwo nrTestFdmOfNumerologiesTestSuiteDlTwo;

// ----------------------------------------------------------------------------

class NrTestFdmOfNumerologiesTestSuiteUlFour : public TestSuite
{
public:
  NrTestFdmOfNumerologiesTestSuiteUlFour ();
};

NrTestFdmOfNumerologiesTestSuiteUlFour::NrTestFdmOfNumerologiesTestSuiteUlFour ()
  : TestSuite ("nr-test-fdm-of-numerologies-ul-4", SYSTEM)
{
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm ul 4, 50e6, 150e6", 4, 50e6, 150e6, false, true), TestCase::QUICK);
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm ul 4, 100e6, 100e6", 4, 100e6, 100e6, false, true), TestCase::QUICK);
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm ul 4, 80e6, 120e6", 4, 80e6, 120e6, false, true), TestCase::QUICK);
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm ul 4 60e6, 140e6", 4, 60e6, 140e6, false, true), TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NrTestFdmOfNumerologiesTestSuiteUlFour nrTestFdmOfNumerologiesTestSuiteUlFour;

// ----------------------------------------------------------------------------

class NrTestFdmOfNumerologiesTestSuiteUlTwo : public TestSuite
{
public:
  NrTestFdmOfNumerologiesTestSuiteUlTwo ();
};

NrTestFdmOfNumerologiesTestSuiteUlTwo::NrTestFdmOfNumerologiesTestSuiteUlTwo ()
  : TestSuite ("nr-test-fdm-of-numerologies-ul-2", SYSTEM)
{
  AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm ul 2 50e6 150e6", 2, 50e6, 150e6, false, true), TestCase::QUICK);
  //AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm ul 2 100e6 100e6", 2, 100e6, 100e6, false, true), TestCase::QUICK);
  //AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm ul 2 80e6 120e6" , 2, 80e6, 120e6, false, true), TestCase::QUICK);
  //AddTestCase (new NrTestFdmOfNumerologiesCase1 ("fdm ul 2 60e6 140e6", 2, 60e6, 140e6, false, true), TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NrTestFdmOfNumerologiesTestSuiteUlTwo nrTestFdmOfNumerologiesTestSuiteUlTwo;





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

// Include a header file from your module to test.
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/nr-module.h"

using namespace ns3;

/**
 * \file nr-system-test-configurations.cc
 * \ingroup test
 *
 * \brief Test the configuration for 5G-LENA. Test that does nothing at the moment.
 */

// This is an example TestCase.
class NrSystemTestConfigurationsTestCase1 : public TestCase
{
public:
  NrSystemTestConfigurationsTestCase1 (std::string name, uint32_t numerology,
                                       std::string scheduler);
  virtual ~NrSystemTestConfigurationsTestCase1 ();

private:
  virtual void DoRun (void);

  uint32_t m_numerology;
  std::string m_scheduler;
};


NrSystemTestConfigurationsTestCase1::NrSystemTestConfigurationsTestCase1 (std::string name, uint32_t numerology,
                                                                          std::string scheduler)
  : TestCase (name)
{
  m_numerology = numerology;
  m_scheduler = scheduler;
}


NrSystemTestConfigurationsTestCase1::~NrSystemTestConfigurationsTestCase1 ()
{}

void
NrSystemTestConfigurationsTestCase1::DoRun (void)
{
  // set mobile device and base station antenna heights in meters, according to the chosen scenario
  double hBS = 35.0; //base station antenna height in meters;
  double hUT = 1.5; //user antenna height in meters;

  // create base stations and mobile terminals
  NodeContainer enbNode;
  NodeContainer ueNode;
  enbNode.Create (1);
  ueNode.Create (1);

  // position the base stations
  Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator> ();
  enbPositionAlloc->Add (Vector (0.0, 0.0, hBS));

  MobilityHelper enbmobility;
  enbmobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  enbmobility.SetPositionAllocator (enbPositionAlloc);
  enbmobility.Install (enbNode);

  // position the mobile terminals and enable the mobility
  MobilityHelper uemobility;
  uemobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  uemobility.Install (ueNode);

  ueNode.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (0, 10, hUT));



  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  // Put the pointers inside nrHelper
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;  // in this example, both bands have a single CC

  // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
  // a single BWP per CC
  CcBwpCreator::SimpleOperationBandConf bandConf1 (28e9, 100e6, numCcPerBand, BandwidthPartInfo::UMi_StreetCanyon);

  // By using the configuration created, it is time to make the operation bands
  OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);

  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds (100)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (100)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  nrHelper->InitializeOperationBand (&band1);

  allBwps = CcBwpCreator::GetAllBwps ({band1});

  nrHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (m_numerology));
  nrHelper->SetSchedulerTypeId (TypeId::LookupByName (m_scheduler));


  // install nr net devices
  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (enbNode, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNode, allBwps);

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
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNode);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));
  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  Ptr<Node> ue = ueNode.Get (0);
  // Set the default gateway for the UE
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverApps.Add (dlPacketSinkHelper.Install (ueNode.Get (0)));

  UdpClientHelper dlClient (ueIpIface.GetAddress (0), dlPort);
  dlClient.SetAttribute ("Interval", TimeValue (MicroSeconds (10000)));
  dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  clientApps.Add (dlClient.Install (remoteHost));

  // start server and client apps
  serverApps.Start (Seconds (0.4));
  clientApps.Start (Seconds (0.4));
  serverApps.Stop (Seconds (1));
  clientApps.Stop (Seconds (1));

  // attach UEs to the closest eNB
  nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  Simulator::Stop (Seconds (1));
  Simulator::Run ();
  Simulator::Destroy ();

  // A wide variety of test macros are available in src/core/test.h
  NS_TEST_ASSERT_MSG_EQ (true, true, "true doesn't equal true for some reason");
  // Use this one for floating point comparisons
  NS_TEST_ASSERT_MSG_EQ_TOL (0.01, 0.01, 0.001, "Numbers are not equal within tolerance");
}


class NrSystemTestConfigurationsTestSuite : public TestSuite
{
public:
  NrSystemTestConfigurationsTestSuite ();

};

NrSystemTestConfigurationsTestSuite::NrSystemTestConfigurationsTestSuite ()
  : TestSuite ("nr-system-test-configurations", SYSTEM)
{
  AddTestCase (new NrSystemTestConfigurationsTestCase1 ("num=0, scheduler=rr", 0, "ns3::NrMacSchedulerTdmaRR"), QUICK);
  AddTestCase (new NrSystemTestConfigurationsTestCase1 ("num=2, scheduler=rr", 2, "ns3::NrMacSchedulerTdmaRR"), QUICK);
  AddTestCase (new NrSystemTestConfigurationsTestCase1 ("num=4, scheduler=rr", 4, "ns3::NrMacSchedulerTdmaRR"), QUICK);

  AddTestCase (new NrSystemTestConfigurationsTestCase1 ("num=0, scheduler=pf", 0, "ns3::NrMacSchedulerTdmaPF"), QUICK);
  AddTestCase (new NrSystemTestConfigurationsTestCase1 ("num=2, scheduler=pf", 2, "ns3::NrMacSchedulerTdmaPF"), QUICK);
  AddTestCase (new NrSystemTestConfigurationsTestCase1 ("num=4, scheduler=pf", 4, "ns3::NrMacSchedulerTdmaPF"), QUICK);

  AddTestCase (new NrSystemTestConfigurationsTestCase1 ("num=0, scheduler=mr", 0, "ns3::NrMacSchedulerTdmaMR"), QUICK);
  AddTestCase (new NrSystemTestConfigurationsTestCase1 ("num=2, scheduler=mr", 2, "ns3::NrMacSchedulerTdmaMR"), QUICK);
  AddTestCase (new NrSystemTestConfigurationsTestCase1 ("num=4, scheduler=mr", 4, "ns3::NrMacSchedulerTdmaMR"), QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NrSystemTestConfigurationsTestSuite nrTestSuite;


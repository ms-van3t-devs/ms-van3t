/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:Gaurav Sathe <gaurav.sathe@tcs.com>
 */

#include <iostream>
#include <sstream>
#include <string>

#include <ns3/object.h>
#include <ns3/spectrum-interference.h>
#include <ns3/spectrum-error-model.h>
#include <ns3/log.h>
#include <ns3/test.h>
#include <ns3/simulator.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include "ns3/cv2x_radio-bearer-stats-calculator.h"
#include <ns3/constant-position-mobility-model.h>
#include <ns3/cv2x_eps-bearer.h>
#include <ns3/node-container.h>
#include <ns3/mobility-helper.h>
#include <ns3/net-device-container.h>
#include <ns3/cv2x_lte-ue-net-device.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-ue-rrc.h>
#include <ns3/cv2x_lte-helper.h>
#include "ns3/string.h"
#include "ns3/double.h"
#include <ns3/cv2x_lte-enb-phy.h>
#include <ns3/cv2x_lte-ue-phy.h>
#include <ns3/boolean.h>
#include <ns3/enum.h>

#include "ns3/cv2x_point-to-point-epc-helper.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"

#include "cv2x_lte-test-deactivate-bearer.h"

NS_LOG_COMPONENT_DEFINE ("cv2x_LenaTestDeactivateBearer");

namespace ns3 {

cv2x_LenaTestBearerDeactivateSuite::cv2x_LenaTestBearerDeactivateSuite ()
  : TestSuite ("lte-test-deactivate-bearer", SYSTEM)
{
  NS_LOG_INFO ("creating cv2x_LenaTestPssFfMacSchedulerSuite");

  bool errorModel = false;

  // Test Case: homogeneous flow test in PSS (different distance)
  // Traffic1 info
  //   UDP traffic: payload size = 100 bytes, interval = 1 ms
  //   UDP rate in scheduler: (payload + RLC header + PDCP header + IP header + UDP header) * 1000 byte/sec -> 132000 byte/rate
  // Maximum throughput = 3 / ( 1/2196000 + 1/1191000 + 1/1383000) = 1486569 byte/s
  // 132000 * 3 = 396000 < 1209046 -> estimated throughput in downlink = 132000 byte/sec
  std::vector<uint16_t> dist_1;

  dist_1.push_back (0);       // User 0 distance --> MCS 28
  dist_1.push_back (0);    // User 1 distance --> MCS 22
  dist_1.push_back (0);    // User 2 distance --> MCS 20

  std::vector<uint16_t> packetSize_1;

  packetSize_1.push_back (100); //1
  packetSize_1.push_back (100); //2
  packetSize_1.push_back (100); //3

  std::vector<uint32_t> estThrPssDl_1;

  estThrPssDl_1.push_back (132000); // User 0 estimated TTI throughput from PSS
  estThrPssDl_1.push_back (132000); // User 1 estimated TTI throughput from PSS
  estThrPssDl_1.push_back (132000); // User 2 estimated TTI throughput from PSS

  AddTestCase (new cv2x_LenaDeactivateBearerTestCase (dist_1,estThrPssDl_1,packetSize_1,1,errorModel,true), TestCase::QUICK);
}

static cv2x_LenaTestBearerDeactivateSuite lenaTestBearerDeactivateSuite; ///< the test suite


std::string
cv2x_LenaDeactivateBearerTestCase::BuildNameString (uint16_t nUser, std::vector<uint16_t> dist)
{
  std::ostringstream oss;
  oss << "distances (m) = [ ";
  for (std::vector<uint16_t>::iterator it = dist.begin (); it != dist.end (); ++it)
    {
      oss << *it << " ";
    }
  oss << "]";
  return oss.str ();
}

cv2x_LenaDeactivateBearerTestCase::cv2x_LenaDeactivateBearerTestCase (std::vector<uint16_t> dist, std::vector<uint32_t> estThrPssDl, std::vector<uint16_t> packetSize, uint16_t interval,bool errorModelEnabled, bool useIdealRrc)
  : TestCase (BuildNameString (dist.size (), dist)),
    m_nUser (dist.size ()),
    m_dist (dist),
    m_packetSize (packetSize),
    m_interval (interval),
    m_estThrPssDl (estThrPssDl),
    m_errorModelEnabled (errorModelEnabled)
{
}

cv2x_LenaDeactivateBearerTestCase::~cv2x_LenaDeactivateBearerTestCase ()
{
}

void
cv2x_LenaDeactivateBearerTestCase::DoRun (void)
{
  if (!m_errorModelEnabled)
    {
      Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
      Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));
    }

  Config::SetDefault ("ns3::cv2x_LteHelper::UseIdealRrc", BooleanValue (true));


  Ptr<cv2x_LteHelper> lteHelper = CreateObject<cv2x_LteHelper> ();
  Ptr<cv2x_PointToPointEpcHelper>  epcHelper = CreateObject<cv2x_PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.001)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);

  // LogComponentEnable ("LenaTestDeactivateBearer", LOG_LEVEL_ALL);
  // LogComponentEnable ("cv2x_LteHelper", logLevel);
  // LogComponentEnable ("cv2x_EpcHelper", logLevel);
  // LogComponentEnable ("cv2x_EpcEnbApplication", logLevel);
  // LogComponentEnable ("cv2x_EpcSgwPgwApplication", logLevel);
  // LogComponentEnable ("cv2x_EpcMme", logLevel);
  // LogComponentEnable ("cv2x_LteEnbRrc", logLevel);

  lteHelper->SetAttribute ("PathlossModel", StringValue ("ns3::FriisSpectrumPropagationLossModel"));

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (m_nUser);

  // Install Mobility Model
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lteHelper->SetSchedulerType ("ns3::cv2x_PssFfMacScheduler");
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);

  Ptr<cv2x_LteEnbNetDevice> lteEnbDev = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ();
  Ptr<cv2x_LteEnbPhy> enbPhy = lteEnbDev->GetPhy ();
  enbPhy->SetAttribute ("TxPower", DoubleValue (30.0));
  enbPhy->SetAttribute ("NoiseFigure", DoubleValue (5.0));

  // Set UEs' position and power
  for (int i = 0; i < m_nUser; i++)
    {
      Ptr<ConstantPositionMobilityModel> mm = ueNodes.Get (i)->GetObject<ConstantPositionMobilityModel> ();
      mm->SetPosition (Vector (m_dist.at (i), 0.0, 0.0));
      Ptr<cv2x_LteUeNetDevice> lteUeDev = ueDevs.Get (i)->GetObject<cv2x_LteUeNetDevice> ();
      Ptr<cv2x_LteUePhy> uePhy = lteUeDev->GetPhy ();
      uePhy->SetAttribute ("TxPower", DoubleValue (23.0));
      uePhy->SetAttribute ("NoiseFigure", DoubleValue (9.0));
    }

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));

  // Assign IP address to UEs
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));

  // Activate an EPS bearer on all UEs

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<NetDevice> ueDevice = ueDevs.Get (u);
      cv2x_GbrQosInformation qos;
      qos.gbrDl = (m_packetSize.at (u) + 32) * (1000 / m_interval) * 8;  // bit/s, considering IP, UDP, RLC, PDCP header size
      qos.gbrUl = (m_packetSize.at (u) + 32) * (1000 / m_interval) * 8;
      qos.mbrDl = qos.gbrDl;
      qos.mbrUl = qos.gbrUl;

      enum cv2x_EpsBearer::Qci q = cv2x_EpsBearer::GBR_CONV_VOICE;
      cv2x_EpsBearer bearer (q, qos);
      bearer.arp.priorityLevel = 15 - (u + 1);
      bearer.arp.preemptionCapability = true;
      bearer.arp.preemptionVulnerability = true;
      lteHelper->ActivateDedicatedEpsBearer (ueDevice, bearer, cv2x_EpcTft::Default ());
    }


  // Install downlink and uplink applications
  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      ++ulPort;
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get (u))); // receive packets from remotehost
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));  // receive packets from UEs

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort); // uplink packets generator
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds (m_interval)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
      dlClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize.at (u)));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);           // downlink packets generator
      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds (m_interval)));
      ulClient.SetAttribute ("MaxPackets", UintegerValue (1000000));
      ulClient.SetAttribute ("PacketSize", UintegerValue (m_packetSize.at (u)));

      clientApps.Add (dlClient.Install (remoteHost));
      clientApps.Add (ulClient.Install (ueNodes.Get (u)));
    }


  serverApps.Start (Seconds (0.030));
  clientApps.Start (Seconds (0.030));

  double statsStartTime = 0.04; // need to allow for RRC connection establishment + SRS
  double statsDuration = 1.0;
  double tolerance = 0.1;

  lteHelper->EnableRlcTraces ();
  Ptr<cv2x_RadioBearerStatsCalculator> rlcStats = lteHelper->GetRlcStats ();
  rlcStats->SetAttribute ("StartTime", TimeValue (Seconds (statsStartTime)));
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (statsDuration)));


  //get ue device pointer for UE-ID 0 IMSI 1 and enb device pointer
  Ptr<NetDevice> ueDevice = ueDevs.Get (0);
  Ptr<NetDevice> enbDevice = enbDevs.Get (0);

  /*
   *   Instantiate De-activation using Simulator::Schedule() method which will initiate bearer de-activation after deActivateTime
   *   Instantiate De-activation in sequence (Time const &time, MEM mem_ptr, OBJ obj, T1 a1, T2 a2, T3 a3)
   */
  Time deActivateTime (Seconds (1.5));
  Simulator::Schedule (deActivateTime, &cv2x_LteHelper::DeActivateDedicatedEpsBearer, lteHelper, ueDevice, enbDevice, 2);

  //stop simulation after 3 seconds
  Simulator::Stop (Seconds (3.0));

  Simulator::Run ();

  NS_LOG_INFO ("DL - Test with " << m_nUser << " user(s)");
  std::vector <uint64_t> dlDataRxed;
  std::vector <uint64_t> dlDataTxed;
  for (int i = 0; i < m_nUser; i++)
    {
      // get the imsi
      uint64_t imsi = ueDevs.Get (i)->GetObject<cv2x_LteUeNetDevice> ()->GetImsi ();
      // get the lcId
      // lcId is hard-coded, since only one dedicated bearer is added
      uint8_t lcId = 4;
      dlDataRxed.push_back (rlcStats->GetDlRxData (imsi, lcId));
      dlDataTxed.push_back (rlcStats->GetDlTxData (imsi, lcId));
      NS_LOG_INFO ("\tUser " << i << " dist " << m_dist.at (i) << " imsi " << imsi << " bytes rxed " << (double)dlDataRxed.at (i) << "  thr " << (double)dlDataRxed.at (i) / statsDuration << " ref " << m_estThrPssDl.at (i));
      NS_LOG_INFO ("\tUser " << i << " imsi " << imsi << " bytes txed " << (double)dlDataTxed.at (i) << "  thr " << (double)dlDataTxed.at (i) / statsDuration);
    }

  for (int i = 0; i < m_nUser; i++)
    {
      uint64_t imsi = ueDevs.Get (i)->GetObject<cv2x_LteUeNetDevice> ()->GetImsi ();

      /*
       * For UE ID-0 IMSI 1, LCID=4 is deactivated hence If traffic seen on it, test case should fail
       * Else For other UE's, test case should validate throughput
       */
      if (imsi == 1)
        {
          NS_TEST_ASSERT_MSG_EQ ((double)dlDataTxed.at (i), 0, "Invalid LCID in Statistics ");
        }
      else
        {
          NS_TEST_ASSERT_MSG_EQ_TOL ((double)dlDataTxed.at (i) / statsDuration, m_estThrPssDl.at (i), m_estThrPssDl.at (i) * tolerance, " Unfair Throughput!");
        }
    }

  Simulator::Destroy ();
}
}

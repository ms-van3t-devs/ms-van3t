/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 */

#include <ns3/object.h>
#include <ns3/spectrum-interference.h>
#include <ns3/spectrum-error-model.h>
#include <ns3/log.h>
#include <ns3/test.h>
#include <ns3/simulator.h>
#include <ns3/packet.h>
#include <ns3/ptr.h>
#include <iostream>
#include <cmath>
#include <ns3/cv2x_radio-bearer-stats-calculator.h>
#include <ns3/mobility-building-info.h>
#include <ns3/hybrid-buildings-propagation-loss-model.h>
#include <ns3/cv2x_eps-bearer.h>
#include <ns3/node-container.h>
#include <ns3/mobility-helper.h>
#include <ns3/net-device-container.h>
#include <ns3/cv2x_lte-ue-net-device.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-ue-rrc.h>
#include <ns3/cv2x_lte-helper.h>
#include <ns3/string.h>
#include <ns3/double.h>
#include <ns3/cv2x_lte-enb-phy.h>
#include <ns3/cv2x_lte-ue-phy.h>
#include <ns3/config.h>
#include <ns3/boolean.h>
#include <ns3/enum.h>
#include <ns3/unused.h>
#include <ns3/cv2x_ff-mac-scheduler.h>
#include <ns3/buildings-helper.h>

#include "cv2x_lte-test-harq.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("cv2x_LenaTestHarq");

cv2x_LenaTestHarqSuite::cv2x_LenaTestHarqSuite ()
  : TestSuite ("lte-harq", SYSTEM)
{
  NS_LOG_INFO ("creating LenaTestHarqTestCase");


  // Tests on DL/UL Data channels (PDSCH, PUSCH)
  // MCS 0 TB size of 66 bytes SINR -9.91 dB expected throughput 31822 bytes/s
  // TBLER 1st tx 1.0
  // TBLER 2nd tx 0.074
  AddTestCase (new cv2x_LenaHarqTestCase (2, 2400, 66, 0.12, 31822), TestCase::QUICK);

  // Tests on DL/UL Data channels (PDSCH, PUSCH)
  // MCS 10 TB size of 472 bytes SINR 0.3 dB expected throughput 209964 bytes/s
  // TBLER 1st tx 1.0
  // TBLER 2nd tx 0.248
  AddTestCase (new cv2x_LenaHarqTestCase (1, 770, 472, 0.06, 209964), TestCase::QUICK);



}

static cv2x_LenaTestHarqSuite lenaTestHarqSuite;

std::string
cv2x_LenaHarqTestCase::BuildNameString (uint16_t nUser, uint16_t dist, uint16_t tbSize)
{
  std::ostringstream oss;
  oss << nUser << " UEs, distance " << dist << " m, TB size " << tbSize;
  return oss.str ();
}

cv2x_LenaHarqTestCase::cv2x_LenaHarqTestCase (uint16_t nUser, uint16_t dist, uint16_t tbSize, double amcBer, double thrRef)
  : TestCase (BuildNameString (nUser, dist, tbSize)),
    m_nUser (nUser),
    m_dist (dist),
    m_amcBer (amcBer),
    m_throughputRef (thrRef)
{
}

cv2x_LenaHarqTestCase::~cv2x_LenaHarqTestCase ()
{
}

void
cv2x_LenaHarqTestCase::DoRun (void)
{

  Config::SetDefault ("ns3::cv2x_LteAmc::Ber", DoubleValue (m_amcBer));
  Config::SetDefault ("ns3::cv2x_LteAmc::AmcModel", EnumValue (cv2x_LteAmc::PiroEW2010));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteHelper::UseIdealRrc", BooleanValue (true));
  //Disable Uplink Power Control
  Config::SetDefault ("ns3::cv2x_LteUePhy::EnableUplinkPowerControl", BooleanValue (false));

//   Config::SetDefault ("ns3::cv2x_RrFfMacScheduler::HarqEnabled", BooleanValue (false));
//   LogComponentEnable ("cv2x_LteEnbRrc", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteUeRrc", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteEnbMac", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteUeMac", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteRlc", LOG_LEVEL_ALL);
//
//   LogComponentEnable ("cv2x_LtePhy", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteEnbPhy", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteUePhy", LOG_LEVEL_ALL);

//   LogComponentEnable ("cv2x_LteSpectrumPhy", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteInterference", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteChunkProcessor", LOG_LEVEL_ALL);
//
//   LogComponentEnable ("LtePropagationLossModel", LOG_LEVEL_ALL);
//   LogComponentEnable ("LossModel", LOG_LEVEL_ALL);
//   LogComponentEnable ("ShadowingLossModel", LOG_LEVEL_ALL);
//   LogComponentEnable ("PenetrationLossModel", LOG_LEVEL_ALL);
//   LogComponentEnable ("MultipathLossModel", LOG_LEVEL_ALL);
//   LogComponentEnable ("PathLossModel", LOG_LEVEL_ALL);
//
//   LogComponentEnable ("cv2x_LteNetDevice", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteUeNetDevice", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteEnbNetDevice", LOG_LEVEL_ALL);

//   LogComponentEnable ("cv2x_RrFfMacScheduler", LOG_LEVEL_ALL);
//   LogComponentEnable ("LenaHelper", LOG_LEVEL_ALL);
//   LogComponentEnable ("RlcStatsCalculator", LOG_LEVEL_ALL);


//   LogComponentEnable ("cv2x_LteSpectrumPhy", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteEnbMac", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteEnbPhy", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteUePhy", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_RrFfMacScheduler", LOG_LEVEL_ALL);
//   LogComponentEnable ("LenaHelper", LOG_LEVEL_ALL);
//   LogComponentEnable ("BuildingsPropagationLossModel", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteMiErrorModel", LOG_LEVEL_ALL);
//   LogComponentEnable ("cv2x_LteAmc", LOG_LEVEL_ALL);
//
//   LogComponentDisableAll (LOG_LEVEL_ALL);

//  LogComponentEnable ("LenaTestHarq", LOG_LEVEL_ALL);


  /**
   * Initialize Simulation Scenario: 1 eNB and m_nUser UEs
   */

  Ptr<cv2x_LteHelper> lena = CreateObject<cv2x_LteHelper> ();

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (m_nUser);

  // Install Mobility Model
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);
  BuildingsHelper::Install (enbNodes);
  
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);
  BuildingsHelper::Install (ueNodes);

  // remove random shadowing component
  lena->SetAttribute ("PathlossModel", StringValue ("ns3::HybridBuildingsPropagationLossModel"));
  lena->SetPathlossModelAttribute ("ShadowSigmaOutdoor", DoubleValue (0.0));
  lena->SetPathlossModelAttribute ("ShadowSigmaIndoor", DoubleValue (0.0));
  lena->SetPathlossModelAttribute ("ShadowSigmaExtWalls", DoubleValue (0.0));

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lena->SetSchedulerType ("ns3::cv2x_RrFfMacScheduler");
  lena->SetSchedulerAttribute ("UlCqiFilter", EnumValue (cv2x_FfMacScheduler::PUSCH_UL_CQI));

  enbDevs = lena->InstallEnbDevice (enbNodes);
  ueDevs = lena->InstallUeDevice (ueNodes);

  // Attach a UE to a eNB
  lena->Attach (ueDevs, enbDevs.Get (0));

  // Activate an EPS bearer
  enum cv2x_EpsBearer::Qci q = cv2x_EpsBearer::GBR_CONV_VOICE;
  cv2x_EpsBearer bearer (q);
  lena->ActivateDataRadioBearer (ueDevs, bearer);


  Ptr<cv2x_LteEnbNetDevice> lteEnbDev = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ();
  Ptr<cv2x_LteEnbPhy> enbPhy = lteEnbDev->GetPhy ();
  enbPhy->SetAttribute ("TxPower", DoubleValue (43.0));
  enbPhy->SetAttribute ("NoiseFigure", DoubleValue (5.0));
  // place the HeNB over the default rooftop level (20 mt.)
  Ptr<MobilityModel> mm = enbNodes.Get (0)->GetObject<MobilityModel> ();
  mm->SetPosition (Vector (0.0, 0.0, 30.0));

  // Set UEs' position and power
  for (int i = 0; i < m_nUser; i++)
    {
      Ptr<MobilityModel> mm = ueNodes.Get (i)->GetObject<MobilityModel> ();
      mm->SetPosition (Vector (m_dist, 0.0, 1.0));
      Ptr<cv2x_LteUeNetDevice> lteUeDev = ueDevs.Get (i)->GetObject<cv2x_LteUeNetDevice> ();
      Ptr<cv2x_LteUePhy> uePhy = lteUeDev->GetPhy ();
      uePhy->SetAttribute ("TxPower", DoubleValue (23.0));
      uePhy->SetAttribute ("NoiseFigure", DoubleValue (9.0));
    }


  double statsStartTime = 0.050; // need to allow for RRC connection establishment + SRS 
  double statsDuration = 2.0;
  Simulator::Stop (Seconds (statsStartTime + statsDuration - 0.0001));

  lena->EnableRlcTraces ();
  Ptr<cv2x_RadioBearerStatsCalculator> rlcStats = lena->GetRlcStats ();
  rlcStats->SetAttribute ("StartTime", TimeValue (Seconds (statsStartTime)));
  rlcStats->SetAttribute ("EpochDuration", TimeValue (Seconds (statsDuration)));

  // for debugging purposes
  lena->EnableMacTraces ();

  Simulator::Run ();

  /**
   * Check that the assignation is done in a RR fashion
   */
  NS_LOG_INFO ("\tTest on downlink data shared channels (PDSCH)");
  NS_LOG_INFO ("Test with " << m_nUser << " user(s) at distance " << m_dist << " expected Thr " << m_throughputRef);
  for (int i = 0; i < m_nUser; i++)
    {
      // get the imsi
      uint64_t imsi = ueDevs.Get (i)->GetObject<cv2x_LteUeNetDevice> ()->GetImsi ();
      uint8_t lcId = 3;
      double txed = rlcStats->GetDlTxData (imsi, lcId);
      double rxed = rlcStats->GetDlRxData (imsi, lcId);
      double tolerance = 0.1;

      NS_LOG_INFO (" User " << i << " imsi " << imsi << " bytes rxed/t " << rxed/statsDuration << " txed/t " << txed/statsDuration << " thr Ref " << m_throughputRef << " Err " << (std::abs (txed/statsDuration - m_throughputRef)) / m_throughputRef);

      NS_TEST_ASSERT_MSG_EQ_TOL (txed/statsDuration, m_throughputRef, m_throughputRef * tolerance, " Unexpected Throughput!");
      NS_TEST_ASSERT_MSG_EQ_TOL (rxed/statsDuration, m_throughputRef, m_throughputRef * tolerance, " Unexpected Throughput!");
    }


  Simulator::Destroy ();
}

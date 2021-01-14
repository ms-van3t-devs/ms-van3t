/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Piotr Gawlowicz
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
 * Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>
 *
 */

#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/callback.h>
#include <ns3/config.h>
#include <ns3/string.h>
#include <ns3/double.h>
#include <ns3/enum.h>
#include <ns3/boolean.h>
#include <ns3/pointer.h>

#include "ns3/mobility-helper.h"
#include "ns3/cv2x_lte-helper.h"
#include "ns3/cv2x_point-to-point-epc-helper.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

#include <ns3/cv2x_ff-mac-scheduler.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-enb-phy.h>
#include <ns3/cv2x_lte-enb-rrc.h>
#include <ns3/cv2x_lte-ue-net-device.h>
#include <ns3/cv2x_lte-ue-phy.h>
#include <ns3/cv2x_lte-ue-rrc.h>

#include "cv2x_lte-ffr-simple.h"
#include "cv2x_lte-simple-spectrum-phy.h"
#include <ns3/cv2x_lte-common.h>

#include "cv2x_lte-test-frequency-reuse.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("cv2x_LteFrequencyReuseTest");

/**
 * TestSuite
 */

cv2x_LteFrequencyReuseTestSuite::cv2x_LteFrequencyReuseTestSuite ()
  : TestSuite ("lte-frequency-reuse", SYSTEM)
{
//  LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_DEBUG);
//  LogComponentEnable ("LteFrequencyReuseTest", logLevel);

  std::vector<bool> availableDlRb;
  std::vector<bool> availableUlRb;
  for (uint32_t i = 0; i < 12; i++)
    {
      availableDlRb.push_back (true);
      availableUlRb.push_back (true);
    }
  for (uint32_t i = 12; i < 25; i++)
    {
      availableDlRb.push_back (false);
      availableUlRb.push_back (false);
    }

  AddTestCase (new cv2x_LteHardFrTestCase ("DownlinkHardFrPf1", 1, "ns3::cv2x_PfFfMacScheduler", 25, 25, 0, 12, 0, 12, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteHardFrTestCase ("DownlinkHardFrPf2", 5, "ns3::cv2x_PfFfMacScheduler", 25, 25, 0, 12, 0, 12, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteHardFrTestCase ("DownlinkHardFrPss1", 1, "ns3::cv2x_PssFfMacScheduler", 25, 25, 0, 12, 0, 12, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteHardFrTestCase ("DownlinkHardFrPss2", 5, "ns3::cv2x_PssFfMacScheduler", 25, 25, 0, 12, 0, 12, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteHardFrTestCase ("DownlinkHardFrCqa1", 1, "ns3::cv2x_CqaFfMacScheduler", 25, 25, 0, 12, 0, 12, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteHardFrTestCase ("DownlinkHardFrCqa2", 5, "ns3::cv2x_CqaFfMacScheduler", 25, 25, 0, 12, 0, 12, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteHardFrTestCase ("DownlinkHardFrFdTbfq1", 1, "ns3::cv2x_FdTbfqFfMacScheduler", 25, 25, 0, 12, 0, 12, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteHardFrTestCase ("DownlinkHardFrFdTbfq2", 5, "ns3::cv2x_FdTbfqFfMacScheduler", 25, 25, 0, 12, 0, 12, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteHardFrTestCase ("DownlinkHardFrTdTbfq1", 1, "ns3::cv2x_TdTbfqFfMacScheduler", 25, 25, 0, 12, 0, 12, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteHardFrTestCase ("DownlinkHardFrTdTbfq2", 5, "ns3::cv2x_TdTbfqFfMacScheduler", 25, 25, 0, 12, 0, 12, availableDlRb, availableUlRb), TestCase::QUICK);

  availableDlRb.clear ();
  availableUlRb.clear ();
  for (uint32_t i = 0; i < 6; i++)
    {
      availableDlRb.push_back (true);
      availableUlRb.push_back (true);
    }
  for (uint32_t i = 6; i < 12; i++)
    {
      availableDlRb.push_back (false);
      availableUlRb.push_back (false);
    }
  for (uint32_t i = 12; i < 18; i++)
    {
      availableDlRb.push_back (true);
      availableUlRb.push_back (true);
    }
  for (uint32_t i = 18; i < 25; i++)
    {
      availableDlRb.push_back (false);
      availableUlRb.push_back (false);
    }

  AddTestCase (new cv2x_LteStrictFrTestCase ("DownlinkStrictFrPf1", 1, "ns3::cv2x_PfFfMacScheduler", 25, 25, 6, 6, 6, 6, 6, 6, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrTestCase ("DownlinkStrictFrPf2", 5, "ns3::cv2x_PfFfMacScheduler", 25, 25, 6, 6, 6, 6, 6, 6, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrTestCase ("DownlinkStrictFrPss1", 1, "ns3::cv2x_PssFfMacScheduler", 25, 25, 6, 6, 6, 6, 6, 6, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrTestCase ("DownlinkStrictFrPss2", 5, "ns3::cv2x_PssFfMacScheduler", 25, 25, 6, 6, 6, 6, 6, 6, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrTestCase ("DownlinkStrictFrCqa1", 1, "ns3::cv2x_CqaFfMacScheduler", 25, 25, 6, 6, 6, 6, 6, 6, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrTestCase ("DownlinkStrictFrCqa2", 5, "ns3::cv2x_CqaFfMacScheduler", 25, 25, 6, 6, 6, 6, 6, 6, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrTestCase ("DownlinkStrictFrCqaFdTbfq1", 1, "ns3::cv2x_FdTbfqFfMacScheduler", 25, 25, 6, 6, 6, 6, 6, 6, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrTestCase ("DownlinkStrictFrCqaFdTbfq2", 5, "ns3::cv2x_FdTbfqFfMacScheduler", 25, 25, 6, 6, 6, 6, 6, 6, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrTestCase ("DownlinkStrictFrCqaTdTbfq1", 1, "ns3::cv2x_TdTbfqFfMacScheduler", 25, 25, 6, 6, 6, 6, 6, 6, availableDlRb, availableUlRb), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrTestCase ("DownlinkStrictFrCqaTdTbfq2", 5, "ns3::cv2x_TdTbfqFfMacScheduler", 25, 25, 6, 6, 6, 6, 6, 6, availableDlRb, availableUlRb), TestCase::QUICK);

  AddTestCase (new cv2x_LteStrictFrAreaTestCase ("cv2x_LteStrictFrAreaTestCasePf1", "ns3::cv2x_PfFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrAreaTestCase ("cv2x_LteStrictFrAreaTestCasePss1", "ns3::cv2x_PssFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrAreaTestCase ("cv2x_LteStrictFrAreaTestCaseCqa1", "ns3::cv2x_CqaFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrAreaTestCase ("cv2x_LteStrictFrAreaTestCaseFdTbfq1", "ns3::cv2x_FdTbfqFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteStrictFrAreaTestCase ("cv2x_LteStrictFrAreaTestCaseTdTbfq1", "ns3::cv2x_TdTbfqFfMacScheduler"), TestCase::QUICK);

  AddTestCase (new cv2x_LteSoftFrAreaTestCase ("cv2x_LteSoftFrAreaTestCasePf1", "ns3::cv2x_PfFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteSoftFrAreaTestCase ("cv2x_LteSoftFrAreaTestCasePss1", "ns3::cv2x_PssFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteSoftFrAreaTestCase ("cv2x_LteSoftFrAreaTestCaseCqa1", "ns3::cv2x_CqaFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteSoftFrAreaTestCase ("cv2x_LteSoftFrAreaTestCaseFdTbfq1", "ns3::cv2x_FdTbfqFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteSoftFrAreaTestCase ("cv2x_LteSoftFrAreaTestCaseTdTbfq1", "ns3::cv2x_TdTbfqFfMacScheduler"), TestCase::QUICK);

  AddTestCase (new cv2x_LteSoftFfrAreaTestCase ("cv2x_LteSoftFfrAreaTestCasePf1", "ns3::cv2x_PfFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteSoftFfrAreaTestCase ("cv2x_LteSoftFfrAreaTestCasePss1", "ns3::cv2x_PssFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteSoftFfrAreaTestCase ("cv2x_LteSoftFfrAreaTestCaseCqa1", "ns3::cv2x_CqaFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteSoftFfrAreaTestCase ("cv2x_LteSoftFfrAreaTestCaseFdTbfq1", "ns3::cv2x_FdTbfqFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteSoftFfrAreaTestCase ("cv2x_LteSoftFfrAreaTestCaseTdTbfq1", "ns3::cv2x_TdTbfqFfMacScheduler"), TestCase::QUICK);

  AddTestCase (new cv2x_LteEnhancedFfrAreaTestCase ("cv2x_LteEnhancedFfrAreaTestCasePf1", "ns3::cv2x_PfFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteEnhancedFfrAreaTestCase ("cv2x_LteEnhancedFfrAreaTestCasePss1", "ns3::cv2x_PssFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteEnhancedFfrAreaTestCase ("cv2x_LteEnhancedFfrAreaTestCaseCqa1", "ns3::cv2x_CqaFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteEnhancedFfrAreaTestCase ("cv2x_LteEnhancedFfrAreaTestCaseFdTbfq1", "ns3::cv2x_FdTbfqFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteEnhancedFfrAreaTestCase ("cv2x_LteEnhancedFfrAreaTestCaseTdTbfq1", "ns3::cv2x_TdTbfqFfMacScheduler"), TestCase::QUICK);

  AddTestCase (new cv2x_LteDistributedFfrAreaTestCase ("cv2x_LteDistributedFfrAreaTestCasePf1", "ns3::cv2x_PfFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteDistributedFfrAreaTestCase ("cv2x_LteDistributedFfrAreaTestCasePss1", "ns3::cv2x_PssFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteDistributedFfrAreaTestCase ("cv2x_LteDistributedFfrAreaTestCaseCqa1", "ns3::cv2x_CqaFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteDistributedFfrAreaTestCase ("cv2x_LteDistributedFfrAreaTestCaseFdTbfq1", "ns3::cv2x_FdTbfqFfMacScheduler"), TestCase::QUICK);
  AddTestCase (new cv2x_LteDistributedFfrAreaTestCase ("cv2x_LteDistributedFfrAreaTestCaseTdTbfq1", "ns3::cv2x_TdTbfqFfMacScheduler"), TestCase::QUICK);
}

static cv2x_LteFrequencyReuseTestSuite lteFrequencyReuseTestSuite;


/**
 * TestCase Data
 */
void
DlDataRxStartNofitication (cv2x_LteFrTestCase *testcase,
                           Ptr<const SpectrumValue> spectrumValue)
{
  testcase->DlDataRxStart (spectrumValue);
}

void
UlDataRxStartNofitication (cv2x_LteFrTestCase *testcase,
                           Ptr<const SpectrumValue> spectrumValue)
{
  testcase->UlDataRxStart (spectrumValue);
}

cv2x_LteFrTestCase::cv2x_LteFrTestCase (std::string name,
                              uint32_t userNum,uint8_t dlBandwidth,uint8_t ulBandwidth,
                              std::vector<bool> availableDlRb, std::vector<bool> availableUlRb)
  : TestCase ("Test: " + name),
    m_userNum (userNum),
    m_dlBandwidth (dlBandwidth),
    m_ulBandwidth (ulBandwidth),
    m_availableDlRb (availableDlRb),
    m_usedMutedDlRbg (false),
    m_availableUlRb (availableUlRb),
    m_usedMutedUlRbg (false)
{
}

cv2x_LteFrTestCase::~cv2x_LteFrTestCase ()
{
}



void
cv2x_LteFrTestCase::DlDataRxStart (Ptr<const SpectrumValue> spectrumValue)
{
  NS_LOG_DEBUG ("DL DATA Power allocation :");
  Values::const_iterator it;
  uint32_t i = 0;
  for (it = spectrumValue->ConstValuesBegin (); it != spectrumValue->ConstValuesEnd (); it++)
    {
      double power =  (*it) * (m_dlBandwidth * 180000);
      NS_LOG_DEBUG ("RB " << i << " POWER: " << " " << power << " isAvailable: " << m_availableDlRb[i]);

      if (m_availableDlRb[i] == false && power > 0)
        {
          m_usedMutedDlRbg = true;
        }
      i++;
    }
}

void
cv2x_LteFrTestCase::UlDataRxStart (Ptr<const SpectrumValue> spectrumValue)
{
  NS_LOG_DEBUG ("UL DATA Power allocation :");
  Values::const_iterator it;
  uint32_t i = 0;
  for (it = spectrumValue->ConstValuesBegin (); it != spectrumValue->ConstValuesEnd (); it++)
    {
      double power =  (*it) * (m_ulBandwidth * 180000);
      NS_LOG_DEBUG ("RB " << i << " POWER: " << " " << power << " isAvailable: " << m_availableUlRb[i]);

      if (m_availableUlRb[i] == false && power > 0)
        {
          m_usedMutedUlRbg = true;
        }
      i++;
    }
}


void
cv2x_LteFrTestCase::DoRun (void)
{

}


cv2x_LteHardFrTestCase::cv2x_LteHardFrTestCase (std::string name, uint32_t userNum,
                                      std::string schedulerType,
                                      uint8_t dlBandwidth, uint8_t ulBandwidth,
                                      uint8_t dlSubBandOffset, uint8_t dlSubBandwidth,
                                      uint8_t ulSubBandOffset, uint8_t ulSubBandwidth,
                                      std::vector<bool> availableDlRb, std::vector<bool> availableUlRb)
  : cv2x_LteFrTestCase (name, userNum, dlBandwidth, ulBandwidth, availableDlRb, availableUlRb),
    m_schedulerType (schedulerType),
    m_dlSubBandOffset (dlSubBandOffset),
    m_dlSubBandwidth (dlSubBandwidth),
    m_ulSubBandOffset (ulSubBandOffset),
    m_ulSubBandwidth (ulSubBandwidth)
{
  NS_LOG_INFO ("Creating LteDownlinkFrTestCase");
}

cv2x_LteHardFrTestCase::~cv2x_LteHardFrTestCase ()
{
}

void
cv2x_LteHardFrTestCase::DoRun (void)
{
  NS_LOG_DEBUG ("cv2x_LteFrTestCase");

  Config::Reset ();
  Config::SetDefault ("ns3::cv2x_LteHelper::UseIdealRrc", BooleanValue (false));

  /**
    * Simulation Topology
    */

  Ptr<cv2x_LteHelper> lteHelper = CreateObject<cv2x_LteHelper> ();
  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFrHardAlgorithm");

  lteHelper->SetFfrAlgorithmAttribute ("DlSubBandOffset", UintegerValue (m_dlSubBandOffset));
  lteHelper->SetFfrAlgorithmAttribute ("DlSubBandwidth", UintegerValue (m_dlSubBandwidth));

  lteHelper->SetFfrAlgorithmAttribute ("UlSubBandOffset", UintegerValue (m_ulSubBandOffset));
  lteHelper->SetFfrAlgorithmAttribute ("UlSubBandwidth", UintegerValue (m_ulSubBandwidth));


  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (m_userNum);
  NodeContainer allNodes = NodeContainer ( enbNodes, ueNodes );

  // Install Mobility Model
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (allNodes);

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lteHelper->SetSchedulerType (m_schedulerType);
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));

  // Activate the default EPS bearer
  enum cv2x_EpsBearer::Qci q = cv2x_EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
  cv2x_EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

  //Test SpectrumPhy to get signals form DL channel
  Ptr<cv2x_LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ()->GetPhy ()->GetDownlinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> dlChannel = enbDlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testDlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  Ptr<cv2x_LteEnbNetDevice> eNbDev = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ();
  testDlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetDlEarfcn (), m_dlBandwidth));
  dlChannel->AddRx (testDlSpectrumPhy);

  testDlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&DlDataRxStartNofitication, this));

  //Test SpectrumPhy to get signals form UL channel
  Ptr<cv2x_LteSpectrumPhy> ueUlSpectrumPhy = ueDevs.Get (0)->GetObject<cv2x_LteUeNetDevice> ()->GetPhy ()->GetUplinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> ulChannel = ueUlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testUlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  testUlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetUlEarfcn (), m_ulBandwidth));
  ulChannel->AddRx (testUlSpectrumPhy);

  testUlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&UlDataRxStartNofitication, this));

  Simulator::Stop (Seconds (0.500));
  Simulator::Run ();

  NS_TEST_ASSERT_MSG_EQ (m_usedMutedDlRbg, false,
                         "Scheduler used DL RBG muted by FFR Algorithm");

  NS_TEST_ASSERT_MSG_EQ (m_usedMutedUlRbg, false,
                         "Scheduler used UL RBG muted by FFR Algorithm");

  Simulator::Destroy ();
}



cv2x_LteStrictFrTestCase::cv2x_LteStrictFrTestCase (std::string name, uint32_t userNum,
                                          std::string schedulerType,
                                          uint8_t dlBandwidth, uint8_t ulBandwidth,
                                          uint8_t dlCommonSubBandwidth, uint8_t dlEdgeSubBandOffset, uint8_t dlEdgeSubBandwidth,
                                          uint8_t ulCommonSubBandwidth, uint8_t ulEdgeSubBandOffset, uint8_t ulEdgeSubBandwidth,
                                          std::vector<bool> availableDlRb, std::vector<bool> availableUlRb)
  : cv2x_LteFrTestCase (name, userNum, dlBandwidth, ulBandwidth, availableDlRb, availableUlRb),
    m_schedulerType (schedulerType),
    m_dlCommonSubBandwidth (dlCommonSubBandwidth),
    m_dlEdgeSubBandOffset (dlEdgeSubBandOffset),
    m_dlEdgeSubBandwidth (dlEdgeSubBandwidth),
    m_ulCommonSubBandwidth (ulCommonSubBandwidth),
    m_ulEdgeSubBandOffset (ulEdgeSubBandOffset),
    m_ulEdgeSubBandwidth (ulEdgeSubBandwidth)
{
  NS_LOG_INFO ("Creating cv2x_LteFrTestCase");
}

cv2x_LteStrictFrTestCase::~cv2x_LteStrictFrTestCase ()
{
}

void
cv2x_LteStrictFrTestCase::DoRun (void)
{
  NS_LOG_DEBUG ("cv2x_LteFrTestCase");

  Config::Reset ();
  Config::SetDefault ("ns3::cv2x_LteHelper::UseIdealRrc", BooleanValue (false));

  /**
    * Simulation Topology
    */

  Ptr<cv2x_LteHelper> lteHelper = CreateObject<cv2x_LteHelper> ();
  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFrStrictAlgorithm");

  lteHelper->SetFfrAlgorithmAttribute ("DlCommonSubBandwidth", UintegerValue (m_dlCommonSubBandwidth));
  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandOffset", UintegerValue (m_dlEdgeSubBandOffset));
  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandwidth", UintegerValue (m_dlEdgeSubBandwidth));

  lteHelper->SetFfrAlgorithmAttribute ("UlCommonSubBandwidth", UintegerValue (m_ulCommonSubBandwidth));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandOffset", UintegerValue (m_ulEdgeSubBandOffset));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandwidth", UintegerValue (m_ulEdgeSubBandwidth));

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  enbNodes.Create (1);
  ueNodes.Create (m_userNum);
  NodeContainer allNodes = NodeContainer ( enbNodes, ueNodes );

  // Install Mobility Model
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (allNodes);

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs;
  lteHelper->SetSchedulerType (m_schedulerType);
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs = lteHelper->InstallUeDevice (ueNodes);

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs, enbDevs.Get (0));

  // Activate the default EPS bearer
  enum cv2x_EpsBearer::Qci q = cv2x_EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
  cv2x_EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs, bearer);

  //Test SpectrumPhy to get signals form DL channel
  Ptr<cv2x_LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ()->GetPhy ()->GetDownlinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> dlChannel = enbDlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testDlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  Ptr<cv2x_LteEnbNetDevice> eNbDev = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ();
  testDlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetDlEarfcn (), m_dlBandwidth));
  dlChannel->AddRx (testDlSpectrumPhy);

  testDlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&DlDataRxStartNofitication, this));

  //Test SpectrumPhy to get signals form UL channel
  Ptr<cv2x_LteSpectrumPhy> ueUlSpectrumPhy = ueDevs.Get (0)->GetObject<cv2x_LteUeNetDevice> ()->GetPhy ()->GetUplinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> ulChannel = ueUlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testUlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  testUlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetUlEarfcn (), m_ulBandwidth));
  ulChannel->AddRx (testUlSpectrumPhy);

  testUlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&UlDataRxStartNofitication, this));

  Simulator::Stop (Seconds (0.500));
  Simulator::Run ();

  NS_TEST_ASSERT_MSG_EQ (m_usedMutedDlRbg, false,
                         "Scheduler used DL RBG muted by FFR Algorithm");

  NS_TEST_ASSERT_MSG_EQ (m_usedMutedUlRbg, false,
                         "Scheduler used UL RBG muted by FFR Algorithm");

  Simulator::Destroy ();
}

void
DlDataRxStartNofiticationArea (cv2x_LteFrAreaTestCase *testcase,
                               Ptr<const SpectrumValue> spectrumValue)
{
  testcase->DlDataRxStart (spectrumValue);
}

void
UlDataRxStartNofiticationArea (cv2x_LteFrAreaTestCase *testcase,
                               Ptr<const SpectrumValue> spectrumValue)
{
  testcase->UlDataRxStart (spectrumValue);
}

cv2x_LteFrAreaTestCase::cv2x_LteFrAreaTestCase (std::string name, std::string schedulerType)
  : TestCase ("Test: " + name),
    m_schedulerType (schedulerType)
{
  m_dlBandwidth = 25;
  m_ulBandwidth = 25;
  m_usedWrongDlRbg = false;
  m_usedWrongUlRbg = false;
}
cv2x_LteFrAreaTestCase::~cv2x_LteFrAreaTestCase ()
{
}

void
cv2x_LteFrAreaTestCase::DlDataRxStart (Ptr<const SpectrumValue> spectrumValue)
{
  //need time to report new UE measurements, and wait because of filtering
  if ( (Simulator::Now () - m_teleportTime ) < MilliSeconds (400))
    {
      return;
    }

  NS_LOG_DEBUG ("DL DATA Power allocation :");
  Values::const_iterator it;
  uint32_t i = 0;
  for (it = spectrumValue->ConstValuesBegin (); it != spectrumValue->ConstValuesEnd (); it++)
    {
      double power =  (*it) * (m_dlBandwidth * 180000);
      NS_LOG_DEBUG ("RB " << i << " POWER: " << " " << power);
      NS_LOG_DEBUG ("RB " << i << " POWER: " << " " << power
                          << " Available: " << m_expectedDlRb[i] << " Expected Power: " << m_expectedDlPower );

      if (m_expectedDlRb[i] == false && power > 0)
        {
          m_usedWrongDlRbg = true;
        }
      else if (m_expectedDlRb[i] == true && power > 0)
        {
          NS_TEST_ASSERT_MSG_EQ_TOL (power, m_expectedDlPower, 0.1,
                                     "Wrong Data Channel DL Power level");
        }
      i++;
    }
}

void
cv2x_LteFrAreaTestCase::UlDataRxStart (Ptr<const SpectrumValue> spectrumValue)
{
  //need time to report new UE measurements, and wait because of filtering
  if ( (Simulator::Now () - m_teleportTime ) < MilliSeconds (400))
    {
      return;
    }

  NS_LOG_DEBUG ("UL DATA Power allocation :");
  Values::const_iterator it;
  uint32_t i = 0;
  for (it = spectrumValue->ConstValuesBegin (); it != spectrumValue->ConstValuesEnd (); it++)
    {
      double power =  (*it) * (m_ulBandwidth * 180000);
      NS_LOG_DEBUG ("RB " << i << " POWER: " << power << " expectedUlPower: " << m_expectedUlPower);
      if (m_expectedUlRb[i] == false && power > 0)
        {
          m_usedWrongUlRbg = true;
        }
      else if (m_expectedUlRb[i] == true && power > 0)
        {
          NS_TEST_ASSERT_MSG_EQ_TOL (power, m_expectedUlPower, 0.01,
                                     "Wrong Data Channel UL Power level" << Simulator::Now ().GetSeconds ());
        }
      i++;
    }
}

void
cv2x_LteFrAreaTestCase::SimpleTeleportUe (uint32_t x, uint32_t y)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Teleport UE to : (" << x << ", " << y << ", 0)");
  m_teleportTime = Simulator::Now ();
  m_ueMobility->SetPosition (Vector (x, y, 0.0));
}

void
cv2x_LteFrAreaTestCase::TeleportUe (uint32_t x, uint32_t y, double expectedPower,
                               std::vector<bool> expectedDlRb)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Teleport UE to : (" << x << ", " << y << ", 0)");
  m_teleportTime = Simulator::Now ();
  m_ueMobility->SetPosition (Vector (x, y, 0.0));
  m_expectedDlPower = expectedPower;
  m_expectedDlRb = expectedDlRb;
}

void
cv2x_LteFrAreaTestCase::TeleportUe2 (Ptr<Node> ueNode, uint32_t x, uint32_t y, double expectedPower,
                                std::vector<bool> expectedDlRb)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Teleport UE to : (" << x << ", " << y << ", 0)");

  Ptr<MobilityModel> ueMobility = ueNode->GetObject<MobilityModel> ();
  ueMobility->SetPosition (Vector (x, y, 0.0));
  m_teleportTime = Simulator::Now ();
  m_expectedDlPower = expectedPower;
  m_expectedDlRb = expectedDlRb;
}

void
cv2x_LteFrAreaTestCase::SetDlExpectedValues (double expectedDlPower, std::vector<bool> expectedDlRb)
{
  NS_LOG_FUNCTION (this);
  m_expectedDlPower = expectedDlPower;
  m_expectedDlRb = expectedDlRb;
}

void
cv2x_LteFrAreaTestCase::SetUlExpectedValues (double expectedUlPower, std::vector<bool> expectedUlRb)
{
  NS_LOG_FUNCTION (this);
  m_expectedUlPower = expectedUlPower;
  m_expectedUlRb = expectedUlRb;
}

void
cv2x_LteFrAreaTestCase::DoRun (void)
{
}

cv2x_LteStrictFrAreaTestCase::cv2x_LteStrictFrAreaTestCase (std::string name, std::string schedulerType)
  : cv2x_LteFrAreaTestCase (name, schedulerType)
{
  NS_LOG_INFO ("Creating cv2x_LteFrTestCase");
}

cv2x_LteStrictFrAreaTestCase::~cv2x_LteStrictFrAreaTestCase ()
{
}

void
cv2x_LteStrictFrAreaTestCase::DoRun (void)
{
  NS_LOG_DEBUG ("cv2x_LteStrictFrAreaTestCase");

  Config::Reset ();
  Config::SetDefault ("ns3::cv2x_LteHelper::UseIdealRrc", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));

  double eNbTxPower = 30;
  Config::SetDefault ("ns3::cv2x_LteEnbPhy::TxPower", DoubleValue (eNbTxPower));
  Config::SetDefault ("ns3::cv2x_LteUePhy::TxPower", DoubleValue (10.0));
  Config::SetDefault ("ns3::cv2x_LteUePhy::EnableUplinkPowerControl", BooleanValue (true));

  Config::SetDefault ("ns3::cv2x_LteUePowerControl::ClosedLoop", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteUePowerControl::AccumulationEnabled", BooleanValue (false));

  Ptr<cv2x_LteHelper> lteHelper = CreateObject<cv2x_LteHelper> ();

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes1;
  NodeContainer ueNodes2;
  enbNodes.Create (2);
  ueNodes1.Create (1);
  ueNodes2.Create (1);
  NodeContainer allNodes = NodeContainer ( enbNodes, ueNodes1, ueNodes2);

  /*
   * The topology is the following:
   *
   *  eNB1            UE1                                      eNB2
   *    |              |                                         |
   *    x ------------ x ------------------------ x ------------ x----UE2
   *         200 m               600 m                  200 m      20 m
   *
   */

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));   // eNB1
  positionAlloc->Add (Vector (1000, 0.0, 0.0)); // eNB2
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  // UE1
  positionAlloc->Add (Vector (1020, 0.0, 0.0));  // UE2
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);
  m_ueMobility = ueNodes1.Get (0)->GetObject<MobilityModel> ();

  // Disable layer-3 filtering
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::RsrpFilterCoefficient",
                      UintegerValue (0));
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::RsrqFilterCoefficient",
                      UintegerValue (0));

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs1;
  NetDeviceContainer ueDevs2;
  lteHelper->SetSchedulerType (m_schedulerType);

  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFrStrictAlgorithm");
  lteHelper->SetFfrAlgorithmAttribute ("RsrqThreshold", UintegerValue (25));
  lteHelper->SetFfrAlgorithmAttribute ("CenterPowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB0));
  lteHelper->SetFfrAlgorithmAttribute ("EdgePowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB3));

  lteHelper->SetFfrAlgorithmAttribute ("DlCommonSubBandwidth", UintegerValue (6));
  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandOffset", UintegerValue (6));
  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandwidth", UintegerValue (6));

  lteHelper->SetFfrAlgorithmAttribute ("UlCommonSubBandwidth", UintegerValue (6));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandOffset", UintegerValue (6));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandwidth", UintegerValue (6));
  enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (0)));

  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFrNoOpAlgorithm");
  enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (1)));

  ueDevs1 = lteHelper->InstallUeDevice (ueNodes1);
  ueDevs2 = lteHelper->InstallUeDevice (ueNodes2);

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs1, enbDevs.Get (0));
  lteHelper->Attach (ueDevs2, enbDevs.Get (1));

  // Activate an EPS bearer
  enum cv2x_EpsBearer::Qci q = cv2x_EpsBearer::GBR_CONV_VOICE;
  cv2x_EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs1, bearer);
  lteHelper->ActivateDataRadioBearer (ueDevs2, bearer);

  //Test SpectrumPhy to get signals form DL channel
  Ptr<cv2x_LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ()->GetPhy ()->GetDownlinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> dlChannel = enbDlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testDlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  Ptr<cv2x_LteEnbNetDevice> eNbDev = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ();
  testDlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetDlEarfcn (), m_dlBandwidth));
  dlChannel->AddRx (testDlSpectrumPhy);

  testDlSpectrumPhy->SetCellId (1);

  testDlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&DlDataRxStartNofiticationArea, this));

  //Test SpectrumPhy to get signals form UL channel
  Ptr<cv2x_LteSpectrumPhy> ueUlSpectrumPhy = ueDevs1.Get (0)->GetObject<cv2x_LteUeNetDevice> ()->GetPhy ()->GetUplinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> ulChannel = ueUlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testUlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  testUlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetUlEarfcn (), m_ulBandwidth));
  ulChannel->AddRx (testUlSpectrumPhy);

  testUlSpectrumPhy->SetCellId (1);

  testUlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&UlDataRxStartNofiticationArea, this));

  std::vector<bool> expectedDlRbCenterArea;
  expectedDlRbCenterArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbCenterArea;
  expectedUlRbCenterArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 0; i < 6; i++)
    {
      expectedDlRbCenterArea[i] = true;
      expectedUlRbCenterArea[i] = true;
    }

  std::vector<bool> expectedDlRbEdgeArea;
  expectedDlRbEdgeArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbEdgeArea;
  expectedUlRbEdgeArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 12; i < 18; i++)
    {
      expectedDlRbEdgeArea[i] = true;
      expectedUlRbEdgeArea[i] = true;
    }

  Simulator::Schedule (MilliSeconds (1),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 200, 0, 1, expectedDlRbCenterArea );
  Simulator::Schedule (MilliSeconds (1),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0150543, expectedUlRbCenterArea );

  Simulator::Schedule (MilliSeconds (501),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 800, 0, 2, expectedDlRbEdgeArea );
  Simulator::Schedule (MilliSeconds (501),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.199526, expectedUlRbEdgeArea );

  Simulator::Schedule (MilliSeconds (1001),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 200, 0, 1, expectedDlRbCenterArea );
  Simulator::Schedule (MilliSeconds (1001),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0150543, expectedUlRbCenterArea );

  Simulator::Stop (Seconds (1.500));
  Simulator::Run ();

  NS_TEST_ASSERT_MSG_EQ (m_usedWrongDlRbg, false,
                         "Scheduler used DL RB muted by FFR Algorithm");
  NS_TEST_ASSERT_MSG_EQ (m_usedWrongUlRbg, false,
                         "Scheduler used UL RB muted by FFR Algorithm");

  Simulator::Destroy ();
}

cv2x_LteSoftFrAreaTestCase::cv2x_LteSoftFrAreaTestCase (std::string name, std::string schedulerType)
  : cv2x_LteFrAreaTestCase (name, schedulerType)
{
  NS_LOG_INFO ("Creating cv2x_LteSoftFrAreaTestCase");
}

cv2x_LteSoftFrAreaTestCase::~cv2x_LteSoftFrAreaTestCase ()
{
}

void
cv2x_LteSoftFrAreaTestCase::DoRun (void)
{
  NS_LOG_DEBUG ("cv2x_LteSoftFrAreaTestCase");

  Config::Reset ();
  Config::SetDefault ("ns3::cv2x_LteHelper::UseIdealRrc", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));

  double eNbTxPower = 30;
  Config::SetDefault ("ns3::cv2x_LteEnbPhy::TxPower", DoubleValue (eNbTxPower));
  Config::SetDefault ("ns3::cv2x_LteUePhy::TxPower", DoubleValue (10.0));
  Config::SetDefault ("ns3::cv2x_LteUePhy::EnableUplinkPowerControl", BooleanValue (true));

  Config::SetDefault ("ns3::cv2x_LteUePowerControl::ClosedLoop", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteUePowerControl::AccumulationEnabled", BooleanValue (false));

  Ptr<cv2x_LteHelper> lteHelper = CreateObject<cv2x_LteHelper> ();

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes1;
  NodeContainer ueNodes2;
  enbNodes.Create (2);
  ueNodes1.Create (1);
  ueNodes2.Create (1);
  NodeContainer allNodes = NodeContainer ( enbNodes, ueNodes1, ueNodes2);

  /*
   * The topology is the following:
   *
   *  eNB1            UE1                                      eNB2
   *    |              |                                         |
   *    x ------------ x ------------------------ x ------------ x----UE2
   *         200 m               600 m                  200 m      20 m
   *
   */

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));   // eNB1
  positionAlloc->Add (Vector (1000, 0.0, 0.0)); // eNB2
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  // UE1
  positionAlloc->Add (Vector (1020, 0.0, 0.0));  // UE2
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);
  m_ueMobility = ueNodes1.Get (0)->GetObject<MobilityModel> ();

  // Disable layer-3 filtering
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::RsrpFilterCoefficient",
                      UintegerValue (0));
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::RsrqFilterCoefficient",
                      UintegerValue (0));

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs1;
  NetDeviceContainer ueDevs2;
  lteHelper->SetSchedulerType (m_schedulerType);

  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFrSoftAlgorithm");
  lteHelper->SetFfrAlgorithmAttribute ("AllowCenterUeUseEdgeSubBand", BooleanValue (false));
  lteHelper->SetFfrAlgorithmAttribute ("RsrqThreshold", UintegerValue (25));
  lteHelper->SetFfrAlgorithmAttribute ("CenterPowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB0));
  lteHelper->SetFfrAlgorithmAttribute ("EdgePowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB3));

  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandOffset", UintegerValue (8));
  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandwidth", UintegerValue (8));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandOffset", UintegerValue (8));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandwidth", UintegerValue (8));
  enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (0)));

  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFrNoOpAlgorithm");
  enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (1)));

  ueDevs1 = lteHelper->InstallUeDevice (ueNodes1);
  ueDevs2 = lteHelper->InstallUeDevice (ueNodes2);

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs1, enbDevs.Get (0));
  lteHelper->Attach (ueDevs2, enbDevs.Get (1));

  // Activate an EPS bearer
  enum cv2x_EpsBearer::Qci q = cv2x_EpsBearer::GBR_CONV_VOICE;
  cv2x_EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs1, bearer);
  lteHelper->ActivateDataRadioBearer (ueDevs2, bearer);

  //Test SpectrumPhy to get signals form DL channel
  Ptr<cv2x_LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ()->GetPhy ()->GetDownlinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> dlChannel = enbDlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testDlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  Ptr<cv2x_LteEnbNetDevice> eNbDev = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ();
  testDlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetDlEarfcn (), m_dlBandwidth));
  dlChannel->AddRx (testDlSpectrumPhy);

  testDlSpectrumPhy->SetCellId (1);

  testDlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&DlDataRxStartNofiticationArea, this));

  //Test SpectrumPhy to get signals form UL channel
  Ptr<cv2x_LteSpectrumPhy> ueUlSpectrumPhy = ueDevs1.Get (0)->GetObject<cv2x_LteUeNetDevice> ()->GetPhy ()->GetUplinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> ulChannel = ueUlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testUlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  testUlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetUlEarfcn (), m_ulBandwidth));
  ulChannel->AddRx (testUlSpectrumPhy);

  testUlSpectrumPhy->SetCellId (1);

  testUlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&UlDataRxStartNofiticationArea, this));


  std::vector<bool> expectedDlRbCenterArea;
  expectedDlRbCenterArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbCenterArea;
  expectedUlRbCenterArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 0; i < 8; i++)
    {
      expectedDlRbCenterArea[i] = true;
      expectedUlRbCenterArea[i] = true;
    }
  for (uint32_t i = 16; i < 25; i++)
    {
      expectedDlRbCenterArea[i] = true;
    }

  std::vector<bool> expectedDlRbEdgeArea;
  expectedDlRbEdgeArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbEdgeArea;
  expectedUlRbEdgeArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 8; i < 16; i++)
    {
      expectedDlRbEdgeArea[i] = true;
      expectedUlRbEdgeArea[i] = true;
    }

  Simulator::Schedule (MilliSeconds (1),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 200, 0, 1, expectedDlRbCenterArea );
  Simulator::Schedule (MilliSeconds (1),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0150543, expectedUlRbCenterArea );

  Simulator::Schedule (MilliSeconds (501),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 800, 0, 2, expectedDlRbEdgeArea );
  Simulator::Schedule (MilliSeconds (501),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.199526, expectedUlRbEdgeArea );

  Simulator::Schedule (MilliSeconds (1001),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 200, 0, 1, expectedDlRbCenterArea );
  Simulator::Schedule (MilliSeconds (1001),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0150543, expectedUlRbCenterArea );

  Simulator::Stop (Seconds (1.500));
  Simulator::Run ();

#if 0
  NS_TEST_ASSERT_MSG_EQ (m_usedWrongDlRbg, false,
                         "Scheduler used DL RBG muted by FFR Algorithm");

#endif
  NS_TEST_ASSERT_MSG_EQ (m_usedWrongUlRbg, false,
                         "Scheduler used UL RB muted by FFR Algorithm");

  Simulator::Destroy ();
}

cv2x_LteSoftFfrAreaTestCase::cv2x_LteSoftFfrAreaTestCase (std::string name, std::string schedulerType)
  : cv2x_LteFrAreaTestCase (name, schedulerType)
{
  NS_LOG_INFO ("Creating cv2x_LteSoftFfrAreaTestCase");
}

cv2x_LteSoftFfrAreaTestCase::~cv2x_LteSoftFfrAreaTestCase ()
{
}

void
cv2x_LteSoftFfrAreaTestCase::DoRun (void)
{
  NS_LOG_DEBUG ("cv2x_LteSoftFfrAreaTestCase");

  Config::Reset ();
  Config::SetDefault ("ns3::cv2x_LteHelper::UseIdealRrc", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));

  double eNbTxPower = 30;
  Config::SetDefault ("ns3::cv2x_LteEnbPhy::TxPower", DoubleValue (eNbTxPower));
  Config::SetDefault ("ns3::cv2x_LteUePhy::TxPower", DoubleValue (10.0));
  Config::SetDefault ("ns3::cv2x_LteUePhy::EnableUplinkPowerControl", BooleanValue (true));

  Config::SetDefault ("ns3::cv2x_LteUePowerControl::ClosedLoop", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteUePowerControl::AccumulationEnabled", BooleanValue (false));

  Ptr<cv2x_LteHelper> lteHelper = CreateObject<cv2x_LteHelper> ();

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes1;
  NodeContainer ueNodes2;
  enbNodes.Create (2);
  ueNodes1.Create (1);
  ueNodes2.Create (1);
  NodeContainer allNodes = NodeContainer ( enbNodes, ueNodes1, ueNodes2);

  /*
   * The topology is the following:
   *
   *  eNB1            UE1                                      eNB2
   *    |              |                                         |
   *    x ------------ x ------------------------ x ------------ x----UE2
   *         200 m               600 m                  200 m      20 m
   *
   */

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));   // eNB1
  positionAlloc->Add (Vector (1000, 0.0, 0.0)); // eNB2
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  // UE1
  positionAlloc->Add (Vector (1020, 0.0, 0.0));  // UE2
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);
  m_ueMobility = ueNodes1.Get (0)->GetObject<MobilityModel> ();

  // Disable layer-3 filtering
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::RsrpFilterCoefficient",
                      UintegerValue (0));
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::RsrqFilterCoefficient",
                      UintegerValue (0));

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs1;
  NetDeviceContainer ueDevs2;
  lteHelper->SetSchedulerType (m_schedulerType);

  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFfrSoftAlgorithm");
  lteHelper->SetFfrAlgorithmAttribute ("CenterRsrqThreshold", UintegerValue (28));
  lteHelper->SetFfrAlgorithmAttribute ("EdgeRsrqThreshold", UintegerValue (18));
  lteHelper->SetFfrAlgorithmAttribute ("CenterAreaPowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB_3));
  lteHelper->SetFfrAlgorithmAttribute ("MediumAreaPowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB0));
  lteHelper->SetFfrAlgorithmAttribute ("EdgeAreaPowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB3));

  lteHelper->SetFfrAlgorithmAttribute ("UlCommonSubBandwidth", UintegerValue (6));
  lteHelper->SetFfrAlgorithmAttribute ("DlCommonSubBandwidth", UintegerValue (6));

  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandOffset", UintegerValue (6));
  lteHelper->SetFfrAlgorithmAttribute ("DlEdgeSubBandwidth", UintegerValue (6));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandOffset", UintegerValue (6));
  lteHelper->SetFfrAlgorithmAttribute ("UlEdgeSubBandwidth", UintegerValue (6));
  enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (0)));

  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFrNoOpAlgorithm");
  enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (1)));

  ueDevs1 = lteHelper->InstallUeDevice (ueNodes1);
  ueDevs2 = lteHelper->InstallUeDevice (ueNodes2);

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs1, enbDevs.Get (0));
  lteHelper->Attach (ueDevs2, enbDevs.Get (1));

  // Activate an EPS bearer
  enum cv2x_EpsBearer::Qci q = cv2x_EpsBearer::GBR_CONV_VOICE;
  cv2x_EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs1, bearer);
  lteHelper->ActivateDataRadioBearer (ueDevs2, bearer);

  //Test SpectrumPhy to get signals form DL channel
  Ptr<cv2x_LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ()->GetPhy ()->GetDownlinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> dlChannel = enbDlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testDlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  Ptr<cv2x_LteEnbNetDevice> eNbDev = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ();
  testDlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetDlEarfcn (), m_dlBandwidth));
  dlChannel->AddRx (testDlSpectrumPhy);

  testDlSpectrumPhy->SetCellId (1);

  testDlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&DlDataRxStartNofiticationArea, this));

  //Test SpectrumPhy to get signals form UL channel
  Ptr<cv2x_LteSpectrumPhy> ueUlSpectrumPhy = ueDevs1.Get (0)->GetObject<cv2x_LteUeNetDevice> ()->GetPhy ()->GetUplinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> ulChannel = ueUlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testUlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  testUlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetUlEarfcn (), m_ulBandwidth));
  ulChannel->AddRx (testUlSpectrumPhy);

  testUlSpectrumPhy->SetCellId (1);

  testUlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&UlDataRxStartNofiticationArea, this));

  double expectedDlPowerCenterArea = 0.5;
  std::vector<bool> expectedDlRbCenterArea;
  expectedDlRbCenterArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbCenterArea;
  expectedUlRbCenterArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 6; i < 12; i++)
    {
      expectedDlRbCenterArea[i] = true;
      expectedUlRbCenterArea[i] = true;
    }
  for (uint32_t i = 18; i < 25; i++)
    {
      expectedDlRbCenterArea[i] = true;
      expectedUlRbCenterArea[i] = true;
    }

  double expectedDlPowerMiddleArea = 1.0;
  std::vector<bool> expectedDlRbMiddleArea;
  expectedDlRbMiddleArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbMiddleArea;
  expectedUlRbMiddleArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 0; i < 6; i++)
    {
      expectedDlRbMiddleArea[i] = true;
      expectedUlRbMiddleArea[i] = true;
    }

  double expectedDlPowerEdgeArea = 2.0;
  std::vector<bool> expectedDlRbEdgeArea;
  expectedDlRbEdgeArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbEdgeArea;
  expectedUlRbEdgeArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 12; i < 18; i++)
    {
      expectedDlRbEdgeArea[i] = true;
      expectedUlRbEdgeArea[i] = true;
    }

  Simulator::Schedule (MilliSeconds (1),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 200, 0,
                       expectedDlPowerCenterArea, expectedDlRbCenterArea );
  Simulator::Schedule (MilliSeconds (1),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0150543, expectedUlRbCenterArea );

  Simulator::Schedule (MilliSeconds (501),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 600, 0,
                       expectedDlPowerMiddleArea, expectedDlRbMiddleArea );
  Simulator::Schedule (MilliSeconds (501),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.135489, expectedUlRbMiddleArea );

  Simulator::Schedule (MilliSeconds (1001),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 800, 0,
                       expectedDlPowerEdgeArea, expectedDlRbEdgeArea );
  Simulator::Schedule (MilliSeconds (1001),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.199526, expectedUlRbEdgeArea );

  Simulator::Schedule (MilliSeconds (1501),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 600, 0,
                       expectedDlPowerMiddleArea, expectedDlRbMiddleArea );
  Simulator::Schedule (MilliSeconds (1501),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.135489, expectedUlRbMiddleArea );

  Simulator::Schedule (MilliSeconds (2001),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 200, 0,
                       expectedDlPowerCenterArea, expectedDlRbCenterArea );
  Simulator::Schedule (MilliSeconds (2001),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0150543, expectedUlRbCenterArea );

  Simulator::Stop (Seconds (2.500));
  Simulator::Run ();

  NS_TEST_ASSERT_MSG_EQ (m_usedWrongDlRbg, false,
                         "Scheduler used DL RBG muted by FFR Algorithm");

  NS_TEST_ASSERT_MSG_EQ (m_usedWrongUlRbg, false,
                         "Scheduler used UL RB muted by FFR Algorithm");

  Simulator::Destroy ();
}


cv2x_LteEnhancedFfrAreaTestCase::cv2x_LteEnhancedFfrAreaTestCase (std::string name, std::string schedulerType)
  : cv2x_LteFrAreaTestCase (name, schedulerType)
{
  NS_LOG_INFO ("Creating cv2x_LteEnhancedFfrAreaTestCase");
}

cv2x_LteEnhancedFfrAreaTestCase::~cv2x_LteEnhancedFfrAreaTestCase ()
{
}

void
cv2x_LteEnhancedFfrAreaTestCase::DoRun (void)
{
  NS_LOG_DEBUG ("cv2x_LteEnhancedFfrAreaTestCase");

  Config::Reset ();
  Config::SetDefault ("ns3::cv2x_LteHelper::UseIdealRrc", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteHelper::UsePdschForCqiGeneration", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));

  double eNbTxPower = 30;
  Config::SetDefault ("ns3::cv2x_LteEnbPhy::TxPower", DoubleValue (eNbTxPower));
  Config::SetDefault ("ns3::cv2x_LteUePhy::TxPower", DoubleValue (10.0));
  Config::SetDefault ("ns3::cv2x_LteUePhy::EnableUplinkPowerControl", BooleanValue (true));

  Config::SetDefault ("ns3::cv2x_LteUePowerControl::ClosedLoop", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteUePowerControl::AccumulationEnabled", BooleanValue (false));


  Ptr<cv2x_LteHelper> lteHelper = CreateObject<cv2x_LteHelper> ();

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes1;
  NodeContainer ueNodes2;
  enbNodes.Create (2);
  ueNodes1.Create (1);
  ueNodes2.Create (1);
  NodeContainer allNodes = NodeContainer ( enbNodes, ueNodes1, ueNodes2);

  /*
   * The topology is the following:
   *
   *  eNB1            UE1                                      eNB2
   *    |              |                                         |
   *    x ------------ x ------------------------ x ------------ x----UE2
   *         200 m               600 m                  200 m      20 m
   *
   */

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));   // eNB1
  positionAlloc->Add (Vector (1000, 0.0, 0.0)); // eNB2
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));  // UE1
  positionAlloc->Add (Vector (1020, 0.0, 0.0));  // UE2
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);
  m_ueMobility = ueNodes1.Get (0)->GetObject<MobilityModel> ();

  // Disable layer-3 filtering
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::RsrpFilterCoefficient",
                      UintegerValue (0));
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::RsrqFilterCoefficient",
                      UintegerValue (0));

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs1;
  NetDeviceContainer ueDevs2;
  lteHelper->SetSchedulerType (m_schedulerType);
  lteHelper->SetSchedulerAttribute ("HarqEnabled",  BooleanValue (true));

  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (m_dlBandwidth));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (m_ulBandwidth));

  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFfrEnhancedAlgorithm");
  lteHelper->SetFfrAlgorithmAttribute ("RsrqThreshold", UintegerValue (25));
  lteHelper->SetFfrAlgorithmAttribute ("DlCqiThreshold", UintegerValue (10));
  lteHelper->SetFfrAlgorithmAttribute ("UlCqiThreshold", UintegerValue (15));
  lteHelper->SetFfrAlgorithmAttribute ("CenterAreaPowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB_6));
  lteHelper->SetFfrAlgorithmAttribute ("EdgeAreaPowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB0));

  lteHelper->SetFfrAlgorithmAttribute ("UlSubBandOffset", UintegerValue (0));
  lteHelper->SetFfrAlgorithmAttribute ("UlReuse3SubBandwidth", UintegerValue (4));
  lteHelper->SetFfrAlgorithmAttribute ("UlReuse1SubBandwidth", UintegerValue (4));

  lteHelper->SetFfrAlgorithmAttribute ("DlSubBandOffset", UintegerValue (0));
  lteHelper->SetFfrAlgorithmAttribute ("DlReuse3SubBandwidth", UintegerValue (4));
  lteHelper->SetFfrAlgorithmAttribute ("DlReuse1SubBandwidth", UintegerValue (4));

  enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (0)));

  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFrNoOpAlgorithm");
  enbDevs.Add (lteHelper->InstallEnbDevice (enbNodes.Get (1)));

  ueDevs1 = lteHelper->InstallUeDevice (ueNodes1);
  ueDevs2 = lteHelper->InstallUeDevice (ueNodes2);

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs1, enbDevs.Get (0));
  lteHelper->Attach (ueDevs2, enbDevs.Get (1));

  // Activate an EPS bearer
  enum cv2x_EpsBearer::Qci q = cv2x_EpsBearer::GBR_CONV_VOICE;
  cv2x_EpsBearer bearer (q);
  lteHelper->ActivateDataRadioBearer (ueDevs1, bearer);
  lteHelper->ActivateDataRadioBearer (ueDevs2, bearer);

  //Test SpectrumPhy to get signals form DL channel
  Ptr<cv2x_LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ()->GetPhy ()->GetDownlinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> dlChannel = enbDlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testDlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  Ptr<cv2x_LteEnbNetDevice> eNbDev = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ();
  testDlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetDlEarfcn (), m_dlBandwidth));
  dlChannel->AddRx (testDlSpectrumPhy);

  testDlSpectrumPhy->SetCellId (1);

  testDlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&DlDataRxStartNofiticationArea, this));

  //Test SpectrumPhy to get signals form UL channel
  Ptr<cv2x_LteSpectrumPhy> ueUlSpectrumPhy = ueDevs1.Get (0)->GetObject<cv2x_LteUeNetDevice> ()->GetPhy ()->GetUplinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> ulChannel = ueUlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testUlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  testUlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetUlEarfcn (), m_ulBandwidth));
  ulChannel->AddRx (testUlSpectrumPhy);

  testUlSpectrumPhy->SetCellId (1);

  testUlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&UlDataRxStartNofiticationArea, this));

  double expectedDlPowerCenterArea = 0.251189;
  std::vector<bool> expectedDlRbCenterArea;
  expectedDlRbCenterArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbCenterArea;
  expectedUlRbCenterArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 0; i < 8; i++)
    {
      expectedDlRbCenterArea[i] = true;
      expectedUlRbCenterArea[i] = true;
    }
  for (uint32_t i = 12; i < 16; i++)
    {
      expectedDlRbCenterArea[i] = true;
      expectedUlRbCenterArea[i] = true;
    }
  for (uint32_t i = 20; i < 24; i++)
    {
      expectedDlRbCenterArea[i] = true;
      expectedUlRbCenterArea[i] = true;
    }

  double expectedDlPowerMiddleArea = 0.251189;
  std::vector<bool> expectedDlRbMiddleArea;
  expectedDlRbMiddleArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbMiddleArea;
  expectedUlRbMiddleArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 4; i < 8; i++)
    {
      expectedDlRbMiddleArea[i] = true;
      expectedUlRbMiddleArea[i] = true;
    }

  double expectedDlPowerEdgeArea = 1.0;
  std::vector<bool> expectedDlRbEdgeArea;
  expectedDlRbEdgeArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbEdgeArea;
  expectedUlRbEdgeArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 0; i < 4; i++)
    {
      expectedDlRbEdgeArea[i] = true;
      expectedUlRbEdgeArea[i] = true;
    }

  Simulator::Schedule (MilliSeconds (1),
                       &cv2x_LteEnhancedFfrAreaTestCase::TeleportUe, this, 100, 0,
                       expectedDlPowerCenterArea, expectedDlRbCenterArea );
  Simulator::Schedule (MilliSeconds (1),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.00250905, expectedUlRbCenterArea );

  Simulator::Schedule (MilliSeconds (501),
                       &cv2x_LteEnhancedFfrAreaTestCase::TeleportUe, this, 300, 0,
                       expectedDlPowerMiddleArea, expectedDlRbMiddleArea );
  Simulator::Schedule (MilliSeconds (501),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0225815, expectedUlRbMiddleArea );

  Simulator::Schedule (MilliSeconds (1001),
                       &cv2x_LteEnhancedFfrAreaTestCase::TeleportUe, this, 600, 0,
                       expectedDlPowerEdgeArea, expectedDlRbEdgeArea );
  Simulator::Schedule (MilliSeconds (1001),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0903259, expectedUlRbEdgeArea );

  Simulator::Schedule (MilliSeconds (1501),
                       &cv2x_LteEnhancedFfrAreaTestCase::TeleportUe, this, 100, 0,
                       expectedDlPowerCenterArea, expectedDlRbCenterArea );
  Simulator::Schedule (MilliSeconds (1501),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.00250905, expectedUlRbCenterArea );

  Simulator::Schedule (MilliSeconds (2001),
                       &cv2x_LteEnhancedFfrAreaTestCase::TeleportUe, this, 300, 0,
                       expectedDlPowerMiddleArea, expectedDlRbMiddleArea );
  Simulator::Schedule (MilliSeconds (2001),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0225815, expectedUlRbCenterArea );

  Simulator::Stop (Seconds (2.500));
  Simulator::Run ();

  NS_TEST_ASSERT_MSG_EQ (m_usedWrongDlRbg, false,
                         "Scheduler used DL RBG muted by FFR Algorithm");

  NS_TEST_ASSERT_MSG_EQ (m_usedWrongUlRbg, false,
                         "Scheduler used UL RB muted by FFR Algorithm");

  Simulator::Destroy ();
}


cv2x_LteDistributedFfrAreaTestCase::cv2x_LteDistributedFfrAreaTestCase (std::string name, std::string schedulerType)
  : cv2x_LteFrAreaTestCase (name, schedulerType)
{
  NS_LOG_INFO ("Creating cv2x_LteDistributedFfrAreaTestCase");
}

cv2x_LteDistributedFfrAreaTestCase::~cv2x_LteDistributedFfrAreaTestCase ()
{
}

void
cv2x_LteDistributedFfrAreaTestCase::DoRun (void)
{
  NS_LOG_DEBUG ("cv2x_LteDistributedFfrAreaTestCase");

  Config::Reset ();
  Config::SetDefault ("ns3::cv2x_LteHelper::UseIdealRrc", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteHelper::UsePdschForCqiGeneration", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::CtrlErrorModelEnabled", BooleanValue (false));
  Config::SetDefault ("ns3::cv2x_LteSpectrumPhy::DataErrorModelEnabled", BooleanValue (false));

  double eNbTxPower = 30;
  Config::SetDefault ("ns3::cv2x_LteEnbPhy::TxPower", DoubleValue (eNbTxPower));
  Config::SetDefault ("ns3::cv2x_LteUePhy::TxPower", DoubleValue (10.0));
  Config::SetDefault ("ns3::cv2x_LteUePhy::EnableUplinkPowerControl", BooleanValue (true));

  Config::SetDefault ("ns3::cv2x_LteUePowerControl::ClosedLoop", BooleanValue (true));
  Config::SetDefault ("ns3::cv2x_LteUePowerControl::AccumulationEnabled", BooleanValue (false));

  // Disable layer-3 filtering
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::RsrpFilterCoefficient",
                      UintegerValue (0));
  Config::SetDefault ("ns3::cv2x_LteEnbRrc::RsrqFilterCoefficient",
                      UintegerValue (0));

  uint8_t bandwidth = 25;

  Ptr<cv2x_LteHelper> lteHelper = CreateObject<cv2x_LteHelper> ();
  Ptr<cv2x_PointToPointEpcHelper> epcHelper = CreateObject<cv2x_PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetHandoverAlgorithmType ("ns3::cv2x_NoOpHandoverAlgorithm"); // disable automatic handover

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
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  // Routing of the Internet Host (towards the LTE network)
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  // interface 0 is localhost, 1 is the p2p device
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // Create Nodes: eNodeB and UE
  NodeContainer enbNodes;
  NodeContainer ueNodes1;
  NodeContainer ueNodes2;
  enbNodes.Create (2);
  ueNodes1.Create (2);
  ueNodes2.Create (1);
  NodeContainer ueNodes = NodeContainer ( ueNodes1, ueNodes2);
  NodeContainer allNodes = NodeContainer ( enbNodes, ueNodes1, ueNodes2);

  /*
   * The topology is the following:
   *
   *  eNB1            UE1                        UE2            eNB2
   *    |              |                          |              |
   *    x ------------ x ------------------------ x ------------ x
   *         200 m               600 m                  200 m
   *
   */

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));                  // eNB1
  positionAlloc->Add (Vector (1000,  0.0, 0.0));                // eNB2

  positionAlloc->Add (Vector (200, 0.0, 0.0));                  // UE1
  positionAlloc->Add (Vector (200, 0.0, 0.0));                  // UE1
  positionAlloc->Add (Vector (800, 0.0, 0.0));                  // UE2

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);
  m_ueMobility = ueNodes2.Get (0)->GetObject<MobilityModel> ();

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer enbDevs;
  NetDeviceContainer ueDevs1;
  NetDeviceContainer ueDevs2;
  lteHelper->SetSchedulerType (m_schedulerType);

  lteHelper->SetEnbDeviceAttribute ("DlBandwidth", UintegerValue (bandwidth));
  lteHelper->SetEnbDeviceAttribute ("UlBandwidth", UintegerValue (bandwidth));

  lteHelper->SetFfrAlgorithmType ("ns3::cv2x_LteFfrDistributedAlgorithm");
  lteHelper->SetFfrAlgorithmAttribute ("CalculationInterval", TimeValue (MilliSeconds (10)));
  lteHelper->SetFfrAlgorithmAttribute ("RsrqThreshold", UintegerValue (25));
  lteHelper->SetFfrAlgorithmAttribute ("RsrpDifferenceThreshold", UintegerValue (5));
  lteHelper->SetFfrAlgorithmAttribute ("EdgeRbNum", UintegerValue (6));
  lteHelper->SetFfrAlgorithmAttribute ("CenterPowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB0));
  lteHelper->SetFfrAlgorithmAttribute ("EdgePowerOffset",
                                       UintegerValue (cv2x_LteRrcSap::PdschConfigDedicated::dB3));

  enbDevs = lteHelper->InstallEnbDevice (enbNodes);
  ueDevs1 = lteHelper->InstallUeDevice (ueNodes1);
  ueDevs2 = lteHelper->InstallUeDevice (ueNodes2);

  NetDeviceContainer ueLteDevs;
  ueLteDevs.Add (ueDevs1);
  ueLteDevs.Add (ueDevs2);

  // Add X2 interface
  lteHelper->AddX2Interface (enbNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIfaces;
  ueIpIfaces = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // Attach a UE to a eNB
  lteHelper->Attach (ueDevs1, enbDevs.Get (0));
  lteHelper->Attach (ueDevs2, enbDevs.Get (1));


  // Install and start applications on UEs and remote host
  uint16_t dlPort = 10000;
  uint16_t ulPort = 20000;

  // randomize a bit start times to avoid simulation artifacts
  // (e.g., buffer overflows due to packet transmissions happening
  // exactly at the same time)
  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (0.010));

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ue = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      for (uint32_t b = 0; b < 1; ++b)
        {
          ++dlPort;
          ++ulPort;

          ApplicationContainer clientApps;
          ApplicationContainer serverApps;

          NS_LOG_LOGIC ("installing UDP DL app for UE " << u);
          UdpClientHelper dlClientHelper (ueIpIfaces.GetAddress (u), dlPort);
          dlClientHelper.SetAttribute ("MaxPackets", UintegerValue (1000000));
          dlClientHelper.SetAttribute ("Interval", TimeValue (MilliSeconds (1.0)));
          clientApps.Add (dlClientHelper.Install (remoteHost));
          PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), dlPort));
          serverApps.Add (dlPacketSinkHelper.Install (ue));

          NS_LOG_LOGIC ("installing UDP UL app for UE " << u);
          UdpClientHelper ulClientHelper (remoteHostAddr, ulPort);
          ulClientHelper.SetAttribute ("MaxPackets", UintegerValue (1000000));
          ulClientHelper.SetAttribute ("Interval", TimeValue (MilliSeconds (1.0)));
          clientApps.Add (ulClientHelper.Install (ue));
          PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory",
                                               InetSocketAddress (Ipv4Address::GetAny (), ulPort));
          serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

          Ptr<cv2x_EpcTft> tft = Create<cv2x_EpcTft> ();
          cv2x_EpcTft::PacketFilter dlpf;
          dlpf.localPortStart = dlPort;
          dlpf.localPortEnd = dlPort;
          tft->Add (dlpf);
          cv2x_EpcTft::PacketFilter ulpf;
          ulpf.remotePortStart = ulPort;
          ulpf.remotePortEnd = ulPort;
          tft->Add (ulpf);
          cv2x_EpsBearer bearer (cv2x_EpsBearer::NGBR_VIDEO_TCP_DEFAULT);
          lteHelper->ActivateDedicatedEpsBearer (ueLteDevs.Get (u), bearer, tft);

          Time startTime = Seconds (startTimeSeconds->GetValue ());
          serverApps.Start (startTime);
          clientApps.Start (startTime);
        }
    }

  //Test SpectrumPhy to get signals form DL channel
  Ptr<cv2x_LteSpectrumPhy> enbDlSpectrumPhy = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ()->GetPhy ()->GetDownlinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> dlChannel = enbDlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testDlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  Ptr<cv2x_LteEnbNetDevice> eNbDev = enbDevs.Get (0)->GetObject<cv2x_LteEnbNetDevice> ();
  testDlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetDlEarfcn (), m_dlBandwidth));
  dlChannel->AddRx (testDlSpectrumPhy);

  testDlSpectrumPhy->SetCellId (2);

  testDlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&DlDataRxStartNofiticationArea, this));

  //Test SpectrumPhy to get signals form UL channel
  Ptr<cv2x_LteSpectrumPhy> ueUlSpectrumPhy = ueDevs1.Get (0)->GetObject<cv2x_LteUeNetDevice> ()->GetPhy ()->GetUplinkSpectrumPhy ()->GetObject<cv2x_LteSpectrumPhy> ();
  Ptr<SpectrumChannel> ulChannel = ueUlSpectrumPhy->GetChannel ();

  Ptr<cv2x_LteSimpleSpectrumPhy> testUlSpectrumPhy = CreateObject<cv2x_LteSimpleSpectrumPhy> ();
  testUlSpectrumPhy->SetRxSpectrumModel (cv2x_LteSpectrumValueHelper::GetSpectrumModel (eNbDev->GetUlEarfcn (), m_ulBandwidth));
  ulChannel->AddRx (testUlSpectrumPhy);

  testUlSpectrumPhy->SetCellId (2);

  testUlSpectrumPhy->TraceConnectWithoutContext ("RxStart",
                                                 MakeBoundCallback (&UlDataRxStartNofiticationArea, this));

  double expectedDlPowerCenterArea = 1.0;
  std::vector<bool> expectedDlRbCenterArea;
  expectedDlRbCenterArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbCenterArea;
  expectedUlRbCenterArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 0; i < m_dlBandwidth; i++)
    {
      expectedDlRbCenterArea[i] = true;
      expectedUlRbCenterArea[i] = true;
    }

  double expectedDlPowerEdgeArea = 2.0;
  std::vector<bool> expectedDlRbEdgeArea;
  expectedDlRbEdgeArea.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbEdgeArea;
  expectedUlRbEdgeArea.resize (m_ulBandwidth, false);
  for (uint32_t i = 0; i < 6; i++)
    {
      expectedDlRbEdgeArea[i] = true;
      expectedUlRbEdgeArea[i] = true;
    }

  std::vector<bool> expectedDlRbEdgeArea2;
  expectedDlRbEdgeArea2.resize (m_dlBandwidth, false);
  std::vector<bool> expectedUlRbEdgeArea2;
  expectedUlRbEdgeArea2.resize (m_dlBandwidth, false);
  for (uint32_t i = 6; i < 12; i++)
    {
      expectedDlRbEdgeArea2[i] = true;
      expectedUlRbEdgeArea2[i] = true;
    }


  Simulator::Schedule (MilliSeconds (1),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 800, 0,
                       expectedDlPowerCenterArea, expectedDlRbCenterArea );
  Simulator::Schedule (MilliSeconds (1),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0225815, expectedUlRbCenterArea );

  Simulator::Schedule (MilliSeconds (501),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 400, 0,
                       expectedDlPowerEdgeArea, expectedDlRbEdgeArea );
  Simulator::Schedule (MilliSeconds (501),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.135489, expectedUlRbEdgeArea );

  Simulator::Schedule (MilliSeconds (1001),
                       &cv2x_LteFrAreaTestCase::TeleportUe2, this, ueNodes1.Get (0), 600, 0,
                       expectedDlPowerEdgeArea, expectedDlRbEdgeArea2 );
  Simulator::Schedule (MilliSeconds (1001),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.135489, expectedUlRbEdgeArea2 );

  Simulator::Schedule (MilliSeconds (1501),
                       &cv2x_LteFrAreaTestCase::TeleportUe2, this, ueNodes1.Get (0), 200, 0,
                       expectedDlPowerEdgeArea, expectedDlRbEdgeArea );
  Simulator::Schedule (MilliSeconds (1501),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.135489, expectedUlRbEdgeArea );

  Simulator::Schedule (MilliSeconds (2001),
                       &cv2x_LteFrAreaTestCase::TeleportUe, this, 800, 0,
                       expectedDlPowerCenterArea, expectedDlRbCenterArea );
  Simulator::Schedule (MilliSeconds (2001),
                       &cv2x_LteFrAreaTestCase::SetUlExpectedValues, this, 0.0225815, expectedUlRbCenterArea );

  Simulator::Stop (Seconds (2.500));
  Simulator::Run ();

#if 0
  NS_TEST_ASSERT_MSG_EQ (m_usedWrongDlRbg, false,
                         "Scheduler used DL RBG muted by FFR Algorithm");
#endif

  NS_TEST_ASSERT_MSG_EQ (m_usedWrongUlRbg, false,
                         "Scheduler used UL RB muted by FFR Algorithm");

  Simulator::Destroy ();

}

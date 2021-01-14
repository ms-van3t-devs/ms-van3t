/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Alexander Krotov
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
 * Author: Alexander Krotov <krotov@iitp.ru>
 *
 */

#include "cv2x_lte-test-secondary-cell-selection.h"

#include <ns3/boolean.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/log.h>
#include <ns3/simulator.h>

#include <ns3/internet-stack-helper.h>
#include <ns3/ipv4-address-helper.h>
#include <ns3/ipv4-interface-container.h>
#include <ns3/friis-spectrum-propagation-loss.h>
#include <ns3/ipv4-static-routing-helper.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-helper.h>
#include <ns3/cv2x_lte-ue-net-device.h>
#include <ns3/cv2x_lte-ue-rrc.h>
#include <ns3/mobility-helper.h>
#include <ns3/net-device-container.h>
#include <ns3/node-container.h>
#include <ns3/cv2x_point-to-point-epc-helper.h>
#include <ns3/point-to-point-helper.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("cv2x_LteSecondaryCellSelectionTest");

/*
 * Test Suite
 */

cv2x_LteSecondaryCellSelectionTestSuite::cv2x_LteSecondaryCellSelectionTestSuite ()
  : TestSuite ("lte-secondary-cell-selection", SYSTEM)
{
  // REAL RRC PROTOCOL

  AddTestCase (new cv2x_LteSecondaryCellSelectionTestCase ("EPC, real RRC, RngRun=1", false, 1U, 2), TestCase::QUICK);
  AddTestCase (new cv2x_LteSecondaryCellSelectionTestCase ("EPC, real RRC, RngRun=1", false, 1U, 4), TestCase::QUICK);

  // IDEAL RRC PROTOCOL

  AddTestCase (new cv2x_LteSecondaryCellSelectionTestCase ("EPC, ideal RRC, RngRun=1", true, 1U, 2), TestCase::QUICK);
  AddTestCase (new cv2x_LteSecondaryCellSelectionTestCase ("EPC, ideal RRC, RngRun=1", true, 1U, 4), TestCase::QUICK);

} // end of cv2x_LteSecondaryCellSelectionTestSuite::cv2x_LteSecondaryCellSelectionTestSuite ()

static cv2x_LteSecondaryCellSelectionTestSuite g_lteSecondaryCellSelectionTestSuite;

/*
 * Test Case
 */

cv2x_LteSecondaryCellSelectionTestCase::cv2x_LteSecondaryCellSelectionTestCase (
  std::string name, bool isIdealRrc, uint64_t rngRun, uint8_t numberOfComponentCarriers)
  : TestCase (name),
    m_isIdealRrc (isIdealRrc),
    m_rngRun (rngRun),
    m_numberOfComponentCarriers (numberOfComponentCarriers)
{
  NS_LOG_FUNCTION (this << GetName ());
}


cv2x_LteSecondaryCellSelectionTestCase::~cv2x_LteSecondaryCellSelectionTestCase ()
{
  NS_LOG_FUNCTION (this << GetName ());
}

void
cv2x_LteSecondaryCellSelectionTestCase::DoRun ()
{
  NS_LOG_FUNCTION (this << GetName ());

  Config::SetGlobal ("RngRun", UintegerValue (m_rngRun));

  // Create helpers.
  auto lteHelper = CreateObject<cv2x_LteHelper> ();
  lteHelper->SetAttribute ("PathlossModel", TypeIdValue (ns3::FriisSpectrumPropagationLossModel::GetTypeId ()));
  lteHelper->SetAttribute ("UseIdealRrc", BooleanValue (m_isIdealRrc));
  lteHelper->SetAttribute ("NumberOfComponentCarriers", UintegerValue (m_numberOfComponentCarriers));

  auto epcHelper = CreateObject<cv2x_PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  // Create nodes.
  auto enbNode = CreateObject<Node> ();
  NodeContainer ueNodes;
  ueNodes.Create (m_numberOfComponentCarriers);

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNode);
  mobility.Install (ueNodes);

  // Physical layer.
  auto enbDev = DynamicCast<cv2x_LteEnbNetDevice> (lteHelper->InstallEnbDevice (enbNode).Get (0));
  auto ueDevs = lteHelper->InstallUeDevice (ueNodes);

  // Network layer.
  InternetStackHelper internet;
  internet.Install (ueNodes);
  epcHelper->AssignUeIpv4Address (ueDevs);

  auto ueDev = DynamicCast<cv2x_LteUeNetDevice> (ueDevs.Get (0));
  std::map< uint8_t, Ptr<cv2x_ComponentCarrierUe> > ueCcMap = ueDev->GetCcMap ();
  for (auto it: ueCcMap)
    {
      std::cerr << "Assign " << it.second->GetDlEarfcn() << std::endl;
      DynamicCast<cv2x_LteUeNetDevice> (ueDevs.Get (it.first))->SetDlEarfcn (it.second->GetDlEarfcn());
    }

  // Enable Idle mode cell selection.
  lteHelper->Attach (ueDevs);

  // Connect to trace sources in UEs
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUeRrc/StateTransition",
                   MakeCallback (&cv2x_LteSecondaryCellSelectionTestCase::StateTransitionCallback,
                                 this));
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUeRrc/InitialSecondaryCellSelectionEndOk",
                   MakeCallback (&cv2x_LteSecondaryCellSelectionTestCase::InitialSecondaryCellSelectionEndOkCallback,
                                 this));
  Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUeRrc/ConnectionEstablished",
                   MakeCallback (&cv2x_LteSecondaryCellSelectionTestCase::ConnectionEstablishedCallback,
                                 this));

  // Run simulation.
  Simulator::Stop (Seconds (2.0));
  Simulator::Run ();

  for (auto &it: enbDev->GetCcMap ())
    {
      ueDev = DynamicCast<cv2x_LteUeNetDevice> (ueDevs.Get (it.first));
      uint16_t expectedCellId = it.second->GetCellId ();
      uint16_t actualCellId = ueDev->GetRrc ()->GetCellId ();
      NS_TEST_ASSERT_MSG_EQ (expectedCellId, actualCellId, "IMSI " << ueDev->GetImsi () << " has attached to an unexpected cell");

      NS_TEST_ASSERT_MSG_EQ (m_lastState.at (ueDev->GetImsi ()),
                             cv2x_LteUeRrc::CONNECTED_NORMALLY,
                             "UE " << ueDev->GetImsi ()
                                   << " is not at CONNECTED_NORMALLY state");
    }

  // Destroy simulator.
  Simulator::Destroy ();
} // end of void cv2x_LteSecondaryCellSelectionTestCase::DoRun ()


void
cv2x_LteSecondaryCellSelectionTestCase::StateTransitionCallback (
  std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti,
  cv2x_LteUeRrc::State oldState, cv2x_LteUeRrc::State newState)
{
  NS_LOG_FUNCTION (this << imsi << cellId << rnti << oldState << newState);
  m_lastState[imsi] = newState;
}


void
cv2x_LteSecondaryCellSelectionTestCase::InitialSecondaryCellSelectionEndOkCallback (
  std::string context, uint64_t imsi, uint16_t cellId)
{
  NS_LOG_FUNCTION (this << imsi << cellId);
}

void
cv2x_LteSecondaryCellSelectionTestCase::ConnectionEstablishedCallback (
  std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << imsi << cellId << rnti);
}

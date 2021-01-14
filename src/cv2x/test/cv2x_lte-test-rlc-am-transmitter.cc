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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#include "ns3/simulator.h"
#include "ns3/log.h"

#include "ns3/cv2x_lte-rlc-header.h"
#include "ns3/cv2x_lte-rlc-am.h"

#include "cv2x_lte-test-rlc-am-transmitter.h"
#include "cv2x_lte-test-entities.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("cv2x_LteRlcAmTransmitterTest");

/**
 * TestSuite 4.1.1 RLC AM: Only transmitter
 */

cv2x_LteRlcAmTransmitterTestSuite::cv2x_LteRlcAmTransmitterTestSuite ()
  : TestSuite ("lte-rlc-am-transmitter", SYSTEM)
{
  // LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);
  // LogComponentEnable ("cv2x_LteRlcAmTransmitterTest", logLevel);

  AddTestCase (new cv2x_LteRlcAmTransmitterOneSduTestCase ("One SDU, one PDU"), TestCase::QUICK);
  AddTestCase (new cv2x_LteRlcAmTransmitterSegmentationTestCase ("Segmentation"), TestCase::QUICK);
  AddTestCase (new cv2x_LteRlcAmTransmitterConcatenationTestCase ("Concatenation"), TestCase::QUICK);
  AddTestCase (new cv2x_LteRlcAmTransmitterReportBufferStatusTestCase ("ReportBufferStatus primitive"), TestCase::QUICK);

}

static cv2x_LteRlcAmTransmitterTestSuite lteRlcAmTransmitterTestSuite;


cv2x_LteRlcAmTransmitterTestCase::cv2x_LteRlcAmTransmitterTestCase (std::string name)
  : TestCase (name)
{
}

cv2x_LteRlcAmTransmitterTestCase::~cv2x_LteRlcAmTransmitterTestCase ()
{
}

void
cv2x_LteRlcAmTransmitterTestCase::DoRun (void)
{
  // LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_ALL);
  // LogComponentEnable ("cv2x_LteRlcAmTransmitterTest", logLevel);
  // LogComponentEnable ("LteTestEntities", logLevel);
  // LogComponentEnable ("cv2x_LteRlc", logLevel);
  // LogComponentEnable ("cv2x_LteRlcAm", logLevel);
  // LogComponentEnable ("cv2x_LteRlcHeader", logLevel);

  uint16_t rnti = 1111;
  uint8_t lcid = 222;

  Packet::EnablePrinting ();

  // Create topology

  // Create transmission PDCP test entity
  txPdcp = CreateObject<cv2x_LteTestPdcp> ();

  // Create transmission RLC entity
  txRlc = CreateObject<cv2x_LteRlcAm> ();
  txRlc->SetRnti (rnti);
  txRlc->SetLcId (lcid);

  // Create transmission MAC test entity
  txMac = CreateObject<cv2x_LteTestMac> ();
  txMac->SetRlcHeaderType (cv2x_LteTestMac::AM_RLC_HEADER);

  // Connect SAPs: PDCP (TX) <-> RLC (Tx) <-> MAC (Tx)
  txPdcp->SetLteRlcSapProvider (txRlc->GetLteRlcSapProvider ());
  txRlc->SetLteRlcSapUser (txPdcp->GetLteRlcSapUser ());

  txRlc->SetLteMacSapProvider (txMac->GetLteMacSapProvider ());
  txMac->SetLteMacSapUser (txRlc->GetLteMacSapUser ());

}

void
cv2x_LteRlcAmTransmitterTestCase::CheckDataReceived (Time time, std::string shouldReceived, std::string assertMsg)
{
  Simulator::Schedule (time, &cv2x_LteRlcAmTransmitterTestCase::DoCheckDataReceived, this, shouldReceived, assertMsg);
}

void
cv2x_LteRlcAmTransmitterTestCase::DoCheckDataReceived (std::string shouldReceived, std::string assertMsg)
{
  NS_TEST_ASSERT_MSG_EQ (shouldReceived, txMac->GetDataReceived (), assertMsg);
}


/**
 * Test 4.1.1.1 One SDU, One PDU
 */
cv2x_LteRlcAmTransmitterOneSduTestCase::cv2x_LteRlcAmTransmitterOneSduTestCase (std::string name)
  : cv2x_LteRlcAmTransmitterTestCase (name)
{
}

cv2x_LteRlcAmTransmitterOneSduTestCase::~cv2x_LteRlcAmTransmitterOneSduTestCase ()
{
}

void
cv2x_LteRlcAmTransmitterOneSduTestCase::DoRun (void)
{
  // Create topology
  cv2x_LteRlcAmTransmitterTestCase::DoRun ();

  //
  // a) One SDU generates one PDU
  //

  // PDCP entity sends data
  txPdcp->SendData (Seconds (0.100), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

  txMac->SendTxOpportunity (Seconds (0.150), 30);
  CheckDataReceived (Seconds (0.200), "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "SDU is not OK");

  Simulator::Stop (Seconds (0.3));
  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * Test 4.1.1.2 Segmentation (One SDU => n PDUs)
 */
cv2x_LteRlcAmTransmitterSegmentationTestCase::cv2x_LteRlcAmTransmitterSegmentationTestCase (std::string name)
  : cv2x_LteRlcAmTransmitterTestCase (name)
{
}

cv2x_LteRlcAmTransmitterSegmentationTestCase::~cv2x_LteRlcAmTransmitterSegmentationTestCase ()
{
}

void
cv2x_LteRlcAmTransmitterSegmentationTestCase::DoRun (void)
{
  // Create topology
  cv2x_LteRlcAmTransmitterTestCase::DoRun ();

  //
  // b) Segmentation: one SDU generates n PDUs
  //

  // PDCP entity sends data
  txPdcp->SendData (Seconds (0.100), "ABCDEFGHIJKLMNOPQRSTUVWXYZZ");

  // MAC entity sends small TxOpp to RLC entity generating four segments
  txMac->SendTxOpportunity (Seconds (0.150), 12);
  CheckDataReceived (Seconds (0.200), "ABCDEFGH", "Segment #1 is not OK");

  txMac->SendTxOpportunity (Seconds (0.250), 12);
  CheckDataReceived (Seconds (0.300), "IJKLMNOP", "Segment #2 is not OK");

  txMac->SendTxOpportunity (Seconds (0.350), 12);
  CheckDataReceived (Seconds (0.400), "QRSTUVWX", "Segment #3 is not OK");

  txMac->SendTxOpportunity (Seconds (0.450), 7);
  CheckDataReceived (Seconds (0.500), "YZZ", "Segment #4 is not OK");

  Simulator::Stop (Seconds (0.6));
  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * Test 4.1.1.3 Concatenation (n SDUs => One PDU)
 */
cv2x_LteRlcAmTransmitterConcatenationTestCase::cv2x_LteRlcAmTransmitterConcatenationTestCase (std::string name)
  : cv2x_LteRlcAmTransmitterTestCase (name)
{
}

cv2x_LteRlcAmTransmitterConcatenationTestCase::~cv2x_LteRlcAmTransmitterConcatenationTestCase ()
{
}

void
cv2x_LteRlcAmTransmitterConcatenationTestCase::DoRun (void)
{
  // Create topology
  cv2x_LteRlcAmTransmitterTestCase::DoRun ();

  //
  // c) Concatenation: n SDUs generate one PDU
  //

  // PDCP entity sends three data packets
  txPdcp->SendData (Seconds (0.100), "ABCDEFGH");
  txPdcp->SendData (Seconds (0.150), "IJKLMNOPQR");
  txPdcp->SendData (Seconds (0.200), "STUVWXYZ");

  // MAC entity sends TxOpp to RLC entity generating only one concatenated PDU

  txMac->SendTxOpportunity (Seconds (0.250), 33);
  CheckDataReceived (Seconds (0.300), "ABCDEFGHIJKLMNOPQRSTUVWXYZ", "Concatenation is not OK");

  Simulator::Stop (Seconds (0.4));
  Simulator::Run ();
  Simulator::Destroy ();
}

/**
 * Test 4.1.1.4 Report Buffer Status (test primitive parameters)
 */
cv2x_LteRlcAmTransmitterReportBufferStatusTestCase::cv2x_LteRlcAmTransmitterReportBufferStatusTestCase (std::string name)
  : cv2x_LteRlcAmTransmitterTestCase (name)
{
}

cv2x_LteRlcAmTransmitterReportBufferStatusTestCase::~cv2x_LteRlcAmTransmitterReportBufferStatusTestCase ()
{
}

void
cv2x_LteRlcAmTransmitterReportBufferStatusTestCase::DoRun (void)
{
  // Create topology
  cv2x_LteRlcAmTransmitterTestCase::DoRun ();

  //
  // d) Test the parameters of the ReportBufferStatus primitive
  //

//   txMac->SendTxOpportunity (Seconds (0.1), (2+2) + (10+6));

  // PDCP entity sends data
  txPdcp->SendData (Seconds (0.100), "ABCDEFGHIJ"); // 10
  txPdcp->SendData (Seconds (0.150), "KLMNOPQRS");  // 9
  txPdcp->SendData (Seconds (0.200), "TUVWXYZ");    // 7

  txMac->SendTxOpportunity (Seconds (0.250), (4+2) + (10+6));
  CheckDataReceived (Seconds (0.300), "ABCDEFGHIJKLMNOP", "SDU #1 is not OK");

  txPdcp->SendData (Seconds (0.350), "ABCDEFGH");     // 8
  txPdcp->SendData (Seconds (0.400), "IJKLMNOPQRST"); // 12
  txPdcp->SendData (Seconds (0.450), "UVWXYZ");       // 6

  txMac->SendTxOpportunity (Seconds (0.500), 4 + 3);
  CheckDataReceived (Seconds (0.550), "QRS", "SDU #2 is not OK");

  txPdcp->SendData (Seconds (0.600), "ABCDEFGH");     // 8
  txPdcp->SendData (Seconds (0.650), "IJKLMNOPQRST"); // 12
  txPdcp->SendData (Seconds (0.700), "UVWXYZ");       // 6

  txPdcp->SendData (Seconds (0.750), "ABCDEFGHIJ");   // 10
  txPdcp->SendData (Seconds (0.800), "KLMNOPQRST");   // 10
  txPdcp->SendData (Seconds (0.850), "UVWXYZ");       // 6

  txMac->SendTxOpportunity (Seconds (0.900), 4 + 7);
  CheckDataReceived (Seconds (0.950), "TUVWXYZ", "SDU #3 is not OK");

  txMac->SendTxOpportunity (Seconds (1.000), (4+2) + (8+2));
  CheckDataReceived (Seconds (1.050), "ABCDEFGHIJ", "SDU #4 is not OK");

  txPdcp->SendData (Seconds (1.100), "ABCDEFGHIJ");   // 10
  txPdcp->SendData (Seconds (1.150), "KLMNOPQRSTU");  // 11
  txPdcp->SendData (Seconds (1.200), "VWXYZ");        // 5

  txMac->SendTxOpportunity (Seconds (1.250), 4 + 3);
  CheckDataReceived (Seconds (1.300), "KLM", "SDU #5 is not OK");

  txMac->SendTxOpportunity (Seconds (1.350), 4 + 3);
  CheckDataReceived (Seconds (1.400), "NOP", "SDU #6 is not OK");

  txMac->SendTxOpportunity (Seconds (1.450), 4 + 4);
  CheckDataReceived (Seconds (1.500), "QRST", "SDU #7 is not OK");

  txMac->SendTxOpportunity (Seconds (1.550), (4+2+1+2+1+2+1) + (6+8+12+6+10+10+3));
  CheckDataReceived (Seconds (1.600), "UVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVW", "SDU #8 is not OK");

  txMac->SendTxOpportunity (Seconds (1.650), (4+2+1+2) + (3+10+10+7));
  CheckDataReceived (Seconds (1.700), "XYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "SDU #9 is not OK");

  Simulator::Stop (Seconds (2));
  Simulator::Run ();
  Simulator::Destroy ();
}

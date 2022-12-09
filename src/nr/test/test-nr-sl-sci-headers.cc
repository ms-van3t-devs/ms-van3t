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

#include "test-nr-sl-sci-headers.h"
#include "ns3/core-module.h"
#include <ns3/packet.h>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestNrSlSciHeaders");

/*
 * Test Suite
 */
NrSlSciHeadersTestSuite::NrSlSciHeadersTestSuite ()
  : TestSuite ("nr-sl-sci-headers", UNIT)
{

  //Test only including the mandatory fields
  NrSlSciF1aHeader sciF1a;
  sciF1a.SetPriority (1);
  sciF1a.SetMcs (12);
  sciF1a.SetSciStage2Format (NrSlSciF1aHeader::SciFormat2A);
  sciF1a.SetSlResourceReservePeriod (200);
  sciF1a.SetTotalSubChannels (1);
  sciF1a.SetIndexStartSubChannel (0);
  sciF1a.SetLengthSubChannel (1);
  sciF1a.SetSlMaxNumPerReserve (1);

  uint16_t sizeSciF1A = 1 + 1 + 1 + 2 + 2 + 1 + 1 + 1; // 10 bytes

  AddTestCase (new NrSlSciF1aTestCase (sciF1a, sizeSciF1A));

  //Test including the mandatory fields and 2 optional field
  sciF1a.SetSlMaxNumPerReserve (2);
  sciF1a.SetGapReTx1 (2);
  sciF1a.SetIndexStartSbChReTx1 (0);

  sizeSciF1A = sizeSciF1A + 2; // 12 bytes

  AddTestCase (new NrSlSciF1aTestCase (sciF1a, sizeSciF1A));

  //Test including the mandatory fields and 4 optional fields
  sciF1a.SetSlMaxNumPerReserve (3);
  sciF1a.SetGapReTx1 (2);
  sciF1a.SetIndexStartSbChReTx1 (0);
  sciF1a.SetGapReTx2 (3);
  sciF1a.SetIndexStartSbChReTx2 (0);

  sizeSciF1A = sizeSciF1A + 2; // 14 bytes

  AddTestCase (new NrSlSciF1aTestCase (sciF1a, sizeSciF1A));

  //SCI Format 2A tests
  uint16_t sizeSciF2a = 5; //5 bytes fixed
  NrSlSciF2aHeader sciF2a;

  sciF2a.SetHarqId (5);
  sciF2a.SetNdi (1);
  sciF2a.SetRv (0);
  sciF2a.SetSrcId (1);
  sciF2a.SetDstId (255);
  sciF2a.SetCastType (NrSlSciF2aHeader::Broadcast);
  sciF2a.SetCsiReq (1);

  AddTestCase (new NrSlSciF2aTestCase (sciF2a, sizeSciF2a));
} // end of NrSlSciHeadersTestSuite::NrSlSciHeadersTestSuite ()


static NrSlSciHeadersTestSuite g_nrSlSciHeadersTestSuite;

/*
 * Test Case SCI Format 1A
 */

std::string
NrSlSciF1aTestCase::BuildNameString (const NrSlSciF1aHeader &sciF1a, uint16_t expectedHeaderSize)
{
  std::ostringstream oss;

  oss << " Checked SCI format 1A : Priority " << +sciF1a.GetPriority ();
  oss << " MCS " << +sciF1a.GetMcs ();
  oss << " SCI Stage 2 Format " << +sciF1a.GetSciStage2Format ();
  oss << " Resource reservation period " << +sciF1a.GetSlResourceReservePeriod ();
  oss << " Total number of Subchannels " << +sciF1a.GetTotalSubChannels ();
  oss << " Index starting Subchannel " << +sciF1a.GetIndexStartSubChannel ();
  oss << " Total number of allocated Subchannels " << +sciF1a.GetLengthSubChannel ();
  oss << " Maximum number of reservations " << +sciF1a.GetSlMaxNumPerReserve ();
  oss << " First retransmission gap in slots " << +sciF1a.GetGapReTx1 ();
  oss << " Second retransmission gap in slots " << +sciF1a.GetGapReTx2 () << "\n";
  return oss.str ();
}

NrSlSciF1aTestCase::NrSlSciF1aTestCase (NrSlSciF1aHeader sciF1a, uint16_t expectedHeaderSize)
  : TestCase (BuildNameString (sciF1a, expectedHeaderSize))
{
  NS_LOG_FUNCTION (this << GetName ());
  m_sciF1a = sciF1a;
  m_expectedHeaderSize = expectedHeaderSize;
}


NrSlSciF1aTestCase::~NrSlSciF1aTestCase ()
{
  NS_LOG_FUNCTION (this << GetName ());
}


void
NrSlSciF1aTestCase::DoRun ()
{
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (m_sciF1a);

  //deserialized
  NrSlSciF1aHeader deSerSciF1a;
  p->RemoveHeader (deSerSciF1a);

  NS_TEST_ASSERT_MSG_EQ (deSerSciF1a, m_sciF1a,
                         "SCI format 1A deserialized version is different than the one we serialized");
  NS_TEST_ASSERT_MSG_EQ (deSerSciF1a.GetSerializedSize (), m_expectedHeaderSize,
                         "SCI format 1A header size is different than the expected size in bytes");

} // end of void NrSlSciF1aTestCase::DoRun ()



/*
 * Test Case SCI Format 02
 */

std::string
NrSlSciF2aTestCase::BuildNameString (const NrSlSciF2aHeader &sciF2a, uint16_t expectedHeaderSize)
{
  std::ostringstream oss;

  oss << " Checked SCI format 2A : HARQ process id " << +sciF2a.GetHarqId ();
  oss << " New data indicator " << +sciF2a.GetNdi ();
  oss << " Redundancy version " << +sciF2a.GetRv ();
  oss << " Source layer 2 Id " << +sciF2a.GetSrcId ();
  oss << " Destination layer 2 id " << sciF2a.GetDstId ();
  oss << " Cast type indicator " << sciF2a.GetCastType ();
  oss << " Channel state information request " << +sciF2a.GetCsiReq ();

  return oss.str ();
}

NrSlSciF2aTestCase::NrSlSciF2aTestCase (NrSlSciF2aHeader sciF2a, uint16_t expectedHeaderSize)
  : TestCase (BuildNameString (sciF2a, expectedHeaderSize))
{
  NS_LOG_FUNCTION (this << GetName ());
  m_sciF2a = sciF2a;
  m_expectedHeaderSize = expectedHeaderSize;
}


NrSlSciF2aTestCase::~NrSlSciF2aTestCase ()
{
  NS_LOG_FUNCTION (this << GetName ());
}


void
NrSlSciF2aTestCase::DoRun ()
{
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (m_sciF2a);

  //deserialized
  NrSlSciF2aHeader deSerSciF02;
  p->RemoveHeader (deSerSciF02);

  NS_TEST_ASSERT_MSG_EQ (deSerSciF02, m_sciF2a,
                         "SCI format 02 deserialized version is different than the one we serialized");
  NS_TEST_ASSERT_MSG_EQ (deSerSciF02.GetSerializedSize (), m_expectedHeaderSize,
                         "SCI format 02 header size is different than the expected size in bytes");

} // end of void NrSlSciF2aTestCase::DoRun ()


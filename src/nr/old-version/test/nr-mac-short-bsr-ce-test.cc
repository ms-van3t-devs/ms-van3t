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
#include <ns3/test.h>
#include <ns3/nr-mac-short-bsr-ce.h>

/**
 * \file test-nr-mac-vs-header.cc
 * \ingroup test
 * \brief Unit-testing for the variable-size MAC header, DL and UL
 *
 */
namespace ns3 {

/**
 * \brief
 */
class NrMacShortBsrCeTest : public TestCase
{
public:
  /**
   * \brief Constructor
   * \param name Name of the test
   */
  NrMacShortBsrCeTest (const std::string &name)
    : TestCase (name)
  {}

private:
  virtual void DoRun (void) override;
};


void
NrMacShortBsrCeTest::DoRun ()
{
  Ptr<Packet> applicationData = Create<Packet> ();

  Packet::EnablePrinting ();
  Packet::EnableChecking ();

  Ptr<Packet> pdu = Create<Packet> ();

  {
    NrMacShortBsrCe bsr;
    bsr.m_bufferSizeLevel_0 = NrMacShortBsrCe::FromBytesToLevel (12);
    bsr.m_bufferSizeLevel_1 = NrMacShortBsrCe::FromBytesToLevel (400);
    bsr.m_bufferSizeLevel_2 = NrMacShortBsrCe::FromBytesToLevel (5400);
    bsr.m_bufferSizeLevel_3 = NrMacShortBsrCe::FromBytesToLevel (500000);

    pdu->AddHeader (bsr);
  }

  std::cout << " the pdu is: ";
  pdu->Print (std::cout);
  std::cout << std::endl;

  // Inside our PDU, there is one subPDU composed by our header: { [HEADER] }
  // Let's try to deserialize it, checking if it's the same

  {
    NrMacShortBsrCe bsr;

    pdu->RemoveHeader (bsr);

    std::cout << "the SDU is: ";
    bsr.Print (std::cout);
    std::cout << std::endl;

    NS_TEST_ASSERT_MSG_EQ (bsr.m_bufferSizeLevel_0, NrMacShortBsrCe::FromBytesToLevel (12),
                           "Deserialize failed for BufferLevel");
    NS_TEST_ASSERT_MSG_EQ (bsr.m_bufferSizeLevel_1, NrMacShortBsrCe::FromBytesToLevel (400),
                           "Deserialize failed for BufferLevel");
    NS_TEST_ASSERT_MSG_EQ (bsr.m_bufferSizeLevel_2, NrMacShortBsrCe::FromBytesToLevel (5400),
                           "Deserialize failed for BufferLevel");
    NS_TEST_ASSERT_MSG_EQ (bsr.m_bufferSizeLevel_3, NrMacShortBsrCe::FromBytesToLevel (500000),
                           "Deserialize failed for BufferLevel");
  }
}

class NrMacShortBsrCeTestSuite : public TestSuite
{
public:
  NrMacShortBsrCeTestSuite () : TestSuite ("nr-mac-short-bsr-ce-test", UNIT)
  {
    AddTestCase (new NrMacShortBsrCeTest ("Short BSR CE test"), QUICK);
  }
};

static NrMacShortBsrCeTestSuite nrMacShortBsrCeTestSuite;

}  // namespace ns3

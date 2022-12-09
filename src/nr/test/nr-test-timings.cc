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
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/eps-bearer-tag.h"
#include "ns3/nr-module.h"
#include "ns3/antenna-module.h"
#include <unordered_map>

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
using namespace ns3;

/**
  * \file test-timings.cc
  * \ingroup test
  *
  * \brief Check numerology timings. The test, that is run for every numerology,
  * checks that the slot number of certain events is the same as the one
  * pre-recorded by an expert, that spent time in checking that such timing is
  * correct. We currently check only RAR and DL_DCI messages, improvements are
  * more than welcome.
  */

static uint32_t packetSize = 21;

static const std::unordered_map<NrControlMessage::messageType, bool, std::hash<int> > messageLog =
{
  { NrControlMessage::messageType::UL_DCI,      false },
  { NrControlMessage::messageType::DL_DCI, false },
  { NrControlMessage::messageType::DL_CQI,   false },
  { NrControlMessage::messageType::MIB,      false },
  { NrControlMessage::messageType::SIB1,     false },
  { NrControlMessage::messageType::RACH_PREAMBLE, false },
  { NrControlMessage::messageType::RAR,      false },
  { NrControlMessage::messageType::BSR,      false },
  { NrControlMessage::messageType::DL_HARQ,  false },
  { NrControlMessage::messageType::SR,       false },
};

typedef std::unordered_map<NrControlMessage::messageType, uint64_t, std::hash<int> > TypeToResult;
typedef std::unordered_map<uint32_t, TypeToResult> NumerologyToType;

class NrTimingsTest : public TestCase
{
public:
  NrTimingsTest (const std::string &name, uint32_t numerology, bool verbose);
  virtual ~NrTimingsTest ();

private:
  virtual void DoRun (void);
  void GnbPhyTx (SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                 uint8_t ccId, Ptr<const NrControlMessage> msg);
  void GnbPhyRx (SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                 uint8_t ccId, Ptr<const NrControlMessage> msg);

  void GnbMacTx (SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                 uint8_t ccId, Ptr<const NrControlMessage> msg);
  void GnbMacRx (SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                 uint8_t ccId, Ptr<const NrControlMessage> msg);

  void UePhyTx (SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                uint8_t ccId, Ptr<const NrControlMessage> msg);
  void UePhyRx (SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                uint8_t ccId, Ptr<const NrControlMessage> msg);

  void UeMacTx (SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                uint8_t ccId, Ptr<const NrControlMessage> msg);
  void UeMacRx (SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                uint8_t ccId, Ptr<const NrControlMessage> msg);

  uint32_t m_numerology;
  bool verbose {false};
};

NrTimingsTest::NrTimingsTest (const std::string &name, uint32_t numerology, bool verbose)
  : TestCase (name),
    m_numerology (numerology),
    verbose (verbose)
{}

NrTimingsTest::~NrTimingsTest ()
{}

static void
SendPacket (const Ptr<NetDevice> &device, const Address& addr)
{
  Ipv4Header header;
  NS_ASSERT (packetSize > header.GetSerializedSize ());
  Ptr<Packet> pkt = Create<Packet> (packetSize - header.GetSerializedSize ());
  header.SetProtocol (0x06);
  EpsBearerTag tag (1, 1);
  pkt->AddPacketTag (tag);
  pkt->AddHeader (header);
  device->Send (pkt, addr, Ipv4L3Protocol::PROT_NUMBER);
}

static const std::unordered_map <NrControlMessage::messageType, std::string, std::hash<int> > TYPE_TO_STRING =
{
  { NrControlMessage::messageType::UL_DCI,   "UL_DCI" },
  { NrControlMessage::messageType::DL_DCI,   "DL_DCI" },
  { NrControlMessage::messageType::DL_CQI,   "DL_CQI" },
  { NrControlMessage::messageType::MIB,      "MIB" },
  { NrControlMessage::messageType::SIB1,     "SIB1" },
  { NrControlMessage::messageType::RACH_PREAMBLE, "RACH_PREAMBLE" },
  { NrControlMessage::messageType::RAR,      "RAR" },
  { NrControlMessage::messageType::BSR,      "BSR" },
  { NrControlMessage::messageType::DL_HARQ,  "DL_HARQ" },
  { NrControlMessage::messageType::SR,       "SR" },
};

void
NrTimingsTest::GnbPhyTx (SfnSf sfn, [[maybe_unused]]  uint16_t nodeId, [[maybe_unused]]  uint16_t rnti, [[maybe_unused]]  uint8_t ccId, Ptr<const NrControlMessage> msg)
{
  static NumerologyToType res =
  {
    {
      4, {
           { NrControlMessage::RAR, SfnSf (1, 6, 4, 4).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 0, 2, 4).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 4).Normalize () },
         },
    },
    {
      3, {
           { NrControlMessage::RAR, SfnSf (1, 6, 4, 3).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 0, 2, 3).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 3).Normalize () },
         },

    },
    {
      2, {
           { NrControlMessage::RAR, SfnSf (1, 7, 0, 2).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 0, 2, 2).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 2).Normalize () },
         },
    },
    {
      1, {
           { NrControlMessage::RAR, SfnSf (1, 8, 0, 1).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 1, 0, 1).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 1).Normalize () },
         },
    },
    {
      0, {
           { NrControlMessage::RAR, SfnSf (2, 0, 0, 0).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 2, 0, 0).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 0).Normalize () },
         },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize () << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType ());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize (),
                                 "The message type " << TYPE_TO_STRING.at (msg->GetMessageType ()) <<
                                 " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
                                 sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::GnbPhyRx (SfnSf sfn, [[maybe_unused]] uint16_t nodeId, [[maybe_unused]] uint16_t rnti, [[maybe_unused]] uint8_t ccId, Ptr<const NrControlMessage> msg)
{

  static NumerologyToType res =
  {
    {
      4, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 4).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 0, 4, 4).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 0, 5, 4).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 4).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 4).Normalize () },
         },
    },
    {
      3, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 3).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 0, 4, 3).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 0, 5, 3).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 3).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 3).Normalize () },
         },
    },
    {
      2, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 2).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 1, 0, 2).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 1, 1, 2).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 2).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 2).Normalize () },
         },
    },
    {
      1, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 1).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 2, 0, 1).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 2, 1, 1).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 1).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 1).Normalize () },
         },
    },
    {
      0, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 7, 0, 0).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 4, 0, 0).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 5, 0, 0).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 0).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 0).Normalize () },
         },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize () << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType ());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize (),
                                 "The message type " << TYPE_TO_STRING.at (msg->GetMessageType ()) <<
                                 " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
                                 sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::GnbMacTx (SfnSf sfn, [[maybe_unused]] uint16_t nodeId, [[maybe_unused]] uint16_t rnti, [[maybe_unused]] uint8_t ccId, Ptr<const NrControlMessage> msg)
{

  static NumerologyToType res =
  {
    {
      4, { { NrControlMessage::RAR, SfnSf (1, 6, 2, 4).Normalize () } },
    },
    {
      3, { { NrControlMessage::RAR, SfnSf (1, 6, 2, 3).Normalize () } },
    },
    {
      2, { { NrControlMessage::RAR, SfnSf (1, 6, 2, 2).Normalize () } },
    },
    {
      1, { { NrControlMessage::RAR, SfnSf (1, 7, 0, 1).Normalize () } },
    },
    {
      0, { { NrControlMessage::RAR, SfnSf (1, 8, 0, 0).Normalize () } },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize () << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType ());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize (),
                                 "The message type " << TYPE_TO_STRING.at (msg->GetMessageType ()) <<
                                 " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
                                 sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::GnbMacRx (SfnSf sfn, [[maybe_unused]] uint16_t nodeId, [[maybe_unused]] uint16_t rnti, [[maybe_unused]] uint8_t ccId, Ptr<const NrControlMessage> msg)
{

  static NumerologyToType res =
  {
    {
      4, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 4).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 0, 5, 4).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 0, 6, 4).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 4).Normalize () },
         },
    },
    {
      3, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 3).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 0, 5, 3).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 0, 6, 3).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 3).Normalize () },
         },
    },
    {
      2, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 2).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 1, 1, 2).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 1, 2, 2).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 2).Normalize () },
         },
    },
    {
      1, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 1).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 2, 1, 1).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 3, 0, 1).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 1).Normalize () },
         },
    },
    {
      0, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 7, 0, 0).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 5, 0, 0).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 6, 0, 0).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 0).Normalize () },
         },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize () << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType ());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize (),
                                 "The message type " << TYPE_TO_STRING.at (msg->GetMessageType ()) <<
                                 " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
                                 sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " not found in the result map;" << std::endl;
    }
}

// UE

void
NrTimingsTest::UePhyTx (SfnSf sfn, [[maybe_unused]] uint16_t nodeId, [[maybe_unused]] uint16_t rnti, [[maybe_unused]] uint8_t ccId, Ptr<const NrControlMessage> msg)
{

  static NumerologyToType res =
  {
    {
      4, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 4).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 0, 4, 4).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 0, 5, 4).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 4).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 4).Normalize () },
         },
    },
    {
      3, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 3).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 0, 4, 3).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 0, 5, 3).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 3).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 3).Normalize () },
         },
    },
    {
      2, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 2).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 1, 0, 2).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 1, 1, 2).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 2).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 2).Normalize () },
         },
    },
    {
      1, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 1, 1).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 2, 0, 1).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 2, 1, 1).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 1).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 1).Normalize () },
         },
    },
    {
      0, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 7, 0, 0).Normalize () },
           { NrControlMessage::DL_HARQ, SfnSf (40, 4, 0, 0).Normalize () },
           { NrControlMessage::DL_CQI, SfnSf (40, 5, 0, 0).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 0).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 0).Normalize () },
         },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize () << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType ());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize (),
                                 "The message type " << TYPE_TO_STRING.at (msg->GetMessageType ()) <<
                                 " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
                                 sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::UePhyRx (SfnSf sfn, [[maybe_unused]] uint16_t nodeId, [[maybe_unused]] uint16_t rnti, [[maybe_unused]] uint8_t ccId, Ptr<const NrControlMessage> msg)
{

  static NumerologyToType res =
  {
    {
      4, {
           { NrControlMessage::RAR, SfnSf (1, 6, 5, 4).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 0, 2, 4).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 4).Normalize () },
         },
    },
    {
      3, {
           { NrControlMessage::RAR, SfnSf (1, 6, 5, 3).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 0, 2, 3).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 3).Normalize () },
         },
    },
    {
      2, {
           { NrControlMessage::RAR, SfnSf (1, 7, 1, 2).Normalize () } ,
           { NrControlMessage::DL_DCI, SfnSf (40, 0, 2, 2).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 2).Normalize () },
         },
    },
    {
      1, {
           { NrControlMessage::RAR, SfnSf (1, 8, 1, 1).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 1, 0, 1).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 1).Normalize () },
         },
    },
    {
      0, {
           { NrControlMessage::RAR, SfnSf (2, 1, 0, 0).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 2, 0, 0).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 0).Normalize () },
         },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize () << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType ());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize (),
                                 "The message type " << TYPE_TO_STRING.at (msg->GetMessageType ()) <<
                                 " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
                                 sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::UeMacTx (SfnSf sfn, [[maybe_unused]] uint16_t nodeId, [[maybe_unused]] uint16_t rnti, [[maybe_unused]] uint8_t ccId, Ptr<const NrControlMessage> msg)
{

  static NumerologyToType res =
  {
    {
      4, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 0, 4).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 4).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 4).Normalize () },
         },
    },
    {
      3, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 0, 3).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 3).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 3).Normalize () },
         },
    },
    {
      2, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 0, 2).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 2).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 2).Normalize () },
         },
    },
    {
      1, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 0, 1).Normalize () },
          //{ NrControlMessage::SR, SfnSf (80, 0, 0, 1).Normalize () },
          //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 1).Normalize () },
         },
    },
    {
      0, {
           { NrControlMessage::RACH_PREAMBLE, SfnSf (1, 6, 0, 0).Normalize () },
           //{ NrControlMessage::SR, SfnSf (80, 0, 0, 0).Normalize () },
           //{ NrControlMessage::BSR, SfnSf (80, 0, 0, 0).Normalize () },
         },
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize () << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType ());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize (),
                                 "The message type " << TYPE_TO_STRING.at (msg->GetMessageType ()) <<
                                 " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
                                 sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " not found in the result map;" << std::endl;
    }
}

void
NrTimingsTest::UeMacRx (SfnSf sfn, [[maybe_unused]] uint16_t nodeId, [[maybe_unused]] uint16_t rnti, [[maybe_unused]] uint8_t ccId, Ptr<const NrControlMessage> msg)
{

  static NumerologyToType res =
  {
    {
      4, {
           { NrControlMessage::RAR, SfnSf (1, 6, 5, 4).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 0, 2, 4).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 4).Normalize () },
         },
    },
    {
      3, {
           { NrControlMessage::RAR, SfnSf (1, 6, 5, 3).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 0, 2, 3).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 3).Normalize () },
         },
    },
    {
      2, {
           { NrControlMessage::RAR, SfnSf (1, 7, 1, 2).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 0, 2, 2).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 2).Normalize () },
         },
    },
    {
      1, {
           { NrControlMessage::RAR, SfnSf (1, 8, 1, 1).Normalize () },
           { NrControlMessage::DL_DCI, SfnSf (40, 1, 0, 1).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 1).Normalize () },
         },
    },
    {
      0, {
           { NrControlMessage::RAR, SfnSf (2, 1, 0, 0).Normalize ()  },
           { NrControlMessage::DL_DCI, SfnSf (40, 2, 0, 0).Normalize () },
           //{ NrControlMessage::DCI, SfnSf (80, 0, 2, 0).Normalize () },
         }
    },
  };

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " at " << sfn << " numerology " << m_numerology
                << " slot count " << sfn.Normalize () << " " << Simulator::Now () << std::endl;
    }

  auto numMap = res.find (m_numerology);
  if (numMap != res.end ())
    {
      auto resMap = numMap->second.find (msg->GetMessageType ());
      if (resMap != numMap->second.end ())
        {
          uint64_t slotN = resMap->second;
          NS_TEST_ASSERT_MSG_EQ (slotN, sfn.Normalize (),
                                 "The message type " << TYPE_TO_STRING.at (msg->GetMessageType ()) <<
                                 " was supposed to be sent at slot " << slotN << " but instead we sent it at " <<
                                 sfn.Normalize () << " in numerology " << m_numerology);
          return;
        }
    }

  if (verbose && messageLog.at (msg->GetMessageType ()))
    {
      std::cerr << __func__ << ": " << TYPE_TO_STRING.at (msg->GetMessageType ())
                << " not found in the result map;" << std::endl;
    }
}

// Ugly pre-processor macro, to speed up writing. The best way would be to use
// static functions... so please forget them, and remember that they work
// only here in the DoRun function, as it is all hard-coded
#define GET_ENB_PHY(X, Y) nrHelper->GetGnbPhy (enbNetDev.Get (X), Y)
#define GET_ENB_MAC(X, Y) nrHelper->GetGnbMac (enbNetDev.Get (X), Y)

#define GET_UE_PHY(X, Y) nrHelper->GetUePhy (ueNetDev.Get (X), Y)
#define GET_UE_MAC(X, Y) nrHelper->GetUeMac (ueNetDev.Get (X), Y)

void
NrTimingsTest::DoRun (void)
{
  ns3::SeedManager::SetRun (5);

  Ptr<Node> ueNode = CreateObject<Node> ();
  Ptr<Node> gNbNode = CreateObject<Node> ();

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (gNbNode);
  mobility.Install (ueNode);
  gNbNode->GetObject<MobilityModel>()->SetPosition (Vector (0.0, 0.0, 10));
  ueNode->GetObject<MobilityModel> ()->SetPosition (Vector (0, 10, 1.5));


  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  // Put the pointers inside nrHelper
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;  // in this example, both bands have a single CC

  CcBwpCreator::SimpleOperationBandConf bandConf1 (28e9, 100e6, numCcPerBand, BandwidthPartInfo::UMi_StreetCanyon);

  OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);


  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds (0)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  nrHelper->InitializeOperationBand (&band1);

  allBwps = CcBwpCreator::GetAllBwps ({band1});
  // Beamforming method
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  // Core latency
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  nrHelper->SetSchedulerAttribute ("StartingMcsDl", UintegerValue (28));
  nrHelper->SetSchedulerAttribute ("StartingMcsUl", UintegerValue (28));

  nrHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (m_numerology));
  nrHelper->SetGnbPhyAttribute ("TxPower", DoubleValue (50.0));

  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (50.0));

  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gNbNode, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNode, allBwps);

  GET_ENB_PHY (0,0)->TraceConnectWithoutContext ("GnbPhyTxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::GnbPhyTx, this));
  GET_ENB_PHY (0,0)->TraceConnectWithoutContext ("GnbPhyRxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::GnbPhyRx, this));

  GET_ENB_MAC (0,0)->TraceConnectWithoutContext ("GnbMacTxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::GnbMacTx, this));
  GET_ENB_MAC (0,0)->TraceConnectWithoutContext ("GnbMacRxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::GnbMacRx, this));

  GET_UE_PHY (0,0)->TraceConnectWithoutContext ("UePhyTxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::UePhyTx, this));
  GET_UE_PHY (0,0)->TraceConnectWithoutContext ("UePhyRxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::UePhyRx, this));

  GET_UE_MAC (0,0)->TraceConnectWithoutContext ("UeMacTxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::UeMacTx, this));
  GET_UE_MAC (0,0)->TraceConnectWithoutContext ("UeMacRxedCtrlMsgsTrace", MakeCallback (&NrTimingsTest::UeMacRx, this));

  // When all the configuration is done, explicitly call UpdateConfig ()

  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  InternetStackHelper internet;
  internet.Install (ueNode);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  // DL at 0.4
  Simulator::Schedule (MilliSeconds (400), &SendPacket, enbNetDev.Get (0), ueNetDev.Get (0)->GetAddress ());

  // UL at 0.8
  //Simulator::Schedule (MilliSeconds (800), &SendPacket, ueNetDev.Get(0), enbNetDev.Get(0)->GetAddress ());

  Simulator::Stop (MilliSeconds (1200));

  if (verbose)
    {
      std::cerr << "Executing test for numerology " << m_numerology << std::endl;
    }
  Simulator::Run ();
  Simulator::Destroy ();
}


class NrTimingsTestSuite : public TestSuite
{
public:
  NrTimingsTestSuite ();
};

NrTimingsTestSuite::NrTimingsTestSuite ()
  : TestSuite ("nr-test-timings", SYSTEM)
{
  AddTestCase (new NrTimingsTest ("num=4", 4, false), TestCase::QUICK);
  AddTestCase (new NrTimingsTest ("num=3", 3, false), TestCase::QUICK);
  AddTestCase (new NrTimingsTest ("num=2", 2, false), TestCase::QUICK);
  AddTestCase (new NrTimingsTest ("num=1", 1, false), TestCase::QUICK);
  AddTestCase (new NrTimingsTest ("num=0", 0, false), TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NrTimingsTestSuite nrTimingsTestSuite;


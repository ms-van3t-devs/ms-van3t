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

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
using namespace ns3;


/**
  * \file nr-test-numerology-delay.cc
  * \ingroup test
  *
  * \brief In this test case we want to observe delays of a single UDP packet, and to track its
  * eNB processing time, air time, UE time depending on the numerology.
  */

static uint32_t packetSize = 1000;

class NrTestNumerologyDelayCase1 : public TestCase
{
public:
  NrTestNumerologyDelayCase1 (std::string name, uint32_t numerology);
  virtual ~NrTestNumerologyDelayCase1 ();
  void DlScheduling (uint32_t frameNo, uint32_t subframeNo, uint32_t slotNum,
                     uint8_t streamId, uint32_t tbSize, uint32_t mcs,
                     uint32_t rnti, uint8_t componentCarrierId);
  void DlSpectrumUeEndRx (RxPacketTraceParams params);
  void DlSpectrumEnbStartTx (GnbPhyPacketCountParameter params);
  void TxRlcPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes);
  void TxPdcpPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes);
  void RxRlcPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay);
  void RxPdcpPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay);

private:
  virtual void DoRun (void);

  static Time GetSlotTime (uint32_t numerology);
  static Time GetSymbolPeriod (uint32_t numerology);

  uint32_t m_numerology {0};
  Time m_sendPacketTime {Seconds (0)};
  uint32_t m_numSym {0};
  bool m_firstMacPdu {true};
  bool m_firstRlcPdu {true};
  bool m_firstDlTransmission {true};
  bool m_firstDlReception {true};
  bool m_firstRxPlcPDU {true};
  Time m_lastDlReceptionFinished {Seconds (0)};
  uint32_t m_slotsCounter {0};
  uint32_t m_totalNumberOfSymbols {0};
  uint32_t m_firstMacPduMcs {0};
  uint32_t m_l1l2 {0};
  Time m_tbDecodeLatency {Seconds (0)};
};

// Add some help text to this case to describe what it is intended to test
NrTestNumerologyDelayCase1::NrTestNumerologyDelayCase1 (std::string name, uint32_t numerology)
  : TestCase (name),
    m_numerology (numerology)
{}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
NrTestNumerologyDelayCase1::~NrTestNumerologyDelayCase1 ()
{}


void
LteTestDlSchedCallback (NrTestNumerologyDelayCase1 *testcase, std::string path,
                        NrSchedulingCallbackInfo info)
{
  testcase->DlScheduling (info.m_frameNum, info.m_subframeNum, info.m_slotNum,
                          info.m_streamId, info.m_tbSize, info.m_mcs,
                          info.m_rnti, info.m_bwpId);
}

void
LteTestRxPacketUeCallback (NrTestNumerologyDelayCase1 *testcase, std::string path,
                           RxPacketTraceParams rxParams)
{

  testcase->DlSpectrumUeEndRx (rxParams);
}

void
LteTestTxPacketEnbCallback (NrTestNumerologyDelayCase1 *testcase, std::string path,
                            GnbPhyPacketCountParameter params)
{

  testcase->DlSpectrumEnbStartTx (params);
}

void
LteTestTxRlcPDUCallback (NrTestNumerologyDelayCase1 *testcase, std::string path,
                         uint16_t rnti, uint8_t lcid, uint32_t bytes)
{

  testcase->TxRlcPDU ( rnti, lcid, bytes);
}

void
LteTestTxPdcpPDUCallback (NrTestNumerologyDelayCase1 *testcase, std::string path,
                          uint16_t rnti, uint8_t lcid, uint32_t bytes)
{
  testcase->TxPdcpPDU (rnti, lcid, bytes);
}


void
LteTestRxRlcPDUCallback (NrTestNumerologyDelayCase1 *testcase, std::string path,
                         uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay)
{

  testcase->RxRlcPDU ( rnti, lcid, bytes, delay);
}

void
LteTestRxPdcpPDUCallback (NrTestNumerologyDelayCase1 *testcase, std::string path,
                          uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay)
{
  testcase->RxPdcpPDU (rnti, lcid, bytes, delay);
}


void
ConnectRlcPdcpTraces (NrTestNumerologyDelayCase1 *testcase)
{

  Config::Connect ("/NodeList/1/DeviceList/*/LteEnbRrc/UeMap/1/DataRadioBearerMap/1/LteRlc/TxPDU",
                   MakeBoundCallback (&LteTestTxRlcPDUCallback, testcase));

  Config::Connect ("/NodeList/1/DeviceList/*/LteEnbRrc/UeMap/1/DataRadioBearerMap/1/LtePdcp/TxPDU",
                   MakeBoundCallback (&LteTestTxPdcpPDUCallback, testcase));

  Config::Connect ("/NodeList/0/DeviceList/*/LteUeRrc/DataRadioBearerMap/1/LteRlc/RxPDU",
                   MakeBoundCallback (&LteTestRxRlcPDUCallback, testcase));

  Config::Connect ("/NodeList/0/DeviceList/*/LteUeRrc/DataRadioBearerMap/1/LtePdcp/RxPDU",
                   MakeBoundCallback (&LteTestRxPdcpPDUCallback, testcase));
}

static void SendPacket (Ptr<NetDevice> device, Address& addr)
{
  Ptr<Packet> pkt = Create<Packet> (packetSize);
  //Adding empty IPV4 header after adding the IPV6 support for NR module.
  //NrNetDevice::Receive need to peek the header to know the IP protocol.
  //Since, there are no apps install in this test, this packet will be
  //dropped in Ipv4L3Protocol::Receive method upon not finding the route.
  Ipv4Header ipHeader;
  pkt->AddHeader (ipHeader);
  EpsBearerTag tag (1, 1);
  pkt->AddPacketTag (tag);
  device->Send (pkt, addr, Ipv4L3Protocol::PROT_NUMBER);
}

void
NrTestNumerologyDelayCase1::DoRun (void)
{
  Ptr<Node> ueNode = CreateObject<Node> ();
  Ptr<Node> gNbNode = CreateObject<Node> ();

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (gNbNode);
  mobility.Install (ueNode);
  gNbNode->GetObject<MobilityModel>()->SetPosition (Vector (0.0, 0.0, 10));
  ueNode->GetObject<MobilityModel> ()->SetPosition (Vector (0, 10, 1.5));

  m_sendPacketTime = MilliSeconds (400);

  ns3::SeedManager::SetRun (5);

  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();

  // Beamforming method
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;

  CcBwpCreator::SimpleOperationBandConf bandConf1 (28e9, 400e6, numCcPerBand, BandwidthPartInfo::UMi_StreetCanyon);
  OperationBandInfo band1 = ccBwpCreator.CreateOperationBandContiguousCc (bandConf1);

  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds (0)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  nrHelper->SetSchedulerAttribute ("FixedMcsDl", BooleanValue (true));
  nrHelper->SetSchedulerAttribute ("StartingMcsDl", UintegerValue (1));

  nrHelper->SetGnbPhyAttribute ("SymbolsPerSlot", UintegerValue (14));
  nrHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (m_numerology));
  nrHelper->SetGnbPhyAttribute ("TxPower", DoubleValue (10));

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Error Model: UE and GNB with same spectrum error model.
  nrHelper->SetUlErrorModel ("ns3::NrEesmIrT1");
  nrHelper->SetDlErrorModel ("ns3::NrEesmIrT1");

  // Both DL and UL AMC will have the same model behind.
  nrHelper->SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel
  nrHelper->SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel

  nrHelper->InitializeOperationBand (&band1);
  allBwps = CcBwpCreator::GetAllBwps ({band1});

  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gNbNode, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNode, allBwps);

  m_l1l2 = nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->GetL1L2CtrlLatency ();
  m_tbDecodeLatency = nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->GetTbDecodeLatency ();


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

  Simulator::Schedule (m_sendPacketTime, &SendPacket, enbNetDev.Get (0), ueNetDev.Get (0)->GetAddress ());

  // attach UEs to the closest eNB
  nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);

  Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbMac/DlScheduling",
                   MakeBoundCallback (&LteTestDlSchedCallback, this));

  Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/NrSpectrumPhyList/*/RxPacketTraceUe",
                   MakeBoundCallback (&LteTestRxPacketUeCallback, this));

  Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbPhy/NrSpectrumPhyList/*/TxPacketTraceEnb",
                   MakeBoundCallback (&LteTestTxPacketEnbCallback, this));

  Simulator::Schedule (MilliSeconds (200), &ConnectRlcPdcpTraces, this);

  nrHelper->EnableTraces ();

  Simulator::Stop (MilliSeconds (1000));
  Simulator::Run ();
  Simulator::Destroy ();
}

Time
NrTestNumerologyDelayCase1::GetSlotTime (uint32_t numerology)
{
  uint16_t slotsPerSubframe  = static_cast<uint16_t> (std::pow (2, numerology));
  return Seconds (0.001 / slotsPerSubframe);
}

Time
NrTestNumerologyDelayCase1::GetSymbolPeriod (uint32_t numerology)
{
  return GetSlotTime (numerology) / 14; // Fix number of symbols to 14 in this test
}

void
NrTestNumerologyDelayCase1::TxPdcpPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes)
{
/*  std::cout<<"\n\n Packet transmitted by gNB PDCP at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n rnti :"<<rnti<<std::endl;
  std::cout<<"\n lcid :"<<(unsigned) lcid<<std::endl;
  std::cout<<"\n bytes :"<<bytes<<std::endl;*/

  NS_TEST_ASSERT_MSG_EQ (Simulator::Now (), m_sendPacketTime, "There should not be delay between packet being sent and being scheduled by the gNb PDCP.");
}

void
NrTestNumerologyDelayCase1::TxRlcPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes)
{
/*  std::cout<<"\n\n Packet transmitted by gNB RLC at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n rnti:"<<rnti<<std::endl;
  std::cout<<"\n lcid:"<<rnti<<(unsigned)lcid;
  std::cout<<"\n no of bytes :"<<bytes<<std::endl;*/

  if (m_firstRlcPdu)
    {
      NS_TEST_ASSERT_MSG_EQ (Simulator::Now (), m_sendPacketTime, "There should not be delay between packet being sent and being transmitted by the gNb RLC.");
      m_firstRlcPdu = false;
    }
}

void
NrTestNumerologyDelayCase1::DlScheduling (uint32_t frameNo, uint32_t subframeNo,
                                          uint32_t slotNum, uint8_t streamId, uint32_t tbSize,
                                          uint32_t mcs, uint32_t rnti, uint8_t componentCarrierId)
{
/*  std::cout<<"\n\n\n MAC sends PDU to PHY at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n MCS :"<<mcs<<std::endl;
  std::cout<<"\n rnti :"<<rnti<<std::endl;
  std::cout<<"\n frameNo :"<<slotNum<<std::endl;
  std::cout<<"\n subframeNo :"<<slotNum<<std::endl;
  std::cout<<"\n slotNo :"<<slotNum<<std::endl;*/

  if (m_firstMacPdu)
    {
      NS_TEST_ASSERT_MSG_EQ (Simulator::Now (), m_sendPacketTime, "There should not be delay between packet being sent and being scheduled by the MAC.");
      m_firstMacPdu = false;
      m_firstMacPduMcs = mcs;
    }
  m_slotsCounter++;
}

void
NrTestNumerologyDelayCase1::DlSpectrumEnbStartTx (GnbPhyPacketCountParameter params)
{
/*  std::cout<<"\n\n Started transmission at eNb PHY at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n cell id :"<<params.m_cellId<<std::endl;
  std::cout<<"\n no of bytes :"<<(unsigned)params.m_noBytes<<std::endl;
  std::cout<<"\n subframe no:"<<params.m_subframeno<<std::endl;*/
  Time delay = m_l1l2 * GetSlotTime (m_numerology);
  Time ctrlDuration = GetSymbolPeriod (m_numerology);
  // first there is L1L2 processing delay
  // the, before it start the transmission of the DATA symbol, there is 1 DL CTRL symbol
  // and then we are here already in the following nano second

  if (m_firstDlTransmission)
    {
      NS_TEST_ASSERT_MSG_EQ (Simulator::Now (), m_sendPacketTime + delay + ctrlDuration + NanoSeconds (1),
                             "The delay between packet scheduled by the MAC and being transmitted should be L1L2 delay, plus the duration of the control.");
      m_firstDlTransmission = false;
    }
}

void
NrTestNumerologyDelayCase1::DlSpectrumUeEndRx (RxPacketTraceParams params)
{
/*  std::cout<<"\n\n Finished reception at UE PHY at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n MCS :"<<params.m_mcs<<std::endl;
  std::cout<<"\n slot :"<<(unsigned int)params.m_slotNum<<std::endl;
  std::cout<<"\n rnti:"<<params.m_rnti<<std::endl;*/

  Time delay = m_l1l2 * GetSlotTime (m_numerology);
  Time ctrlDuration = GetSymbolPeriod (m_numerology);
  Time dataDuration = (GetSymbolPeriod (m_numerology) * params.m_numSym) - NanoSeconds (1);

/*  std::cout<<"\n symbol duration:"<<  Seconds (m_nrPhyMacCommon->GetSymbolPeriod());
  std::cout<<"\n symbols:" << (unsigned) params.m_numSym;
  std::cout<<"\n my calculation:"<<m_sendPacketTime + delay + ctrlDuration + dataDuration;*/

  if (m_firstDlReception)
    {
      NS_TEST_ASSERT_MSG_EQ (Simulator::Now (), m_sendPacketTime + delay + ctrlDuration + dataDuration,
                             "The duration of the transmission of the packet is not correct");
      m_firstDlReception = false;
      m_numSym = params.m_numSym;
    }

  m_lastDlReceptionFinished = Simulator::Now ();
  m_totalNumberOfSymbols += params.m_numSym;
}

void
NrTestNumerologyDelayCase1::RxRlcPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t rlcDelay)
{
/*  std::cout<<"\n\n Packet received by UE RLC at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n rnti:"<<rnti<<std::endl;
  std::cout<<"\n lcid:"<<(unsigned)lcid<<std::endl;
  std::cout<<"\n bytes :"<< bytes<<std::endl;
  std::cout<<"\n delay :"<< rlcDelay<<std::endl;*/

  Time delay = m_l1l2 * GetSlotTime (m_numerology);
  Time ctrlDuration = GetSymbolPeriod (m_numerology);
  Time dataDuration = (GetSymbolPeriod (m_numerology) * m_numSym) - NanoSeconds (1);

  if (m_firstRxPlcPDU)
    {
      NS_TEST_ASSERT_MSG_EQ (Simulator::Now (), m_sendPacketTime + delay + ctrlDuration + dataDuration + m_tbDecodeLatency,
                             "The duration of the reception by RLC is not correct.");
      m_firstRxPlcPDU = false;
    }
}

void
NrTestNumerologyDelayCase1::RxPdcpPDU (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t pdcpDelay)
{
/*  std::cout<<"\n\n Packet received by UE PDCP at:"<<Simulator::Now()<<std::endl;
  std::cout<<"\n rnti :"<<rnti<<std::endl;
  std::cout<<"\n lcid:"<<(unsigned)lcid<<std::endl;
  std::cout<<"\n bytes :"<< bytes<<std::endl;
  std::cout<<"\n delay :"<<pdcpDelay<<std::endl;*/

  Time delay = m_l1l2 * GetSlotTime (m_numerology);
  Time ctrlDuration = GetSymbolPeriod (m_numerology);
  Time dataDuration = (GetSymbolPeriod (m_numerology) * m_numSym) - NanoSeconds (1);

  NS_TEST_ASSERT_MSG_EQ (Simulator::Now (), m_lastDlReceptionFinished + m_tbDecodeLatency,
                         "The duration of the reception by PDCP is not correct.");

  std::cout << "\n Numerology:" << m_numerology << "\t Packet of :" << packetSize << " bytes\t#Slots:"
            << m_slotsCounter << "\t#Symbols:" << m_totalNumberOfSymbols << "\tPacket PDCP delay:" << pdcpDelay
            << "\tRLC delay of first PDU:" << delay + ctrlDuration + dataDuration + m_tbDecodeLatency
            << "\tMCS of the first PDU:" << m_firstMacPduMcs;
}


// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined
//
class NrTestNumerologyDelayTestSuite : public TestSuite
{
public:
  NrTestNumerologyDelayTestSuite ();
};

NrTestNumerologyDelayTestSuite::NrTestNumerologyDelayTestSuite ()
  : TestSuite ("nr-test-numerology-delay", SYSTEM)
{
  AddTestCase (new NrTestNumerologyDelayCase1 ("num=0", 0), TestCase::QUICK);
  AddTestCase (new NrTestNumerologyDelayCase1 ("num=1", 1), TestCase::QUICK);
  AddTestCase (new NrTestNumerologyDelayCase1 ("num=2", 2), TestCase::QUICK);
  AddTestCase (new NrTestNumerologyDelayCase1 ("num=3", 3), TestCase::QUICK);
  AddTestCase (new NrTestNumerologyDelayCase1 ("num=4", 4), TestCase::QUICK);
  AddTestCase (new NrTestNumerologyDelayCase1 ("num=5", 5), TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NrTestNumerologyDelayTestSuite nrTestSuite;


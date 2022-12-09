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
#include <ns3/object-factory.h>
#include <ns3/nr-gnb-phy.h>
#include <ns3/node.h>
#include <ns3/nr-ch-access-manager.h>
#include <ns3/nr-mac-scheduler-tdma-rr.h>
#include <ns3/multi-model-spectrum-channel.h>
#include <ns3/beam-manager.h>
#include <ns3/uniform-planar-array.h>

/**
 * \file nr-phy-patterns.cc
 * \ingroup test
 *
 * \brief The test creates a fake MAC that checks if, when PHY calls the DL/UL slot
 * allocations, it does it for the right slot in pattern. In other words, if the
 * PHY calls the UL slot allocation for a slot that should be DL, the test will fail.
 */
namespace ns3 {

class TestGnbMac : public NrGnbMac
{
public:
  static TypeId GetTypeId (void);
  TestGnbMac (const std::string &pattern);
  virtual ~TestGnbMac (void) override;
  virtual void DoSlotDlIndication (const SfnSf &sfnSf, LteNrTddSlotType type) override;
  virtual void DoSlotUlIndication (const SfnSf &sfnSf, LteNrTddSlotType type) override;
  virtual void SetCurrentSfn (const SfnSf &sfn) override;

private:
  std::vector<LteNrTddSlotType> m_pattern;
  std::set <uint32_t> m_slotCreated;
  uint32_t m_totalSlotToCreate {0};
};

NS_OBJECT_ENSURE_REGISTERED (TestGnbMac);

TypeId
TestGnbMac::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TestGnbMac")
    .SetParent<NrGnbMac> ()
    //.AddConstructor<TestGnbMac> ()
  ;
  return tid;
}

TestGnbMac::TestGnbMac (const std::string &pattern)
{
  static std::unordered_map<std::string, LteNrTddSlotType> lookupTable =
  {
    { "DL", LteNrTddSlotType::DL },
    { "UL", LteNrTddSlotType::UL },
    { "S",  LteNrTddSlotType::S },
    { "F",  LteNrTddSlotType::F },
  };

  std::stringstream ss (pattern);
  std::string token;
  std::vector<std::string> extracted;

  while (std::getline (ss, token, '|'))
    {
      extracted.push_back (token);
    }

  for (const auto & v : extracted)
    {
      if (lookupTable.find (v) == lookupTable.end ())
        {
          NS_FATAL_ERROR ("Pattern type " << v << " not valid. Valid values are: DL UL F S");
        }
      m_pattern.push_back (lookupTable[v]);
    }

  for (const auto & v : m_pattern)
    {
      switch (v)
        {
          case LteNrTddSlotType::F:
            m_totalSlotToCreate += 2; // But since we are using std::set,
                                      // duplicated slots will not be counted
            break;
          case LteNrTddSlotType::DL:
            m_totalSlotToCreate += 1;
            break;
          case LteNrTddSlotType::UL:
            m_totalSlotToCreate += 1;
            break;
          case LteNrTddSlotType::S:
            m_totalSlotToCreate += 1;
            break;
        }
    }
}

TestGnbMac::~TestGnbMac ()
{
  NS_ASSERT_MSG (m_slotCreated.size () == m_pattern.size (),
                 "The number of created slot (" << m_slotCreated.size () <<
                 ") is not equal to the pattern size " << m_pattern.size () <<
                 ", we have to create " << m_totalSlotToCreate << " slots");
}

void
TestGnbMac::DoSlotDlIndication (const SfnSf &sfnSf, LteNrTddSlotType type)
{
  uint32_t pos = sfnSf.Normalize ();
  pos = pos % m_pattern.size ();

  NS_ASSERT (type == LteNrTddSlotType::DL || type == LteNrTddSlotType::S || type == LteNrTddSlotType::F);
  NS_ASSERT_MSG (m_pattern[pos] == LteNrTddSlotType::DL
                 || m_pattern[pos] == LteNrTddSlotType::S
                 || m_pattern[pos] == LteNrTddSlotType::F,
                 "MAC called to generate a DL slot, but in the pattern there is " << m_pattern[pos]);

  m_slotCreated.insert (pos);

  NrGnbMac::DoSlotDlIndication (sfnSf, type);
}

void
TestGnbMac::DoSlotUlIndication (const SfnSf &sfnSf, LteNrTddSlotType type)
{
  uint32_t pos = sfnSf.Normalize ();
  pos = pos % m_pattern.size ();

  NS_ASSERT (type == LteNrTddSlotType::UL || type == LteNrTddSlotType::S || type == LteNrTddSlotType::F);
  NS_ASSERT_MSG (m_pattern[pos] == LteNrTddSlotType::UL
                 || m_pattern[pos] == LteNrTddSlotType::F,
                 "MAC called to generate a UL slot, but in the pattern there is " << m_pattern[pos]);

  m_slotCreated.insert (pos);

  NrGnbMac::DoSlotUlIndication (sfnSf, type);
}

void
TestGnbMac::SetCurrentSfn (const SfnSf &sfnSf)
{
  NrGnbMac::SetCurrentSfn (sfnSf);
}

/**
 * \brief TestCase for the PHY TDD Patterns
 */
class LtePhyPatternTestCase : public TestCase
{
public:
  /**
   * \brief Create LtePatternTestCase
   * \param name Name of the test
   */
  LtePhyPatternTestCase (const std::string &pattern, const std::string &name)
    : TestCase (name), m_pattern (pattern)
  {}
  ~LtePhyPatternTestCase ();

private:
  virtual void DoRun (void) override;
  void Print (const std::string &msg1, const std::string& msg2,
              const std::map<uint32_t, std::vector<uint32_t> > &str);
  void StartSimu ();
  Ptr<NrGnbMac> CreateMac (const Ptr<NrMacScheduler> &sched) const;
  Ptr<NrGnbPhy> CreatePhy (const Ptr<NrGnbMac> &mac) const;

  Ptr<NrGnbPhy> m_phy;
  std::string m_pattern;
};

LtePhyPatternTestCase::~LtePhyPatternTestCase ()
{
  if (m_phy)
    {
      m_phy->Dispose ();
      m_phy = nullptr;
    }
}

void
LtePhyPatternTestCase::DoRun ()
{
  ObjectFactory schedFactory;
  schedFactory.SetTypeId (NrMacSchedulerTdmaRR::GetTypeId ());
  Ptr<NrMacScheduler> sched = DynamicCast<NrMacScheduler> (schedFactory.Create ());

  auto mac = CreateMac (sched);
  m_phy = CreatePhy (mac);

  m_phy->SetPattern (m_pattern);

  // Finishing initialization
  m_phy->SetPhySapUser (mac->GetPhySapUser ());
  m_phy->Initialize ();
  mac->SetPhySapProvider (m_phy->GetPhySapProvider ());
  mac->Initialize ();

  StartSimu ();
}

void
LtePhyPatternTestCase::StartSimu ()
{
  Simulator::Stop (MilliSeconds (200));
  Simulator::Run ();
  Simulator::Destroy ();
}

Ptr<NrGnbPhy>
LtePhyPatternTestCase::CreatePhy (const Ptr<NrGnbMac> &mac) const
{
  Ptr<NrSpectrumPhy> channelPhy = CreateObject<NrSpectrumPhy> ();
  Ptr<NrGnbPhy> phy = CreateObject <NrGnbPhy> ();
  Ptr<UniformPlanarArray> antenna = CreateObject <UniformPlanarArray> ();

  phy->InstallCentralFrequency (28e9);

  phy->ScheduleStartEventLoop (0, 0, 0, 0);

  // PHY <--> CAM
  Ptr<NrChAccessManager> cam = DynamicCast<NrChAccessManager> (CreateObject<NrAlwaysOnAccessManager> ());
  cam->SetNrSpectrumPhy (channelPhy);
  cam->SetNrGnbMac (mac);
  phy->SetCam (cam);

  Ptr<NrHarqPhy> harq = Create<NrHarqPhy> ();
  channelPhy->InstallHarqPhyModule (harq);

  Ptr<LteChunkProcessor> pData = Create<LteChunkProcessor> ();
  channelPhy->AddDataSinrChunkProcessor (pData);

  channelPhy->InstallPhy (phy);

  phy->InstallSpectrumPhy (channelPhy);
  Ptr<BeamManager> beamManager = CreateObject<BeamManager>();
  beamManager->Configure (antenna);
  channelPhy->SetBeamManager(beamManager);
  return phy;
}

Ptr<NrGnbMac>
LtePhyPatternTestCase::CreateMac (const Ptr<NrMacScheduler> &sched) const
{
  Ptr<NrGnbMac> mac = CreateObject<TestGnbMac> (m_pattern);

  sched->SetMacSchedSapUser (mac->GetNrMacSchedSapUser ());
  sched->SetMacCschedSapUser (mac->GetNrMacCschedSapUser ());

  mac->SetNrMacSchedSapProvider (sched->GetMacSchedSapProvider ());
  mac->SetNrMacCschedSapProvider (sched->GetMacCschedSapProvider ());

  return mac;
}

void
LtePhyPatternTestCase::Print (const std::string &msg1, const std::string& msg2,
                              const std::map<uint32_t, std::vector<uint32_t> > &str)
{
  for (const auto & v : str)
    {
      for (const auto & i : v.second)
        {
          std::cout << msg1 << i << msg2 << v.first << std::endl;
        }
    }
}



class NrLtePatternsTestSuite : public TestSuite
{
public:
  NrLtePatternsTestSuite () : TestSuite ("nr-phy-patterns", UNIT)
  {

    AddTestCase (new LtePhyPatternTestCase ("DL|S|UL|UL|DL|DL|S|UL|UL|DL|",
                                            "LTE TDD Pattern 1 test"), QUICK);

    AddTestCase (new LtePhyPatternTestCase ("DL|S|UL|DL|DL|DL|S|UL|DL|DL|",
                                            "LTE TDD Pattern 2 test"), QUICK);

    AddTestCase (new LtePhyPatternTestCase ("DL|S|UL|UL|UL|DL|DL|DL|DL|DL|",
                                            "LTE TDD Pattern 3 test"), QUICK);

    AddTestCase (new LtePhyPatternTestCase ("DL|S|UL|UL|DL|DL|DL|DL|DL|DL|",
                                            "LTE TDD Pattern 4 test"), QUICK);

    AddTestCase (new LtePhyPatternTestCase ("DL|S|UL|DL|DL|DL|DL|DL|DL|DL|",
                                            "LTE TDD Pattern 5 test"), QUICK);

    AddTestCase (new LtePhyPatternTestCase ("DL|S|UL|UL|UL|DL|S|UL|UL|DL|",
                                            "LTE TDD Pattern 6 test"), QUICK);

    AddTestCase (new LtePhyPatternTestCase ("DL|S|UL|UL|UL|DL|S|UL|UL|UL|",
                                            "LTE TDD Pattern 0 test"), QUICK);

    AddTestCase (new LtePhyPatternTestCase ("F|F|F|F|F|F|F|F|F|F|",
                                            "LTE TDD Pattern NR test"), QUICK);

  }
};

static NrLtePatternsTestSuite nrLtePatternsTestSuite;

//!< Pattern test suite

}  // namespace ns3

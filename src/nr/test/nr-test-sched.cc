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
#include <ns3/nr-mac-scheduler-ns3.h>
#include <ns3/nr-mac-sched-sap.h>

/**
 * \file nr-test-sched.cc
 * \ingroup test
 *
 * \brief The class is a stub for a future, unit-testing component for the various
 * kind of schedulers. The idea is to check what is happening to the scheduling
 * part following a black-box approach: passing inputs, and then see what
 * is the output, and if it is like we would expect. The reference API is the
 * FF API, and we should check what happens, for example, when adding or
 * removing users, when a CQI is passed, etc.
 */
namespace ns3 {

/**
 * \ingroup test
 * \brief The TestCschedSapUser class
 *
 * This class doesn't do absolutely nothing. Thank you for the attention.
 */
class TestCschedSapUser : public NrMacCschedSapUser
{
public:
  TestCschedSapUser () : NrMacCschedSapUser () { }
  virtual void CschedCellConfigCnf ([[maybe_unused]] const struct CschedCellConfigCnfParameters& params) override
  {
  }

  virtual void CschedUeConfigCnf ([[maybe_unused]] const struct CschedUeConfigCnfParameters& params) override
  {
  }

  virtual void CschedLcConfigCnf ([[maybe_unused]] const struct CschedLcConfigCnfParameters& params) override
  {
  }

  virtual void CschedLcReleaseCnf ([[maybe_unused]] const struct CschedLcReleaseCnfParameters& params) override
  {
  }

  virtual void CschedUeReleaseCnf ([[maybe_unused]] const struct CschedUeReleaseCnfParameters& params) override
  {
  }

  virtual void CschedUeConfigUpdateInd ([[maybe_unused]] const struct CschedUeConfigUpdateIndParameters& params) override
  {
  }

  virtual void CschedCellConfigUpdateInd ([[maybe_unused]] const struct CschedCellConfigUpdateIndParameters& params) override
  {
  }
};

class TestSchedSapUser;
/**
 * \brief TestSched testcase
 */
class NrSchedGeneralTestCase : public TestCase
{
public:
  /**
   * \brief Create NrSchedGeneralTestCase
   * \param scheduler Scheduler to test
   * \param name Name of the test
   */
  NrSchedGeneralTestCase (const std::string &scheduler, const std::string &name)
    : TestCase (name),
      m_scheduler (scheduler) {}

  /**
   * \brief Destroy the object instance
   */
  virtual ~NrSchedGeneralTestCase () override {}

  void SchedConfigInd (const struct NrMacSchedSapUser::SchedConfigIndParameters& params);

protected:
  void TestSAPInterface (const Ptr<NrMacScheduler> &sched);
  void TestAddingRemovingUsersNoData (const Ptr<NrMacSchedulerNs3> &sched);
  void TestSchedNewData (const Ptr<NrMacSchedulerNs3> &sched);
  void TestSchedNewDlData (const Ptr<NrMacSchedulerNs3> &sched);
  void TestSchedNewUlData (const Ptr<NrMacSchedulerNs3> &sched);
  void TestSchedNewDlUlData (const Ptr<NrMacSchedulerNs3> &sched);

protected:
  void AddOneUser (uint16_t rnti, const Ptr<NrMacSchedulerNs3> &sched);
  void TestingRemovingUsers (const Ptr<NrMacSchedulerNs3> &sched);
  void TestingAddingUsers (const Ptr<NrMacSchedulerNs3> &sched);
  void LcConfigFor (uint16_t rnti, uint32_t bytes, const Ptr<NrMacSchedulerNs3> &sched);

private:
  virtual void DoRun (void) override;

  std::string m_scheduler            {}; //!< Type of the scheduler
  TestCschedSapUser *m_cSchedSapUser {nullptr};
  TestSchedSapUser *m_schedSapUser   {nullptr};
};

class TestSchedSapUser : public NrMacSchedSapUser
{
public:
  TestSchedSapUser (NrSchedGeneralTestCase *testCase)
    : NrMacSchedSapUser (),
      m_testCase (testCase)
  { }

  virtual void SchedConfigInd (const struct SchedConfigIndParameters& params) override
  {
    m_testCase->SchedConfigInd (params);
  }

  // For the rest, setup some hard-coded values; for the moment, there is
  // no need to have real values here.
  virtual Ptr<const SpectrumModel> GetSpectrumModel () const override
  {
    return nullptr;
  }

  virtual uint32_t GetNumRbPerRbg () const override
  {
    return 1;
  }

  virtual uint8_t GetNumHarqProcess () const override
  {
    return 20;
  }

  virtual uint16_t GetBwpId () const override
  {
    return 0;
  }

  virtual uint16_t GetCellId () const override
  {
    return 0;
  }

  virtual uint32_t GetSymbolsPerSlot () const override
  {
    return 14;
  }

  virtual Time GetSlotPeriod () const override
  {
    return MilliSeconds (1);
  }

private:
  NrSchedGeneralTestCase *m_testCase;
};

void
NrSchedGeneralTestCase::TestSAPInterface (const Ptr<NrMacScheduler> &sched)
{
  NS_ABORT_IF (sched->GetMacSchedSapProvider () == nullptr);
  NS_ABORT_IF (sched->GetMacCschedSapProvider () == nullptr);
  sched->SetMacCschedSapUser (m_cSchedSapUser);
  sched->SetMacSchedSapUser (m_schedSapUser);
}

void
NrSchedGeneralTestCase::SchedConfigInd ([[maybe_unused]] const struct NrMacSchedSapUser::SchedConfigIndParameters& params)
{
}

void
NrSchedGeneralTestCase::AddOneUser (uint16_t rnti, const Ptr<NrMacSchedulerNs3> &sched)
{
  NrMacCschedSapProvider::CschedUeConfigReqParameters params;
  params.m_rnti = rnti;
  params.m_beamConfId = BeamConfId (BeamId(8, 120.0), BeamId::GetEmptyBeamId());
  sched->DoCschedUeConfigReq (params);
}

void
NrSchedGeneralTestCase::TestingAddingUsers (const Ptr<NrMacSchedulerNs3> &sched)
{
  for (uint16_t i = 0; i < 80; ++i)
    {
      AddOneUser (i, sched);
      NS_TEST_ASSERT_MSG_EQ (sched->m_ueMap.size (),
                             static_cast<uint32_t> (i + 1), "UE not saved in the map");
    }
}

void
NrSchedGeneralTestCase::TestingRemovingUsers (const Ptr<NrMacSchedulerNs3> &sched)
{
  for (uint16_t i = 80; i > 0; --i)
    {
      NrMacCschedSapProvider::CschedUeReleaseReqParameters params;
      params.m_rnti = i - 1;
      sched->DoCschedUeReleaseReq (params);
      NS_TEST_ASSERT_MSG_EQ (sched->m_ueMap.size (),
                             static_cast<uint32_t> (i - 1), "UE released from the map. Map size " <<
                             sched->m_ueMap.size () << " counter " << i);
    }
}

void
NrSchedGeneralTestCase::TestAddingRemovingUsersNoData (const Ptr<NrMacSchedulerNs3> &sched)
{
  NS_TEST_ASSERT_MSG_EQ (sched->m_ueMap.size (), 0, "some UE are in the map");
  TestingAddingUsers (sched);
  TestingRemovingUsers (sched);
  NS_TEST_ASSERT_MSG_EQ (sched->m_ueMap.size (), 0, sched->m_ueMap.size () << " UEs are still in the map");
}

void
NrSchedGeneralTestCase::TestSchedNewData (const Ptr<NrMacSchedulerNs3> &sched)
{
  TestSchedNewDlData (sched);
  TestSchedNewUlData (sched);
  TestSchedNewDlUlData (sched);
}

void
NrSchedGeneralTestCase::LcConfigFor (uint16_t rnti, uint32_t bytes,
                                     const Ptr<NrMacSchedulerNs3> &sched)
{
  NrMacCschedSapProvider::CschedLcConfigReqParameters params;
  LogicalChannelConfigListElement_s lc;
  params.m_rnti = rnti;
  params.m_reconfigureFlag = false;
  params.m_logicalChannelConfigList.emplace_back (lc);

  sched->DoCschedLcConfigReq (params);
}

void
NrSchedGeneralTestCase::TestSchedNewDlData (const Ptr<NrMacSchedulerNs3> &sched)
{
  // Add 80 users
  TestingAddingUsers (sched);
}

void
NrSchedGeneralTestCase::TestSchedNewUlData (const Ptr<NrMacSchedulerNs3> &sched)
{}

void
NrSchedGeneralTestCase::TestSchedNewDlUlData (const Ptr<NrMacSchedulerNs3> &sched)
{}

void
NrSchedGeneralTestCase::DoRun ()
{
  m_cSchedSapUser = new TestCschedSapUser ();
  m_schedSapUser = new TestSchedSapUser (this);

  ObjectFactory factory;
  factory.SetTypeId (m_scheduler);
  Ptr<NrMacSchedulerNs3> sched = DynamicCast<NrMacSchedulerNs3> (factory.Create ());
  NS_ABORT_MSG_IF (sched == nullptr, "Can't create a NrMacSchedulerNs3 from type " + m_scheduler);

  TestSAPInterface (sched);
  TestAddingRemovingUsersNoData (sched);
  TestSchedNewData (sched);

  delete m_cSchedSapUser;
  delete m_schedSapUser;
}

class NrTestSchedSuite : public TestSuite
{
public:
  NrTestSchedSuite () : TestSuite ("nr-test-sched", SYSTEM)
  {
    AddTestCase (new NrSchedGeneralTestCase ("ns3::NrMacSchedulerTdmaRR", "TdmaRR test"), QUICK);
    AddTestCase (new NrSchedGeneralTestCase ("ns3::NrMacSchedulerTdmaPF", "TdmaPF test"), QUICK);
    AddTestCase (new NrSchedGeneralTestCase ("ns3::NrMacSchedulerOfdmaRR", "OfdmaRR test"), QUICK);
    AddTestCase (new NrSchedGeneralTestCase ("ns3::NrMacSchedulerOfdmaPF", "OfdmaPF test"), QUICK);
  }
};

static NrTestSchedSuite nrSchedTestSuite; //!< Nr scheduler test suite

}  // namespace ns3

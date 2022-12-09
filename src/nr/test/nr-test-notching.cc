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
#include <ns3/test.h>
#include <ns3/object-factory.h>
#include <ns3/node.h>
#include <ns3/nr-gnb-mac.h>
#include <ns3/nr-mac-scheduler-ns3.h>
#include <ns3/nr-mac-sched-sap.h>
#include <ns3/nr-phy-sap.h>
#include <ns3/nr-control-messages.h>
#include <ns3/beam-conf-id.h>
#include <algorithm>

/**
 * \file nr-test-notching.cc
 * \ingroup test
 *
 * \brief This test is used to validate the notching functionality.
 * In order to do so, it creates a fake MAC and checks in the method
 * TestNotchingGnbMac::DoSchedConfigIndication() that RBG mask is
 * in the DCI is constructed in accordance with the (tested)
 * notching mask.
 *
 * \see TestNotchingGnbMac::DoSchedConfigIndication
 */
namespace ns3 {

class TestNotchingPhySapProvider : public NrPhySapProvider
{
public:
  TestNotchingPhySapProvider ();
  virtual ~TestNotchingPhySapProvider (void) override;
  virtual uint32_t GetSymbolsPerSlot () const override;
  virtual Ptr<const SpectrumModel> GetSpectrumModel () override;
  virtual uint16_t GetBwpId () const override;
  virtual uint16_t GetCellId () const override;
  virtual Time GetSlotPeriod () const override;
  virtual void SendMacPdu (const Ptr<Packet> &p, const SfnSf & sfn, uint8_t symStart, uint8_t streamId) override;
  virtual void SendControlMessage (Ptr<NrControlMessage> msg) override;
  virtual void SendRachPreamble (uint8_t PreambleId, uint8_t Rnti) override;
  virtual void SetSlotAllocInfo (const SlotAllocInfo &slotAllocInfo) override;
  virtual void NotifyConnectionSuccessful () override;
  virtual uint32_t GetRbNum () const override;
  virtual BeamConfId GetBeamConfId (uint8_t rnti) const override;
  void SetParams (uint32_t numOfUesPerBeam, uint32_t numOfBeams);

private:
  uint32_t m_sapNumOfUesPerBeam = 0;
  uint32_t m_sapNumOfBeams = 0;
};

TestNotchingPhySapProvider::TestNotchingPhySapProvider ()
{}

TestNotchingPhySapProvider::~TestNotchingPhySapProvider ()
{}

void
TestNotchingPhySapProvider::SetParams (uint32_t numOfUesPerBeam, uint32_t numOfBeams)
{
  m_sapNumOfUesPerBeam = numOfUesPerBeam;
  m_sapNumOfBeams = numOfBeams;
}

uint32_t
TestNotchingPhySapProvider::GetSymbolsPerSlot () const
{
  //Fixed 14 symbols per slot.
  return 14;
}

Ptr<const SpectrumModel>
TestNotchingPhySapProvider::GetSpectrumModel ()
{
  return nullptr;
}

uint16_t
TestNotchingPhySapProvider::GetBwpId () const
{
  return 0;
}

uint16_t
TestNotchingPhySapProvider::GetCellId () const
{
  return 0;
}

Time
TestNotchingPhySapProvider::GetSlotPeriod () const
{
  //If in the future the scheduler calls this method, remove this assert"
  NS_FATAL_ERROR ("GetSlotPeriod should not be called");
  return MilliSeconds (1);
}

void
TestNotchingPhySapProvider::SendMacPdu (const Ptr<Packet> &p, const SfnSf & sfn, uint8_t symStart, uint8_t streamId)
{}

void
TestNotchingPhySapProvider::SendControlMessage (Ptr<NrControlMessage> msg)
{}

void
TestNotchingPhySapProvider::SendRachPreamble (uint8_t PreambleId, uint8_t Rnti)
{}

void
TestNotchingPhySapProvider::SetSlotAllocInfo (const SlotAllocInfo &slotAllocInfo)
{}

void
TestNotchingPhySapProvider::NotifyConnectionSuccessful ()
{}

uint32_t
TestNotchingPhySapProvider::GetRbNum () const
{
  //If in the future the scheduler calls this method, remove this assert"
  NS_FATAL_ERROR ("GetRbNum should not be called");
  return 53;
}

BeamConfId
TestNotchingPhySapProvider::GetBeamConfId (uint8_t rnti) const
{
  BeamId beamId = BeamId (0, 0.0);
  uint8_t rntiCnt = 1;
  for (uint32_t beam = 0; beam < m_sapNumOfUesPerBeam; beam++)
    {
      for (uint32_t u = 0; u < m_sapNumOfBeams; u++)
        {
          if (rntiCnt == rnti && beam == 0)
            {
              beamId = BeamId (0, 0.0);
            }
          else if (rntiCnt == rnti && beam == 1)
            {
              beamId = BeamId (1, 120.0);
            }
          else if (rnti == 0)
            {
              beamId = BeamId (0, 0.0);
            }
          rntiCnt++;
        }
    }
  return BeamConfId (beamId, BeamId::GetEmptyBeamId());
}


class TestNotchingGnbMac : public NrGnbMac
{
public:
  static TypeId GetTypeId (void);
  TestNotchingGnbMac (const std::vector<u_int8_t> &inputMask);
  virtual ~TestNotchingGnbMac (void) override;
  virtual void DoSchedConfigIndication (NrMacSchedSapUser::SchedConfigIndParameters ind) override;
  void SetVerbose (bool verbose);

private:
  std::vector<u_int8_t> m_inputMask;
  bool m_verboseMac = false;
};

NS_OBJECT_ENSURE_REGISTERED (TestNotchingGnbMac);

TypeId
TestNotchingGnbMac::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::TestNotchingGnbMac")
    .SetParent<NrGnbMac> ()
    //.AddConstructor<TestNotchingGnbMac> ()
  ;
  return tid;
}

TestNotchingGnbMac::TestNotchingGnbMac (const std::vector<u_int8_t> &inputMask)
{
  m_inputMask = inputMask;
}

TestNotchingGnbMac::~TestNotchingGnbMac ()
{}

void
TestNotchingGnbMac::SetVerbose (bool verbose)
{
  m_verboseMac = verbose;
}

void
TestNotchingGnbMac::DoSchedConfigIndication (NrMacSchedSapUser::SchedConfigIndParameters ind)
{
  // Check that the allocations in `ind` have the correct RBG mask
  // Will be called after SchedDlTriggerReq is called
  // test that ind.m_slotAllocInfo is ok: the sfnf, and the varAlloc deque

  for (unsigned islot = 0; islot < ind.m_slotAllocInfo.m_varTtiAllocInfo.size (); islot++)
    {
      VarTtiAllocInfo &varTtiAllocInfo = ind.m_slotAllocInfo.m_varTtiAllocInfo[islot];

      if (varTtiAllocInfo.m_dci->m_rnti == 0)
        {
          continue;
        }

      if (m_verboseMac)
        {
          std::ostringstream oss;
          for (auto & x: varTtiAllocInfo.m_dci->m_rbgBitmask)
            {
              oss << std::to_string (x) << " ";
            }

          std::cout << "UE " << varTtiAllocInfo.m_dci->m_rnti << " assigned RBG" <<
            " with mask: " << oss.str () << std::endl;
        }

      NS_ASSERT_MSG (varTtiAllocInfo.m_dci->m_rbgBitmask.size () == m_inputMask.size (),
                     "dci bitmask is not of same size as the mask");

      unsigned zeroes = std::count (varTtiAllocInfo.m_dci->m_rbgBitmask.begin (),
                                    varTtiAllocInfo.m_dci->m_rbgBitmask.end (), 0);

      NS_ASSERT_MSG (zeroes != m_inputMask.size (), "dci rbgBitmask is filled with zeros");

      for (unsigned index = 0; index < varTtiAllocInfo.m_dci->m_rbgBitmask.size (); index++)
        {
          if (m_inputMask[index] == 0)
            {
              NS_ASSERT_MSG (varTtiAllocInfo.m_dci->m_rbgBitmask[index] == 0,
                             "dci is diff from mask");
            }

        }
    }
}

/**
 * \brief TestCase for the notching mask
 */
class NrNotchingTestCase : public TestCase
{
public:
  /**
   * \brief Create NrNotchingTestCase
   * \param name Name of the test
   * \param mask The mask to be tested
   * \param schedulerType The type of the scheduler to be tested
   * \param numOfUesPerBeam The number of UEs per beam to be tested
   * \param beamsNum The number beams to be tested
   */
  NrNotchingTestCase (const std::string &name, const std::vector<uint8_t> &mask,
                      const std::string &schedulerType,
                      uint32_t numOfUesPerBeam, uint32_t beamsNum)
    : TestCase (name), m_mask (mask), m_schedulerType (schedulerType),
      m_numOfUesPerBeam (numOfUesPerBeam), m_beamsNum (beamsNum)
  {}

  ~NrNotchingTestCase ();

private:
  virtual void DoRun (void) override;
  Ptr<NrMacSchedulerNs3> CreateScheduler (const std::string &schedulerType) const;
  Ptr<TestNotchingGnbMac> CreateMac (Ptr<NrMacSchedulerNs3> &scheduler,
                                     NrMacCschedSapProvider::CschedCellConfigReqParameters &params) const;

  bool m_verbose = false;
  const std::vector<uint8_t> m_mask;
  const std::string m_schedulerType;
  uint32_t m_numOfUesPerBeam;
  uint32_t m_beamsNum;
  TestNotchingPhySapProvider * m_phySapProvider;
};

NrNotchingTestCase::~NrNotchingTestCase ()
{}

Ptr<NrMacSchedulerNs3>
NrNotchingTestCase::CreateScheduler (const std::string &schedulerType) const
{
  ObjectFactory schedFactory;
  schedFactory.SetTypeId (schedulerType);
  Ptr<NrMacSchedulerNs3> sched = DynamicCast<NrMacSchedulerNs3> (schedFactory.Create ());
  NS_ABORT_MSG_IF (sched == nullptr, "Can't create a NrMacSchedulerNs3 from type " + schedulerType);

  return sched;
}

Ptr<TestNotchingGnbMac>
NrNotchingTestCase::CreateMac (Ptr<NrMacSchedulerNs3> &scheduler,
                               NrMacCschedSapProvider::CschedCellConfigReqParameters &params) const
{
  Ptr<TestNotchingGnbMac> mac = CreateObject<TestNotchingGnbMac> (m_mask);

  mac->SetNrMacSchedSapProvider (scheduler->GetMacSchedSapProvider ());
  mac->SetNrMacCschedSapProvider (scheduler->GetMacCschedSapProvider ());
  scheduler->SetMacSchedSapUser (mac->GetNrMacSchedSapUser ());
  scheduler->SetMacCschedSapUser (mac->GetNrMacCschedSapUser ());
  scheduler->SetDlNotchedRbgMask (m_mask);
  scheduler->SetUlNotchedRbgMask (m_mask);
  // Config sched
  scheduler->DoCschedCellConfigReq (params);

  return mac;
}

void
NrNotchingTestCase::DoRun ()
{
  NrMacCschedSapProvider::CschedCellConfigReqParameters params;
  params.m_ulBandwidth = m_mask.size ();
  params.m_dlBandwidth = m_mask.size ();

  auto sched = CreateScheduler (m_schedulerType);
  auto mac = CreateMac (sched, params);

  m_phySapProvider = new TestNotchingPhySapProvider ();
  m_phySapProvider->SetParams (m_beamsNum, m_numOfUesPerBeam);

  mac->SetPhySapProvider (m_phySapProvider);
  mac->SetVerbose (m_verbose);

  Ptr<NrAmc> amc = CreateObject<NrAmc> ();
  sched->InstallDlAmc (amc);

  //uint32_t rbPerRbg = mac->GetNumRbPerRbg ();
  //std::cout << "numRbPerRbg: " << rbPerRbg << std::endl;

  uint16_t rntiCnt = 1;
  for (uint32_t beam = 0; beam < m_beamsNum; beam++)
    {
      for (uint32_t u = 0; u < m_numOfUesPerBeam; u++)
        {
          NrMacCschedSapProvider::CschedUeConfigReqParameters paramsUe;
          paramsUe.m_rnti = rntiCnt;
          paramsUe.m_beamConfId = m_phySapProvider->GetBeamConfId (rntiCnt);

          if (m_verbose)
            {
              std::cout << "beam: " << beam << " ue: " << u <<
                " rnti: " << paramsUe.m_rnti <<
                " beam Id: " << paramsUe.m_beamConfId <<
                " scheduler: " << m_schedulerType << std::endl;
              if (beam == (m_beamsNum - 1) && u == (m_numOfUesPerBeam - 1))
                {
                  std::ostringstream ss;
                  for (auto & x: m_mask)
                    {
                      ss << std::to_string (x) << " ";
                    }

                  std::cout << "The defined mask is:         " <<
                    ss.str () << std::endl;
                }
            }

          //Add Users
          sched->DoCschedUeConfigReq (paramsUe); // Repeat for the number of UEs

          // Create LC
          NrMacCschedSapProvider::CschedLcConfigReqParameters paramsLc;
          paramsLc.m_rnti = rntiCnt;
          paramsLc.m_reconfigureFlag = false;

          LogicalChannelConfigListElement_s lc;
          lc.m_logicalChannelIdentity = 1;
          lc.m_logicalChannelGroup = 2;
          lc.m_direction = LogicalChannelConfigListElement_s::DIR_DL;
          lc.m_qosBearerType = LogicalChannelConfigListElement_s::QBT_NON_GBR;
          lc.m_qci = 9;
          paramsLc.m_logicalChannelConfigList.emplace_back (lc);

          sched->DoCschedLcConfigReq (paramsLc);

          // Update queue
          NrMacSchedSapProvider::SchedDlRlcBufferReqParameters paramsDlRlc;
          paramsDlRlc.m_rnti = rntiCnt;
          paramsDlRlc.m_logicalChannelIdentity = 1;
          paramsDlRlc.m_rlcRetransmissionHolDelay = 0;
          paramsDlRlc.m_rlcRetransmissionQueueSize = 0;
          paramsDlRlc.m_rlcStatusPduSize = 0;
          paramsDlRlc.m_rlcTransmissionQueueHolDelay = 0;
          paramsDlRlc.m_rlcTransmissionQueueSize = 1284;

          sched->DoSchedDlRlcBufferReq (paramsDlRlc);

          rntiCnt++;
        }
    }

  // Call scheduling
  NrMacSchedSapProvider::SchedDlTriggerReqParameters paramsDlTrigger;
  paramsDlTrigger.m_snfSf = SfnSf (0, 0, 0, 0);
  paramsDlTrigger.m_slotType = LteNrTddSlotType::DL;
  sched->DoSchedDlTriggerReq (paramsDlTrigger);

  delete m_phySapProvider;
}


class NrNotchingTestSuite : public TestSuite
{
public:
  NrNotchingTestSuite () : TestSuite ("nr-test-notching", UNIT)
  {
    //We simulate BW of 10 MHz so the size of the mask is 53 RBGs
    //considering that 1 RBG contains 1 RB
    std::vector<uint8_t> notchedMask1 {0, 0, 1, 0, 0,
                                       0, 0, 1, 1, 1,
                                       1, 1, 1, 0, 1,
                                       1, 1, 1, 1, 1,
                                       1, 1, 1, 1, 1,
                                       1, 1, 1, 0, 0,
                                       1, 1, 1, 1, 1,
                                       1, 1, 1, 1, 1,
                                       1, 1, 1, 0, 0,
                                       0, 0, 0, 0, 0,
                                       0, 1, 1};

    std::vector<uint8_t> notchedMask2 {0, 0, 0, 0, 0,
                                       0, 0, 1, 1, 1,
                                       1, 1, 1, 0, 1,
                                       1, 1, 1, 1, 1,
                                       1, 0, 0, 1, 1,
                                       1, 1, 1, 0, 0,
                                       0, 0, 0, 0, 0,
                                       1, 1, 1, 1, 1,
                                       1, 1, 0, 0, 0,
                                       0, 0, 0, 0, 0,
                                       0, 1, 0};

    std::list<std::string> subdivision     = {"Tdma", "Ofdma"};
    std::list<std::string> scheds          = {"RR"};
    std::list<uint32_t>    uesPerBeamList  = {1, 2, 4, 6};
    std::list<uint32_t>    beams           = {1, 2};

    for (const auto & subType : subdivision)
      {
        for (const auto & sched : scheds)
          {
            for (const auto & uesPerBeam : uesPerBeamList)
              {
                for (const auto & beam : beams)
                  {
                    std::stringstream ss, schedName;
                    ss << ", " << subType << " " << sched << ", " <<
                      uesPerBeam << " UE per beam, " <<
                      beam << " beam";

                    schedName << "ns3::NrMacScheduler" << subType << sched;

                    AddTestCase (new NrNotchingTestCase (ss.str (), notchedMask1,
                                                         schedName.str (),
                                                         uesPerBeam, beam),
                                 QUICK);
                    AddTestCase (new NrNotchingTestCase (ss.str (), notchedMask2,
                                                         schedName.str (),
                                                         uesPerBeam, beam),
                                 QUICK);
                  }
              }
          }
      }
  }
};

static NrNotchingTestSuite nrNotchingTestSuite;

//!< Nr Notching test suite

}  // namespace ns3

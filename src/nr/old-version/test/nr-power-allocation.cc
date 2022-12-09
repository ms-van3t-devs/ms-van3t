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

#include "ns3/core-module.h"
#include "ns3/test.h"
#include "ns3/nr-spectrum-value-helper.h"


using namespace ns3;

/**
 * \ingroup test
 * \file test-antenna-3gpp-model-conf.cc
 *
 * \brief This test case checks whether the power allocation assigns
 * correctly power over active RBs using the specified power allocation
 * type.
 */

NS_LOG_COMPONENT_DEFINE ("PowerAllocationTestCase");

class PowerAllocationTestCase : public TestCase
{
public:

  PowerAllocationTestCase (const std::string & name);
  virtual ~PowerAllocationTestCase ();

private:
  virtual void DoRun (void);
};

PowerAllocationTestCase::PowerAllocationTestCase (const std::string & name) :
  TestCase (name)
{}

PowerAllocationTestCase::~PowerAllocationTestCase ()
{}

void
PowerAllocationTestCase::DoRun (void)
{
  Ptr<const SpectrumModel> sm =  NrSpectrumValueHelper::GetSpectrumModel (200, 2e9, 15000);

  std::vector<int> activeRbs;
  double totalPower = 30;
  double transmittedTxPsd = 0;
  Ptr<SpectrumValue> txPsd = nullptr;

  NS_LOG_INFO ("Testing for number of RBs:" << sm->GetNumBands ());

  // fill in all RBs
  for (size_t rbId = 0; rbId < sm->GetNumBands (); rbId++)
    {
      activeRbs.push_back (rbId);
    }

  txPsd = NrSpectrumValueHelper::CreateTxPowerSpectralDensity  (totalPower, activeRbs, sm, NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_BW);
  transmittedTxPsd = 10 * log10 (Integral (*txPsd) * 1000);
  NS_TEST_ASSERT_MSG_EQ_TOL (totalPower, transmittedTxPsd, 0.01, "Total power and transmitted power should be equal when all RBs are active regardless power allocation type.");

  NS_LOG_INFO ("Testing for power allocation type: UNIFORM_POWER_ALLOCATION_BW and using RBs: " << activeRbs.size () << " transmitted power is: " << transmittedTxPsd);

  txPsd = NrSpectrumValueHelper::CreateTxPowerSpectralDensity  (totalPower, activeRbs, sm, NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_USED);
  transmittedTxPsd = 10 * log10 (Integral (*txPsd) * 1000);
  NS_TEST_ASSERT_MSG_EQ_TOL (totalPower, transmittedTxPsd, 0.01, "Total power and transmitted power should be equal when all RBs are active regardless power allocation type.");

  NS_LOG_INFO ("Testing for power allocation type: UNIFORM_POWER_ALLOCATION_USED and using RBs: " << activeRbs.size () << " transmitted power is: " << transmittedTxPsd);

  // empty RBs list
  activeRbs.erase (activeRbs.begin (), activeRbs.end ());
  activeRbs.clear ();
  NS_ABORT_IF (!activeRbs.empty ());

  // fill in only half RBs
  for (size_t rbId = 0; rbId < sm->GetNumBands () / 10; rbId++)
    {
      activeRbs.push_back (rbId);
    }

  NS_LOG_INFO ("Testing for number of RBs:" << sm->GetNumBands () / 2);

  txPsd = NrSpectrumValueHelper::CreateTxPowerSpectralDensity  (totalPower, activeRbs, sm, NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_BW);
  transmittedTxPsd = 10 * log10 (Integral (*txPsd) * 1000);

  // if only 10th part of RBs is being used then 10th times lower power should be transmitted
  NS_TEST_ASSERT_MSG_EQ_TOL (totalPower - 10, transmittedTxPsd, 0.05,"If only half of RBs are active then only half of total power should be transmitted when uniform "
                             "power allocation over all bandwidth is being configured.");

  NS_LOG_INFO ("Testing for power allocation type: UNIFORM_POWER_ALLOCATION_BW and using RBs: " << activeRbs.size () << " transmitted power is: " << transmittedTxPsd);

  txPsd = NrSpectrumValueHelper::CreateTxPowerSpectralDensity  (totalPower, activeRbs, sm, NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_USED);
  transmittedTxPsd = 10 * log10 (Integral (*txPsd) * 1000);
  NS_TEST_ASSERT_MSG_EQ_TOL (totalPower, transmittedTxPsd, 0.01, "If only half of RBs are active then the total power should be transmitted when uniform "
                             "power allocation over active RBs is being configured.");

  NS_LOG_INFO ("Testing for power allocation type: UNIFORM_POWER_ALLOCATION_USED and using RBs: " << activeRbs.size () << " transmitted power is: " << transmittedTxPsd);

  // empty RBs list
  activeRbs.erase (activeRbs.begin (), activeRbs.end ());
  activeRbs.clear ();
  NS_ABORT_IF (!activeRbs.empty ());

  // fill in only half RBs
  for (size_t rbId = 0; rbId < sm->GetNumBands () / 10; rbId++)
    {
      activeRbs.push_back (rbId);
    }

  NS_LOG_INFO ("Testing for number of RBs:" << sm->GetNumBands () / 2);

  txPsd = NrSpectrumValueHelper::CreateTxPowerSpectralDensity  (totalPower, activeRbs, sm, NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_BW);
  transmittedTxPsd = 10 * log10 (Integral (*txPsd) * 1000);

  // if only 10th part of RBs is being used then 10th times lower power should be transmitted
  NS_TEST_ASSERT_MSG_EQ_TOL (totalPower - 10, transmittedTxPsd, 0.05,"If only half of RBs are active then only half of total power should be transmitted when uniform "
                             "power allocation over all bandwidth is being configured.");

  NS_LOG_INFO ("Testing for power allocation type: UNIFORM_POWER_ALLOCATION_BW and using RBs: " << activeRbs.size () << " transmitted power is: " << transmittedTxPsd);

  txPsd = NrSpectrumValueHelper::CreateTxPowerSpectralDensity  (totalPower, activeRbs, sm, NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_USED);
  transmittedTxPsd = 10 * log10 (Integral (*txPsd) * 1000);
  NS_TEST_ASSERT_MSG_EQ_TOL (totalPower, transmittedTxPsd, 0.01, "If only half of RBs are active then the total power should be transmitted when uniform "
                             "power allocation over active RBs is being configured.");

  NS_LOG_INFO ("Testing for power allocation type: UNIFORM_POWER_ALLOCATION_USED and using RBs: " << activeRbs.size () << " transmitted power is: " << transmittedTxPsd);


  Simulator::Run ();
  Simulator::Destroy ();
}


class PowerAllocationTestSuite : public TestSuite
{
public:
  PowerAllocationTestSuite ();
};

PowerAllocationTestSuite::PowerAllocationTestSuite ()
  : TestSuite ("nr-power-allocation", SYSTEM)
{
  AddTestCase (new PowerAllocationTestCase ("nr-power-allocation"), TestDuration::QUICK);
}

// Allocate an instance of this TestSuite
static PowerAllocationTestSuite testSuite;

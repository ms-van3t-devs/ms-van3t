/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
#include <ns3/nr-spectrum-value-helper.h>
#include <ns3/spectrum-value.h>
#include <ns3/nr-eesm-error-model.h>
#include <ns3/nr-eesm-ir-t1.h>
#include <ns3/nr-eesm-cc-t1.h>
#include <ns3/ptr.h>
#include <iostream>
#include <cmath>

/**
 * \file nr-test-harq.cc
 * \ingroup test
 *
 * \brief System-testing for effective SINR computation for
 * HARQ Incremental Redundancy (IR) and Chase Combining (CC).
 *
 * Testing values are computed using the equations in:
 * "New Radio Physical Layer Abstraction for System-Level Simulations of 5G Networks,
 * Sandra Lagen, Et al".
 * available at: https://arxiv.org/abs/2001.10309
 */
namespace ns3 {


class TestHarqTestCase : public TestCase
{
public:
  TestHarqTestCase (std::vector<std::vector<double> > rxSinrDb,
                    std::vector<double> refEffSinrPerRx,
                    uint16_t mcs,
                    uint16_t tbSize,
                    const std::string &name)
    : TestCase (name),
      m_rxSinrDb (rxSinrDb),
      m_refEffSinrPerRx (refEffSinrPerRx),
      m_mcs (mcs),
      m_tbSize (tbSize)
  {}

private:
  virtual void DoRun (void) override;
  void ValidateHarqForTwoRx ();
  NrErrorModel::NrErrorModelHistory GetTbDecodStats (std::vector<double> sinrRx, NrErrorModel::NrErrorModelHistory harqHistory, std::string harqType);
  std::vector<std::vector<double> > m_rxSinrDb; //!< SINR (dB) for each RB of each reception (each internal vector is a reception)
  std::vector<double> m_refEffSinrPerRx; //!< Effective SINR values to be used for validation for each HARQ technique
  uint16_t m_mcs {0}; //!< MCS value
  uint16_t m_tbSize {0}; //!< Transport Block (TB) size
};

NrErrorModel::NrErrorModelHistory
TestHarqTestCase::GetTbDecodStats (std::vector<double> sinrRx, NrErrorModel::NrErrorModelHistory harqHistory, std::string harqType)
{
  uint8_t nRbsRx = sinrRx.size ();

  NrSpectrumValueHelper helper;
  Ptr<const SpectrumModel> spectModelRx = helper.GetSpectrumModel (nRbsRx, 3.6e9, 15000);
  SpectrumValue sinrRxSpecVal (spectModelRx);

  for (uint i = 0; i < sinrRx.size (); i++)
    {
      sinrRxSpecVal [i] = pow (10.0, sinrRx.at (i) / 10.0);
    }

  std::vector<int> rbMap;
  int rbIndex = 0;
  for (uint8_t i = 0; i < sinrRx.size (); i++, rbIndex++)
    {
      rbMap.push_back (rbIndex);
    }

  Ptr<NrErrorModelOutput> output;
  if (harqType == "IR")
    {
      Ptr<NrEesmIrT1> errorModelIr = CreateObject<NrEesmIrT1>();
      output = errorModelIr->GetTbDecodificationStats (sinrRxSpecVal, rbMap, m_tbSize, m_mcs, harqHistory);
    }
  else if (harqType == "CC")
    {
      Ptr<NrEesmCcT1> errorModelCc = CreateObject<NrEesmCcT1>();
      output = errorModelCc->GetTbDecodificationStats (sinrRxSpecVal, rbMap, m_tbSize, m_mcs, harqHistory);
    }
  else
    {
      NS_FATAL_ERROR ("Unknown HARQ type. Use IR or CC");
    }

  NrErrorModel::NrErrorModelHistory history;
  history.push_back (output);
  return history;
}

void
TestHarqTestCase::ValidateHarqForTwoRx ()
{
  std::vector<double> sinrRx1 = m_rxSinrDb.at (0);
  std::vector<double> sinrRx2 = m_rxSinrDb.at (1);
  NrErrorModel::NrErrorModelHistory history;

  //Incremental Redundancy
  history = GetTbDecodStats (sinrRx1, history, "IR");
  auto outputIr = history.at (0);
  auto eesmOutputIr = DynamicCast<NrEesmErrorModelOutput> (outputIr);
  double sinrEffIr = eesmOutputIr->m_sinrEff;

  //std::cout << "sinrEff IR Rx1 = " << sinrEffIr << std::endl;
  NS_TEST_ASSERT_MSG_EQ_TOL (sinrEffIr, m_refEffSinrPerRx.at (0), 0.0001, "Resulted effective SINR of IR for RX 1 should be equal to the test value with tol +-0.0001");

  history = GetTbDecodStats (sinrRx2, history, "IR");
  outputIr = history.at (0);
  eesmOutputIr = DynamicCast<NrEesmErrorModelOutput> (outputIr);
  sinrEffIr = eesmOutputIr->m_sinrEff;

  //std::cout << "sinrEff IR Rx2 = " << sinrEffIr << std::endl;
  NS_TEST_ASSERT_MSG_EQ_TOL (sinrEffIr, m_refEffSinrPerRx.at (1), 0.0001, "Resulted effective SINR of IR for RX 2 should be equal to the test value with tol +-0.0001");

  history.clear ();
  // Chase Combining
  history = GetTbDecodStats (sinrRx1, history, "CC");
  auto outputCc = history.at (0);
  auto eesmOutputCc = DynamicCast<NrEesmErrorModelOutput> (outputCc);
  double sinrEffCc = eesmOutputCc->m_sinrEff;

  //std::cout << "sinrEff CC Rx1 = " << sinrEffCc << std::endl;
  NS_TEST_ASSERT_MSG_EQ_TOL (sinrEffCc, m_refEffSinrPerRx.at (2), 0.0001, "Resulted effective SINR of CC for RX 1 should be equal to the test value with tol +-0.0001");

  history = GetTbDecodStats (sinrRx2, history, "CC");
  outputCc = history.at (0);
  eesmOutputCc = DynamicCast<NrEesmErrorModelOutput> (outputCc);
  sinrEffCc = eesmOutputCc->m_sinrEff;

  //std::cout << "sinrEff CC Rx2 = " << sinrEffCc << std::endl;
  NS_TEST_ASSERT_MSG_EQ_TOL (sinrEffCc, m_refEffSinrPerRx.at (3), 0.0001, "Resulted effective SINR of CC for RX 2 should be equal to the test value with tol +-0.0001");
}

void
TestHarqTestCase::DoRun ()
{
  switch (m_rxSinrDb.size ())
  {
    case 2:
      ValidateHarqForTwoRx ();
      break;
    default:
      NS_FATAL_ERROR ("Unsupported number of RX given to test HARQ");
  }


}

class TestHarq : public TestSuite
{
public:
  TestHarq () : TestSuite ("nr-test-harq", SYSTEM)
  {
    //test-1: 2 receptions
    std::vector<std::vector<double> > rxSinrDb;
    std::vector<double> sinrDbRx1 = {1.0, 3.5};
    rxSinrDb.push_back (sinrDbRx1);
    std::vector<double> sinrDbRx2 = {1.0, 1.5, 2.0, 2.5, 3.0, 3.5};
    rxSinrDb.push_back (sinrDbRx2);
    std::vector<double> refEffSinrPerRx;
    //first the effective SINRs of the two receptions with IR
    refEffSinrPerRx.push_back (1.67919); // After 1st RX
    refEffSinrPerRx.push_back (1.67907); //After 2nd RX
    //the last two are the effective SINRs of the two receptions with CC
    refEffSinrPerRx.push_back (1.67919); // After 1st RX
    refEffSinrPerRx.push_back (3.3318); //After 2nd RX
    uint16_t mcs = 5;
    uint16_t tbSize = 256;
    AddTestCase (new TestHarqTestCase (rxSinrDb, refEffSinrPerRx, mcs, tbSize, "HARQ test with 2 receptions"), QUICK);
  }
};

static TestHarq testHarq; //!< HARQ test

}  // namespace ns3

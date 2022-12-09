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

#include "nr-spectrum-phy-test.h"
#include "ns3/nr-spectrum-phy.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/nr-spectrum-value-helper.h"
#include "ns3/nr-interference.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/nr-gnb-phy.h"

namespace ns3 {

NoLossSpectrumPropagationLossModel::NoLossSpectrumPropagationLossModel ()
{}

NoLossSpectrumPropagationLossModel::~NoLossSpectrumPropagationLossModel ()
{}
TypeId
NoLossSpectrumPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NoLossSpectrumPropagationLossModel")
    .SetParent<SpectrumPropagationLossModel> ()
    .SetGroupName ("Spectrum")
  ;
  return tid;
}

Ptr<SpectrumValue>
NoLossSpectrumPropagationLossModel::DoCalcRxPowerSpectralDensity (Ptr<const SpectrumValue> txPsd,
                                                                  Ptr<const MobilityModel> a,
                                                                  Ptr<const MobilityModel> b) const
{
  return Copy (txPsd);
}


SetNoisePsdTestCase::SetNoisePsdTestCase (double txPower, double bandwidth, double noiseFigureFirst, double noiseFigureSecond,
                                          double expectedSnrFirst, double expectedSnrSecond, uint8_t numerology)
  : TestCase ("NrSpectrumPhy configuration test case")
{
  m_txPower = txPower;
  m_bandwidth = bandwidth;
  m_noiseFigureFirst = noiseFigureFirst;
  m_noiseFigureSecond = noiseFigureSecond;
  m_expectedSnrFirst = expectedSnrFirst;
  m_expectedSnrSecond = expectedSnrSecond;
  m_numerology = numerology;
}

SetNoisePsdTestCase::~SetNoisePsdTestCase ()
{}

void SetNoisePsdTestCase::SaveSnr (double snr)
{
  m_snr.push_back (snr);
}

void TestSaveSnr (SetNoisePsdTestCase* test, double snr)
{
  test->SaveSnr (snr);
}

void
SetNoisePsdTestCase::DoEvaluateTest ()
{
  if (m_snr.size () != 2)
    {
      NS_TEST_ASSERT_MSG_EQ (true, false, "Test could not be evaluated, something wrong in test setup. Expects to obtain two SNR values during the simulation.");
    }
  else
    {
      NS_TEST_ASSERT_MSG_NE (m_snr.at (0), m_snr.at (1), "SNR should not be equal because noise figure has been changed.");
      NS_TEST_ASSERT_MSG_EQ_TOL (m_snr.at (0), m_expectedSnrFirst, m_expectedSnrFirst * 0.1, "First SNR is not as expected.");
      NS_TEST_ASSERT_MSG_EQ_TOL (m_snr.at (1), m_expectedSnrSecond, m_expectedSnrSecond * 0.1, "Second SNR is not as expected.");
    }

}

void
SetNoisePsdTestCase::DoRun (void)
{
  double centerFrequency = 28e9;

  Ptr<NrSpectrumPhy> rxPhy = CreateObject<NrSpectrumPhy> ();
  rxPhy->SetMobility (CreateObject<ConstantPositionMobilityModel>());
  Ptr<MultiModelSpectrumChannel> spectrumChannel = CreateObject<MultiModelSpectrumChannel>();
  spectrumChannel->AddSpectrumPropagationLossModel (CreateObject<NoLossSpectrumPropagationLossModel>());
  rxPhy->SetChannel (spectrumChannel);
  Ptr<NrGnbPhy> phy = CreateObject<NrGnbPhy>();
  phy->DoSetCellId (99);
  rxPhy->InstallPhy (phy);
  double subcarrierSpacing = 15000 * static_cast<uint32_t> (std::pow (2, m_numerology));
  uint32_t rbNum = m_bandwidth / (12 * subcarrierSpacing);
  Ptr<const SpectrumModel> sm =  NrSpectrumValueHelper::GetSpectrumModel (rbNum, centerFrequency, subcarrierSpacing);

  std::vector<int> activeRbs;
  for (size_t rbId = 0; rbId < sm->GetNumBands (); rbId++)
    {
      activeRbs.push_back (rbId);
    }

  Ptr<const SpectrumValue> txPsd = NrSpectrumValueHelper::CreateTxPowerSpectralDensity  (m_txPower, activeRbs, sm, NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_BW);
  Ptr<const SpectrumValue> nsv0first = NrSpectrumValueHelper::CreateNoisePowerSpectralDensity (m_noiseFigureFirst, sm);
  Ptr<const SpectrumValue> nsv0second = NrSpectrumValueHelper::CreateNoisePowerSpectralDensity (m_noiseFigureSecond, sm);

  Ptr<NrSpectrumSignalParametersDataFrame> params1 = Create<NrSpectrumSignalParametersDataFrame>();
  params1->duration = Time (MilliSeconds (1));
  params1->psd = Copy (txPsd);
  params1->cellId = 99;
  Ptr<NrSpectrumPhy> txPhy = CreateObject<NrSpectrumPhy>();
  txPhy->SetMobility (CreateObject<ConstantPositionMobilityModel>());
  params1->txPhy = txPhy;

  rxPhy->GetNrInterference ()->TraceConnectWithoutContext ("SnrPerProcessedChunk",
                                                           MakeBoundCallback (&TestSaveSnr, this));

  Simulator::Schedule (MilliSeconds (0), &NrSpectrumPhy::SetNoisePowerSpectralDensity, rxPhy, nsv0first);
  // spectrum phy can be attached to spectrum channel only once that the spectrum model of the spectrum phy is being set
  // spectrum model is being set when noise power spectral density is set for the first time
  Simulator::Schedule (MilliSeconds (0), &MultiModelSpectrumChannel::AddRx, spectrumChannel, rxPhy);
  Simulator::Schedule (MilliSeconds (1), &MultiModelSpectrumChannel::StartTx, spectrumChannel, params1);
  Simulator::Schedule (MilliSeconds (3), &NrInterference::EndRx, rxPhy->GetNrInterference ());

  Simulator::Schedule (MilliSeconds (4), &NrSpectrumPhy::SetNoisePowerSpectralDensity, rxPhy, nsv0second);
  Simulator::Schedule (MilliSeconds (5), &MultiModelSpectrumChannel::StartTx, spectrumChannel, params1);
  Simulator::Schedule (MilliSeconds (7), &NrInterference::EndRx, rxPhy->GetNrInterference ());

  Simulator::Schedule (MilliSeconds (9), &SetNoisePsdTestCase::DoEvaluateTest, this);

  Simulator::Run ();
  Simulator::Destroy ();

}

NrSpectrumPhyTestSuite::NrSpectrumPhyTestSuite ()
  : TestSuite ("nr-spectrum-phy-test")
{
  struct TestInputValues
  {
    double bandwidth {100e6}; //Hz
    double txPower {10}; //dBm
    double noiseFigure1 {5}; //dB
    double noiseFigure2 {6}; //dB
    uint8_t numerology {0}; //integer value
    TestInputValues (double b, double t, double n1, double n2, double u) : bandwidth (b), txPower (t), noiseFigure1 (n1), noiseFigure2 (n2), numerology (u){}
  };

  std::vector <TestInputValues> testInputValuesSet = {
                                                        { 100e6,  10.0, 4, 6, 0},
                                                        { 200e6,   5.0, 5, 6, 0},
                                                        { 300e6,  20.0, 7, 5, 0},
                                                        {   1e9,  30.0, 4, 5, 0},
                                                        {  20e6,   4.0, 5, 6, 0},
                                                        {  10e6,   1.0, 8, 9, 0},
                                                        {   5e6,   1.0, 8, 9, 0},
                                                        { 1.4e6,   0.5, 8, 9, 0},
                                                        { 100e6,  10.0, 4, 6, 1},
                                                        { 200e6,   5.0, 5, 6, 1},
                                                        { 300e6,  20.0, 7, 5, 1},
                                                        {   1e9,  30.0, 4, 5, 1},
                                                        {  20e6,   4.0, 5, 6, 1},
                                                        {  10e6,   1.0, 8, 9, 1},
                                                        {   5e6,   1.0, 8, 9, 1},
                                                        { 1.4e6,   0.5, 8, 9, 1},
                                                        { 100e6,  10.0, 4, 6, 2},
                                                        { 200e6,   5.0, 5, 6, 2},
                                                        { 300e6,  20.0, 7, 5, 2},
                                                        {   1e9,  30.0, 4, 5, 2},
                                                        {  20e6,   4.0, 5, 6, 2},
                                                        {  10e6,   1.0, 8, 9, 2},
                                                        {   5e6,   1.0, 8, 9, 2},
                                                        { 1.4e6,   0.5, 8, 9, 2},
                                                        { 100e6,  10.0, 4, 6, 3},
                                                        { 200e6,   5.0, 5, 6, 3},
                                                        { 300e6,  20.0, 7, 5, 3},
                                                        {   1e9,  30.0, 4, 5, 3},
                                                        {  20e6,   4.0, 5, 6, 3},
                                                        {  10e6,   1.0, 8, 9, 3},
                                                        {   5e6,   1.0, 8, 9, 3},
                                                        //{ 1.4e6,   0.5, 8, 9, 3}, This test case will not work since 1.4MHz cannot s not enough width for numerology 3, because numerology 3 RB width is 1.44 MHz
                                                        { 100e6,  10.0, 4, 6, 4},
                                                        { 200e6,   5.0, 5, 6, 4},
                                                        { 300e6,  20.0, 7, 5, 4},
                                                        {   1e9,  30.0, 4, 5, 4},
                                                        {  20e6,   4.0, 5, 6, 4},
                                                        {  10e6,   1.0, 8, 9, 4},
                                                        {   5e6,   1.0, 8, 9, 4},
                                                        //{ 1.4e6,   0.5, 8, 9, 4} This test case will not work since 1.4MHz cannot is not enough width for numerology 3, because numerology 3 RB width is 2.88 MHz
                                                      };

  for (auto input: testInputValuesSet)
    {
      uint64_t effectiveBandwidth = NrSpectrumValueHelper::GetEffectiveBandwidth (input.bandwidth, input.numerology);
      double txPowerW = std::pow (10., (input.txPower - 30) / 10); // W
      double txPowerWDensity = txPowerW / (double) effectiveBandwidth; // W/Hz
      double kT_W_Hz = std::pow (10.0, (-174.0 - 30) / 10.0); // W/Hz

      //calculate what should be SNR for noise figure 1
      double noiseFigureLinear1 = std::pow (10.0, input.noiseFigure1 / 10.0);
      double noisePowerSpectralDensity1 =  kT_W_Hz * noiseFigureLinear1;
      double expectedSnr1 = txPowerWDensity / noisePowerSpectralDensity1;

      //calculate what should be SNR for noise figure 2
      double noiseFigureLinear2 = std::pow (10.0, input.noiseFigure2 / 10.0);
      double noisePowerSpectralDensity2 =  kT_W_Hz * noiseFigureLinear2;
      double expectedSnr2 = txPowerWDensity / noisePowerSpectralDensity2;

      AddTestCase (new SetNoisePsdTestCase (input.txPower, input.bandwidth, input.noiseFigure1, input.noiseFigure2, expectedSnr1, expectedSnr2, input.numerology),
                   TestDuration::QUICK);
    }
}


// Allocate an instance of this TestSuite
static NrSpectrumPhyTestSuite g_nrSpectrumPhyTestSuite;

}  // namespace ns3

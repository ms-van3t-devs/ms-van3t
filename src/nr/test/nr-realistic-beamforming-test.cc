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

#include <ns3/core-module.h>
#include <ns3/mobility-module.h>
#include <ns3/nr-module.h>
#include <ns3/simple-net-device.h>
#include <ns3/spectrum-model.h>
#include <ns3/three-gpp-channel-model.h>
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/antenna-module.h>
#include <ns3/beamforming-vector.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrRealisticBeamformingTest");

/**
 * \ingroup test
 * \file nr-realistic-beamforming-test.cc
 *
 * \brief This test tests how different levels of received SINR SRS
 * affect the realistic beamforming algorithm performance. What is expected
 * is that when SINR is high that realistic beamforming algorithm will
 * select the same beamforming vector pair as it would ideal beamforming
 * algorithm that has the perfect knowledge of the channel.
 * On the other hand, when SINR is low it is expected that the error in
 * estimation of the channel is high, thus the selected beamforming pair
 * is expected to be different from those that are selected by the ideal
 * beamforming algorithm.
 * Note that as the ideal and realistic beamforming algorithms are not exactly
 * the same, i.e., ideal beamforming algorithm assumes perfect knowledge
 * of the full channel (including long-term component of the fading,
 * the Doppler, and frequency-selectivity) while realistic beamforming
 * algorithm only estimates the long-term component of the fading.
 * Hence, then slight variations on the best beam selection may appear.
 */

class NrRealisticBeamformingTestSuite : public TestSuite
{
public:
  NrRealisticBeamformingTestSuite ();
};

class NrRealisticBeamformingTestCase : public TestCase
{
public:
  NrRealisticBeamformingTestCase (std::string name, enum TestDuration duration);
  virtual ~NrRealisticBeamformingTestCase ();

private:

  virtual void DoRun (void);
  enum TestDuration m_duration {TestCase::QUICK}; //!< the test execution mode type
};


/**
 * TestSuite
 */
NrRealisticBeamformingTestSuite::NrRealisticBeamformingTestSuite ()
  : TestSuite ("nr-realistic-beamforming-test", SYSTEM)
{
  NS_LOG_INFO ("Creating NrRealisticBeamformingTestSuite");


  enum TestDuration durationQuick = TestCase::QUICK;
  enum TestDuration durationExtensive = TestCase::EXTENSIVE;

  AddTestCase (new NrRealisticBeamformingTestCase ("RealisticBeamforming basic test case", durationQuick), durationQuick);
  AddTestCase (new NrRealisticBeamformingTestCase ("RealisticBeamforming basic test case", durationExtensive), durationExtensive);


}

/**
 * TestCase
 */

NrRealisticBeamformingTestCase::NrRealisticBeamformingTestCase (std::string name, enum TestDuration duration) : TestCase (name)
{
  m_duration = duration;
}

NrRealisticBeamformingTestCase::~NrRealisticBeamformingTestCase ()
{}

void
NrRealisticBeamformingTestCase::DoRun (void)
{
  RngSeedManager::SetSeed (1);

  std::list<Vector> uePositionsExtensive = { Vector ( 10,-10, 1.5),
                                             Vector (  0, 10, 1.5),
                                             Vector (  0,-10, 1.5)};


  uint16_t totalCounter = 0, highSinrCounter = 0, lowSinrCounter = 0;

  std::list <uint16_t> rngList = (m_duration == TestCase::EXTENSIVE) ? std::list<uint16_t> ({2,3}) : std::list<uint16_t> ({1});

  std::list <Vector> uePositions = (m_duration == TestCase::EXTENSIVE) ? uePositionsExtensive : std::list<Vector> ( { Vector ( 10, 10, 1.5),
                                                                                                                      Vector (-10, 10, 1.5)});

  std::list<uint16_t> antennaConfList = (m_duration == TestCase::EXTENSIVE) ? std::list<uint16_t> ({3, 4}) : std::list<uint16_t> ({2});

  for (auto rng:rngList)
    {
      RngSeedManager::SetRun (rng);

      for (const auto& pos:uePositions)
        {
          for (const auto& antennaConf:antennaConfList)
            {
              for (auto iso:{false, true})
                {
                  totalCounter++;

                  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
                  // Create Nodes: eNodeB and UE
                  NodeContainer gnbNodes;
                  NodeContainer ueNodes;
                  gnbNodes.Create (1);
                  ueNodes.Create (1);
                  NodeContainer allNodes = NodeContainer (gnbNodes, ueNodes);

                  // Install Mobility Model
                  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
                  positionAlloc->Add (Vector (0, 0.0, 10)); // gNB
                  positionAlloc->Add (pos);  // UE

                  MobilityHelper mobility;
                  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
                  mobility.SetPositionAllocator (positionAlloc);
                  mobility.Install (allNodes);

                  // Create Devices and install them in the Nodes (eNB and UE)
                  NetDeviceContainer gnbDevs;
                  NetDeviceContainer ueDevs;
                  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

                  CcBwpCreator::SimpleOperationBandConf bandConf (29e9, 100e6, 1, BandwidthPartInfo::UMa_LoS);
                  CcBwpCreator ccBwpCreator;
                  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);
                  //Initialize channel and pathloss, plus other things inside band.
                  nrHelper->InitializeOperationBand (&band);
                  BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps ({band});

                  // Antennas for the gNbs
                  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (antennaConf));
                  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (antennaConf));

                  // Antennas for the UEs
                  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (antennaConf));
                  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (antennaConf));

                  //Antenna element type for both gNB and UE
                  if (iso)
                    {
                      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
                      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
                    }
                  else
                    {
                      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
                      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
                    }

                  nrHelper->SetGnbBeamManagerTypeId (RealisticBfManager::GetTypeId ());

                  gnbDevs = nrHelper->InstallGnbDevice (gnbNodes, allBwps);
                  ueDevs = nrHelper->InstallUeDevice (ueNodes, allBwps);

                  for (auto it = gnbDevs.Begin (); it != gnbDevs.End (); ++it)
                    {
                      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
                    }

                  for (auto it = ueDevs.Begin (); it != ueDevs.End (); ++it)
                    {
                      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
                    }

                  Ptr<NrUePhy> uePhy = nrHelper->GetUePhy (ueDevs.Get (0), 0);

                  Ptr<NrSpectrumPhy> txSpectrumPhy = nrHelper->GetGnbPhy (gnbDevs.Get (0), 0)->GetSpectrumPhy (0);
                  Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel ();
                  Ptr<ThreeGppPropagationLossModel> propagationLossModel =  DynamicCast<ThreeGppPropagationLossModel> (txSpectrumChannel->GetPropagationLossModel ());
                  NS_ASSERT (propagationLossModel != nullptr);
                  propagationLossModel->AssignStreams (1);
                  Ptr<ChannelConditionModel> channelConditionModel = propagationLossModel->GetChannelConditionModel ();
                  channelConditionModel->AssignStreams (1);
                  Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel = DynamicCast<ThreeGppSpectrumPropagationLossModel> (txSpectrumChannel->GetPhasedArraySpectrumPropagationLossModel());
                  NS_ASSERT (spectrumLossModel != nullptr);
                  Ptr <ThreeGppChannelModel> channel = DynamicCast<ThreeGppChannelModel> (spectrumLossModel->GetChannelModel ());
                  channel->AssignStreams (1);

                  double sinrSrsHighLineal = std::pow (10.0, 0.1 * 40);
                  double sinrSrsLowLineal = std::pow (10.0, 0.1 * (-10));

                  Ptr<CellScanBeamforming> cellScanBeamforming = CreateObject<CellScanBeamforming>();

                  BeamformingVectorPair bfPairIdeal = cellScanBeamforming->GetBeamformingVectors (txSpectrumPhy, uePhy->GetSpectrumPhy(0));

                  Ptr<RealisticBeamformingAlgorithm> realisticBeamforming = CreateObject<RealisticBeamformingAlgorithm>();
                  realisticBeamforming->Install (DynamicCast<NrGnbNetDevice> (gnbDevs.Get (0)),
                                                 DynamicCast<NrUeNetDevice> (ueDevs.Get (0)),
                                                 txSpectrumPhy,
                                                 uePhy->GetSpectrumPhy(0),
                                                 DynamicCast<NrGnbNetDevice> (gnbDevs.Get (0))->GetScheduler(0)
                                                );

                  // directly update max SINR SRS to a high value, skipping other set functions of the algorithm
                  realisticBeamforming->m_maxSrsSinrPerSlot = sinrSrsHighLineal;

                  BeamformingVectorPair bfPairReal1 = realisticBeamforming->GetBeamformingVectors ();

                  // directly update max SINR SRS to a new lower value, skipping other set functions of the algorithm,
                  realisticBeamforming->m_maxSrsSinrPerSlot = sinrSrsLowLineal;

                  BeamformingVectorPair bfPairReal2 = realisticBeamforming->GetBeamformingVectors ();

                  if ((bfPairIdeal.first.second == bfPairReal1.first.second) && (bfPairIdeal.second.second == bfPairReal1.second.second))
                    {
                      highSinrCounter++;
                    }

                  if (!((bfPairIdeal.first.second == bfPairReal2.first.second) && (bfPairIdeal.second.second == bfPairReal2.second.second)))
                    {
                      lowSinrCounter++;
                    }
                }
            }
        }
    }

  double tolerance = 0.2;
  if (m_duration == TestCase::EXTENSIVE)
    {
      tolerance = 0.2;
    }
  else
    {
      tolerance = 0.3;  // relax tolerance for QUICK mode since there are only 4 test configurations, e.g.,
                        // if 3 results of 4 are as expected that is already enough, but that gives 0.75 thus
                        // it needs larger tolerance than 0.2 which is fine for EXTENSIVE mode
    }

  NS_TEST_ASSERT_MSG_EQ_TOL (highSinrCounter / (double) totalCounter, 1, tolerance,
                             "The pair of beamforming vectors should be equal in most of the cases when SINR is high, and they are not");
  NS_TEST_ASSERT_MSG_EQ_TOL (lowSinrCounter / (double) totalCounter, 1, tolerance,
                             "The pair of beamforming vectors should not be equal in most of the cases when SINR is low, and they are");



  NS_LOG_INFO ("The result is as expected when high SINR in " << highSinrCounter << " out of " << totalCounter << " total cases.");
  NS_LOG_INFO ("The result is as expected when low SINR in " << lowSinrCounter << " out of " << totalCounter << " total cases.");

  Simulator::Destroy ();
}

// Do not forget to allocate an instance of this TestSuite
static NrRealisticBeamformingTestSuite nrTestSuite;

}//namespace ns-3

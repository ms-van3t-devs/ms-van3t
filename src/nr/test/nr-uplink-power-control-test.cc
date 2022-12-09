/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "ns3/test.h"
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/callback.h>
#include <ns3/config.h>
#include <ns3/string.h>
#include <ns3/double.h>
#include <ns3/boolean.h>
#include <ns3/pointer.h>
#include <ns3/integer.h>
#include <ns3/mobility-helper.h>
#include <ns3/spectrum-value.h>
#include <ns3/ff-mac-scheduler.h>
#include <ns3/nr-module.h>
#include <ns3/rng-seed-manager.h>
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/antenna-module.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("NrUplinkPowerControlTestCase");

/**
 * \file nr-test-uplink-power-control.cc
 * \ingroup test
 *
 * \brief Test suite for NrUplinkPowerControlTestCase.
 *
 */
class NrUplinkPowerControlTestSuite : public TestSuite
{
public:
  NrUplinkPowerControlTestSuite ();
};


/**
 * \file nr-test-uplink-power-control.cc
 * \ingroup test
 *
 * \brief NR uplink power control test case. Tests PUSCH and PUCCH
 * power control adaptation. Move UE to different positions and check
 * whether the power is adjusted as expected (open loop, closed loop
 * absolute/accumulated mode).
 */
class NrUplinkPowerControlTestCase : public TestCase
{
public:
  /**
   * \brief Constructor
   * \param name the test case name
   * \param closedLoop - whether open or closed loop mode will be activated, if true closed loop will be used, if false open loop
   * \param accumulatedMode - if closed loop is activated then this variable defines whether absolute or accumulation mode is being used
   */
  NrUplinkPowerControlTestCase (std::string name, bool closedLoop, bool accumulatedMode);
  /**
   * \brief Destructor
   */
  virtual ~NrUplinkPowerControlTestCase ();

  /**
   * Function that moves the UE to a different position
   *
   * \param distance a new distance to be set between UE and gNB
   * \param expectedPuschTxPower the expected PUSCH transmit power in dBm after moving to a new position
   * \param expectedPucchTxPower the expected PUCCH transmit power in dBm after moving to a new position
   */
  void MoveUe (uint32_t distance, double expectedPuschTxPower, double expectedPucchTxPower);

  /**
   * PUSCH transmit power trace function
   * \param cellId the cell ID
   * \param rnti the RNTI
   * \param txPower the transmit power in dBm
   */
  void PuschTxPowerTrace (uint16_t cellId, uint16_t rnti, double txPower);
  /**
   * PUCCH transmit power trace function
   * \param cellId the cell ID
   * \param rnti the RNTI
   * \param txPower the transmit power in dBm
   */
  void PucchTxPowerTrace (uint16_t cellId, uint16_t rnti, double txPower);

protected:
  virtual void DoRun (void);

  Ptr<MobilityModel> m_ueMobility;      //!< UE mobility model
  Ptr<NrUePowerControl> m_ueUpc;        //!< UE uplink power control
  Time m_movingTime;                    //!< moving time
  double m_expectedPuschTxPower {0.0};  //!< expected PUSCH transmit power in dBm
  double m_expectedPucchTxPower {0.0};  //!< expected PUCCH transmit power in dBm
  bool m_closedLoop {true};             //!< indicates whether open or closed loops is being used
  bool m_accumulatedMode {true};        //!< if closed loop is configured, this would indicate the type of a TPC mode to be used for the closed loop power control.
  bool m_puschTxPowerTraceFired {true}; //!< flag to indicate if the trace, which calls the test function got executed
  bool m_pucchTxPowerTraceFired {true}; //!< Flag to indicate if the trace, which calls the test function got executed
};


NrUplinkPowerControlTestSuite::NrUplinkPowerControlTestSuite ()
  : TestSuite ("nr-uplink-power-control-test", SYSTEM)
{
  //LogLevel logLevel = (LogLevel)(LOG_PREFIX_FUNC | LOG_PREFIX_TIME | LOG_LEVEL_DEBUG);
  //LogComponentEnable ("NrUplinkPowerControlTestSuite", logLevel);
  NS_LOG_INFO ("Creating NrUplinkPowerControlTestSuite");
  AddTestCase (new NrUplinkPowerControlTestCase ("OpenLoopPowerControlTest", false, false), TestCase::QUICK);
  AddTestCase (new NrUplinkPowerControlTestCase ("ClosedLoopPowerControlAbsoluteModeTest", true, false), TestCase::QUICK);
  AddTestCase (new NrUplinkPowerControlTestCase ("ClosedLoopPowerControlAccumulatedModeTest", true, true), TestCase::QUICK);
}

static NrUplinkPowerControlTestSuite lteUplinkPowerControlTestSuite;

void
PuschTxPowerReport (NrUplinkPowerControlTestCase *testcase,
                    uint16_t cellId, uint16_t rnti, double txPower)
{
  testcase->PuschTxPowerTrace (cellId, rnti, txPower);
}

void
PucchTxPowerReport (NrUplinkPowerControlTestCase *testcase,
                    uint16_t cellId, uint16_t rnti, double txPower)
{
  testcase->PucchTxPowerTrace (cellId, rnti, txPower);
}

NrUplinkPowerControlTestCase::NrUplinkPowerControlTestCase (std::string name, bool closedLoop, bool accumulatedMode)
  : TestCase (name)
{
  NS_LOG_INFO ("Creating NrUplinkPowerControlTestCase");
  m_closedLoop = closedLoop;
  m_accumulatedMode = accumulatedMode; // if closed loop is configured, this would indicate the type of a TPC mode to be used for the closed loop power control.
}

NrUplinkPowerControlTestCase::~NrUplinkPowerControlTestCase ()
{}

void
NrUplinkPowerControlTestCase::MoveUe (uint32_t distance, double expectedPuschTxPower, double expectedPucchTxPower)
{
  NS_LOG_FUNCTION (this);

  NS_TEST_ASSERT_MSG_EQ (m_pucchTxPowerTraceFired, true, "Power trace for PUCCH did not get triggered. Test check for PUCCH did not execute as expected. ");
  m_pucchTxPowerTraceFired = false; // reset
  NS_TEST_ASSERT_MSG_EQ (m_puschTxPowerTraceFired, true, "Power trace for PUSCH did not get triggered. Test check did PUSCH not execute as expected. ");
  m_puschTxPowerTraceFired = false; // reset
  Vector newPosition = m_ueMobility->GetPosition ();
  newPosition.x = distance;
  m_ueMobility->SetPosition (newPosition);
  NS_LOG_DEBUG ("Move UE to : " << m_ueMobility->GetPosition ());
  m_movingTime = Simulator::Now ();
  m_expectedPuschTxPower = expectedPuschTxPower;
  m_expectedPucchTxPower = expectedPucchTxPower;
}

void
NrUplinkPowerControlTestCase::PuschTxPowerTrace (uint16_t cellId, uint16_t rnti, double txPower)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("PuschTxPower for CellId: " << cellId << " RNTI: " << rnti << " PuschTxPower: " << txPower);
  //wait because of RSRP filtering
  if ((Simulator::Now () - m_movingTime ) < MilliSeconds (50))
    {
      return;
    }
  else
    {
      if (m_puschTxPowerTraceFired == false)
        {
          m_puschTxPowerTraceFired = true;
        }
    }

  // we allow some tollerance because of layer 3 filtering
  NS_TEST_ASSERT_MSG_EQ_TOL (txPower, m_expectedPuschTxPower, 1 + abs (m_expectedPuschTxPower * 0.1), "Wrong Pusch Tx Power");
}

void
NrUplinkPowerControlTestCase::PucchTxPowerTrace (uint16_t cellId, uint16_t rnti, double txPower)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("PucchTxPower : CellId: " << cellId << " RNTI: " << rnti << " PuschTxPower: " << txPower);
  //wait because of RSRP filtering
  if ( (Simulator::Now () - m_movingTime ) < MilliSeconds (50))
    {
      return;
    }
  else
    {
      if (m_pucchTxPowerTraceFired == false)
        {
          m_pucchTxPowerTraceFired = true;
        }
    }

  // we allow some tollerance because of layer 3 filtering
  NS_TEST_ASSERT_MSG_EQ_TOL (txPower, m_expectedPucchTxPower, 1 + abs (m_expectedPucchTxPower * 0.1), "Wrong Pucch Tx Power");

}

void
NrUplinkPowerControlTestCase::DoRun (void)
{
  std::string scenario = "InH-OfficeMixed"; //scenario
  double frequency = 2e9; // central frequency
  double bandwidth = 4.6e6; //bandwidth
  double hBS = 1.5; // base station antenna height in meters, and
  double hUT = 1.5; // user antenna height in meters
  double gNBTxPower = 30; // gNb tx power
  double ueTxPower = 10; // ue tx power
  enum BandwidthPartInfo::Scenario scenarioEnum = BandwidthPartInfo::InH_OfficeMixed_LoS;
  uint16_t numerology = 0; // numerology to be used
  uint16_t numCcPerBand = 1; // number of component carrier in the assigned band
  Time udpAppStartTime = MilliSeconds (50);
  Time simTime = MilliSeconds (2500);

  Config::Reset ();

  RngSeedManager::SetSeed (1);
  RngSeedManager::SetRun (1);

  Config::SetDefault ("ns3::NrUePhy::EnableUplinkPowerControl", BooleanValue (true));
  Config::SetDefault ("ns3::NrUePowerControl::ClosedLoop", BooleanValue (m_closedLoop));
  Config::SetDefault ("ns3::NrUePowerControl::AccumulationEnabled", BooleanValue (m_accumulatedMode));
  Config::SetDefault ("ns3::NrUePowerControl::PoNominalPusch", IntegerValue (-90));
  Config::SetDefault ("ns3::NrUePowerControl::PoNominalPucch", IntegerValue (-80));
  Config::SetDefault ("ns3::NrUePowerControl::PsrsOffset", IntegerValue (9));
  Config::SetDefault ("ns3::ThreeGppPropagationLossModel::ShadowingEnabled", BooleanValue (false));

  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject <IdealBeamformingHelper> ();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  // Create Nodes: eNodeB and UE
  NodeContainer gnbNodes;
  NodeContainer ueNodes;
  gnbNodes.Create (1);
  ueNodes.Create (1);
  NodeContainer allNodes = NodeContainer (gnbNodes, ueNodes);

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.1, 0.0, hBS)); // gNB
  positionAlloc->Add (Vector (0, 0.0, hUT));  // UE

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (allNodes);
  m_ueMobility = ueNodes.Get (0)->GetObject<MobilityModel> ();

  // Create Devices and install them in the Nodes (eNB and UE)
  NetDeviceContainer gnbDevs;
  NetDeviceContainer ueDevs;
  nrHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (numerology));
  nrHelper->SetGnbPhyAttribute ("TxPower", DoubleValue (gNBTxPower));
  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (ueTxPower));
  nrHelper->SetUePhyAttribute ("EnableUplinkPowerControl", BooleanValue (true));

  CcBwpCreator::SimpleOperationBandConf bandConf (frequency, bandwidth, numCcPerBand, scenarioEnum);
  CcBwpCreator ccBwpCreator;
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);
  //Initialize channel and pathloss, plus other things inside band.
  nrHelper->InitializeOperationBand (&band, NrHelper::INIT_PROPAGATION | NrHelper::INIT_CHANNEL);
  BandwidthPartInfoPtrVector allBwps = CcBwpCreator::GetAllBwps ({band});

  // Configure ideal beamforming method
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));

  // Antennas for the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  gnbDevs = nrHelper->InstallGnbDevice (gnbNodes, allBwps);
  ueDevs = nrHelper->InstallUeDevice (ueNodes, allBwps);

  Ptr<NrUePhy> uePhy = nrHelper->GetUePhy (ueDevs.Get (0), 0);

  m_ueUpc = uePhy->GetUplinkPowerControl ();

  m_ueUpc->TraceConnectWithoutContext ("ReportPuschTxPower",
                                       MakeBoundCallback (&PuschTxPowerReport, this));
  m_ueUpc->TraceConnectWithoutContext ("ReportPucchTxPower",
                                       MakeBoundCallback (&PucchTxPowerReport, this));


  Ptr<const NrSpectrumPhy> txSpectrumPhy = nrHelper->GetGnbPhy (gnbDevs.Get (0), 0)->GetSpectrumPhy ();
  Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel ();
  Ptr<ThreeGppPropagationLossModel> propagationLossModel =  DynamicCast<ThreeGppPropagationLossModel> (txSpectrumChannel->GetPropagationLossModel ());
  NS_ASSERT (propagationLossModel != nullptr);
  propagationLossModel->AssignStreams (1);
  Ptr<ChannelConditionModel> channelConditionModel = propagationLossModel->GetChannelConditionModel ();
  channelConditionModel->AssignStreams (1);
  Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel = DynamicCast<ThreeGppSpectrumPropagationLossModel> (txSpectrumChannel->GetPhasedArraySpectrumPropagationLossModel ());
  NS_ASSERT_MSG (spectrumLossModel == nullptr, "3GPP spectrum model should be disabled in this test to have deterministic behaviour.");

  // When all the configuration is done, explicitly call UpdateConfig ()
  for (auto it = gnbDevs.Begin (); it != gnbDevs.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueDevs.Begin (); it != ueDevs.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }

  // traffic configuration
  Ptr<Node> pgw = epcHelper->GetPgwNode ();
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // connect a remoteHost to pgw. Setup routing too
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (2500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.000)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);

  Ipv4InterfaceContainer ueIpIface = epcHelper->AssignUeIpv4Address (ueDevs);
  // Set the default gateway for the UE
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (0)->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  // Attach a UE to a gNB
  nrHelper->AttachToEnb (ueDevs.Get (0), gnbDevs.Get (0));

  /*
   * Traffic part. Install two kinds of traffic: low-latency and voice, each
   * identified by a particular source port.
   */
  uint16_t dlPort = 1234;
  uint16_t ulPort = 1236;

  ApplicationContainer serverApps;
  // The sink will always listen to the specified ports
  UdpServerHelper dlPacketSink (dlPort);
  UdpServerHelper ulPacketSink (ulPort);

  // The server, that is the application which is listening, is installed in the UE
  // for the DL traffic, and in the remote host for the UL traffic
  serverApps.Add (dlPacketSink.Install (ueNodes.Get (0)));
  serverApps.Add (ulPacketSink.Install (remoteHost));

  UdpClientHelper dlClient;
  dlClient.SetAttribute ("RemotePort", UintegerValue (dlPort));
  dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  dlClient.SetAttribute ("PacketSize", UintegerValue (100));
  dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds (1)));
  EpsBearer dlBearer (EpsBearer::GBR_CONV_VIDEO);
  Ptr<EpcTft> dlTft = Create<EpcTft> ();
  EpcTft::PacketFilter dlpf;
  dlpf.localPortStart = dlPort;
  dlpf.localPortEnd = dlPort;
  dlTft->Add (dlpf);

  UdpClientHelper ulClient;
  ulClient.SetAttribute ("RemotePort", UintegerValue (ulPort));
  ulClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  ulClient.SetAttribute ("PacketSize", UintegerValue (100));
  ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds (1)));
  EpsBearer ulBearer (EpsBearer::GBR_CONV_VIDEO);
  Ptr<EpcTft> ulTft = Create<EpcTft> ();
  EpcTft::PacketFilter ulpf;
  ulpf.remotePortStart = ulPort;
  ulpf.remotePortEnd = ulPort;
  ulpf.direction = EpcTft::UPLINK;
  ulTft->Add (ulpf);

  ApplicationContainer clientApps;
  //set and add downlink app to container
  Address ueAddress = ueIpIface.GetAddress (0);
  dlClient.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
  clientApps.Add (dlClient.Install (remoteHost));
  nrHelper->ActivateDedicatedEpsBearer (ueDevs.Get (0), dlBearer, dlTft);
  //set and add uplink app to container
  ulClient.SetAttribute ("RemoteAddress", AddressValue (internetIpIfaces.GetAddress (1)));
  clientApps.Add (ulClient.Install (ueNodes.Get (0)));
  nrHelper->ActivateDedicatedEpsBearer (ueDevs.Get (0), ulBearer, ulTft);

  // start UDP server and client apps
  serverApps.Start (udpAppStartTime);
  clientApps.Start (udpAppStartTime);
  serverApps.Stop (simTime);
  clientApps.Stop (simTime);


  /** Formula for the pathloss can be found in propagation/three-gpp-propagation-loss-model.cc
   * in ThreeGppIndoorOfficePropagationLossModel::GetLossLos function.
   * That function computes the pathloss according to 3GPP TR 38.901, Table 7.4.1-1., i.e.,
   * the formula is the following:
   * double loss = 32.4 + 17.3 * log10 (distance3D) + 20.0 * log10 (m_frequency / 1e9);
   * Hence, e.g. for distance of 90 meters we have:
   * loss = 32.4 + 17.3 * log10 (10) + 20.0 * log10 (2e9/1e9)
   * loss = 72.229
   *
   * So, then the open loop uplink power control for that value can be calculated:
   *
   * distance = 10;
   * rbNum (data) = 24 (4.6 MHz -> 24 RBs)
   * numerology = 0
   * pathloss =  32.4 + 17.3 * log10 (distance) + 20.0 * log10 (freq/1e9)
   * pathloss =  32.4 + 17.3 * log10 (10) + 20.0 * log10 (2e9/1e9)
   * pathloss = 55.7206
   *
   * txPower for PUSCH:
   * txPower = PoPusch + 10 * log10 (rbNum) + alpha * pathLoss + deltaTF + fc
   * txPower = -90 + 0 + 10 * log10 (24) + 1 * (55.7206)  + 0 + 0
   * txPower = -20.4773
   *
   * txPower = min (max (Pcmin, txPower), Pcmax)
   * txPower = min (max (-40, txPower), 23)
   * txPower = -20.4773
   */

  //Changing UE position
  if (!m_closedLoop)
    {
      Simulator::Schedule (MilliSeconds (0),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 10, -21, -11 );
      Simulator::Schedule (MilliSeconds (200),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 100, -3, 7 );
      Simulator::Schedule (MilliSeconds (400),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 200, 2, 12);
      Simulator::Schedule (MilliSeconds (600),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 300, 5, 15 );
      Simulator::Schedule (MilliSeconds (800),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 400, 7, 17 );
      Simulator::Schedule (MilliSeconds (1000),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 600, 10, 20 );
      Simulator::Schedule (MilliSeconds (1200),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 800, 12, 22 );
      Simulator::Schedule (MilliSeconds (1400),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 1000, 14, 23 );
      Simulator::Schedule (MilliSeconds (1600),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 10, -20, -10 );
      Simulator::Schedule (MilliSeconds (1800),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 100, -3, 7 );
      Simulator::Schedule (MilliSeconds (2000),
                           &NrUplinkPowerControlTestCase::MoveUe, this, 1000, 14, 23 );
    }
  else
    {
      /**
       *  By default the TPC command is 1 which is mapped to 0 for Accumulated mode,
       *  and to -1 in Absolute mode, TS38.213 Table Table 7.1.1-1
       */
      if (m_accumulatedMode)
        {
          Simulator::Schedule (MilliSeconds (0),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 10, -21, -11 );
          Simulator::Schedule (MilliSeconds (200),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 100, -3, 7 );
          Simulator::Schedule (MilliSeconds (400),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 200, 2, 12);
          Simulator::Schedule (MilliSeconds (600),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 300, 5, 15 );
          Simulator::Schedule (MilliSeconds (800),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 400, 7, 17 );
          Simulator::Schedule (MilliSeconds (1000),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 600, 10, 20 );
          Simulator::Schedule (MilliSeconds (1200),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 800, 12, 22 );
          Simulator::Schedule (MilliSeconds (1400),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 1000, 14, 23 );
          Simulator::Schedule (MilliSeconds (1600),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 10, -20, -10 );
          Simulator::Schedule (MilliSeconds (1800),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 100, -3, 7 );
          Simulator::Schedule (MilliSeconds (2000),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 1000, 14, 23 );
        }
      else
        {
          Simulator::Schedule (MilliSeconds (0),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 10, -22, -12 );
          Simulator::Schedule (MilliSeconds (200),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 100, -4, 6 );
          Simulator::Schedule (MilliSeconds (400),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 200, 1, 11);
          Simulator::Schedule (MilliSeconds (600),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 300, 4, 14 );
          Simulator::Schedule (MilliSeconds (800),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 400, 6, 16 );
          Simulator::Schedule (MilliSeconds (1000),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 600, 9, 19 );
          Simulator::Schedule (MilliSeconds (1200),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 800, 11, 21 );
          Simulator::Schedule (MilliSeconds (1400),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 1000, 13, 23 );
          Simulator::Schedule (MilliSeconds (1600),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 10, -21, -11 );
          Simulator::Schedule (MilliSeconds (1800),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 100, -4, 6 );
          Simulator::Schedule (MilliSeconds (2000),
                               &NrUplinkPowerControlTestCase::MoveUe, this, 1000, 13, 23 );
        }
    }

  Simulator::Stop (simTime);
  Simulator::Run ();

  Simulator::Destroy ();
}

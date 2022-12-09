/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/nr-module.h"
#include "ns3/config-store-module.h"
#include "ns3/test.h"
#include "ns3/antenna-module.h"


using namespace ns3;

/**
 * \ingroup test
 * \file test-antenna-3gpp-model-conf.cc
 *
 * \brief This test case checks if the throughput/SINR/MCS
 * obtained is as expected for the configured antenna model and for
 * different positions of UE. The test scenario consists of a scenario in
 * which a single UE is attached to a gNB.
 * UE performs a UDP full buffer downlink traffic.
 * gNB is configured to have 1 bandwidth part.
 * Currently there are 2 types of antenna elements: omni and 3gpp directional.
 *
 */


class TestAntenna3gppModelConf : public TestCase
{
public:

  enum DirectionGnbUeXYAngle
  {
    DirectionGnbUe_45,
    DirectionGnbUe_135,
    DirectionGnbUe_225,
    DirectionGnbUe_315,
    DirectionGnbUe_0,
    DirectionGnbUe_90,
    DirectionGnbUe_180,
    DirectionGnbUe_270,
  };



  TestAntenna3gppModelConf (const std::string & name, DirectionGnbUeXYAngle conf,
                            bool gNbOmniAntennaElem, bool ueOmniAntennaElem,
                            uint8_t ueNoOfAntennas, std::string losCondition);
  virtual ~TestAntenna3gppModelConf ();
  void UeReception (RxPacketTraceParams params);

private:

  virtual void DoRun (void);
  std::string m_name;
  DirectionGnbUeXYAngle m_conf;
  bool m_ueOmniAntennaElem;
  bool m_gNbOmniAntennaElem;

  uint8_t m_ueNoOfAntennas;
  std::string m_losCondition;
  Ptr<MinMaxAvgTotalCalculator<double> > m_sinrCell1;
  Ptr<MinMaxAvgTotalCalculator<double> > m_sinrCell2;
  Ptr<MinMaxAvgTotalCalculator<double> > m_mcsCell1;
  Ptr<MinMaxAvgTotalCalculator<double> > m_mcsCell2;
  Ptr<MinMaxAvgTotalCalculator<double> > m_rbNumCell1;
  Ptr<MinMaxAvgTotalCalculator<double> > m_rbNumCell2;

};

void UETraceReception (TestAntenna3gppModelConf* test, RxPacketTraceParams params)
{
  test->UeReception (params);
}



void
TestAntenna3gppModelConf::UeReception (RxPacketTraceParams params)
{
  if (params.m_cellId == 1)
    {
      m_sinrCell1->Update (params.m_sinr);
      m_mcsCell1->Update (params.m_mcs);
      m_rbNumCell1->Update (params.m_rbAssignedNum);
    }
  else if (params.m_cellId == 2)
    {
      m_sinrCell2->Update (params.m_sinr);
      m_mcsCell2->Update (params.m_mcs);
      m_rbNumCell2->Update (params.m_rbAssignedNum);
    }
  else
    {
      NS_ABORT_MSG ("Cell does not exist ... ");
    }
}

TestAntenna3gppModelConf::TestAntenna3gppModelConf (const std::string & name,
                                                    DirectionGnbUeXYAngle conf,
                                                    bool gNbOmniAntennaElem,
                                                    bool ueOmniAntennaElem,
                                                    uint8_t ueNoOfAntennas,
                                                    std::string losCondition)
  : TestCase (name)
{
  m_name = name;
  m_conf = conf;
  m_gNbOmniAntennaElem = gNbOmniAntennaElem;
  m_ueOmniAntennaElem = ueOmniAntennaElem;
  m_ueNoOfAntennas = ueNoOfAntennas;
  m_losCondition = losCondition;
  m_sinrCell1 = Create<MinMaxAvgTotalCalculator<double> >();
  m_sinrCell2 = Create<MinMaxAvgTotalCalculator<double> >();
  m_mcsCell1 = Create<MinMaxAvgTotalCalculator<double> >();
  m_mcsCell2 = Create<MinMaxAvgTotalCalculator<double> >();
  m_rbNumCell1 = Create<MinMaxAvgTotalCalculator<double> >();
  m_rbNumCell2 = Create<MinMaxAvgTotalCalculator<double> >();
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
TestAntenna3gppModelConf::~TestAntenna3gppModelConf ()
{}

void
TestAntenna3gppModelConf::DoRun (void)
{
  std::cout << "\n\n\n" << m_name << std::endl;
  // set simulation time and mobility
  Time simTime = MilliSeconds (1000);
  Time udpAppStartTimeDl = MilliSeconds (400);
  Time udpAppStopTimeDl = MilliSeconds (1000);
  uint32_t packetSize = 1000;
  DataRate udpRate = DataRate ("2Mbps");

  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));
  Config::SetDefault ("ns3::EpsBearer::Release", UintegerValue (15));


  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 1.5;
  double ueHeight = 1.5;
  gNbNodes.Create (1);
  ueNodes.Create (1);

  Ptr<ListPositionAllocator> gNbPositionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();


  gNbPositionAlloc->Add (Vector (0,0,gNbHeight));

  if (m_conf == DirectionGnbUe_45)
    {
      uePositionAlloc->Add (Vector (20,20,ueHeight));
    }
  else if (m_conf == DirectionGnbUe_135)
    {
      uePositionAlloc->Add (Vector (-20,20,ueHeight));
    }
  else if (m_conf == DirectionGnbUe_225)
    {
      uePositionAlloc->Add (Vector (-20,-20,ueHeight));
    }
  else if (m_conf == DirectionGnbUe_315)
    {
      uePositionAlloc->Add (Vector (20,-20,ueHeight));
    }
  else if (m_conf == DirectionGnbUe_0)
    {
      uePositionAlloc->Add (Vector (20,0,ueHeight));
    }
  else if (m_conf == DirectionGnbUe_90)
    {
      uePositionAlloc->Add (Vector (0,20,ueHeight));
    }
  else if (m_conf == DirectionGnbUe_180)
    {
      uePositionAlloc->Add (Vector (-20, 0,ueHeight));
    }
  else if (m_conf == DirectionGnbUe_270)
    {
      uePositionAlloc->Add (Vector (0,-20,ueHeight));
    }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (gNbPositionAlloc);
  mobility.Install (gNbNodes);

  mobility.SetPositionAllocator (uePositionAlloc);
  mobility.Install (ueNodes);


  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  // Put the pointers inside nrHelper
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (CellScanBeamforming::GetTypeId ()));
  double beamSearchAngleStep = 30.0;
  idealBeamformingHelper->SetBeamformingAlgorithmAttribute ("BeamSearchAngleStep", DoubleValue (beamSearchAngleStep));
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);


  // set the number of antenna elements of UE
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (sqrt (m_ueNoOfAntennas)));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (sqrt (m_ueNoOfAntennas)));
  if (m_ueOmniAntennaElem)
    {
      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
    }
  else
    {
      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
    }
  // set the number of antenna elements of gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  if (m_gNbOmniAntennaElem)
    {
      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
    }
  else
    {
      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
    }

  // UE transmit power
  nrHelper->SetUePhyAttribute ("TxPower", DoubleValue (20.0));

  // gNB transmit power
  nrHelper->SetGnbPhyAttribute ("TxPower", DoubleValue (44.0));

  nrHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (3.0));


  nrHelper->SetEpcHelper (epcHelper);

  /*
   * Spectrum division. We create two operational bands, each of them containing
   * one component carrier, and each CC containing a single bandwidth part
   * centered at the frequency specified by the input parameters.
   * Each spectrum part length is, as well, specified by the input parameters.
   * Both operational bands will use the StreetCanyon channel modeling.
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  double centralFrequency = 28e9;
  double bandwidth = 20e6;
  const uint8_t numCcPerBand = 1;
  BandwidthPartInfo::Scenario scenario;
  // set LOS,NLOS condition
  if (m_losCondition == "l")
    {
      scenario = BandwidthPartInfo::UMi_StreetCanyon_LoS;
    }
  else if (m_losCondition == "n")
    {
      scenario = BandwidthPartInfo::UMi_StreetCanyon_nLoS;
    }
  else
    {
      scenario = BandwidthPartInfo::UMi_StreetCanyon;
    }
  CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequency, bandwidth, numCcPerBand, scenario);

  // By using the configuration created, it is time to make the operation bands
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

  // Shadowing
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));

  /*
   * Initialize channel and pathloss, plus other things inside band1. If needed,
   * the band configuration can be done manually, but we leave it for more
   * sophisticated examples. For the moment, this method will take care
   * of all the spectrum initialization needs.
   */
  nrHelper->InitializeOperationBand (&band);
  allBwps = CcBwpCreator::GetAllBwps ({band});

//  nrHelper->Initialize();

  uint32_t bwpIdForLowLat = 0;
  // gNb routing between Bearer and bandwidh part
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  // UE routing between Bearer and bandwidh part
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));

  // install nr net devices
  NetDeviceContainer gNbDevs = nrHelper->InstallGnbDevice (gNbNodes, allBwps);
  NetDeviceContainer ueNetDevs = nrHelper->InstallUeDevice (ueNodes, allBwps);


  for (auto it = gNbDevs.Begin (); it != gNbDevs.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDevs.Begin (); it != ueNetDevs.End (); ++it)
    {
      DynamicCast<NrUeNetDevice> (*it)->UpdateConfig ();
    }


  // create the internet and install the IP stack on the UEs
  // get SGW/PGW and create a single RemoteHost
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
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // in this container, interface 0 is the pgw, 1 is the remoteHost
  //Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDevs));


  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // attach UEs to the closest eNB
  nrHelper->AttachToClosestEnb (ueNetDevs, gNbDevs);

  // assign IP address to UEs, and install UDP downlink applications
  uint16_t dlPort = 1234;
  ApplicationContainer clientAppsDl;
  ApplicationContainer serverAppsDl;

  Time udpInterval = Time::FromDouble ((packetSize * 8) / static_cast<double> (udpRate.GetBitRate ()), Time::S);


  UdpServerHelper dlPacketSinkHelper (dlPort);
  serverAppsDl.Add (dlPacketSinkHelper.Install (ueNodes));


  UdpClientHelper dlClient (ueIpIface.GetAddress (0), dlPort);
  dlClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
  dlClient.SetAttribute ("Interval", TimeValue (udpInterval));
  dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  clientAppsDl.Add (dlClient.Install (remoteHost));

  Ptr<EpcTft> tft = Create<EpcTft> ();
  EpcTft::PacketFilter dlpf;
  dlpf.localPortStart = dlPort;
  dlpf.localPortEnd = dlPort;
  tft->Add (dlpf);

  EpsBearer bearer (EpsBearer::NGBR_LOW_LAT_EMBB);
  nrHelper->ActivateDedicatedEpsBearer (ueNetDevs.Get (0), bearer, tft);


  // start UDP server and client apps
  serverAppsDl.Start (udpAppStartTimeDl);
  clientAppsDl.Start (udpAppStartTimeDl);

  serverAppsDl.Stop (udpAppStopTimeDl);
  clientAppsDl.Stop (udpAppStopTimeDl);

  Ptr<NrSpectrumPhy > ue1SpectrumPhy = nrHelper->GetUePhy (ueNetDevs.Get (0), 0)->GetSpectrumPhy ();
  ue1SpectrumPhy->TraceConnectWithoutContext ("RxPacketTraceUe", MakeBoundCallback (&UETraceReception, this));

  //nrHelper->EnableTraces();
  Simulator::Stop (simTime);
  Simulator::Run ();

  std::cout << serverAppsDl.GetN () << std::endl;
  Ptr<UdpServer> serverApp1 = serverAppsDl.Get (0)->GetObject<UdpServer> ();
//  double throughput1 = (serverApp1->GetReceived () * packetSize * 8)/(udpAppStopTimeDl-udpAppStartTimeDl).GetSeconds ();
  double throughput1 = (serverApp1->GetReceived () * (packetSize + 28) * 8) / (udpAppStopTimeDl - udpAppStartTimeDl).GetSeconds ();

  std::cout << "\n UE:  " << throughput1 / 1e6 << " Mbps" <<
    "\t Avg.SINR:" << 10 * log10 (m_sinrCell1->getMean ()) <<
    "\t Avg.MCS:" << m_mcsCell1->getMean () << "\t Avg. RB Num:" <<
    m_rbNumCell1->getMean ();


  Simulator::Destroy ();
}


// The TestSuite class names the TestNrSystemTestOfdmaTestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined
//
class Antenna3gppModelConfTestSuite : public TestSuite
{
public:
  Antenna3gppModelConfTestSuite ();
};

Antenna3gppModelConfTestSuite::Antenna3gppModelConfTestSuite ()
  : TestSuite ("nr-antenna-3gpp-model-conf", SYSTEM)
{


  std::list<TestAntenna3gppModelConf::DirectionGnbUeXYAngle> conf = { TestAntenna3gppModelConf::DirectionGnbUe_45,
                                                                      TestAntenna3gppModelConf::DirectionGnbUe_135,
                                                                      TestAntenna3gppModelConf::DirectionGnbUe_225,
                                                                      TestAntenna3gppModelConf::DirectionGnbUe_315,
                                                                      TestAntenna3gppModelConf::DirectionGnbUe_0,
                                                                      TestAntenna3gppModelConf::DirectionGnbUe_90,
                                                                      TestAntenna3gppModelConf::DirectionGnbUe_180,
                                                                      TestAntenna3gppModelConf::DirectionGnbUe_270};

  std::list<uint8_t> ueNoOfAntennas = {16};

  std::list<std::string> losConditions = {"l"};

//  std::list<TypeId> gNbantennaArrayModelTypes = {AntennaArrayModel::GetTypeId(), AntennaArray3gppModel::GetTypeId ()};
  std::list<bool> gNbOmniAntennaElement = {false, true};

//  std::list<TypeId> ueAntennaArrayModelTypes = {AntennaArrayModel::GetTypeId(), AntennaArray3gppModel::GetTypeId ()};
  std::list<bool> ueOmniAntennaElement = {false, true};

  for (const auto & losCondition : losConditions)
    {
      for (const auto & c : conf)
        {
          for (const auto & oaaGnb : gNbOmniAntennaElement)
            {
              for (const auto & oaaUe : ueOmniAntennaElement)
                {
                  for (const auto & n : ueNoOfAntennas)
                    {
                      std::stringstream ss;
                      ss << " Test: ";

                      if (c == TestAntenna3gppModelConf::DirectionGnbUe_45)
                        {
                          ss << "DirectionGnbUe_45";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_135)
                        {
                          ss << "DirectionGnbUe_135";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_225)
                        {
                          ss << "DirectionGnbUe_225";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_315)
                        {
                          ss << "DirectionGnbUe_315";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_0)
                        {
                          ss << "DirectionGnbUe_0";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_90)
                        {
                          ss << "DirectionGnbUe_90";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_180)
                        {
                          ss << "DirectionGnbUe_180";
                        }
                      else if (c == TestAntenna3gppModelConf::DirectionGnbUe_270)
                        {
                          ss << "DirectionGnbUe_270";
                        }

                      ss << " , channelCondition: " << losCondition;

                      ss << " , UE number of antennas:" << (unsigned)n;

                      if (oaaGnb == true)
                        {
                          ss << " , gNB antenna element type: omni";
                        }
                      else
                        {
                          ss << " , gNB antenna element type: 3gpp";
                        }

                      if (oaaUe == true)
                        {
                          ss << " , UE antenna element type: omni";
                        }
                      else
                        {
                          ss << " , UE antenna element type: 3gpp";
                        }

                      AddTestCase (new TestAntenna3gppModelConf (ss.str (), c, oaaGnb, oaaUe, n, losCondition), TestDuration::QUICK);
                    }
                }
            }
        }
    }

}

// Do not forget to allocate an instance of this TestSuite
static Antenna3gppModelConfTestSuite testSuite;


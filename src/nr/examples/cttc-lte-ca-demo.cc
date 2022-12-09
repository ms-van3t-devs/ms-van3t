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
/**
 * \ingroup examples
 * \file cttc-lte-ca-demo.cc
 * \brief Example for setting LTE CA scenario
 *
 * This example describes how to setup a simulation using the 3GPP channel model
 * from TR 38.900. This example consists of 1 gNb and 1 UE. Have a look at the
 * possible parameters to know what you can configure through the command line.
 *
 * The example allows 2 configurations:
 *
 * An exclusivley TDD scenario, with 2 bands including 1 and 2 CCs, respectively.
 * Each CC includes 1 BWP. In this case 3 flows are created, 2 DL and 1 UL.
 *
 * A mixed TDD/FDD scenario with 2 bands including 1 and 2 CCs respectively. The
 * 1st and the 2nd CC include 1 TDD BWP each, while the 3rd CC is set to FDD
 * operation mode, thus it includes 2 BWPs (one for DL and 1 for UL). In this
 * case 4 flows are created, 2 DL and 2 UL.
 *
 * The example will print on-screen the end-to-end result of one (or two) flows,
 * as well as writing them on a file.
 *
 * \code{.unparsed}
$ ./ns3 run "cttc-lte-ca-demo --PrintHelp"
    \endcode
 *
 */


#include "ns3/core-module.h"
#include "ns3/config-store.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/log.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/bandwidth-part-gnb.h"
#include "ns3/nr-module.h"
#include "ns3/antenna-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("3gppChannelFdmLteComponentCarriersExample");


int
main (int argc, char *argv[])
{
  uint16_t gNbNum = 1;
  uint16_t ueNumPergNb = 1;

  uint8_t numBands = 2;
  double centralFrequencyBand40 = 2350e6;
  double bandwidthBand40 = 50e6;
  double centralFrequencyBand38 = 2595e6;
  double bandwidthBand38 = 100e6;

  double bandwidth = 20e6;

  uint16_t numerologyBwp0 = 0;
  uint16_t numerologyBwp1 = 0;
  uint16_t numerologyBwp2 = 0;
  uint16_t numerologyBwpDl = 0;
  uint16_t numerologyBwpUl = 0;

  double totalTxPower = 13;
  std::string pattern = "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"; // Pattern can be e.g. "DL|S|UL|UL|DL|DL|S|UL|UL|DL|"
  std::string patternDL = "DL|DL|DL|DL|DL|DL|DL|DL|DL|DL|";
  std::string patternUL = "UL|UL|UL|UL|UL|UL|UL|UL|UL|UL|";
  std::string operationMode = "TDD";  // TDD or FDD (mixed TDD and FDD mode)

  bool cellScan = false;
  double beamSearchAngleStep = 10.0;

  uint32_t udpPacketSizeUll = 915;
  uint32_t udpPacketSizeBe = 915;
  uint32_t lambdaUll = 10000;
  uint32_t lambdaBe = 10000;

  bool enableLowLat = true;
  bool enableVideo = true;
  bool enableVoice = true;
  bool enableGaming = false;  //If FDD is selected is set automaticaly to true

  bool logging = false;

  std::string simTag = "default";
  std::string outputDir = "./";

  double simTime = 1.4; // seconds
  double udpAppStartTime = 0.4; //seconds



  CommandLine cmd;

  cmd.AddValue ("simTime", "Simulation time", simTime);
  cmd.AddValue ("gNbNum",
                "The number of gNbs in multiple-ue topology",
                gNbNum);
  cmd.AddValue ("ueNumPergNb",
                "The number of UE per gNb in multiple-ue topology",
                ueNumPergNb);
  cmd.AddValue ("numBands",
                "Number of operation bands. More than one implies non-contiguous CC",
                numBands);
  cmd.AddValue ("bandwidthBand40",
                "The system bandwidth to be used in band 1",
                bandwidthBand40);
  cmd.AddValue ("bandwidthBand38",
                "The system bandwidth to be used in band 2",
                bandwidthBand38);
  cmd.AddValue ("bandwidth",
                "The bandwidth of the CCs ",
                bandwidth);
  cmd.AddValue ("numerologyBwp0",
                "The numerology to be used in bandwidth part 1",
                numerologyBwp0);
  cmd.AddValue ("numerologyBwp1",
                "The numerology to be used in bandwidth part 1",
                numerologyBwp1);
  cmd.AddValue ("numerologyBwp2",
                "The numerology to be used in bandwidth part 2",
                numerologyBwp2);
  cmd.AddValue ("numerologyBwpDl",
                "The numerology to be used in bandwidth part 2",
                numerologyBwpDl);
  cmd.AddValue ("numerologyBwpUl",
                "The numerology to be used in bandwidth part 2",
                numerologyBwpUl);
  cmd.AddValue ("totalTxPower",
                "total tx power that will be proportionally assigned to"
                " bandwidth parts depending on each BWP bandwidth ",
                totalTxPower);
  cmd.AddValue ("tddPattern",
                "LTE TDD pattern to use (e.g. --tddPattern=DL|S|UL|UL|UL|DL|S|UL|UL|UL|)",
                pattern);
  cmd.AddValue ("operationMode",
                "The network operation mode can be TDD or FDD (In this case it"
                "will be mixed TDD and FDD)",
                operationMode);
  cmd.AddValue ("cellScan",
                "Use beam search method to determine beamforming vector,"
                "true to use cell scanning method",
                cellScan);
  cmd.AddValue ("beamSearchAngleStep",
                "Beam search angle step for beam search method",
                beamSearchAngleStep);
  cmd.AddValue ("packetSizeUll",
                "packet size in bytes to be used by ultra low latency traffic",
                udpPacketSizeUll);
  cmd.AddValue ("packetSizeBe",
                "packet size in bytes to be used by best effort traffic",
                udpPacketSizeBe);
  cmd.AddValue ("lambdaUll",
                "Number of UDP packets in one second for ultra low latency traffic",
                lambdaUll);
  cmd.AddValue ("lambdaBe",
                "Number of UDP packets in one second for best effor traffic",
                lambdaBe);
  cmd.AddValue ("enableLowLat",
                "If true, enables low latency traffic transmission (DL)",
                enableLowLat);
  cmd.AddValue ("enableVideo",
                "If true, enables video traffic transmission (DL)",
                enableVideo);
  cmd.AddValue ("enableVoice",
                "If true, enables voice traffic transmission (UL)",
                enableVoice);
  cmd.AddValue ("enableGaming",
                "If true, enables gaming traffic transmission (UL)",
                enableGaming);
  cmd.AddValue ("logging",
                "Enable logging",
                logging);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);
  cmd.AddValue ("outputDir",
                "directory where to store simulation results",
                outputDir);

  cmd.Parse (argc, argv);

  NS_ABORT_IF (numBands < 1);
  NS_ABORT_MSG_IF (enableLowLat == false && enableVideo == false && enableVoice == false
                   && enableGaming == false && operationMode == "TDD", "For TDD enable one of the flows");

  //ConfigStore inputConfig;
  //inputConfig.ConfigureDefaults ();

  // enable logging or not
  if (logging)
    {
//      LogComponentEnable ("Nr3gppPropagationLossModel", LOG_LEVEL_ALL);
//      LogComponentEnable ("Nr3gppBuildingsPropagationLossModel", LOG_LEVEL_ALL);
//      LogComponentEnable ("Nr3gppChannel", LOG_LEVEL_ALL);
//      LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
//      LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
//      LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);
//      LogComponentEnable ("BwpManagerGnb", LOG_LEVEL_INFO);
//      LogComponentEnable ("BwpManagerAlgorithm", LOG_LEVEL_INFO);
      LogComponentEnable ("NrGnbPhy", LOG_LEVEL_INFO);
      LogComponentEnable ("NrUePhy", LOG_LEVEL_INFO);
//      LogComponentEnable ("NrGnbMac", LOG_LEVEL_INFO);
//      LogComponentEnable ("NrUeMac", LOG_LEVEL_INFO);
    }

  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  double gNbHeight = 10;
  double ueHeight = 1.5;

  gNbNodes.Create (gNbNum);
  ueNodes.Create (ueNumPergNb * gNbNum);

  Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> staPositionAlloc = CreateObject<ListPositionAllocator> ();
  int32_t yValue = 0.0;

  for (uint32_t i = 1; i <= gNbNodes.GetN (); ++i)
    {
      // 2.0, -2.0, 6.0, -6.0, 10.0, -10.0, ....
      if (i % 2 != 0)
        {
          yValue = static_cast<int> (i) * 30;
        }
      else
        {
          yValue = -yValue;
        }

      apPositionAlloc->Add (Vector (0.0, yValue, gNbHeight));


      // 1.0, -1.0, 3.0, -3.0, 5.0, -5.0, ...
      double xValue = 0.0;
      for (uint32_t j = 1; j <= ueNumPergNb; ++j)
        {
          if (j % 2 != 0)
            {
              xValue = j;
            }
          else
            {
              xValue = -xValue;
            }

          if (yValue > 0)
            {
              staPositionAlloc->Add (Vector (xValue, 10, ueHeight));
            }
          else
            {
              staPositionAlloc->Add (Vector (xValue, -10, ueHeight));
            }
        }
    }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (apPositionAlloc);
  mobility.Install (gNbNodes);

  mobility.SetPositionAllocator (staPositionAlloc);
  mobility.Install (ueNodes);


  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();

  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));
  epcHelper->SetAttribute ("S1uLinkDelay", TimeValue (MilliSeconds (0)));
  if (cellScan)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (CellScanBeamforming::GetTypeId ()));
      idealBeamformingHelper->SetBeamformingAlgorithmAttribute ("BeamSearchAngleStep", DoubleValue (beamSearchAngleStep));
    }
  else
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (QuasiOmniDirectPathBeamforming::GetTypeId ()));
    }
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (999999999));

  std::string errorModel = "ns3::NrLteMiErrorModel";
  // Scheduler
  nrHelper->SetSchedulerAttribute ("FixedMcsDl", BooleanValue (false));
  nrHelper->SetSchedulerAttribute ("FixedMcsUl", BooleanValue (false));

  // Error Model: UE and GNB with same spectrum error model.
  nrHelper->SetUlErrorModel (errorModel);
  nrHelper->SetDlErrorModel (errorModel);

  // Both DL and UL AMC will have the same model behind.
  nrHelper->SetGnbDlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel
  nrHelper->SetGnbUlAmcAttribute ("AmcModel", EnumValue (NrAmc::ErrorModel)); // NrAmc::ShannonModel or NrAmc::ErrorModel

  /*
   * Adjust the average number of Reference symbols per RB only for LTE case,
   * which is larger than in NR. We assume a value of 4 (could be 3 too).
   */
  nrHelper->SetGnbDlAmcAttribute ("NumRefScPerRb", UintegerValue (2));
  nrHelper->SetGnbUlAmcAttribute ("NumRefScPerRb", UintegerValue (2));
  nrHelper->SetGnbMacAttribute ("NumRbPerRbg", UintegerValue (4));
  nrHelper->SetSchedulerAttribute ("DlCtrlSymbols", UintegerValue (1));
  nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerOfdmaPF"));

  /*
   * Setup the operation bands.
   * In this example, two standard operation bands are deployed:
   *
   * Band 38 that has a component carrier (CC) of 20 MHz
   * Band 40 that has two non-contiguous CCs of 20 MHz each.
   *
   * If TDD mode is defined, 1 BWP per CC is created. All BWPs are TDD.
   * If FDD mode is defined, Band 40 CC2 containes 2 BWPs (1 DL - 1 UL), while
   * Band 40 CC1 has one TDD BWP and Band 38 CC0 also has one TDD BWP.
   *
   * This example manually creates a non-contiguous CC configuration with 2 CCs.
   * First CC has two BWPs and the second only one.
   *
   * The configured spectrum division for TDD mode is:
   * |--------- Band 40 --------|   |--------------- Band 38 ---------------|
   * |---------- CC0 -----------|   |-------- CC1-------||------- CC2-------|
   * |---------- BWP0 ----------|   |------- BWP1 ------||------ BWP2 ------|
   *
   * The configured spectrum division for FDD mode is:
   * |-------- Band 40 ---------|   |----------------- Band 38 ----------------|
   * |---------- CC0 -----------|   |------- CC1-------|  |-------- CC2--------|
   * |---------- BWP0 ----------|   |------ BWP1 ------|  |- BWP2DL -|- BWP2UL-|
   *
   *
   * In this example, each UE generates as many flows as the number of bwps
   * (i.e. 3 flows in case of TDD mode and 4 in case mixed TDD with FDD).
   * Each flow will be transmitted on a dedicated BWP. In particular, low
   * latency flow is set as DL and goes through BWP0, voice is set as UL and
   * goes through BWP1, video is set as DL and goes through BWP2DL, and gaming
   * is enabled only in the mixed TDD/FDD mode, it is set as UL and goes
   * through BWP2UL.
   */
  uint8_t numCcs = 3;

  BandwidthPartInfoPtrVector allBwps;

  // Create the configuration for band40 (CC0 - BWP0)
  OperationBandInfo band40;
  band40.m_centralFrequency  = centralFrequencyBand40;
  band40.m_channelBandwidth = bandwidthBand40;
  band40.m_lowerFrequency = band40.m_centralFrequency - band40.m_channelBandwidth / 2;
  band40.m_higherFrequency = band40.m_centralFrequency + band40.m_channelBandwidth / 2;

  // Component Carrier 0
  std::unique_ptr<ComponentCarrierInfo> cc0 (new ComponentCarrierInfo ());
  cc0->m_ccId = 0;
  cc0->m_centralFrequency = band40.m_lowerFrequency + bandwidth;
  cc0->m_channelBandwidth = bandwidth;
  cc0->m_lowerFrequency = cc0->m_centralFrequency - cc0->m_channelBandwidth / 2;
  cc0->m_higherFrequency = cc0->m_centralFrequency + cc0->m_channelBandwidth / 2;

  // BWP 0
  std::unique_ptr<BandwidthPartInfo> bwp0 (new BandwidthPartInfo ());
  bwp0->m_bwpId = 0;
  bwp0->m_centralFrequency = cc0->m_centralFrequency;
  bwp0->m_channelBandwidth = cc0->m_channelBandwidth;
  bwp0->m_lowerFrequency = cc0->m_lowerFrequency;
  bwp0->m_higherFrequency = cc0->m_higherFrequency;

  cc0->AddBwp (std::move (bwp0));

  band40.AddCc (std::move (cc0));


  // Create the configuration for band38
  OperationBandInfo band38;
  band38.m_centralFrequency  = centralFrequencyBand38;
  band38.m_channelBandwidth = bandwidthBand38;
  band38.m_lowerFrequency = band38.m_centralFrequency - band38.m_channelBandwidth / 2;
  band38.m_higherFrequency = band38.m_centralFrequency + band38.m_channelBandwidth / 2;


  //(CC1 - BWP1)
  // Component Carrier 1
  std::unique_ptr<ComponentCarrierInfo> cc1 (new ComponentCarrierInfo ());
  cc1->m_ccId = 1;
  cc1->m_centralFrequency = band38.m_lowerFrequency + bandwidth;
  cc1->m_channelBandwidth = bandwidth;
  cc1->m_lowerFrequency = cc1->m_centralFrequency - cc1->m_channelBandwidth / 2;
  cc1->m_higherFrequency = cc1->m_centralFrequency + cc1->m_channelBandwidth / 2;

  // BWP 1
  std::unique_ptr<BandwidthPartInfo> bwp1 (new BandwidthPartInfo ());
  bwp1->m_bwpId = 1;
  bwp1->m_centralFrequency = cc1->m_centralFrequency;
  bwp1->m_channelBandwidth = cc1->m_channelBandwidth;
  bwp1->m_lowerFrequency = cc1->m_lowerFrequency;
  bwp1->m_higherFrequency = cc1->m_higherFrequency;

  cc1->AddBwp (std::move (bwp1));

  std::unique_ptr<ComponentCarrierInfo> cc2 (new ComponentCarrierInfo ());
  std::unique_ptr<BandwidthPartInfo> bwp2 (new BandwidthPartInfo ());
  std::unique_ptr<BandwidthPartInfo> bwpdl (new BandwidthPartInfo ());
  std::unique_ptr<BandwidthPartInfo> bwpul (new BandwidthPartInfo ());

  // Component Carrier 2
  cc2->m_ccId = 2;
  cc2->m_centralFrequency = band38.m_higherFrequency - bandwidth;
  cc2->m_channelBandwidth = bandwidth;
  cc2->m_lowerFrequency = cc2->m_centralFrequency - cc2->m_channelBandwidth / 2;
  cc2->m_higherFrequency = cc2->m_centralFrequency + cc2->m_channelBandwidth / 2;

  if (operationMode == "TDD") //(CC2 - BWP2)
    {
      bwp2->m_bwpId = 1;
      bwp2->m_centralFrequency = cc2->m_centralFrequency;
      bwp2->m_channelBandwidth = cc2->m_channelBandwidth;
      bwp2->m_lowerFrequency = cc2->m_lowerFrequency;
      bwp2->m_higherFrequency = cc2->m_higherFrequency;

      cc2->AddBwp (std::move (bwp2));
    }
  else  //FDD case  (CC2 - BWPdl & BWPul)
    {
      // BWP DL
      bwpdl->m_bwpId = 2;
      bwpdl->m_channelBandwidth = cc2->m_channelBandwidth / 2;
      bwpdl->m_lowerFrequency = cc2->m_lowerFrequency;
      bwpdl->m_higherFrequency = bwpdl->m_lowerFrequency + bwpdl->m_channelBandwidth;
      bwpdl->m_centralFrequency = bwpdl->m_lowerFrequency + bwpdl->m_channelBandwidth / 2;

      cc2->AddBwp (std::move (bwpdl));

      // BWP UL
      bwpul->m_bwpId = 3;
      bwpul->m_channelBandwidth = cc2->m_channelBandwidth / 2;
      bwpul->m_lowerFrequency = cc2->m_centralFrequency;
      bwpul->m_higherFrequency = cc2->m_higherFrequency;
      bwpul->m_centralFrequency = bwpul->m_lowerFrequency + bwpul->m_channelBandwidth / 2;

      cc2->AddBwp (std::move (bwpul));
    }

  band38.AddCc (std::move (cc1));
  band38.AddCc (std::move (cc2));

  nrHelper->InitializeOperationBand (&band40);
  nrHelper->InitializeOperationBand (&band38);

  allBwps = CcBwpCreator::GetAllBwps ({band38, band40});

  // Antennas for all the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Antennas for all the gNbs
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (2));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));


  //Assign each flow type to a BWP
  uint32_t bwpIdForLowLat = 0;
  uint32_t bwpIdForVoice = 1;
  uint32_t bwpIdForVideo = 2;
  uint32_t bwpIdForVideoGaming = 3;

  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (bwpIdForVideo));
  nrHelper->SetGnbBwpManagerAlgorithmAttribute ("NGBR_VOICE_VIDEO_GAMING", UintegerValue (bwpIdForVideoGaming));

  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_LOW_LAT_EMBB", UintegerValue (bwpIdForLowLat));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("GBR_CONV_VOICE", UintegerValue (bwpIdForVoice));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VIDEO_TCP_PREMIUM", UintegerValue (bwpIdForVideo));
  nrHelper->SetUeBwpManagerAlgorithmAttribute ("NGBR_VOICE_VIDEO_GAMING", UintegerValue (bwpIdForVideoGaming));


  // install nr net devices
  NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice (gNbNodes, allBwps);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, allBwps);

  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (enbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);

  // Share the total transmission power among CCs proportionally with the BW
  double x = pow (10, totalTxPower / 10);
  double totalBandwidth = numCcs * bandwidth;

  // Band40: CC0 - BWP0 & Band38: CC1 - BWP1
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Numerology", UintegerValue (numerologyBwp0));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("TxPower",
                                                            DoubleValue (10 * log10 ((band40.GetBwpAt (0, 0)->m_channelBandwidth / totalBandwidth) * x)));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("Pattern", StringValue (pattern));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 0)->SetAttribute ("RbOverhead", DoubleValue (0.1));


  nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Numerology", UintegerValue (numerologyBwp1));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("TxPower",
                                                            DoubleValue (10 * log10 ((band38.GetBwpAt (0, 0)->m_channelBandwidth / totalBandwidth) * x)));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("Pattern", StringValue (pattern));
  nrHelper->GetGnbPhy (enbNetDev.Get (0), 1)->SetAttribute ("RbOverhead", DoubleValue (0.1));


  //Band38: CC2 - BWP2
  if (operationMode == "TDD")
    {
      nrHelper->GetGnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("Numerology", UintegerValue (numerologyBwp2));
      nrHelper->GetGnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("TxPower",
                                                                DoubleValue (10 * log10 ((band38.GetBwpAt (1, 0)->m_channelBandwidth / totalBandwidth) * x)));
      nrHelper->GetGnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("Pattern", StringValue (pattern));
      nrHelper->GetGnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("RbOverhead", DoubleValue (0.1));
    }
  else  //FDD case
    {
      nrHelper->GetGnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("Numerology", UintegerValue (numerologyBwpDl));
      nrHelper->GetGnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("TxPower",
                                                                DoubleValue (10 * log10 ((band38.GetBwpAt (1, 0)->m_channelBandwidth / totalBandwidth) * x)));
      nrHelper->GetGnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("Pattern", StringValue (patternDL));
      nrHelper->GetGnbPhy (enbNetDev.Get (0), 2)->SetAttribute ("RbOverhead", DoubleValue (0.1));

      nrHelper->GetGnbPhy (enbNetDev.Get (0), 3)->SetAttribute ("Numerology", UintegerValue (numerologyBwpUl));
      nrHelper->GetGnbPhy (enbNetDev.Get (0), 3)->SetAttribute ("Pattern", StringValue (patternUL));
      nrHelper->GetGnbPhy (enbNetDev.Get (0), 3)->SetAttribute ("RbOverhead", DoubleValue (0.1));

      // Link the two FDD BWP:
      nrHelper->GetBwpManagerGnb (enbNetDev.Get (0))->SetOutputLink (3, 2);

      // Set the UE routing:
      for (uint32_t i = 0; i < ueNetDev.GetN (); i++)
        {
          nrHelper->GetBwpManagerUe (ueNetDev.Get (i))->SetOutputLink (2, 3);
        }

      //enable 4rth flow
      enableGaming = true;
    }


  // When all the configuration is done, explicitly call UpdateConfig ()
  for (auto it = enbNetDev.Begin (); it != enbNetDev.End (); ++it)
    {
      DynamicCast<NrGnbNetDevice> (*it)->UpdateConfig ();
    }

  for (auto it = ueNetDev.Begin (); it != ueNetDev.End (); ++it)
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
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // Set the default gateway for the UEs
  for (uint32_t j = 0; j < ueNodes.GetN (); ++j)
    {
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNodes.Get (j)->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

  // attach UEs to the closest eNB
  nrHelper->AttachToClosestEnb (ueNetDev, enbNetDev);


  // install UDP applications
  uint16_t dlPortLowLat = 1234;
  uint16_t ulPortVoice = 1235;
  uint16_t dlPortVideo = 1236;
  uint16_t ulPortGaming = 1237;

  ApplicationContainer serverApps;

  // The sink will always listen to the specified ports
  UdpServerHelper dlPacketSinkLowLat (dlPortLowLat);
  UdpServerHelper ulPacketSinkVoice (ulPortVoice);
  UdpServerHelper dlPacketSinkVideo (dlPortVideo);
  UdpServerHelper ulPacketSinkGaming (ulPortGaming);

  // The server, that is the application which is listening, is installed in the UE
  // for the DL traffic, and in the remote host for the UL traffic
  serverApps.Add (dlPacketSinkLowLat.Install (ueNodes));
  serverApps.Add (ulPacketSinkVoice.Install (remoteHost));
  serverApps.Add (dlPacketSinkVideo.Install (ueNodes));
  serverApps.Add (ulPacketSinkGaming.Install (remoteHost));

  /*
   * Configure attributes for the different generators, using user-provided
   * parameters for generating a CBR traffic
   *
   * Low-Latency configuration and object creation:
   */
  UdpClientHelper dlClientLowLat;
  dlClientLowLat.SetAttribute ("RemotePort", UintegerValue (dlPortLowLat));
  dlClientLowLat.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  dlClientLowLat.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
  dlClientLowLat.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaBe)));

  // The bearer that will carry low latency traffic
  EpsBearer lowLatBearer (EpsBearer::NGBR_LOW_LAT_EMBB);

  // The filter for the low-latency traffic
  Ptr<EpcTft> lowLatTft = Create<EpcTft> ();
  EpcTft::PacketFilter dlpfLowLat;
  dlpfLowLat.localPortStart = dlPortLowLat;
  dlpfLowLat.localPortEnd = dlPortLowLat;
  lowLatTft->Add (dlpfLowLat);

  // Voice configuration and object creation:
  UdpClientHelper ulClientVoice;
  ulClientVoice.SetAttribute ("RemotePort", UintegerValue (ulPortVoice));
  ulClientVoice.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  ulClientVoice.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeBe));
  ulClientVoice.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaBe)));

  // The bearer that will carry voice traffic
  EpsBearer voiceBearer (EpsBearer::GBR_CONV_VOICE);

  // The filter for the voice traffic
  Ptr<EpcTft> voiceTft = Create<EpcTft> ();
  EpcTft::PacketFilter ulpfVoice;
  ulpfVoice.localPortStart = ulPortVoice;
  ulpfVoice.localPortEnd = ulPortVoice;
  ulpfVoice.direction = EpcTft::UPLINK;
  voiceTft->Add (ulpfVoice);

  //Video configuration and object creation:
  UdpClientHelper dlClientVideo;
  dlClientVideo.SetAttribute ("RemotePort", UintegerValue (dlPortVideo));
  dlClientVideo.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  dlClientVideo.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeUll));
  dlClientVideo.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaUll)));

  // The bearer that will carry video traffic
  EpsBearer videoBearer (EpsBearer::NGBR_VIDEO_TCP_PREMIUM);

  // The filter for the video traffic
  Ptr<EpcTft> videoTft = Create<EpcTft> ();
  EpcTft::PacketFilter dlpfVideo;
  dlpfVideo.localPortStart = dlPortVideo;
  dlpfVideo.localPortEnd = dlPortVideo;
  videoTft->Add (dlpfVideo);

  // Gaming configuration and object creation:
  UdpClientHelper ulClientGaming;
  ulClientGaming.SetAttribute ("RemotePort", UintegerValue (ulPortGaming));
  ulClientGaming.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
  ulClientGaming.SetAttribute ("PacketSize", UintegerValue (udpPacketSizeUll));
  ulClientGaming.SetAttribute ("Interval", TimeValue (Seconds (1.0 / lambdaUll)));

  // The bearer that will carry gaming traffic
  EpsBearer gamingBearer (EpsBearer::NGBR_VOICE_VIDEO_GAMING);

  // The filter for the gaming traffic
  Ptr<EpcTft> gamingTft = Create<EpcTft> ();
  EpcTft::PacketFilter ulpfGaming;
  ulpfGaming.remotePortStart = ulPortGaming;
  ulpfGaming.remotePortEnd = ulPortGaming;
  ulpfGaming.direction = EpcTft::UPLINK;
  gamingTft->Add (ulpfGaming);


  //  Install the applications
  ApplicationContainer clientApps;

  for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
    {
      Ptr<Node> ue = ueNodes.Get (i);
      Ptr<NetDevice> ueDevice = ueNetDev.Get (i);
      Address ueAddress = ueIpIface.GetAddress (i);

      // The client, who is transmitting, is installed in the remote host,
      // with destination address set to the address of the UE
      if (enableLowLat)
        {
          dlClientLowLat.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
          clientApps.Add (dlClientLowLat.Install (remoteHost));

          nrHelper->ActivateDedicatedEpsBearer (ueDevice, lowLatBearer, lowLatTft);
        }
      if (enableVideo)
        {
          dlClientVideo.SetAttribute ("RemoteAddress", AddressValue (ueAddress));
          clientApps.Add (dlClientVideo.Install (remoteHost));

          nrHelper->ActivateDedicatedEpsBearer (ueDevice, videoBearer, videoTft);
        }

      // For the uplink, the installation happens in the UE, and the remote address
      // is the one of the remote host

      if (enableVoice)
        {
          ulClientVoice.SetAttribute ("RemoteAddress", AddressValue (internetIpIfaces.GetAddress (1)));
          clientApps.Add (ulClientVoice.Install (ue));

          nrHelper->ActivateDedicatedEpsBearer (ueDevice, voiceBearer, voiceTft);
        }

      if (enableGaming)
        {
          ulClientGaming.SetAttribute ("RemoteAddress", AddressValue (internetIpIfaces.GetAddress (1)));
          clientApps.Add (ulClientGaming.Install (ue));

          nrHelper->ActivateDedicatedEpsBearer (ueDevice, gamingBearer, gamingTft);
        }
    }

  // start UDP server and client apps
  serverApps.Start (Seconds (udpAppStartTime));
  clientApps.Start (Seconds (udpAppStartTime));
  serverApps.Stop (Seconds (simTime));
  clientApps.Stop (Seconds (simTime));

  // enable the traces provided by the nr module
  nrHelper->EnableTraces ();


  FlowMonitorHelper flowmonHelper;
  NodeContainer endpointNodes;
  endpointNodes.Add (remoteHost);
  endpointNodes.Add (ueNodes);

  Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);
  monitor->SetAttribute ("DelayBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("JitterBinWidth", DoubleValue (0.001));
  monitor->SetAttribute ("PacketSizeBinWidth", DoubleValue (20));

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  /*
   * To check what was installed in the memory, i.e., BWPs of eNb Device, and its configuration.
   * Example is: Node 1 -> Device 0 -> BandwidthPartMap -> {0,1} BWPs -> NrGnbPhy -> NrPhyMacCommong-> Numerology, Bandwidth, ...
  GtkConfigStore config;
  config.ConfigureAttributes ();
  */

  // Print per-flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();

  double averageFlowThroughput = 0.0;
  double averageFlowDelay = 0.0;

  std::ofstream outFile;
  std::string filename = outputDir + "/" + simTag;
  outFile.open (filename.c_str (), std::ofstream::out | std::ofstream::trunc);
  if (!outFile.is_open ())
    {
      std::cerr << "Can't open file " << filename << std::endl;
      return 1;
    }

  outFile.setf (std::ios_base::fixed);

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      std::stringstream protoStream;
      protoStream << (uint16_t) t.protocol;
      if (t.protocol == 6)
        {
          protoStream.str ("TCP");
        }
      if (t.protocol == 17)
        {
          protoStream.str ("UDP");
        }
      outFile << "Flow " << i->first << " (" << t.sourceAddress << ":" << t.sourcePort << " -> " << t.destinationAddress << ":" << t.destinationPort << ") proto " << protoStream.str () << "\n";
      outFile << "  Tx Packets: " << i->second.txPackets << "\n";
      outFile << "  Tx Bytes:   " << i->second.txBytes << "\n";
      outFile << "  TxOffered:  " << i->second.txBytes * 8.0 / (simTime - udpAppStartTime) / 1000 / 1000  << " Mbps\n";
      outFile << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      if (i->second.rxPackets > 0)
        {
          // Measure the duration of the flow from receiver's perspective
          //double rxDuration = i->second.timeLastRxPacket.GetSeconds () - i->second.timeFirstTxPacket.GetSeconds ();
          double rxDuration = (simTime - udpAppStartTime);

          averageFlowThroughput += i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000;
          averageFlowDelay += 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets;

          outFile << "  Throughput: " << i->second.rxBytes * 8.0 / rxDuration / 1000 / 1000  << " Mbps\n";
          outFile << "  Mean delay:  " << 1000 * i->second.delaySum.GetSeconds () / i->second.rxPackets << " ms\n";
          //outFile << "  Mean upt:  " << i->second.uptSum / i->second.rxPackets / 1000/1000 << " Mbps \n";
          outFile << "  Mean jitter:  " << 1000 * i->second.jitterSum.GetSeconds () / i->second.rxPackets  << " ms\n";
        }
      else
        {
          outFile << "  Throughput:  0 Mbps\n";
          outFile << "  Mean delay:  0 ms\n";
          outFile << "  Mean jitter: 0 ms\n";
        }
      outFile << "  Rx Packets: " << i->second.rxPackets << "\n";
    }

  outFile << "\n\n  Aggregated throughput: " << averageFlowThroughput << "\n";
  outFile << "  Mean flow throughput: " << averageFlowThroughput / stats.size () << "\n";
  outFile << "  Mean flow delay: " << averageFlowDelay / stats.size () << "\n";

  outFile.close ();

  std::ifstream f (filename.c_str ());

  if (f.is_open ())
    {
      std::cout << f.rdbuf ();
    }

  Simulator::Destroy ();
  return 0;
}



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

/**
 * \file rem-beam-example.cc
 * \ingroup examples
 * \brief Rem beam configuration example
 *
 * This is a simple example which can be used to test different configurations of
 * gNB antenna array parameters and visualize its radiation through REM map.
 * Parameters that can be provided as input through the command line to configure
 * antenna array and its beamforming vector are:
 *  - numRowsGnb (number of rows of antenna array)
 *  - numColumnsGnb (number of columns of antenna array)
 *  - sector (sector with which will be created the beamforming vector, see
 *   CreateDirectionalBfv function)
 *  - theta (elevation that will be used to configure the beamforming vector.
 *
 * The rest of parameters are for REM map configuration, such as parameters for
 * resolution and REM area.
 *
 * ./ns3 run "rem-beam-example"
 *
 * The output of this example are REM csv files from which REM figures can be
 * generated with the following command:
 * \code{.unparsed}
 * $  gnuplot -p nr-rem-{simTag}-gnbs.txt nr-rem-{simTag}-ues.txt nr-rem-{simTag}-buildings.txt nr-rem-{simTag}-plot-rem.gnuplot
 *\endcode
 *
 * If no simTag is specified then to plot run the following command:
 *
 *\code{.unparsed}
 * gnuplot -p nr-rem--gnbs.txt nr-rem--ues.txt nr-rem--buildings.txt nr-rem--plot-rem.gnuplot
 * \endcode
 *
 * The following files will be generated (in the root project folder if not specified
 * differently): nr-rem--sinr.png, nr-rem--snr.png and nr-rem--ipsd.png
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store.h"
#include "ns3/nr-helper.h"
#include "ns3/log.h"
#include "ns3/nr-module.h"
#include "ns3/internet-module.h"
#include "ns3/antenna-module.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  //gnb antenna parameters
  uint32_t numRowsGnb = 1;
  uint32_t numColumnsGnb = 1;
  uint16_t sector = 0;
  double theta = 60;
  double simTime = 1;

  //Rem parameters
  double xMin = -1000.0;
  double xMax = 1000.0;
  uint16_t xRes = 100;
  double yMin = -1000.0;
  double yMax = 1000.0;
  uint16_t yRes = 100;
  std::string simTag = "";

  CommandLine cmd;
  cmd.AddValue ("simTag",
                "The simTag to be used for REM files creation",
                simTag);
  cmd.AddValue ("numRowsGnb",
                "Number of rows for the gNB antenna",
                numRowsGnb);
  cmd.AddValue ("numColumnsGnb",
                "Number of columns for the gNB antenna",
                numColumnsGnb);
  cmd.AddValue ("sector",
                "sector to be configured for",
                sector);
  cmd.AddValue ("theta",
                "thea angle to be configured",
                theta);
  cmd.AddValue ("xMin",
                "The min x coordinate of the rem map",
                xMin);
  cmd.AddValue ("xMax",
                "The max x coordinate of the rem map",
                xMax);
  cmd.AddValue ("xRes",
                "The resolution on the x axis of the rem map",
                xRes);
  cmd.AddValue ("yMin",
                "The min y coordinate of the rem map",
                yMin);
  cmd.AddValue ("yMax",
                "The max y coordinate of the rem map",
                yMax);
  cmd.AddValue ("yRes",
                "The resolution on the y axis of the rem map",
                yRes);

  cmd.Parse (argc, argv);

  // create gNB and UE
  NodeContainer gnbNodes;
  NodeContainer ueNodes;
  gnbNodes.Create (1);
  ueNodes.Create (1);

  // install mobility and initilize positions
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (gnbNodes);
  gnbNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (0, 0, 10));
  mobility.Install (ueNodes);
  ueNodes.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (10, 10, 0));

  // Create and configure helpers
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject <IdealBeamformingHelper> ();
  idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathQuasiOmniBeamforming::GetTypeId ()));
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  /*
   *  Create spectrum configuration: a single operational band with 1 CC and 1 BWP.
   *
   * |---------------Band---------------|
   * |---------------CC-----------------|
   * |---------------BWP----------------|
   */
  BandwidthPartInfoPtrVector singleBwp;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;

  CcBwpCreator::SimpleOperationBandConf bandConf (2e9, 20e6, numCcPerBand);
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

  //Initialize channel and pathloss, plus other things inside band.
  Config::SetDefault ("ns3::ThreeGppChannelModel::UpdatePeriod",TimeValue (MilliSeconds (0)));
  nrHelper->SetChannelConditionModelAttribute ("UpdatePeriod", TimeValue (MilliSeconds (0)));
  nrHelper->SetPathlossAttribute ("ShadowingEnabled", BooleanValue (false));
  nrHelper->InitializeOperationBand (&band);
  singleBwp = CcBwpCreator::GetAllBwps ({band});

  // Antennas for the UEs
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (1));
  nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));

  // Configuration of phy and antenna for the gNbs
  nrHelper->SetGnbPhyAttribute ("TxPower", DoubleValue (10));
  nrHelper->SetGnbPhyAttribute ("Numerology", UintegerValue (0));
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (numRowsGnb));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (numColumnsGnb));
  nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));

  // install nr net devices
  NetDeviceContainer gnbNetDev = nrHelper->InstallGnbDevice (gnbNodes, singleBwp);
  NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice (ueNodes, singleBwp);

  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (gnbNetDev, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDev, randomStream);

  // this is probably not necessary, since we did not update configuration after installation
  DynamicCast<NrGnbNetDevice> (gnbNetDev.Get (0))->UpdateConfig ();
  DynamicCast<NrUeNetDevice> (ueNetDev.Get (0))->UpdateConfig ();

  // install the IP stack on the UEs, this is needed to allow attachment
  InternetStackHelper internet;
  internet.Install (ueNodes);
  epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueNetDev));

  // we need to attach UEs to gNB so that they obtain the same configuration of channel as gNB
  nrHelper->AttachToEnb (ueNetDev.Get (0), gnbNetDev.Get (0));

  // configure REM parameters
  Ptr<NrRadioEnvironmentMapHelper> remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();
  remHelper->SetMinX (xMin);
  remHelper->SetMaxX (xMax);
  remHelper->SetResX (xRes);
  remHelper->SetMinY (yMin);
  remHelper->SetMaxY (yMax);
  remHelper->SetResY (yRes);
  remHelper->SetSimTag (simTag);
  remHelper->SetRemMode (NrRadioEnvironmentMapHelper::BEAM_SHAPE);

  //configure beam that will be shown in REM map
  DynamicCast<NrGnbNetDevice> (gnbNetDev.Get (0))->GetPhy (0)->GetSpectrumPhy(0)->GetBeamManager ()->SetSector (sector, theta);
  DynamicCast<NrUeNetDevice> (ueNetDev.Get (0))->GetPhy (0)->GetSpectrumPhy(0)->GetBeamManager ()->ChangeToQuasiOmniBeamformingVector ();
  remHelper->CreateRem (gnbNetDev, ueNetDev.Get (0), 0);

  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}




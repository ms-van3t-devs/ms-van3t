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
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/eps-bearer-tag.h"
#include "ns3/config-store-module.h"
#include "ns3/nr-module.h"
#include "ns3/antenna-module.h"

using namespace ns3;

/**
 * \ingroup examples
 * \file cttc-3gpp-indoor-calibration.cc
 * \brief Simulation script for the NR-MIMO Phase 1 system-level calibration
 *
 * The scenario implemented in the present simulation script is according to
 * the topology described in 3GPP TR 38.900 V15.0.0 (2018-06) Figure 7.2-1:
 * "Layout of indoor office scenarios".
 *
 * The simulation assumptions and the configuration parameters follow
 * the evaluation assumptions agreed at 3GPP TSG RAN WG1 meeting #88,
 * and which are summarised in R1-1703534 Table 1.
 * In the following Figure is illustrated the scenario with the gNB positions
 * which are represented with "x". The UE nodes are randomly uniformly dropped
 * in the area. There are 10 UEs per gNB.
 *
 * <pre>
 *   +----------------------120 m------------------ +
 *   |                                              |
 *   |                                              |
 *   |      x      x      x      x      x-20m-x     |
 *   |                                        |     |
 *   50m                                     20m    |
     |                                        |     |
 *   |      x      x      x      x      x     x     |
 *   |                                              |
 *   |                                              |
 *   +----------------------------------------------+
 * </pre>
 * The results of the simulation are files containing data that is being
 * collected over the course of the simulation execution:
 *
 * - SINR values for all the 120 UEs
 * - SNR values for all the 120 UEs
 * - RSSI values for all the 120 UEs
 *
 * Additionally there are files that contain:
 *
 * - UE positions
 * - gNB positions
 * - distances of UEs from the gNBs to which they are attached
 *
 * The file names are created by default in the root project directory if not
 * configured differently by setting resultsDirPath parameter of the Run()
 * function.
 *
 * The file names by default start with the prefixes such as "sinrs", "snrs",
 * "rssi", "gnb-positions,", "ue-positions" which are followed by the
 * string that briefly describes the configuration parameters that are being
 * set in the specific simulation execution.
 */


NS_LOG_COMPONENT_DEFINE ("Nr3gppIndoorCalibration");


/**
 * \brief Main class
 */
class Nr3gppIndoorCalibration
{

public:

  /**
   * \brief This function converts a linear SINR value that is encapsulated in
   * params structure to dBs, and then it prints the dB value to an output file
   * containing SINR values.
   * @param params RxPacketTraceParams structure that contains different
   * attributes that define the reception of the packet
   *
   */
  void UeReception (RxPacketTraceParams params);

  /**
   * \brief This function converts a linear SNR value to dBs and prints it to
   * the output file containing SNR values.
   * @param snr SNR value in linear units
   */
  void UeSnrPerProcessedChunk (double snr);

  /**
   * \brief This function prints out the RSSI value in dBm to file.
   * @param rssidBm RSSI value in dBm
   */
  void UeRssiPerProcessedChunk (double rssidBm);

  /**
   * Function that will actually configure all the simulation parameters,
   * topology and run the simulation by using the parameters that are being
   * configured for the specific run.
   *
   * @param centralFrequencyBand The band central frequency
   * @param bandwidthBand The band bandwidth
   * @param numerology The numerology
   * @param totalTxPower The gNB power
   * @param gNbAntennaModel antenna model to be used by gNB device, can be ISO
   * directional 3GPP
   * @param ueAntennaModel antenna model to be used by gNB device, can be ISO
   * directional 3GPP
   * @param indoorScenario defines the indoor scenario to be used in the simulation
   * campaign, currently the two different indoor scenarios are considered:
   * InH-OfficeOpen and InH-OfficeMixed
   * @param speed the speed of UEs in km/h
   * @param resultsDirPath results directory path
   * @param tag A tag that contains some simulation run specific values in order
   * to be able to distinguish the results file for different runs for different
   * parameters configuration
   * @param duration The duration of the simulation
   */
  void Run (double centralFrequencyBand, double bandwidthBand, uint16_t numerology,
            double totalTxPower, bool cellScan, double beamSearchAngleStep,
            bool gNbAntennaModel, bool ueAntennaModel, std::string indoorScenario,
            double speed, std::string resultsDirPath,
            std::string tag, uint32_t duration);
  /**
   * \brief Destructor that closes the output file stream and finished the
   * writing into the files.
   */
  ~Nr3gppIndoorCalibration ();

  /**
   * \brief Function selects UE nodes that are placed with a minimum
   * distance from its closest gNB.
   * \param ueNodes - container of UE nodes
   * \param gnbNodes - container of gNB nodes
   * \param min3DDistance - the minimum that shall be between UE and gNB
   * \param numberOfUesToBeSelected - the number of UE nodes to be selected
   * from the original container
   */
  NodeContainer SelectWellPlacedUes (const NodeContainer ueNodes, const NodeContainer gnbNodes,
                                     double min3DDistance, uint32_t numberOfUesToBeSelected);

private:

  std::ofstream m_outSinrFile;         //!< the output file stream for the SINR file
  std::ofstream m_outSnrFile;          //!< the output file stream for the SNR file
  std::ofstream m_outRssiFile;         //!< the output file stream for the RSSI file
  std::ofstream m_outUePositionsFile;  //!< the output file stream for the UE positions file
  std::ofstream m_outGnbPositionsFile; //!< the output file stream for the gNB positions file
  std::ofstream m_outDistancesFile;    //!< the output file stream for the distances file

};

/**
 * Function that creates the output file name for the results.
 * @param directoryName Directory name
 * @param filePrefix The prefix for the file name, e.g. sinr, snr,..
 * @param tag A tag that contains some simulation run specific values in order to be
 * able to distinguish the results file for different runs for different parameters
 * configuration
 * @return returns The full path file name string
 */
std::string
BuildFileNameString (std::string directoryName, std::string filePrefix, std::string tag)
{
  std::ostringstream oss;
  oss << directoryName << filePrefix << tag;
  return oss.str ();
}

/**
 * Creates a string tag that contains some simulation run specific values in
 * order to be able to distinguish the results files for different runs for
 * different parameters.
 * @param gNbAntennaModel gNb antenna model
 * @param ueAntennaModel UE antenna model
 * @param scenario The indoor scenario to be used
 * @param speed The speed of UEs in km/h
 * @return the parameter specific simulation name
 */
std::string
BuildTag (bool gNbAntennaModel, bool ueAntennaModel, std::string scenario,
          double speed)
{
  std::ostringstream oss;
  std::string ao;

  std::string gnbAm;
  if (gNbAntennaModel)
    {
      gnbAm = "ISO";
    }
  else
    {
      gnbAm = "3GPP";
    }

  std::string ueAm;
  if (ueAntennaModel)
    {
      ueAm = "ISO";
    }
  else
    {
      ueAm = "3GPP";
    }

  std::string gm = "";
  oss << "-ao" << ao << "-amGnb" << gnbAm << "-amUE" << ueAm <<
    "-sc" << scenario << "-sp" << speed << "-gm" << gm;

  return oss.str ();
}

/**
 * A callback function that redirects a call to the scenario instance.
 * @param scenario A pointer to a simulation instance
 * @param params RxPacketTraceParams structure containing RX parameters
 */
void UeReceptionTrace (Nr3gppIndoorCalibration* scenario, RxPacketTraceParams params)
{
  scenario->UeReception (params);
}

/**
 * A callback function that redirects a call to the scenario instance.
 * @param scenario A pointer to a simulation instance
 * @param snr SNR value
 */
void UeSnrPerProcessedChunkTrace (Nr3gppIndoorCalibration* scenario, double snr)
{
  scenario->UeSnrPerProcessedChunk (snr);
}

/**
 * A callback function that redirects a call to the scenario instance.
 * @param scenario A pointer to a simulation instance
 * @param rssidBm rssidBm RSSI value in dBm
 */
void UeRssiPerProcessedChunkTrace (Nr3gppIndoorCalibration* scenario, double rssidBm)
{
  scenario->UeRssiPerProcessedChunk (rssidBm);
}


void
Nr3gppIndoorCalibration::UeReception (RxPacketTraceParams params)
{
  m_outSinrFile << params.m_cellId << params.m_rnti <<
    "\t" << 10 * log10 (params.m_sinr) << std::endl;
}

void
Nr3gppIndoorCalibration::UeSnrPerProcessedChunk (double snr)
{
  m_outSnrFile << 10 * log10 (snr) << std::endl;
}


void
Nr3gppIndoorCalibration::UeRssiPerProcessedChunk (double rssidBm)
{
  m_outRssiFile << rssidBm << std::endl;
}

Nr3gppIndoorCalibration::~Nr3gppIndoorCalibration ()
{
  m_outSinrFile.close ();
  m_outSnrFile.close ();
  m_outRssiFile.close ();
}

NodeContainer
Nr3gppIndoorCalibration::SelectWellPlacedUes (const NodeContainer ueNodes,
                                              const NodeContainer gnbNodes,
                                              double minDistance,
                                              uint32_t numberOfUesToBeSelected)
{
  NodeContainer ueNodesFiltered;
  bool correctDistance = true;

  for (NodeContainer::Iterator itUe = ueNodes.Begin (); itUe != ueNodes.End (); itUe++)
    {
      correctDistance = true;
      Ptr<MobilityModel> ueMm = (*itUe)->GetObject<MobilityModel> ();
      Vector uePos = ueMm->GetPosition ();

      for (NodeContainer::Iterator itGnb = gnbNodes.Begin (); itGnb != gnbNodes.End (); itGnb++)
        {
          Ptr<MobilityModel> gnbMm = (*itGnb)->GetObject<MobilityModel> ();
          Vector gnbPos = gnbMm->GetPosition ();
          double x = uePos.x - gnbPos.x;
          double y = uePos.y - gnbPos.y;
          double distance = sqrt (x * x + y * y);

          if (distance < minDistance)
            {
              correctDistance = false;
              //NS_LOG("The UE node "<<(*itUe)->GetId() << " has wrong position, discarded.");
              break;
            }
          else
            {
              m_outDistancesFile << distance << std::endl;
            }
        }

      if (correctDistance)
        {
          ueNodesFiltered.Add (*itUe);
        }
      if (ueNodesFiltered.GetN () >= numberOfUesToBeSelected)
        {
          // there are enough candidate UE nodes
          break;
        }
    }
  return ueNodesFiltered;
}

void
Nr3gppIndoorCalibration::Run (double centralFrequencyBand, double bandwidthBand,
                              uint16_t numerology, double totalTxPower,
                              bool cellScan, double beamSearchAngleStep,
                              bool gNbAntennaModel, bool ueAntennaModel,
                              std::string indoorScenario, double speed,
                              std::string resultsDirPath, std::string tag,
                              uint32_t duration)
{
  Time simTime = MilliSeconds (duration);
  Time udpAppStartTimeDl = MilliSeconds (100);
  Time udpAppStopTimeDl = MilliSeconds (duration);
  uint32_t packetSize = 1000;
  DataRate udpRate = DataRate ("0.1kbps");
  // initially created 240 UE nodes, out of which will be selected 120 UEs that
  // are well placed respecting the minimum distance parameter that is configured
  uint16_t ueCount = 240;
  // the minimum distance parameter
  double minDistance = 0;
  // BS atnenna height is 3 meters
  double gNbHeight = 3;
  // UE antenna height is 1.5 meters
  double ueHeight = 1.5;

  BandwidthPartInfo::Scenario scenario;
  if (indoorScenario == "InH-OfficeMixed")
    {
      scenario =  BandwidthPartInfo::InH_OfficeMixed;
    }
  else if (indoorScenario == "InH-OfficeOpen")
    {
      scenario = BandwidthPartInfo::InH_OfficeOpen;
    }
  else
    {
      NS_ABORT_MSG ("Unsupported scenario");
    }

  // if simulation tag is not provided create one
  if (tag == "")
    {
      tag = BuildTag (gNbAntennaModel, ueAntennaModel, indoorScenario, speed);
    }
  std::string filenameSinr = BuildFileNameString ( resultsDirPath, "sinrs", tag);
  std::string filenameSnr = BuildFileNameString ( resultsDirPath, "snrs", tag);
  std::string filenameRssi = BuildFileNameString ( resultsDirPath, "rssi", tag);
  std::string filenameUePositions = BuildFileNameString ( resultsDirPath, "ue-positions", tag);
  std::string filenameGnbPositions = BuildFileNameString ( resultsDirPath, "gnb-positions", tag);
  std::string filenameDistances = BuildFileNameString ( resultsDirPath, "distances", tag);

  m_outSinrFile.open (filenameSinr.c_str ());
  m_outSinrFile.setf (std::ios_base::fixed);

  if (!m_outSinrFile.is_open ())
    {
      NS_ABORT_MSG ("Can't open file " << filenameSinr);
    }

  m_outSnrFile.open (filenameSnr.c_str ());
  m_outSnrFile.setf (std::ios_base::fixed);

  if (!m_outSnrFile.is_open ())
    {
      NS_ABORT_MSG ("Can't open file " << filenameSnr);
    }

  m_outRssiFile.open (filenameRssi.c_str ());
  m_outRssiFile.setf (std::ios_base::fixed);

  if (!m_outRssiFile.is_open ())
    {
      NS_ABORT_MSG ("Can't open file " << filenameRssi);
    }

  m_outUePositionsFile.open (filenameUePositions.c_str ());
  m_outUePositionsFile.setf (std::ios_base::fixed);

  if (!m_outUePositionsFile.is_open ())
    {
      NS_ABORT_MSG ("Can't open file " << filenameUePositions);
    }

  m_outGnbPositionsFile.open (filenameGnbPositions.c_str ());
  m_outGnbPositionsFile.setf (std::ios_base::fixed);

  if (!m_outGnbPositionsFile.is_open ())
    {
      NS_ABORT_MSG ("Can't open file " << filenameGnbPositions);
    }

  m_outDistancesFile.open (filenameDistances.c_str ());
  m_outDistancesFile.setf (std::ios_base::fixed);

  if (!m_outDistancesFile.is_open ())
    {
      NS_ABORT_MSG ("Can't open file " << filenameDistances);
    }


  // create base stations and mobile terminals
  NodeContainer gNbNodes;
  NodeContainer ueNodes;
  MobilityHelper mobility;

  gNbNodes.Create (12);
  ueNodes.Create (ueCount);

  // Creating positions of the gNB according to the 3gpp TR 38.900 Figure 7.2.-1
  Ptr<ListPositionAllocator> gNbPositionAlloc = CreateObject<ListPositionAllocator> ();

  for (uint8_t j = 0; j < 2; j++)
    {
      for (uint8_t i = 0; i < 6; i++)
        {
          gNbPositionAlloc->Add (Vector ( i * 20, j * 20, gNbHeight));
        }
    }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (gNbPositionAlloc);
  mobility.Install (gNbNodes);

  double minBigBoxX = -10.0;
  double minBigBoxY = -15.0;
  double maxBigBoxX = 110.0;
  double maxBigBoxY =  35.0;

  // Creating positions of the UEs according to the 3gpp TR 38.900 and
  // R11700144, uniformly randombly distributed in the rectangular area

  NodeContainer selectedUeNodes;
  for (uint8_t j = 0; j < 2; j++)
    {
      double minSmallBoxY = minBigBoxY + j * (maxBigBoxY - minBigBoxY) / 2;

      for (uint8_t i = 0; i < 6; i++)
        {
          double minSmallBoxX = minBigBoxX + i * (maxBigBoxX - minBigBoxX) / 6;
          Ptr<UniformRandomVariable> ueRandomVarX = CreateObject<UniformRandomVariable> ();

          double minX = minSmallBoxX;
          double maxX = minSmallBoxX + (maxBigBoxX - minBigBoxX) / 6 - 0.0001;
          double minY = minSmallBoxY;
          double maxY = minSmallBoxY + (maxBigBoxY - minBigBoxY) / 2 - 0.0001;

          Ptr<RandomBoxPositionAllocator> ueRandomRectPosAlloc = CreateObject<RandomBoxPositionAllocator> ();
          ueRandomVarX->SetAttribute ("Min", DoubleValue (minX));
          ueRandomVarX->SetAttribute ("Max", DoubleValue (maxX));
          ueRandomRectPosAlloc->SetX (ueRandomVarX);
          Ptr<UniformRandomVariable> ueRandomVarY = CreateObject<UniformRandomVariable> ();
          ueRandomVarY->SetAttribute ("Min", DoubleValue (minY));
          ueRandomVarY->SetAttribute ("Max", DoubleValue (maxY));
          ueRandomRectPosAlloc->SetY (ueRandomVarY);
          Ptr<ConstantRandomVariable> ueRandomVarZ = CreateObject<ConstantRandomVariable> ();
          ueRandomVarZ->SetAttribute ("Constant", DoubleValue (ueHeight));
          ueRandomRectPosAlloc->SetZ (ueRandomVarZ);

          uint8_t smallBoxIndex = j * 6 + i;

          NodeContainer smallBoxCandidateNodes;
          NodeContainer smallBoxGnbNode;

          smallBoxGnbNode.Add (gNbNodes.Get (smallBoxIndex));

          for (uint32_t n = smallBoxIndex * ueCount / 12; n < smallBoxIndex *
               static_cast <uint32_t> (ueCount / 12) +
               static_cast <uint32_t> (ueCount / 12); n++ )
            {
              smallBoxCandidateNodes.Add (ueNodes.Get (n));
            }
          mobility.SetPositionAllocator (ueRandomRectPosAlloc);
          mobility.Install (smallBoxCandidateNodes);
          NodeContainer sn = SelectWellPlacedUes (smallBoxCandidateNodes,
                                                  smallBoxGnbNode, minDistance, 10);
          selectedUeNodes.Add (sn);
        }
    }

  for (uint32_t j = 0; j < selectedUeNodes.GetN (); j++)
    {
      Vector v = selectedUeNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ();
      m_outUePositionsFile << j << "\t" << v.x << "\t" << v.y << "\t" << v.z << " " << std::endl;
    }

  for (uint32_t j = 0; j < gNbNodes.GetN (); j++)
    {
      Vector v = gNbNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ();
      m_outGnbPositionsFile << j << "\t" << v.x << "\t" << v.y << "\t" << v.z << " " << std::endl;
    }

  m_outUePositionsFile.close ();
  m_outGnbPositionsFile.close ();
  m_outDistancesFile.close ();

  // setup the nr simulation
  Ptr<NrHelper> nrHelper = CreateObject<NrHelper> ();
  Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper> ();
  Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper> ();

  nrHelper->SetBeamformingHelper (idealBeamformingHelper);
  nrHelper->SetEpcHelper (epcHelper);

  /*
   * Spectrum division. We create one operational band, containing one
   * component carrier, which in turn contains a single bandwidth part
   * centered at the frequency specified by the input parameters.
   *
   * The configured spectrum division is:
   * |------------------------Band-------------------------|
   * |-------------------------CC--------------------------|
   * |-------------------------BWP-------------------------|
   *
   * Each spectrum part length is, as well, specified by the input parameters.
   * The band will use the indoor channel modeling specified by scenario.
   */
  BandwidthPartInfoPtrVector allBwps;
  CcBwpCreator ccBwpCreator;
  const uint8_t numCcPerBand = 1;

  // Create the configuration for the CcBwpHelper. SimpleOperationBandConf creates
  // a single BWP per CC
  CcBwpCreator::SimpleOperationBandConf bandConf (centralFrequencyBand,
                                                  bandwidthBand,
                                                  numCcPerBand,
                                                  scenario);

  // By using the configuration created, make the operation band
  OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc (bandConf);

  nrHelper->InitializeOperationBand (&band);
  allBwps = CcBwpCreator::GetAllBwps ({band});


  // Disable channel matrix update to speed up the simulation execution
  //Config::SetDefault ("ns3::Nr3gppChannel::UpdatePeriod", TimeValue (MilliSeconds(0)));
  //Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));
  //Config::SetDefault ("ns3::LteRlcUmLowLat::MaxTxBufferSize", UintegerValue(999999999));
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));

  if (cellScan)
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (CellScanBeamforming::GetTypeId ()));
      idealBeamformingHelper->SetBeamformingAlgorithmAttribute ("BeamSearchAngleStep", DoubleValue (beamSearchAngleStep));
    }
  else
    {
      idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));
    }

  nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerTdmaPF"));

  // Antennas for all the UEs - Should be 2x4 = 8 antenna elements
  nrHelper->SetUeAntennaAttribute ("NumRows", UintegerValue (2));
  nrHelper->SetUeAntennaAttribute ("NumColumns", UintegerValue (4));
  //Antenna element type for UEs
  if (ueAntennaModel)
    {
      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
    }
  else
    {
      nrHelper->SetUeAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
    }
  // Antennas for all the gNbs - Should be 4x8 = 32 antenna elements
  nrHelper->SetGnbAntennaAttribute ("NumRows", UintegerValue (4));
  nrHelper->SetGnbAntennaAttribute ("NumColumns", UintegerValue (8));
  //Antenna element type for gNBs
  if (gNbAntennaModel)
    {
      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<IsotropicAntennaModel> ()));
    }
  else
    {
      nrHelper->SetGnbAntennaAttribute ("AntennaElement", PointerValue (CreateObject<ThreeGppAntennaModel> ()));
    }


  //mobility.SetPositionAllocator (ueRandomRectPosAlloc);
  //install nr net devices
  NetDeviceContainer gNbDevs = nrHelper->InstallGnbDevice (gNbNodes, allBwps);
  NetDeviceContainer ueNetDevs = nrHelper->InstallUeDevice (selectedUeNodes, allBwps);

  int64_t randomStream = 1;
  randomStream += nrHelper->AssignStreams (gNbDevs, randomStream);
  randomStream += nrHelper->AssignStreams (ueNetDevs, randomStream);

  for (uint32_t i = 0; i < gNbDevs.GetN (); i++)
    {
      nrHelper->GetGnbPhy (gNbDevs.Get (i), 0)->SetAttribute ("Numerology", UintegerValue (numerology));
      nrHelper->GetGnbPhy (gNbDevs.Get (i), 0)->SetAttribute ("TxPower", DoubleValue (10 * log10 (totalTxPower)));
      // gNB noise figure shall be set to 7 dB
      nrHelper->GetGnbPhy (gNbDevs.Get (i), 0)->SetAttribute ("NoiseFigure", DoubleValue (7));
    }
  for (uint32_t j = 0; j < ueNetDevs.GetN (); j++)
    {
      // UE noise figure shall be set to 10 dB
      nrHelper->GetUePhy (ueNetDevs.Get (j), 0)->SetAttribute ("NoiseFigure", DoubleValue (10));
    }


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

  // configure UDP downlink traffic
  for (uint32_t i = 0; i < ueNetDevs.GetN (); i++)
    {
      UdpClientHelper dlClient (ueIpIface.GetAddress (i), dlPort);
      dlClient.SetAttribute ("MaxPackets", UintegerValue (0xFFFFFFFF));
      dlClient.SetAttribute ("PacketSize", UintegerValue (packetSize));
      dlClient.SetAttribute ("Interval", TimeValue (udpInterval));   // we try to saturate, we just need to measure during a short time, how much traffic can handle each BWP
      clientAppsDl.Add (dlClient.Install (remoteHost));
    }

  // start UDP server and client apps
  serverAppsDl.Start (udpAppStartTimeDl);
  clientAppsDl.Start (udpAppStartTimeDl);

  serverAppsDl.Stop (udpAppStopTimeDl);
  clientAppsDl.Stop (udpAppStopTimeDl);

  for (uint32_t i = 0; i < ueNetDevs.GetN (); i++)
    {
      Ptr<NrSpectrumPhy > ue1SpectrumPhy = DynamicCast<NrUeNetDevice>
          (ueNetDevs.Get (i))->GetPhy (0)->GetSpectrumPhy ();
      ue1SpectrumPhy->TraceConnectWithoutContext ("RxPacketTraceUe", MakeBoundCallback (&UeReceptionTrace, this));
      Ptr<NrInterference> ue1SpectrumPhyInterference = ue1SpectrumPhy->GetNrInterference ();
      NS_ABORT_IF (!ue1SpectrumPhyInterference);
      ue1SpectrumPhyInterference->TraceConnectWithoutContext ("SnrPerProcessedChunk", MakeBoundCallback (&UeSnrPerProcessedChunkTrace, this));
      ue1SpectrumPhyInterference->TraceConnectWithoutContext ("RssiPerProcessedChunk", MakeBoundCallback (&UeRssiPerProcessedChunkTrace, this));
    }

  Simulator::Stop (simTime);
  Simulator::Run ();
  Simulator::Destroy ();
}

int
main (int argc, char *argv[])
{
  // Parameters according to R1-1703534 3GPP TSG RAN WG1 Meetging #88, 2017
  // Evaluation assumptions for Phase 1 NR MIMO system level calibration,
  double centralFrequencyBand = 30e9;
  double bandwidthBand = 40e6;
  uint16_t numerology = 2;
  double totalTxPower = 23;

  uint32_t duration = 150;
  bool cellScan = false;
  double beamSearchAngleStep = 10.0;
  bool enableGnbIso = true;
  bool enableUeIso = true;
  std::string indoorScenario = "InH-OfficeOpen";
  double speed = 3.00;
  std::string resultsDir = "./";
  std::string simTag = "";

  CommandLine cmd;

  cmd.AddValue ("duration",
                "Simulation duration in ms, should be greater than 100 ms to allow the collection of traces",
                duration);
  cmd.AddValue ("cellScan",
                "Use beam search method to determine beamforming vector,"
                "true to use cell scanning method",
                cellScan);
  cmd.AddValue ("beamSearchAngleStep",
                "Beam search angle step for beam search method",
                beamSearchAngleStep);
  cmd.AddValue ("enableGnbIso",
                "Enable Isotropic antenna for the gNB",
                enableGnbIso);
  cmd.AddValue ("enableGnbIso",
                "Enable Isotropic antenna for the UE",
                enableUeIso);
  cmd.AddValue ("indoorScenario",
                "The indoor scenario to be used can be: InH-OfficeMixed or InH-OfficeOpen",
                indoorScenario);
  cmd.AddValue ("speed",
                "UE speed in km/h",
                speed);
  cmd.AddValue ("resultsDir",
                "directory where to store the simulation results",
                resultsDir);
  cmd.AddValue ("simTag",
                "tag to be appended to output filenames to distinguish simulation campaigns",
                simTag);

  cmd.Parse (argc, argv);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  Nr3gppIndoorCalibration phase1CalibrationScenario;
  phase1CalibrationScenario.Run (centralFrequencyBand, bandwidthBand, numerology,
                                 totalTxPower, cellScan, beamSearchAngleStep,
                                 enableGnbIso, enableUeIso, indoorScenario,
                                 speed, resultsDir, simTag, duration);

  return 0;
}


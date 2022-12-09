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

#include "nr-radio-environment-map-helper.h"
#include <ns3/abort.h>
#include <ns3/log.h>
#include <ns3/double.h>
#include <ns3/integer.h>
#include <ns3/uinteger.h>
#include <ns3/string.h>
#include <ns3/boolean.h>
#include <ns3/pointer.h>
#include <ns3/config.h>
#include <ns3/simulator.h>
#include <ns3/node.h>
#include <ns3/mobility-model.h>
#include <ns3/spectrum-converter.h>
#include <ns3/buildings-module.h>
#include <ns3/nr-gnb-net-device.h>
#include <ns3/nr-ue-net-device.h>
#include <ns3/nr-spectrum-phy.h>
#include "nr-spectrum-value-helper.h"
#include <ns3/beamforming-vector.h>
#include <ctime>
#include <fstream>
#include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrRadioEnvironmentMapHelper");

NS_OBJECT_ENSURE_REGISTERED (NrRadioEnvironmentMapHelper);

NrRadioEnvironmentMapHelper::NrRadioEnvironmentMapHelper ()
{
  NS_LOG_FUNCTION (this);
}

NrRadioEnvironmentMapHelper::~NrRadioEnvironmentMapHelper ()
{

}

void
NrRadioEnvironmentMapHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
NrRadioEnvironmentMapHelper::GetTypeId (void)
{
  NS_LOG_FUNCTION ("NrRadioEnvironmentMapHelper::GetTypeId");
  static TypeId tid = TypeId ("ns3::NrRadioEnvironmentMapHelper")
                      .SetParent<Object> ()
                      .SetGroupName("Nr")
                      .AddConstructor<NrRadioEnvironmentMapHelper> ()
                      .AddAttribute ("SimTag",
                                     "simulation tag that will be concatenated to output file names"
                                     "in order to distinguish them, for example: nr-rem-${SimTag}.out. "
                                     "nr-rem-${SimTag}-ues.txt, nr-rem-${SimTag}-gnbs.txt, nr-rem-${SimTag}-buildings.txt.",
                                      StringValue (""),
                                      MakeStringAccessor (&NrRadioEnvironmentMapHelper::SetSimTag),
                                      MakeStringChecker ())
                      .AddAttribute ("XMin",
                                     "The min x coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMinX,
                                                         &NrRadioEnvironmentMapHelper::GetMinX),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("YMin",
                                     "The min y coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMinY,
                                                         &NrRadioEnvironmentMapHelper::GetMinY),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("XMax",
                                     "The max x coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMaxX,
                                                         &NrRadioEnvironmentMapHelper::GetMaxX),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("YMax",
                                     "The max y coordinate of the map.",
                                     DoubleValue (0.0),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetMaxY,
                                                         &NrRadioEnvironmentMapHelper::GetMaxY),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("XRes",
                                     "The resolution (number of points) of the"
                                     "map along the x axis.",
                                     UintegerValue (100),
                                     MakeUintegerAccessor (&NrRadioEnvironmentMapHelper::SetResX,
                                                           &NrRadioEnvironmentMapHelper::GetResX),
                                     MakeUintegerChecker<uint32_t> (2,std::numeric_limits<uint16_t>::max ()))
                      .AddAttribute ("YRes",
                                     "The resolution (number of points) of the"
                                     "map along the y axis.",
                                     UintegerValue (100),
                                     MakeUintegerAccessor (&NrRadioEnvironmentMapHelper::SetResY,
                                                           &NrRadioEnvironmentMapHelper::GetResY),
                                     MakeUintegerChecker<uint16_t> (2,std::numeric_limits<uint16_t>::max ()))
                      .AddAttribute ("Z",
                                     "The value of the z coordinate for which"
                                     "the map is to be generated.",
                                     DoubleValue (1.5),
                                     MakeDoubleAccessor (&NrRadioEnvironmentMapHelper::SetZ,
                                                         &NrRadioEnvironmentMapHelper::GetZ),
                                     MakeDoubleChecker<double> ())
                      .AddAttribute ("IterForAverage",
                                     "Number of iterations for the calculation"
                                     "of the average rem value.",
                                     UintegerValue (1),
                                     MakeUintegerAccessor (&NrRadioEnvironmentMapHelper::SetNumOfItToAverage),
                                                           //&NrRadioEnvironmentMapHelper::GetMaxPointsPerIt),
                                     MakeUintegerChecker<uint16_t> ())
                      .AddAttribute ("RemMode",
                                     "There are three high level modes of Rem generation: "
                                     "a) BEAM_SHAPE in which are represented the beams that are configured "
                                     "in the user's script scenario, considering that the receiver always "
                                     "has quasi-omni, and that all the beams point toward the UE which is "
                                     "passed as UE of interest. The purpose of this map is to illustrate "
                                     "the REM of the scenario that is configured."
                                     "b) COVERAGE_AREA which produces two REM maps: the worst-case SINR and "
                                     "best-SNR for each rem position; Worst case SINR means that all interfering "
                                     "devices use for the transmission the beam towards the rem point;"
                                     "and also for the best-SNR, for each transmitting device and the REM point "
                                     "are used the best directional beam-pair and then is selected the best SNR."
                                     "c) UE_COVERAGE which is similar as the above, although the Tx Device"
                                     "is the UE (UL direction), and the Rx device is each gNB to which it is "
                                     "connected each time, while the rest of gNBs (if they are present) are"
                                     "pointing their beams towards the Rx gNB. In case of TDD, the SINR map"
                                     "will show the interference caused by the DL of these gNBs.",
                                     EnumValue (NrRadioEnvironmentMapHelper::COVERAGE_AREA),
                                     MakeEnumAccessor (&NrRadioEnvironmentMapHelper::SetRemMode,
                                                       &NrRadioEnvironmentMapHelper::GetRemMode),
                                     MakeEnumChecker (NrRadioEnvironmentMapHelper::BEAM_SHAPE, "BeamShape",
                                                      NrRadioEnvironmentMapHelper::COVERAGE_AREA, "CoverageArea",
                                                      NrRadioEnvironmentMapHelper::UE_COVERAGE, "UeCoverageArea"))
                      .AddAttribute ("InstallationDelay",
                                     "How many time it is needed in the simulation to configure phy parameters at UE, "
                                     "depends on RRC message timing.",
                                     TimeValue (MilliSeconds (100)),
                                     MakeTimeAccessor (&NrRadioEnvironmentMapHelper::SetInstallationDelay),
                                     MakeTimeChecker())
    ;
  return tid;
}

void
NrRadioEnvironmentMapHelper::SetRemMode (enum RemMode remMode)
{
  m_remMode = remMode;
}

void
NrRadioEnvironmentMapHelper::SetSimTag (const std::string &simTag)
{
  m_simTag = simTag;
}

void
NrRadioEnvironmentMapHelper::SetMinX (double xMin)
{
  m_xMin = xMin;
}

void
NrRadioEnvironmentMapHelper::SetMinY (double yMin)
{
  m_yMin = yMin;
}

void
NrRadioEnvironmentMapHelper::SetMaxX (double xMax)
{
  m_xMax = xMax;
}

void
NrRadioEnvironmentMapHelper::SetMaxY (double yMax)
{
  m_yMax = yMax;
}

void
NrRadioEnvironmentMapHelper::SetResX (uint16_t xRes)
{
  m_xRes = xRes;
}

void
NrRadioEnvironmentMapHelper::SetResY (uint16_t yRes)
{
  m_yRes = yRes;
}

void
NrRadioEnvironmentMapHelper::SetZ (double z)
{
  m_z = z;
}

void
NrRadioEnvironmentMapHelper::SetNumOfItToAverage (uint16_t numOfIterationsToAverage)
{
  m_numOfIterationsToAverage = numOfIterationsToAverage;
}

void
NrRadioEnvironmentMapHelper::SetInstallationDelay (const Time &installationDelay)
{
  m_installationDelay = installationDelay;
}

NrRadioEnvironmentMapHelper::RemMode
NrRadioEnvironmentMapHelper::GetRemMode () const
{
  return m_remMode;
}

double
NrRadioEnvironmentMapHelper::GetMinX () const
{
  return m_xMin;
}

double
NrRadioEnvironmentMapHelper::GetMinY () const
{
  return m_yMin;
}

double
NrRadioEnvironmentMapHelper::GetMaxX () const
{
  return m_xMax;
}

double
NrRadioEnvironmentMapHelper::GetMaxY () const
{
  return m_yMax;
}

uint16_t
NrRadioEnvironmentMapHelper::GetResX () const
{
  return m_xRes;
}

uint16_t
NrRadioEnvironmentMapHelper::GetResY () const
{
  return m_yRes;
}

double
NrRadioEnvironmentMapHelper::GetZ () const
{
  return m_z;
}

double
NrRadioEnvironmentMapHelper::DbmToW (double dBm) const
{
  return std::pow (10.0, 0.1 * (dBm - 30.0));
}

double
NrRadioEnvironmentMapHelper::WToDbm (double w) const
{
  return 10.0 * std::log10 (w) + 30.0;
}

double
NrRadioEnvironmentMapHelper::DbToRatio (double dB) const
{
  return std::pow (10.0, 0.1 * dB);
}

double
NrRadioEnvironmentMapHelper::RatioToDb (double ratio) const
{
  return 10.0 * std::log10 (ratio);
}

void NrRadioEnvironmentMapHelper::ConfigureRrd (const Ptr<NetDevice> &rrdDevice)
{
  NS_LOG_FUNCTION (this);
  // RTD and RRD devices should have the same spectrum model to perform calculation,
  // if some of the RTD devices is of the different model then its transmission will have to
  // converted into spectrum model of this device
  m_rrd.spectrumModel = m_rrdPhy->GetSpectrumModel ();
  m_rrd.mob->SetPosition (rrdDevice->GetNode ()->GetObject<MobilityModel> ()->GetPosition ());

  std::ostringstream oss;
  oss << "nr-rem-" << m_simTag.c_str() << "-ues.txt";
  PrintGnuplottableUeListToFile (oss.str ());

  Ptr<MobilityBuildingInfo> buildingInfo = CreateObject<MobilityBuildingInfo> ();
  m_rrd.mob->AggregateObject (buildingInfo);

  m_rrd.antenna = m_deviceToAntenna.find (rrdDevice)->second;

  m_noisePsd = NrSpectrumValueHelper::CreateNoisePowerSpectralDensity (m_rrdPhy->GetNoiseFigure (), m_rrd.spectrumModel);

  ConfigurePropagationModelsFactories (m_rrdPhy); // we can call only once configuration of prop.models

}

void NrRadioEnvironmentMapHelper::ConfigureRtdList (const NetDeviceContainer& rtdDevs)
{
  NS_LOG_FUNCTION (this);

  for (NetDeviceContainer::Iterator netDevIt = rtdDevs.Begin ();
      netDevIt != rtdDevs.End ();
      ++netDevIt)
    {
      Ptr<NrPhy> rtdPhy = m_rtdDeviceToPhy.find (*netDevIt)->second;
      if (rtdPhy->GetSpectrumModel () != m_rrd.spectrumModel)
        {
          if (rtdPhy->GetSpectrumModel ()->IsOrthogonal (*m_rrd.spectrumModel))
            {
              NS_LOG_WARN ("RTD device is configured to operate on a spectrum "
                           "that is orthogonal to the one of RRD device. Hence, "
                           "that RTD device will not be considered in the "
                           "calculation of this REM map.");
              continue;
            }
          else
            {
              NS_LOG_WARN ("RTD device with different spectrum model, this may slow "
                           "down significantly the REM map creation. Consider setting "
                           "the same frequency, bandwidth, and numerology to all "
                           "devices which are used for REM map creation.");
            }
        };

      RemDevice rtd;
      //Configure spectrum model which will be needed to create tx PSD
      rtd.spectrumModel = rtdPhy->GetSpectrumModel ();
      rtd.mob->SetPosition ((*netDevIt)->GetNode ()->GetObject<MobilityModel> ()->GetPosition ());
      Ptr<MobilityBuildingInfo> buildingInfo = CreateObject<MobilityBuildingInfo> ();
      rtd.mob->AggregateObject (buildingInfo);

      rtd.antenna = m_deviceToAntenna.find (*netDevIt)->second;

      rtd.txPower = rtdPhy->GetTxPower ();

      NS_LOG_DEBUG ("power of UE: " << rtd.txPower);

      NS_LOG_INFO ("RTD spectrum model: " << rtd.spectrumModel->GetUid () <<
                   ", RTD number of bands: " << rtd.spectrumModel->GetNumBands () <<
                   ", create new RTD element... " <<
                   ", rtdPhy->GetCentralFrequency () " <<
                   rtdPhy->GetCentralFrequency () / 1e6 << " MHz " <<
                   //", BW: " << rtdPhy->GetChannelBandwidth () / 1e6 << " MHz " <<
                   ", num: "<< rtdPhy->GetNumerology ());

      m_remDev.push_back (rtd);
    }
  NS_ASSERT_MSG (m_remDev.size (),"No RTD devices configured. Check if the RTD "
                                  "devices are on the operating on the same "
                                  "spectrum as RRD device.");
}

void
NrRadioEnvironmentMapHelper::ConfigurePropagationModelsFactories (const Ptr<const NrPhy>& rtdPhy)
{
  NS_LOG_FUNCTION (this);
  Ptr<const NrSpectrumPhy> txSpectrumPhy = rtdPhy->GetSpectrumPhy ();
  Ptr<SpectrumChannel> txSpectrumChannel = txSpectrumPhy->GetSpectrumChannel ();

  /***** configure pathloss model factory *****/
  m_propagationLossModel = txSpectrumChannel->GetPropagationLossModel ();
  /***** configure spectrum model factory *****/
  m_phasedArraySpectrumLossModel = txSpectrumChannel->GetPhasedArraySpectrumPropagationLossModel ();

  /***** configure ChannelConditionModel factory if ThreeGppPropagationLossModel propagation model is being used ****/
  Ptr<ThreeGppPropagationLossModel> propagationLossModel =  DynamicCast<ThreeGppPropagationLossModel> (txSpectrumChannel->GetPropagationLossModel ());
  if (propagationLossModel)
    {
      Ptr<ChannelConditionModel> channelConditionModel = propagationLossModel->GetChannelConditionModel ();
      if (channelConditionModel)
        {
          m_channelConditionModelFactory = ConfigureObjectFactory (channelConditionModel);
        }
      else
        {
          NS_FATAL_ERROR ("ThreeGppPropagationLossModel does not have configured ChannelConditionModel");
        }
    }
  else
    {
      NS_LOG_WARN ("RemHelper currently only knows that ThreeGppPropagationLossModel can have ChannelConditionModel. Other models do not support it yet.");
    }

  /***** configure ThreeGppChannelModel (MatrixBasedChannelModel) factory if spectrumLossModel is ThreeGppSpectrumPropagationLossModel *****/
  Ptr<ThreeGppSpectrumPropagationLossModel> spectrumLossModel = DynamicCast<ThreeGppSpectrumPropagationLossModel> (txSpectrumChannel->GetPhasedArraySpectrumPropagationLossModel());
  if (spectrumLossModel)
    {
      if (spectrumLossModel->GetChannelModel ())
        {
          m_matrixBasedChannelModelFactory = ConfigureObjectFactory (spectrumLossModel->GetChannelModel ());
        }
      else
        {
          NS_FATAL_ERROR ("ThreeGppSpectrumPropagationLossModel does not have configured MatrixBasedChannelModel");
        }
    }
  else
    {
      NS_LOG_WARN ("RemHelper currently only knows that ThreeGppSpectrumPropagationLossModel can have MatrixBasedChannelModel. Other models do not support it yet.");
    }
}

ObjectFactory
NrRadioEnvironmentMapHelper::ConfigureObjectFactory (const Ptr<Object>& object) const
{
  NS_LOG_FUNCTION (this);
  ObjectFactory objectFactory;
  TypeId tid = object->GetInstanceTypeId ();
  objectFactory.SetTypeId (object->GetInstanceTypeId ());

  NS_LOG_DEBUG ("Configure object factory for:" << tid.GetName ());

  bool hasParent = false;
  do
    {
      for (size_t i = 0; i < tid.GetAttributeN (); i++)
        {
          ns3::TypeId::AttributeInformation attributeInfo = tid.GetAttribute (i);

          if (attributeInfo.checker->GetValueTypeName () == "ns3::PointerValue")
            {
              if (attributeInfo.name == "ChannelConditionModel")
                {
                  NS_LOG_INFO ("Skipping to copy ChannelConditionModel."
                               "According to REM design it should be created "
                               "as a new object (not copied).");
                }
              else if (attributeInfo.name == "ChannelModel")
                {
                  NS_LOG_INFO ("Skipping to copy ChannelModel."
                               "According to REM design it should be created "
                               "as a new object (not copied).");
                }
              else
                {
                  NS_LOG_WARN ("This factory has a PointerValue attribute that "
                               "is not compatible with this REM helper version.");
                }
              continue;
            }

          // create initial attribute value to store attribute
          Ptr<AttributeValue> attributeValue = attributeInfo.checker->Create () ;
          // get the attribute value from the object
          object->GetAttribute(attributeInfo.name, *attributeValue);
          //skip pointer value attributes and warn if there is some new unexpceted attribute
          objectFactory.Set (attributeInfo.name, *attributeValue);
          //log current attribute name and value
          NS_LOG_DEBUG ("Copy attribute: " << attributeInfo.name << " value:" <<
                        attributeValue->SerializeToString (attributeInfo.checker));
        }

      if (tid.HasParent ())
        {
          tid = tid.GetParent ();
          hasParent = true;
        }
      else
        {
          hasParent = false;
        }
    }
  while (hasParent);
  return objectFactory;
}

void
NrRadioEnvironmentMapHelper::CreateRem (const NetDeviceContainer &rtdNetDev,
                                        const Ptr<NetDevice> &rrdDevice, uint8_t bwpId)
{
  NS_LOG_FUNCTION (this);

  for (NetDeviceContainer::Iterator netDevIt = rtdNetDev.Begin ();
      netDevIt != rtdNetDev.End ();
      ++netDevIt)
    {
      Ptr<NrGnbNetDevice> gnbRtdNetDevice = DynamicCast<NrGnbNetDevice> (*netDevIt);
      Ptr<NrUeNetDevice> ueRtdNetDevice = DynamicCast<NrUeNetDevice> (*netDevIt);

      if (gnbRtdNetDevice)
      {
        std::cout << "gnb is RTD (transmitter)" << std::endl;
        m_rtdDeviceToPhy.insert (std::make_pair (*netDevIt, (*netDevIt)->GetObject<NrGnbNetDevice> ()->GetPhy (bwpId)));
      }
      else if (ueRtdNetDevice)
      {
        std::cout << "ue is RTD (transmitter)" << std::endl;
        m_rtdDeviceToPhy.insert (std::make_pair (*netDevIt, (*netDevIt)->GetObject<NrUeNetDevice> ()->GetPhy (bwpId)));
      }
      else
      {
        NS_FATAL_ERROR ("no RTD device!");
      }
    }

  Ptr<NrGnbNetDevice> gnbRrdNetDevice = DynamicCast<NrGnbNetDevice> (rrdDevice);
  Ptr<NrUeNetDevice> ueRrdNetDevice = DynamicCast<NrUeNetDevice> (rrdDevice);

  if (gnbRrdNetDevice)
  {
    std::cout <<"gnb is RRD (receiver)" << std::endl;
    m_rrdPhy = (rrdDevice)->GetObject<NrGnbNetDevice> ()->GetPhy (bwpId);
  }
  else if (ueRrdNetDevice)
  {
    std::cout << "ue is RRD (receiver)" << std::endl;
    m_rrdPhy = (rrdDevice)->GetObject<NrUeNetDevice> ()->GetPhy (bwpId);
  }
  else
  {
    NS_FATAL_ERROR ("no RRD device!");
  }

  // save user defined beams, it is like a snapshot, because later during
  // simulation they might change, and for the BEAM_SHAPE type of map
  // is necessary to have a specific snapshot of beams for each device

  // since we have delayed install maybe beamforming vector has changed in the RTD device
  // we want to save beamforming vector toward UE in the case that we will
  // need it for BEAM_SHAPE calculation
  SaveAntennasWithUserDefinedBeams (rtdNetDev, rrdDevice);

  Simulator::Schedule (m_installationDelay, &NrRadioEnvironmentMapHelper::DelayedInstall, this, rtdNetDev, rrdDevice);
}

void
NrRadioEnvironmentMapHelper::SaveAntennasWithUserDefinedBeams (const NetDeviceContainer &rtdNetDev,
                                                               const Ptr<NetDevice> &rrdDevice)
{
  m_deviceToAntenna.insert (std::make_pair (rrdDevice, Copy (m_rrdPhy->GetSpectrumPhy()->GetAntenna ()->GetObject<UniformPlanarArray>())));
  for (NetDeviceContainer::Iterator rtdNetDevIt = rtdNetDev.Begin ();
       rtdNetDevIt != rtdNetDev.End ();
       ++rtdNetDevIt)
      {
        Ptr<NrPhy> rtdPhy = m_rtdDeviceToPhy.find (*rtdNetDevIt)->second;
        m_deviceToAntenna.insert (std::make_pair (*rtdNetDevIt, Copy (rtdPhy->GetSpectrumPhy()->GetAntenna ()->GetObject<UniformPlanarArray>())));
      }
}

void
NrRadioEnvironmentMapHelper::DelayedInstall (const NetDeviceContainer &rtdNetDev,
                                             const Ptr<NetDevice> &rrdDevice)
{
  NS_LOG_FUNCTION (this);
  //Save REM creation start time
  m_remStartTime = std::chrono::system_clock::now ();

  ConfigureRrd (rrdDevice);
  ConfigureRtdList (rtdNetDev);
  CreateListOfRemPoints ();
  if (m_remMode == COVERAGE_AREA)
    {
      CalcCoverageAreaRemMap ();
    }
  else if (m_remMode == BEAM_SHAPE)
    {
      CalcBeamShapeRemMap ();
    }
  else if (m_remMode == UE_COVERAGE)
    {
      CalcUeCoverageRemMap ();
    }
  else
    {
      NS_FATAL_ERROR ("Unknown REM mode");
    }
  PrintRemToFile ();

  std::ostringstream ossGnbs;
  ossGnbs << "nr-rem-" << m_simTag.c_str () << "-gnbs.txt";
  PrintGnuplottableGnbListToFile (ossGnbs.str ());
  std::ostringstream ossBuildings;
  ossBuildings << "nr-rem-" << m_simTag.c_str () << "-buildings.txt";
  PrintGnuplottableBuildingListToFile (ossBuildings.str ());
}

void
NrRadioEnvironmentMapHelper::CreateListOfRemPoints ()
{
  NS_LOG_FUNCTION (this);

  //Create the list of the REM Points

  m_xStep = (m_xMax - m_xMin) / (m_xRes);
  m_yStep = (m_yMax - m_yMin) / (m_yRes);

  NS_ASSERT_MSG (m_xMax > m_xMin, "xMax must be higher than xMin");
  NS_ASSERT_MSG (m_yMax > m_yMin, "yMax must be higher than yMin");
  NS_ASSERT_MSG (m_xRes != 0 || m_yRes != 0, "Resolution must be higher than 0");

  NS_LOG_INFO ("m_xStep: " << m_xStep << " m_yStep: " << m_yStep);

  for (double x = m_xMin; x < m_xMax + 0.5*m_xStep; x += m_xStep)
    {
      for (double y = m_yMin; y < m_yMax + 0.5*m_yStep ; y += m_yStep)
        {
          //In case a REM Point is in the same position as a rtd, ignore this point
          bool isPositionRtd = false;
          for (std::list<RemDevice>::iterator itRtd = m_remDev.begin (); itRtd != m_remDev.end (); ++itRtd)
          {
            if (itRtd->mob->GetPosition () == Vector (x, y, m_z))
            {
              isPositionRtd = true;
            }
          }

          if (!isPositionRtd)
          {
            RemPoint remPoint;

            remPoint.pos.x = x;
            remPoint.pos.y = y;
            remPoint.pos.z = m_z;

            m_rem.push_back (remPoint);
          }
        }
    }
}

void
NrRadioEnvironmentMapHelper::ConfigureQuasiOmniBfv (RemDevice& device)
{
  NS_LOG_FUNCTION (this);
  // configure beam on rrd antenna to be quasi-omni
  UintegerValue numRows, numColumns;
  device.antenna->GetAttribute ("NumRows", numRows);
  device.antenna->GetAttribute ("NumColumns", numColumns);
  // configure RRD antenna to have quasi omni beamforming vector
  device.antenna->SetBeamformingVector (CreateQuasiOmniBfv (numRows.Get (), numColumns.Get ()));
}

void
NrRadioEnvironmentMapHelper::ConfigureDirectPathBfv (RemDevice& device,
                                                     const RemDevice& otherDevice,
                                                     const Ptr<const UniformPlanarArray>& antenna)
{
  NS_LOG_FUNCTION (this);
  device.antenna->SetBeamformingVector (CreateDirectPathBfv (device.mob, otherDevice.mob, antenna));
}

Ptr<SpectrumValue>
NrRadioEnvironmentMapHelper::CalcRxPsdValue (RemDevice& device, RemDevice& otherDevice) const
{
  PropagationModels tempPropModels = CreateTemporalPropagationModels ();

  std::vector<int> activeRbs;
  for (size_t rbId = 0; rbId < device.spectrumModel->GetNumBands(); rbId++)
    {
      activeRbs.push_back(rbId);
    }

  Ptr<const SpectrumValue> txPsd = NrSpectrumValueHelper::CreateTxPowerSpectralDensity (device.txPower, activeRbs,
                                                                                        device.spectrumModel, NrSpectrumValueHelper::UNIFORM_POWER_ALLOCATION_BW);

  // check if RTD has the same spectrum model as RRD
  // if they have do nothing, if they dont, then convert txPsd of RTD device so to be according to spectrum model of RRD

  Ptr <const SpectrumValue> convertedTxPsd;
  if (device.spectrumModel->GetUid () == otherDevice.spectrumModel->GetUid ())
    {
      NS_LOG_LOGIC ("no spectrum conversion needed");
      convertedTxPsd = txPsd;
    }
  else
    {
      NS_LOG_LOGIC ("Converting TXPSD of RTD device " <<
                    device.spectrumModel->GetUid ()  << " --> " <<
                    otherDevice.spectrumModel->GetUid ());

      SpectrumConverter converter (device.spectrumModel, otherDevice.spectrumModel);
      convertedTxPsd = converter.Convert (txPsd);
    }

  // Copy TX PSD to RX PSD, they are now equal rxPsd == txPsd
  Ptr<SpectrumValue> rxPsd = convertedTxPsd->Copy ();
  double pathLossDb = tempPropModels.remPropagationLossModelCopy->CalcRxPower (0, device.mob, otherDevice.mob);
  double pathGainLinear = DbToRatio (pathLossDb);

  NS_LOG_DEBUG ("Tx power in dBm:" <<  WToDbm (Integral (*convertedTxPsd)));
  NS_LOG_DEBUG ("PathlosDb:" << pathLossDb);

  // Apply now calculated pathloss to rxPsd, now rxPsd < txPsd because we had some losses
  *(rxPsd) *= pathGainLinear;

  NS_LOG_DEBUG ("RX power in dBm after pathloss:" << WToDbm (Integral (*rxPsd)));

  // Now we call spectrum model, which in this keys add a beamforming gain
  rxPsd = tempPropModels.remSpectrumLossModelCopy->DoCalcRxPowerSpectralDensity (rxPsd, device.mob, otherDevice.mob, device.antenna, otherDevice.antenna);

  NS_LOG_DEBUG ("RX power in dBm after fading: " << WToDbm (Integral (*rxPsd)));

  return rxPsd;
}

Ptr<SpectrumValue>
NrRadioEnvironmentMapHelper::GetMaxValue (const std::list <Ptr<SpectrumValue>>& values) const
{
  //TODO add this abort, if necessary add include for abort.h
  NS_ABORT_MSG_IF (values.size () == 0, "Must provide a list of values.");

  Ptr<SpectrumValue> maxValue = Create <SpectrumValue> (m_rrd.spectrumModel);
  *maxValue = **(values.begin ());

  for (const auto &value: values)
    {
      if (Sum (*(value)) > Sum (*(maxValue)))
        {
          *maxValue = *value;
        }
    }
  return maxValue;
}

double
NrRadioEnvironmentMapHelper::CalculateMaxSnr (const std::list <Ptr<SpectrumValue>>& receivedPowerList) const
{
  Ptr<SpectrumValue> maxSnr = GetMaxValue (receivedPowerList);
  SpectrumValue snr = (*maxSnr) / (*m_noisePsd);
  return RatioToDb (Sum (snr) / snr.GetSpectrumModel ()->GetNumBands ());
}

double
NrRadioEnvironmentMapHelper::CalculateSnr (const Ptr<SpectrumValue>& usefulSignal) const
{
   SpectrumValue snr = (*usefulSignal) / (*m_noisePsd);

   return RatioToDb (Sum (snr) / snr.GetSpectrumModel ()->GetNumBands ());
}

double
NrRadioEnvironmentMapHelper::CalculateSinr (const Ptr<SpectrumValue>& usefulSignal,
                                            const std::list <Ptr<SpectrumValue>>& interferenceSignals) const
{
  Ptr<SpectrumValue> interferencePsd = nullptr;

  if (interferenceSignals.size () == 0)
    {
      return CalculateSnr (usefulSignal);
    }
  else
    {
      interferencePsd = Create<SpectrumValue> (m_rrd.spectrumModel);
    }

  // sum all interfering signals
  for (auto rxInterfPower: interferenceSignals)
    {
      *interferencePsd += (*rxInterfPower);
    }
  // calculate sinr

  SpectrumValue sinr = (*usefulSignal) / (*interferencePsd + *m_noisePsd) ;

  // calculate average sinr over RBs, convert it from linear to dB units, and return it
  return RatioToDb (Sum (sinr) / sinr.GetSpectrumModel ()->GetNumBands ()) ;
}

double
NrRadioEnvironmentMapHelper::CalculateSir (const Ptr<SpectrumValue>& usefulSignal,
                                           const std::list <Ptr<SpectrumValue>>& interferenceSignals) const
{
  Ptr<SpectrumValue> interferencePsd = nullptr;

  if (interferenceSignals.size () == 0)
    {
      //return CalculateSnr (usefulSignal);
      SpectrumValue signal = (*usefulSignal);
      return RatioToDb (Sum (signal) / signal.GetSpectrumModel ()->GetNumBands ());
    }
  else
    {
      interferencePsd = Create<SpectrumValue> (m_rrd.spectrumModel);
    }

  // sum all interfering signals
  for (auto rxInterfPower: interferenceSignals)
    {
      *interferencePsd += (*rxInterfPower);
    }
  // calculate sinr

  SpectrumValue sir = (*usefulSignal) / (*interferencePsd) ;

  // calculate average sir over RBs, convert it from linear to dB units, and return it
  return RatioToDb (Sum (sir) / sir.GetSpectrumModel ()->GetNumBands ()) ;
}

double
NrRadioEnvironmentMapHelper::CalculateMaxSinr (const std::list <Ptr<SpectrumValue>>& receivedPowerList) const
{
  // we calculate sinr considering for each RTD as if it would be TX device, and the rest of RTDs interferers
  std::list <double> sinrList;

  for (std::list <Ptr<SpectrumValue>>::const_iterator it = receivedPowerList.begin ();
        it!=receivedPowerList.end (); it++)
    {
      //all signals - rxPower = interference
      std::list <Ptr<SpectrumValue>> interferenceSignals;
      std::list <Ptr<SpectrumValue>>::const_iterator tempit = it;

      if (it!=receivedPowerList.begin ())
        {
          interferenceSignals.insert (interferenceSignals.begin (), receivedPowerList.begin (), it);
        }

      interferenceSignals.insert (interferenceSignals.end (), ++tempit, receivedPowerList.end ());
      NS_ASSERT(interferenceSignals.size () == receivedPowerList.size ()-1);
      sinrList.push_back (CalculateSinr (*it, interferenceSignals));
    }
  return GetMaxValue (sinrList);
}

double
NrRadioEnvironmentMapHelper::CalculateMaxSir (const std::list <Ptr<SpectrumValue>>& receivedPowerList) const
{
  // we calculate sinr considering for each RTD as if it would be TX device, and the rest of RTDs interferers
  std::list <double> sirList;

  for (std::list <Ptr<SpectrumValue>>::const_iterator it = receivedPowerList.begin ();
        it!=receivedPowerList.end (); it++)
    {
      //all signals - rxPower = interference
      std::list <Ptr<SpectrumValue>> interferenceSignals;
      std::list <Ptr<SpectrumValue>>::const_iterator tempit = it;

      if (it!=receivedPowerList.begin ())
        {
          interferenceSignals.insert (interferenceSignals.begin (), receivedPowerList.begin (), it);
        }

      interferenceSignals.insert (interferenceSignals.end (), ++tempit, receivedPowerList.end ());
      NS_ASSERT(interferenceSignals.size () == receivedPowerList.size ()-1);
      sirList.push_back (CalculateSir (*it, interferenceSignals));
    }
  return GetMaxValue (sirList);
}

void
NrRadioEnvironmentMapHelper::CalcBeamShapeRemMap ()
{
  NS_LOG_FUNCTION (this);
  uint16_t calcRxPsdCounter = 0;

  uint32_t remSizeNextReport = m_rem.size () / 100;
  uint32_t remPointCounter = 0;

  for (std::list<RemPoint>::iterator itRemPoint = m_rem.begin ();
      itRemPoint != m_rem.end ();
      ++itRemPoint)
    {
      //perform calculation m_numOfIterationsToAverage times and get the average value
      double sumSnr = 0.0, sumSinr = 0.0;
      double sumSir = 0.0;
      std::list<double> rxPsdsListPerIt; //list to save the summed rxPower in each RemPoint for each Iteration (linear)
      m_rrd.mob->SetPosition (itRemPoint->pos);

      Ptr <MobilityBuildingInfo> buildingInfo = m_rrd.mob->GetObject <MobilityBuildingInfo> ();
      buildingInfo->MakeConsistent (m_rrd.mob);
      NS_ASSERT_MSG (buildingInfo, "buildingInfo is null");

      for (uint16_t i = 0; i < m_numOfIterationsToAverage; i++)
        {
          std::list <Ptr<SpectrumValue>> receivedPowerList;// RTD node id, rxPsd of the singal coming from that node

          for (std::list<RemDevice>::iterator itRtd = m_remDev.begin ();
               itRtd != m_remDev.end ();
               ++itRtd)
            {
              calcRxPsdCounter++;

               // calculate received power from the current RTD device
              receivedPowerList.push_back (CalcRxPsdValue (*itRtd, m_rrd));
            } //end for std::list<RemDev>::iterator  (RTDs)

          sumSnr += CalculateMaxSnr (receivedPowerList);
          sumSinr += CalculateMaxSinr (receivedPowerList);
          sumSir += CalculateMaxSir (receivedPowerList);

          //Sum all the rxPowers (for this RemPoint) and put the result to the list for each Iteration (linear)
          rxPsdsListPerIt.push_back (CalculateAggregatedIpsd (receivedPowerList));

          receivedPowerList.clear ();
        }//end for m_numOfIterationsToAverage  (Average)

      //Sum the rxPower for all the Iterations (linear)
      double rxPsdsAllIt = SumListElements (rxPsdsListPerIt);

      itRemPoint->avgSnrDb = sumSnr / static_cast <double> (m_numOfIterationsToAverage);
      itRemPoint->avgSinrDb = sumSinr / static_cast <double> (m_numOfIterationsToAverage);
      itRemPoint->avgSirDb = sumSir / static_cast <double> (m_numOfIterationsToAverage);
      //do the average (for the rxPowers in each RemPoint) in linear and then convert to dBm
      itRemPoint->avRxPowerDbm = WToDbm (rxPsdsAllIt / static_cast <double> (m_numOfIterationsToAverage));

      NS_LOG_INFO ("Avg snr value saved:" << itRemPoint->avgSnrDb);
      NS_LOG_INFO ("Avg sinr value saved:" << itRemPoint->avgSinrDb);
      NS_LOG_INFO ("Avg ipsd value saved (dBm):" << itRemPoint->avRxPowerDbm);

      if (++remPointCounter == remSizeNextReport)
        {
          PrintProgressReport (&remSizeNextReport);
        }

    } //end for std::list<RemPoint>::iterator  (RemPoints)

  auto remEndTime = std::chrono::system_clock::now ();
  std::chrono::duration<double> remElapsedSeconds = remEndTime - m_remStartTime;
  NS_LOG_INFO ("REM map created. Total time needed to create the REM map:" <<
                 remElapsedSeconds.count () / 60 << " minutes.");
}

double
NrRadioEnvironmentMapHelper::GetMaxValue (const std::list<double>& listOfValues) const
{
  NS_ABORT_MSG_IF (listOfValues.size () == 0, "GetMaxValue should not be called "
                                              "with an empty list.");

  double maxValue = *(listOfValues.begin ());
  //start from second element, the first is already taken into account
  for(auto it = ++listOfValues.begin (); it != listOfValues.end (); ++it)
    {
      if (*it > maxValue)
        {
          maxValue = *it;
        }
    }
  return maxValue;
}

double
NrRadioEnvironmentMapHelper::CalculateAggregatedIpsd (const std::list <Ptr<SpectrumValue>>& receivedSignals)
{
    Ptr<SpectrumValue> sumRxPowers = nullptr;
    sumRxPowers = Create<SpectrumValue> (m_rrd.spectrumModel);

    // sum the received power of all the rtds
    for (auto rxPowersIt: receivedSignals)
      {
        *sumRxPowers += (*rxPowersIt);
      }

    SpectrumValue sumRxPowersSv = (*sumRxPowers) ;

    return (Integral (sumRxPowersSv));
}

double
NrRadioEnvironmentMapHelper::SumListElements (const std::list<double>& listOfValues)
{
    NS_ABORT_MSG_IF (listOfValues.size () == 0, "SumListElements should not be called "
                                                "with an empty list.");

    double sum = 0;

    for(auto it = listOfValues.begin (); it != listOfValues.end (); ++it)
      {
        sum += *it;
      }
    return sum;
}

void
NrRadioEnvironmentMapHelper::CalcCoverageAreaRemMap ()
{
  NS_LOG_FUNCTION (this);
  uint16_t calcRxPsdCounter = 0;
  uint32_t remSizeNextReport = m_rem.size () / 100;
  uint32_t remPointCounter = 0;

  for (std::list<RemPoint>::iterator itRemPoint = m_rem.begin ();
      itRemPoint != m_rem.end ();
      ++itRemPoint)
    {
      //perform calculation m_numOfIterationsToAverage times and get the average value
      double sumSnr = 0.0, sumSinr = 0.0;
      m_rrd.mob->SetPosition (itRemPoint->pos);

      // all RTDs should point toward that RemPoint with DirectPah beam, this is definition of worst-case scenario
     for(std::list<RemDevice>::iterator itRtd = m_remDev.begin ();
         itRtd != m_remDev.end ();
                ++itRtd)
        {
          ConfigureDirectPathBfv (*itRtd, m_rrd, itRtd->antenna);
        }

      std::list<double> rxPsdsListPerIt; //list to save the summed rxPower in each RemPoint for each Iteration (linear)

      for (uint16_t i = 0; i < m_numOfIterationsToAverage; i++)
        {
          std::list<double> sinrsPerBeam; // vector in which we will save sinr per each RRD beam
          std::list<double> snrsPerBeam; // vector in which we will save snr per each RRD beam

          std::list<Ptr<SpectrumValue>> rxPsdsList; //vector in which we will save the sum of rxPowers per remPoint (linear)

          // For each beam configuration at RemPoint/RRD we should calculate SINR, there are as many beam configurations at RemPoint as many RTDs
          for (std::list<RemDevice>::iterator itRtdBeam = m_remDev.begin (); itRtdBeam != m_remDev.end (); ++itRtdBeam)
            {
              //configure RRD beam toward RTD
              ConfigureDirectPathBfv (m_rrd, *itRtdBeam, m_rrd.antenna);

              //Calculate the received power from this RTD for this RemPoint
              Ptr<SpectrumValue> receivedPowerFromRtd = CalcRxPsdValue (*itRtdBeam, m_rrd);
              //and put it to the list of the received powers for this RemPoint (to sum all later)
              rxPsdsList.push_back (receivedPowerFromRtd);

              NS_LOG_DEBUG ("beam node: " << itRtdBeam->dev->GetNode ()->GetId () <<
                            " is Rxed in RemPoint with Rx Power in W: " << (Integral (*receivedPowerFromRtd)));
              NS_LOG_DEBUG ("RxPower in dBm: " << WToDbm (Integral (*receivedPowerFromRtd)));

              std::list<Ptr<SpectrumValue>> interferenceSignalsRxPsds;
              Ptr<SpectrumValue> usefulSignalRxPsd;

              // For this configuration of beam at RRD, we need to calculate RX PSD,
              // and in order to be able to calculate SINR for that beam,
              // we need to calculate received PSD for each RTD using this beam at RRD
              for(std::list<RemDevice>::iterator itRtdCalc = m_remDev.begin (); itRtdCalc != m_remDev.end (); ++itRtdCalc)
                {
                  // increase counter de calcRXPsd calls
                  calcRxPsdCounter++;
                  // calculate received power from the current RTD device
                  Ptr<SpectrumValue> receivedPower = CalcRxPsdValue (*itRtdCalc, m_rrd);

                  // is this received power useful signal (from RTD for which I configured my beam) or is interference signal

                  if (itRtdBeam->dev->GetNode ()->GetId () == itRtdCalc->dev->GetNode ()->GetId ())
                    {
                      if (usefulSignalRxPsd != nullptr)
                        {
                          NS_FATAL_ERROR ("Already assigned usefulSignal!");
                        }
                      usefulSignalRxPsd = receivedPower;
                    }
                  else
                    {
                      interferenceSignalsRxPsds.push_back (receivedPower);  //interference
                    }

                } //end for std::list<RemDev>::iterator itRtdCalc (RTDs)

              sinrsPerBeam.push_back (CalculateSinr (usefulSignalRxPsd, interferenceSignalsRxPsds));
              snrsPerBeam.push_back (CalculateSnr (usefulSignalRxPsd));

              NS_LOG_INFO ("Done:" <<
                           (double)calcRxPsdCounter/(m_rem.size()*m_numOfIterationsToAverage*m_remDev.size ()*m_remDev.size()) * 100 <<
                           " %."); // how many times will be called CalcRxPsdValues

            } //end for std::list<RemDev>::iterator itRtdBeam (RTDs)

          sumSnr += GetMaxValue (snrsPerBeam);
          sumSinr += GetMaxValue (sinrsPerBeam);

          //Sum all the rxPowers (for this RemPoint) and put the result to the list for each Iteration (linear)
          rxPsdsListPerIt.push_back (CalculateAggregatedIpsd (rxPsdsList));

        }//end for m_numOfIterationsToAverage  (Average)

      //Sum the rxPower for all the Iterations (linear)
      double rxPsdsAllIt = SumListElements (rxPsdsListPerIt);

      itRemPoint->avgSnrDb = sumSnr / static_cast <double> (m_numOfIterationsToAverage);
      itRemPoint->avgSinrDb = sumSinr / static_cast <double> (m_numOfIterationsToAverage);
      //do the average (for the rxPowers in each RemPoint) in linear and then convert to dBm
      itRemPoint->avRxPowerDbm = WToDbm (rxPsdsAllIt / static_cast <double> (m_numOfIterationsToAverage));

      if (++remPointCounter == remSizeNextReport)
        {
          PrintProgressReport (&remSizeNextReport);
        }

      NS_LOG_DEBUG ("itRemPoint->avRxPowerDb  in dB: " << itRemPoint->avRxPowerDbm);

    } //end for std::list<RemPoint>::iterator  (RemPoints)

  auto remEndTime = std::chrono::system_clock::now ();
  std::chrono::duration<double> remElapsedSeconds = remEndTime - m_remStartTime;
  NS_LOG_INFO ("REM map created. Total time needed to create the REM map:" <<
                 remElapsedSeconds.count () / 60 << " minutes.");
}

void
NrRadioEnvironmentMapHelper::PrintProgressReport (uint32_t* remSizeNextReport)
{
  auto remTimeUpToNow = std::chrono::system_clock::now ();
  std::chrono::duration<double> remElapsedSecondsUpToNow = remTimeUpToNow - m_remStartTime;
  double minutesUpToNow = ((double) remElapsedSecondsUpToNow.count ()) / 60;
  double minutesLeftEstimated = ((double) (minutesUpToNow) / *remSizeNextReport) * ((m_rem.size () - *remSizeNextReport));
  std::cout << "\n REM done:" << ceil (((double) *remSizeNextReport / m_rem.size()) * 100) << " %." << " Minutes up to now: " << minutesUpToNow << ". Minutes left estimated:" << minutesLeftEstimated << "."; // how many times will be called CalcRxPsdValues
  // we want progress report for 1%, 10%, 20%, 30%, and so on
  if (*remSizeNextReport < m_rem.size () / 10 )
    {
      *remSizeNextReport = m_rem.size () / 10;
    }
  else
    {
      *remSizeNextReport += m_rem.size () / 10;
    }
}

void
NrRadioEnvironmentMapHelper::CalcUeCoverageRemMap ()
{
    NS_LOG_FUNCTION (this);

    uint32_t remSizeNextReport = m_rem.size () / 100;
    uint32_t remPointCounter = 0;

    for (std::list<RemPoint>::iterator itRemPoint = m_rem.begin ();
        itRemPoint != m_rem.end ();
        ++itRemPoint)
      {
        //perform calculation m_numOfIterationsToAverage times and get the average value
        double sumSnr = 0.0, sumSinr = 0.0;
        m_rrd.mob->SetPosition (itRemPoint->pos);

        for (uint16_t i = 0; i < m_numOfIterationsToAverage; i++)
          {
            std::list<double> sinrsPerBeam; // vector in which we will save sinr per each RRD beam
            std::list<double> snrsPerBeam; // vector in which we will save snr per each RRD beam

            //"Associate" UE (RemPoint) with this RTD
            for (std::list<RemDevice>::iterator itRtdAssociated = m_remDev.begin ();
                 itRtdAssociated != m_remDev.end ();
                 ++itRtdAssociated)
              {
                //configure RRD (RemPoint) beam toward RTD (itRtdAssociated)
                ConfigureDirectPathBfv (m_rrd, *itRtdAssociated, m_rrd.antenna);
                //configure RTD (itRtdAssociated) beam toward RRD (RemPoint)
                ConfigureDirectPathBfv (*itRtdAssociated, m_rrd, itRtdAssociated->antenna);

                std::list<Ptr<SpectrumValue>> interferenceSignalsRxPsds;
                Ptr<SpectrumValue> usefulSignalRxPsd;

                for(std::list<RemDevice>::iterator itRtdInterferer = m_remDev.begin ();
                    itRtdInterferer != m_remDev.end ();
                    ++itRtdInterferer)
                  {
                    if (itRtdAssociated->dev->GetNode ()->GetId () != itRtdInterferer->dev->GetNode ()->GetId ())
                    {
                      //configure RTD (itRtdInterferer) beam toward RTD (itRtdAssociated)
                      ConfigureDirectPathBfv (*itRtdInterferer, *itRtdAssociated, itRtdInterferer->antenna);

                      // calculate received power (interference) from the current RTD device
                      Ptr<SpectrumValue> receivedPower = CalcRxPsdValue (*itRtdInterferer, *itRtdAssociated);

                      interferenceSignalsRxPsds.push_back (receivedPower);  //interference
                    }
                    else
                    {
                      // calculate received power (useful Signal) from the current RRD device
                      Ptr<SpectrumValue> receivedPower = CalcRxPsdValue (m_rrd, *itRtdAssociated);
                      if (usefulSignalRxPsd != nullptr)
                        {
                          NS_FATAL_ERROR ("Already assigned usefulSignal!");
                        }
                      usefulSignalRxPsd = receivedPower;
                    }

                  }//end for std::list<RemDev>::iterator itRtdInterferer (RTD)

                sinrsPerBeam.push_back (CalculateSinr (usefulSignalRxPsd, interferenceSignalsRxPsds));
                snrsPerBeam.push_back (CalculateSnr (usefulSignalRxPsd));

              }//end for std::list<RemDev>::iterator itRtdAssociated (RTD)

            sumSnr += GetMaxValue (snrsPerBeam);
            sumSinr += GetMaxValue (sinrsPerBeam);

          }//end for m_numOfIterationsToAverage  (Average)

        itRemPoint->avgSnrDb = sumSnr / static_cast <double> (m_numOfIterationsToAverage);
        itRemPoint->avgSinrDb = sumSinr / static_cast <double> (m_numOfIterationsToAverage);

        if (++remPointCounter == remSizeNextReport)
          {
            PrintProgressReport (&remSizeNextReport);
          }

      }//end for std::list<RemPoint>::iterator  (RemPoints)

    auto remEndTime = std::chrono::system_clock::now ();
    std::chrono::duration<double> remElapsedSeconds = remEndTime - m_remStartTime;
    NS_LOG_INFO ("REM map created. Total time needed to create the REM map:" <<
                 remElapsedSeconds.count () / 60 << " minutes.");
}

NrRadioEnvironmentMapHelper::PropagationModels
NrRadioEnvironmentMapHelper::CreateTemporalPropagationModels () const
{
  NS_LOG_FUNCTION (this);

  PropagationModels propModels;
  //create rem copy of channel condition
  Ptr<ChannelConditionModel> condModelCopy = m_channelConditionModelFactory.Create<ChannelConditionModel> ();

  //create rem copy of propagation model
  ObjectFactory propLossModelFactory = ConfigureObjectFactory (m_propagationLossModel);
  propModels.remPropagationLossModelCopy = propLossModelFactory.Create <ThreeGppPropagationLossModel> ();
  propModels.remPropagationLossModelCopy->SetChannelConditionModel (condModelCopy);

  //create rem copy of spectrum loss model
  ObjectFactory spectrumLossModelFactory = ConfigureObjectFactory (m_phasedArraySpectrumLossModel);
  if (spectrumLossModelFactory.IsTypeIdSet())
    {
      Ptr<MatrixBasedChannelModel> channelModelCopy = m_matrixBasedChannelModelFactory.Create<MatrixBasedChannelModel>();
      channelModelCopy->SetAttribute("ChannelConditionModel", PointerValue (condModelCopy));
      spectrumLossModelFactory.Set ("ChannelModel", PointerValue (channelModelCopy));
      propModels.remSpectrumLossModelCopy = spectrumLossModelFactory.Create <ThreeGppSpectrumPropagationLossModel> ();
    }
  return propModels;
}

void
NrRadioEnvironmentMapHelper::PrintGnuplottableGnbListToFile (const std::string &filename)
{
  NS_LOG_FUNCTION (this);
  std::ofstream gnbOutFile;
  gnbOutFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!gnbOutFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  for (std::list<RemDevice>::iterator itRtd = m_remDev.begin ();
       itRtd != m_remDev.end ();
       ++itRtd)
   {
      Vector pos = itRtd->dev->GetNode ()->GetObject<MobilityModel> ()->GetPosition ();

      gnbOutFile << "set label \"" << itRtd->dev->GetNode ()->GetId () <<
                    "\" at "<< pos.x << "," << pos.y <<
                    " left font \"Helvetica,4\" textcolor rgb \"white\" front  point pt 2 ps 1 lc rgb \"white\" offset 0,0" <<
                    std::endl;
    }

  gnbOutFile.close ();
}

void
NrRadioEnvironmentMapHelper::PrintGnuplottableUeListToFile (const std::string &filename)
{
  NS_LOG_FUNCTION (this);
  std::ofstream ueOutFile;
  ueOutFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!ueOutFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  Vector pos = m_rrd.node->GetObject<MobilityModel> ()->GetPosition ();

  ueOutFile << "set label \"" << m_rrd.dev->GetNode ()->GetId () <<
               "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,4\" textcolor rgb \"grey\" front point pt 1 ps 1 lc rgb \"grey\" offset 0,0" <<
               std::endl;

  ueOutFile.close ();
}

void
NrRadioEnvironmentMapHelper::PrintGnuplottableBuildingListToFile (const std::string &filename)
{
  NS_LOG_FUNCTION (this);
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  uint32_t index = 0;

  for (BuildingList::Iterator it = BuildingList::Begin (); it != BuildingList::End (); ++it)
    {
      ++index;
      Box box = (*it)->GetBoundaries ();
      outFile << "set object " << index << " rect from " <<
                 box.xMin  << "," << box.yMin << " to " <<
                 box.xMax  << "," << box.yMax <<
                 " front fs empty " << " border 3 " <<
                 std::endl;
    }

  outFile.close ();
}

void
NrRadioEnvironmentMapHelper::PrintRemToFile ()
{
  NS_LOG_FUNCTION (this);

  std::ostringstream oss;
  oss << "nr-rem-" << m_simTag.c_str() <<".out";

  std::ofstream outFile;
  std::string outputFile = oss.str ();
  outFile.open (outputFile.c_str ());

  if (!outFile.is_open ())
      {
        NS_FATAL_ERROR ("Can't open file " << (outputFile));
        return;
      }

  for (std::list<RemPoint>::iterator it = m_rem.begin ();
       it != m_rem.end ();
       ++it)
    {
      outFile << it->pos.x << "\t" <<
                 it->pos.y << "\t" <<
                 it->pos.z << "\t" <<
                 it->avgSnrDb << "\t" <<
                 it->avgSinrDb << "\t" <<
                 it->avRxPowerDbm << "\t" <<
                 it->avgSirDb << "\t" <<
                 std::endl;
    }

  outFile.close();

  CreateCustomGnuplotFile ();
  Finalize ();
}

void
NrRadioEnvironmentMapHelper::CreateCustomGnuplotFile ()
{
  NS_LOG_FUNCTION (this);
  std::ostringstream oss;
  oss << "nr-rem-" << m_simTag.c_str () << "-plot-rem.gnuplot";
  std::string filename = oss.str ();

  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      NS_LOG_ERROR ("Can't open file " << filename);
      return;
    }

  outFile << "set xlabel \"x-coordinate (m)\"" << std::endl;
  outFile << "set ylabel \"y-coordinate (m)\"" << std::endl;
  outFile << "set cblabel \"SNR (dB)\"" << std::endl;
  outFile << "set cblabel offset 3" << std::endl;
  outFile << "unset key" << std::endl;
  outFile << "set terminal png" << std::endl;
  outFile << "set output \"nr-rem-" << m_simTag << "-snr.png\"" << std::endl;
  outFile << "set size ratio -1" << std::endl;
  outFile << "set cbrange [-5:30]" << std::endl;
  outFile << "set xrange [" << m_xMin << ":" << m_xMax << "]" << std::endl;
  outFile << "set yrange [" << m_yMin << ":" << m_yMax << "]" << std::endl;
  outFile << "set xtics font \"Helvetica,17\"" << std::endl;
  outFile << "set ytics font \"Helvetica,17\"" << std::endl;
  outFile << "set cbtics font \"Helvetica,17\"" << std::endl;
  outFile << "set xlabel font \"Helvetica,17\"" << std::endl;
  outFile << "set ylabel font \"Helvetica,17\"" << std::endl;
  outFile << "set cblabel font \"Helvetica,17\"" << std::endl;
  outFile << "plot \"nr-rem-" << m_simTag << ".out\" using ($1):($2):($4) with image" << std::endl;

  outFile << "set xlabel \"x-coordinate (m)\"" << std::endl;
  outFile << "set ylabel \"y-coordinate (m)\"" << std::endl;
  outFile << "set cblabel \"SINR (dB)\"" << std::endl;
  outFile << "set cblabel offset 3" << std::endl;
  outFile << "unset key" << std::endl;
  outFile << "set terminal png" << std::endl;
  outFile << "set output \"nr-rem-" << m_simTag << "-sinr.png\"" << std::endl;
  outFile << "set size ratio -1" << std::endl;
  outFile << "set cbrange [-5:30]" << std::endl;
  outFile << "set xrange [" <<m_xMin << ":" << m_xMax<< "]" << std::endl;
  outFile << "set yrange [" <<m_yMin << ":" << m_yMax<< "]" << std::endl;
  outFile << "set xtics font \"Helvetica,17\"" << std::endl;
  outFile << "set ytics font \"Helvetica,17\"" << std::endl;
  outFile << "set cbtics font \"Helvetica,17\"" << std::endl;
  outFile << "set xlabel font \"Helvetica,17\"" << std::endl;
  outFile << "set ylabel font \"Helvetica,17\"" << std::endl;
  outFile << "set cblabel font \"Helvetica,17\"" << std::endl;
  outFile << "plot \"nr-rem-" << m_simTag << ".out\" using ($1):($2):($5) with image" << std::endl;

  outFile << "set xlabel \"x-coordinate (m)\"" << std::endl;
  outFile << "set ylabel \"y-coordinate (m)\"" << std::endl;
  outFile << "set cblabel \"IPSD (dBm)\"" << std::endl;
  outFile << "set cblabel offset 3" << std::endl;
  outFile << "unset key" << std::endl;
  outFile << "set terminal png" << std::endl;
  outFile << "set output \"nr-rem-" << m_simTag << "-ipsd.png\"" << std::endl;
  outFile << "set size ratio -1" << std::endl;
  outFile << "set cbrange [-100:-20]" << std::endl;
  outFile << "set xrange [" << m_xMin << ":" << m_xMax << "]" << std::endl;
  outFile << "set yrange [" << m_yMin << ":" << m_yMax << "]" << std::endl;
  outFile << "set xtics font \"Helvetica,17\"" << std::endl;
  outFile << "set ytics font \"Helvetica,17\"" << std::endl;
  outFile << "set cbtics font \"Helvetica,17\"" << std::endl;
  outFile << "set xlabel font \"Helvetica,17\"" << std::endl;
  outFile << "set ylabel font \"Helvetica,17\"" << std::endl;
  outFile << "set cblabel font \"Helvetica,17\"" << std::endl;
  outFile << "plot \"nr-rem-" << m_simTag << ".out\" using ($1):($2):($6) with image" << std::endl;

  outFile << "set xlabel \"x-coordinate (m)\"" << std::endl;
  outFile << "set ylabel \"y-coordinate (m)\"" << std::endl;
  outFile << "set cblabel \"SIR (dB)\"" << std::endl;
  outFile << "set cblabel offset 3" << std::endl;
  outFile << "unset key" << std::endl;
  outFile << "set terminal png" << std::endl;
  outFile << "set output \"nr-rem-" << m_simTag << "-sir.png\"" << std::endl;
  outFile << "set size ratio -1" << std::endl;
  outFile << "set cbrange [-5:30]" << std::endl;
  outFile << "set xrange [" << m_xMin << ":" << m_xMax << "]" << std::endl;
  outFile << "set yrange [" << m_yMin << ":" << m_yMax << "]" << std::endl;
  outFile << "set xtics font \"Helvetica,17\"" << std::endl;
  outFile << "set ytics font \"Helvetica,17\"" << std::endl;
  outFile << "set cbtics font \"Helvetica,17\"" << std::endl;
  outFile << "set xlabel font \"Helvetica,17\"" << std::endl;
  outFile << "set ylabel font \"Helvetica,17\"" << std::endl;
  outFile << "set cblabel font \"Helvetica,17\"" << std::endl;
  outFile << "plot \"nr-rem-" << m_simTag << ".out\" using ($1):($2):($7) with image" << std::endl;

  outFile.close ();
}

void
NrRadioEnvironmentMapHelper::Finalize ()
{
  NS_LOG_FUNCTION (this);
  // TODO test if we can call this  
  //std::ostringstream oss;
  //oss <<"gnuplot -p nr-"<<m_simTag.c_str()<<"-ues.txt nr-"<<m_simTag.c_str()<<"-gnbs.txt nr-rem-"<<m_simTag.c_str()<<"-buildings.txt nr-rem-"<<m_simTag.c_str()<<"-plot-rem.gnuplot";
  //system(oss.str().c_str());
  // TODO if yes, then we can add also convert command
  Simulator::Stop ();
}


} // namespace ns3

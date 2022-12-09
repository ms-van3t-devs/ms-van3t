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

#ifndef NR_RADIO_ENVIRONMENT_MAP_HELPER_H
#define NR_RADIO_ENVIRONMENT_MAP_HELPER_H

#include <ns3/object-factory.h>
#include "ns3/simple-net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/nr-gnb-phy.h"
#include "ns3/nr-ue-phy.h"
#include <ns3/three-gpp-propagation-loss-model.h>
#include <ns3/three-gpp-spectrum-propagation-loss-model.h>
#include <ns3/three-gpp-channel-model.h>
#include <fstream>
#include <ns3/mobility-helper.h>
#include <chrono>

namespace ns3 {

class Node;
class NetDevice;
class SpectrumChannel;
class MobilityModel;
class MobilityHelper;
class ChannelConditionModel;
class UniformPlanarArray;

/**
 * \brief Generate a radio environment map
 *
 * The purpose of the radio environment map helper is to generate rem maps, where
 * for each point on the map (rem point) a rem value is calculated (SNR/SINR/IPSD).
 *
 * Two general types of maps can be generated according to whether the BeamShape
 * or CoverageArea is selected.
 * The first case considers the configuration of the beamforming vectors (for
 * each transmitting device RTD) as defined by the user in the scenario script
 * for which the (SNR/SINR/IPSD) maps are generated.
 * In the second case, the beams are reconfigured during the map generation for
 * each rem point in order to visualize the coverage area in terms of SNR, SINR
 * and IPSD.
 *
 * The NrRadioEnvironmentMapHelper gives the possibility to generate maps either
 * for the DL or the UL direction. This can be done by passing to the rem helper
 * the desired transmitting device(s) (RTD(s)) and receiving device (RRD), which
 * for the DL case correspond to gNB(s) and UE, while for the UL case to UE(s)
 * and gNB.
 *
 * Moreover, an UL map can be generated to visualize the coverage area of a tx
 * device (UE), while there is the possibility to add interference from DL gNB
 * device(s) to study a worst case mixed FDD-TDD scenario.
 *
 * Let us notice that for the SNR/SINR/IPSD calculations at each REM Point the
 * channel is re-created to avoid spatial and temporal dependencies among
 * independent REM calculations. Moreover, the calculations are the average of
 * N iterations (specified by the user) in order to consider the randomness of
 * the channel
 *
 * For the CoverageArea REM generation the user can include the following code
 * in the desired example script:
 *
 * \code{.unparsed}
$   Ptr<NrRadioEnvironmentMapHelper> remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();
$   remHelper->CreateRem (gnbNetDev, ueNetDev, indexPhy);
    \endcode
 *
 * While for the BeamShape REM generation the user can include the following code:
 *
 * \code{.unparsed}
$  Ptr<NrRadioEnvironmentMapHelper> remHelper = CreateObject<NrRadioEnvironmentMapHelper> ();
$  remHelper->SetRemMode (NrRadioEnvironmentMapHelper::BEAM_SHAPE);
$  gnbNetDev.Get(0)->GetObject<NrGnbNetDevice>()->GetPhy(remBwpId)->GetBeamManager()->ChangeBeamformingVector(ueNetDev.Get(0));
$  remHelper->CreateRem (gnbNetDev, ueRemDevice, indexPhy);
    \endcode
 *
 * Please refer to the rest parameters of the REM map that can be set
 * through the command line (e.g. x, y, z coordinates and resolution)
 *
 * The output of the NrRadioEnvironmentMapHelper are REM csv files from which
 * the REM figures can be generated with the following command:
 * \code{.unparsed}
$  gnuplot -p nr-rem-SimTag-gnbs.txt nr-rem-SimTag-ues.txt nr-rem-SimTag-buildings.txt nr-rem-SimTag-plot-rem.gnuplot
    \endcode
 */


class NrRadioEnvironmentMapHelper : public Object
{
public:

  enum RemMode {
         BEAM_SHAPE,
         COVERAGE_AREA,
         UE_COVERAGE
  };

  /**
   * \brief NrRadioEnvironmentMapHelper constructor
   */
  NrRadioEnvironmentMapHelper ();

  /**
   * \brief destructor
   */
  virtual ~NrRadioEnvironmentMapHelper ();


  // inherited from Object
  virtual void DoDispose (void);


  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Set the type of REM Map to be generated
   * \param remType the desired type (BeamShape/CoverageArea/UeCoverage)
   */
  void SetRemMode (enum RemMode remType);

  /**
   * \brief Set simTag that will be contatenated to 
   * output file names
   * \param simTag string to be used as simulation tag
   */
  void SetSimTag (const std::string &simTag);

  /**
   * \brief Sets the min x coordinate of the map
   * \param xMin The min x coordinate
   */
  void SetMinX (double xMin);

  /**
   * \brief Sets the min y coordinate of the map
   * \param yMin The min y coordinate
   */
  void SetMinY (double yMin);

  /**
   * \brief Sets the max x coordinate of the map
   * \param xMax The max x coordinate
   */
  void SetMaxX (double xMax);

  /**
   * \brief Sets the max y coordinate of the map
   * \param yMax The max y coordinate
   */
  void SetMaxY (double yMax);

  /**
   * \brief Sets the resolution (number of points) of the map along the x axis
   * \param xRes The x axis resolution
   */
  void SetResX (uint16_t xRes);

  /**
   * \brief Sets the resolution (number of points) of the map along the y axis
   * \param yRes The y axis resolution
   */
  void SetResY (uint16_t yRes);

  /**
   * \brief Sets the z coordinate of the map
   * \param z The z coordinate
   */
  void SetZ (double z);

  /**
   * \brief Sets the number of iterations to calculate the average of rem value
   * \param numOfIterationsToAverage The number of iterations
   */
  void SetNumOfItToAverage (uint16_t numOfIterationsToAverage);

  /**
   * \brief Sets the installation delay
   * \param installationDelay delay for the REM installation
   */
  void SetInstallationDelay (const Time &installationDelay);

  /**
   * \brief Get the type of REM Map to be generated
   * \return The type of the map (BeamShape/CoverageArea/UeCoverage)
   */
  RemMode GetRemMode () const;

  /**
   * \return Gets the value of the min x coordinate of the map
   */
  double GetMinX () const;

  /**
   * \return Gets the value of the min y coordinate of the map
   */
  double GetMinY () const;

  /**
   * \return Gets the value of the max x coordinate of the map
   */
  double GetMaxX () const;

  /**
   * \return Gets the value of the max y coordinate of the map
   */
  double GetMaxY () const;

  /**
   * \return Gets the value of the resolution (number of points)
   * of the map along the x axis
   */
  uint16_t GetResX () const;

  /**
   * \return Gets the value of the resolution (number of points)
   * of the map along the y axis
   */
  uint16_t GetResY () const;

  /**
   * \return Gets the value of the z coordinate of the map
   */
  double GetZ () const;

  /**
   * \brief Convert from Watts to dBm.
   * \param w the power in Watts
   * \return the equivalent dBm for the given Watts
   */
  double WToDbm (double w) const;

  /**
   * \brief Convert from dBm to Watts.
   * \param dbm the power in dBm
   * \return the equivalent Watts for the given dBm
   */
  double DbmToW (double dbm) const;

  /**
   * \brief Convert from dB to ratio.
   * \param dB the value in dB
   * \return ratio in linear scale
   */
  double DbToRatio (double dB) const;

  /**
   * \brief Convert from ratio to dB.
   * \param ratio the ratio in linear scale
   * \return the value in dB
   */
  double RatioToDb (double ratio) const;

  /**
   * \brief This function is used for the creation of the REM map. When this
   * function is called from an example, it is responsible for "installing"
   * the REM through a callback to the DelayedInstall after a delay of
   * installationDelay. Then the DelayedInstall takes care to perform all
   * the necessary actions (call the necessary functions).
   * \param rtdNetDev The transmitting device(s) for which the map will be generated
   * \param rrdDevice The receiving device for which the map will be generated
   * \param bwpId The bwpId
   */
  void CreateRem (const NetDeviceContainer &rtdNetDev,
                  const Ptr<NetDevice> &rrdDevice, uint8_t bwpId);

private:

  /**
   * \brief This struct includes the coordinates of each Rem Point
   * and the SNR/SINR/IPSD values as resulted from the calculations
   */
  struct RemPoint
  {
    Vector pos {0,0,0};
    double avgSnrDb {0};
    double avgSinrDb {0};
    double avgSirDb {0};
    double avRxPowerDbm {0};
  };

  /**
   * \brief This struct includes the configuration of all the devices of
   * the REM: Rem Transmitting Devices (RTDs) and Rem Receiving Device (RRD)
   */
  struct RemDevice
  {
    Ptr<Node> node;
    Ptr<SimpleNetDevice> dev;
    Ptr<MobilityModel> mob;
    Ptr<UniformPlanarArray> antenna;
    double txPower {0};
    double bandwidth {0};
    double frequency {0};
    uint16_t numerology {0};
    Ptr<const SpectrumModel> spectrumModel {};

    RemDevice ()
    {
      node = CreateObject<Node> ();
      dev = CreateObject<SimpleNetDevice> ();
      node->AddDevice (dev);
      MobilityHelper mobility;
      mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
      mobility.Install (node);

      mob = node->GetObject<MobilityModel> ();
    }
  };

  /**
   * \brief This struct includes the pointers that copy the propagation
   * Loss Model and Spectrum Propagation Loss model (from the example used
   * to generate the REM map)
   */
  struct PropagationModels
  {
    Ptr<ThreeGppPropagationLossModel> remPropagationLossModelCopy;
    Ptr<ThreeGppSpectrumPropagationLossModel> remSpectrumLossModelCopy;
  };

  /**
   * \brief This method creates the list of Rem Points (coordinates) based on
   * the min/max coprdinates and the resolution defined by the user
   */
  void CreateListOfRemPoints ();

  /**
   * \brief Configures the REM Receiving Device (RRD)
   */
  void ConfigureRrd (const Ptr<NetDevice> &rrdDevice);

  /**
   * \brief Configure REM Transmission Devices (RTDs) List
   * \param rtdDevs NetDeviceContainer of the transmitting objects for whose
   * transmissions will be created this REM map
   */
  void ConfigureRtdList (const NetDeviceContainer& rtdDevs);

  /**
   * \brief This function is used to performed a delayed installation of REM map
   * so that there is the sufficient time for the UE to be configured from RRC.
   * Then, this function is responsible to call all the necessary functions.
   * \param rtdNetDev The container of the Tx devices for which the map will be generated
   * \param rrdDevice The Rx device for which the map will be generated
   */
   void DelayedInstall (const NetDeviceContainer &rtdNetDev,
                        const Ptr<NetDevice> &rrdDevice);

  /**
   * Function that saves all antenna configurations in a map. This is
   * done at the installation time to pick up also
   * user defined configuration of beams.
   */
  void SaveAntennasWithUserDefinedBeams (const NetDeviceContainer &rtdNetDev,
                                         const Ptr<NetDevice> &rrdDevice);

  /**
   * \brief This function generates a BeamShape map. Using the configuration
   * of antennas as have been set in the user scenario script, it calculates
   * the SNR/SINR/IPSD.
   */
  void CalcBeamShapeRemMap ();

  /**
   * \brief This function generates a CoverageArea map. In this case, all the
   * antennas of the rtds are set to point towards the rem point and the antenna
   * of the rem point towards each rtd device.
   */
  void CalcCoverageAreaRemMap ();

  /**
   * \brief This function generates a Ue Coverage map that depicts the SNR of
   * this UE with respect to its UL transmission towards the gNB form various
   * points on the map.
   * An additional SINR map is also generated that can be used in mixed TDD/FDD
   * scenarios considering interference from neighbor gNBs that transmit in DL.
   */
  void CalcUeCoverageRemMap ();

  /**
   * \brief This method calculates the PSD
   * \return The PSD (spectrumValue)
   */
  Ptr<SpectrumValue> CalcRxPsdValue (RemDevice& device, RemDevice& otherDevice) const;

  /**
   * \brief This function calculates the SNR.
   * \param usefulSignal The useful Signal
   * \return The snr
   */
  double CalculateSnr (const Ptr<SpectrumValue>& usefulSignal) const;

  /**
   * \brief This function finds the max value in a space of frequency-dependent
   * values (such as PSD).
   * \param values The list of spectrumValues for which we want to find the max
   * \return The max spectrumValue
   */
  Ptr<SpectrumValue> GetMaxValue(const std::list <Ptr<SpectrumValue>>& values) const;

  /**
   * \brief This function finds the max value in a space of frequency-dependent
   * values (such as PSD).
   * \param values The list of spectrumValues for which we want to find the max
   * \return The max value (snr)
   */
  double CalculateMaxSnr (const std::list <Ptr<SpectrumValue>>& receivedPowerList) const;

  /**
   * \brief This function finds the max value in a space of frequency-dependent
   * values (such as PSD).
   * \param values The list of spectrumValues for which we want to find the max
   * \return The max value (sinr)
   */
  double CalculateMaxSinr (const std::list <Ptr<SpectrumValue>>& receivedPowerList) const;

  /**
   * \brief This function finds the max value in a space of frequency-dependent
   * values (such as PSD).
   * \param values The list of spectrumValues for which we want to find the max
   * \return The max value (sinr)
   */
  double CalculateMaxSir (const std::list <Ptr<SpectrumValue>>& receivedPowerList) const;

  /**
   * \brief This function calculates the SINR for a given space of frequency-dependent
   * values (such as PSD).
   * \param usefulSignal The spectrumValue considered as useful signal
   * \param interferenceSignals The list of spectrumValues considered as interference
   * \return The max value (sinr)
   */
  double CalculateSinr (const Ptr<SpectrumValue>& usefulSignal,
                        const std::list <Ptr<SpectrumValue>>& interferenceSignals) const;

  /**
   * \brief This function calculates the SIR for a given space of frequency-dependent
   * values (such as PSD).
   * \param usefulSignal The spectrumValue considered as useful signal
   * \param interferenceSignals The list of spectrumValues considered as interference
   * \return The max value (sir)
   */
  double CalculateSir (const Ptr<SpectrumValue>& usefulSignal,
                       const std::list <Ptr<SpectrumValue>>& interferenceSignals) const;

  /**
   * \brief This function finds the max value in a list of double values.
   * \param values The list of double values
   * \return The max value
   */
  double GetMaxValue (const std::list<double>& listOfValues) const;

  /**
   * \brief This function returns the integral of the sum of the elements of a
   * list of SpectrumValues
   * \return The integral of the sum of the elements of the list
   */
  double CalculateAggregatedIpsd (const std::list <Ptr<SpectrumValue>>& interferenceSignals);

  /**
   * \brief This function returns the sum of the elements of a list of double values
   * \return The sum of the elements of the list
   */
  double SumListElements (const std::list<double>& listOfValues);

  /**
   * \brief Configures propagation loss model factories
   */
  void ConfigurePropagationModelsFactories (const Ptr<const NrPhy>& rtdPhy);

  /**
   * \brief Configures the object factories with the parameters set in the
   * user scenario script.
   * \return Configured ObjectFactory instance
   */
  ObjectFactory ConfigureObjectFactory (const Ptr<Object>& object) const;

  /**
   * \brief This method creates the temporal Propagation Models
   * \return The struct with the temporal propagation models (created for each
   * rem point)
   */
  PropagationModels CreateTemporalPropagationModels () const;

  /**
   * \brief Prints REM generation progress report
   */
  void PrintProgressReport (uint32_t* remSizeNextReport);

  /**
   * \brief Prints the position of the RTDs.
   */
  void PrintGnuplottableGnbListToFile (const std::string &filename);

  /**
   * \brief Print the position of the RRD.
   */
  void PrintGnuplottableUeListToFile (const std::string &filename);

  /**
   * \brief Print the position of the Buildings.
   */
  void PrintGnuplottableBuildingListToFile (const std::string &filename);

  /**
   * \brief this method goes through every Rem Point and prints the
   * calculated SNR/SINR/IPSD values.
   */
  void PrintRemToFile ();

  /*
   * Creates rem_plot${SimTag}.gnuplot file
   */
  void CreateCustomGnuplotFile ();

  /**
   * \brief Called when the map generation procedure has been completed.
   */
  void Finalize ();

  /**
   * \brief Configures quasi-omni beamforming vector on antenna of the device
   * \param device which antenna array will be configured to quasi-omni beamforming vector
   */
  void ConfigureQuasiOmniBfv (RemDevice& device);

  /**
   * \brief Configures direct-path beamforming vector of "device" toward "otherDevice"
   * \param device whose beamforming vector will be configured
   * \param otherDevice toward this device will be configured the beamforming vector of device
   * \param antenna of the first device
   */
  void ConfigureDirectPathBfv (RemDevice& device, const RemDevice& otherDevice,
                               const Ptr<const UniformPlanarArray>& antenna);

  std::list<RemDevice> m_remDev; ///< List of REM Transmiting Devices (RTDs).
  std::list<RemPoint> m_rem; ///< List of REM points.

  std::chrono::system_clock::time_point m_remStartTime; //!< Time at which REM generation has started

  enum RemMode m_remMode;

  double m_xMin {0};   ///< The `XMin` attribute.
  double m_xMax {0};   ///< The `XMax` attribute.
  uint16_t m_xRes {0}; ///< The `XRes` attribute.
  double m_xStep {0};  ///< Distance along X axis between adjacent listening points.

  double m_yMin {0};   ///< The `YMin` attribute.
  double m_yMax {0};   ///< The `YMax` attribute.
  uint16_t m_yRes {0}; ///< The `YRes` attribute.
  double m_yStep {0};  ///< Distance along Y axis between adjacent listening points.
  double m_z {0};  ///< The `Z` attribute.

  uint16_t m_numOfIterationsToAverage {1};
  Time m_installationDelay {Seconds(0)};

  RemDevice m_rrd;

  Ptr<NrPhy> m_rrdPhy;  ///< Pointer to the phy of the RRD
  std::map <const Ptr<NetDevice>, Ptr<NrPhy>> m_rtdDeviceToPhy;     ///< Map for storing the phy of each RTD device
  std::map <const Ptr<NetDevice>, Ptr<UniformPlanarArray>> m_deviceToAntenna;

  Ptr<PropagationLossModel> m_propagationLossModel;
  Ptr<PhasedArraySpectrumPropagationLossModel> m_phasedArraySpectrumLossModel;
  ObjectFactory m_channelConditionModelFactory;
  ObjectFactory m_matrixBasedChannelModelFactory;

  Ptr<SpectrumValue> m_noisePsd; // noise figure PSD that will be used for calculations

  std::string m_simTag;   ///< The `SimTag` attribute.

}; // end of `class NrRadioEnvironmentMapHelper`

} // end of `namespace ns3`


#endif // NR_RADIO_ENVIRONMENT_MAP_HELPER_H

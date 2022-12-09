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

#ifndef SRC_NR_MODEL_REALISTIC_BEAMFORMING_ALGORITHM_H_
#define SRC_NR_MODEL_REALISTIC_BEAMFORMING_ALGORITHM_H_

#include <ns3/object.h>
#include "beam-id.h"
#include "nr-spectrum-phy.h"
#include "ns3/three-gpp-channel-model.h"
#include "realistic-bf-manager.h"
#include "nr-ue-net-device.h"
#include "nr-gnb-net-device.h"
#include <queue>

namespace ns3 {

class SpectrumModel;
class SpectrumValue;
class RealisticBeamformingHelper;
class NrRealisticBeamformingTestCase;

/**
 * \ingroup gnb-phy
 * \brief Generate "Real" beamforming vectors
 * This class is inherited by all algorithms that do not assume the
 * perfect knowledge of the channel, but instead are performing the
 * estimation of the channel based on measurements, e.g., based on
 * SRS SINR/SNR measurement.
 *
 * RealisticBeamformingAlgorithm purpose is to generate beams for the pair
 * of communicating devices based on the SRS measurements. Differently from
 * IdealBeamformingAlgorithm this type of algorithm does not assume a perfect
 * knowledge of the channel. It instead estimates the long-term fast fading
 * channel component based on the received SRS. Accordingly, this approach
 * could be used with any beamforming algorithm that makes use of the channel
 * estimation, e.g., beam search method (e.g., such as the one implemented in
 * CellScanBeamforming class). Note that the LOS type of method (e.g., such as
 * the one implemented in DirectPathBeamforming class) does not use the
 * channel matrix, but instead the angles of arrival and departure of the LOS
 * path, and so, the proposed method is not valid for it. Currently, it is
 * only compatible with the beam search method."
 */
class RealisticBeamformingAlgorithm: public Object
{

  friend RealisticBeamformingHelper;
  friend NrRealisticBeamformingTestCase;

public:

  /*
   * \brief The structure that contains the information about the update time,
   * srsSinr and the channel matrix.
   */
  struct DelayedUpdateInfo
  {
    Time updateTime; //!< time that will be used to check if the event is using the correct SRS measurement and channel
    double srsSinr;  //!< SRS SINR/SNR value
    Ptr<const MatrixBasedChannelModel::ChannelMatrix> channelMatrix; //!< saved deep copy of the channel matrix at the time instant when the SRS is received
  };

  /*
   * \brief The structure that contains the information about what is the trigger
   * of the realistic beamforming algorithm, and the periodicity or the delay.
   */
  struct TriggerEventConf
  {
    RealisticBfManager::TriggerEvent event;
    uint16_t updatePeriodicity;
    Time updateDelay;
  };

  /**
   * \brief constructor
   */
  RealisticBeamformingAlgorithm ();
  /*
   * \brief It is necessary to call this function in order to have
   * initialized a pair of gNB and UE devices for which will be
   * called this algorithm. And also the ccId.
   * \param gnbDevice gNB instance of devicePair for which will work this algorithm
   * \param ueDevice UE instance of devicePair for which will work this algorithm
   * \param gnbSpectrumPhy the spectrum phy instance of the gNB
   * \param ueSpectrumPhy the spectrum phy of the UE
   * \param scheduler the pointer to the MAC scheduler to obtain the number of
   * SRS symbols
   */
  void Install (const Ptr<NrGnbNetDevice>& gnbDevice,
                const Ptr<NrUeNetDevice>& ueDevice,
                const Ptr<NrSpectrumPhy> & gnbSpectrumPhy,
                const Ptr<NrSpectrumPhy>& ueSpectrumPhy,
                const Ptr<NrMacScheduler>& scheduler);
  /**
   * \brief destructor
   */
  virtual ~RealisticBeamformingAlgorithm ();
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);
  /**
   * \brief Assign a fixed random variable stream number to the random variables
   * used by this model. Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream the first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);
  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices by using the direct-path beamforming vector for gNB
   * and quasi-omni beamforming vector for UEs
   * \return the gNB and UE beamforming vectors
   */
  virtual BeamformingVectorPair GetBeamformingVectors ();
  /**
   * \return Gets value of BeamSearchAngleStep attribute
   */
  double GetBeamSearchAngleStep () const;
  /**
   * \brief Sets the value of BeamSearchAngleStep attribute
   * \param beamSearchAngleStep the beam search angle step value
   */
  void SetBeamSearchAngleStep (double beamSearchAngleStep);
  /**
   * \brief Saves SRS SINR report
   * \param cellId the cell ID
   * \param rnti the RNTI of the UE
   * \param srsSinr the SINR report for the received SRS
   */
  void NotifySrsSinrReport (uint16_t cellId, uint16_t rnti, double srsSinr);
  /**
   * \brief Saves SRS SNR report
   * \param cellId the cell ID
   * \param rnti the RNTI
   * \param srsSnr the SRS SNR report
   */
  void NotifySrsSnrReport (uint16_t cellId, uint16_t rnti, double srsSnr);
  /**
   * \brief Saves SRS report (SNR or SINR depending on the configuration)
   * \param cellId the cell ID
   * \param rnti the RNTI
   * \param srsReport the SRS report which can be SNR or SIN depending on the configuration
   */
  void NotifySrsReport (uint16_t cellId, uint16_t rnti, double srsReport);
  /**
   * \brief RunTask callback will be triggered when the event for updating the beamforming vectors occurs
   * The parameters are: gnb device, ue device, gnb spectrum phy, ue spectrum phy.
   */
  typedef Callback<void,
                   const Ptr<NrGnbNetDevice>&,
                   const Ptr<NrUeNetDevice>&,
                   const Ptr<NrSpectrumPhy>&,
                   const Ptr<NrSpectrumPhy>&> RealisticBfHelperCallback;
  /*
   * \brief Set whether to use SRS SNR report
   * \bool v boolean indicator, if true then SRS SNR report will be used
   */
  void SetUseSnrSrs (bool v);
  /*
   * \brief Get whether the algorithm uses SRS SNR report
   * \return the boolean indicator indicating whether SRS SNR is used
   */
  bool UseSnrSrs () const;

private:

  /**
   * \brief Private function that is used to obtain the number of SRS symbols per slot
   * \return the number of SRS symbols per slot
   */
  uint8_t GetSrsSymbolsPerSlot ();

  /**
   * \brief Private function that is used to obtain the realistic beamforming configuration from the
   * realistic beamforming manager
   * \return returns the realistic beamforming configuration
   */
  RealisticBeamformingAlgorithm::TriggerEventConf GetTriggerEventConf () const;

  /**
   * \brief Private function that is used to notify its the helper that is time
   * to update beamforming vectors.
   * Basically, with this function realistic algoritm pass control to the
   * realistic beamforming helper in the sense of calling necessary BF updates, e.g.
   * calling BeamManager's function, etc. because we are trying to decouple responsabilities
   * of the algorithm, which should be only to provide the best beamforming vector pair for
   * two communicating devices, and beamforming helper to take care of managing the necessary
   * updates.
   */
  void NotifyHelper ();

  /*
   * \brief Sets RealisticBeamformingHelperCallback that will be notified when it is necessary to update
   * the beamforming vectors. Function RunTask will then call back RealisticBeamformingAlgorithm that
   * notified it about the necessity to update the beamforming vectors. It is done in this way, in order
   * to split functionalities and responsibilities of the BF helper class, and BF algorithm class.
   * BF helper class takes care of necessary BF vector updates, and necessary calls of BeamManager class.
   * While BF algorithm class takes care of trigger event, parameters, and algorithm, but it is not
   * responsible to update the beamforming vector of devices.
   * param callback the realistic beamforming helper callback
   */
  void SetTriggerCallback (RealisticBfHelperCallback callback);
  /**
   * \brief Gets the channel matrix between gNb and UE device of this algorithm
   * This is needed when delayed trigger event is used and delay is larger then SRS periodicity,
   * so there can be various SRS reports and corresponding channel matrices for which
   * will be nececessary to perform bemaforming update using channel matrix corresponding to
   * the time of the reception of SRS.
   * \return returns a deep copy of the current channel matrix
   */
  Ptr<const MatrixBasedChannelModel::ChannelMatrix> GetChannelMatrix () const;
  /**
   * \brief Calculates an estimation of the long term component based on the channel measurements
   * \param channelMatrix the channel matrix H
   * \param aW the beamforming vector of the first device
   * \param bW the beamforming vector of the second device
   * \param a the first node mobility model
   * \param b the second node mobility model
   * \param srsSinr the SRS report to be used to estimate the long term component metric
   * \param aArray the antenna array of the first device
   * \param bArray the antenna array of the second device
   * \return the estimated long term component
   */
  UniformPlanarArray::ComplexVector GetEstimatedLongTermComponent (const Ptr<const MatrixBasedChannelModel::ChannelMatrix>& channelMatrix,
                                                                          const UniformPlanarArray::ComplexVector &aW,
                                                                          const UniformPlanarArray::ComplexVector &bW,
                                                                          Ptr<const MobilityModel> a,
                                                                          Ptr<const MobilityModel> b,
                                                                          double srsSinr,
                                                                          Ptr<const PhasedArrayModel> aArray,
                                                                          Ptr<const PhasedArrayModel> bArray) const;

  /*
   * \brief Calculates the total metric based on the each element of the long term component
   * \param longTermComponent the vector of complex numbers representing the long term component per cluster
   */
  double CalculateTheEstimatedLongTermMetric (const UniformPlanarArray::ComplexVector& longTermComponent) const;

  /**
   * \brief Removes the "oldest" delayed update info - from the beggining of the queue
   */
  void RemoveUsedDelayedUpdateInfo () ;

  // attribute members, configuration variables
  double m_beamSearchAngleStep {30}; //!< The beam angle step that will be used to define the set of beams for which will be estimated the channel
  bool m_useSnrSrs  {true};          //!< SRS SNR used as measurement (attribute)
  //variable members, counters, and saving values
  double m_maxSrsSinrPerSlot {0}; //!< the maximum SRS SINR/SNR per slot in Watts, e.g. if there are 4 SRS symbols per UE, this value will represent the maximum
  std::queue <DelayedUpdateInfo> m_delayedUpdateInfo; //!< the vector of SRS SINRs/SNRs and saved channel matrices, needed for when trigger event update is based on delay
  uint8_t m_srsSymbolsCounter {0}; //!< the counter that gets reset after reaching the number of symbols per SRS transmission
  uint16_t m_srsPeriodicityCounter {0}; //!< the counter of SRS reports between consecutive beamforming updates, this counter is incremented once the counter
                                        //   m_srsSymbolsPerSlotCounter reaches the number of symbols per SRS transmission, i.e., when SRS transmissions in the
                                        //   current slot have finished*/
  Ptr<NormalRandomVariable> m_normalRandomVariable; //!< The random variable used for the estimation of the error
  RealisticBfHelperCallback m_helperCallback; //!< When it is necessary to update the beamforming vectors for this pair of devices,
                                              //the helper will be notified through this callback
  /*
   * \brief Parameters needed to pass to helper once that the helpers callback functions is being called
   */
  Ptr<NrGnbNetDevice> m_gnbDevice; //!< pointer to gNB device
  Ptr<NrUeNetDevice> m_ueDevice;  //!< pointer to UE device 
  Ptr<NrSpectrumPhy> m_gnbSpectrumPhy; //!< pointer to gNB spectrum phy
  Ptr<NrSpectrumPhy> m_ueSpectrumPhy;  //!< pointer to UE spectrum phy
  Ptr<NrMacScheduler> m_scheduler; //!< pointer to gNB MAC scheduler

};

} // end of namespace ns-3
#endif

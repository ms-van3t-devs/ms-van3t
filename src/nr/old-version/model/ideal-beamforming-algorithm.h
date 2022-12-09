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

#ifndef SRC_NR_MODEL_IDEAL_BEAMFORMING_ALGORITHM_H_
#define SRC_NR_MODEL_IDEAL_BEAMFORMING_ALGORITHM_H_

#include <ns3/object.h>
#include "beam-id.h"
#include "beamforming-vector.h"
#include "beamforming-algorithm.h"

namespace ns3 {

class SpectrumModel;
class SpectrumValue;
class NrGnbNetDevice;
class NrUeNetDevice;

/**
 * \ingroup gnb-phy
 * \brief Generate "Ideal" beamforming vectors
 *
 * IdealBeamformingAlgorithm purpose is to generate beams for the pair
 * of communicating devices.
 *
 * Algorithms that inherit this class assume a perfect knowledge of the channel,
 * because of which this group of algorithms is called "ideal".
 */
class IdealBeamformingAlgorithm: public BeamformingAlgorithm
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);
};

/**
 * \ingroup gnb-phy
 * \brief The CellScanBeamforming class
 */
class CellScanBeamforming: public IdealBeamformingAlgorithm
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \return Gets value of BeamSearchAngleStep attribute
   */
  double GetBeamSearchAngleStep () const;

  /**
   * \brief Sets the value of BeamSearchAngleStep attribute
   */
  void SetBeamSearchAngleStep (double beamSearchAngleStep);

  /**
   * \brief constructor
   */
  CellScanBeamforming () = default;

  /**
   * \brief destructor
   */
  virtual ~CellScanBeamforming () override = default;

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices by using cell scan method
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                      const Ptr<const NrUeNetDevice>& ueDev,
                                      BeamformingVector* gnbBfv,
                                      BeamformingVector* ueBfv,
                                      uint16_t ccId) const override;

private:

  double m_beamSearchAngleStep {30};

};

/**
 * \ingroup gnb-phy
 * \brief The CellScanQuasiOmniBeamforming class
 */
class CellScanQuasiOmniBeamforming: public IdealBeamformingAlgorithm
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \return Gets value of BeamSearchAngleStep attribute
   */
  double GetBeamSearchAngleStep () const;

  /**
   * \brief Sets the value of BeamSearchAngleStep attribute
   */
  void SetBeamSearchAngleStep (double beamSearchAngleStep);

  /**
   * \brief constructor
   */
  CellScanQuasiOmniBeamforming () = default;

  /**
   * \brief destructor
   */
  virtual ~CellScanQuasiOmniBeamforming () override = default;

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices by using cell scan method at gnbDev and a fixed quasi-omni beamforming vector at UE
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                      const Ptr<const NrUeNetDevice>& ueDev,
                                      BeamformingVector* gnbBfv,
                                      BeamformingVector* ueBfv,
                                      uint16_t ccId) const override;

private:

  double m_beamSearchAngleStep {30};

};

/**
 * \ingroup gnb-phy
 * \brief The DirectPathBeamforming class
 */
class DirectPathBeamforming: public IdealBeamformingAlgorithm
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices by using the direct path direction
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                      const Ptr<const NrUeNetDevice>& ueDev,
                                      BeamformingVector* gnbBfv,
                                      BeamformingVector* ueBfv,
                                      uint16_t ccId) const override;
};

/**
 * \ingroup gnb-phy
 * \brief The QuasiOmniDirectPathBeamforming class
 */
class QuasiOmniDirectPathBeamforming: public DirectPathBeamforming
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices by using the quasi omni beamforming vector for gNB
   * and direct path beamforming vector for UEs
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                      const Ptr<const NrUeNetDevice>& ueDev,
                                      BeamformingVector* gnbBfv,
                                      BeamformingVector* ueBfv,
                                      uint16_t ccId) const override;

};


/**
 * \ingroup gnb-phy
 * \brief The QuasiOmniDirectPathBeamforming class
 */
class DirectPathQuasiOmniBeamforming: public DirectPathBeamforming
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices by using the direct-path beamforming vector for gNB
   * and quasi-omni beamforming vector for UEs
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                      const Ptr<const NrUeNetDevice>& ueDev,
                                      BeamformingVector* gnbBfv,
                                      BeamformingVector* ueBfv,
                                      uint16_t ccId) const override;

};


/**
 * \ingroup gnb-phy
 * \brief The OptimalCovMatrixBeamforming class not implemented yet.
 * TODO The idea was to port one of the initial beamforming methods that
 * were implemented in NYU/University of Padova mmwave module.
 * Method is based on a long term covariation matrix.
 */
class OptimalCovMatrixBeamforming : public IdealBeamformingAlgorithm
{

public:

  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Function that generates the beamforming vectors for a pair of
   * communicating devices by using the direct-path beamforming vector for gNB
   * and quasi-omni beamforming vector for UEs
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                      const Ptr<const NrUeNetDevice>& ueDev,
                                      BeamformingVector* gnbBfv,
                                      BeamformingVector* ueBfv,
                                      uint16_t ccId) const override;
};


} // end of ns3 namespace
#endif

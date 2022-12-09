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

#ifndef SRC_NR_MODEL_BEAMFORMING_ALGORITHM_H_
#define SRC_NR_MODEL_BEAMFORMING_ALGORITHM_H_

#include <ns3/object.h>
#include "beamforming-vector.h"

namespace ns3 {

class SpectrumModel;
class SpectrumValue;
class NrGnbNetDevice;
class NrUeNetDevice;

/**
 * \ingroup gnb-phy
 * \brief Generate "Ideal" beamforming vectors
 *
 * BeamformingAlgorithm purpose is to generate beams for the pair
 * of communicating devices.
 */
class BeamformingAlgorithm: public Object
{

public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief constructor
   */
  BeamformingAlgorithm ();

  /**
   * \brief destructor
   */
  virtual ~BeamformingAlgorithm ();

  /**
   * \brief Function that generates the beamforming vectors for a pair of communicating devices
   * \param [in] gnbDev gNb beamforming device
   * \param [in] ueDev UE beamforming device
   * \param [out] gnbBfv the best beamforming vector for gNbDev device antenna array to communicate with ueDev according to this algorithm criteria
   * \param [out] ueBfv the best beamforming vector for ueDev device antenna array to communicate with gNbDev device according to this algorithm criteria
   */
  virtual void GetBeamformingVectors (const Ptr<const NrGnbNetDevice>& gnbDev,
                                      const Ptr<const NrUeNetDevice>& ueDev,
                                      BeamformingVector* gnbBfv,
                                      BeamformingVector* ueBfv,
                                      uint16_t ccId) const = 0;
};


} // end of ns3 namespace
#endif

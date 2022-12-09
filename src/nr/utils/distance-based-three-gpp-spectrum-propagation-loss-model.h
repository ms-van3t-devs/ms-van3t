/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 */

#ifndef DISTANCE_BASED_THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H
#define DISTANCE_BASED_THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H

#include "ns3/three-gpp-spectrum-propagation-loss-model.h"

namespace ns3 {

/**
 * \ingroup nr-utils
 * \brief Distance based 3GPP Spectrum Propagation Loss Model
 *
 * This class inherits ThreeGppSpectrumPropagationLossModel
 * and calculates the fading and beamforming only for the signals
 * being transmitted among nodes whose distance is lower than the
 * max allowed distance that can be configured
 * through the attribute of this class.
 *
 * \see ThreeGppSpectrumPropagationLossModel
 */
class DistanceBasedThreeGppSpectrumPropagationLossModel : public ThreeGppSpectrumPropagationLossModel
{
public:
  /**
   * Constructor
   */
  DistanceBasedThreeGppSpectrumPropagationLossModel ();

  /**
   * Destructor
   */
  virtual ~DistanceBasedThreeGppSpectrumPropagationLossModel ();
  
  /**
   * Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /*
   * \brief Sets the max distance
   */
  void SetMaxDistance (double maxDistance);

  /*
   * \brief Gets the configured max distance
   */
  double GetMaxDistance () const;

  /**
   * \brief Computes the received PSD.
   *
   * This function computes the received PSD by applying the 3GPP fast fading
   * model and the beamforming gain. However, if the distance between a and b
   * is higher than allowe this class will return 0 PSD.
   *
   * \param txPsd tx PSD
   * \param a first node mobility model
   * \param b second node mobility model
   * \param aPhasedArrayModel the antenna array of the first node
   * \param bPhasedArrayModel the antenna array of the second node
   * \return the received PSD
   */
  virtual Ptr<SpectrumValue> DoCalcRxPowerSpectralDensity (Ptr<const SpectrumValue> txPsd,
                                                           Ptr<const MobilityModel> a,
                                                           Ptr<const MobilityModel> b,
                                                           Ptr<const PhasedArrayModel> aPhasedArrayModel,
                                                           Ptr<const PhasedArrayModel> bPhasedArrayModel) const override;

private:

  double m_maxDistance {1000}; //!< the maximum distance of the nodes a and b in order to calcluate fast fading and the beamforming gain
};
} // namespace ns3

#endif /* THREE_GPP_SPECTRUM_PROPAGATION_LOSS_H */

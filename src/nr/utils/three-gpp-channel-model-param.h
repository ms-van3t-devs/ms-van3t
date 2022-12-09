/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering,
 * New York University
 * Copyright (c) 2019 SIGNET Lab, Department of Information Engineering,
 * University of Padova
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

#ifndef THREE_GPP_CHANNEL_PARAM_H
#define THREE_GPP_CHANNEL_PARAM_H

#include  <complex.h>
#include "ns3/angles.h"
#include <ns3/object.h>
#include <ns3/nstime.h>
#include <ns3/random-variable-stream.h>
#include <ns3/boolean.h>
#include <unordered_map>
#include <ns3/channel-condition-model.h>
#include <ns3/three-gpp-channel-model.h>

namespace ns3 {

class MobilityModel;

/**
 * \ingroup spectrum
 * \brief Channel Matrix Generation following 3GPP TR 38.901
 *
 * The class implements the channel matrix generation procedure
 * described in 3GPP TR 38.901.
 *
 * \see GetChannel
 */
class ThreeGppChannelModelParam : public ThreeGppChannelModel
{
public:
  /**
   * Constructor
   */
  ThreeGppChannelModelParam ();

  /**
   * Destructor
   */
  ~ThreeGppChannelModelParam ();
  
  void DoDispose () override;

  /**
   * Get the type ID
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  void SetRo (double ro);

  
private:

  /**
   * Compute the channel matrix between two devices using the procedure
   * described in 3GPP TR 38.901
   * \param channelParams the channel parameters
   * \param table3gpp the 3gpp parameters table
   * \param sMob the mobility model of node s
   * \param uMob the mobility model of node u
   * \param sAntenna the antenna array of node s
   * \param uAntenna the antenna array of node u
   * \return the channel realization
   */
  virtual Ptr<ChannelMatrix> GetNewChannel (Ptr<const ThreeGppChannelParams> channelParams,
                                            Ptr<const ParamsTable> table3gpp,
                                            const Ptr<const MobilityModel> sMob,
                                            const Ptr<const MobilityModel> uMob,
                                            Ptr<const PhasedArrayModel> sAntenna,
                                            Ptr<const PhasedArrayModel> uAntenna) const override;

  double m_Ro {1.0}; //!< cross polarization correlation parameter
  double m_parametrizedCorrelation {true}; //!< whether the parameter Ro will be used as correlation term
};
} // namespace ns3

#endif /* THREE_GPP_CHANNEL_PARAM_H */

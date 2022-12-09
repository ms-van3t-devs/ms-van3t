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

#include <ns3/object.h>
#include <ns3/vector.h>
#include <ns3/object-factory.h>
#include <ns3/beamforming-vector.h>

#ifndef SRC_NR_HELPER_BEAMFORMING_HELPER_BASE_H_
#define SRC_NR_HELPER_BEAMFORMING_HELPER_BASE_H_

namespace ns3 {

class NrSpectrumPhy;
class NrGnbNetDevice;
class NrUeNetDevice;

/**
 * \ingroup helper
 * \brief The BeamformingHelperBase class that is being
 * used as the general interface for beamforming helper
 * classes. Currently, there are two beamforming helper classes:
 * `IdealBeamformingHelper` and `RealisticBeamformingHelper`
 * that inherit this base beamforming helper class
 */
class BeamformingHelperBase : public Object
{
public:
  /**
   * \brief BeamformingHelperBase constructor
   */
  BeamformingHelperBase ();
  /**
   * \brief ~BeamformingHelperBase destructor
   */
  virtual ~BeamformingHelperBase ();

  /**
   * \brief Get the Type ID
   * \return the TypeId of the instance
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Creates a new beamforming task, which means the pair of
   * devices for which the configured algorithm for updating the
   * beamforming vectors will be run either periodically or
   * as specified by the algorithm.
   * \param gNbDev gNb device
   * \param ueDev UE device
   */
  virtual void AddBeamformingTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                   const Ptr<NrUeNetDevice>& ueDev) = 0;

  /**
   * \brief Set the beamforming method that will be executed each
   * time when is necessary to update the beamforming algorithms
   * \param beamformingMethod the beamforming method to be set
   */
  virtual void SetBeamformingMethod (const TypeId &beamformingMethod) = 0;

  /**
   * \brief Set an attribute for the beafmorming algorithm that will be created.
   * \param n the name of the attribute
   * \param v the value of the attribute
   */
  void SetBeamformingAlgorithmAttribute (const std::string &n, const AttributeValue &v);

protected:

  /**
   * \brief This function runs the beamforming algorithm among the provided gNB and UE
   * device, and for a specified bwp index
   * \param gNbDev a pointer to a gNB device
   * \param ueDev a pointer to a UE device
   * \param [in] gnbSpectrumPhy the spectrum phy of the gNB
   * \param [in] ueSpectrumPhy the spectrum phy of the UE
   */
  virtual void RunTask (const Ptr<NrGnbNetDevice>& gNbDev,
                        const Ptr<NrUeNetDevice>& ueDev,
                        const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
                        const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const;

  /**
   * \brief Function that will call the configured algorithm for the specified devices and obtain
   * the beamforming vectors for each of them.
   * \param [in] gnbSpectrumPhy the spectrum phy of the gNB
   * \param [in] ueSpectrumPhy the spectrum phy of the UE
   * \return the beamforming vector pair of the gNB and the UE
   */
  virtual BeamformingVectorPair  GetBeamformingVectors (const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
                                                        const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const = 0;

  ObjectFactory m_algorithmFactory; //!< Object factory that will be used to create beamforming algorithms
};

}; //ns3 namespace


#endif /* SRC_NR_HELPER_BEAMFORMING_HELPER_BASE_H_ */

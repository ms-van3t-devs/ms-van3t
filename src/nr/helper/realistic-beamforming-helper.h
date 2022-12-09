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

#include <ns3/object-factory.h>
#include "beamforming-helper-base.h"
#include <ns3/realistic-beamforming-algorithm.h>
#include <ns3/node.h>
#include <ns3/beamforming-vector.h>

#ifndef SRC_NR_HELPER_REALISTIC_BEAMFORMING_HELPER_H_
#define SRC_NR_HELPER_REALISTIC_BEAMFORMING_HELPER_H_

namespace ns3 {

class NrGnbNetDevice;
class NrUeNetDevice;
class NrGnbPhy;
class NrUePhy;
class NrSpectrumPhy;


/**
 * \ingroup helper
 * \brief The RealisticBeamformingHelper class that helps user create beamforming tasks
 * and configure when these tasks should be executed. This helper also collects SRS measurements
 * for each gNB and UE. This helper class is currently compatible only with the
 * RealisticBeamformingAlgorithm.
 *
 * Similarly to IdealBeamformingHelper, since there is no real beamforming procedure,
 * there must be a class that will be able to emulate the beamforming procedure, and that is,
 * to update the beamforming vectors of both devices, gNB and UE, at the same time.
 *
 * In ideal algorithms there is a run function that is used to run all beamforming tasks (updates)
 * at the same time. Here is different, not all beams are updated at the same time,
 * instead each beamforming task will be triggered based on its own event (SRS count or delay)
 * To allow that, this class has attribute through which can be set the trigger event type.
 * E.g., the trigger event can be that it has been received a certain number of SRSs signals
 * from a UE.
 * This helper saves all SRS reports for each gNB and all of its users, and it is saved per
 * component carrier identified by cellId.
 *
 */

/**
 * \brief Calculate the Cantor function for two unsigned int
 * \param x1 first value max value 65535
 * \param x2 second value max value 65535
 * \return \f$ (((x1 + x2) * (x1 + x2 + 1))/2) + x2; \f$ max value 4294836225
 */
static constexpr uint32_t Cantor (uint32_t x1, uint32_t x2)
{
  return (((x1 + x2) * (x1 + x2 + 1)) / 2) + x2;
}



class RealisticBeamformingHelper : public BeamformingHelperBase
{
public:

  /**
   * \brief Get the Type ID
   * \return the TypeId of the instance
   */
  static TypeId GetTypeId (void);
  /**
   * \brief Adds the beamforming task to the list of tasks
   * \gnbDev gNbDev pointer to gNB device
   * \ueDev ueDev pointer to UE device
   */
  virtual void AddBeamformingTask (const Ptr<NrGnbNetDevice>& gNbDev,
                                   const Ptr<NrUeNetDevice>& ueDev) override;

  /**
   * \brief Function that forwards the SRS SINR to the correct RealisticBeamformingAlgorithm
   * \param srsSinr
   * \param rnti
   */
  void SaveSrsSinrReport (uint16_t cellId, uint16_t rnti, double srsSinr);
  /**
   * \brief When the condition for triggering a beamforming update is fullfilled
   * this function will be triggered
   * \param cellId id that uniquely identifies the gNB phy
   * \param rnti id that uniquely identifies the user of gNb
   * \param srsSinr value of srsSinr to be passed to RealisticBeamformingAlgorithm
   */
  void TriggerBeamformingAlgorithm (uint16_t cellId, uint16_t rnti, double srsSinr);

  /**
   * \brief SetBeamformingMethod
   * \param beamformingMethod
   */
  virtual void SetBeamformingMethod (const TypeId &beamformingMethod) override;

private:


  virtual BeamformingVectorPair GetBeamformingVectors (const Ptr<NrSpectrumPhy>& gnbSpectrumPhy,
                                                       const Ptr<NrSpectrumPhy>& ueSpectrumPhy) const override;

  typedef std::pair< Ptr<NrSpectrumPhy>, Ptr<NrSpectrumPhy> > BfAntennaPair;
  typedef std::map <BfAntennaPair, Ptr<RealisticBeamformingAlgorithm>> AntennaPairToAlgorithm;

  AntennaPairToAlgorithm m_antennaPairToAlgorithm;

};

}; //ns3 namespace


#endif /* SRC_NR_HELPER_REALISTIC_BEAMFORMING_HELPER_H_ */

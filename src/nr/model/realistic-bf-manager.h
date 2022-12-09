/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef SRC_NR_MODEL_REALISTIC_BF_MANAGER_H_
#define SRC_NR_MODEL_REALISTIC_BF_MANAGER_H_

#include <ns3/object.h>
#include "beam-manager.h"
#include <ns3/nstime.h>

namespace ns3 {

class NrUeNetDevice;
class NrGnbNetDevice;
class BeamformingHelperBase;

/**
 * \ingroup gnb-phy
 *
 * \brief Antenna array management
 *
 * BeamManager is responsible of installation and configuration of antenna
 * array. Additionally, in the case of gNB it saves the map of beamforming
 * vectors per device.
 */
class RealisticBfManager: public BeamManager
{
public:

  enum TriggerEvent
  {
    SRS_COUNT,
    DELAYED_UPDATE
  };

  /**
   * \brief RealisticBfManager constructor
   */
  RealisticBfManager();

  /**
   * \brief ~RealisticBfManager
   */
  virtual ~RealisticBfManager();

  /**
   * \brief GetTypeId
   * \return the TypeId of this instance
   */
  static TypeId GetTypeId ();

  /**
   * \brief Sets the beamforming update trigger event, trigger event type
   * is one for all the nodes
   * \param triggerEvent triggerEvent type
   */
  void SetTriggerEvent (RealisticBfManager::TriggerEvent triggerEvent);
  /**
   * \return Returns the trigger event type
   */
  RealisticBfManager::TriggerEvent GetTriggerEvent () const;

  /**
   * \brief Sets the periodicity of the beamforming update in the number of the
   * SRS SINR reports
   */
  void SetUpdatePeriodicity (uint16_t periodicity);

  /**
   * \returns Gets the periodicity in the number of SRS SINR reports
   */
  uint16_t GetUpdatePeriodicity () const;
  /**
   * \brief Sets the delay after the SRS SINR report reception and triggering of the
   * beamforming update
   * \param delay the delay after reception of SRS SINR
   */
  void SetUpdateDelay (Time delay);
  /**
   * \return returns the delay after sSRS SINR report and beamforming
   */
  Time GetUpdateDelay () const;

private:

  TriggerEvent m_triggerEvent; //!< Defines what will be the trigger event for the update of the beamforming vectors
  uint16_t m_updatePeriodicity {3}; //!< Periodicity of beamforming update in number of SRS SINR reports
  Time m_updateDelay {Seconds (0)}; //!< How much time to wait after the last SRS to update the beamforming vectors


};

} /* namespace ns3 */

#endif /* SRC_NR_MODEL_BEAM_MANAGER_H_ */

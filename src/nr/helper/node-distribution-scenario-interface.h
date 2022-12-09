/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
#ifndef NODE_DISTRIBUTION_SCENARIO_INTERFACE_H
#define NODE_DISTRIBUTION_SCENARIO_INTERFACE_H

#include "scenario-parameters.h"
#include <ns3/node-container.h>
#include <ns3/vector.h>

namespace ns3 {

/**
 * \brief Represents a scenario with base stations and user terminals.
 *
 * Set the relevant settings, and then call CreateScenario. After that call,
 * the node containers can be retrieved through GetBaseStations and GetUserTerminals.
 *
 * Site sectorizaton must be set before setting the number of sites or base stations.
 */
class NodeDistributionScenarioInterface : public ScenarioParameters
{
public:
  /**
   * \brief ~NodeDistributionScenarioInterface
   */
  virtual ~NodeDistributionScenarioInterface ();
  /**
   * \brief Get the list of gnb/base station nodes
   * \return A NodeContainer with all the Gnb (or base stations)
   */
  const NodeContainer & GetBaseStations () const;
  /**
   * \brief Get the list of user nodes
   * \return A NodeContainer with all the users
   */
  const NodeContainer & GetUserTerminals () const;

  /**
   * \brief Create the scenario, with the configured parameter.
   *
   * After this call, GetGnbs and GetUes will return the containers
   * with the nodes created and positioned.
   */
  virtual void CreateScenario () = 0;

  /**
   * \brief Set number of sites/towers.
   * \param n the number of sites
   */
  void SetSitesNumber (std::size_t n);

  /**
   * \brief Set the number of base stations.
   * \param n the number of bs
   *
   * Will invalidate already existing BS (recreating the container)
   */
  void SetBsNumber (std::size_t n);

  /**
   * \brief Set the number of UT/UE.
   * \param n the number of ut
   *
   * Will invalidate already existing UT (recreating the container)
   */
  void SetUtNumber (std::size_t n);

  /**
   * \brief Gets the number of sites with cell base stations.
   * \return Number of sites in the network deployment
   */
  std::size_t GetNumSites () const;

  /**
   * \brief Gets the total number of cells deployed
   * \return Number of cells in the network deployment
   */
  std::size_t GetNumCells () const;

  /**
   * \brief Returns the orientation in degrees of the antenna array
   * for the given cellId.
   * The orientation is the azimuth angle of the antenna bore sight.
   * \param cellId Cell Id
   * \return The antenna orientation in degrees [0, 360]
   */
  double GetAntennaOrientationDegrees (std::size_t cellId) const;

  /**
   * \brief Returns the orientation in radians of the antenna array
   * for the given cellId
   * \param cellId Cell Id
   * \return The antenna orientation in radians [-PI, PI]
   */
  double GetAntennaOrientationRadians (std::size_t cellId) const;

  /**
   * \brief Returns the position of the cell antenna
   * \param sitePos Site position coordinates in meters
   * \param cellId Id Cell id of the antenna
   */
  Vector GetAntennaPosition (const Vector &sitePos, uint16_t cellId) const;

  /**
   * \brief Gets the site index the queried cell id belongs to
   * \param cellId Cell index
   * \return site id
   */
  uint16_t GetSiteIndex (std::size_t cellId) const;

  /**
   * \brief Get the sector index the queried cell id belongs to.
   * \param cellId Cell index.
   * \return The sector id.
   */
  uint16_t GetSectorIndex (std::size_t cellId) const;

  /**
   * \brief Get the cell (base station) index the queried UE id belongs to.
   * \param ueId UE index.
   * \return The cell id.
   */
  uint16_t GetCellIndex (std::size_t ueId) const;

protected:
  
  std::size_t m_numSites; //!< Number of sites with base stations
  std::size_t m_numBs; //!< Number of base stations to create
  std::size_t m_numUt; //!< Number of user terminals to create
  NodeContainer m_bs;  //!< Base stations
  NodeContainer m_ut;  //!< User Terminals

};

} // namespace ns3

#endif // NODE_DISTRIBUTION_SCENARIO_INTERFACE_H

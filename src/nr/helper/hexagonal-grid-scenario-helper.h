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
#ifndef HEXAGONAL_GRID_SCENARIO_HELPER_H
#define HEXAGONAL_GRID_SCENARIO_HELPER_H

#include "node-distribution-scenario-interface.h"
#include <ns3/vector.h>
#include <ns3/random-variable-stream.h>

namespace ns3 {

/**
 * @brief The HexagonalGridScenarioHelper class
 *
 * TODO: Documentation, tests
 */
class HexagonalGridScenarioHelper : public NodeDistributionScenarioInterface
{
public:

  /**
   * \brief HexagonalGridScenarioHelper
   */
  HexagonalGridScenarioHelper ();

  /**
   * \brief ~HexagonalGridScenarioHelper
   */
  virtual ~HexagonalGridScenarioHelper () override;

  /**
   * \brief Sets the number of outer rings of sites around the central site
   *
   * Relation between the number of rings and the number of rings.
   *
   * 0 rings = 1 + 6 * 0 = 1 site
   * 1 rings = 1 + 6 * 1 = 7 sites
   * 2 rings = 1 + 6 * 2 = 13 sites
   * 3 rings = 1 + 6 * 3 = 19 sites
   * 4 rings = 1 + 6 * 4 = 31 sites
   * 5 rings = 1 + 6 * 5 = 37 sites
   *
   * 0 rings = (1 + 6 * 0 ) * 3 = 3 gNBs
   * 1 rings = (1 + 6 * 1 ) * 3 = 21 gNBs
   * 2 rings = (1 + 6 * 2 ) * 3 = 39 gNBs
   * 3 rings = (1 + 6 * 3 ) * 3 = 57 gNBs
   * 4 rings = (1 + 6 * 5 ) * 3 = 93 gNBs
   * 5 rings = (1 + 6 * 6 ) * 3 = 111 gNBs
   *
   * If 10 UEs per gNB:
   *
   * 0 rings = (1 + 6 * 0 ) * 3 * 10 = 30 UEs
   * 1 rings = (1 + 6 * 1 ) * 3 * 10 = 210 UEs
   * 2 rings = (1 + 6 * 2 ) * 3 * 10 = 390 UEs
   * 3 rings = (1 + 6 * 3 ) * 3 * 10 = 570 UEs
   * 4 rings = (1 + 6 * 5 ) * 3 * 10 = 930 UEs
   * 5 rings = (1 + 6 * 6 ) * 3 * 10  = 1110 UEs
   */
  void SetNumRings (uint8_t numRings);

  /**
   * \brief Gets the radius of the hexagonal cell
   * \returns Cell radius in meters
   */
  double GetHexagonalCellRadius () const;

  /**
   * \brief Returns the cell center coordinates
   * \param sitePos Site position coordinates
   * \param cellId Cell Id
   */
  Vector GetHexagonalCellCenter (const Vector &sitePos,
                                 uint16_t cellId) const;
  
  // inherited
  virtual void CreateScenario () override;

  /**
   * \brief This function can be used to create a scenario with
   *        UEs with mobility and define a percentage of UEs, if needed,
   *        that will have a random antenna height > 1.5 m
   * \param speed UE speed
   * \param percentage Percentage (decimal) of UEs with random antenna height > 1.5 m
   */
  void CreateScenarioWithMobility (const Vector &speed, double percentage);

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

private:
  uint8_t m_numRings {0};  //!< Number of outer rings of sites around the central site
  Vector m_centralPos {Vector (0,0,0)};     //!< Central site position
  double m_hexagonalRadius {0.0};  //!< Cell radius

  static std::vector<double> siteDistances;
  static std::vector<double> siteAngles;

  Ptr<UniformRandomVariable> m_r; //!< random variable used for the random generation of the radius
  Ptr<UniformRandomVariable> m_theta; //!< random variable used for the generation of angle
};

} // namespace ns3

#endif // HEXAGONAL_GRID_SCENARIO_HELPER_H

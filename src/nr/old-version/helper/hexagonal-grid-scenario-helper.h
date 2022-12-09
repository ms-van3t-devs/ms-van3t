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
   * \param numSecors The number of sectors of a site
   * \param hexagonRadius Radius of the hexagonal cell
   */
  Vector GetHexagonalCellCenter (const Vector &sitePos,
                                 uint16_t cellId) const;
  
  // inherited
  virtual void CreateScenario () override;

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

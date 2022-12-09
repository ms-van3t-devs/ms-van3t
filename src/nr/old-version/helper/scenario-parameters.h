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
#ifndef SCENARIO_PARAMETERS_H
#define SCENARIO_PARAMETERS_H

#include <ns3/node-container.h>
#include <ns3/vector.h>

namespace ns3 {

/**
 * \brief Basic simulation scenario parameters.
 */
class ScenarioParameters
{
public:
  /**
   * \brief Type of site sectorization
   */
  enum SiteSectorizationType
  {
    NONE = 0,   //!< Unconfigured value
    SINGLE = 1, //!< Site with a 360º-width sector
    TRIPLE = 3  //!< Site with 3 120º-width sectors
  };
  
  /**
   * \brief ~ScenarioParameters
   */
  virtual ~ScenarioParameters (void);

  /**
   * \brief SetGnbHeight
   * \param h height
   */
  void SetBsHeight (double h);

  /**
   * \brief SetUeHeight
   * \param h heights
   */
  void SetUtHeight (double h);

  /**
   * \brief Gets the number of sectors per site
   */
  uint32_t GetNumSectorsPerSite (void) const;

  /**
   * \brief Sets the number of sectors of every site.
   * \param numSectors Number of sectors. Values can be 1 or 3.
   */
  void SetSectorization (SiteSectorizationType numSectors);
  /** \copydoc SetSectorization(SiteSectorizationType) */
  void SetSectorization (uint32_t numSectors);

  /**
   * \brief Sets parameters to the specified scenario
   * \param scenario Scenario to simulate
   */
  void SetScenarioParameters (const std::string &scenario);

  /**
   * \brief Sets parameters to the specified scenario
   * \param scenario Scenario to simulate
   */
  void SetScenarioParameters (const ScenarioParameters &scenario);
  
  /**
   * \brief Sets the Urban Macro (UMa) scenario parameters
   */
  void SetUMaParameters (void);

  /**
   * \brief Sets the Urban Micro (UMi) scenario parameters
   */
  void SetUMiParameters (void);

  /**
   * \brief Sets rural Macro scenario parameters
   */
  void SetRMaParameters (void);

  // Keep the data members public to facilitate defining custom scenarios  
  double m_isd {-1.0};  //!< Inter-site distance (ISD) in meters
  double m_bsHeight {-1.0}; //!< Height of gNB nodes
  double m_utHeight {-1.0}; //!< Height of UE nodes
  SiteSectorizationType m_sectorization {NONE};  //!< Number of sectors per site
  double m_minBsUtDistance {-1.0}; //!< Minimum distance between BS and UT in meters
  double m_antennaOffset {-1.0}; //!< Cell antenna offset in meters w.r.t. site location

  /** Maximum distance between a sector antenna panel and the site it belongs to  */
  static double MAX_ANTENNA_OFFSET;

};

} // namespace ns3

#endif // SCENARIO_PARAMETERS_H

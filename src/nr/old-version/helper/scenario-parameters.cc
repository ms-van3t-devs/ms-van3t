/* Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; */
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

#include "scenario-parameters.h"


namespace ns3 {

double ScenarioParameters::MAX_ANTENNA_OFFSET = 1;  

ScenarioParameters::~ScenarioParameters (void)
{
}

void
ScenarioParameters::SetBsHeight (double h)
{
  m_bsHeight = h;
}

void
ScenarioParameters::SetUtHeight (double h)
{
  m_utHeight = h;
}

uint32_t
ScenarioParameters::GetNumSectorsPerSite (void) const
{
  return static_cast<uint32_t> (m_sectorization);
}

void
ScenarioParameters::SetSectorization (SiteSectorizationType numSectors)
{
  m_sectorization = numSectors;
}

void
ScenarioParameters::SetSectorization (uint32_t numSectors)
{
  SetSectorization (static_cast<SiteSectorizationType> (numSectors));
}

void
ScenarioParameters::SetScenarioParameters (const std::string &scenario)
{
  if (scenario == "UMa")
    {
      SetUMaParameters ();
    }
  else if (scenario == "UMi")
    {
      SetUMiParameters ();
    }
  else if (scenario == "RMa")
    {
      SetRMaParameters();
    }
  else
    {
      NS_ABORT_MSG("Unrecognized scenario: " << scenario);
    }

}

void
ScenarioParameters::SetScenarioParameters (const ScenarioParameters &scenario)
{
  m_isd = scenario.m_isd;
  m_bsHeight = scenario.m_bsHeight;
  m_utHeight = scenario.m_utHeight;
  m_sectorization = scenario.m_sectorization;
  m_minBsUtDistance = scenario.m_minBsUtDistance;
  m_antennaOffset = scenario.m_antennaOffset;
}
  

void
ScenarioParameters::SetUMaParameters (void)
{
  m_isd = 1732;
  m_bsHeight = 30.0;
  m_utHeight = 1.5;
  m_sectorization = SiteSectorizationType::TRIPLE;
  m_minBsUtDistance = 30.203; // minimum 2D distace is 10 meters considering UE height of 1.5 m
  m_antennaOffset = 1.0;
}

void
ScenarioParameters::SetUMiParameters (void)
{
  m_isd = 500;
  m_bsHeight = 10.0;
  m_utHeight = 1.5;
  m_sectorization = SiteSectorizationType::TRIPLE;
  m_minBsUtDistance = 10;
  m_antennaOffset = 1.0;
}

void
ScenarioParameters::SetRMaParameters (void)
{
  m_isd = 7000;
  m_bsHeight = 45.0;
  m_utHeight = 1.5;
  m_sectorization = SiteSectorizationType::TRIPLE;
  m_minBsUtDistance = 44.63; // minimum 2D distace is 10 meters considering UE height of 1.5 m
  m_antennaOffset = 1.0;
}


} //namespace ns3

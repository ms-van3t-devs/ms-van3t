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

#include "beam-id.h"

namespace ns3 {

std::ostream& operator<< (std::ostream &os, const BeamId &item)
{
  os << "[Sector: " << static_cast<uint16_t> (item.GetSector ())
               << " elevation: " << item.GetElevation () << "]";
  return os;
}

BeamId::BeamId ()
{
}

BeamId::BeamId (uint16_t sector, double elevation)
{
  m_sector = sector;
  m_elevation = elevation;
}

bool
BeamId::operator==(const BeamId& p) const
{
  return m_sector == p.GetSector() && m_elevation == p.GetElevation();
}

bool
BeamId::operator!=(const BeamId& p) const
{
  return (m_sector != p.GetSector() || m_elevation != p.GetElevation());
}

uint16_t
BeamId::GetSector () const
{
  return m_sector;
}

double
BeamId::GetElevation () const
{
  return m_elevation;
}


BeamId
BeamId::GetEmptyBeamId ()
{
  return BeamId (0, 0);
}

/**
 * \brief Calculate the Cantor function for two unsigned int
 * \param x1 first value max value 65535
 * \param x2 second value max value 65535
 * \return \f$ (((x1 + x2) * (x1 + x2 + 1))/2) + x2; \f$ max value 4294836225
 */
static constexpr uint32_t Cantor (uint16_t x1, uint16_t x2)
{
  return (((x1 + x2) * (x1 + x2 + 1)) / 2) + x2;
}

uint32_t
BeamId::GetCantor () const
{
  return Cantor (m_sector, static_cast<uint16_t> (m_elevation));
}


size_t BeamIdHash::operator() (const BeamId &x) const
{
  return std::hash<uint32_t> () (x.GetCantor ());
}

} /* namespace ns3 */


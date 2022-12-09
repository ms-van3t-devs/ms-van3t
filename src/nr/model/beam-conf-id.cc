/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

/*
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
 *
 */

#include "beam-conf-id.h"
#include <ns3/assert.h>

namespace ns3 {

std::ostream& operator<< (std::ostream &os, const BeamConfId &item)
{
  os << "First: " << item.GetFirstBeam () << " Second: " << item.GetSecondBeam ();
  return os;
}

BeamConfId::BeamConfId ()
{
  m_firstBeam = BeamId::GetEmptyBeamId ();
  m_secondBeam = BeamId::GetEmptyBeamId ();
}


BeamConfId::BeamConfId (BeamId firstBeam, BeamId secondBeam)
{
  m_firstBeam = firstBeam;
  m_secondBeam = secondBeam;
}

bool
BeamConfId::operator== (const BeamConfId& p) const
{
  return m_firstBeam == p.GetFirstBeam () && m_secondBeam == p.GetSecondBeam ();
}

bool
BeamConfId::operator!= (const BeamConfId& p) const
{
  return (m_firstBeam != p.GetFirstBeam () || m_secondBeam != p.GetSecondBeam ());
}

BeamId
BeamConfId::GetFirstBeam () const
{
  return m_firstBeam;
}

BeamId
BeamConfId::GetSecondBeam () const
{
  return m_secondBeam;
}

BeamConfId
BeamConfId::GetEmptyBeamConfId ()
{
  return BeamConfId (BeamId::GetEmptyBeamId (), BeamId::GetEmptyBeamId ());
}

/**
 * \brief Calculate the Cantor function for two unsigned int
 * \param x1 first value max value 4294967295
 * \param x2 second value max value 4294967295
 * \return \f$ (((x1 + x2) * (x1 + x2 + 1))/2) + x2; \f$ max value 18446744073709551615
 */
static constexpr uint64_t Cantor64bit (uint32_t x1, uint32_t x2)
{
  return (((x1 + x2) * (x1 + x2 + 1)) / 2) + x2;
}


size_t BeamConfIdHash::operator() (const BeamConfId &x) const
{
  uint32_t firstBeamCantor = 0;
  uint32_t secondBeamCantor = 0;

  if (x.GetFirstBeam () != BeamId::GetEmptyBeamId ())
    {
      firstBeamCantor = x.GetFirstBeam ().GetCantor ();
    }

  if (x.GetSecondBeam () != BeamId::GetEmptyBeamId ())
    {
      secondBeamCantor = x.GetSecondBeam ().GetCantor ();
    }

  return std::hash<uint64_t>()(Cantor64bit (firstBeamCantor, secondBeamCantor));
}

} /* namespace ns3 */

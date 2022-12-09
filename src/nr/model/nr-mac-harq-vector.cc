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
#include "nr-mac-harq-vector.h"

namespace ns3 {

bool
NrMacHarqVector::Erase (uint8_t id)
{
  at (id).Erase ();
  --m_usedSize;

  uint32_t count = 0;
  for (const auto & v : *this)
    {
      if (v.second.m_active)
        {
          ++count;
        }
    }
  NS_ASSERT (count == m_usedSize);
  return true;
}

bool
NrMacHarqVector::Insert (uint8_t *id, const HarqProcess& element)
{
  if (m_usedSize >= m_maxSize)
    {
      return false;
    }

  NS_ABORT_IF (element.m_active == false);

  *id = FirstAvailableId ();
  if (*id == 255)
    {
      return false;
    }

  NS_ABORT_IF (at (*id).m_active == true);
  at (*id) = element;

  NS_ABORT_IF (at (*id).m_active == false);
  NS_ABORT_IF (this->FirstAvailableId () == *id);

  ++m_usedSize;
  return true;
}

std::ostream &
operator<< (std::ostream & os, NrMacHarqVector const & item)
{
  for (const auto & p : item)
    {
      os << "Process ID " << static_cast<uint32_t> (p.first)
         << ": " << p.second << std::endl;
    }
  return os;
}

} // namespace ns3

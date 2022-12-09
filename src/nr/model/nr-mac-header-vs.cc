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

#include "nr-mac-header-vs.h"
#include <ns3/log.h>
#include <algorithm>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrMacHeaderVs);
NS_LOG_COMPONENT_DEFINE("NrMacHeaderVs");

// Maybe we can use the name here? Like LC_ID_1
std::vector<uint8_t>
NrMacHeaderVs::m_allowedLcId = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
                                 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
                                 26, 27, 28, 29, 30, 31, 32 };
TypeId
NrMacHeaderVs::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrMacHeaderVs")
    .SetParent<Header> ()
    .AddConstructor<NrMacHeaderVs> ();
  return tid;
}

TypeId NrMacHeaderVs::GetInstanceTypeId () const
{
  return GetTypeId ();
}

NrMacHeaderVs::NrMacHeaderVs()
{
  NS_LOG_FUNCTION (this);
}

void
NrMacHeaderVs::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);

  // 0x3F: 0 0 1 1 1 1 1 1
  uint8_t firstByte = m_lcid & 0x3F;  // R, F bit set to 0, the rest equal to lcId

  if (m_size > 127)
    {
      // 0x40: 0 1 0 0 0 0 0 0
      firstByte = firstByte | 0x40; // set the F bit to 1, the rest as before
    }

  start.WriteU8 (firstByte);

  if (m_size > 127)
    {
      start.WriteHtonU16 (m_size);
    }
  else
    {
      start.WriteU8 (static_cast<uint8_t> (m_size));
    }
}

uint32_t
NrMacHeaderVs::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this);

  uint32_t readBytes = 1;
  uint8_t firstByte = start.ReadU8 ();

  // 0x3F: 0 0 1 1 1 1 1 1
  m_lcid = firstByte & 0x3F; // Clear the first 2 bits, the rest is the lcId

  // 0xC0: 1 1 0 0 0 0 0 0
  uint8_t lFieldSize = firstByte & 0xC0;

  if (lFieldSize == 0x40)
    {
      // the F bit is set to 1
      m_size = start.ReadNtohU16 ();
      readBytes += 2;
    }
  else if (lFieldSize == 0x00)
    {
      // the F bit is set to 0
      m_size = start.ReadU8 ();
      readBytes += 1;
    }
  else
    {
      NS_FATAL_ERROR ("The author of the code, who lies behind a christmas tree, is guilty");
    }

  if (m_size > 127)
    {
      NS_ASSERT (readBytes == 3);
    }
  else
    {
      NS_ASSERT (readBytes == 2);
    }
  return readBytes;
}

uint32_t
NrMacHeaderVs::GetSerializedSize () const
{
  NS_LOG_FUNCTION (this);
  if (m_size > 127)
    {
      return 3;
    }
  return 2;
}

void
NrMacHeaderVs::Print (std::ostream &os) const
{
  os << "LCid " << +m_lcid << " size " << m_size;
}

bool
NrMacHeaderVs::operator == (const NrMacHeaderVs &o) const
{
  return m_lcid == o.m_lcid && m_size == o.m_size;
}

void NrMacHeaderVs::SetLcId (uint8_t lcId)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (std::find (m_allowedLcId.begin(), m_allowedLcId.end(), lcId) != m_allowedLcId.end ());
  m_lcid = lcId;
}

uint8_t
NrMacHeaderVs::GetLcId () const
{
  return m_lcid;
}

void
NrMacHeaderVs::SetSize(uint16_t size)
{
  m_size = size;
}

uint16_t
NrMacHeaderVs::GetSize() const
{
  return m_size;
}

} // namespace ns3


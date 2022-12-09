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

#include "nr-mac-header-fs.h"
#include <ns3/log.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrMacHeaderFs);
NS_LOG_COMPONENT_DEFINE ("NrMacHeaderFs");

TypeId
NrMacHeaderFs::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrMacHeaderFs")
    .SetParent<Header> ()
    .AddConstructor <NrMacHeaderFs> ()
  ;
  return tid;
}

TypeId
NrMacHeaderFs::GetInstanceTypeId () const
{
  return GetTypeId ();
}

NrMacHeaderFs::NrMacHeaderFs ()
{
  NS_LOG_FUNCTION (this);
}

void
NrMacHeaderFs::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this);

  // 0x3F: 0 0 1 1 1 1 1 1
  uint8_t firstByte = m_lcid & 0x3F;  // R, R bit set to 0, the rest equal to lcId
  start.WriteU8 (firstByte);
}

uint32_t
NrMacHeaderFs::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this);

  m_lcid = start.ReadU8 ();

  return 1;
}

uint32_t
NrMacHeaderFs::GetSerializedSize () const
{
  NS_LOG_FUNCTION (this);
  return 1;
}

void
NrMacHeaderFs::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this);
  os << "LCid " << +m_lcid;
}

void
NrMacHeaderFs::SetLcId (uint8_t lcId)
{
  NS_ASSERT (lcId == PADDING);
  m_lcid = PADDING;
}

uint8_t
NrMacHeaderFs::GetLcId () const
{
  return m_lcid;
}

bool
NrMacHeaderFs::operator == (const NrMacHeaderFs &o) const
{
  return m_lcid == o.m_lcid;
}

} // namespace ns3


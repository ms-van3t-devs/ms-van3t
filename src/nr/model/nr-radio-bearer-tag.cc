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



#include "nr-radio-bearer-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrRadioBearerTag);

TypeId
NrRadioBearerTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrRadioBearerTag")
    .SetParent<Tag> ()
    .AddConstructor<NrRadioBearerTag> ()
    .AddAttribute ("rnti", "The rnti that indicates the UE to which packet belongs",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrRadioBearerTag::GetRnti),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("lcid", "The id whithin the UE identifying the logical channel to which the packet belongs",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrRadioBearerTag::GetLcid),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("size", "Size in bytes of the RLC PDU",
                   UintegerValue (0),
                   MakeUintegerAccessor (&NrRadioBearerTag::GetSize),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

TypeId
NrRadioBearerTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

NrRadioBearerTag::NrRadioBearerTag ()
  : m_rnti (0),
  m_lcid (0),
  m_layer (0),
  m_size (0)
{
}
NrRadioBearerTag::NrRadioBearerTag (uint16_t rnti, uint8_t lcid, uint32_t size)
  : m_rnti (rnti),
  m_lcid (lcid),
  m_size (size)
{
}

NrRadioBearerTag::NrRadioBearerTag (uint16_t rnti, uint8_t lcid, uint32_t size, uint8_t layer)
  : m_rnti (rnti),
  m_lcid (lcid),
  m_layer (layer),
  m_size (size)
{
}

void
NrRadioBearerTag::SetRnti (uint16_t rnti)
{
  m_rnti = rnti;
}

void
NrRadioBearerTag::SetLcid (uint8_t lcid)
{
  m_lcid = lcid;
}

void
NrRadioBearerTag::SetLayer (uint8_t layer)
{
  m_layer = layer;
}

void
NrRadioBearerTag::SetSize (uint32_t size)
{
  m_size = size;
}

uint32_t
NrRadioBearerTag::GetSerializedSize (void) const
{
  return 4;
}

void
NrRadioBearerTag::Serialize (TagBuffer i) const
{
  i.WriteU16 (m_rnti);
  i.WriteU8 (m_lcid);
  i.WriteU8 (m_layer);
  i.WriteU32 (m_size);
}

void
NrRadioBearerTag::Deserialize (TagBuffer i)
{
  m_rnti = (uint16_t) i.ReadU16 ();
  m_lcid = (uint8_t) i.ReadU8 ();
  m_layer = (uint8_t) i.ReadU8 ();
}

uint16_t
NrRadioBearerTag::GetRnti () const
{
  return m_rnti;
}

uint8_t
NrRadioBearerTag::GetLcid () const
{
  return m_lcid;
}

uint8_t
NrRadioBearerTag::GetLayer () const
{
  return m_layer;
}

uint32_t
NrRadioBearerTag::GetSize () const
{
  return m_size;
}

void
NrRadioBearerTag::Print (std::ostream &os) const
{
  os << "rnti=" << m_rnti << ", lcid=" << (uint16_t) m_lcid << ", layer=" << (uint16_t)m_layer;
}

} // namespace ns3



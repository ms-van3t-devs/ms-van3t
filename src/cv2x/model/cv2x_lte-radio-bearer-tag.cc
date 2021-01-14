/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 * Modified by: NIST (D2D)
 */


#include "cv2x_lte-radio-bearer-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteRadioBearerTag);

TypeId
cv2x_LteRadioBearerTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteRadioBearerTag")
    .SetParent<Tag> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteRadioBearerTag> ()
    .AddAttribute ("rnti", "The rnti that indicates the UE to which packet belongs",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteRadioBearerTag::GetRnti),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("lcid", "The id within the UE identifying the logical channel to which the packet belongs",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteRadioBearerTag::GetLcid),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("srcL2Id", "For sidelink communication, the source L2 identifier",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteRadioBearerTag::GetSourceL2Id),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("dstL2Id", "For sidelink communication, the destination L2 identifier",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteRadioBearerTag::GetDestinationL2Id),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

TypeId
cv2x_LteRadioBearerTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

cv2x_LteRadioBearerTag::cv2x_LteRadioBearerTag ()
  : m_rnti (0),
    m_lcid (0),
    m_layer (0),
    m_srcL2Id (0),
    m_dstL2Id (0)
{
}
cv2x_LteRadioBearerTag::cv2x_LteRadioBearerTag (uint16_t rnti, uint8_t lcid)
  : m_rnti (rnti),
    m_lcid (lcid),
    m_layer (0),
    m_srcL2Id (0),
    m_dstL2Id (0)
{
}

cv2x_LteRadioBearerTag::cv2x_LteRadioBearerTag (uint16_t rnti, uint8_t lcid, uint8_t layer)
: m_rnti (rnti),
  m_lcid (lcid),
  m_layer (layer),
  m_srcL2Id (0),
  m_dstL2Id (0)
{
}

cv2x_LteRadioBearerTag::cv2x_LteRadioBearerTag (uint16_t rnti, uint8_t lcid, uint32_t srcL2Id, uint32_t dstL2Id)
: m_rnti (rnti),
  m_lcid (lcid),
  m_layer (0),
  m_srcL2Id (srcL2Id),
  m_dstL2Id (dstL2Id)
{
}

void
cv2x_LteRadioBearerTag::SetRnti (uint16_t rnti)
{
  m_rnti = rnti;
}

void
cv2x_LteRadioBearerTag::SetLcid (uint8_t lcid)
{
  m_lcid = lcid;
}

void
cv2x_LteRadioBearerTag::SetLayer (uint8_t layer)
{
  m_layer = layer;
}

void
cv2x_LteRadioBearerTag::SetSourceL2Id (uint32_t src)
{
  m_srcL2Id = src;
}
  
void
cv2x_LteRadioBearerTag::SetDestinationL2Id (uint32_t dst)
{
  m_dstL2Id = dst;
}

uint32_t
cv2x_LteRadioBearerTag::GetSerializedSize (void) const
{
  return 12;
}

void
cv2x_LteRadioBearerTag::Serialize (TagBuffer i) const
{
  i.WriteU16 (m_rnti);
  i.WriteU8 (m_lcid);
  i.WriteU8 (m_layer);
  i.WriteU32 (m_srcL2Id);
  i.WriteU32 (m_dstL2Id);
}

void
cv2x_LteRadioBearerTag::Deserialize (TagBuffer i)
{
  m_rnti = (uint16_t) i.ReadU16 ();
  m_lcid = (uint8_t) i.ReadU8 ();
  m_layer = (uint8_t) i.ReadU8 ();
  m_srcL2Id = (uint32_t) i.ReadU32 ();
  m_dstL2Id = (uint32_t) i.ReadU32 ();
}

uint16_t
cv2x_LteRadioBearerTag::GetRnti () const
{
  return m_rnti;
}

uint8_t
cv2x_LteRadioBearerTag::GetLcid () const
{
  return m_lcid;
}

uint8_t
cv2x_LteRadioBearerTag::GetLayer () const
{
  return m_layer;
}

uint32_t
cv2x_LteRadioBearerTag::GetSourceL2Id () const
{
  return m_srcL2Id;
}

uint32_t
cv2x_LteRadioBearerTag::GetDestinationL2Id () const
{
  return m_dstL2Id;
}

void
cv2x_LteRadioBearerTag::Print (std::ostream &os) const
{
  os << "rnti=" << m_rnti << ", lcid=" << (uint16_t) m_lcid << ", layer=" << (uint16_t)m_layer;
}

} // namespace ns3

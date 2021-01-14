/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011,2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 *         Nicola Baldo <nbaldo@cttc.es>
 */


#include "cv2x_eps-bearer-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_EpsBearerTag);

TypeId
cv2x_EpsBearerTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_EpsBearerTag")
    .SetParent<Tag> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_EpsBearerTag> ()
    .AddAttribute ("rnti", "The rnti that indicates the UE which packet belongs",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_EpsBearerTag::GetRnti),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("bid", "The EPS bearer id within the UE to which the packet belongs",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_EpsBearerTag::GetBid),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

TypeId
cv2x_EpsBearerTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

cv2x_EpsBearerTag::cv2x_EpsBearerTag ()
  : m_rnti (0),
    m_bid (0)
{
}
cv2x_EpsBearerTag::cv2x_EpsBearerTag (uint16_t rnti, uint8_t bid)
  : m_rnti (rnti),
    m_bid (bid)
{
}

void
cv2x_EpsBearerTag::SetRnti (uint16_t rnti)
{
  m_rnti = rnti;
}

void
cv2x_EpsBearerTag::SetBid (uint8_t bid)
{
  m_bid = bid;
}

uint32_t
cv2x_EpsBearerTag::GetSerializedSize (void) const
{
  return 3;
}

void
cv2x_EpsBearerTag::Serialize (TagBuffer i) const
{
  i.WriteU16 (m_rnti);
  i.WriteU8 (m_bid);
}

void
cv2x_EpsBearerTag::Deserialize (TagBuffer i)
{
  m_rnti = (uint16_t) i.ReadU16 ();
  m_bid = (uint8_t) i.ReadU8 ();
}

uint16_t
cv2x_EpsBearerTag::GetRnti () const
{
  return m_rnti;
}

uint8_t
cv2x_EpsBearerTag::GetBid () const
{
  return m_bid;
}

void
cv2x_EpsBearerTag::Print (std::ostream &os) const
{
  os << "rnti=" << m_rnti << ", bid=" << (uint16_t) m_bid;
}

} // namespace ns3

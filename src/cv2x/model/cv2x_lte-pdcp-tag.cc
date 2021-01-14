/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 CTTC
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
 * Author: Jaume Nin <jaume.nin@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 */

#include "cv2x_lte-pdcp-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_PdcpTag);

cv2x_PdcpTag::cv2x_PdcpTag ()
  : m_senderTimestamp (Seconds (0))
{
  // Nothing to do here
}


cv2x_PdcpTag::cv2x_PdcpTag (Time senderTimestamp)
  : m_senderTimestamp (senderTimestamp)

{
  // Nothing to do here
}

TypeId
cv2x_PdcpTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_PdcpTag")
    .SetParent<Tag> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_PdcpTag> ();
  return tid;
}

TypeId
cv2x_PdcpTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
cv2x_PdcpTag::GetSerializedSize (void) const
{
  return sizeof(Time);
}

void
cv2x_PdcpTag::Serialize (TagBuffer i) const
{
  int64_t senderTimestamp = m_senderTimestamp.GetNanoSeconds ();
  i.Write ((const uint8_t *)&senderTimestamp, sizeof(int64_t));
}

void
cv2x_PdcpTag::Deserialize (TagBuffer i)
{
  int64_t senderTimestamp;
  i.Read ((uint8_t *)&senderTimestamp, 8);
  m_senderTimestamp   = NanoSeconds (senderTimestamp);

}

void
cv2x_PdcpTag::Print (std::ostream &os) const
{
  os << m_senderTimestamp;
}

Time
cv2x_PdcpTag::GetSenderTimestamp (void) const
{
  return m_senderTimestamp;
}

void
cv2x_PdcpTag::SetSenderTimestamp (Time senderTimestamp)
{
  this->m_senderTimestamp = senderTimestamp;
}


} // namespace ns3


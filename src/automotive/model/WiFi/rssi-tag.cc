/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

 * Created by:
 *  Diego Gasco, Politecnico di Torino (diego.gasco@polito.it, diego.gasco99@gmail.com)
*/

#include "ns3/double.h"
#include "ns3/rssi-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RssiTag);

TypeId
RssiTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RssiTag")
    .SetParent<Tag> ()
    .SetGroupName ("Wifi")
    .AddConstructor<RssiTag> ()
    .AddAttribute ("Rssi", "The RSSI of the last packet received",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RssiTag::Get),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
RssiTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

RssiTag::RssiTag ()
  : m_rssi (0)
{
}

uint32_t
RssiTag::GetSerializedSize (void) const
{
  return sizeof (double);
}

void
RssiTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_rssi);
}

void
RssiTag::Deserialize (TagBuffer i)
{
  m_rssi = i.ReadDouble ();
}

void
RssiTag::Print (std::ostream &os) const
{
  os << "Rssi=" << m_rssi;
}

void
RssiTag::Set (double rssi)
{
  m_rssi = rssi;
}

double
RssiTag::Get (void) const
{
  return m_rssi;
}

}

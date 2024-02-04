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
#include "ns3/rsrp-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RsrpTag);

TypeId
RsrpTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RsrpTag")
    .SetParent<Tag> ()
    .SetGroupName ("Wifi")
    .AddConstructor<RsrpTag> ()
    .AddAttribute ("Rsrp", "The RSRP of the last packet received",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RsrpTag::Get),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
RsrpTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

RsrpTag::RssiTag ()
  : m_rsrp (0)
{
}

uint32_t
RsrpTag::GetSerializedSize (void) const
{
  return sizeof (double);
}

void
RsrpTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_rsrp);
}

void
RsrpTag::Deserialize (TagBuffer i)
{
  m_rsrp = i.ReadDouble ();
}

void
RsrpTag::Print (std::ostream &os) const
{
  os << "Rsrp=" << m_rsrp;
}

void
RsrpTag::Set (double rsrp)
{
  m_rsrp = rsrp;
}

double
RsrpTag::Get (void) const
{
  return m_rsrp;
}

}

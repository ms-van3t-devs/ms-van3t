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
#include "ns3/sinr-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SinrTag);

TypeId
SinrTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SinrTag")
    .SetParent<Tag> ()
    .SetGroupName ("Wifi")
    .AddConstructor<SinrTag> ()
    .AddAttribute ("Sinr", "The SINR of the last packet received",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&SinrTag::Get),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
SinrTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SinrTag::SinrTag ()
  : m_sinr (0)
{
}

uint32_t
SinrTag::GetSerializedSize (void) const
{
  return sizeof (double);
}

void
SinrTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_sinr);
}

void
SinrTag::Deserialize (TagBuffer i)
{
  m_sinr = i.ReadDouble ();
}

void
SinrTag::Print (std::ostream &os) const
{
  os << "Sinr=" << m_sinr;
}

void
SinrTag::Set (double sinr)
{
  m_sinr = sinr;
}

double
SinrTag::Get (void) const
{
  return m_sinr;
}

}

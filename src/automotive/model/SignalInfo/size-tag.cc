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
#include "ns3/size-tag.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SizeTag);

TypeId
SizeTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SizeTag")
    .SetParent<Tag> ()
    .SetGroupName ("Wifi")
    .AddConstructor<SizeTag> ()
    .AddAttribute ("Size", "The SIZE of the last packet received",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&SizeTag::Get),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

TypeId
SizeTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

SizeTag::SizeTag ()
  : m_size (0)
{
}

uint32_t
SizeTag::GetSerializedSize (void) const
{
  return sizeof (double);
}

void
SizeTag::Serialize (TagBuffer i) const
{
  i.WriteDouble (m_size);
}

void
SizeTag::Deserialize (TagBuffer i)
{
  m_size = i.ReadDouble ();
}

void
SizeTag::Print (std::ostream &os) const
{
  os << "Size=" << m_size;
}

void
SizeTag::Set (double size)
{
  m_size = size;
}

double
SizeTag::Get (void) const
{
  return m_size;
}

}

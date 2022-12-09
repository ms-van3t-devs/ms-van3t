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

#include "nr-sl-sci-f1a-header.h"
#include <ns3/log.h>
#include <algorithm>


namespace ns3 {


NS_OBJECT_ENSURE_REGISTERED (NrSlSciF1aHeader);
NS_LOG_COMPONENT_DEFINE ("NrSlSciF1aHeader");

NrSlSciF1aHeader::NrSlSciF1aHeader ()
{
}

NrSlSciF1aHeader::~NrSlSciF1aHeader ()
{
}

/**
 * TS 38.212 Table 8.3.1.1-1 specifies the values for 2nd-stage SCI formats.
 * Currently, we only support SCI_FORMAT_2A = 0;
 */
std::vector<NrSlSciF1aHeader::SciStage2Format_t>
NrSlSciF1aHeader::m_allowedSciStage2Format = { SciFormat2A };

TypeId
NrSlSciF1aHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlSciF1aHeader")
    .SetParent<Header> ()
    .AddConstructor<NrSlSciF1aHeader> ()
  ;
  return tid;
}

TypeId
NrSlSciF1aHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
NrSlSciF1aHeader::SetPriority (uint8_t priority)
{
  m_priority = priority;
}

void
NrSlSciF1aHeader::SetTotalSubChannels (uint16_t totalSubChannels)
{
  NS_ASSERT_MSG (m_totalSubChannels > 0, "Total number of sub-channels must be greater than 0");
  m_totalSubChannels = totalSubChannels;
}

void
NrSlSciF1aHeader::SetIndexStartSubChannel (uint8_t indexStartSubChannel)
{
  m_indexStartSubChannel = indexStartSubChannel;
}

void
NrSlSciF1aHeader::SetLengthSubChannel (uint8_t lengthSubChannel)
{
  m_lengthSubChannel = lengthSubChannel;
}

void
NrSlSciF1aHeader::SetSlResourceReservePeriod (uint16_t slResourceReservePeriod)
{
  m_slResourceReservePeriod = slResourceReservePeriod;
}

void
NrSlSciF1aHeader::SetMcs (uint8_t mcs)
{
  m_mcs = mcs;
}

void
NrSlSciF1aHeader::SetSlMaxNumPerReserve (uint8_t slMaxNumPerReserve)
{
  //Just a sanity check
  bool valueCheck = false;
  valueCheck = (slMaxNumPerReserve == 1) || (slMaxNumPerReserve == 2) || (slMaxNumPerReserve == 3);
  NS_ASSERT_MSG (valueCheck, "Invalid value " << +slMaxNumPerReserve << " for SlMaxNumPerReserve. Only 1, 2, or 3 should be used");
  m_slMaxNumPerReserve = slMaxNumPerReserve;
}

void
NrSlSciF1aHeader::SetGapReTx1 (uint8_t gapReTx1)
{
  NS_ASSERT_MSG (m_slMaxNumPerReserve == 2 || m_slMaxNumPerReserve == 3, "SlMaxNumPerReserve should be set to 2 or 3 before setting GapReTx1");
  m_gapReTx1 = gapReTx1;
}

void
NrSlSciF1aHeader::SetGapReTx2 (uint8_t gapReTx2)
{
  NS_ASSERT_MSG (m_slMaxNumPerReserve == 3, "SlMaxNumPerReserve should be set to 3 before setting GapReTx2");
  NS_ASSERT_MSG (gapReTx2 != GetGapReTx1 (), "The second retransmission should be perform in a different slot than the first retransmission");
  m_gapReTx2 = gapReTx2;
}

void
NrSlSciF1aHeader::SetSciStage2Format (uint8_t formatValue)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (std::find (m_allowedSciStage2Format.begin(), m_allowedSciStage2Format.end(), formatValue) != m_allowedSciStage2Format.end ());
  m_slSciStage2Format = formatValue;
}

void
NrSlSciF1aHeader::SetIndexStartSbChReTx1 (uint8_t sbChIndexReTx1)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_slMaxNumPerReserve == 2 || m_slMaxNumPerReserve == 3, "SlMaxNumPerReserve should be set to 2 or 3 before calling this method");
  m_indexStartSbChReTx1 = sbChIndexReTx1;
}

void
NrSlSciF1aHeader::SetIndexStartSbChReTx2 (uint8_t sbChIndexReTx2)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_slMaxNumPerReserve == 3, "SlMaxNumPerReserve should be set to 3 before setting GapReTx2");
  m_indexStartSbChReTx2 = sbChIndexReTx2;
}

uint8_t
NrSlSciF1aHeader::GetPriority () const
{
  return m_priority;
}
uint16_t
NrSlSciF1aHeader::GetTotalSubChannels () const
{
  return m_totalSubChannels;
}

uint8_t
NrSlSciF1aHeader::GetIndexStartSubChannel () const
{
  return m_indexStartSubChannel;
}

uint8_t
NrSlSciF1aHeader::GetLengthSubChannel () const
{
  return m_lengthSubChannel;
}

uint16_t
NrSlSciF1aHeader::GetSlResourceReservePeriod () const
{
  return m_slResourceReservePeriod;
}

uint8_t
NrSlSciF1aHeader::GetMcs () const
{
  return m_mcs;
}

uint8_t
NrSlSciF1aHeader::GetSlMaxNumPerReserve () const
{
  return m_slMaxNumPerReserve;
}

uint8_t
NrSlSciF1aHeader::GetGapReTx1 () const
{
  return m_gapReTx1;
}

uint8_t
NrSlSciF1aHeader::GetGapReTx2 () const
{
  return m_gapReTx2;
}

uint8_t
NrSlSciF1aHeader::GetSciStage2Format () const
{
  return m_slSciStage2Format;
}

uint8_t
NrSlSciF1aHeader::GetIndexStartSbChReTx1 () const
{
  return m_indexStartSbChReTx1;
}

uint8_t
NrSlSciF1aHeader::GetIndexStartSbChReTx2 () const
{
  return m_indexStartSbChReTx2;
}



bool
NrSlSciF1aHeader::EnsureMandConfig () const
{
  bool shouldBeSet = m_priority != std::numeric_limits <uint8_t>::max ()
    && m_mcs != std::numeric_limits <uint8_t>::max ()
    && m_slSciStage2Format != std::numeric_limits <uint8_t>::max ()
    && m_slResourceReservePeriod != std::numeric_limits <uint16_t>::max ()
    && m_totalSubChannels != std::numeric_limits <uint16_t>::max ()
    && m_indexStartSubChannel != std::numeric_limits <uint8_t>::max ()
    && m_lengthSubChannel != std::numeric_limits <uint8_t>::max ()
    && m_slMaxNumPerReserve != std::numeric_limits <uint8_t>::max ();

  return shouldBeSet;
}


void
NrSlSciF1aHeader::Print (std::ostream &os)  const
{
  NS_LOG_FUNCTION (this);
  os << "Priority " << +m_priority
     << ", MCS " << +m_mcs
     << ", 2nd stage SCI format " << +m_slSciStage2Format
     << ", Resource reservation period " << +m_slResourceReservePeriod
     << ", Total number of Subchannels " << +m_totalSubChannels
     << ", Index starting Subchannel " << +m_indexStartSubChannel
     << ", Total number of allocated Subchannel " << +m_lengthSubChannel
     << ", Maximum number of reservations " << +m_slMaxNumPerReserve
     << ", First retransmission gap in slots " << +m_gapReTx1
     << ", Second retransmission gap in slots " << +m_gapReTx2
     << ", Index starting Subchannel ReTx1 " << +m_indexStartSbChReTx1
     << ", Index starting Subchannel ReTx2 " << +m_indexStartSbChReTx2;
}

uint32_t
NrSlSciF1aHeader::GetSerializedSize (void) const
{
  uint32_t totalSize = 0; //bytes
  //Always present
  //priority =  1 byte
  //mcs =  1 byte
  //slSciStage2Format = 1 byte
  //slResourceReservePeriod = 2 bytes
  //totalSubChannels = 2 bytes
  //indexStartSubChannel = 1 byte
  //lengthSubChannel = 1 byte
  //slMaxNumPerReserve = 1 byte

  //Optional fields
  //gapReTx1 = 1 byte if slMaxNumPerReserve == 2
  //indexStartSbChReTx1 = 1 byte if slMaxNumPerReserve == 2
  //gapReTx2 = 1 byte if slMaxNumPerReserve == 3
  //indexStartSbChReTx1 = 1 byte if slMaxNumPerReserve == 3
  totalSize = 1 + 1 + 1 + 2 + 2 + 1 + 1 + 1;
  totalSize = (m_slMaxNumPerReserve == 2 ? totalSize + 2 : totalSize + 0); //only gapReTx1 and indexStartSbChReTx1
  totalSize = (m_slMaxNumPerReserve == 3 ? totalSize + 4 : totalSize + 0); // all gapReTx1, indexStartSbChReTx1, gapReTx2, and indexStartSbChReTx2

  return totalSize;
}

void
NrSlSciF1aHeader::Serialize (Buffer::Iterator start) const
{
  NS_ASSERT_MSG (EnsureMandConfig (), "All the mandatory fields must be set before serializing");
  Buffer::Iterator i = start;

  i.WriteU8 (m_priority);
  i.WriteU8 (m_mcs);
  i.WriteU8 (m_slSciStage2Format);
  i.WriteHtonU16 (m_slResourceReservePeriod);
  i.WriteHtonU16 (m_totalSubChannels);
  i.WriteU8 (m_indexStartSubChannel);
  i.WriteU8 (m_lengthSubChannel);

  i.WriteU8 (m_slMaxNumPerReserve);
  if (m_slMaxNumPerReserve == 2 || m_slMaxNumPerReserve == 3)
    {
      i.WriteU8 (m_gapReTx1);
      i.WriteU8 (m_indexStartSbChReTx1);
    }
  if (m_slMaxNumPerReserve == 3)
    {
      i.WriteU8 (m_gapReTx2);
      i.WriteU8 (m_indexStartSbChReTx2);
    }
}

uint32_t
NrSlSciF1aHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_priority = i.ReadU8 ();
  m_mcs = i.ReadU8 ();
  m_slSciStage2Format = i.ReadU8 ();
  m_slResourceReservePeriod = i.ReadNtohU16 ();
  m_totalSubChannels = i.ReadNtohU16 ();
  m_indexStartSubChannel = i.ReadU8 ();
  m_lengthSubChannel = i.ReadU8 ();

  m_slMaxNumPerReserve = i.ReadU8 ();
  if (m_slMaxNumPerReserve == 2 || m_slMaxNumPerReserve == 3)
    {
      m_gapReTx1 = i.ReadU8 ();
      m_indexStartSbChReTx1 = i.ReadU8 ();
    }
  if (m_slMaxNumPerReserve == 3)
    {
      m_gapReTx2 = i.ReadU8 ();
      m_indexStartSbChReTx2 = i.ReadU8 ();
    }

  return GetSerializedSize ();
}

bool
NrSlSciF1aHeader::operator == (const NrSlSciF1aHeader &b) const
{
  if (m_priority == b.m_priority
      && m_mcs == b.m_mcs
      && m_slSciStage2Format == b.m_slSciStage2Format
      && m_slResourceReservePeriod == b.m_slResourceReservePeriod
      && m_totalSubChannels == b.m_totalSubChannels
      && m_indexStartSubChannel == b.m_indexStartSubChannel
      && m_lengthSubChannel == b.m_lengthSubChannel
      && m_slMaxNumPerReserve == b.m_slMaxNumPerReserve
      && m_gapReTx1 == b.m_gapReTx1
      && m_gapReTx2 == b.m_gapReTx2
      && m_indexStartSbChReTx1 == b.m_indexStartSbChReTx1
      && m_indexStartSbChReTx2 == b.m_indexStartSbChReTx2
      )
    {
      return true;
    }

  return false;
}

}  // namespace ns3

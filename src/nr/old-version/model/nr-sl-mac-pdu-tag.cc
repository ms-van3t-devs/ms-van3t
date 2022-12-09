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


#include "nr-sl-mac-pdu-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrSlMacPduTag);


NrSlMacPduTag::NrSlMacPduTag (uint16_t rnti, SfnSf sfn, uint8_t symStart, uint8_t numSym, uint32_t tbSize, uint32_t dstL2Id)
  :  m_rnti (rnti), m_sfnSf (sfn), m_symStart (symStart), m_numSym (numSym), m_tbSize (tbSize), m_dstL2Id (dstL2Id)
{
}

TypeId
NrSlMacPduTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlMacPduTag")
    .SetParent<Tag> ()
    .AddConstructor<NrSlMacPduTag> ();
  return tid;
}

TypeId
NrSlMacPduTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
NrSlMacPduTag::GetSerializedSize (void) const
{
  return 2 + 8 + 1 + 1 + 4 + 4;
}

void
NrSlMacPduTag::Serialize (TagBuffer i) const
{
  i.WriteU16 (m_rnti);
  i.WriteU64 (m_sfnSf.GetEncoding ());
  i.WriteU8 (m_symStart);
  i.WriteU8 (m_numSym);
  i.WriteU32 (m_tbSize);
  i.WriteU32 (m_dstL2Id);
}

void
NrSlMacPduTag::Deserialize (TagBuffer i)
{
  m_rnti = i.ReadU16 ();
  uint64_t v = i.ReadU64 ();
  m_sfnSf.FromEncoding (v);

  m_symStart = (uint8_t)i.ReadU8 ();
  m_numSym = (uint8_t)i.ReadU8 ();
  m_tbSize = i.ReadU32 ();
  m_dstL2Id = i.ReadU32 ();
}

void
NrSlMacPduTag::Print (std::ostream &os) const
{
  os << "RNTI " << m_rnti
     << ", Destination id " << m_dstL2Id
     << ", Frame " << m_sfnSf.GetFrame ()
     << ", Subframe " << +m_sfnSf.GetSubframe ()
     << ", Slot " << m_sfnSf.GetSlot ()
     << ", PSCCH symbol start " << +m_symStart
     << ", Total number of symbols " << +m_numSym
     << ", TB size " << m_tbSize << " bytes";
}

uint16_t
NrSlMacPduTag::GetRnti () const
{
  return m_rnti;
}

void
NrSlMacPduTag::SetRnti (uint16_t rnti)
{
  m_rnti = rnti;
}

SfnSf
NrSlMacPduTag::GetSfn () const
{
  return m_sfnSf;
}

void
NrSlMacPduTag::SetSfn (SfnSf sfn)
{
  m_sfnSf = sfn;
}


uint8_t
NrSlMacPduTag::GetSymStart () const
{
  return m_symStart;
}


uint8_t
NrSlMacPduTag::GetNumSym () const
{
  return m_numSym;
}


void
NrSlMacPduTag::SetSymStart (uint8_t symStart)
{
  m_symStart = symStart;
}


void
NrSlMacPduTag::SetNumSym (uint8_t numSym)
{
  m_numSym = numSym;
}


uint32_t
NrSlMacPduTag::GetTbSize () const
{
  return m_tbSize;
}

void
NrSlMacPduTag::SetTbSize (uint32_t tbSize)
{
  m_tbSize = tbSize;
}

uint32_t
NrSlMacPduTag::GetDstL2Id () const
{
  return m_dstL2Id;
}

void
NrSlMacPduTag::SetDstL2Id (uint32_t dstL2Id)
{
  m_dstL2Id = dstL2Id;
}

} // namespace ns3


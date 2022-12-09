/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
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


#include "nr-phy-mac-common.h"
#include <ns3/log.h>
#include <ns3/uinteger.h>
#include <ns3/double.h>
#include <ns3/string.h>
#include <ns3/attribute-accessor-helper.h>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrPhyMacCommon");

void
SlotAllocInfo::Merge (const SlotAllocInfo &other)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (other.m_type != NONE && m_type != NONE);
  NS_ASSERT (other.m_sfnSf == m_sfnSf);

  if (other.m_type * m_type == 6)
    {
      m_type = BOTH;
    }

  m_numSymAlloc += other.m_numSymAlloc;

  for (auto extAlloc : other.m_varTtiAllocInfo)
    {
      m_varTtiAllocInfo.push_front (extAlloc);
    }

  // Sort over the symStart of the DCI (VarTtiAllocInfo::operator <)
  std::sort (m_varTtiAllocInfo.begin (), m_varTtiAllocInfo.end ());
}

bool
SlotAllocInfo::ContainsDataAllocation () const
{
  NS_LOG_FUNCTION (this);
  for (const auto & allocation : m_varTtiAllocInfo)
    {
      if (allocation.m_dci->m_type == DciInfoElementTdma::DATA)
        {
          return true;
        }
    }
  return false;
}

bool
SlotAllocInfo::ContainsDlCtrlAllocation() const
{
  NS_LOG_FUNCTION (this);

  for (const auto & allocation : m_varTtiAllocInfo)
    {
      if (allocation.m_dci->m_type == DciInfoElementTdma::CTRL && allocation.m_dci->m_format == DciInfoElementTdma::DL)
        {
          return true;
        }
    }
  return false;
}

bool
SlotAllocInfo::ContainsUlCtrlAllocation () const
{
  NS_LOG_FUNCTION (this);

  for (const auto & allocation : m_varTtiAllocInfo)
    {
      if (allocation.m_dci->m_type == DciInfoElementTdma::SRS)
        {
          return true;
        }
    }
  return false;
}

bool
SlotAllocInfo::operator < (const SlotAllocInfo &rhs) const
{
  return m_sfnSf < rhs.m_sfnSf;
}

std::ostream & operator<< (std::ostream & os, DciInfoElementTdma::DciFormat const & item)
{
  if (item == DciInfoElementTdma::DL)
    {
      os << "DL";
    }
  else if (item == DciInfoElementTdma::UL)
    {
      os << "UL";
    }
  else
    {
      os << "NA";
    }
  return os;
}

std::ostream &operator<< (std::ostream &os, const DlHarqInfo &item)
{
  if (item.IsReceivedOk ())
    {
      os << "ACK feedback ";
    }
  else
    {
      os << "NACK feedback ";
    }
  os << "for ProcessID: " << static_cast<uint32_t> (item.m_harqProcessId) << " of UE "
     << static_cast<uint32_t> (item.m_rnti);
  for (uint8_t stream = 0; stream < item.m_numRetx.size (); stream++)
    {
      os << "stream " << static_cast<uint32_t> (stream) << " Num Retx: " << static_cast<uint32_t> (item.m_numRetx [stream]);
    }

  os << " BWP index: " << static_cast<uint32_t> (item.m_bwpIndex);

  return os;
}

std::ostream &operator<< (std::ostream &os, const UlHarqInfo &item)
{
  if (item.IsReceivedOk ())
    {
      os << "ACK feedback ";
    }
  else
    {
      os << "NACK feedback ";
    }
  os << "for ProcessID: " << static_cast<uint32_t> (item.m_harqProcessId) << " of UE "
     << static_cast<uint32_t> (item.m_rnti) << " Num Retx: " << static_cast<uint32_t> (item.m_numRetx);

  return os;
}

std::ostream & operator<< (std::ostream & os, SfnSf const & item)
{
  os << "FrameNum: " << static_cast<uint32_t> (item.GetFrame ()) <<
    " SubFrameNum: " << static_cast<uint32_t> (item.GetSubframe ()) <<
    " SlotNum: " << static_cast<uint32_t> (item.GetSlot ());
  return os;
}

std::ostream &operator<< (std::ostream &os, const SlotAllocInfo &item)
{
  os << "Allocation for slot " << item.m_sfnSf << " total symbols allocated: "
     << item.m_numSymAlloc << " of type " << item.m_type
     << ", tti: " << item.m_varTtiAllocInfo.size ()
     << " composed by the following allocations: " << std::endl;
  for (const auto & alloc : item.m_varTtiAllocInfo)
    {
      std::string direction, type;
      if (alloc.m_dci->m_type == DciInfoElementTdma::CTRL)
        {
          type = "CTRL";
        }
      else if (alloc.m_dci->m_type == DciInfoElementTdma::SRS)
        {
          type = "SRS";
        }
      else
        {
          type = "DATA";
        }

      if (alloc.m_dci->m_format == DciInfoElementTdma::UL)
        {
          direction = "UL";
        }
      else
        {
          direction = "DL";
        }
      os << "[Allocation from sym " << static_cast<uint32_t> (alloc.m_dci->m_symStart) <<
            " to sym " << static_cast<uint32_t> (alloc.m_dci->m_numSym + alloc.m_dci->m_symStart) <<
            " direction " << direction << " type " << type << "]" << std::endl;
    }
  return os;
}

std::ostream &operator<<(std::ostream &os, const SlotAllocInfo::AllocationType &item)
{
  switch (item)
    {
    case SlotAllocInfo::NONE:
      os << "NONE";
      break;
    case SlotAllocInfo::DL:
      os << "DL";
      break;
    case SlotAllocInfo::UL:
      os << "UL";
      break;
    case SlotAllocInfo::BOTH:
      os << "BOTH";
      break;
    }

  return os;
}

std::ostream &operator<< (std::ostream &os, const DciInfoElementTdma &item)
{
  os << "RNTI=" << item.m_rnti << "|" << item.m_format << "|SYM=" << +item.m_symStart
     << "|NSYM=" << +item.m_numSym;

  for (uint8_t imcs = 0; imcs < item.m_mcs.size (); imcs++)
    {
      os << "|McsStream" << imcs << "=" << +item.m_mcs.at (imcs);
    }

  for (uint32_t itbs = 0; itbs < item.m_tbSize.size (); itbs++)
    {
      os << "|TBsStream" << itbs << "=" << item.m_tbSize.at (itbs);
    }

  for (uint8_t indi = 0; indi < item.m_ndi.size (); indi++)
    {
      os << "|NdiStream" << indi << "=" << +item.m_ndi.at (indi);
    }

  for (uint8_t irv = 0; irv < item.m_rv.size (); irv++)
    {
      os << "|RvStream" << irv << "=" << +item.m_rv.at (irv);
    }

  os << "|TYPE=" << item.m_type
     << "|BWP=" << +item.m_bwpIndex << "|HARQP=" << +item.m_harqProcess
     << "|RBG=";

  uint16_t start = 65000, end = 0;
  bool canPrint = false;
  for (uint32_t i = 0; i < item.m_rbgBitmask.size(); ++i)
    {
      if (item.m_rbgBitmask[i] == 1)
        {
          canPrint = true;
        }

      if (item.m_rbgBitmask[i] == 1 && end < i)
        {
          end = i;
        }
      if (item.m_rbgBitmask[i] == 1 && start > i)
        {
          start = i;
        }

      if (item.m_rbgBitmask[i] == 0 && canPrint)
        {
          os << "[" << +start << ";" << +end << "]";
          start = 65000;
          end = 0;
          canPrint = false;
        }
    }

  if (canPrint)
    {
      os << "[" << +start << ";" << +end << "]";
    }


  return os;
}

}

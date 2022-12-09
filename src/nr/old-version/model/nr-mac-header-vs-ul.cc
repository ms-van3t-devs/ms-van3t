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

#include "nr-mac-header-vs-ul.h"
#include <ns3/log.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrMacHeaderVsUl);
NS_LOG_COMPONENT_DEFINE("NrMacHeaderVsUl");

TypeId
NrMacHeaderVsUl::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrMacHeaderVsUl")
    .SetParent<NrMacHeaderVs> ()
    .AddConstructor <NrMacHeaderVsUl> ()
    ;
  return tid;
}

TypeId NrMacHeaderVsUl::GetInstanceTypeId () const
{
  return GetTypeId ();
}

NrMacHeaderVsUl::NrMacHeaderVsUl()
{
  NS_LOG_FUNCTION (this);
}

NrMacHeaderVsUl::~NrMacHeaderVsUl ()
{
  NS_LOG_FUNCTION (this);
}

void
NrMacHeaderVsUl::SetLcId (uint8_t lcId)
{
  if (lcId <= 32)
    {
      NrMacHeaderVs::SetLcId (lcId);
    }
  else
    {
      m_lcid = lcId;
      NS_ASSERT (IsVariableSizeHeader ());
    }
}

bool
NrMacHeaderVsUl::IsVariableSizeHeader () const
{
  if (m_lcid <= 32) return true;
  if (m_lcid == MULTIPLE_ENTRY_PHR_FOUR_OCTECT) return true;
  if (m_lcid == MULTIPLE_ENTRY_PHR_ONE_OCTET) return true;
  if (m_lcid == LONG_TRUNCATED_BSR) return true;
  if (m_lcid == LONG_BSR) return true;

  return false;
}


} // namespace ns3


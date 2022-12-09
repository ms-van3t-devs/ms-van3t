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

#include "nr-mac-header-fs-ul.h"
#include <ns3/log.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrMacHeaderFsUl);
NS_LOG_COMPONENT_DEFINE("NrMacHeaderFsUl");

TypeId
NrMacHeaderFsUl::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrMacHeaderFsUl")
    .SetParent<NrMacHeaderFs> ()
    .AddConstructor <NrMacHeaderFsUl> ()
    ;
  return tid;
}

TypeId NrMacHeaderFsUl::GetInstanceTypeId () const
{
  return GetTypeId ();
}

NrMacHeaderFsUl::NrMacHeaderFsUl()
{
  NS_LOG_FUNCTION (this);
}

NrMacHeaderFsUl::~NrMacHeaderFsUl ()
{
  NS_LOG_FUNCTION (this);
}

void
NrMacHeaderFsUl::SetLcId (uint8_t lcId)
{
  m_lcid = lcId;
  NS_ASSERT (IsFixedSizeHeader ());
}

bool
NrMacHeaderFsUl::IsFixedSizeHeader () const
{
  if (m_lcid == BIT_RATE_QUERY) return true;
  if (m_lcid == CONFIGURED_GRANT_CONFIRMATION) return true;
  if (m_lcid == SINGLE_ENTRY_PHR) return true;
  if (m_lcid == C_RNTI) return true;
  if (m_lcid == SHORT_TRUNCATED_BSR) return true;
  if (m_lcid == SHORT_BSR) return true;
  if (m_lcid == PADDING) return true;

  return false;
}
} // namespace ns3


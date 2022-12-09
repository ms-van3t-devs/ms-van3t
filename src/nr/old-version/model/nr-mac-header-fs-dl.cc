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

#include "nr-mac-header-fs-dl.h"
#include <ns3/log.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrMacHeaderFsDl);
NS_LOG_COMPONENT_DEFINE("NrMacHeaderFsDl");

TypeId
NrMacHeaderFsDl::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrMacHeaderFsDl")
    .SetParent<NrMacHeaderFs> ()
    .AddConstructor <NrMacHeaderFsDl> ()
    ;
  return tid;
}

TypeId NrMacHeaderFsDl::GetInstanceTypeId () const
{
  return GetTypeId ();
}

NrMacHeaderFsDl::NrMacHeaderFsDl()
{
  NS_LOG_FUNCTION (this);
}

NrMacHeaderFsDl::~NrMacHeaderFsDl ()
{
  NS_LOG_FUNCTION (this);
}

bool
NrMacHeaderFsDl::IsFixedSizeHeader () const
{
  if (m_lcid == RECOMMENDED_BIT_RATE) return true;
  if (m_lcid == SP_ZP_CSI_RS) return true;
  if (m_lcid == PUCCH_SPATIAL_RELATION) return true;
  if (m_lcid == SP_CSI_REPORT) return true;
  if (m_lcid == TCI_STATE_INDICATION_PDCCH) return true;
  if (m_lcid == DUPLICATION) return true;
  if (m_lcid == SCELL_FOUR_OCTET) return true;
  if (m_lcid == SCELL_ONE_OCTET) return true;
  if (m_lcid == LONG_DRX) return true;
  if (m_lcid == DRX) return true;
  if (m_lcid == TIMING_ADVANCE) return true;
  if (m_lcid == UE_CONTENTION_RESOLUTION) return true;
  if (m_lcid == PADDING) return true;

  return false;
}

void
NrMacHeaderFsDl::SetLcId (uint8_t v)
{
  // Here we are sure, thanks to the compiler, that v is one of the allowed values
  m_lcid = v;
  NS_ASSERT (IsFixedSizeHeader ());
}

} // namespace ns3


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

#include "nr-mac-header-vs-dl.h"
#include <ns3/log.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (NrMacHeaderVsDl);
NS_LOG_COMPONENT_DEFINE("NrMacHeaderVsDl");

TypeId
NrMacHeaderVsDl::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrMacHeaderVsDl")
    .SetParent<NrMacHeaderVs> ()
    .AddConstructor <NrMacHeaderVsDl> ()
    ;
  return tid;
}

TypeId NrMacHeaderVsDl::GetInstanceTypeId () const
{
  return GetTypeId ();
}

NrMacHeaderVsDl::NrMacHeaderVsDl()
{
  NS_LOG_FUNCTION (this);
}

NrMacHeaderVsDl::~NrMacHeaderVsDl ()
{
  NS_LOG_FUNCTION (this);
}

void
NrMacHeaderVsDl::SetLcId (uint8_t lcId)
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
NrMacHeaderVsDl::IsVariableSizeHeader () const
{
  if (m_lcid <= 32) return true;
  if (m_lcid == SP_SRS) return true;
  if (m_lcid == TCI_STATES_PDSCH) return true;
  if (m_lcid == APERIODIC_CSI) return true;
  if (m_lcid == SP_CSI_RS_IM) return true;
  return false;
}

} // namespace ns3


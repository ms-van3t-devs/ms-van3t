/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Danilo Abrignani
 *
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
 *
 * Author: Danilo Abrignani <danilo.abrignani@unibo.it>
 */

#include "cv2x_component-carrier.h"
#include <ns3/uinteger.h>
#include <ns3/boolean.h>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/cv2x_lte-enb-phy.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_ComponentCarrier");

NS_OBJECT_ENSURE_REGISTERED ( cv2x_ComponentCarrier);

TypeId cv2x_ComponentCarrier::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_ComponentCarrier")
    .SetParent<Object> ()
    .AddConstructor<cv2x_ComponentCarrier> ()
    .AddAttribute ("UlBandwidth",
                   "Uplink Transmission Bandwidth Configuration in number of Resource Blocks",
                   UintegerValue (25),
                   MakeUintegerAccessor (&cv2x_ComponentCarrier::SetUlBandwidth,
                                         &cv2x_ComponentCarrier::GetUlBandwidth),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlBandwidth",
                   "Downlink Transmission Bandwidth Configuration in number of Resource Blocks",
                   UintegerValue (25),
                   MakeUintegerAccessor (&cv2x_ComponentCarrier::SetDlBandwidth,
                                         &cv2x_ComponentCarrier::GetDlBandwidth),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlEarfcn",
                   "Downlink E-UTRA Absolute Radio Frequency Channel Number (EARFCN) "
                   "as per 3GPP 36.101 Section 5.7.3. ",
                   UintegerValue (100),
                   MakeUintegerAccessor (&cv2x_ComponentCarrier::m_dlEarfcn),
                   MakeUintegerChecker<uint32_t> (0, 262143))
    .AddAttribute ("UlEarfcn",
                   "Uplink E-UTRA Absolute Radio Frequency Channel Number (EARFCN) "
                   "as per 3GPP 36.101 Section 5.7.3. ",
                   UintegerValue (18100),
                   MakeUintegerAccessor (&cv2x_ComponentCarrier::m_ulEarfcn),
                   MakeUintegerChecker<uint32_t> (18000, 262143))
    .AddAttribute ("CsgId",
                   "The Closed Subscriber Group (CSG) identity that this eNodeB belongs to",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_ComponentCarrier::SetCsgId,
                                         &cv2x_ComponentCarrier::GetCsgId),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CsgIndication",
                   "If true, only UEs which are members of the CSG (i.e. same CSG ID) "
                   "can gain access to the eNodeB, therefore enforcing closed access mode. "
                   "Otherwise, the eNodeB operates as a non-CSG cell and implements open access mode.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&cv2x_ComponentCarrier::SetCsgIndication,
                                        &cv2x_ComponentCarrier::GetCsgIndication),
                   MakeBooleanChecker ())
    .AddAttribute ("PrimaryCarrier",
                   "If true, this Carrier Component will be the Primary Carrier Component (PCC) "
                   "Only one PCC per eNodeB is (currently) allowed",
                   BooleanValue (false),
                   MakeBooleanAccessor (&cv2x_ComponentCarrier::SetAsPrimary,
                                        &cv2x_ComponentCarrier::IsPrimary),
                   MakeBooleanChecker ())
  ;
  return tid;
}
cv2x_ComponentCarrier::cv2x_ComponentCarrier ()
  : m_isConstructed (false)
{
  NS_LOG_FUNCTION (this);
}

cv2x_ComponentCarrier::~cv2x_ComponentCarrier (void)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_ComponentCarrier::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}

uint8_t
cv2x_ComponentCarrier::GetUlBandwidth () const
{
  return m_ulBandwidth;
}

void
cv2x_ComponentCarrier::SetUlBandwidth (uint8_t bw)
{
  NS_LOG_FUNCTION (this << uint16_t (bw));
  switch (bw)
    {
    case 6:
    case 15:
    case 25:
    case 50:
    case 75:
    case 100:
      m_ulBandwidth = bw;
      break;

    default:
      NS_FATAL_ERROR ("Invalid bandwidth value " << (uint16_t) bw);
      break;
    }
}

uint8_t
cv2x_ComponentCarrier::GetDlBandwidth () const
{
  return m_dlBandwidth;
}

void
cv2x_ComponentCarrier::SetDlBandwidth (uint8_t bw)
{
  NS_LOG_FUNCTION (this << uint16_t (bw));
  switch (bw)
    {
    case 6:
    case 15:
    case 25:
    case 50:
    case 75:
    case 100:
      m_dlBandwidth = bw;
      break;

    default:
      NS_FATAL_ERROR ("Invalid bandwidth value " << (uint16_t) bw);
      break;
    }
}

uint32_t
cv2x_ComponentCarrier::GetDlEarfcn () const
{
  return m_dlEarfcn;
}

void
cv2x_ComponentCarrier::SetDlEarfcn (uint32_t earfcn)
{
  NS_LOG_FUNCTION (this << earfcn);
  m_dlEarfcn = earfcn;
}

uint32_t
cv2x_ComponentCarrier::GetUlEarfcn () const
{
  return m_ulEarfcn;
}

void
cv2x_ComponentCarrier::SetUlEarfcn (uint32_t earfcn)
{
  NS_LOG_FUNCTION (this << earfcn);
  m_ulEarfcn = earfcn;
}

uint32_t
cv2x_ComponentCarrier::GetCsgId () const
{
  return m_csgId;
}

void
cv2x_ComponentCarrier::SetCsgId (uint32_t csgId)
{
  NS_LOG_FUNCTION (this << csgId);
  m_csgId = csgId;
}

bool
cv2x_ComponentCarrier::GetCsgIndication () const
{
  return m_csgIndication;
}

void
cv2x_ComponentCarrier::SetCsgIndication (bool csgIndication)
{
  NS_LOG_FUNCTION (this << csgIndication);
  m_csgIndication = csgIndication;
}

bool
cv2x_ComponentCarrier::IsPrimary () const
{
  return m_primaryCarrier;
}

void
cv2x_ComponentCarrier::SetAsPrimary (bool primaryCarrier)
{
  NS_LOG_FUNCTION (this << primaryCarrier);
  m_primaryCarrier = primaryCarrier;
}

void
cv2x_ComponentCarrier::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
}

} // namespace ns3



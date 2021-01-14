/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Piotr Gawlowicz
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
 * Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>
 *
 */

#include "cv2x_lte-ffr-algorithm.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteFfrAlgorithm");

/// Type 0 RGB allocation
static const int Type0AllocationRbg[4] = {
  10,       // RGB size 1
  26,       // RGB size 2
  63,       // RGB size 3
  110       // RGB size 4
};  // see table 7.1.6.1-1 of 3GPP TS 36.213

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteFfrAlgorithm);


cv2x_LteFfrAlgorithm::cv2x_LteFfrAlgorithm () :
  m_needReconfiguration (true)
{
}


cv2x_LteFfrAlgorithm::~cv2x_LteFfrAlgorithm ()
{
}


TypeId
cv2x_LteFfrAlgorithm::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::cv2x_LteFfrAlgorithm")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddAttribute ("FrCellTypeId",
                   "Downlink FR cell type ID for automatic configuration,"
                   "default value is 0 and it means that user needs to configure FR algorithm manually,"
                   "if it is set to 1,2 or 3 FR algorithm will be configured automatically",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteFfrAlgorithm::SetFrCellTypeId,&cv2x_LteFfrAlgorithm::GetFrCellTypeId),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("EnabledInUplink",
                   "If FR algorithm will also work in Uplink, default value true",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_LteFfrAlgorithm::m_enabledInUplink),
                   MakeBooleanChecker ())
  ;
  return tid;
}


void
cv2x_LteFfrAlgorithm::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

uint8_t
cv2x_LteFfrAlgorithm::GetUlBandwidth () const
{
  NS_LOG_FUNCTION (this);
  return m_ulBandwidth;
}

void
cv2x_LteFfrAlgorithm::SetUlBandwidth (uint8_t bw)
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
      NS_FATAL_ERROR ("invalid bandwidth value " << (uint16_t) bw);
      break;
    }
}

uint8_t
cv2x_LteFfrAlgorithm::GetDlBandwidth () const
{
  NS_LOG_FUNCTION (this);
  return m_dlBandwidth;
}

void
cv2x_LteFfrAlgorithm::SetDlBandwidth (uint8_t bw)
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
      NS_FATAL_ERROR ("invalid bandwidth value " << (uint16_t) bw);
      break;
    }
}

void
cv2x_LteFfrAlgorithm::SetFrCellTypeId (uint8_t cellTypeId)
{
  NS_LOG_FUNCTION (this << uint16_t (cellTypeId));
  m_frCellTypeId = cellTypeId;
  m_needReconfiguration = true;
}

uint8_t
cv2x_LteFfrAlgorithm::GetFrCellTypeId () const
{
  NS_LOG_FUNCTION (this);
  return m_frCellTypeId;
}

int
cv2x_LteFfrAlgorithm::GetRbgSize (int dlbandwidth)
{
  for (int i = 0; i < 4; i++)
    {
      if (dlbandwidth < Type0AllocationRbg[i])
        {
          return (i + 1);
        }
    }

  return (-1);
}

void
cv2x_LteFfrAlgorithm::DoSetCellId (uint16_t cellId )
{
  NS_LOG_FUNCTION (this);
  m_cellId = cellId;
}

void
cv2x_LteFfrAlgorithm::DoSetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  NS_LOG_FUNCTION (this);
  SetDlBandwidth (dlBandwidth);
  SetUlBandwidth (ulBandwidth);
}

} // end of namespace ns3

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Budiarto Herman
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include "cv2x_no-op-handover-algorithm.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_NoOpHandoverAlgorithm");

NS_OBJECT_ENSURE_REGISTERED (cv2x_NoOpHandoverAlgorithm);


cv2x_NoOpHandoverAlgorithm::cv2x_NoOpHandoverAlgorithm ()
  : m_handoverManagementSapUser (0)
{
  NS_LOG_FUNCTION (this);
  m_handoverManagementSapProvider = new cv2x_MemberLteHandoverManagementSapProvider<cv2x_NoOpHandoverAlgorithm> (this);
}


cv2x_NoOpHandoverAlgorithm::~cv2x_NoOpHandoverAlgorithm ()
{
  NS_LOG_FUNCTION (this);
}


void
cv2x_NoOpHandoverAlgorithm::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_handoverManagementSapProvider;
}


TypeId
cv2x_NoOpHandoverAlgorithm::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::cv2x_NoOpHandoverAlgorithm")
    .SetParent<cv2x_LteHandoverAlgorithm> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_NoOpHandoverAlgorithm> ()
  ;
  return tid;
}


void
cv2x_NoOpHandoverAlgorithm::SetLteHandoverManagementSapUser (cv2x_LteHandoverManagementSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_handoverManagementSapUser = s;
}


cv2x_LteHandoverManagementSapProvider*
cv2x_NoOpHandoverAlgorithm::GetLteHandoverManagementSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_handoverManagementSapProvider;
}


void
cv2x_NoOpHandoverAlgorithm::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  cv2x_LteHandoverAlgorithm::DoInitialize ();
}


void
cv2x_NoOpHandoverAlgorithm::DoReportUeMeas (uint16_t rnti,
                                       cv2x_LteRrcSap::MeasResults measResults)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) measResults.measId);
}


} // end of namespace ns3

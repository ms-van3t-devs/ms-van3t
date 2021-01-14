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

#include "cv2x_lte-fr-no-op-algorithm.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteFrNoOpAlgorithm");

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteFrNoOpAlgorithm);


cv2x_LteFrNoOpAlgorithm::cv2x_LteFrNoOpAlgorithm ()
  : m_ffrSapUser (0), m_ffrRrcSapUser (0)
{
  NS_LOG_FUNCTION (this);
  m_ffrSapProvider = new cv2x_MemberLteFfrSapProvider<cv2x_LteFrNoOpAlgorithm> (this);
  m_ffrRrcSapProvider = new cv2x_MemberLteFfrRrcSapProvider<cv2x_LteFrNoOpAlgorithm> (this);
}


cv2x_LteFrNoOpAlgorithm::~cv2x_LteFrNoOpAlgorithm ()
{
  NS_LOG_FUNCTION (this);
}


void
cv2x_LteFrNoOpAlgorithm::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ffrSapProvider;
  delete m_ffrRrcSapProvider;
}


TypeId
cv2x_LteFrNoOpAlgorithm::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::cv2x_LteFrNoOpAlgorithm")
    .SetParent<cv2x_LteFfrAlgorithm> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteFrNoOpAlgorithm> ()
  ;
  return tid;
}


void
cv2x_LteFrNoOpAlgorithm::SetLteFfrSapUser (cv2x_LteFfrSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ffrSapUser = s;
}


cv2x_LteFfrSapProvider*
cv2x_LteFrNoOpAlgorithm::GetLteFfrSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrSapProvider;
}

void
cv2x_LteFrNoOpAlgorithm::SetLteFfrRrcSapUser (cv2x_LteFfrRrcSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ffrRrcSapUser = s;
}


cv2x_LteFfrRrcSapProvider*
cv2x_LteFrNoOpAlgorithm::GetLteFfrRrcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrRrcSapProvider;
}


void
cv2x_LteFrNoOpAlgorithm::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  cv2x_LteFfrAlgorithm::DoInitialize ();
}

void
cv2x_LteFrNoOpAlgorithm::Reconfigure ()
{
  NS_LOG_FUNCTION (this);
}

std::vector <bool>
cv2x_LteFrNoOpAlgorithm::DoGetAvailableDlRbg ()
{
  NS_LOG_FUNCTION (this);
  std::vector <bool> rbgMap;
  int rbgSize = GetRbgSize (m_dlBandwidth);
  rbgMap.resize (m_dlBandwidth/rbgSize, false);
  return rbgMap;
}

bool
cv2x_LteFrNoOpAlgorithm::DoIsDlRbgAvailableForUe (int i, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  return true;
}

std::vector <bool>
cv2x_LteFrNoOpAlgorithm::DoGetAvailableUlRbg ()
{
  NS_LOG_FUNCTION (this);
  std::vector <bool> rbgMap;
  rbgMap.resize (m_ulBandwidth, false);
  return rbgMap;
}

bool
cv2x_LteFrNoOpAlgorithm::DoIsUlRbgAvailableForUe (int i, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
cv2x_LteFrNoOpAlgorithm::DoReportDlCqiInfo (const struct cv2x_FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
cv2x_LteFrNoOpAlgorithm::DoReportUlCqiInfo (const struct cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
cv2x_LteFrNoOpAlgorithm::DoReportUlCqiInfo (std::map <uint16_t, std::vector <double> > ulCqiMap)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

uint8_t
cv2x_LteFrNoOpAlgorithm::DoGetTpc (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  return 1; // 1 is mapped to 0 for Accumulated mode, and to -1 in Absolute mode TS36.213 Table 5.1.1.1-2
}

uint8_t
cv2x_LteFrNoOpAlgorithm::DoGetMinContinuousUlBandwidth ()
{
  NS_LOG_FUNCTION (this);
  return m_ulBandwidth;
}

void
cv2x_LteFrNoOpAlgorithm::DoReportUeMeas (uint16_t rnti,
                                    cv2x_LteRrcSap::MeasResults measResults)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) measResults.measId);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

void
cv2x_LteFrNoOpAlgorithm::DoRecvLoadInformation (cv2x_EpcX2Sap::LoadInformationParams params)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_WARN ("Method should not be called, because it is empty");
}

} // end of namespace ns3

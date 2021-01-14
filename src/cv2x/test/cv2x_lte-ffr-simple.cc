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

#include "cv2x_lte-ffr-simple.h"
#include <ns3/log.h>
#include "ns3/cv2x_lte-rrc-sap.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteFfrSimple");

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteFfrSimple);


cv2x_LteFfrSimple::cv2x_LteFfrSimple ()
  : m_ffrSapUser (0),
    m_ffrRrcSapUser (0),
    m_dlOffset (0),
    m_dlSubBand (0),
    m_ulOffset (0),
    m_ulSubBand (0),
    m_measId (0),
    m_changePdschConfigDedicated (false),
    m_tpc (1),
    m_tpcNum (0),
    m_accumulatedMode (false)
{
  NS_LOG_FUNCTION (this);
  m_ffrSapProvider = new cv2x_MemberLteFfrSapProvider<cv2x_LteFfrSimple> (this);
  m_ffrRrcSapProvider = new cv2x_MemberLteFfrRrcSapProvider<cv2x_LteFfrSimple> (this);
}


cv2x_LteFfrSimple::~cv2x_LteFfrSimple ()
{
  NS_LOG_FUNCTION (this);
}


void
cv2x_LteFfrSimple::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ffrSapProvider;
  delete m_ffrRrcSapProvider;
}


TypeId
cv2x_LteFfrSimple::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::cv2x_LteFfrSimple")
    .SetParent<cv2x_LteFfrAlgorithm> ()
    .AddConstructor<cv2x_LteFfrSimple> ()
    .AddAttribute ("UlSubBandOffset",
                   "Uplink Offset in number of Resource Block Groups",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteFfrSimple::m_ulOffset),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("UlSubBandwidth",
                   "Uplink Transmission SubBandwidth Configuration in number of Resource Block Groups",
                   UintegerValue (25),
                   MakeUintegerAccessor (&cv2x_LteFfrSimple::m_ulSubBand),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlSubBandOffset",
                   "Downlink Offset in number of Resource Block Groups",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteFfrSimple::m_dlOffset),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlSubBandwidth",
                   "Downlink Transmission SubBandwidth Configuration in number of Resource Block Groups",
                   UintegerValue (12),
                   MakeUintegerAccessor (&cv2x_LteFfrSimple::m_dlSubBand),
                   MakeUintegerChecker<uint8_t> ())
    .AddTraceSource ("ChangePdschConfigDedicated",
                     "trace fired upon change of PdschConfigDedicated",
                     MakeTraceSourceAccessor (&cv2x_LteFfrSimple::m_changePdschConfigDedicatedTrace),
                     "ns3::cv2x_LteFfrSimple::PdschTracedCallback")
  ;
  return tid;
}


void
cv2x_LteFfrSimple::SetLteFfrSapUser (cv2x_LteFfrSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ffrSapUser = s;
}


cv2x_LteFfrSapProvider*
cv2x_LteFfrSimple::GetLteFfrSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrSapProvider;
}

void
cv2x_LteFfrSimple::SetLteFfrRrcSapUser (cv2x_LteFfrRrcSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ffrRrcSapUser = s;
}


cv2x_LteFfrRrcSapProvider*
cv2x_LteFfrSimple::GetLteFfrRrcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrRrcSapProvider;
}


void
cv2x_LteFfrSimple::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  cv2x_LteFfrAlgorithm::DoInitialize ();

  NS_LOG_LOGIC (this << " requesting Event A4 measurements"
                     << " (threshold = 0" << ")");
  cv2x_LteRrcSap::ReportConfigEutra reportConfig;
  reportConfig.eventId = cv2x_LteRrcSap::ReportConfigEutra::EVENT_A1;
  reportConfig.threshold1.choice = cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ;
  reportConfig.threshold1.range = 0;
  reportConfig.triggerQuantity = cv2x_LteRrcSap::ReportConfigEutra::RSRQ;
  reportConfig.reportInterval = cv2x_LteRrcSap::ReportConfigEutra::MS120;
  m_measId = m_ffrRrcSapUser->AddUeMeasReportConfigForFfr (reportConfig);

  m_pdschConfigDedicated.pa = cv2x_LteRrcSap::PdschConfigDedicated::dB0;
}

void
cv2x_LteFfrSimple::Reconfigure ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteFfrSimple::ChangePdschConfigDedicated (bool change)
{
  m_changePdschConfigDedicated = change;
}

void
cv2x_LteFfrSimple::SetPdschConfigDedicated (cv2x_LteRrcSap::PdschConfigDedicated pdschConfigDedicated)
{
  m_pdschConfigDedicated = pdschConfigDedicated;
}

void
cv2x_LteFfrSimple::SetTpc (uint32_t tpc, uint32_t num, bool acculumatedMode)
{
  m_tpc = tpc;
  m_tpcNum = num;
  m_accumulatedMode = acculumatedMode;
}

std::vector <bool>
cv2x_LteFfrSimple::DoGetAvailableDlRbg ()
{
  NS_LOG_FUNCTION (this);

  if (m_dlRbgMap.empty ())
    {
      int rbgSize = GetRbgSize (m_dlBandwidth);
      m_dlRbgMap.resize (m_dlBandwidth / rbgSize, true);

      for (uint8_t i = m_dlOffset; i < (m_dlOffset + m_dlSubBand); i++)
        {
          m_dlRbgMap[i] = false;

        }
    }

  return m_dlRbgMap;
}

bool
cv2x_LteFfrSimple::DoIsDlRbgAvailableForUe (int i, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  return true;
}

std::vector <bool>
cv2x_LteFfrSimple::DoGetAvailableUlRbg ()
{
  NS_LOG_FUNCTION (this);

  if (m_ulRbgMap.empty ())
    {
      m_ulRbgMap.resize (m_ulBandwidth, true);

      for (uint8_t i = m_ulOffset; i < (m_ulOffset + m_ulSubBand); i++)
        {
          m_ulRbgMap[i] = false;
        }
    }

  return m_ulRbgMap;
}

bool
cv2x_LteFfrSimple::DoIsUlRbgAvailableForUe (int i, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  return true;
}

void
cv2x_LteFfrSimple::DoReportDlCqiInfo (const struct cv2x_FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteFfrSimple::DoReportUlCqiInfo (const struct cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteFfrSimple::DoReportUlCqiInfo (std::map <uint16_t, std::vector <double> > ulCqiMap)
{
  NS_LOG_FUNCTION (this);
}

uint8_t
cv2x_LteFfrSimple::DoGetTpc (uint16_t rnti)
{
  NS_LOG_FUNCTION (this);

  if (m_accumulatedMode)
    {
      if (m_tpcNum > 0)
        {
          m_tpcNum--;
          return m_tpc;
        }
      else
        {
          return 1;
        }
    }
  else
    {
      return m_tpc;
    }

  return 1; // 1 is mapped to 0 for Accumulated mode, and to -1 in Absolute mode TS36.213 Table 5.1.1.1-2
}

uint8_t
cv2x_LteFfrSimple::DoGetMinContinuousUlBandwidth ()
{
  NS_LOG_FUNCTION (this);
  return m_ulBandwidth;
}

void
cv2x_LteFfrSimple::DoReportUeMeas (uint16_t rnti,
                              cv2x_LteRrcSap::MeasResults measResults)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) measResults.measId);

  std::map<uint16_t, cv2x_LteRrcSap::PdschConfigDedicated>::iterator it;

  it = m_ues.find (rnti);

  if (it == m_ues.end ())
    {
      cv2x_LteRrcSap::PdschConfigDedicated pdschConfigDedicated;
      pdschConfigDedicated.pa = cv2x_LteRrcSap::PdschConfigDedicated::dB0;
      m_ues.insert (std::pair<uint16_t, cv2x_LteRrcSap::PdschConfigDedicated> (rnti,
                                                                          pdschConfigDedicated));
    }

  if (m_changePdschConfigDedicated)
    {
      UpdatePdschConfigDedicated ();
    }
}

void
cv2x_LteFfrSimple::UpdatePdschConfigDedicated ()
{
  NS_LOG_FUNCTION (this);

  std::map<uint16_t, cv2x_LteRrcSap::PdschConfigDedicated>::iterator it;
  for (it = m_ues.begin (); it != m_ues.end (); it++)
    {
      if (it->second.pa != m_pdschConfigDedicated.pa)
        {
          m_changePdschConfigDedicatedTrace (it->first, m_pdschConfigDedicated.pa);
          cv2x_LteRrcSap::PdschConfigDedicated pdschConfigDedicated = m_pdschConfigDedicated;
          m_ffrRrcSapUser->SetPdschConfigDedicated (it->first, pdschConfigDedicated );
        }
    }
}

void
cv2x_LteFfrSimple::DoRecvLoadInformation (cv2x_EpcX2Sap::LoadInformationParams params)
{
  NS_LOG_FUNCTION (this);
}

} // end of namespace ns3

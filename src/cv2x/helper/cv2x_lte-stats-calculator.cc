/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jnin@cttc.es>
 * Modified by: NIST
 */

#include "cv2x_lte-stats-calculator.h"

#include <ns3/log.h>
#include <ns3/config.h>
#include <ns3/cv2x_lte-enb-rrc.h>
#include <ns3/cv2x_lte-ue-rrc.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-ue-net-device.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteStatsCalculator");

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteStatsCalculator);

cv2x_LteStatsCalculator::cv2x_LteStatsCalculator ()
  : m_dlOutputFilename (""),
    m_ulOutputFilename ("")
{
  // Nothing to do here

}

cv2x_LteStatsCalculator::~cv2x_LteStatsCalculator ()
{
  // Nothing to do here
}


TypeId
cv2x_LteStatsCalculator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteStatsCalculator")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteStatsCalculator> ()
  ;
  return tid;
}


void
cv2x_LteStatsCalculator::SetUlOutputFilename (std::string outputFilename)
{
  m_ulOutputFilename = outputFilename;
}

std::string
cv2x_LteStatsCalculator::GetUlOutputFilename (void)
{
  return m_ulOutputFilename;
}

void
cv2x_LteStatsCalculator::SetDlOutputFilename (std::string outputFilename)
{
  m_dlOutputFilename = outputFilename;
}

std::string
cv2x_LteStatsCalculator::GetDlOutputFilename (void)
{
  return m_dlOutputFilename;
}

void
cv2x_LteStatsCalculator::SetSlUeOutputFilename (std::string outputFilename)
{
  m_slUeOutputFilename = outputFilename;
}

std::string
cv2x_LteStatsCalculator::GetSlUeOutputFilename (void)
{
  return m_slUeOutputFilename;
}

void
cv2x_LteStatsCalculator::SetSlPscchOutputFilename (std::string outputFilename)
{
  m_slPscchOutputFilename = outputFilename;
}

std::string
cv2x_LteStatsCalculator::GetSlPscchOutputFilename (void)
{
  return m_slPscchOutputFilename;
}

void
cv2x_LteStatsCalculator::SetSlSchUeOutputFilename (std::string outputFilename)
{
  m_slSchUeOutputFilename = outputFilename;
}

std::string
cv2x_LteStatsCalculator::GetSlSchUeOutputFilename (void)
{
  return m_slSchUeOutputFilename;
}

void
cv2x_LteStatsCalculator::SetSlOutputFilename (std::string outputFilename)
{
  m_slOutputFilename = outputFilename;
}

std::string
cv2x_LteStatsCalculator::GetSlOutputFilename (void)
{
  return m_slOutputFilename;
}

bool
cv2x_LteStatsCalculator::ExistsImsiPath (std::string path)
{
  if (m_pathImsiMap.find (path) == m_pathImsiMap.end () )
    {
      return false;
    }
  else
    {
      return true;
    }
}

void
cv2x_LteStatsCalculator::SetImsiPath (std::string path, uint64_t imsi)
{
  NS_LOG_FUNCTION (this << path << imsi);
  m_pathImsiMap[path] = imsi;
}

uint64_t
cv2x_LteStatsCalculator::GetImsiPath (std::string path)
{
  return m_pathImsiMap.find (path)->second;
}

bool
cv2x_LteStatsCalculator::ExistsCellIdPath (std::string path)
{
  if (m_pathCellIdMap.find (path) == m_pathCellIdMap.end () )
    {
      return false;
    }
  else
    {
      return true;
    }
}

void
cv2x_LteStatsCalculator::SetCellIdPath (std::string path, uint16_t cellId)
{
  NS_LOG_FUNCTION (this << path << cellId);
  m_pathCellIdMap[path] = cellId;
}

uint16_t
cv2x_LteStatsCalculator::GetCellIdPath (std::string path)
{
  return m_pathCellIdMap.find (path)->second;
}


uint64_t
cv2x_LteStatsCalculator::FindImsiFromEnbRlcPath (std::string path)
{
  NS_LOG_FUNCTION (path);
  // Sample path input:
  // /NodeList/#NodeId/DeviceList/#DeviceId/cv2x_LteEnbRrc/UeMap/#C-RNTI/DataRadioBearerMap/#LCID/cv2x_LteRlc/RxPDU

  // We retrieve the cv2x_UeManager associated to the C-RNTI and perform the IMSI lookup
  std::string ueMapPath = path.substr (0, path.find ("/DataRadioBearerMap"));
  Config::MatchContainer match = Config::LookupMatches (ueMapPath);

  if (match.GetN () != 0)
    {
      Ptr<Object> ueInfo = match.Get (0);
      NS_LOG_LOGIC ("FindImsiFromEnbRlcPath: " << path << ", " << ueInfo->GetObject<cv2x_UeManager> ()->GetImsi ());
      return ueInfo->GetObject<cv2x_UeManager> ()->GetImsi ();
    }
  else
    {
      NS_FATAL_ERROR ("Lookup " << ueMapPath << " got no matches");
    }
}

uint64_t
cv2x_LteStatsCalculator::FindImsiFromUePhy (std::string path)
{
  NS_LOG_FUNCTION (path);
  // Sample path input:
  // /NodeList/#NodeId/DeviceList/#DeviceId/cv2x_LteUePhy

  // We retrieve the UeInfo associated to the C-RNTI and perform the IMSI lookup
  std::string ueRlcPath = path.substr (0, path.find ("/cv2x_LteUePhy"));
  ueRlcPath += "/cv2x_LteUeRrc";
  Config::MatchContainer match = Config::LookupMatches (ueRlcPath);

  if (match.GetN () != 0)
    {
      Ptr<Object> ueRrc = match.Get (0);
      return ueRrc->GetObject<cv2x_LteUeRrc> ()->GetImsi ();
    }
  else
    {
      NS_FATAL_ERROR ("Lookup " << ueRlcPath << " got no matches");
    }
  return 0;
}


uint64_t
cv2x_LteStatsCalculator::FindImsiFromLteNetDevice (std::string path)
{
  NS_LOG_FUNCTION (path);
  // Sample path input:
  // /NodeList/#NodeId/DeviceList/#DeviceId/

  // We retrieve the Imsi associated to the cv2x_LteUeNetDevice
  Config::MatchContainer match = Config::LookupMatches (path);

  if (match.GetN () != 0)
    {
      Ptr<Object> ueNetDevice = match.Get (0);
      NS_LOG_LOGIC ("FindImsiFromLteNetDevice: " << path << ", " << ueNetDevice->GetObject<cv2x_LteUeNetDevice> ()->GetImsi ());
      return ueNetDevice->GetObject<cv2x_LteUeNetDevice> ()->GetImsi ();
    }
  else
    {
      NS_FATAL_ERROR ("Lookup " << path << " got no matches");
    }
}

uint16_t
cv2x_LteStatsCalculator::FindCellIdFromEnbRlcPath (std::string path)
{
  NS_LOG_FUNCTION (path);
  // Sample path input:
  // /NodeList/#NodeId/DeviceList/#DeviceId/cv2x_LteEnbRrc/UeMap/#C-RNTI/DataRadioBearerMap/#LCID/cv2x_LteRlc/RxPDU

  // We retrieve the CellId associated to the Enb
  std::string enbNetDevicePath = path.substr (0, path.find ("/cv2x_LteEnbRrc"));
  Config::MatchContainer match = Config::LookupMatches (enbNetDevicePath);
  if (match.GetN () != 0)
    {
      Ptr<Object> enbNetDevice = match.Get (0);
      NS_LOG_LOGIC ("FindCellIdFromEnbRlcPath: " << path << ", " << enbNetDevice->GetObject<cv2x_LteEnbNetDevice> ()->GetCellId ());
      return enbNetDevice->GetObject<cv2x_LteEnbNetDevice> ()->GetCellId ();
    }
  else
    {
      NS_FATAL_ERROR ("Lookup " << enbNetDevicePath << " got no matches");
    }
}

uint64_t
cv2x_LteStatsCalculator::FindImsiFromEnbMac (std::string path, uint16_t rnti)
{
  NS_LOG_FUNCTION (path << rnti);

  // /NodeList/#NodeId/DeviceList/#DeviceId/cv2x_LteEnbMac/DlScheduling
  std::ostringstream oss;
  std::string p = path.substr (0, path.find ("/cv2x_LteEnbMac"));
  oss << rnti;
  p += "/cv2x_LteEnbRrc/UeMap/" + oss.str ();
  uint64_t imsi = FindImsiFromEnbRlcPath (p);
  NS_LOG_LOGIC ("FindImsiFromEnbMac: " << path << ", " << rnti << ", " << imsi);
  return imsi;
}

uint16_t
cv2x_LteStatsCalculator::FindCellIdFromEnbMac (std::string path, uint16_t rnti)
{
  NS_LOG_FUNCTION (path << rnti);
  // /NodeList/#NodeId/DeviceList/#DeviceId/cv2x_LteEnbMac/DlScheduling
  std::ostringstream oss;
  std::string p = path.substr (0, path.find ("/cv2x_LteEnbMac"));
  oss << rnti;
  p += "/cv2x_LteEnbRrc/UeMap/" + oss.str ();
  uint16_t cellId = FindCellIdFromEnbRlcPath (p);
  NS_LOG_LOGIC ("FindCellIdFromEnbMac: " << path << ", "<< rnti << ", " << cellId);
  return cellId;
}


uint64_t
cv2x_LteStatsCalculator::FindImsiForEnb (std::string path, uint16_t rnti)
{
  NS_LOG_FUNCTION (path << rnti);
  uint64_t imsi = 0;
  if (path.find ("/DlPhyTransmission"))
    {
      // /NodeList/0/DeviceList/0/cv2x_LteEnbPhy/DlPhyTransmission/cv2x_LteEnbRrc/UeMap/1
      std::ostringstream oss;
      std::string p = path.substr (0, path.find ("/cv2x_LteEnbPhy"));
      oss << rnti;
      p += "/cv2x_LteEnbRrc/UeMap/" + oss.str ();
      imsi = FindImsiFromEnbRlcPath (p);
      NS_LOG_LOGIC ("FindImsiForEnb[Tx]: " << path << ", " << rnti << ", " << imsi);
    }
  else if (path.find ("/UlPhyReception"))
    {
      std::string p = path.substr (0, path.find ("/cv2x_LteUePhy"));
      imsi = FindImsiFromLteNetDevice (p);
      NS_LOG_LOGIC ("FindImsiForEnb[Rx]: " << path << ", " << rnti << ", " << imsi);
    }
  return imsi;
}


uint64_t
cv2x_LteStatsCalculator::FindImsiForUe (std::string path, uint16_t rnti)
{
  NS_LOG_FUNCTION (path << rnti);
  uint64_t imsi = 0;
  if (path.find ("/UlPhyTransmission"))
    {
      std::string p = path.substr (0, path.find ("/cv2x_LteUePhy"));
      imsi = FindImsiFromLteNetDevice (p);
      NS_LOG_LOGIC ("FindImsiForUe[Tx]: " << path << ", " << rnti << ", " << imsi);
    }
  else if (path.find ("/DlPhyReception"))
    {
      // /NodeList/0/DeviceList/0/cv2x_LteEnbPhy/cv2x_LteSpectrumPhy
      std::ostringstream oss;
      std::string p = path.substr (0, path.find ("/cv2x_LteEnbPhy"));
      oss << rnti;
      p += "/cv2x_LteEnbRrc/UeMap/" + oss.str ();
      imsi = FindImsiFromEnbRlcPath (p);
      NS_LOG_LOGIC ("FindImsiForUe[Rx]: " << path << ", " << rnti << ", " << imsi);
    }
  return imsi;
}


} // namespace ns3

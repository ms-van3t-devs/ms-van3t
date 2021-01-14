/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */


#include <ns3/log.h>


#include "cv2x_radio-bearer-stats-connector.h"
#include "cv2x_radio-bearer-stats-calculator.h"
#include <ns3/cv2x_lte-enb-rrc.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-ue-rrc.h>
#include <ns3/cv2x_lte-ue-net-device.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_RadioBearerStatsConnector");

/**
  * Less than operator for CellIdRnti, because it is used as key in map
  */
bool
operator < (const cv2x_RadioBearerStatsConnector::CellIdRnti& a, const cv2x_RadioBearerStatsConnector::CellIdRnti& b)
{
  return ( (a.cellId < b.cellId) || ( (a.cellId == b.cellId) && (a.rnti < b.rnti) ) );
}

/**
 * This structure is used as interface between trace
 * sources and cv2x_RadioBearerStatsCalculator. It stores
 * and provides calculators with cellId and IMSI,
 * because most trace sources do not provide it.
 */
struct cv2x_BoundCallbackArgument : public SimpleRefCount<cv2x_BoundCallbackArgument>
{
public:
  Ptr<cv2x_RadioBearerStatsCalculator> stats;  //!< statistics calculator
  uint64_t imsi; //!< imsi
  uint16_t cellId; //!< cellId
};

/**
 * Callback function for DL TX statistics for both RLC and PDCP
 * \param arg
 * \param path
 * \param rnti
 * \param lcid
 * \param packetSize
 */
void
DlTxPduCallback (Ptr<cv2x_BoundCallbackArgument> arg, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize)
{
  NS_LOG_FUNCTION (path << rnti << (uint16_t)lcid << packetSize);
  arg->stats->DlTxPdu (arg->cellId, arg->imsi, rnti, lcid, packetSize);
}

/**
 * Callback function for DL RX statistics for both RLC and PDCP
 * \param arg
 * \param path
 * \param rnti
 * \param lcid
 * \param packetSize
 * \param delay
 */
void
DlRxPduCallback (Ptr<cv2x_BoundCallbackArgument> arg, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize, uint64_t delay)
{
  NS_LOG_FUNCTION (path << rnti << (uint16_t)lcid << packetSize << delay);
  arg->stats->DlRxPdu (arg->cellId, arg->imsi, rnti, lcid, packetSize, delay);
}

/**
 * Callback function for UL TX statistics for both RLC and PDCP
 * \param arg
 * \param path
 * \param rnti
 * \param lcid
 * \param packetSize
 */
void
UlTxPduCallback (Ptr<cv2x_BoundCallbackArgument> arg, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize)
{
  NS_LOG_FUNCTION (path << rnti << (uint16_t)lcid << packetSize);
 
  arg->stats->UlTxPdu (arg->cellId, arg->imsi, rnti, lcid, packetSize);
}

/**
 * Callback function for UL RX statistics for both RLC and PDCP
 * \param arg
 * \param path
 * \param rnti
 * \param lcid
 * \param packetSize
 * \param delay
 */
void
UlRxPduCallback (Ptr<cv2x_BoundCallbackArgument> arg, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize, uint64_t delay)
{
  NS_LOG_FUNCTION (path << rnti << (uint16_t)lcid << packetSize << delay);
 
  arg->stats->UlRxPdu (arg->cellId, arg->imsi, rnti, lcid, packetSize, delay);
}



cv2x_RadioBearerStatsConnector::cv2x_RadioBearerStatsConnector ()
  : m_connected (false)
{
}

void 
cv2x_RadioBearerStatsConnector::EnableRlcStats (Ptr<cv2x_RadioBearerStatsCalculator> rlcStats)
{
  m_rlcStats = rlcStats;
  EnsureConnected ();
}

void 
cv2x_RadioBearerStatsConnector::EnablePdcpStats (Ptr<cv2x_RadioBearerStatsCalculator> pdcpStats)
{
  m_pdcpStats = pdcpStats;
  EnsureConnected ();
}

void 
cv2x_RadioBearerStatsConnector::EnsureConnected ()
{
  NS_LOG_FUNCTION (this);
  if (!m_connected)
    {
      Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteEnbRrc/NewUeContext",
		       MakeBoundCallback (&cv2x_RadioBearerStatsConnector::NotifyNewUeContextEnb, this));
      Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUeRrc/RandomAccessSuccessful",
		       MakeBoundCallback (&cv2x_RadioBearerStatsConnector::NotifyRandomAccessSuccessfulUe, this));
      Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteEnbRrc/ConnectionReconfiguration",
		       MakeBoundCallback (&cv2x_RadioBearerStatsConnector::NotifyConnectionReconfigurationEnb, this));
      Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUeRrc/ConnectionReconfiguration",
		       MakeBoundCallback (&cv2x_RadioBearerStatsConnector::NotifyConnectionReconfigurationUe, this));
      Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteEnbRrc/HandoverStart",
		       MakeBoundCallback (&cv2x_RadioBearerStatsConnector::NotifyHandoverStartEnb, this));
      Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUeRrc/HandoverStart",
		       MakeBoundCallback (&cv2x_RadioBearerStatsConnector::NotifyHandoverStartUe, this));
      Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteEnbRrc/HandoverEndOk",
		       MakeBoundCallback (&cv2x_RadioBearerStatsConnector::NotifyHandoverEndOkEnb, this));
      Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteUeRrc/HandoverEndOk",
		       MakeBoundCallback (&cv2x_RadioBearerStatsConnector::NotifyHandoverEndOkUe, this));
      m_connected = true;
    }
}

void 
cv2x_RadioBearerStatsConnector::NotifyRandomAccessSuccessfulUe (cv2x_RadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectSrb0Traces (context, imsi, cellId, rnti);
}

void 
cv2x_RadioBearerStatsConnector::NotifyConnectionSetupUe (cv2x_RadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectSrb1TracesUe (context, imsi, cellId, rnti);
}

void 
cv2x_RadioBearerStatsConnector::NotifyConnectionReconfigurationUe (cv2x_RadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectTracesUeIfFirstTime (context, imsi, cellId, rnti);
}

void 
cv2x_RadioBearerStatsConnector::NotifyHandoverStartUe (cv2x_RadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti, uint16_t targetCellId)
{
  c->DisconnectTracesUe (context, imsi, cellId, rnti);
}

void 
cv2x_RadioBearerStatsConnector::NotifyHandoverEndOkUe (cv2x_RadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectTracesUe (context, imsi, cellId, rnti);
}

void 
cv2x_RadioBearerStatsConnector::NotifyNewUeContextEnb (cv2x_RadioBearerStatsConnector* c, std::string context, uint16_t cellId, uint16_t rnti)
{
  c->StoreUeManagerPath (context, cellId, rnti);
}

void 
cv2x_RadioBearerStatsConnector::NotifyConnectionReconfigurationEnb (cv2x_RadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectTracesEnbIfFirstTime (context, imsi, cellId, rnti);
}

void 
cv2x_RadioBearerStatsConnector::NotifyHandoverStartEnb (cv2x_RadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti, uint16_t targetCellId)
{
  c->DisconnectTracesEnb (context, imsi, cellId, rnti);
}

void 
cv2x_RadioBearerStatsConnector::NotifyHandoverEndOkEnb (cv2x_RadioBearerStatsConnector* c, std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  c->ConnectTracesEnb (context, imsi, cellId, rnti);
}

void 
cv2x_RadioBearerStatsConnector::StoreUeManagerPath (std::string context, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << context << cellId << rnti);
  std::ostringstream ueManagerPath;
  ueManagerPath <<  context.substr (0, context.rfind ("/")) << "/UeMap/" << (uint32_t) rnti;
  CellIdRnti key;
  key.cellId = cellId;
  key.rnti = rnti;
  m_ueManagerPathByCellIdRnti[key] = ueManagerPath.str ();
}

void 
cv2x_RadioBearerStatsConnector::ConnectSrb0Traces (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << imsi << cellId << rnti);
  std::string ueRrcPath =  context.substr (0, context.rfind ("/"));
  CellIdRnti key;
  key.cellId = cellId;
  key.rnti = rnti;
  std::map<CellIdRnti, std::string>::iterator it = m_ueManagerPathByCellIdRnti.find (key);
  NS_ASSERT (it != m_ueManagerPathByCellIdRnti.end ());
  std::string ueManagerPath = it->second;  
  NS_LOG_LOGIC (this << " ueManagerPath: " << ueManagerPath);
  m_ueManagerPathByCellIdRnti.erase (it);

  if (m_rlcStats)
    {
      Ptr<cv2x_BoundCallbackArgument> arg = Create<cv2x_BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_rlcStats;

      // diconnect eventually previously connected SRB0 both at UE and eNB
      Config::Disconnect (ueRrcPath + "/Srb0/cv2x_LteRlc/TxPDU",
                          MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Disconnect (ueRrcPath + "/Srb0/cv2x_LteRlc/RxPDU",
                          MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Disconnect (ueManagerPath + "/Srb0/cv2x_LteRlc/TxPDU",
                          MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Disconnect (ueManagerPath + "/Srb0/cv2x_LteRlc/RxPDU",
                          MakeBoundCallback (&UlRxPduCallback, arg));

      // connect SRB0 both at UE and eNB
      Config::Connect (ueRrcPath + "/Srb0/cv2x_LteRlc/TxPDU",
                       MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Connect (ueRrcPath + "/Srb0/cv2x_LteRlc/RxPDU",
                       MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Connect (ueManagerPath + "/Srb0/cv2x_LteRlc/TxPDU",
                       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (ueManagerPath + "/Srb0/cv2x_LteRlc/RxPDU",
                       MakeBoundCallback (&UlRxPduCallback, arg));

      // connect SRB1 at eNB only (at UE SRB1 will be setup later)
      Config::Connect (ueManagerPath + "/Srb1/cv2x_LteRlc/TxPDU",
                       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (ueManagerPath + "/Srb1/cv2x_LteRlc/RxPDU",
                       MakeBoundCallback (&UlRxPduCallback, arg));
    }
  if (m_pdcpStats)
    {
      Ptr<cv2x_BoundCallbackArgument> arg = Create<cv2x_BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_pdcpStats;

      // connect SRB1 at eNB only (at UE SRB1 will be setup later)
      Config::Connect (ueManagerPath + "/Srb1/cv2x_LtePdcp/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
      Config::Connect (ueManagerPath + "/Srb1/cv2x_LtePdcp/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
    }
}

void 
cv2x_RadioBearerStatsConnector::ConnectSrb1TracesUe (std::string ueRrcPath, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << imsi << cellId << rnti);
   if (m_rlcStats)
    {
      Ptr<cv2x_BoundCallbackArgument> arg = Create<cv2x_BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_rlcStats;
      Config::Connect (ueRrcPath + "/Srb1/cv2x_LteRlc/TxPDU",
                       MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Connect (ueRrcPath + "/Srb1/cv2x_LteRlc/RxPDU",
                       MakeBoundCallback (&DlRxPduCallback, arg));
    }
  if (m_pdcpStats)
    {
      Ptr<cv2x_BoundCallbackArgument> arg = Create<cv2x_BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_pdcpStats;
      Config::Connect (ueRrcPath + "/Srb1/cv2x_LtePdcp/RxPDU",
		       MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Connect (ueRrcPath + "/Srb1/cv2x_LtePdcp/TxPDU",
		       MakeBoundCallback (&UlTxPduCallback, arg));
    }
}
  
void 
cv2x_RadioBearerStatsConnector::ConnectTracesUeIfFirstTime (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << context);
  if (m_imsiSeenUe.find (imsi) == m_imsiSeenUe.end ())
    {
      m_imsiSeenUe.insert (imsi);
      ConnectTracesUe (context, imsi, cellId, rnti);
    }
}
 
void 
cv2x_RadioBearerStatsConnector::ConnectTracesEnbIfFirstTime (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << context);
   if (m_imsiSeenEnb.find (imsi) == m_imsiSeenEnb.end ())
    {
      m_imsiSeenEnb.insert (imsi);
      ConnectTracesEnb (context, imsi, cellId, rnti);
    }
}

void 
cv2x_RadioBearerStatsConnector::ConnectTracesUe (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << context);
  NS_LOG_LOGIC (this << "expected context should match /NodeList/*/DeviceList/*/cv2x_LteUeRrc/");
  std::string basePath = context.substr (0, context.rfind ("/"));
  if (m_rlcStats)
    {
      Ptr<cv2x_BoundCallbackArgument> arg = Create<cv2x_BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_rlcStats;
      Config::Connect (basePath + "/DataRadioBearerMap/*/cv2x_LteRlc/TxPDU",
		       MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Connect (basePath + "/DataRadioBearerMap/*/cv2x_LteRlc/RxPDU",
		       MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Connect (basePath + "/Srb1/cv2x_LteRlc/TxPDU",
		       MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Connect (basePath + "/Srb1/cv2x_LteRlc/RxPDU",
		       MakeBoundCallback (&DlRxPduCallback, arg));

    }
  if (m_pdcpStats)
    {
      Ptr<cv2x_BoundCallbackArgument> arg = Create<cv2x_BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_pdcpStats;
      Config::Connect (basePath + "/DataRadioBearerMap/*/cv2x_LtePdcp/RxPDU",
		       MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Connect (basePath + "/DataRadioBearerMap/*/cv2x_LtePdcp/TxPDU",
		       MakeBoundCallback (&UlTxPduCallback, arg));
      Config::Connect (basePath + "/Srb1/cv2x_LtePdcp/RxPDU",
		       MakeBoundCallback (&DlRxPduCallback, arg));
      Config::Connect (basePath + "/Srb1/cv2x_LtePdcp/TxPDU",
		       MakeBoundCallback (&UlTxPduCallback, arg));
    }
}

void 
cv2x_RadioBearerStatsConnector::ConnectTracesEnb (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this << context);
  NS_LOG_LOGIC (this << "expected context  should match /NodeList/*/DeviceList/*/cv2x_LteEnbRrc/");
  std::ostringstream basePath;
  basePath <<  context.substr (0, context.rfind ("/")) << "/UeMap/" << (uint32_t) rnti;
  if (m_rlcStats)
    {
      Ptr<cv2x_BoundCallbackArgument> arg = Create<cv2x_BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_rlcStats;
      Config::Connect (basePath.str () + "/DataRadioBearerMap/*/cv2x_LteRlc/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
      Config::Connect (basePath.str () + "/DataRadioBearerMap/*/cv2x_LteRlc/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb0/cv2x_LteRlc/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb0/cv2x_LteRlc/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb1/cv2x_LteRlc/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb1/cv2x_LteRlc/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
    }
  if (m_pdcpStats)
    {
      Ptr<cv2x_BoundCallbackArgument> arg = Create<cv2x_BoundCallbackArgument> ();
      arg->imsi = imsi;
      arg->cellId = cellId; 
      arg->stats = m_pdcpStats;
      Config::Connect (basePath.str () + "/DataRadioBearerMap/*/cv2x_LtePdcp/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (basePath.str () + "/DataRadioBearerMap/*/cv2x_LtePdcp/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb1/cv2x_LtePdcp/TxPDU",
		       MakeBoundCallback (&DlTxPduCallback, arg));
      Config::Connect (basePath.str () + "/Srb1/cv2x_LtePdcp/RxPDU",
		       MakeBoundCallback (&UlRxPduCallback, arg));
    }
}

void 
cv2x_RadioBearerStatsConnector::DisconnectTracesUe (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
}


void 
cv2x_RadioBearerStatsConnector::DisconnectTracesEnb (std::string context, uint64_t imsi, uint16_t cellId, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
}



} // namespace ns3

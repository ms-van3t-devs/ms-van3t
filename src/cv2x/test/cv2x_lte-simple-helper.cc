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
 * Author: Manuel Requena <manuel.requena@cttc.es> (Based on lte-helper.cc)
 */


#include "ns3/log.h"
#include "ns3/callback.h"
#include "ns3/config.h"
#include "ns3/simple-channel.h"
#include "ns3/error-model.h"

#include "cv2x_lte-simple-helper.h"
#include "cv2x_lte-simple-net-device.h"
#include "cv2x_lte-test-entities.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteSimpleHelper");

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteSimpleHelper);

cv2x_LteSimpleHelper::cv2x_LteSimpleHelper (void)
{
  NS_LOG_FUNCTION (this);
  m_enbDeviceFactory.SetTypeId (cv2x_LteSimpleNetDevice::GetTypeId ());
  m_ueDeviceFactory.SetTypeId (cv2x_LteSimpleNetDevice::GetTypeId ());
}

void
cv2x_LteSimpleHelper::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  m_phyChannel = CreateObject<SimpleChannel> ();

  Object::DoInitialize ();
}

cv2x_LteSimpleHelper::~cv2x_LteSimpleHelper (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId cv2x_LteSimpleHelper::GetTypeId (void)
{
  static TypeId
  tid =
    TypeId ("ns3::cv2x_LteSimpleHelper")
    .SetParent<Object> ()
    .AddConstructor<cv2x_LteSimpleHelper> ()
    .AddAttribute ("RlcEntity",
                   "Specify which type of RLC will be used. ",
                   EnumValue (RLC_UM),
                   MakeEnumAccessor (&cv2x_LteSimpleHelper::m_lteRlcEntityType),
                   MakeEnumChecker (RLC_UM, "RlcUm",
                                    RLC_AM, "RlcAm"))
  ;
  return tid;
}

void
cv2x_LteSimpleHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_phyChannel = 0;

  m_enbMac->Dispose ();
  m_enbMac = 0;
  m_ueMac->Dispose ();
  m_ueMac = 0;

  Object::DoDispose ();
}


NetDeviceContainer
cv2x_LteSimpleHelper::InstallEnbDevice (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  Initialize ();  // will run DoInitialize () if necessary
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleEnbDevice (node);
      devices.Add (device);
    }
  return devices;
}

NetDeviceContainer
cv2x_LteSimpleHelper::InstallUeDevice (NodeContainer c)
{
  NS_LOG_FUNCTION (this);
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<NetDevice> device = InstallSingleUeDevice (node);
      devices.Add (device);
    }
  return devices;
}


Ptr<NetDevice>
cv2x_LteSimpleHelper::InstallSingleEnbDevice (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this);

  m_enbRrc = CreateObject<cv2x_LteTestRrc> ();
  m_enbPdcp = CreateObject<cv2x_LtePdcp> ();

  if (m_lteRlcEntityType == RLC_UM)
    {
      m_enbRlc = CreateObject<cv2x_LteRlcUm> ();
    }
  else // m_lteRlcEntityType == RLC_AM
    {
      m_enbRlc = CreateObject<cv2x_LteRlcAm> ();
    }

  m_enbRlc->SetRnti (11);
  m_enbRlc->SetLcId (12);

  Ptr<cv2x_LteSimpleNetDevice> enbDev = m_enbDeviceFactory.Create<cv2x_LteSimpleNetDevice> ();
  enbDev->SetAddress (Mac48Address::Allocate ());
  enbDev->SetChannel (m_phyChannel);

  n->AddDevice (enbDev);

  m_enbMac = CreateObject<cv2x_LteTestMac> ();
  m_enbMac->SetDevice (enbDev);

  m_enbRrc->SetDevice (enbDev);

  enbDev->SetReceiveCallback (MakeCallback (&cv2x_LteTestMac::Receive, m_enbMac));

  // Connect SAPs: RRC <-> PDCP <-> RLC <-> MAC

  m_enbRrc->SetLtePdcpSapProvider (m_enbPdcp->GetLtePdcpSapProvider ());
  m_enbPdcp->SetLtePdcpSapUser (m_enbRrc->GetLtePdcpSapUser ());

  m_enbPdcp->SetLteRlcSapProvider (m_enbRlc->GetLteRlcSapProvider ());
  m_enbRlc->SetLteRlcSapUser (m_enbPdcp->GetLteRlcSapUser ());

  m_enbRlc->SetLteMacSapProvider (m_enbMac->GetLteMacSapProvider ());
  m_enbMac->SetLteMacSapUser (m_enbRlc->GetLteMacSapUser ());

  return enbDev;
}

Ptr<NetDevice>
cv2x_LteSimpleHelper::InstallSingleUeDevice (Ptr<Node> n)
{
  NS_LOG_FUNCTION (this);

  m_ueRrc = CreateObject<cv2x_LteTestRrc> ();
  m_uePdcp = CreateObject<cv2x_LtePdcp> ();

  if (m_lteRlcEntityType == RLC_UM)
    {
      m_ueRlc = CreateObject<cv2x_LteRlcUm> ();
    }
  else // m_lteRlcEntityType == RLC_AM
    {
      m_ueRlc = CreateObject<cv2x_LteRlcAm> ();
    }

  m_ueRlc->SetRnti (21);
  m_ueRlc->SetLcId (22);

  Ptr<cv2x_LteSimpleNetDevice> ueDev = m_ueDeviceFactory.Create<cv2x_LteSimpleNetDevice> ();
  ueDev->SetAddress (Mac48Address::Allocate ());
  ueDev->SetChannel (m_phyChannel);

  n->AddDevice (ueDev);

  m_ueMac = CreateObject<cv2x_LteTestMac> ();
  m_ueMac->SetDevice (ueDev);

  ueDev->SetReceiveCallback (MakeCallback (&cv2x_LteTestMac::Receive, m_ueMac));

  // Connect SAPs: RRC <-> PDCP <-> RLC <-> MAC

  m_ueRrc->SetLtePdcpSapProvider (m_uePdcp->GetLtePdcpSapProvider ());
  m_uePdcp->SetLtePdcpSapUser (m_ueRrc->GetLtePdcpSapUser ());

  m_uePdcp->SetLteRlcSapProvider (m_ueRlc->GetLteRlcSapProvider ());
  m_ueRlc->SetLteRlcSapUser (m_uePdcp->GetLteRlcSapUser ());

  m_ueRlc->SetLteMacSapProvider (m_ueMac->GetLteMacSapProvider ());
  m_ueMac->SetLteMacSapUser (m_ueRlc->GetLteMacSapUser ());

  return ueDev;
}


void
cv2x_LteSimpleHelper::EnableLogComponents (void)
{
  LogLevel level = (LogLevel) (LOG_LEVEL_ALL | LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_PREFIX_FUNC);

  LogComponentEnable ("Config", level);
  LogComponentEnable ("cv2x_LteSimpleHelper", level);
  LogComponentEnable ("LteTestEntities", level);
  LogComponentEnable ("cv2x_LtePdcp", level);
  LogComponentEnable ("cv2x_LteRlc", level);
  LogComponentEnable ("cv2x_LteRlcUm", level);
  LogComponentEnable ("cv2x_LteRlcAm", level);
  LogComponentEnable ("cv2x_LteSimpleNetDevice", level);
  LogComponentEnable ("SimpleNetDevice", level);
  LogComponentEnable ("SimpleChannel", level);
}

void
cv2x_LteSimpleHelper::EnableTraces (void)
{
//   EnableMacTraces ();
  EnableRlcTraces ();
  EnablePdcpTraces ();
}

void
cv2x_LteSimpleHelper::EnableRlcTraces (void)
{
  EnableDlRlcTraces ();
  EnableUlRlcTraces ();
}


  /**
   * DL transmit PDU callback
   *
   * \param rlcStats the stats calculator
   * \param path
   * \param rnti the RNTI
   * \param lcid the LCID
   * \param packetSize the packet size
   */
void
cv2x_LteSimpleHelperDlTxPduCallback (Ptr<cv2x_RadioBearerStatsCalculator> rlcStats, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize)
{
  NS_LOG_FUNCTION (rlcStats << path << rnti << (uint16_t)lcid << packetSize);
  uint64_t imsi = 111;
  uint16_t cellId = 222;
  rlcStats->DlTxPdu (cellId, imsi, rnti, lcid, packetSize);
}

  /**
   * DL receive PDU callback
   *
   * \param rlcStats the stats calculator
   * \param path
   * \param rnti the RNTI
   * \param lcid the LCID
   * \param packetSize the packet size
   * \param delay the delay
   */
void
cv2x_LteSimpleHelperDlRxPduCallback (Ptr<cv2x_RadioBearerStatsCalculator> rlcStats, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize, uint64_t delay)
{
  NS_LOG_FUNCTION (rlcStats << path << rnti << (uint16_t)lcid << packetSize << delay);
  uint64_t imsi = 333;
  uint16_t cellId = 555;
  rlcStats->DlRxPdu (cellId, imsi, rnti, lcid, packetSize, delay);
}

void
cv2x_LteSimpleHelper::EnableDlRlcTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();

                //   Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteRlc/TxPDU",
                //                    MakeBoundCallback (&cv2x_LteSimpleHelperDlTxPduCallback, m_rlcStats));
                //   Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteRlc/RxPDU",
                //                    MakeBoundCallback (&cv2x_LteSimpleHelperDlRxPduCallback, m_rlcStats));
}

  /**
   * UL transmit PDU callback
   *
   * \param rlcStats the stats calculator
   * \param path
   * \param rnti the RNTI
   * \param lcid the LCID
   * \param packetSize the packet size
   */
void
cv2x_LteSimpleHelperUlTxPduCallback (Ptr<cv2x_RadioBearerStatsCalculator> rlcStats, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize)
{
  NS_LOG_FUNCTION (rlcStats << path << rnti << (uint16_t)lcid << packetSize);
  uint64_t imsi = 1111;
  uint16_t cellId = 555;
  rlcStats->UlTxPdu (cellId, imsi, rnti, lcid, packetSize);
}

  /**
   * UL receive PDU callback
   *
   * \param rlcStats the stats calculator
   * \param path
   * \param rnti the RNTI
   * \param lcid the LCID
   * \param packetSize the packet size
   * \param delay the delay
   */
void
cv2x_LteSimpleHelperUlRxPduCallback (Ptr<cv2x_RadioBearerStatsCalculator> rlcStats, std::string path,
                 uint16_t rnti, uint8_t lcid, uint32_t packetSize, uint64_t delay)
{
  NS_LOG_FUNCTION (rlcStats << path << rnti << (uint16_t)lcid << packetSize << delay);
  uint64_t imsi = 444;
  uint16_t cellId = 555;
  rlcStats->UlRxPdu (cellId, imsi, rnti, lcid, packetSize, delay);
}


void
cv2x_LteSimpleHelper::EnableUlRlcTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();

                  //   Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteRlc/TxPDU",
                  //                    MakeBoundCallback (&cv2x_LteSimpleHelperUlTxPduCallback, m_rlcStats));
                  //   Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LteRlc/RxPDU",
                  //                    MakeBoundCallback (&cv2x_LteSimpleHelperUlRxPduCallback, m_rlcStats));
}


void
cv2x_LteSimpleHelper::EnablePdcpTraces (void)
{
  EnableDlPdcpTraces ();
  EnableUlPdcpTraces ();
}

void
cv2x_LteSimpleHelper::EnableDlPdcpTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();

                  //   Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LtePdcp/TxPDU",
                  //                    MakeBoundCallback (&cv2x_LteSimpleHelperDlTxPduCallback, m_pdcpStats));
                  //   Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LtePdcp/RxPDU",
                  //                    MakeBoundCallback (&cv2x_LteSimpleHelperDlRxPduCallback, m_pdcpStats));
}

void
cv2x_LteSimpleHelper::EnableUlPdcpTraces (void)
{
  NS_LOG_FUNCTION_NOARGS ();

                  //   Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LtePdcp/TxPDU",
                  //                    MakeBoundCallback (&cv2x_LteSimpleHelperUlTxPduCallback, m_pdcpStats));
                  //   Config::Connect ("/NodeList/*/DeviceList/*/cv2x_LtePdcp/RxPDU",
                  //                    MakeBoundCallback (&cv2x_LteSimpleHelperUlRxPduCallback, m_pdcpStats));
}


} // namespace ns3

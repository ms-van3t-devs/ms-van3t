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
 * Authors: Nicola Baldo <nbaldo@cttc.es>
 *          Lluis Parcerisa <lparcerisa@cttc.cat>
 * Modified by: NIST
 */

#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/nstime.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/simulator.h>

#include "cv2x_lte-rrc-protocol-real.h"
#include "cv2x_lte-ue-rrc.h"
#include "cv2x_lte-enb-rrc.h"
#include "cv2x_lte-enb-net-device.h"
#include "cv2x_lte-ue-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteRrcProtocolReal");

/// RRC real message delay
const Time RRC_REAL_MSG_DELAY = MilliSeconds (0); 

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteUeRrcProtocolReal);

cv2x_LteUeRrcProtocolReal::cv2x_LteUeRrcProtocolReal ()
  :  m_ueRrcSapProvider (0),
    m_enbRrcSapProvider (0)
{
  m_ueRrcSapUser = new cv2x_MemberLteUeRrcSapUser<cv2x_LteUeRrcProtocolReal> (this);
  m_completeSetupParameters.srb0SapUser = new cv2x_LteRlcSpecificLteRlcSapUser<cv2x_LteUeRrcProtocolReal> (this);
  m_completeSetupParameters.srb1SapUser = new cv2x_LtePdcpSpecificLtePdcpSapUser<cv2x_LteUeRrcProtocolReal> (this);    
}

cv2x_LteUeRrcProtocolReal::~cv2x_LteUeRrcProtocolReal ()
{
}

void
cv2x_LteUeRrcProtocolReal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ueRrcSapUser;
  delete m_completeSetupParameters.srb0SapUser;
  delete m_completeSetupParameters.srb1SapUser;
  m_rrc = 0;
}

TypeId
cv2x_LteUeRrcProtocolReal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteUeRrcProtocolReal")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteUeRrcProtocolReal> ()
  ;
  return tid;
}

void 
cv2x_LteUeRrcProtocolReal::SetLteUeRrcSapProvider (cv2x_LteUeRrcSapProvider* p)
{
  m_ueRrcSapProvider = p;
}

cv2x_LteUeRrcSapUser* 
cv2x_LteUeRrcProtocolReal::GetLteUeRrcSapUser ()
{
  return m_ueRrcSapUser;
}

void 
cv2x_LteUeRrcProtocolReal::SetUeRrc (Ptr<cv2x_LteUeRrc> rrc)
{
  m_rrc = rrc;
}

void 
cv2x_LteUeRrcProtocolReal::DoSetup (cv2x_LteUeRrcSapUser::SetupParameters params)
{
  NS_LOG_FUNCTION (this);

  m_setupParameters.srb0SapProvider = params.srb0SapProvider;
  m_setupParameters.srb1SapProvider = params.srb1SapProvider; 
  m_ueRrcSapProvider->CompleteSetup (m_completeSetupParameters);
}

void 
cv2x_LteUeRrcProtocolReal::DoSendRrcConnectionRequest (cv2x_LteRrcSap::RrcConnectionRequest msg)
{
  // initialize the RNTI and get the EnbLteRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionRequestHeader rrcConnectionRequestHeader;
  rrcConnectionRequestHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionRequestHeader);

  cv2x_LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = m_rnti;
  transmitPdcpPduParameters.lcid = 0;
  transmitPdcpPduParameters.srcL2Id = 0; //Added to avoid warning
  transmitPdcpPduParameters.dstL2Id = 0; //Added to avoid warning
  
  m_setupParameters.srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
cv2x_LteUeRrcProtocolReal::DoSendRrcConnectionSetupCompleted (cv2x_LteRrcSap::RrcConnectionSetupCompleted msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionSetupCompleteHeader rrcConnectionSetupCompleteHeader;
  rrcConnectionSetupCompleteHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionSetupCompleteHeader);

  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;
  transmitPdcpSduParameters.srcL2Id = 0; //Added to avoid warning
  transmitPdcpSduParameters.dstL2Id = 0; //Added to avoid warning


  if (m_setupParameters.srb1SapProvider)
    {
      m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
    }
}

void 
cv2x_LteUeRrcProtocolReal::DoSendRrcConnectionReconfigurationCompleted (cv2x_LteRrcSap::RrcConnectionReconfigurationCompleted msg)
{
  // re-initialize the RNTI and get the EnbLteRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionReconfigurationCompleteHeader rrcConnectionReconfigurationCompleteHeader;
  rrcConnectionReconfigurationCompleteHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReconfigurationCompleteHeader);

  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;
  transmitPdcpSduParameters.srcL2Id = 0; //Added to avoid warning
  transmitPdcpSduParameters.dstL2Id = 0; //Added to avoid warning

  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void 
cv2x_LteUeRrcProtocolReal::DoSendMeasurementReport (cv2x_LteRrcSap::MeasurementReport msg)
{
  // re-initialize the RNTI and get the EnbLteRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Ptr<Packet> packet = Create<Packet> ();

  cv2x_MeasurementReportHeader measurementReportHeader;
  measurementReportHeader.SetMessage (msg);

  packet->AddHeader (measurementReportHeader);

  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;
  transmitPdcpSduParameters.srcL2Id = 0; //Added to avoid warning
  transmitPdcpSduParameters.dstL2Id = 0; //Added to avoid warning

  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void 
cv2x_LteUeRrcProtocolReal::DoSendRrcConnectionReestablishmentRequest (cv2x_LteRrcSap::RrcConnectionReestablishmentRequest msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionReestablishmentRequestHeader rrcConnectionReestablishmentRequestHeader;
  rrcConnectionReestablishmentRequestHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentRequestHeader);

  cv2x_LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = m_rnti;
  transmitPdcpPduParameters.lcid = 0;
  transmitPdcpPduParameters.srcL2Id = 0; //Added to avoid warning
  transmitPdcpPduParameters.dstL2Id = 0; //Added to avoid warning

  m_setupParameters.srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
cv2x_LteUeRrcProtocolReal::DoSendRrcConnectionReestablishmentComplete (cv2x_LteRrcSap::RrcConnectionReestablishmentComplete msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionReestablishmentCompleteHeader rrcConnectionReestablishmentCompleteHeader;
  rrcConnectionReestablishmentCompleteHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentCompleteHeader);

  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;
  transmitPdcpSduParameters.srcL2Id = 0; //Added to avoid warning
  transmitPdcpSduParameters.dstL2Id = 0; //Added to avoid warning

  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void
cv2x_LteUeRrcProtocolReal::DoSendSidelinkUeInformation (cv2x_LteRrcSap::SidelinkUeInformation msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  //todo: define header
  cv2x_SidelinkUeInformationHeader sidelinkUeInformationHeader;
  sidelinkUeInformationHeader.SetMessage (msg);

  packet->AddHeader (sidelinkUeInformationHeader);
  
  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = m_rnti;
  transmitPdcpSduParameters.lcid = 1;
  transmitPdcpSduParameters.srcL2Id = 0; //Added to avoid warning
  transmitPdcpSduParameters.dstL2Id = 0; //Added to avoid warning

  m_setupParameters.srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void 
cv2x_LteUeRrcProtocolReal::SetEnbRrcSapProvider ()
{
  uint16_t cellId = m_rrc->GetCellId ();

  // walk list of all nodes to get the peer eNB
  Ptr<cv2x_LteEnbNetDevice> enbDev;
  NodeList::Iterator listEnd = NodeList::End ();
  bool found = false;
  for (NodeList::Iterator i = NodeList::Begin (); 
       (i != listEnd) && (!found); 
       ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; 
           (j < nDevs) && (!found);
           j++)
        {
          enbDev = node->GetDevice (j)->GetObject <cv2x_LteEnbNetDevice> ();
          if (enbDev == 0)
            {
              continue;
            }
          else
            {
              if (enbDev->HasCellId (cellId))
                {
                  found = true;
                  break;
                }
            }
        }
    }
  NS_ASSERT_MSG (found, " Unable to find eNB with CellId =" << cellId);
  m_enbRrcSapProvider = enbDev->GetRrc ()->GetLteEnbRrcSapProvider ();
  Ptr<cv2x_LteEnbRrcProtocolReal> enbRrcProtocolReal = enbDev->GetRrc ()->GetObject<cv2x_LteEnbRrcProtocolReal> ();
  enbRrcProtocolReal->SetUeRrcSapProvider (m_rnti, m_ueRrcSapProvider);
}

void
cv2x_LteUeRrcProtocolReal::DoReceivePdcpPdu (Ptr<Packet> p)
{
  // Get type of message received
  cv2x_RrcDlCcchMessage rrcDlCcchMessage;
  p->PeekHeader (rrcDlCcchMessage);

  // Declare possible headers to receive
  cv2x_RrcConnectionReestablishmentHeader rrcConnectionReestablishmentHeader;
  cv2x_RrcConnectionReestablishmentRejectHeader rrcConnectionReestablishmentRejectHeader;
  cv2x_RrcConnectionSetupHeader rrcConnectionSetupHeader;
  cv2x_RrcConnectionRejectHeader rrcConnectionRejectHeader;

  // Declare possible messages
  cv2x_LteRrcSap::RrcConnectionReestablishment rrcConnectionReestablishmentMsg;
  cv2x_LteRrcSap::RrcConnectionReestablishmentReject rrcConnectionReestablishmentRejectMsg;
  cv2x_LteRrcSap::RrcConnectionSetup rrcConnectionSetupMsg;
  cv2x_LteRrcSap::RrcConnectionReject rrcConnectionRejectMsg;

  // Deserialize packet and call member recv function with appropriate structure
  switch ( rrcDlCcchMessage.GetMessageType () )
    {
    case 0:
      // RrcConnectionReestablishment
      p->RemoveHeader (rrcConnectionReestablishmentHeader);
      rrcConnectionReestablishmentMsg = rrcConnectionReestablishmentHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionReestablishment (rrcConnectionReestablishmentMsg);
      break;
    case 1:
      // RrcConnectionReestablishmentReject
      p->RemoveHeader (rrcConnectionReestablishmentRejectHeader);
      rrcConnectionReestablishmentRejectMsg = rrcConnectionReestablishmentRejectHeader.GetMessage ();
      // m_ueRrcSapProvider->RecvRrcConnectionReestablishmentReject (rrcConnectionReestablishmentRejectMsg);
      break;
    case 2:
      // RrcConnectionReject
      p->RemoveHeader (rrcConnectionRejectHeader);
      rrcConnectionRejectMsg = rrcConnectionRejectHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionReject (rrcConnectionRejectMsg);
      break;
    case 3:
      // RrcConnectionSetup
      p->RemoveHeader (rrcConnectionSetupHeader);
      rrcConnectionSetupMsg = rrcConnectionSetupHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionSetup (rrcConnectionSetupMsg);
      break;
    }
}

void
cv2x_LteUeRrcProtocolReal::DoReceivePdcpSdu (cv2x_LtePdcpSapUser::ReceivePdcpSduParameters params)
{
  // Get type of message received
  cv2x_RrcDlDcchMessage rrcDlDcchMessage;
  params.pdcpSdu->PeekHeader (rrcDlDcchMessage);

  // Declare possible headers to receive
  cv2x_RrcConnectionReconfigurationHeader rrcConnectionReconfigurationHeader;
  cv2x_RrcConnectionReleaseHeader rrcConnectionReleaseHeader;

  // Declare possible messages to receive
  cv2x_LteRrcSap::RrcConnectionReconfiguration rrcConnectionReconfigurationMsg;
  cv2x_LteRrcSap::RrcConnectionRelease rrcConnectionReleaseMsg;

  // Deserialize packet and call member recv function with appropriate structure
  switch ( rrcDlDcchMessage.GetMessageType () )
    {
    case 4:
      params.pdcpSdu->RemoveHeader (rrcConnectionReconfigurationHeader);
      rrcConnectionReconfigurationMsg = rrcConnectionReconfigurationHeader.GetMessage ();
      m_ueRrcSapProvider->RecvRrcConnectionReconfiguration (rrcConnectionReconfigurationMsg);
      break;
    case 5:
      params.pdcpSdu->RemoveHeader (rrcConnectionReleaseHeader);
      rrcConnectionReleaseMsg = rrcConnectionReleaseHeader.GetMessage ();
      //m_ueRrcSapProvider->RecvRrcConnectionRelease (rrcConnectionReleaseMsg);
      break;
    }
}

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteEnbRrcProtocolReal);

cv2x_LteEnbRrcProtocolReal::cv2x_LteEnbRrcProtocolReal ()
  :  m_enbRrcSapProvider (0)
{
  NS_LOG_FUNCTION (this);
  m_enbRrcSapUser = new cv2x_MemberLteEnbRrcSapUser<cv2x_LteEnbRrcProtocolReal> (this);
}

cv2x_LteEnbRrcProtocolReal::~cv2x_LteEnbRrcProtocolReal ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteEnbRrcProtocolReal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_enbRrcSapUser;
  for (std::map<uint16_t, cv2x_LteEnbRrcSapProvider::CompleteSetupUeParameters>::iterator 
         it = m_completeSetupUeParametersMap.begin ();
       it != m_completeSetupUeParametersMap.end ();
       ++it)
    {     
      delete it->second.srb0SapUser;
      delete it->second.srb1SapUser;
    }
  m_completeSetupUeParametersMap.clear ();
}

TypeId
cv2x_LteEnbRrcProtocolReal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteEnbRrcProtocolReal")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteEnbRrcProtocolReal> ()
  ;
  return tid;
}

void 
cv2x_LteEnbRrcProtocolReal::SetLteEnbRrcSapProvider (cv2x_LteEnbRrcSapProvider* p)
{
  m_enbRrcSapProvider = p;
}

cv2x_LteEnbRrcSapUser* 
cv2x_LteEnbRrcProtocolReal::GetLteEnbRrcSapUser ()
{
  return m_enbRrcSapUser;
}

void 
cv2x_LteEnbRrcProtocolReal::SetCellId (uint16_t cellId)
{
  m_cellId = cellId;
}

cv2x_LteUeRrcSapProvider* 
cv2x_LteEnbRrcProtocolReal::GetUeRrcSapProvider (uint16_t rnti)
{
  std::map<uint16_t, cv2x_LteUeRrcSapProvider*>::const_iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  return it->second;
}

void 
cv2x_LteEnbRrcProtocolReal::SetUeRrcSapProvider (uint16_t rnti, cv2x_LteUeRrcSapProvider* p)
{
  std::map<uint16_t, cv2x_LteUeRrcSapProvider*>::iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  it->second = p;
}

void 
cv2x_LteEnbRrcProtocolReal::DoSetupUe (uint16_t rnti, cv2x_LteEnbRrcSapUser::SetupUeParameters params)
{
  NS_LOG_FUNCTION (this << rnti);

  // // walk list of all nodes to get the peer UE RRC SAP Provider
  // Ptr<cv2x_LteUeRrc> ueRrc;
  // NodeList::Iterator listEnd = NodeList::End ();
  // bool found = false;
  // for (NodeList::Iterator i = NodeList::Begin (); (i != listEnd) && (found == false); i++)
  //   {
  //     Ptr<Node> node = *i;
  //     int nDevs = node->GetNDevices ();
  //     for (int j = 0; j < nDevs; j++)
  //       {
  //         Ptr<cv2x_LteUeNetDevice> ueDev = node->GetDevice (j)->GetObject <cv2x_LteUeNetDevice> ();
  //         if (!ueDev)
  //           {
  //             continue;
  //           }
  //         else
  //           {
  //             ueRrc = ueDev->GetRrc ();
  //             if ((ueRrc->GetRnti () == rnti) && (ueRrc->GetCellId () == m_cellId))
  //               {
  //              found = true;
  //              break;
  //               }
  //           }
  //       }
  //   }
  // NS_ASSERT_MSG (found , " Unable to find UE with RNTI=" << rnti << " cellId=" << m_cellId);
  // m_enbRrcSapProviderMap[rnti] = ueRrc->GetLteUeRrcSapProvider ();

  // just create empty entry, the UeRrcSapProvider will be set by the
  // ue upon connection request or connection reconfiguration
  // completed 
  m_enbRrcSapProviderMap[rnti] = 0;

  // Store SetupUeParameters
  m_setupUeParametersMap[rnti] = params;

  cv2x_LteEnbRrcSapProvider::CompleteSetupUeParameters completeSetupUeParameters;
  std::map<uint16_t, cv2x_LteEnbRrcSapProvider::CompleteSetupUeParameters>::iterator 
    csupIt = m_completeSetupUeParametersMap.find (rnti);
  if (csupIt == m_completeSetupUeParametersMap.end ())
    {
      // Create cv2x_LteRlcSapUser, cv2x_LtePdcpSapUser
      cv2x_LteRlcSapUser* srb0SapUser = new cv2x_RealProtocolRlcSapUser (this,rnti);
      cv2x_LtePdcpSapUser* srb1SapUser = new cv2x_LtePdcpSpecificLtePdcpSapUser<cv2x_LteEnbRrcProtocolReal> (this);
      completeSetupUeParameters.srb0SapUser = srb0SapUser;
      completeSetupUeParameters.srb1SapUser = srb1SapUser;
      // Store cv2x_LteRlcSapUser, cv2x_LtePdcpSapUser
      m_completeSetupUeParametersMap[rnti] = completeSetupUeParameters;      
    }
  else
    {
      completeSetupUeParameters = csupIt->second;
    }
  m_enbRrcSapProvider->CompleteSetupUe (rnti, completeSetupUeParameters);
}

void 
cv2x_LteEnbRrcProtocolReal::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::map<uint16_t, cv2x_LteEnbRrcSapProvider::CompleteSetupUeParameters>::iterator 
    it = m_completeSetupUeParametersMap.find (rnti);
  NS_ASSERT (it != m_completeSetupUeParametersMap.end ());
  delete it->second.srb0SapUser;
  delete it->second.srb1SapUser;
  m_completeSetupUeParametersMap.erase (it);
  m_enbRrcSapProviderMap.erase (rnti);
  m_setupUeParametersMap.erase (rnti);
}

void 
cv2x_LteEnbRrcProtocolReal::DoSendSystemInformation (uint16_t cellId, cv2x_LteRrcSap::SystemInformation msg)
{
  NS_LOG_FUNCTION (this << cellId);
  // walk list of all nodes to get UEs with this cellId
  Ptr<cv2x_LteUeRrc> ueRrc;
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; ++j)
        {
          Ptr<cv2x_LteUeNetDevice> ueDev = node->GetDevice (j)->GetObject <cv2x_LteUeNetDevice> ();
          if (ueDev != 0)
            {
              Ptr<cv2x_LteUeRrc> ueRrc = ueDev->GetRrc ();
              NS_LOG_LOGIC ("considering UE IMSI " << ueDev->GetImsi () << " that has cellId " << ueRrc->GetCellId ());
              if (ueRrc->GetCellId () == cellId)
                {
                  NS_LOG_LOGIC ("sending SI to IMSI " << ueDev->GetImsi ());
                  //ueRrc->GetLteUeRrcSapProvider ()->RecvSystemInformation (msg);
                  Simulator::Schedule (RRC_REAL_MSG_DELAY, 
                                       &cv2x_LteUeRrcSapProvider::RecvSystemInformation,
                                       ueRrc->GetLteUeRrcSapProvider (), 
                                       msg);
                }
            }
        }
    } 
}

void 
cv2x_LteEnbRrcProtocolReal::DoSendRrcConnectionSetup (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionSetup msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionSetupHeader rrcConnectionSetupHeader;
  rrcConnectionSetupHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionSetupHeader);

  cv2x_LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;
  transmitPdcpPduParameters.srcL2Id = 0; // Added to avoid warnings
  transmitPdcpPduParameters.dstL2Id = 0; // Added to avoid warnings

  m_setupUeParametersMap.at (rnti).srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
cv2x_LteEnbRrcProtocolReal::DoSendRrcConnectionReject (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReject msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionRejectHeader rrcConnectionRejectHeader;
  rrcConnectionRejectHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionRejectHeader);

  cv2x_LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;
  transmitPdcpPduParameters.srcL2Id = 0; // Added to avoid warnings
  transmitPdcpPduParameters.dstL2Id = 0; // Added to avoid warnings

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
cv2x_LteEnbRrcProtocolReal::DoSendRrcConnectionReconfiguration (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReconfiguration msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionReconfigurationHeader rrcConnectionReconfigurationHeader;
  rrcConnectionReconfigurationHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReconfigurationHeader);

  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = rnti;
  transmitPdcpSduParameters.lcid = 1;
  transmitPdcpSduParameters.srcL2Id = 0; //Added to avoid warning
  transmitPdcpSduParameters.dstL2Id = 0; //Added to avoid warning

  m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void 
cv2x_LteEnbRrcProtocolReal::DoSendRrcConnectionReestablishment (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReestablishment msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionReestablishmentHeader rrcConnectionReestablishmentHeader;
  rrcConnectionReestablishmentHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentHeader);

  cv2x_LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;
  transmitPdcpPduParameters.srcL2Id = 0; // Added to avoid warnings
  transmitPdcpPduParameters.dstL2Id = 0; // Added to avoid warnings

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
cv2x_LteEnbRrcProtocolReal::DoSendRrcConnectionReestablishmentReject (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReestablishmentReject msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionReestablishmentRejectHeader rrcConnectionReestablishmentRejectHeader;
  rrcConnectionReestablishmentRejectHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReestablishmentRejectHeader);

  cv2x_LteRlcSapProvider::TransmitPdcpPduParameters transmitPdcpPduParameters;
  transmitPdcpPduParameters.pdcpPdu = packet;
  transmitPdcpPduParameters.rnti = rnti;
  transmitPdcpPduParameters.lcid = 0;
  transmitPdcpPduParameters.srcL2Id = 0; // Added to avoid warnings
  transmitPdcpPduParameters.dstL2Id = 0; // Added to avoid warnings

  m_setupUeParametersMap[rnti].srb0SapProvider->TransmitPdcpPdu (transmitPdcpPduParameters);
}

void 
cv2x_LteEnbRrcProtocolReal::DoSendRrcConnectionRelease (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionRelease msg)
{
  Ptr<Packet> packet = Create<Packet> ();

  cv2x_RrcConnectionReleaseHeader rrcConnectionReleaseHeader;
  rrcConnectionReleaseHeader.SetMessage (msg);

  packet->AddHeader (rrcConnectionReleaseHeader);

  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters transmitPdcpSduParameters;
  transmitPdcpSduParameters.pdcpSdu = packet;
  transmitPdcpSduParameters.rnti = rnti;
  transmitPdcpSduParameters.lcid = 1;
  transmitPdcpSduParameters.srcL2Id = 0; //Added to avoid warning
  transmitPdcpSduParameters.dstL2Id = 0; //Added to avoid warning

  m_setupUeParametersMap[rnti].srb1SapProvider->TransmitPdcpSdu (transmitPdcpSduParameters);
}

void
cv2x_LteEnbRrcProtocolReal::DoReceivePdcpPdu (uint16_t rnti, Ptr<Packet> p)
{
  // Get type of message received
  cv2x_RrcUlCcchMessage rrcUlCcchMessage;
  p->PeekHeader (rrcUlCcchMessage);

  // Declare possible headers to receive
  cv2x_RrcConnectionReestablishmentRequestHeader rrcConnectionReestablishmentRequestHeader;
  cv2x_RrcConnectionRequestHeader rrcConnectionRequestHeader;

  // Deserialize packet and call member recv function with appropriate structure
  switch ( rrcUlCcchMessage.GetMessageType () )
    {
    case 0:
      p->RemoveHeader (rrcConnectionReestablishmentRequestHeader);
      cv2x_LteRrcSap::RrcConnectionReestablishmentRequest rrcConnectionReestablishmentRequestMsg;
      rrcConnectionReestablishmentRequestMsg = rrcConnectionReestablishmentRequestHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionReestablishmentRequest (rnti,rrcConnectionReestablishmentRequestMsg);
      break;
    case 1:
      p->RemoveHeader (rrcConnectionRequestHeader);
      cv2x_LteRrcSap::RrcConnectionRequest rrcConnectionRequestMsg;
      rrcConnectionRequestMsg = rrcConnectionRequestHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionRequest (rnti,rrcConnectionRequestMsg);
      break;
    }
}

void
cv2x_LteEnbRrcProtocolReal::DoReceivePdcpSdu (cv2x_LtePdcpSapUser::ReceivePdcpSduParameters params)
{
  // Get type of message received
  cv2x_RrcUlDcchMessage rrcUlDcchMessage;
  params.pdcpSdu->PeekHeader (rrcUlDcchMessage);

  // Declare possible headers to receive
  cv2x_MeasurementReportHeader measurementReportHeader;
  cv2x_RrcConnectionReconfigurationCompleteHeader rrcConnectionReconfigurationCompleteHeader;
  cv2x_RrcConnectionReestablishmentCompleteHeader rrcConnectionReestablishmentCompleteHeader;
  cv2x_RrcConnectionSetupCompleteHeader rrcConnectionSetupCompleteHeader;
  cv2x_SidelinkUeInformationHeader sidelinkUeInformationHeader; 

  // Declare possible messages to receive
  cv2x_LteRrcSap::MeasurementReport measurementReportMsg;
  cv2x_LteRrcSap::RrcConnectionReconfigurationCompleted rrcConnectionReconfigurationCompleteMsg;
  cv2x_LteRrcSap::RrcConnectionReestablishmentComplete rrcConnectionReestablishmentCompleteMsg;
  cv2x_LteRrcSap::RrcConnectionSetupCompleted rrcConnectionSetupCompletedMsg;
  cv2x_LteRrcSap::SidelinkUeInformation sidelinkUeInformationMsg;

  // Deserialize packet and call member recv function with appropriate structure
  switch ( rrcUlDcchMessage.GetMessageType () )
    {
    case 1:
      params.pdcpSdu->RemoveHeader (measurementReportHeader);
      measurementReportMsg = measurementReportHeader.GetMessage ();
      m_enbRrcSapProvider->RecvMeasurementReport (params.rnti,measurementReportMsg);
      break;
    case 2:
      params.pdcpSdu->RemoveHeader (rrcConnectionReconfigurationCompleteHeader);
      rrcConnectionReconfigurationCompleteMsg = rrcConnectionReconfigurationCompleteHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionReconfigurationCompleted (params.rnti,rrcConnectionReconfigurationCompleteMsg);
      break;
    case 3:
      params.pdcpSdu->RemoveHeader (rrcConnectionReestablishmentCompleteHeader);
      rrcConnectionReestablishmentCompleteMsg = rrcConnectionReestablishmentCompleteHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionReestablishmentComplete (params.rnti,rrcConnectionReestablishmentCompleteMsg);
      break;
    case 4:
      params.pdcpSdu->RemoveHeader (rrcConnectionSetupCompleteHeader);
      rrcConnectionSetupCompletedMsg = rrcConnectionSetupCompleteHeader.GetMessage ();
      m_enbRrcSapProvider->RecvRrcConnectionSetupCompleted (params.rnti, rrcConnectionSetupCompletedMsg);
      break;
    case 20:
      params.pdcpSdu->RemoveHeader (sidelinkUeInformationHeader);
      sidelinkUeInformationMsg = sidelinkUeInformationHeader.GetMessage ();
      m_enbRrcSapProvider->RecvSidelinkUeInformation (params.rnti,sidelinkUeInformationMsg);
      break;
    }
}

Ptr<Packet> 
cv2x_LteEnbRrcProtocolReal::DoEncodeHandoverPreparationInformation (cv2x_LteRrcSap::HandoverPreparationInfo msg)
{
  cv2x_HandoverPreparationInfoHeader h;
  h.SetMessage (msg);

  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

cv2x_LteRrcSap::HandoverPreparationInfo 
cv2x_LteEnbRrcProtocolReal::DoDecodeHandoverPreparationInformation (Ptr<Packet> p)
{
  cv2x_HandoverPreparationInfoHeader h;
  p->RemoveHeader (h);
  cv2x_LteRrcSap::HandoverPreparationInfo msg = h.GetMessage ();
  return msg;
}

Ptr<Packet> 
cv2x_LteEnbRrcProtocolReal::DoEncodeHandoverCommand (cv2x_LteRrcSap::RrcConnectionReconfiguration msg)
{
  cv2x_RrcConnectionReconfigurationHeader h;
  h.SetMessage (msg);
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

cv2x_LteRrcSap::RrcConnectionReconfiguration
cv2x_LteEnbRrcProtocolReal::DoDecodeHandoverCommand (Ptr<Packet> p)
{
  cv2x_RrcConnectionReconfigurationHeader h;
  p->RemoveHeader (h);
  cv2x_LteRrcSap::RrcConnectionReconfiguration msg = h.GetMessage ();
  return msg;
}

//////////////////////////////////////////////////////

cv2x_RealProtocolRlcSapUser::cv2x_RealProtocolRlcSapUser (cv2x_LteEnbRrcProtocolReal* pdcp, uint16_t rnti)
  : m_pdcp (pdcp),
    m_rnti (rnti)
{
}

cv2x_RealProtocolRlcSapUser::cv2x_RealProtocolRlcSapUser ()
{
}

void
cv2x_RealProtocolRlcSapUser::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdcpPdu (m_rnti, p);
}

} // namespace ns3

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
 * Author: Jaume Nin <jnin@cttc.cat>
 *         Nicola Baldo <nbaldo@cttc.cat>
 */


#include "cv2x_epc-enb-application.h"
#include "ns3/log.h"
#include "ns3/mac48-address.h"
#include "ns3/ipv4.h"
#include "ns3/inet-socket-address.h"
#include "ns3/uinteger.h"

#include "cv2x_epc-gtpu-header.h"
#include "cv2x_eps-bearer-tag.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_EpcEnbApplication");

cv2x_EpcEnbApplication::EpsFlowId_t::EpsFlowId_t ()
{
}

cv2x_EpcEnbApplication::EpsFlowId_t::EpsFlowId_t (const uint16_t a, const uint8_t b)
  : m_rnti (a),
    m_bid (b)
{
}

bool
operator == (const cv2x_EpcEnbApplication::EpsFlowId_t &a, const cv2x_EpcEnbApplication::EpsFlowId_t &b)
{
  return ( (a.m_rnti == b.m_rnti) && (a.m_bid == b.m_bid) );
}

bool
operator < (const cv2x_EpcEnbApplication::EpsFlowId_t& a, const cv2x_EpcEnbApplication::EpsFlowId_t& b)
{
  return ( (a.m_rnti < b.m_rnti) || ( (a.m_rnti == b.m_rnti) && (a.m_bid < b.m_bid) ) );
}


TypeId
cv2x_EpcEnbApplication::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_EpcEnbApplication")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddTraceSource ("RxFromEnb",
                     "Receive data packets from LTE Enb Net Device",
                     MakeTraceSourceAccessor (&cv2x_EpcEnbApplication::m_rxLteSocketPktTrace),
                     "ns3::cv2x_EpcEnbApplication::RxTracedCallback")
    .AddTraceSource ("RxFromS1u",
                     "Receive data packets from S1-U Net Device",
                     MakeTraceSourceAccessor (&cv2x_EpcEnbApplication::m_rxS1uSocketPktTrace),
                     "ns3::cv2x_EpcEnbApplication::RxTracedCallback")
    ;
  return tid;
}

void
cv2x_EpcEnbApplication::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_lteSocket = 0;
  m_lteSocket6 = 0;
  m_s1uSocket = 0;
  delete m_s1SapProvider;
  delete m_s1apSapEnb;
}

cv2x_EpcEnbApplication::cv2x_EpcEnbApplication (Ptr<Socket> lteSocket, Ptr<Socket> lteSocket6, Ptr<Socket> s1uSocket, Ipv4Address enbS1uAddress, Ipv4Address sgwS1uAddress, uint16_t cellId)
  : m_lteSocket (lteSocket),
    m_lteSocket6 (lteSocket6),
    m_s1uSocket (s1uSocket),    
    m_enbS1uAddress (enbS1uAddress),
    m_sgwS1uAddress (sgwS1uAddress),
    m_gtpuUdpPort (2152), // fixed by the standard
    m_s1SapUser (0),
    m_s1apSapMme (0),
    m_cellId (cellId)
{
  NS_LOG_FUNCTION (this << lteSocket << s1uSocket << sgwS1uAddress);
  m_s1uSocket->SetRecvCallback (MakeCallback (&cv2x_EpcEnbApplication::RecvFromS1uSocket, this));
  m_lteSocket->SetRecvCallback (MakeCallback (&cv2x_EpcEnbApplication::RecvFromLteSocket, this));
  m_lteSocket6->SetRecvCallback (MakeCallback (&cv2x_EpcEnbApplication::RecvFromLteSocket, this));
  m_s1SapProvider = new cv2x_MemberEpcEnbS1SapProvider<cv2x_EpcEnbApplication> (this);
  m_s1apSapEnb = new cv2x_MemberEpcS1apSapEnb<cv2x_EpcEnbApplication> (this);
}


cv2x_EpcEnbApplication::~cv2x_EpcEnbApplication (void)
{
  NS_LOG_FUNCTION (this);
}


void 
cv2x_EpcEnbApplication::SetS1SapUser (cv2x_EpcEnbS1SapUser * s)
{
  m_s1SapUser = s;
}

  
cv2x_EpcEnbS1SapProvider* 
cv2x_EpcEnbApplication::GetS1SapProvider ()
{
  return m_s1SapProvider;
}

void 
cv2x_EpcEnbApplication::SetS1apSapMme (cv2x_EpcS1apSapMme * s)
{
  m_s1apSapMme = s;
}

  
cv2x_EpcS1apSapEnb* 
cv2x_EpcEnbApplication::GetS1apSapEnb ()
{
  return m_s1apSapEnb;
}

void 
cv2x_EpcEnbApplication::DoInitialUeMessage (uint64_t imsi, uint16_t rnti)
{
  NS_LOG_FUNCTION (this);
  // side effect: create entry if not exist
  m_imsiRntiMap[imsi] = rnti;
  m_s1apSapMme->InitialUeMessage (imsi, rnti, imsi, m_cellId);
}

void 
cv2x_EpcEnbApplication::DoPathSwitchRequest (cv2x_EpcEnbS1SapProvider::PathSwitchRequestParameters params)
{
  NS_LOG_FUNCTION (this);
  uint16_t enbUeS1Id = params.rnti;  
  uint64_t mmeUeS1Id = params.mmeUeS1Id;
  uint64_t imsi = mmeUeS1Id;
  // side effect: create entry if not exist
  m_imsiRntiMap[imsi] = params.rnti;

  uint16_t gci = params.cellId;
  std::list<cv2x_EpcS1apSapMme::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList;
  for (std::list<cv2x_EpcEnbS1SapProvider::BearerToBeSwitched>::iterator bit = params.bearersToBeSwitched.begin ();
       bit != params.bearersToBeSwitched.end ();
       ++bit)
    {
      EpsFlowId_t flowId;
      flowId.m_rnti = params.rnti;
      flowId.m_bid = bit->epsBearerId;
      uint32_t teid = bit->teid;
      
      EpsFlowId_t rbid (params.rnti, bit->epsBearerId);
      // side effect: create entries if not exist
      m_rbidTeidMap[params.rnti][bit->epsBearerId] = teid;
      m_teidRbidMap[teid] = rbid;

      cv2x_EpcS1apSapMme::ErabSwitchedInDownlinkItem erab;
      erab.erabId = bit->epsBearerId;
      erab.enbTransportLayerAddress = m_enbS1uAddress;
      erab.enbTeid = bit->teid;

      erabToBeSwitchedInDownlinkList.push_back (erab);
    }
  m_s1apSapMme->PathSwitchRequest (enbUeS1Id, mmeUeS1Id, gci, erabToBeSwitchedInDownlinkList);
}

void 
cv2x_EpcEnbApplication::DoUeContextRelease (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  std::map<uint16_t, std::map<uint8_t, uint32_t> >::iterator rntiIt = m_rbidTeidMap.find (rnti);
  if (rntiIt != m_rbidTeidMap.end ())
    {
      for (std::map<uint8_t, uint32_t>::iterator bidIt = rntiIt->second.begin ();
           bidIt != rntiIt->second.end ();
           ++bidIt)
        {
          uint32_t teid = bidIt->second;
          m_teidRbidMap.erase (teid);
        }
      m_rbidTeidMap.erase (rntiIt);
    }
}

void 
cv2x_EpcEnbApplication::DoInitialContextSetupRequest (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<cv2x_EpcS1apSapEnb::ErabToBeSetupItem> erabToBeSetupList)
{
  NS_LOG_FUNCTION (this);
  
  for (std::list<cv2x_EpcS1apSapEnb::ErabToBeSetupItem>::iterator erabIt = erabToBeSetupList.begin ();
       erabIt != erabToBeSetupList.end ();
       ++erabIt)
    {
      // request the RRC to setup a radio bearer

      uint64_t imsi = mmeUeS1Id;
      std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
      NS_ASSERT_MSG (imsiIt != m_imsiRntiMap.end (), "unknown IMSI");
      uint16_t rnti = imsiIt->second;
      
      struct cv2x_EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters params;
      params.rnti = rnti;
      params.bearer = erabIt->erabLevelQosParameters;
      params.bearerId = erabIt->erabId;
      params.gtpTeid = erabIt->sgwTeid;
      m_s1SapUser->DataRadioBearerSetupRequest (params);

      EpsFlowId_t rbid (rnti, erabIt->erabId);
      // side effect: create entries if not exist
      m_rbidTeidMap[rnti][erabIt->erabId] = params.gtpTeid;
      m_teidRbidMap[params.gtpTeid] = rbid;

    }
}

void 
cv2x_EpcEnbApplication::DoPathSwitchRequestAcknowledge (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t gci, std::list<cv2x_EpcS1apSapEnb::ErabSwitchedInUplinkItem> erabToBeSwitchedInUplinkList)
{
  NS_LOG_FUNCTION (this);

  uint64_t imsi = mmeUeS1Id;
  std::map<uint64_t, uint16_t>::iterator imsiIt = m_imsiRntiMap.find (imsi);
  NS_ASSERT_MSG (imsiIt != m_imsiRntiMap.end (), "unknown IMSI");
  uint16_t rnti = imsiIt->second;
  cv2x_EpcEnbS1SapUser::PathSwitchRequestAcknowledgeParameters params;
  params.rnti = rnti;
  m_s1SapUser->PathSwitchRequestAcknowledge (params);
}

void 
cv2x_EpcEnbApplication::RecvFromLteSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);  
  if(m_lteSocket6)
    {
      NS_ASSERT (socket == m_lteSocket || socket == m_lteSocket6);
    }
  else
    {
      NS_ASSERT (socket == m_lteSocket);
    }
  Ptr<Packet> packet = socket->Recv ();

  cv2x_EpsBearerTag tag;
  bool found = packet->RemovePacketTag (tag);
  NS_ASSERT (found);
  uint16_t rnti = tag.GetRnti ();
  uint8_t bid = tag.GetBid ();
  NS_LOG_LOGIC ("received packet with RNTI=" << (uint32_t) rnti << ", BID=" << (uint32_t)  bid);
  std::map<uint16_t, std::map<uint8_t, uint32_t> >::iterator rntiIt = m_rbidTeidMap.find (rnti);
  if (rntiIt == m_rbidTeidMap.end ())
    {
      NS_LOG_WARN ("UE context not found, discarding packet");
    }
  else
    {
      std::map<uint8_t, uint32_t>::iterator bidIt = rntiIt->second.find (bid);
      NS_ASSERT (bidIt != rntiIt->second.end ());
      uint32_t teid = bidIt->second;
      m_rxLteSocketPktTrace (packet->Copy ());
      SendToS1uSocket (packet, teid);
    }
}

void 
cv2x_EpcEnbApplication::RecvFromS1uSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);  
  NS_ASSERT (socket == m_s1uSocket);
  Ptr<Packet> packet = socket->Recv ();
  cv2x_GtpuHeader gtpu;
  packet->RemoveHeader (gtpu);
  uint32_t teid = gtpu.GetTeid ();
  std::map<uint32_t, EpsFlowId_t>::iterator it = m_teidRbidMap.find (teid);
  NS_ASSERT (it != m_teidRbidMap.end ());

  m_rxS1uSocketPktTrace (packet->Copy ());
  SendToLteSocket (packet, it->second.m_rnti, it->second.m_bid);
}

void 
cv2x_EpcEnbApplication::SendToLteSocket (Ptr<Packet> packet, uint16_t rnti, uint8_t bid)
{
  NS_LOG_FUNCTION (this << packet << rnti << (uint16_t) bid << packet->GetSize ());  
  cv2x_EpsBearerTag tag (rnti, bid);
  packet->AddPacketTag (tag);
  uint8_t ipType;

  packet->CopyData (&ipType, 1);
  ipType = (ipType>>4) & 0x0f;

  int sentBytes;
  if (ipType == 0x04)
    {
      sentBytes = m_lteSocket->Send (packet);
    }
  else if (ipType == 0x06)
    {
      sentBytes = m_lteSocket6->Send (packet);
    }
  else
    {
      NS_ABORT_MSG ("cv2x_EpcEnbApplication::SendToLteSocket - Unknown IP type...");
    }

  NS_ASSERT (sentBytes > 0);
}


void 
cv2x_EpcEnbApplication::SendToS1uSocket (Ptr<Packet> packet, uint32_t teid)
{
  NS_LOG_FUNCTION (this << packet << teid <<  packet->GetSize ());  
  cv2x_GtpuHeader gtpu;
  gtpu.SetTeid (teid);
  // From 3GPP TS 29.281 v10.0.0 Section 5.1
  // Length of the payload + the non obligatory GTP-U header
  gtpu.SetLength (packet->GetSize () + gtpu.GetSerializedSize () - 8);  
  packet->AddHeader (gtpu);
  uint32_t flags = 0;
  m_s1uSocket->SendTo (packet, flags, InetSocketAddress (m_sgwS1uAddress, m_gtpuUdpPort));
}

void
cv2x_EpcEnbApplication::DoReleaseIndication (uint64_t imsi, uint16_t rnti, uint8_t bearerId)
{
  NS_LOG_FUNCTION (this << bearerId );
  std::list<cv2x_EpcS1apSapMme::ErabToBeReleasedIndication> erabToBeReleaseIndication;
  cv2x_EpcS1apSapMme::ErabToBeReleasedIndication erab;
  erab.erabId = bearerId;
  erabToBeReleaseIndication.push_back (erab);
  //From 3GPP TS 23401-950 Section 5.4.4.2, enB sends EPS bearer Identity in Bearer Release Indication message to MME
  m_s1apSapMme->ErabReleaseIndication (imsi, rnti, erabToBeReleaseIndication);
}

}  // namespace ns3

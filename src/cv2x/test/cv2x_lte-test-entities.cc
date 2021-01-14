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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/node.h"

#include "ns3/cv2x_lte-rlc-header.h"
#include "ns3/cv2x_lte-rlc-am-header.h"
#include "ns3/cv2x_lte-pdcp-header.h"

#include "cv2x_lte-test-entities.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteTestEntities");

/////////////////////////////////////////////////////////////////////

TypeId
cv2x_LteTestRrc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteTestRrc")
    .SetParent<Object> ()
    .AddConstructor<cv2x_LteTestRrc> ()
    ;

  return tid;
}

cv2x_LteTestRrc::cv2x_LteTestRrc ()
{
  NS_LOG_FUNCTION (this);

  m_txPdus = 0;
  m_txBytes = 0;
  m_rxPdus = 0;
  m_rxBytes = 0;
  m_txLastTime = Time (0);
  m_rxLastTime = Time (0);;

  m_pdcpSapUser = new cv2x_LtePdcpSpecificLtePdcpSapUser<cv2x_LteTestRrc> (this);
//   Simulator::ScheduleNow (&cv2x_LteTestRrc::Start, this);
}

cv2x_LteTestRrc::~cv2x_LteTestRrc ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteTestRrc::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_pdcpSapUser;
}

void
cv2x_LteTestRrc::SetDevice (Ptr<NetDevice> device)
{
  m_device = device;
}

void
cv2x_LteTestRrc::SetLtePdcpSapProvider (cv2x_LtePdcpSapProvider* s)
{
  m_pdcpSapProvider = s;
}

cv2x_LtePdcpSapUser*
cv2x_LteTestRrc::GetLtePdcpSapUser (void)
{
  return m_pdcpSapUser;
}


std::string
cv2x_LteTestRrc::GetDataReceived (void)
{
  NS_LOG_FUNCTION (this);
  return m_receivedData;
}

// Stats
uint32_t
cv2x_LteTestRrc::GetTxPdus (void)
{
  NS_LOG_FUNCTION (this << m_txPdus);
  return m_txPdus;
}

uint32_t
cv2x_LteTestRrc::GetTxBytes (void)
{
  NS_LOG_FUNCTION (this << m_txBytes);
  return m_txBytes;
}

uint32_t
cv2x_LteTestRrc::GetRxPdus (void)
{
  NS_LOG_FUNCTION (this << m_rxPdus);
  return m_rxPdus;
}

uint32_t
cv2x_LteTestRrc::GetRxBytes (void)
{
  NS_LOG_FUNCTION (this << m_rxBytes);
  return m_rxBytes;
}

Time
cv2x_LteTestRrc::GetTxLastTime (void)
{
  NS_LOG_FUNCTION (this << m_txLastTime);
  return m_txLastTime;
}

Time
cv2x_LteTestRrc::GetRxLastTime (void)
{
  NS_LOG_FUNCTION (this << m_rxLastTime);
  return m_rxLastTime;
}


void
cv2x_LteTestRrc::SetArrivalTime (Time arrivalTime)
{
  NS_LOG_FUNCTION (this << arrivalTime);
  m_arrivalTime = arrivalTime;
}

void
cv2x_LteTestRrc::SetPduSize (uint32_t pduSize)
{
  NS_LOG_FUNCTION (this << pduSize);
  m_pduSize = pduSize;
}


/**
 * PDCP SAP
 */

void
cv2x_LteTestRrc::DoReceivePdcpSdu (cv2x_LtePdcpSapUser::ReceivePdcpSduParameters params)
{
  NS_LOG_FUNCTION (this << params.pdcpSdu->GetSize ());
  Ptr<Packet> p = params.pdcpSdu;
//   NS_LOG_LOGIC ("PDU received = " << (*p));

  uint32_t dataLen = p->GetSize ();
  uint8_t *buf = new uint8_t[dataLen];

  // Stats
  m_rxPdus++;
  m_rxBytes += dataLen;
  m_rxLastTime = Simulator::Now ();

  p->CopyData (buf, dataLen);
  m_receivedData = std::string ((char *)buf, dataLen);

//   NS_LOG_LOGIC (m_receivedData);

  delete [] buf;
}

/**
 * START
 */

void
cv2x_LteTestRrc::Start ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT_MSG (m_arrivalTime != Time (0), "Arrival time must be different from 0");

  // Stats
  m_txPdus++;
  m_txBytes += m_pduSize;
  m_txLastTime = Simulator::Now ();

  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters p;
  p.rnti = 1111;
  p.lcid = 222;
  p.srcL2Id = 0;
  p.dstL2Id = 0;
  p.pdcpSdu = Create<Packet> (m_pduSize);
  
  bool haveContext = false;
  Ptr<Node> node;
  if (m_device != 0)
    {
      node = m_device->GetNode ();
      if (node != 0)
        {                    
          haveContext = true;
        }
    }
  if (haveContext)
    {
      Simulator::ScheduleWithContext (node->GetId (), Seconds (0), &cv2x_LtePdcpSapProvider::TransmitPdcpSdu, m_pdcpSapProvider, p);
    }
  else
    {
      Simulator::Schedule (Seconds (0), &cv2x_LtePdcpSapProvider::TransmitPdcpSdu, m_pdcpSapProvider, p);
    }

  m_nextPdu = Simulator::Schedule (m_arrivalTime, &cv2x_LteTestRrc::Start, this);
//   Simulator::Run ();
}

void
cv2x_LteTestRrc::Stop ()
{
  NS_LOG_FUNCTION (this);
  m_nextPdu.Cancel ();
}

void
cv2x_LteTestRrc::SendData (Time at, std::string dataToSend)
{
  NS_LOG_FUNCTION (this << at << dataToSend.length () << dataToSend);

  // Stats
  m_txPdus++;
  m_txBytes += dataToSend.length ();

  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters p;
  p.rnti = 1111;
  p.lcid = 222;
  p.srcL2Id = 0;
  p.dstL2Id = 0;

  NS_LOG_LOGIC ("Data(" << dataToSend.length () << ") = " << dataToSend.data ());
  p.pdcpSdu = Create<Packet> ((uint8_t *) dataToSend.data (), dataToSend.length ());

  NS_LOG_LOGIC ("Packet(" << p.pdcpSdu->GetSize () << ")");
  Simulator::Schedule (at, &cv2x_LtePdcpSapProvider::TransmitPdcpSdu, m_pdcpSapProvider, p);
}

/////////////////////////////////////////////////////////////////////

TypeId
cv2x_LteTestPdcp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteTestPdcp")
    .SetParent<Object> ()
    .AddConstructor<cv2x_LteTestPdcp> ()
    ;

  return tid;
}

cv2x_LteTestPdcp::cv2x_LteTestPdcp ()
{
  NS_LOG_FUNCTION (this);
  m_rlcSapUser = new cv2x_LteRlcSpecificLteRlcSapUser<cv2x_LteTestPdcp> (this);
  Simulator::ScheduleNow (&cv2x_LteTestPdcp::Start, this);
}

cv2x_LteTestPdcp::~cv2x_LteTestPdcp ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteTestPdcp::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_rlcSapUser;
}

void
cv2x_LteTestPdcp::SetLteRlcSapProvider (cv2x_LteRlcSapProvider* s)
{
  m_rlcSapProvider = s;
}

cv2x_LteRlcSapUser*
cv2x_LteTestPdcp::GetLteRlcSapUser (void)
{
  return m_rlcSapUser;
}


std::string
cv2x_LteTestPdcp::GetDataReceived (void)
{
  NS_LOG_FUNCTION (this);

  return m_receivedData;
}


/**
 * RLC SAP
 */

void
cv2x_LteTestPdcp::DoReceivePdcpPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p->GetSize ());
  NS_LOG_LOGIC ("Data = " << (*p));

  uint32_t dataLen = p->GetSize ();
  uint8_t *buf = new uint8_t[dataLen];
  p->CopyData (buf, dataLen);
  m_receivedData = std::string ((char *)buf, dataLen);

  NS_LOG_LOGIC (m_receivedData);

  delete [] buf;
}

/**
 * START
 */

void
cv2x_LteTestPdcp::Start ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteTestPdcp::SendData (Time time, std::string dataToSend)
{
  NS_LOG_FUNCTION (this << time << dataToSend.length () << dataToSend);

  cv2x_LteRlcSapProvider::TransmitPdcpPduParameters p;
  p.rnti = 1111;
  p.lcid = 222;
  p.srcL2Id = 0;
  p.dstL2Id = 0;

  NS_LOG_LOGIC ("Data(" << dataToSend.length () << ") = " << dataToSend.data ());
  p.pdcpPdu = Create<Packet> ((uint8_t *) dataToSend.data (), dataToSend.length ());

  NS_LOG_LOGIC ("Packet(" << p.pdcpPdu->GetSize () << ")");
  Simulator::Schedule (time, &cv2x_LteRlcSapProvider::TransmitPdcpPdu, m_rlcSapProvider, p);
}

/////////////////////////////////////////////////////////////////////

TypeId
cv2x_LteTestMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteTestMac")
    .SetParent<Object> ()
    .AddConstructor<cv2x_LteTestMac> ()
    ;

  return tid;
}

cv2x_LteTestMac::cv2x_LteTestMac ()
{
  NS_LOG_FUNCTION (this);
  m_device = 0;
  m_macSapProvider = new cv2x_EnbMacMemberLteMacSapProvider<cv2x_LteTestMac> (this);
  m_macSapUser = 0;
  m_macLoopback = 0;
  m_pdcpHeaderPresent = false;
  m_rlcHeaderType = UM_RLC_HEADER;
  m_txOpportunityMode = MANUAL_MODE;
  m_txOppTime = Seconds (0.001);
  m_txOppSize = 0;

  m_txPdus = 0;
  m_txBytes = 0;
  m_rxPdus = 0;
  m_rxBytes = 0;

//   m_cmacSapProvider = new cv2x_EnbMacMemberLteEnbCmacSapProvider (this);
//   m_schedSapUser = new cv2x_EnbMacMemberFfMacSchedSapUser (this);
//   m_cschedSapUser = new cv2x_EnbMacMemberFfMacCschedSapUser (this);
//   m_enbPhySapUser = new cv2x_EnbMacMemberLteEnbPhySapUser (this);
}

cv2x_LteTestMac::~cv2x_LteTestMac ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteTestMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_macSapProvider;
//   delete m_cmacSapProvider;
//   delete m_schedSapUser;
//   delete m_cschedSapUser;
//   delete m_enbPhySapUser;

  m_device = 0;
}

void
cv2x_LteTestMac::SetDevice (Ptr<NetDevice> device)
{
  m_device = device;
}

void
cv2x_LteTestMac::SetLteMacSapUser (cv2x_LteMacSapUser* s)
{
  m_macSapUser = s;
}

cv2x_LteMacSapProvider*
cv2x_LteTestMac::GetLteMacSapProvider (void)
{
  return m_macSapProvider;
}

void
cv2x_LteTestMac::SetLteMacLoopback (Ptr<cv2x_LteTestMac> s)
{
  m_macLoopback = s;
}

std::string
cv2x_LteTestMac::GetDataReceived (void)
{
  NS_LOG_FUNCTION (this);
  return m_receivedData;
}

// Stats
uint32_t
cv2x_LteTestMac::GetTxPdus (void)
{
  NS_LOG_FUNCTION (this << m_txPdus);
  return m_txPdus;
}

uint32_t
cv2x_LteTestMac::GetTxBytes (void)
{
  NS_LOG_FUNCTION (this << m_txBytes);
  return m_txBytes;
}

uint32_t
cv2x_LteTestMac::GetRxPdus (void)
{
  NS_LOG_FUNCTION (this << m_rxPdus);
  return m_rxPdus;
}

uint32_t
cv2x_LteTestMac::GetRxBytes (void)
{
  NS_LOG_FUNCTION (this << m_rxBytes);
  return m_rxBytes;
}


void
cv2x_LteTestMac::SendTxOpportunity (Time time, uint32_t bytes)
{
  NS_LOG_FUNCTION (this << time << bytes);
  bool haveContext = false;
  Ptr<Node> node;
  if (m_device != 0)
    {
      node = m_device->GetNode ();
      if (node != 0)
        {                    
          haveContext = true;
        }
    }
  if (haveContext)
    {
      Simulator::ScheduleWithContext (node->GetId (), time, &cv2x_LteMacSapUser::NotifyTxOpportunity, m_macSapUser, bytes, 0, 0, 0, 0, 0);
    }
  else
    {
      Simulator::Schedule (time, &cv2x_LteMacSapUser::NotifyTxOpportunity, m_macSapUser, bytes, 0, 0, 0, 0, 0);
    }
    
  if (m_txOpportunityMode == RANDOM_MODE)
  {
    if (m_txOppTime != Seconds (0))
    {
      Simulator::Schedule (m_txOppTime, &cv2x_LteTestMac::SendTxOpportunity, this, m_txOppTime, m_txOppSize);
    }
  }
}

void
cv2x_LteTestMac::SetPdcpHeaderPresent (bool present)
{
  NS_LOG_FUNCTION (this << present);
  m_pdcpHeaderPresent = present;
}

void
cv2x_LteTestMac::SetRlcHeaderType (uint8_t rlcHeaderType)
{
  NS_LOG_FUNCTION (this << rlcHeaderType);
  m_rlcHeaderType = rlcHeaderType;
}

void
cv2x_LteTestMac::SetTxOpportunityMode (uint8_t mode)
{
  NS_LOG_FUNCTION (this << (uint32_t)mode);
  m_txOpportunityMode = mode;

  if (m_txOpportunityMode == RANDOM_MODE)
    {
      if (m_txOppTime != Seconds (0.0))
        {
          SendTxOpportunity (m_txOppTime, m_txOppSize);
        }
    }
}

void
cv2x_LteTestMac::SetTxOppTime (Time txOppTime)
{
  NS_LOG_FUNCTION (this << txOppTime);
  m_txOppTime = txOppTime;
}

void
cv2x_LteTestMac::SetTxOppSize (uint32_t txOppSize)
{
  NS_LOG_FUNCTION (this << txOppSize);
  m_txOppSize = txOppSize;
}


/**
 * MAC SAP
 */

void
cv2x_LteTestMac::DoTransmitPdu (cv2x_LteMacSapProvider::TransmitPduParameters params)
{
  NS_LOG_FUNCTION (this << params.pdu->GetSize ());

  m_txPdus++;
  m_txBytes += params.pdu->GetSize ();

  if (m_device)
    {
      m_device->Send (params.pdu, m_device->GetBroadcast (), 0);
    }
  else if (m_macLoopback)
    {
      Simulator::Schedule (Seconds (0.1), &cv2x_LteMacSapUser::ReceivePdu,
                           m_macLoopback->m_macSapUser, params.pdu, params.rnti, params.lcid);
    }
  else
    {
      cv2x_LtePdcpHeader pdcpHeader;

      if (m_rlcHeaderType == AM_RLC_HEADER)
        {
          // Remove AM RLC header
          cv2x_LteRlcAmHeader rlcAmHeader;
          params.pdu->RemoveHeader (rlcAmHeader);
          NS_LOG_LOGIC ("AM RLC header: " << rlcAmHeader);
        }
      else // if (m_rlcHeaderType == UM_RLC_HEADER)
        {
          // Remove UM RLC header
          cv2x_LteRlcHeader rlcHeader;
          params.pdu->RemoveHeader (rlcHeader);
          NS_LOG_LOGIC ("UM RLC header: " << rlcHeader);
        }

      // Remove PDCP header, if present
      if (m_pdcpHeaderPresent)
        {
          params.pdu->RemoveHeader (pdcpHeader);
          NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);
        }

      // Copy data to a string
      uint32_t dataLen = params.pdu->GetSize ();
      uint8_t *buf = new uint8_t[dataLen];
      params.pdu->CopyData (buf, dataLen);
      m_receivedData = std::string ((char *)buf, dataLen);

      NS_LOG_LOGIC ("Data (" << dataLen << ") = " << m_receivedData);
      delete [] buf;
    }
}

void
cv2x_LteTestMac::DoReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this << params.txQueueSize << params.retxQueueSize << params.statusPduSize);

  if (m_txOpportunityMode == AUTOMATIC_MODE)
    {
      // cancel all previously scheduled TxOpps
      for (std::list<EventId>::iterator it = m_nextTxOppList.begin ();
           it != m_nextTxOppList.end ();
           ++it)
        {          
          it->Cancel ();
        }
      m_nextTxOppList.clear ();

      int32_t size = params.statusPduSize + params.txQueueSize  + params.retxQueueSize;
      Time time = m_txOppTime;
      while (size > 0)
        {
          EventId e = Simulator::Schedule (time, 
                                           &cv2x_LteMacSapUser::NotifyTxOpportunity,
                                           m_macSapUser, m_txOppSize, 0, 0, 0, params.rnti, params.lcid);
          m_nextTxOppList.push_back (e);
          size -= m_txOppSize;
          time += m_txOppTime;
        }
    }
}


bool
cv2x_LteTestMac::Receive (Ptr<NetDevice> nd, Ptr<const Packet> p, uint16_t protocol, const Address& addr)
{
  NS_LOG_FUNCTION (this << addr << protocol << p->GetSize ());

  m_rxPdus++;
  m_rxBytes += p->GetSize ();

  Ptr<Packet> packet = p->Copy ();
  m_macSapUser->ReceivePdu (packet, 0, 0);
  return true;
}








NS_OBJECT_ENSURE_REGISTERED (cv2x_EpcTestRrc);

cv2x_EpcTestRrc::cv2x_EpcTestRrc ()
  : m_s1SapProvider (0)
{
  NS_LOG_FUNCTION (this);
  m_s1SapUser = new cv2x_MemberEpcEnbS1SapUser<cv2x_EpcTestRrc> (this);
}


cv2x_EpcTestRrc::~cv2x_EpcTestRrc ()
{
  NS_LOG_FUNCTION (this);
}


void
cv2x_EpcTestRrc::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_s1SapUser;
}

TypeId
cv2x_EpcTestRrc::GetTypeId (void)
{
  NS_LOG_FUNCTION ("cv2x_EpcTestRrc::GetTypeId");
  static TypeId tid = TypeId ("ns3::cv2x_EpcTestRrc")
    .SetParent<Object> ()
    .AddConstructor<cv2x_EpcTestRrc> ()
  ;
  return tid;
}
void 
cv2x_EpcTestRrc::SetS1SapProvider (cv2x_EpcEnbS1SapProvider * s)
{
  m_s1SapProvider = s;
}

  
cv2x_EpcEnbS1SapUser* 
cv2x_EpcTestRrc::GetS1SapUser ()
{
  return m_s1SapUser;
}

void 
cv2x_EpcTestRrc::DoDataRadioBearerSetupRequest (cv2x_EpcEnbS1SapUser::DataRadioBearerSetupRequestParameters request)
{

}
  
void 
cv2x_EpcTestRrc::DoPathSwitchRequestAcknowledge (cv2x_EpcEnbS1SapUser::PathSwitchRequestAcknowledgeParameters params)
{

}


} // namespace ns3


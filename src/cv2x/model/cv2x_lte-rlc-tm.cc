/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011,2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Authors: Manuel Requena <manuel.requena@cttc.es>
 *          Nicola Baldo <nbaldo@cttc.es>
 * Modified by: NIST (D2D)
 */

#include "ns3/simulator.h"
#include "ns3/log.h"

#include "ns3/cv2x_lte-rlc-tm.h"
#include "ns3/cv2x_lte-rlc-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteRlcTm");

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteRlcTm);

cv2x_LteRlcTm::cv2x_LteRlcTm ()
  : m_maxTxBufferSize (0),
    m_txBufferSize (0)
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteRlcTm::~cv2x_LteRlcTm ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
cv2x_LteRlcTm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteRlcTm")
    .SetParent<cv2x_LteRlc> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteRlcTm> ()
    .AddAttribute ("MaxTxBufferSize",
                   "Maximum Size of the Transmission Buffer (in Bytes)",
                   UintegerValue (2 * 1024 * 1024),
                   MakeUintegerAccessor (&cv2x_LteRlcTm::m_maxTxBufferSize),
                   MakeUintegerChecker<uint32_t> ())
    ;
  return tid;
}

void
cv2x_LteRlcTm::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_rbsTimer.Cancel ();
  m_txBuffer.clear ();

  cv2x_LteRlc::DoDispose ();
}


/**
 * RLC SAP
 */

void
cv2x_LteRlcTm::DoTransmitPdcpPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  if (m_txBufferSize + p->GetSize () <= m_maxTxBufferSize)
    {
      /** Store arrival time */
      cv2x_RlcTag timeTag (Simulator::Now ());
      p->AddPacketTag (timeTag);

      NS_LOG_LOGIC ("Tx Buffer: New packet added");
      m_txBuffer.push_back (p);
      m_txBufferSize += p->GetSize ();
      NS_LOG_LOGIC ("NumOfBuffers = " << m_txBuffer.size() );
      NS_LOG_LOGIC ("txBufferSize = " << m_txBufferSize);
    }
  else
    {
      // Discard full RLC SDU
      NS_LOG_LOGIC ("TxBuffer is full. RLC SDU discarded");
      NS_LOG_LOGIC ("MaxTxBufferSize = " << m_maxTxBufferSize);
      NS_LOG_LOGIC ("txBufferSize    = " << m_txBufferSize);
      NS_LOG_LOGIC ("packet size     = " << p->GetSize ());
    }

  /** Report Buffer Status */
  DoReportBufferStatus ();
  m_rbsTimer.Cancel ();
}


/**
 * MAC SAP
 */

void
cv2x_LteRlcTm::DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << bytes  << (uint32_t) layer << (uint32_t) harqId);

  // 5.1.1.1 Transmit operations 
  // 5.1.1.1.1 General
  // When submitting a new TMD PDU to lower layer, the transmitting TM RLC entity shall:
  // - submit a RLC SDU without any modification to lower layer.


  if ( m_txBuffer.size () == 0 )
    {
      NS_LOG_LOGIC ("No data pending");
      return;
    }

  Ptr<Packet> packet = (*(m_txBuffer.begin ()))->Copy ();

  if (bytes < packet->GetSize ())
    {
      NS_LOG_WARN ("TX opportunity too small = " << bytes << " (PDU size: " << packet->GetSize () << ")");
      return;
    }

  m_txBufferSize -= (*(m_txBuffer.begin()))->GetSize ();
  m_txBuffer.erase (m_txBuffer.begin ());
 
  // Sender timestamp
  cv2x_RlcTag rlcTag (Simulator::Now ());
  packet->ReplacePacketTag (rlcTag);
  m_txPdu (m_rnti, m_lcid, packet->GetSize ());

  // Send RLC PDU to MAC layer
  cv2x_LteMacSapProvider::TransmitPduParameters params;
  params.pdu = packet;
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  params.srcL2Id = m_srcL2Id;
  params.dstL2Id = m_dstL2Id;
  params.layer = layer;
  params.harqProcessId = harqId;
  params.componentCarrierId = componentCarrierId;

  m_macSapProvider->TransmitPdu (params);

  if (! m_txBuffer.empty ())
    {
      m_rbsTimer.Cancel ();
      m_rbsTimer = Simulator::Schedule (MilliSeconds (10), &cv2x_LteRlcTm::ExpireRbsTimer, this);
    }
}

void
cv2x_LteRlcTm::DoNotifyHarqDeliveryFailure ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteRlcTm::DoReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  // Receiver timestamp
  cv2x_RlcTag rlcTag;
  Time delay;
  NS_ASSERT_MSG (p->PeekPacketTag (rlcTag), "cv2x_RlcTag is missing");
  p->RemovePacketTag (rlcTag);
  delay = Simulator::Now() - rlcTag.GetSenderTimestamp ();
  m_rxPdu (m_rnti, m_lcid, p->GetSize (), delay.GetNanoSeconds ());

  // 5.1.1.2 Receive operations 
  // 5.1.1.2.1  General
  // When receiving a new TMD PDU from lower layer, the receiving TM RLC entity shall:
  // - deliver the TMD PDU without any modification to upper layer.

   m_rlcSapUser->ReceivePdcpPdu (p);
}


void
cv2x_LteRlcTm::DoReportBufferStatus (void)
{
  Time holDelay (0);
  uint32_t queueSize = 0;

  if (! m_txBuffer.empty ())
    {
      cv2x_RlcTag holTimeTag;
      NS_ASSERT_MSG (m_txBuffer.front ()->PeekPacketTag (holTimeTag), "cv2x_RlcTag is missing");
      m_txBuffer.front ()->PeekPacketTag (holTimeTag);
      holDelay = Simulator::Now () - holTimeTag.GetSenderTimestamp ();

      queueSize = m_txBufferSize; // just data in tx queue (no header overhead for RLC TM)
    }

  cv2x_LteMacSapProvider::ReportBufferStatusParameters r;
  r.rnti = m_rnti;
  r.lcid = m_lcid;
  r.srcL2Id = m_srcL2Id;
  r.dstL2Id = m_dstL2Id;
  r.txQueueSize = queueSize;
  r.txQueueHolDelay = holDelay.GetMilliSeconds () ;
  r.retxQueueSize = 0;
  r.retxQueueHolDelay = 0;
  r.statusPduSize = 0;

  NS_LOG_LOGIC ("Send ReportBufferStatus = " << r.txQueueSize << ", " << r.txQueueHolDelay );
  m_macSapProvider->ReportBufferStatus (r);
}

void
cv2x_LteRlcTm::ExpireRbsTimer (void)
{
  NS_LOG_LOGIC ("RBS Timer expires");

  if (! m_txBuffer.empty ())
    {
      DoReportBufferStatus ();
      m_rbsTimer = Simulator::Schedule (MilliSeconds (10), &cv2x_LteRlcTm::ExpireRbsTimer, this);
    }
}

} // namespace ns3

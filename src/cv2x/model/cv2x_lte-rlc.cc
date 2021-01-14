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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Modified by: NIST (D2D)
 */


#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ns3/cv2x_lte-rlc.h"
#include "ns3/cv2x_lte-rlc-tag.h"
// #include "cv2x_lte-mac-sap.h"
#include "ns3/cv2x_lte-rlc-sap.h"
// #include "cv2x_ff-mac-sched-sap.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteRlc");


/// cv2x_LteRlcSpecificLteMacSapUser class
class cv2x_LteRlcSpecificLteMacSapUser : public cv2x_LteMacSapUser
{
public:
  /**
   * Constructor
   *
   * \param rlc the RLC
   */
  cv2x_LteRlcSpecificLteMacSapUser (cv2x_LteRlc* rlc);

  // Interface implemented from cv2x_LteMacSapUser
  virtual void NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid);
  virtual void NotifyHarqDeliveryFailure ();
  virtual void ReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid);

private:
  cv2x_LteRlcSpecificLteMacSapUser ();
  cv2x_LteRlc* m_rlc; ///< the RLC
};

cv2x_LteRlcSpecificLteMacSapUser::cv2x_LteRlcSpecificLteMacSapUser (cv2x_LteRlc* rlc)
  : m_rlc (rlc)
{
}

cv2x_LteRlcSpecificLteMacSapUser::cv2x_LteRlcSpecificLteMacSapUser ()
{
}

void
cv2x_LteRlcSpecificLteMacSapUser::NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid)
{
  m_rlc->DoNotifyTxOpportunity (bytes, layer, harqId, componentCarrierId, rnti, lcid);
}

void
cv2x_LteRlcSpecificLteMacSapUser::NotifyHarqDeliveryFailure ()
{
  m_rlc->DoNotifyHarqDeliveryFailure ();
}

void
cv2x_LteRlcSpecificLteMacSapUser::ReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid)
{
  m_rlc->DoReceivePdu (p, rnti, lcid);
}


///////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteRlc);

cv2x_LteRlc::cv2x_LteRlc ()
  : m_rlcSapUser (0),
    m_macSapProvider (0),
    m_rnti (0),
    m_lcid (0),
    m_srcL2Id (0),
    m_dstL2Id (0)
{
  NS_LOG_FUNCTION (this);
  m_rlcSapProvider = new cv2x_LteRlcSpecificLteRlcSapProvider<cv2x_LteRlc> (this);
  m_macSapUser = new cv2x_LteRlcSpecificLteMacSapUser (this);
}

cv2x_LteRlc::~cv2x_LteRlc ()
{
  NS_LOG_FUNCTION (this);
}

TypeId cv2x_LteRlc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteRlc")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddTraceSource ("TxPDU",
                     "PDU transmission notified to the MAC.",
                     MakeTraceSourceAccessor (&cv2x_LteRlc::m_txPdu),
                     "ns3::cv2x_LteRlc::NotifyTxTracedCallback")
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&cv2x_LteRlc::m_rxPdu),
                     "ns3::cv2x_LteRlc::ReceiveTracedCallback")
    ;
  return tid;
}

void
cv2x_LteRlc::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete (m_rlcSapProvider);
  delete (m_macSapUser);
}

void
cv2x_LteRlc::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << (uint32_t) rnti);
  m_rnti = rnti;
}

void
cv2x_LteRlc::SetLcId (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << (uint32_t) lcId);
  m_lcid = lcId;
}

void
cv2x_LteRlc::SetSourceL2Id (uint32_t src)
{
  NS_LOG_FUNCTION (this << src);
  m_srcL2Id = src;
}
  
void
cv2x_LteRlc::SetDestinationL2Id (uint32_t dst)
{
  NS_LOG_FUNCTION (this << dst);
  m_dstL2Id = dst;
}

void
cv2x_LteRlc::SetLteRlcSapUser (cv2x_LteRlcSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_rlcSapUser = s;
}

cv2x_LteRlcSapProvider*
cv2x_LteRlc::GetLteRlcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_rlcSapProvider;
}

void
cv2x_LteRlc::SetLteMacSapProvider (cv2x_LteMacSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_macSapProvider = s;
}

cv2x_LteMacSapUser*
cv2x_LteRlc::GetLteMacSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_macSapUser;
}



////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteRlcSm);

cv2x_LteRlcSm::cv2x_LteRlcSm ()
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteRlcSm::~cv2x_LteRlcSm ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
cv2x_LteRlcSm::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteRlcSm")
    .SetParent<cv2x_LteRlc> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteRlcSm> ()
    ;
  return tid;
}

void
cv2x_LteRlcSm::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  ReportBufferStatus ();
}

void
cv2x_LteRlcSm::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  cv2x_LteRlc::DoDispose ();
}

void
cv2x_LteRlcSm::DoTransmitPdcpPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
}

void
cv2x_LteRlcSm::DoReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << p);
  // RLC Performance evaluation
  cv2x_RlcTag rlcTag;
  Time delay;
  NS_ASSERT_MSG (p->PeekPacketTag (rlcTag), "cv2x_RlcTag is missing");
  p->RemovePacketTag (rlcTag);
  delay = Simulator::Now() - rlcTag.GetSenderTimestamp ();
  NS_LOG_LOGIC (" RNTI=" << m_rnti 
                << " LCID=" << (uint32_t) m_lcid 
                << " size=" << p->GetSize () 
                << " delay=" << delay.GetNanoSeconds ());
  m_rxPdu(m_rnti, m_lcid, p->GetSize (), delay.GetNanoSeconds () );
}

void
cv2x_LteRlcSm::DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid)
{
  NS_LOG_FUNCTION (this << bytes);
  cv2x_LteMacSapProvider::TransmitPduParameters params;
  params.pdu = Create<Packet> (bytes);
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  params.srcL2Id = m_srcL2Id;
  params.dstL2Id = m_dstL2Id;
  params.layer = layer;
  params.harqProcessId = harqId;
  params.componentCarrierId = componentCarrierId;

  // RLC Performance evaluation
  cv2x_RlcTag tag (Simulator::Now());
  params.pdu->AddPacketTag (tag);
  NS_LOG_LOGIC (" RNTI=" << m_rnti 
                << " LCID=" << (uint32_t) m_lcid 
                << " size=" << bytes);
  m_txPdu(m_rnti, m_lcid, bytes);

  m_macSapProvider->TransmitPdu (params);
  ReportBufferStatus ();
}

void
cv2x_LteRlcSm::DoNotifyHarqDeliveryFailure ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteRlcSm::ReportBufferStatus ()
{
  NS_LOG_FUNCTION (this);
  cv2x_LteMacSapProvider::ReportBufferStatusParameters p;
  p.rnti = m_rnti;
  p.lcid = m_lcid;
  p.srcL2Id = m_srcL2Id;
  p.dstL2Id = m_dstL2Id;
  p.txQueueSize = 80000;
  p.txQueueHolDelay = 10;
  p.retxQueueSize = 0;
  p.retxQueueHolDelay = 0;
  p.statusPduSize = 0;
  m_macSapProvider->ReportBufferStatus (p);
}




//////////////////////////////////////////

// cv2x_LteRlcTm::~cv2x_LteRlcTm ()
// {
// }

//////////////////////////////////////////

// cv2x_LteRlcUm::~cv2x_LteRlcUm ()
// {
// }

//////////////////////////////////////////

// cv2x_LteRlcAm::~cv2x_LteRlcAm ()
// {
// }


} // namespace ns3

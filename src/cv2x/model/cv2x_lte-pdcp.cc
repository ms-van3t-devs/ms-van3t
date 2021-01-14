/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Modified by: NIST (D2D)
 */

#include "ns3/log.h"
#include "ns3/simulator.h"

#include "ns3/cv2x_lte-pdcp.h"
#include "ns3/cv2x_lte-pdcp-header.h"
#include "ns3/cv2x_lte-pdcp-sap.h"
#include "ns3/cv2x_lte-pdcp-tag.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LtePdcp");

/// cv2x_LtePdcpSpecificLteRlcSapUser class
class cv2x_LtePdcpSpecificLteRlcSapUser : public cv2x_LteRlcSapUser
{
public:
  /**
   * Constructor
   *
   * \param pdcp PDCP
   */
  cv2x_LtePdcpSpecificLteRlcSapUser (cv2x_LtePdcp* pdcp);

  // Interface provided to lower RLC entity (implemented from cv2x_LteRlcSapUser)
  virtual void ReceivePdcpPdu (Ptr<Packet> p);

private:
  cv2x_LtePdcpSpecificLteRlcSapUser ();
  cv2x_LtePdcp* m_pdcp; ///< the PDCP
};

cv2x_LtePdcpSpecificLteRlcSapUser::cv2x_LtePdcpSpecificLteRlcSapUser (cv2x_LtePdcp* pdcp)
  : m_pdcp (pdcp)
{
}

cv2x_LtePdcpSpecificLteRlcSapUser::cv2x_LtePdcpSpecificLteRlcSapUser ()
{
}

void
cv2x_LtePdcpSpecificLteRlcSapUser::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdu (p);
}

///////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (cv2x_LtePdcp);

cv2x_LtePdcp::cv2x_LtePdcp ()
  : m_pdcpSapUser (0),
    m_rlcSapProvider (0),
    m_rnti (0),
    m_lcid (0),
    m_srcL2Id (0),
    m_dstL2Id (0),
    m_txSequenceNumber (0),
    m_rxSequenceNumber (0)
{
  NS_LOG_FUNCTION (this);
  m_pdcpSapProvider = new cv2x_LtePdcpSpecificLtePdcpSapProvider<cv2x_LtePdcp> (this);
  m_rlcSapUser = new cv2x_LtePdcpSpecificLteRlcSapUser (this);
}

cv2x_LtePdcp::~cv2x_LtePdcp ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
cv2x_LtePdcp::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LtePdcp")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddTraceSource ("TxPDU",
                     "PDU transmission notified to the RLC.",
                     MakeTraceSourceAccessor (&cv2x_LtePdcp::m_txPdu),
                     "ns3::cv2x_LtePdcp::PduTxTracedCallback")
    .AddTraceSource ("RxPDU",
                     "PDU received.",
                     MakeTraceSourceAccessor (&cv2x_LtePdcp::m_rxPdu),
                     "ns3::cv2x_LtePdcp::PduRxTracedCallback")
    ;
  return tid;
}

void
cv2x_LtePdcp::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete (m_pdcpSapProvider);
  delete (m_rlcSapUser);
}


void
cv2x_LtePdcp::SetRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << (uint32_t) rnti);
  m_rnti = rnti;
}

void
cv2x_LtePdcp::SetLcId (uint8_t lcId)
{
  NS_LOG_FUNCTION (this << (uint32_t) lcId);
  m_lcid = lcId;
}

void
cv2x_LtePdcp::SetSourceL2Id (uint32_t src)
{
  NS_LOG_FUNCTION (this << src);
  m_srcL2Id = src;
}

void
cv2x_LtePdcp::SetDestinationL2Id (uint32_t dst)
{
  NS_LOG_FUNCTION (this << dst);
  m_dstL2Id = dst;
}

void
cv2x_LtePdcp::SetLtePdcpSapUser (cv2x_LtePdcpSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_pdcpSapUser = s;
}

cv2x_LtePdcpSapProvider*
cv2x_LtePdcp::GetLtePdcpSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_pdcpSapProvider;
}

void
cv2x_LtePdcp::SetLteRlcSapProvider (cv2x_LteRlcSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_rlcSapProvider = s;
}

cv2x_LteRlcSapUser*
cv2x_LtePdcp::GetLteRlcSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_rlcSapUser;
}

cv2x_LtePdcp::Status 
cv2x_LtePdcp::GetStatus ()
{
  Status s;
  s.txSn = m_txSequenceNumber;
  s.rxSn = m_rxSequenceNumber;
  return s;
}

void 
cv2x_LtePdcp::SetStatus (Status s)
{
  m_txSequenceNumber = s.txSn;
  m_rxSequenceNumber = s.rxSn;
}

////////////////////////////////////////

void
cv2x_LtePdcp::DoTransmitPdcpSdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  cv2x_LtePdcpHeader pdcpHeader;
  pdcpHeader.SetSequenceNumber (m_txSequenceNumber);

  m_txSequenceNumber++;
  if (m_txSequenceNumber > m_maxPdcpSn)
    {
      m_txSequenceNumber = 0;
    }

  pdcpHeader.SetDcBit (cv2x_LtePdcpHeader::DATA_PDU);

  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);
  p->AddHeader (pdcpHeader);

  // Sender timestamp
  cv2x_PdcpTag pdcpTag (Simulator::Now ());
  p->AddPacketTag (pdcpTag);
  m_txPdu (m_rnti, m_lcid, p->GetSize ());

  cv2x_LteRlcSapProvider::TransmitPdcpPduParameters params;
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  params.srcL2Id = m_srcL2Id;
  params.dstL2Id = m_dstL2Id;
  params.pdcpPdu = p;

  m_rlcSapProvider->TransmitPdcpPdu (params);
}

void
cv2x_LtePdcp::DoReceivePdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << m_rnti << (uint32_t) m_lcid << p->GetSize ());

  // Receiver timestamp
  cv2x_PdcpTag pdcpTag;
  Time delay;
  NS_ASSERT_MSG (p->PeekPacketTag (pdcpTag), "cv2x_PdcpTag is missing");
  p->RemovePacketTag (pdcpTag);
  delay = Simulator::Now() - pdcpTag.GetSenderTimestamp ();
  m_rxPdu(m_rnti, m_lcid, p->GetSize (), delay.GetNanoSeconds ());

  cv2x_LtePdcpHeader pdcpHeader;
  p->RemoveHeader (pdcpHeader);
  NS_LOG_LOGIC ("PDCP header: " << pdcpHeader);

  m_rxSequenceNumber = pdcpHeader.GetSequenceNumber () + 1;
  if (m_rxSequenceNumber > m_maxPdcpSn)
    {
      m_rxSequenceNumber = 0;
    }

  cv2x_LtePdcpSapUser::ReceivePdcpSduParameters params;
  params.pdcpSdu = p;
  params.rnti = m_rnti;
  params.lcid = m_lcid;
  params.srcL2Id = m_srcL2Id;
  params.dstL2Id = m_dstL2Id;
  m_pdcpSapUser->ReceivePdcpSdu (params);
}


} // namespace ns3

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "nr-sl-ue-mac-harq.h"
#include <ns3/packet-burst.h>
#include <ns3/packet.h>
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlUeMacHarq");
NS_OBJECT_ENSURE_REGISTERED (NrSlUeMacHarq);

TypeId
NrSlUeMacHarq::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrSlUeMacHarq")
    .SetParent<Object> ()
    .AddConstructor <NrSlUeMacHarq> ()
    .SetGroupName ("nr")
  ;

  return tid;
}

NrSlUeMacHarq::NrSlUeMacHarq ()
{
}

NrSlUeMacHarq::~NrSlUeMacHarq ()
{
}

void
NrSlUeMacHarq::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_nrSlHarqIdBuffer.clear ();
  for (auto it:m_nrSlHarqPktBuffer)
    {
      it.pktBurst = nullptr;
    }
  m_nrSlHarqPktBuffer.clear ();
}

void
NrSlUeMacHarq::InitHarqBuffer (uint8_t maxSlProcesses)
{
  NS_LOG_FUNCTION (this << +maxSlProcesses);

  NS_ASSERT_MSG (m_nrSlHarqIdBuffer.size () == 0 && m_nrSlHarqPktBuffer.size () == 0, "HARQ buffers not empty. Can not initialize.");

  m_nrSlHarqIdBuffer.resize (maxSlProcesses);
  for (uint8_t id = 0; id < maxSlProcesses; id++)
    {
      m_nrSlHarqIdBuffer.at (id) = id;
    }

  m_nrSlHarqPktBuffer.resize (maxSlProcesses);
  for (uint8_t i = 0; i < maxSlProcesses; i++)
    {
      Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
      m_nrSlHarqPktBuffer.at (i).pktBurst = pb;
    }
}

uint8_t
NrSlUeMacHarq::AssignNrSlHarqProcessId (uint32_t dstL2Id)
{
  NS_LOG_FUNCTION (this << dstL2Id);
  NS_ABORT_MSG_IF (GetNumAvaiableHarqIds () == 0, "All the Sidelink processes are busy");
  uint8_t availableHarqId = m_nrSlHarqIdBuffer.front ();
  //remove it. It is the indication that this id is not available anymore
  m_nrSlHarqIdBuffer.pop_front ();
  //set the given destination in m_nrSlHarqPktBuffer at the index equal to
  //availableHarqId so we can check it while adding the packet.
  m_nrSlHarqPktBuffer.at (availableHarqId).dstL2Id = dstL2Id;
  return availableHarqId;
}

uint8_t
NrSlUeMacHarq::GetNumAvaiableHarqIds () const
{
  return m_nrSlHarqIdBuffer.size ();
}

bool
NrSlUeMacHarq::IsHarqIdAvaiable (uint8_t harqId) const
{
  for (const auto & it : m_nrSlHarqIdBuffer)
    {
      if (it == harqId)
        {
          return true;
        }
    }
  return false;
}

void
NrSlUeMacHarq::AddPacket (uint32_t dstL2Id, uint8_t lcId, uint8_t harqId, Ptr<Packet> pkt)
{
  NS_LOG_FUNCTION (this << dstL2Id << +lcId << +harqId);
  NS_ABORT_MSG_IF (m_nrSlHarqPktBuffer.at (harqId).dstL2Id != dstL2Id, "the HARQ id " << +harqId << " does not belongs to the destination " << dstL2Id);
  m_nrSlHarqPktBuffer.at (harqId).lcidList.insert (lcId);
  NS_ASSERT_MSG (m_nrSlHarqPktBuffer.at (harqId).pktBurst != nullptr, " Packet burst not initialized for HARQ id " << +harqId);
  m_nrSlHarqPktBuffer.at (harqId).pktBurst->AddPacket (pkt);
  //Each LC have one MAC PDU in a TB. Packet burst here, imitates a TB, therefore,
  //the number of LCs inside lcidList and the packets inside the packet burst
  //must be equal.
  NS_ABORT_MSG_IF (m_nrSlHarqPktBuffer.at (harqId).lcidList.size () != m_nrSlHarqPktBuffer.at (harqId).pktBurst->GetNPackets (),
                   "Mismatch in number of LCIDs and the number of packets for SL HARQ ID " << +harqId << " dest " << dstL2Id);
}

void
NrSlUeMacHarq::RecvNrSlHarqFeedback (uint32_t dstL2Id, uint8_t harqId)
{
  NS_LOG_FUNCTION (this << dstL2Id << +harqId);
  NS_ABORT_MSG_IF (m_nrSlHarqPktBuffer.at (harqId).dstL2Id != dstL2Id, "the HARQ id " << +harqId << " does not belongs to the destination " << dstL2Id);
  //we expect the given HARQ to be not available before inserting back in m_nrSlHarqIdBuffer
  NS_ASSERT_MSG (IsHarqIdAvaiable (harqId) == false, "Can not receive a feedback for a already available HARQ id " << harqId);
  //Put back the HARQ id so to be assigned again
  m_nrSlHarqIdBuffer.push_back (harqId);
  //Refresh HARQ packet buffer
  Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
  m_nrSlHarqPktBuffer.at (harqId).pktBurst = pb;
  m_nrSlHarqPktBuffer.at (harqId).lcidList.clear ();
  m_nrSlHarqPktBuffer.at (harqId).dstL2Id = std::numeric_limits <uint32_t>::max ();
}

Ptr<PacketBurst>
NrSlUeMacHarq::GetPacketBurst (uint32_t dstL2Id, uint8_t harqId) const
{
  NS_LOG_FUNCTION (this << dstL2Id << +harqId);
  NS_ABORT_MSG_IF (m_nrSlHarqPktBuffer.at (harqId).dstL2Id != dstL2Id, "the HARQ id " << +harqId << " does not belongs to the destination " << dstL2Id);
  Ptr<PacketBurst> pb = m_nrSlHarqPktBuffer.at (harqId).pktBurst;
  return pb;
}



} // namespace ns3



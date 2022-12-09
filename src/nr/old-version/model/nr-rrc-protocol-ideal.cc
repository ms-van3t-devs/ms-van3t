/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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


#include "nr-rrc-protocol-ideal.h"

#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/nstime.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/simulator.h>

#include "ns3/lte-ue-rrc.h"
#include "ns3/lte-enb-rrc.h"
#include "nr-ue-net-device.h"
#include "nr-gnb-net-device.h"

NS_LOG_COMPONENT_DEFINE ("nrRrcProtocolIdeal");


namespace ns3 {


static const Time RRC_IDEAL_MSG_DELAY = MilliSeconds (0);

NS_OBJECT_ENSURE_REGISTERED (nrUeRrcProtocolIdeal);

nrUeRrcProtocolIdeal::nrUeRrcProtocolIdeal ()
  :  m_ueRrcSapProvider (0),
  m_enbRrcSapProvider (0)
{
  m_ueRrcSapUser = new MemberLteUeRrcSapUser<nrUeRrcProtocolIdeal> (this);
}

nrUeRrcProtocolIdeal::~nrUeRrcProtocolIdeal ()
{
}

void
nrUeRrcProtocolIdeal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ueRrcSapUser;
  m_rrc = 0;
}

TypeId
nrUeRrcProtocolIdeal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::nrUeRrcProtocolIdeal")
    .SetParent<Object> ()
    .AddConstructor<nrUeRrcProtocolIdeal> ()
  ;
  return tid;
}

void
nrUeRrcProtocolIdeal::SetLteUeRrcSapProvider (LteUeRrcSapProvider* p)
{
  m_ueRrcSapProvider = p;
}

LteUeRrcSapUser*
nrUeRrcProtocolIdeal::GetLteUeRrcSapUser ()
{
  return m_ueRrcSapUser;
}

void
nrUeRrcProtocolIdeal::SetUeRrc (Ptr<LteUeRrc> rrc)
{
  m_rrc = rrc;
}

void
nrUeRrcProtocolIdeal::DoSetup (LteUeRrcSapUser::SetupParameters params)
{
  NS_LOG_FUNCTION (this);
  // We don't care about SRB0/SRB1 since we use ideal RRC messages.
}

void
nrUeRrcProtocolIdeal::DoSendRrcConnectionRequest (LteRrcSap::RrcConnectionRequest msg)
{
  // initialize the RNTI and get the EnbLteRrcSapProvider for the
  // gNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteEnbRrcSapProvider::RecvRrcConnectionRequest,
                       m_enbRrcSapProvider,
                       m_rnti,
                       msg);
}

void
nrUeRrcProtocolIdeal::DoSendRrcConnectionSetupCompleted (LteRrcSap::RrcConnectionSetupCompleted msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteEnbRrcSapProvider::RecvRrcConnectionSetupCompleted,
                       m_enbRrcSapProvider,
                       m_rnti,
                       msg);
}

void
nrUeRrcProtocolIdeal::DoSendRrcConnectionReconfigurationCompleted (LteRrcSap::RrcConnectionReconfigurationCompleted msg)
{
  // re-initialize the RNTI and get the EnbLteRrcSapProvider for the
  // gNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();

  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteEnbRrcSapProvider::RecvRrcConnectionReconfigurationCompleted,
                       m_enbRrcSapProvider,
                       m_rnti,
                       msg);
}

void
nrUeRrcProtocolIdeal::DoSendRrcConnectionReestablishmentRequest (LteRrcSap::RrcConnectionReestablishmentRequest msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteEnbRrcSapProvider::RecvRrcConnectionReestablishmentRequest,
                       m_enbRrcSapProvider,
                       m_rnti,
                       msg);
}

void
nrUeRrcProtocolIdeal::DoSendRrcConnectionReestablishmentComplete (LteRrcSap::RrcConnectionReestablishmentComplete msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteEnbRrcSapProvider::RecvRrcConnectionReestablishmentComplete,
                       m_enbRrcSapProvider,
                       m_rnti,
                       msg);
}

void
nrUeRrcProtocolIdeal::DoSendMeasurementReport (LteRrcSap::MeasurementReport msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteEnbRrcSapProvider::RecvMeasurementReport,
                       m_enbRrcSapProvider,
                       m_rnti,
                       msg);
}

void
nrUeRrcProtocolIdeal::DoSendIdealUeContextRemoveRequest (uint16_t rnti)
{
  NS_FATAL_ERROR ("nrUeRrcProtocolIdeal does not have RLF functionality yet");
}

void
nrUeRrcProtocolIdeal::SetEnbRrcSapProvider ()
{
  uint16_t bwpId = m_rrc->GetCellId ();

  // walk list of all nodes to get the peer gNB
  Ptr<NrGnbNetDevice> gnbDev;
  NodeList::Iterator listEnd = NodeList::End ();
  bool found = false;
  for (NodeList::Iterator i = NodeList::Begin ();
       (i != listEnd) && (!found);
       ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; (j < nDevs) && (!found); j++)
        {
          gnbDev = node->GetDevice (j)->GetObject <NrGnbNetDevice> ();
          if (gnbDev != nullptr)
            {
              for (uint32_t h = 0; h < gnbDev->GetCcMapSize (); ++h)
                {
                  if (gnbDev->GetBwpId (h) == bwpId)
                    {
                      found = true;
                      break;
                    }
                }
            }
        }
    }
  NS_ASSERT_MSG (found, " Unable to find gNB with BwpID =" << bwpId);
  m_enbRrcSapProvider = gnbDev->GetRrc ()->GetLteEnbRrcSapProvider ();
  Ptr<NrGnbRrcProtocolIdeal> enbRrcProtocolIdeal = gnbDev->GetRrc ()->GetObject<NrGnbRrcProtocolIdeal> ();
  enbRrcProtocolIdeal->SetUeRrcSapProvider (m_rnti, m_ueRrcSapProvider);
}


NS_OBJECT_ENSURE_REGISTERED (NrGnbRrcProtocolIdeal);

NrGnbRrcProtocolIdeal::NrGnbRrcProtocolIdeal ()
  :  m_enbRrcSapProvider (0)
{
  NS_LOG_FUNCTION (this);
  m_enbRrcSapUser = new MemberLteEnbRrcSapUser<NrGnbRrcProtocolIdeal> (this);
}

NrGnbRrcProtocolIdeal::~NrGnbRrcProtocolIdeal ()
{
  NS_LOG_FUNCTION (this);
}

void
NrGnbRrcProtocolIdeal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_enbRrcSapUser;
}

TypeId
NrGnbRrcProtocolIdeal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrGnbRrcProtocolIdeal")
    .SetParent<Object> ()
    .AddConstructor<NrGnbRrcProtocolIdeal> ()
  ;
  return tid;
}

void
NrGnbRrcProtocolIdeal::SetLteEnbRrcSapProvider (LteEnbRrcSapProvider* p)
{
  m_enbRrcSapProvider = p;
}

LteEnbRrcSapUser*
NrGnbRrcProtocolIdeal::GetLteEnbRrcSapUser ()
{
  return m_enbRrcSapUser;
}

LteUeRrcSapProvider*
NrGnbRrcProtocolIdeal::GetUeRrcSapProvider (uint16_t rnti)
{
  std::map<uint16_t, LteUeRrcSapProvider*>::const_iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  return it->second;
}

void
NrGnbRrcProtocolIdeal::SetUeRrcSapProvider (uint16_t rnti, LteUeRrcSapProvider* p)
{
  std::map<uint16_t, LteUeRrcSapProvider*>::iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  it->second = p;
}

void
NrGnbRrcProtocolIdeal::DoSetupUe (uint16_t rnti, LteEnbRrcSapUser::SetupUeParameters params)
{
  NS_LOG_FUNCTION (this << rnti);
  m_enbRrcSapProviderMap[rnti] = 0;

}

void
NrGnbRrcProtocolIdeal::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  m_enbRrcSapProviderMap.erase (rnti);
}

void
NrGnbRrcProtocolIdeal::DoSendSystemInformation (uint16_t cellId, LteRrcSap::SystemInformation msg)
{
  NS_LOG_FUNCTION (this << cellId);
  // walk list of all nodes to get UEs with this cellId
  Ptr<LteUeRrc> ueRrc;
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
    {
      Ptr<Node> node = *i;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; ++j)
        {
          Ptr<NrUeNetDevice> nrUeDev = node->GetDevice (j)->GetObject <NrUeNetDevice> ();
          if (nrUeDev != 0)
            {
              Ptr<LteUeRrc> ueRrc = nrUeDev->GetRrc ();
              NS_LOG_LOGIC ("considering UE IMSI " << nrUeDev->GetImsi () << " that has cellId " << ueRrc->GetCellId ());
              if (ueRrc->GetCellId () == cellId)
                {
                  NS_LOG_LOGIC ("sending SI to IMSI " << nrUeDev->GetImsi ());
                  ueRrc->GetLteUeRrcSapProvider ()->RecvSystemInformation (msg);
                  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                                       &LteUeRrcSapProvider::RecvSystemInformation,
                                       ueRrc->GetLteUeRrcSapProvider (),
                                       msg);
                }
            }
        }
    }
}

void
NrGnbRrcProtocolIdeal::DoSendRrcConnectionSetup (uint16_t rnti, LteRrcSap::RrcConnectionSetup msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteUeRrcSapProvider::RecvRrcConnectionSetup,
                       GetUeRrcSapProvider (rnti),
                       msg);
}

void
NrGnbRrcProtocolIdeal::DoSendRrcConnectionReconfiguration (uint16_t rnti, LteRrcSap::RrcConnectionReconfiguration msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteUeRrcSapProvider::RecvRrcConnectionReconfiguration,
                       GetUeRrcSapProvider (rnti),
                       msg);
}

void
NrGnbRrcProtocolIdeal::DoSendRrcConnectionReestablishment (uint16_t rnti, LteRrcSap::RrcConnectionReestablishment msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteUeRrcSapProvider::RecvRrcConnectionReestablishment,
                       GetUeRrcSapProvider (rnti),
                       msg);
}

void
NrGnbRrcProtocolIdeal::DoSendRrcConnectionReestablishmentReject (uint16_t rnti, LteRrcSap::RrcConnectionReestablishmentReject msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteUeRrcSapProvider::RecvRrcConnectionReestablishmentReject,
                       GetUeRrcSapProvider (rnti),
                       msg);
}

void
NrGnbRrcProtocolIdeal::DoSendRrcConnectionRelease (uint16_t rnti, LteRrcSap::RrcConnectionRelease msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteUeRrcSapProvider::RecvRrcConnectionRelease,
                       GetUeRrcSapProvider (rnti),
                       msg);
}

void
NrGnbRrcProtocolIdeal::DoSendRrcConnectionReject (uint16_t rnti, LteRrcSap::RrcConnectionReject msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY,
                       &LteUeRrcSapProvider::RecvRrcConnectionReject,
                       GetUeRrcSapProvider (rnti),
                       msg);
}

/*
 * The purpose of NrGnbRrcProtocolIdeal is to avoid encoding
 * messages. In order to do so, we need to have some form of encoding for
 * inter-node RRC messages like HandoverPreparationInfo and HandoverCommand. Doing so
 * directly is not practical (these messages includes a lot of
 * information elements, so encoding all of them would defeat the
 * purpose of NrGnbRrcProtocolIdeal. The workaround is to store the
 * actual message in a global map, so that then we can just encode the
 * key in a header and send that between gNBs over X2.
 *
 */

static std::map<uint32_t, LteRrcSap::HandoverPreparationInfo> g_handoverPreparationInfoMsgMap;
static uint32_t g_handoverPreparationInfoMsgIdCounter = 0;

/*
 * This header encodes the map key discussed above. We keep this
 * private since it should not be used outside this file.
 *
 */
class NrIdealHandoverPreparationInfoHeader : public Header
{
public:
  uint32_t GetMsgId ();
  void SetMsgId (uint32_t id);
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_msgId;
};

uint32_t
NrIdealHandoverPreparationInfoHeader::GetMsgId ()
{
  return m_msgId;
}

void
NrIdealHandoverPreparationInfoHeader::SetMsgId (uint32_t id)
{
  m_msgId = id;
}


TypeId
NrIdealHandoverPreparationInfoHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrIdealHandoverPreparationInfoHeader")
    .SetParent<Header> ()
    .AddConstructor<NrIdealHandoverPreparationInfoHeader> ()
  ;
  return tid;
}

TypeId
NrIdealHandoverPreparationInfoHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void NrIdealHandoverPreparationInfoHeader::Print (std::ostream &os)  const
{
  os << " msgId=" << m_msgId;
}

uint32_t NrIdealHandoverPreparationInfoHeader::GetSerializedSize (void) const
{
  return 4;
}

void NrIdealHandoverPreparationInfoHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU32 (m_msgId);
}

uint32_t NrIdealHandoverPreparationInfoHeader::Deserialize (Buffer::Iterator start)
{
  m_msgId = start.ReadU32 ();
  return GetSerializedSize ();
}



Ptr<Packet>
NrGnbRrcProtocolIdeal::DoEncodeHandoverPreparationInformation (LteRrcSap::HandoverPreparationInfo msg)
{
  uint32_t msgId = ++g_handoverPreparationInfoMsgIdCounter;
  NS_ASSERT_MSG (g_handoverPreparationInfoMsgMap.find (msgId) == g_handoverPreparationInfoMsgMap.end (), "msgId " << msgId << " already in use");
  NS_LOG_INFO (" encoding msgId = " << msgId);
  g_handoverPreparationInfoMsgMap.insert (std::pair<uint32_t, LteRrcSap::HandoverPreparationInfo> (msgId, msg));
  NrIdealHandoverPreparationInfoHeader h;
  h.SetMsgId (msgId);
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

LteRrcSap::HandoverPreparationInfo
NrGnbRrcProtocolIdeal::DoDecodeHandoverPreparationInformation (Ptr<Packet> p)
{
  NrIdealHandoverPreparationInfoHeader h;
  p->RemoveHeader (h);
  uint32_t msgId = h.GetMsgId ();
  NS_LOG_INFO (" decoding msgId = " << msgId);
  std::map<uint32_t, LteRrcSap::HandoverPreparationInfo>::iterator it = g_handoverPreparationInfoMsgMap.find (msgId);
  NS_ASSERT_MSG (it != g_handoverPreparationInfoMsgMap.end (), "msgId " << msgId << " not found");
  LteRrcSap::HandoverPreparationInfo msg = it->second;
  g_handoverPreparationInfoMsgMap.erase (it);
  return msg;
}



static std::map<uint32_t, LteRrcSap::RrcConnectionReconfiguration> g_handoverCommandMsgMap;
static uint32_t g_handoverCommandMsgIdCounter = 0;

/*
 * This header encodes the map key discussed above. We keep this
 * private since it should not be used outside this file.
 *
 */
class NrIdealHandoverCommandHeader : public Header
{
public:
  uint32_t GetMsgId ();
  void SetMsgId (uint32_t id);
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_msgId;
};

uint32_t
NrIdealHandoverCommandHeader::GetMsgId ()
{
  return m_msgId;
}

void
NrIdealHandoverCommandHeader::SetMsgId (uint32_t id)
{
  m_msgId = id;
}


TypeId
NrIdealHandoverCommandHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrIdealHandoverCommandHeader")
    .SetParent<Header> ()
    .AddConstructor<NrIdealHandoverCommandHeader> ()
  ;
  return tid;
}

TypeId
NrIdealHandoverCommandHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void NrIdealHandoverCommandHeader::Print (std::ostream &os)  const
{
  os << " msgId=" << m_msgId;
}

uint32_t NrIdealHandoverCommandHeader::GetSerializedSize (void) const
{
  return 4;
}

void NrIdealHandoverCommandHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteU32 (m_msgId);
}

uint32_t NrIdealHandoverCommandHeader::Deserialize (Buffer::Iterator start)
{
  m_msgId = start.ReadU32 ();
  return GetSerializedSize ();
}



Ptr<Packet>
NrGnbRrcProtocolIdeal::DoEncodeHandoverCommand (LteRrcSap::RrcConnectionReconfiguration msg)
{
  uint32_t msgId = ++g_handoverCommandMsgIdCounter;
  NS_ASSERT_MSG (g_handoverCommandMsgMap.find (msgId) == g_handoverCommandMsgMap.end (), "msgId " << msgId << " already in use");
  NS_LOG_INFO (" encoding msgId = " << msgId);
  g_handoverCommandMsgMap.insert (std::pair<uint32_t, LteRrcSap::RrcConnectionReconfiguration> (msgId, msg));
  NrIdealHandoverCommandHeader h;
  h.SetMsgId (msgId);
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

LteRrcSap::RrcConnectionReconfiguration
NrGnbRrcProtocolIdeal::DoDecodeHandoverCommand (Ptr<Packet> p)
{
  NrIdealHandoverCommandHeader h;
  p->RemoveHeader (h);
  uint32_t msgId = h.GetMsgId ();
  NS_LOG_INFO (" decoding msgId = " << msgId);
  std::map<uint32_t, LteRrcSap::RrcConnectionReconfiguration>::iterator it = g_handoverCommandMsgMap.find (msgId);
  NS_ASSERT_MSG (it != g_handoverCommandMsgMap.end (), "msgId " << msgId << " not found");
  LteRrcSap::RrcConnectionReconfiguration msg = it->second;
  g_handoverCommandMsgMap.erase (it);
  return msg;
}





} // namespace ns3

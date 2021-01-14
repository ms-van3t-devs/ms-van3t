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
 * Modified by: NIST (D2D)
 */

#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/nstime.h>
#include <ns3/node-list.h>
#include <ns3/node.h>
#include <ns3/simulator.h>

#include "cv2x_lte-rrc-protocol-ideal.h"
#include "cv2x_lte-ue-rrc.h"
#include "cv2x_lte-enb-rrc.h"
#include "cv2x_lte-enb-net-device.h"
#include "cv2x_lte-ue-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteRrcProtocolIdeal");

/**
 * \ingroup lte
 *
 */

/// RRC ideal message delay
static const Time RRC_IDEAL_MSG_DELAY = MilliSeconds (0);

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteUeRrcProtocolIdeal);

cv2x_LteUeRrcProtocolIdeal::cv2x_LteUeRrcProtocolIdeal ()
  :  m_ueRrcSapProvider (0),
     m_enbRrcSapProvider (0)
{
  m_ueRrcSapUser = new cv2x_MemberLteUeRrcSapUser<cv2x_LteUeRrcProtocolIdeal> (this);
}

cv2x_LteUeRrcProtocolIdeal::~cv2x_LteUeRrcProtocolIdeal ()
{
}

void
cv2x_LteUeRrcProtocolIdeal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ueRrcSapUser;
  m_rrc = 0;
}

TypeId
cv2x_LteUeRrcProtocolIdeal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteUeRrcProtocolIdeal")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteUeRrcProtocolIdeal> ()
    ;
  return tid;
}

void 
cv2x_LteUeRrcProtocolIdeal::SetLteUeRrcSapProvider (cv2x_LteUeRrcSapProvider* p)
{
  m_ueRrcSapProvider = p;
}

cv2x_LteUeRrcSapUser* 
cv2x_LteUeRrcProtocolIdeal::GetLteUeRrcSapUser ()
{
  return m_ueRrcSapUser;
}

void 
cv2x_LteUeRrcProtocolIdeal::SetUeRrc (Ptr<cv2x_LteUeRrc> rrc)
{
  m_rrc = rrc;
}

void 
cv2x_LteUeRrcProtocolIdeal::DoSetup (cv2x_LteUeRrcSapUser::SetupParameters params)
{
  NS_LOG_FUNCTION (this);
  // We don't care about SRB0/SRB1 since we use ideal RRC messages.
}

void 
cv2x_LteUeRrcProtocolIdeal::DoSendRrcConnectionRequest (cv2x_LteRrcSap::RrcConnectionRequest msg)
{
  // initialize the RNTI and get the EnbLteRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();
    
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
                       &cv2x_LteEnbRrcSapProvider::RecvRrcConnectionRequest,
                       m_enbRrcSapProvider,
                       m_rnti, 
                       msg);
}

void 
cv2x_LteUeRrcProtocolIdeal::DoSendRrcConnectionSetupCompleted (cv2x_LteRrcSap::RrcConnectionSetupCompleted msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &cv2x_LteEnbRrcSapProvider::RecvRrcConnectionSetupCompleted,
                       m_enbRrcSapProvider,
		       m_rnti, 
		       msg);
}

void 
cv2x_LteUeRrcProtocolIdeal::DoSendRrcConnectionReconfigurationCompleted (cv2x_LteRrcSap::RrcConnectionReconfigurationCompleted msg)
{
  // re-initialize the RNTI and get the EnbLteRrcSapProvider for the
  // eNB we are currently attached to
  m_rnti = m_rrc->GetRnti ();
  SetEnbRrcSapProvider ();
    
   Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
                        &cv2x_LteEnbRrcSapProvider::RecvRrcConnectionReconfigurationCompleted,
                        m_enbRrcSapProvider,
                        m_rnti, 
                        msg);
}

void 
cv2x_LteUeRrcProtocolIdeal::DoSendRrcConnectionReestablishmentRequest (cv2x_LteRrcSap::RrcConnectionReestablishmentRequest msg)
{
   Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &cv2x_LteEnbRrcSapProvider::RecvRrcConnectionReestablishmentRequest,
                       m_enbRrcSapProvider,
		       m_rnti, 
                        msg);
}

void 
cv2x_LteUeRrcProtocolIdeal::DoSendRrcConnectionReestablishmentComplete (cv2x_LteRrcSap::RrcConnectionReestablishmentComplete msg)
{
   Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &cv2x_LteEnbRrcSapProvider::RecvRrcConnectionReestablishmentComplete,
                       m_enbRrcSapProvider,
		       m_rnti, 
msg);
}

void 
cv2x_LteUeRrcProtocolIdeal::DoSendMeasurementReport (cv2x_LteRrcSap::MeasurementReport msg)
{
   Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
                        &cv2x_LteEnbRrcSapProvider::RecvMeasurementReport,
                        m_enbRrcSapProvider,
                        m_rnti, 
                        msg);
}

void
cv2x_LteUeRrcProtocolIdeal::DoSendSidelinkUeInformation (cv2x_LteRrcSap::SidelinkUeInformation msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
                        &cv2x_LteEnbRrcSapProvider::RecvSidelinkUeInformation,
                        m_enbRrcSapProvider,
                        m_rnti, 
                        msg);
}

void 
cv2x_LteUeRrcProtocolIdeal::SetEnbRrcSapProvider ()
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
  Ptr<cv2x_LteEnbRrcProtocolIdeal> enbRrcProtocolIdeal = enbDev->GetRrc ()->GetObject<cv2x_LteEnbRrcProtocolIdeal> ();
  enbRrcProtocolIdeal->SetUeRrcSapProvider (m_rnti, m_ueRrcSapProvider);
}


NS_OBJECT_ENSURE_REGISTERED (cv2x_LteEnbRrcProtocolIdeal);

cv2x_LteEnbRrcProtocolIdeal::cv2x_LteEnbRrcProtocolIdeal ()
  :  m_enbRrcSapProvider (0)
{
  NS_LOG_FUNCTION (this);
  m_enbRrcSapUser = new cv2x_MemberLteEnbRrcSapUser<cv2x_LteEnbRrcProtocolIdeal> (this);
}

cv2x_LteEnbRrcProtocolIdeal::~cv2x_LteEnbRrcProtocolIdeal ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteEnbRrcProtocolIdeal::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_enbRrcSapUser;  
}

TypeId
cv2x_LteEnbRrcProtocolIdeal::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteEnbRrcProtocolIdeal")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteEnbRrcProtocolIdeal> ()
    ;
  return tid;
}

void 
cv2x_LteEnbRrcProtocolIdeal::SetLteEnbRrcSapProvider (cv2x_LteEnbRrcSapProvider* p)
{
  m_enbRrcSapProvider = p;
}

cv2x_LteEnbRrcSapUser* 
cv2x_LteEnbRrcProtocolIdeal::GetLteEnbRrcSapUser ()
{
  return m_enbRrcSapUser;
}

void 
cv2x_LteEnbRrcProtocolIdeal::SetCellId (uint16_t cellId)
{
  m_cellId = cellId;
}

cv2x_LteUeRrcSapProvider* 
cv2x_LteEnbRrcProtocolIdeal::GetUeRrcSapProvider (uint16_t rnti)
{
  std::map<uint16_t, cv2x_LteUeRrcSapProvider*>::const_iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  return it->second;
}

void 
cv2x_LteEnbRrcProtocolIdeal::SetUeRrcSapProvider (uint16_t rnti, cv2x_LteUeRrcSapProvider* p)
{
  std::map<uint16_t, cv2x_LteUeRrcSapProvider*>::iterator it;
  it = m_enbRrcSapProviderMap.find (rnti);
  NS_ASSERT_MSG (it != m_enbRrcSapProviderMap.end (), "could not find RNTI = " << rnti);
  it->second = p;
}

void 
cv2x_LteEnbRrcProtocolIdeal::DoSetupUe (uint16_t rnti, cv2x_LteEnbRrcSapUser::SetupUeParameters params)
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
  //       	  found = true;
  //       	  break;
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

}

void 
cv2x_LteEnbRrcProtocolIdeal::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  m_enbRrcSapProviderMap.erase (rnti);
}

void 
cv2x_LteEnbRrcProtocolIdeal::DoSendSystemInformation (uint16_t cellId, cv2x_LteRrcSap::SystemInformation msg)
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
                  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
                                       &cv2x_LteUeRrcSapProvider::RecvSystemInformation,
                                       ueRrc->GetLteUeRrcSapProvider (), 
                                       msg);          
                }             
            }
        }
    } 
}

void 
cv2x_LteEnbRrcProtocolIdeal::DoSendRrcConnectionSetup (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionSetup msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &cv2x_LteUeRrcSapProvider::RecvRrcConnectionSetup,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
cv2x_LteEnbRrcProtocolIdeal::DoSendRrcConnectionReconfiguration (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReconfiguration msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &cv2x_LteUeRrcSapProvider::RecvRrcConnectionReconfiguration,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
cv2x_LteEnbRrcProtocolIdeal::DoSendRrcConnectionReestablishment (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReestablishment msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &cv2x_LteUeRrcSapProvider::RecvRrcConnectionReestablishment,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
cv2x_LteEnbRrcProtocolIdeal::DoSendRrcConnectionReestablishmentReject (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReestablishmentReject msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &cv2x_LteUeRrcSapProvider::RecvRrcConnectionReestablishmentReject,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
cv2x_LteEnbRrcProtocolIdeal::DoSendRrcConnectionRelease (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionRelease msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &cv2x_LteUeRrcSapProvider::RecvRrcConnectionRelease,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

void 
cv2x_LteEnbRrcProtocolIdeal::DoSendRrcConnectionReject (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReject msg)
{
  Simulator::Schedule (RRC_IDEAL_MSG_DELAY, 
		       &cv2x_LteUeRrcSapProvider::RecvRrcConnectionReject,
		       GetUeRrcSapProvider (rnti), 
		       msg);
}

/*
 * The purpose of cv2x_LteEnbRrcProtocolIdeal is to avoid encoding
 * messages. In order to do so, we need to have some form of encoding for
 * inter-node RRC messages like HandoverPreparationInfo and HandoverCommand. Doing so
 * directly is not practical (these messages includes a lot of
 * information elements, so encoding all of them would defeat the
 * purpose of cv2x_LteEnbRrcProtocolIdeal. The workaround is to store the
 * actual message in a global map, so that then we can just encode the
 * key in a header and send that between eNBs over X2.
 * 
 */

static std::map<uint32_t, cv2x_LteRrcSap::HandoverPreparationInfo> g_handoverPreparationInfoMsgMap; ///< handover preparation info message map
static uint32_t g_handoverPreparationInfoMsgIdCounter = 0; ///< handover preparation info message ID counter

/**
 * This header encodes the map key discussed above. We keep this
 * private since it should not be used outside this file.
 * 
 */
class cv2x_IdealHandoverPreparationInfoHeader : public Header
{
public:
  /**
   * Get the message ID function
   *
   * \returns the message ID
   */
  uint32_t GetMsgId ();
  /**
   * Set the message ID function
   *
   * \param id the message ID 
   */
  void SetMsgId (uint32_t id);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_msgId; ///< message ID
};

uint32_t 
cv2x_IdealHandoverPreparationInfoHeader::GetMsgId ()
{
  return m_msgId;
}  

void 
cv2x_IdealHandoverPreparationInfoHeader::SetMsgId (uint32_t id)
{
  m_msgId = id;
}  


TypeId
cv2x_IdealHandoverPreparationInfoHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_IdealHandoverPreparationInfoHeader")
    .SetParent<Header> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_IdealHandoverPreparationInfoHeader> ()
  ;
  return tid;
}

TypeId
cv2x_IdealHandoverPreparationInfoHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void cv2x_IdealHandoverPreparationInfoHeader::Print (std::ostream &os)  const
{
  os << " msgId=" << m_msgId;
}

uint32_t cv2x_IdealHandoverPreparationInfoHeader::GetSerializedSize (void) const
{
  return 4;
}

void cv2x_IdealHandoverPreparationInfoHeader::Serialize (Buffer::Iterator start) const
{  
  start.WriteU32 (m_msgId);
}

uint32_t cv2x_IdealHandoverPreparationInfoHeader::Deserialize (Buffer::Iterator start)
{
  m_msgId = start.ReadU32 ();
  return GetSerializedSize ();
}



Ptr<Packet> 
cv2x_LteEnbRrcProtocolIdeal::DoEncodeHandoverPreparationInformation (cv2x_LteRrcSap::HandoverPreparationInfo msg)
{
  uint32_t msgId = ++g_handoverPreparationInfoMsgIdCounter;
  NS_ASSERT_MSG (g_handoverPreparationInfoMsgMap.find (msgId) == g_handoverPreparationInfoMsgMap.end (), "msgId " << msgId << " already in use");
  NS_LOG_INFO (" encoding msgId = " << msgId);
  g_handoverPreparationInfoMsgMap.insert (std::pair<uint32_t, cv2x_LteRrcSap::HandoverPreparationInfo> (msgId, msg));
  cv2x_IdealHandoverPreparationInfoHeader h;
  h.SetMsgId (msgId);
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

cv2x_LteRrcSap::HandoverPreparationInfo 
cv2x_LteEnbRrcProtocolIdeal::DoDecodeHandoverPreparationInformation (Ptr<Packet> p)
{
  cv2x_IdealHandoverPreparationInfoHeader h;
  p->RemoveHeader (h);
  uint32_t msgId = h.GetMsgId ();
  NS_LOG_INFO (" decoding msgId = " << msgId);
  std::map<uint32_t, cv2x_LteRrcSap::HandoverPreparationInfo>::iterator it = g_handoverPreparationInfoMsgMap.find (msgId);
  NS_ASSERT_MSG (it != g_handoverPreparationInfoMsgMap.end (), "msgId " << msgId << " not found");
  cv2x_LteRrcSap::HandoverPreparationInfo msg = it->second;
  g_handoverPreparationInfoMsgMap.erase (it);
  return msg;
}



static std::map<uint32_t, cv2x_LteRrcSap::RrcConnectionReconfiguration> g_handoverCommandMsgMap; ///< handover command message map
static uint32_t g_handoverCommandMsgIdCounter = 0; ///< handover command message ID counter

/**
 * This header encodes the map key discussed above. We keep this
 * private since it should not be used outside this file.
 * 
 */
class cv2x_IdealHandoverCommandHeader : public Header
{
public:
  /**
   * Get the message ID function
   *
   * \returns the message ID
   */
  uint32_t GetMsgId ();
  /**
   * Set the message ID function
   *
   * \param id the message ID
   */
  void SetMsgId (uint32_t id);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_msgId; ///< message ID
};

uint32_t 
cv2x_IdealHandoverCommandHeader::GetMsgId ()
{
  return m_msgId;
}  

void 
cv2x_IdealHandoverCommandHeader::SetMsgId (uint32_t id)
{
  m_msgId = id;
}  


TypeId
cv2x_IdealHandoverCommandHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_IdealHandoverCommandHeader")
    .SetParent<Header> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_IdealHandoverCommandHeader> ()
  ;
  return tid;
}

TypeId
cv2x_IdealHandoverCommandHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void cv2x_IdealHandoverCommandHeader::Print (std::ostream &os)  const
{
  os << " msgId=" << m_msgId;
}

uint32_t cv2x_IdealHandoverCommandHeader::GetSerializedSize (void) const
{
  return 4;
}

void cv2x_IdealHandoverCommandHeader::Serialize (Buffer::Iterator start) const
{  
  start.WriteU32 (m_msgId);
}

uint32_t cv2x_IdealHandoverCommandHeader::Deserialize (Buffer::Iterator start)
{
  m_msgId = start.ReadU32 ();
  return GetSerializedSize ();
}



Ptr<Packet> 
cv2x_LteEnbRrcProtocolIdeal::DoEncodeHandoverCommand (cv2x_LteRrcSap::RrcConnectionReconfiguration msg)
{
  uint32_t msgId = ++g_handoverCommandMsgIdCounter;
  NS_ASSERT_MSG (g_handoverCommandMsgMap.find (msgId) == g_handoverCommandMsgMap.end (), "msgId " << msgId << " already in use");
  NS_LOG_INFO (" encoding msgId = " << msgId);
  g_handoverCommandMsgMap.insert (std::pair<uint32_t, cv2x_LteRrcSap::RrcConnectionReconfiguration> (msgId, msg));
  cv2x_IdealHandoverCommandHeader h;
  h.SetMsgId (msgId);
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (h);
  return p;
}

cv2x_LteRrcSap::RrcConnectionReconfiguration
cv2x_LteEnbRrcProtocolIdeal::DoDecodeHandoverCommand (Ptr<Packet> p)
{
  cv2x_IdealHandoverCommandHeader h;
  p->RemoveHeader (h);
  uint32_t msgId = h.GetMsgId ();
  NS_LOG_INFO (" decoding msgId = " << msgId);
  std::map<uint32_t, cv2x_LteRrcSap::RrcConnectionReconfiguration>::iterator it = g_handoverCommandMsgMap.find (msgId);
  NS_ASSERT_MSG (it != g_handoverCommandMsgMap.end (), "msgId " << msgId << " not found");
  cv2x_LteRrcSap::RrcConnectionReconfiguration msg = it->second;
  g_handoverCommandMsgMap.erase (it);
  return msg;
}





} // namespace ns3

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 *         Nicola Baldo <nbaldo@cttc.es>
 *         Marco Miozzo <mmiozzo@cttc.es>
 * Modified by:
 *          Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 *          Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)
 */

#include "ns3/llc-snap-header.h"
#include "ns3/simulator.h"
#include "ns3/callback.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "cv2x_lte-net-device.h"
#include "ns3/packet-burst.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "ns3/cv2x_lte-enb-net-device.h"
#include "cv2x_lte-ue-net-device.h"
#include "cv2x_lte-ue-mac.h"
#include "cv2x_lte-ue-rrc.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv6-header.h"
#include "ns3/ipv4.h"
#include "ns3/ipv6.h"
#include "cv2x_lte-amc.h"
#include "cv2x_lte-ue-phy.h"
#include "cv2x_epc-ue-nas.h"
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/ipv6-l3-protocol.h>
#include <ns3/log.h>
#include "cv2x_epc-tft.h"
#include <ns3/cv2x_lte-ue-component-carrier-manager.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteUeNetDevice");

NS_OBJECT_ENSURE_REGISTERED ( cv2x_LteUeNetDevice);


TypeId cv2x_LteUeNetDevice::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_LteUeNetDevice")
    .SetParent<cv2x_LteNetDevice> ()
    .AddConstructor<cv2x_LteUeNetDevice> ()
    .AddAttribute ("cv2x_EpcUeNas",
                   "The NAS associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteUeNetDevice::m_nas),
                   MakePointerChecker <cv2x_EpcUeNas> ())
    .AddAttribute ("cv2x_LteUeRrc",
                   "The RRC associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteUeNetDevice::m_rrc),
                   MakePointerChecker <cv2x_LteUeRrc> ())
    .AddAttribute ("cv2x_LteUeComponentCarrierManager",
                   "The cv2x_ComponentCarrierManager associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteUeNetDevice::m_componentCarrierManager),
                   MakePointerChecker <cv2x_LteUeComponentCarrierManager> ())
    .AddAttribute ("cv2x_ComponentCarrierMapUe", "List of all component Carrier.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&cv2x_LteUeNetDevice::m_ccMap),
                   MakeObjectMapChecker<cv2x_ComponentCarrierUe> ())
    .AddAttribute ("Imsi",
                   "International Mobile Subscriber Identity assigned to this UE",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteUeNetDevice::m_imsi),
                   MakeUintegerChecker<uint64_t> ())
    .AddAttribute ("DlEarfcn",
                   "Downlink E-UTRA Absolute Radio Frequency Channel Number (EARFCN) "
                   "as per 3GPP 36.101 Section 5.7.3. ",
                   UintegerValue (100),
                   MakeUintegerAccessor (&cv2x_LteUeNetDevice::SetDlEarfcn,
                                         &cv2x_LteUeNetDevice::GetDlEarfcn),
                   MakeUintegerChecker<uint32_t> (0, 262143))
    .AddAttribute ("CsgId",
                   "The Closed Subscriber Group (CSG) identity that this UE is associated with, "
                   "i.e., giving the UE access to cells which belong to this particular CSG. "
                   "This restriction only applies to initial cell selection and EPC-enabled simulation. "
                   "This does not revoke the UE's access to non-CSG cells. ",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteUeNetDevice::SetCsgId,
                                         &cv2x_LteUeNetDevice::GetCsgId),
                   MakeUintegerChecker<uint32_t> ())
  ;

  return tid;
}


cv2x_LteUeNetDevice::cv2x_LteUeNetDevice (void)
  : m_isConstructed (false)
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteUeNetDevice::~cv2x_LteUeNetDevice (void)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteUeNetDevice::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_targetEnb = 0;

  m_rrc->Dispose ();
  m_rrc = 0;
  
  m_nas->Dispose ();
  m_nas = 0;
  for (uint32_t i = 0; i < m_ccMap.size (); i++)
    {
      m_ccMap.at (i)->Dispose ();
    }
  m_componentCarrierManager->Dispose ();
  cv2x_LteNetDevice::DoDispose ();
}

void
cv2x_LteUeNetDevice::UpdateConfig (void)
{
  NS_LOG_FUNCTION (this);

  if (m_isConstructed)
    {
      NS_LOG_LOGIC (this << " Updating configuration: IMSI " << m_imsi
                         << " CSG ID " << m_csgId);
      m_nas->SetImsi (m_imsi);
      m_rrc->SetImsi (m_imsi);
      m_nas->SetCsgId (m_csgId); // this also handles propagation to RRC
    }
  else
    {
      /*
       * NAS and RRC instances are not be ready yet, so do nothing now and
       * expect ``DoInitialize`` to re-invoke this function.
       */
    }
}



Ptr<cv2x_LteUeMac>
cv2x_LteUeNetDevice::GetMac (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ccMap.at (0)->GetMac ();
}


Ptr<cv2x_LteUeRrc>
cv2x_LteUeNetDevice::GetRrc (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rrc;
}


Ptr<cv2x_LteUePhy>
cv2x_LteUeNetDevice::GetPhy (void) const
{
  NS_LOG_FUNCTION (this);
  return m_ccMap.at (0)->GetPhy ();
}

Ptr<cv2x_LteUeComponentCarrierManager>
cv2x_LteUeNetDevice::GetComponentCarrierManager (void) const
{
  NS_LOG_FUNCTION (this);
  return m_componentCarrierManager;
}

Ptr<cv2x_EpcUeNas>
cv2x_LteUeNetDevice::GetNas (void) const
{
  NS_LOG_FUNCTION (this);
  return m_nas;
}

uint64_t
cv2x_LteUeNetDevice::GetImsi () const
{
  NS_LOG_FUNCTION (this);
  return m_imsi;
}

uint32_t
cv2x_LteUeNetDevice::GetDlEarfcn () const
{
  NS_LOG_FUNCTION (this);
  return m_dlEarfcn;
}

void
cv2x_LteUeNetDevice::SetDlEarfcn (uint32_t earfcn)
{
  NS_LOG_FUNCTION (this << earfcn);
  m_dlEarfcn = earfcn;
}

uint32_t
cv2x_LteUeNetDevice::GetCsgId () const
{
  NS_LOG_FUNCTION (this);
  return m_csgId;
}

void
cv2x_LteUeNetDevice::SetCsgId (uint32_t csgId)
{
  NS_LOG_FUNCTION (this << csgId);
  m_csgId = csgId;
  UpdateConfig (); // propagate the change down to NAS and RRC
}

void
cv2x_LteUeNetDevice::SetTargetEnb (Ptr<cv2x_LteEnbNetDevice> enb)
{
  NS_LOG_FUNCTION (this << enb);
  m_targetEnb = enb;
}


Ptr<cv2x_LteEnbNetDevice>
cv2x_LteUeNetDevice::GetTargetEnb (void)
{
  NS_LOG_FUNCTION (this);
  return m_targetEnb;
}

std::map < uint8_t, Ptr<cv2x_ComponentCarrierUe> >
cv2x_LteUeNetDevice::GetCcMap ()
{
  return m_ccMap;
}

void
cv2x_LteUeNetDevice::SetCcMap (std::map< uint8_t, Ptr<cv2x_ComponentCarrierUe> > ccm)
{
  m_ccMap = ccm;
}

void 
cv2x_LteUeNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
  UpdateConfig ();

  std::map< uint8_t, Ptr<cv2x_ComponentCarrierUe> >::iterator it;
  for (it = m_ccMap.begin (); it != m_ccMap.end (); ++it)
    {
      it->second->GetPhy ()->Initialize ();
      it->second->GetMac ()->Initialize ();
    }
  m_rrc->Initialize ();
}

bool
cv2x_LteUeNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << dest << protocolNumber);
  if (protocolNumber != Ipv4L3Protocol::PROT_NUMBER && protocolNumber != Ipv6L3Protocol::PROT_NUMBER)
    {
      NS_LOG_INFO ("unsupported protocol " << protocolNumber << ", only IPv4 and IPv6 are supported");
      return true;
    }  
  return m_nas->Send (packet);
}


} // namespace ns3

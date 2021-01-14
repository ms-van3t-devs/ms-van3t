/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Author: Marco Miozzo <mmiozzo@cttc.es> : Update to FF API Architecture
 * Author: Nicola Baldo <nbaldo@cttc.es>  : Integrated with new RRC and MAC architecture
 * Author: Danilo Abrignani <danilo.abrignani@unibo.it> : Integrated with new architecture - GSoC 2015 - Carrier Aggregation
 */

#include <ns3/llc-snap-header.h>
#include <ns3/simulator.h>
#include <ns3/callback.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/cv2x_lte-net-device.h>
#include <ns3/packet-burst.h>
#include <ns3/uinteger.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/pointer.h>
#include <ns3/enum.h>
#include <ns3/cv2x_lte-amc.h>
#include <ns3/cv2x_lte-enb-mac.h>
#include <ns3/cv2x_lte-enb-net-device.h>
#include <ns3/cv2x_lte-enb-rrc.h>
#include <ns3/cv2x_lte-ue-net-device.h>
#include <ns3/cv2x_lte-enb-phy.h>
#include <ns3/cv2x_ff-mac-scheduler.h>
#include <ns3/cv2x_lte-handover-algorithm.h>
#include <ns3/cv2x_lte-anr.h>
#include <ns3/cv2x_lte-ffr-algorithm.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/ipv6-l3-protocol.h>
#include <ns3/abort.h>
#include <ns3/log.h>
#include <ns3/cv2x_lte-enb-component-carrier-manager.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteEnbNetDevice");

NS_OBJECT_ENSURE_REGISTERED ( cv2x_LteEnbNetDevice);

TypeId cv2x_LteEnbNetDevice::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_LteEnbNetDevice")
    .SetParent<cv2x_LteNetDevice> ()
    .AddConstructor<cv2x_LteEnbNetDevice> ()
    .AddAttribute ("cv2x_LteEnbRrc",
                   "The RRC associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteEnbNetDevice::m_rrc),
                   MakePointerChecker <cv2x_LteEnbRrc> ())
    .AddAttribute ("cv2x_LteHandoverAlgorithm",
                   "The handover algorithm associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteEnbNetDevice::m_handoverAlgorithm),
                   MakePointerChecker <cv2x_LteHandoverAlgorithm> ())
    .AddAttribute ("cv2x_LteAnr",
                   "The automatic neighbour relation function associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteEnbNetDevice::m_anr),
                   MakePointerChecker <cv2x_LteAnr> ())
    .AddAttribute ("cv2x_LteFfrAlgorithm",
                   "The FFR algorithm associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteEnbNetDevice::m_ffrAlgorithm),
                   MakePointerChecker <cv2x_LteFfrAlgorithm> ())
    .AddAttribute ("cv2x_LteEnbComponentCarrierManager",
                   "The RRC associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteEnbNetDevice::m_componentCarrierManager),
                   MakePointerChecker <cv2x_LteEnbComponentCarrierManager> ())
    .AddAttribute ("cv2x_ComponentCarrierMap", "List of component carriers.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&cv2x_LteEnbNetDevice::m_ccMap),
                   MakeObjectMapChecker<cv2x_ComponentCarrierEnb> ())
    .AddAttribute ("UlBandwidth",
                   "Uplink Transmission Bandwidth Configuration in number of Resource Blocks",
                   UintegerValue (25),
                   MakeUintegerAccessor (&cv2x_LteEnbNetDevice::SetUlBandwidth, 
                                         &cv2x_LteEnbNetDevice::GetUlBandwidth),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("DlBandwidth",
                   "Downlink Transmission Bandwidth Configuration in number of Resource Blocks",
                   UintegerValue (25),
                   MakeUintegerAccessor (&cv2x_LteEnbNetDevice::SetDlBandwidth, 
                                         &cv2x_LteEnbNetDevice::GetDlBandwidth),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("CellId",
                   "Cell Identifier",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteEnbNetDevice::m_cellId),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("DlEarfcn",
                   "Downlink E-UTRA Absolute Radio Frequency Channel Number (EARFCN) "
                   "as per 3GPP 36.101 Section 5.7.3. ",
                   UintegerValue (100),
                   MakeUintegerAccessor (&cv2x_LteEnbNetDevice::m_dlEarfcn),
                   MakeUintegerChecker<uint32_t> (0, 262143))
    .AddAttribute ("UlEarfcn",
                   "Uplink E-UTRA Absolute Radio Frequency Channel Number (EARFCN) "
                   "as per 3GPP 36.101 Section 5.7.3. ",
                   UintegerValue (18100),
                   MakeUintegerAccessor (&cv2x_LteEnbNetDevice::m_ulEarfcn),
                   MakeUintegerChecker<uint32_t> (0, 262143))
    .AddAttribute ("CsgId",
                   "The Closed Subscriber Group (CSG) identity that this eNodeB belongs to",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteEnbNetDevice::SetCsgId,
                                         &cv2x_LteEnbNetDevice::GetCsgId),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("CsgIndication",
                   "If true, only UEs which are members of the CSG (i.e. same CSG ID) "
                   "can gain access to the eNodeB, therefore enforcing closed access mode. "
                   "Otherwise, the eNodeB operates as a non-CSG cell and implements open access mode.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&cv2x_LteEnbNetDevice::SetCsgIndication,
                                        &cv2x_LteEnbNetDevice::GetCsgIndication),
                   MakeBooleanChecker ())
  ;
  return tid;
}

cv2x_LteEnbNetDevice::cv2x_LteEnbNetDevice ()
  : m_isConstructed (false),
    m_isConfigured (false),
    m_anr (0),
    m_componentCarrierManager(0)
{
  NS_LOG_FUNCTION (this);
}

cv2x_LteEnbNetDevice::~cv2x_LteEnbNetDevice (void)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteEnbNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION (this);

  m_rrc->Dispose ();
  m_rrc = 0;

  m_handoverAlgorithm->Dispose ();
  m_handoverAlgorithm = 0;

  if (m_anr != 0)
    {
      m_anr->Dispose ();
      m_anr = 0;
    }
  m_componentCarrierManager->Dispose();
  m_componentCarrierManager = 0;
  // cv2x_ComponentCarrierEnb::DoDispose() will call DoDispose
  // of its PHY, MAC, FFR and scheduler instance
  for (uint32_t i = 0; i < m_ccMap.size (); i++)
    {
      m_ccMap.at (i)->Dispose ();
      m_ccMap.at (i) = 0;
    }
   
  cv2x_LteNetDevice::DoDispose ();
}



Ptr<cv2x_LteEnbMac>
cv2x_LteEnbNetDevice::GetMac () const
{
  return m_ccMap.at (0)->GetMac ();
}

Ptr<cv2x_LteEnbPhy>
cv2x_LteEnbNetDevice::GetPhy () const
{
  return m_ccMap.at (0)->GetPhy ();
}

Ptr<cv2x_LteEnbMac>
cv2x_LteEnbNetDevice::GetMac (uint8_t index) 
{
  return m_ccMap.at (index)->GetMac ();
}

Ptr<cv2x_LteEnbPhy>
cv2x_LteEnbNetDevice::GetPhy(uint8_t index)  
{
  return m_ccMap.at (index)->GetPhy ();
}

Ptr<cv2x_LteEnbRrc>
cv2x_LteEnbNetDevice::GetRrc () const
{
  return m_rrc;
}

Ptr<cv2x_LteEnbComponentCarrierManager>
cv2x_LteEnbNetDevice::GetComponentCarrierManager () const
{
  return  m_componentCarrierManager;
}

uint16_t
cv2x_LteEnbNetDevice::GetCellId () const
{
  return m_cellId;
}

bool
cv2x_LteEnbNetDevice::HasCellId (uint16_t cellId) const
{
  for (auto &it: m_ccMap)
    {
      if (it.second->GetCellId () == cellId)
        {
          return true;
        }
    }
  return false;
}

uint8_t 
cv2x_LteEnbNetDevice::GetUlBandwidth () const
{
  return m_ulBandwidth;
}

void 
cv2x_LteEnbNetDevice::SetUlBandwidth (uint8_t bw)
{ 
  NS_LOG_FUNCTION (this << uint16_t (bw));
  switch (bw)
    { 
    case 6:
    case 15:
    case 25:
    case 50:
    case 75:
    case 100:
      m_ulBandwidth = bw;
      break;

    default:
      NS_FATAL_ERROR ("invalid bandwidth value " << (uint16_t) bw);
      break;
    }
}

uint8_t 
cv2x_LteEnbNetDevice::GetDlBandwidth () const
{
  return m_dlBandwidth;
}

void 
cv2x_LteEnbNetDevice::SetDlBandwidth (uint8_t bw)
{
  NS_LOG_FUNCTION (this << uint16_t (bw));
  switch (bw)
    { 
    case 6:
    case 15:
    case 25:
    case 50:
    case 75:
    case 100:
      m_dlBandwidth = bw;
      break;

    default:
      NS_FATAL_ERROR ("invalid bandwidth value " << (uint16_t) bw);
      break;
    }
}

uint32_t 
cv2x_LteEnbNetDevice::GetDlEarfcn () const
{
  return m_dlEarfcn;
}

void 
cv2x_LteEnbNetDevice::SetDlEarfcn (uint32_t earfcn)
{ 
  NS_LOG_FUNCTION (this << earfcn);
  m_dlEarfcn = earfcn;
}

uint32_t 
cv2x_LteEnbNetDevice::GetUlEarfcn () const
{
  return m_ulEarfcn;
}

void 
cv2x_LteEnbNetDevice::SetUlEarfcn (uint32_t earfcn)
{ 
  NS_LOG_FUNCTION (this << earfcn);
  m_ulEarfcn = earfcn;
}

uint32_t
cv2x_LteEnbNetDevice::GetCsgId () const
{
  return m_csgId;
}

void
cv2x_LteEnbNetDevice::SetCsgId (uint32_t csgId)
{
  NS_LOG_FUNCTION (this << csgId);
  m_csgId = csgId;
  UpdateConfig (); // propagate the change to RRC level
}

bool
cv2x_LteEnbNetDevice::GetCsgIndication () const
{
  return m_csgIndication;
}

void
cv2x_LteEnbNetDevice::SetCsgIndication (bool csgIndication)
{
  NS_LOG_FUNCTION (this << csgIndication);
  m_csgIndication = csgIndication;
  UpdateConfig (); // propagate the change to RRC level
}

std::map < uint8_t, Ptr<cv2x_ComponentCarrierEnb> >
cv2x_LteEnbNetDevice::GetCcMap ()
{
  return m_ccMap;
}

void
cv2x_LteEnbNetDevice::SetCcMap (std::map< uint8_t, Ptr<cv2x_ComponentCarrierEnb> > ccm)
{
  NS_ASSERT_MSG (!m_isConfigured, "attempt to set CC map after configuration");
  m_ccMap = ccm;
}

void 
cv2x_LteEnbNetDevice::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
  UpdateConfig ();
  std::map< uint8_t, Ptr<cv2x_ComponentCarrierEnb> >::iterator it;
  for (it = m_ccMap.begin (); it != m_ccMap.end (); ++it)
    {
       it->second->Initialize ();
    }
  m_rrc->Initialize ();
  m_componentCarrierManager->Initialize();
  m_handoverAlgorithm->Initialize ();

  if (m_anr != 0)
    {
      m_anr->Initialize ();
    }

  m_ffrAlgorithm->Initialize ();
}


bool
cv2x_LteEnbNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet   << dest << protocolNumber);
  NS_ASSERT_MSG (protocolNumber == Ipv4L3Protocol::PROT_NUMBER || protocolNumber == Ipv6L3Protocol::PROT_NUMBER, "unsupported protocol " << protocolNumber << ", only IPv4/IPv6 is supported");
  return m_rrc->SendData (packet);
}


void
cv2x_LteEnbNetDevice::UpdateConfig (void)
{
  NS_LOG_FUNCTION (this);

  if (m_isConstructed)
    {
      if (!m_isConfigured)
        {
          NS_LOG_LOGIC (this << " Configure cell " << m_cellId);
          // we have to make sure that this function is called only once
          NS_ASSERT (!m_ccMap.empty ());
          m_rrc->ConfigureCell (m_ccMap);
          m_isConfigured = true;
        }

      NS_LOG_LOGIC (this << " Updating SIB1 of cell " << m_cellId
                         << " with CSG ID " << m_csgId
                         << " and CSG indication " << m_csgIndication);
      m_rrc->SetCsgId (m_csgId, m_csgIndication);
    }
  else
    {
      /*
       * Lower layers are not ready yet, so do nothing now and expect
       * ``DoInitialize`` to re-invoke this function.
       */
    }
}


} // namespace ns3

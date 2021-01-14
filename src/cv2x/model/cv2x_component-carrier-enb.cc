/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Danilo Abrignani
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
 * Author: Danilo Abrignani <danilo.abrignani@unibo.it>
 */

#include "cv2x_component-carrier-enb.h"
#include <ns3/uinteger.h>
#include <ns3/boolean.h>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/cv2x_lte-enb-phy.h>
#include <ns3/pointer.h>
#include <ns3/cv2x_lte-enb-mac.h>
#include <ns3/cv2x_lte-ffr-algorithm.h>
#include <ns3/cv2x_ff-mac-scheduler.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_ComponentCarrierEnb");
NS_OBJECT_ENSURE_REGISTERED (cv2x_ComponentCarrierEnb);

TypeId cv2x_ComponentCarrierEnb::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_ComponentCarrierEnb")
    .SetParent<cv2x_ComponentCarrier> ()
    .AddConstructor<cv2x_ComponentCarrierEnb> ()
    .AddAttribute ("cv2x_LteEnbPhy",
                   "The PHY associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_ComponentCarrierEnb::m_phy),
                   MakePointerChecker <cv2x_LteEnbPhy> ())
    .AddAttribute ("cv2x_LteEnbMac",
                   "The MAC associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_ComponentCarrierEnb::m_mac),
                   MakePointerChecker <cv2x_LteEnbMac> ())
    .AddAttribute ("cv2x_FfMacScheduler",
                   "The scheduler associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_ComponentCarrierEnb::m_scheduler),
                   MakePointerChecker <cv2x_FfMacScheduler> ())
    .AddAttribute ("cv2x_LteFfrAlgorithm",
                   "The FFR algorithm associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_ComponentCarrierEnb::m_ffrAlgorithm),
                   MakePointerChecker <cv2x_LteFfrAlgorithm> ())
  ;
  return tid;
}
cv2x_ComponentCarrierEnb::cv2x_ComponentCarrierEnb ()
{
  NS_LOG_FUNCTION (this);
}

cv2x_ComponentCarrierEnb::~cv2x_ComponentCarrierEnb (void)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_ComponentCarrierEnb::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  if (m_phy)
    {
      m_phy->Dispose ();
      m_phy = 0;
    }
  if (m_mac)
    {
      m_mac->Dispose ();
      m_mac = 0;
    }
  if (m_scheduler)
    {
      m_scheduler->Dispose ();
      m_scheduler = 0;
    }
  if (m_ffrAlgorithm)
    {
      m_ffrAlgorithm->Dispose ();
      m_ffrAlgorithm = 0;
    }

  Object::DoDispose ();
}


void
cv2x_ComponentCarrierEnb::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
  m_phy->Initialize ();
  m_mac->Initialize ();
  m_ffrAlgorithm->Initialize ();
  m_scheduler->Initialize();

}

uint16_t
cv2x_ComponentCarrierEnb::GetCellId ()
{
  return m_cellId;
}

Ptr<cv2x_LteEnbPhy>
cv2x_ComponentCarrierEnb::GetPhy ()
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

void
cv2x_ComponentCarrierEnb::SetCellId (uint16_t cellId)
{
  NS_LOG_FUNCTION (this << cellId);
  m_cellId = cellId;
}

void 
cv2x_ComponentCarrierEnb::SetPhy (Ptr<cv2x_LteEnbPhy> s)
{
  NS_LOG_FUNCTION (this);
  m_phy = s;
}

Ptr<cv2x_LteEnbMac>
cv2x_ComponentCarrierEnb::GetMac ()
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}
void 
cv2x_ComponentCarrierEnb::SetMac (Ptr<cv2x_LteEnbMac> s)
{
  NS_LOG_FUNCTION (this);
  m_mac = s;
}

Ptr<cv2x_LteFfrAlgorithm>
cv2x_ComponentCarrierEnb::GetFfrAlgorithm ()
{
  NS_LOG_FUNCTION (this);
  return m_ffrAlgorithm;
}

void 
cv2x_ComponentCarrierEnb::SetFfrAlgorithm (Ptr<cv2x_LteFfrAlgorithm> s)
{
  NS_LOG_FUNCTION (this);
  m_ffrAlgorithm = s;
}

Ptr<cv2x_FfMacScheduler>
cv2x_ComponentCarrierEnb::GetFfMacScheduler ()
{
  NS_LOG_FUNCTION (this);
  return m_scheduler;
}

void
cv2x_ComponentCarrierEnb::SetFfMacScheduler (Ptr<cv2x_FfMacScheduler> s)
{
  NS_LOG_FUNCTION (this);
  m_scheduler = s;
} 

} // namespace ns3



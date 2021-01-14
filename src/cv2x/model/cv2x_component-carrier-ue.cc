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

#include "cv2x_component-carrier-ue.h"
#include <ns3/uinteger.h>
#include <ns3/boolean.h>
#include <ns3/simulator.h>
#include <ns3/log.h>
#include <ns3/abort.h>
#include <ns3/cv2x_lte-ue-phy.h>
#include <ns3/cv2x_lte-ue-mac.h>
#include <ns3/pointer.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_ComponentCarrierUe");

NS_OBJECT_ENSURE_REGISTERED ( cv2x_ComponentCarrierUe);

TypeId cv2x_ComponentCarrierUe::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_ComponentCarrierUe")
    .SetParent<cv2x_ComponentCarrier> ()
    .AddConstructor<cv2x_ComponentCarrierUe> ()
    .AddAttribute ("cv2x_LteUePhy",
                   "The PHY associated to this EnbNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_ComponentCarrierUe::m_phy),
                   MakePointerChecker <cv2x_LteUePhy> ())
    .AddAttribute ("cv2x_LteUeMac",
                   "The MAC associated to this UeNetDevice",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_ComponentCarrierUe::m_mac),
                   MakePointerChecker <cv2x_LteUeMac> ())
  ;
  return tid;
}
cv2x_ComponentCarrierUe::cv2x_ComponentCarrierUe ()
{
  NS_LOG_FUNCTION (this);
}

cv2x_ComponentCarrierUe::~cv2x_ComponentCarrierUe (void)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_ComponentCarrierUe::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_phy->Dispose ();
  m_phy = 0;
  m_mac->Dispose ();
  m_mac = 0;
  Object::DoDispose ();
}


void
cv2x_ComponentCarrierUe::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  m_isConstructed = true;
  m_phy->Initialize ();
  m_mac->Initialize();
}

void
cv2x_ComponentCarrierUe::SetPhy (Ptr<cv2x_LteUePhy> s)
{
  NS_LOG_FUNCTION (this);
  m_phy = s;
}


Ptr<cv2x_LteUePhy>
cv2x_ComponentCarrierUe::GetPhy () const
{
  NS_LOG_FUNCTION (this);
  return m_phy;
}

void 
cv2x_ComponentCarrierUe::SetMac (Ptr<cv2x_LteUeMac> s)
{
  NS_LOG_FUNCTION (this);
  m_mac = s;
}

Ptr<cv2x_LteUeMac>
cv2x_ComponentCarrierUe::GetMac () const
{
  NS_LOG_FUNCTION (this);
  return m_mac;
}

} // namespace ns3



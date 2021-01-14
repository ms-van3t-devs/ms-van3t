/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011-2013 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Jaume Nin <jnin@cttc.es>
 *         Nicola Baldo <nbaldo@cttc.es>
 *         Manuel Requena <manuel.requena@cttc.es>
 * Modified by: NIST (D2D)
 */

#include <ns3/cv2x_epc-helper.h>
#include <ns3/log.h>
#include <ns3/node.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv6-address.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_EpcHelper");

NS_OBJECT_ENSURE_REGISTERED (cv2x_EpcHelper);


cv2x_EpcHelper::cv2x_EpcHelper () 
{
  NS_LOG_FUNCTION (this);
}

cv2x_EpcHelper::~cv2x_EpcHelper ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
cv2x_EpcHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_EpcHelper")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    ;
  return tid;
}

void
cv2x_EpcHelper::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  Object::DoDispose ();
}



} // namespace ns3

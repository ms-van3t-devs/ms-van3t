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

#include "cv2x_lte-radio-bearer-info.h"
#include "cv2x_lte-ue-rrc.h"
#include "cv2x_lte-rlc.h"
#include "cv2x_lte-pdcp.h"

#include <ns3/log.h>



namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteRadioBearerInfo);

cv2x_LteRadioBearerInfo::cv2x_LteRadioBearerInfo (void)
{
}

cv2x_LteRadioBearerInfo::~cv2x_LteRadioBearerInfo (void)
{
}
  
TypeId 
cv2x_LteRadioBearerInfo::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_LteRadioBearerInfo")
    .SetParent<Object> ()
    .AddConstructor<cv2x_LteRadioBearerInfo> ()
    ;
  return tid;
}
  
  
TypeId 
cv2x_LteDataRadioBearerInfo::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_LteDataRadioBearerInfo")
    .SetParent<cv2x_LteRadioBearerInfo> ()
    .AddConstructor<cv2x_LteDataRadioBearerInfo> ()
    .AddAttribute ("DrbIdentity", "The id of this Data Radio Bearer",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0), // unused (attribute is read-only
                   MakeUintegerAccessor (&cv2x_LteDataRadioBearerInfo::m_drbIdentity),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("cv2x_EpsBearerIdentity", "The id of the EPS bearer corresponding to this Data Radio Bearer",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0), // unused (attribute is read-only
                   MakeUintegerAccessor (&cv2x_LteDataRadioBearerInfo::m_epsBearerIdentity),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("logicalChannelIdentity", "The id of the Logical Channel corresponding to this Data Radio Bearer",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0), // unused (attribute is read-only
                   MakeUintegerAccessor (&cv2x_LteDataRadioBearerInfo::m_logicalChannelIdentity),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("cv2x_LteRlc", "RLC instance of the radio bearer.",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteRadioBearerInfo::m_rlc),
                   MakePointerChecker<cv2x_LteRlc> ())
    .AddAttribute ("cv2x_LtePdcp", "PDCP instance of the radio bearer.",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteRadioBearerInfo::m_pdcp),
                   MakePointerChecker<cv2x_LtePdcp> ())
    ;
  return tid;
}


TypeId 
cv2x_LteSignalingRadioBearerInfo::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_LteSignalingRadioBearerInfo")
    .SetParent<cv2x_LteRadioBearerInfo> ()
    .AddConstructor<cv2x_LteSignalingRadioBearerInfo> ()
    .AddAttribute ("SrbIdentity", "The id of this Signaling Radio Bearer",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0), // unused (attribute is read-only
                   MakeUintegerAccessor (&cv2x_LteSignalingRadioBearerInfo::m_srbIdentity),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("cv2x_LteRlc", "RLC instance of the radio bearer.",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteRadioBearerInfo::m_rlc),
                   MakePointerChecker<cv2x_LteRlc> ())
    .AddAttribute ("cv2x_LtePdcp", "PDCP instance of the radio bearer.",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteRadioBearerInfo::m_pdcp),
                   MakePointerChecker<cv2x_LtePdcp> ())
    ;
  return tid;
}


TypeId 
cv2x_LteSidelinkRadioBearerInfo::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::cv2x_LteSidelinkRadioBearerInfo")
    .SetParent<cv2x_LteRadioBearerInfo> ()
    .AddConstructor<cv2x_LteSidelinkRadioBearerInfo> ()
    .AddAttribute ("DestinationL2Id", "The destination identifier for the communication",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0), // unused (attribute is read-only
                   MakeUintegerAccessor (&cv2x_LteSidelinkRadioBearerInfo::m_destinationL2Id),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("logicalChannelIdentity", "The id of the Logical Channel corresponding to this Data Radio Bearer",
                   TypeId::ATTR_GET, // allow only getting it.
                   UintegerValue (0), // unused (attribute is read-only
                   MakeUintegerAccessor (&cv2x_LteSidelinkRadioBearerInfo::m_logicalChannelIdentity),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("cv2x_LteRlc", "RLC instance of the radio bearer.",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteRadioBearerInfo::m_rlc),
                   MakePointerChecker<cv2x_LteRlc> ())
    .AddAttribute ("cv2x_LtePdcp", "PDCP instance of the radio bearer.",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteRadioBearerInfo::m_pdcp),
                   MakePointerChecker<cv2x_LtePdcp> ())
    // .AddAttribute ("Tx", "If true, indicates that the UE is interesting in transmitting to the group.",
    //                TypeId::ATTR_GET, // allow only getting it.
    //                BooleanValue (false),
    //                MakeBooleanAccessor (&cv2x_LteSidelinkRadioBearerInfo::m_tx),
    //                MakeBooleanChecker ())
    // .AddAttribute ("Rx", "If true, indicates that the UE is interesting in receiving from the group.",
    //                TypeId::ATTR_GET, // allow only getting it.
    //                BooleanValue (false),
    //                MakeBooleanAccessor (&cv2x_LteSidelinkRadioBearerInfo::m_rx),
    //                MakeBooleanChecker ())
    ;
  return tid;
}



} // namespace ns3

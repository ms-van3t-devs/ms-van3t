/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 */


#include "cv2x_ff-mac-scheduler.h"
#include <ns3/log.h>
#include <ns3/enum.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_FfMacScheduler");

NS_OBJECT_ENSURE_REGISTERED (cv2x_FfMacScheduler);


cv2x_FfMacScheduler::cv2x_FfMacScheduler ()
: m_ulCqiFilter (ALL_UL_CQI)
{
  NS_LOG_FUNCTION (this);
}


cv2x_FfMacScheduler::~cv2x_FfMacScheduler ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_FfMacScheduler::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
cv2x_FfMacScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_FfMacScheduler")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddAttribute ("UlCqiFilter",
                   "The filter to apply on UL CQIs received",
                   EnumValue (cv2x_FfMacScheduler::ALL_UL_CQI),
                   MakeEnumAccessor (&cv2x_FfMacScheduler::m_ulCqiFilter),
                   MakeEnumChecker (cv2x_FfMacScheduler::SRS_UL_CQI, "SRS_UL_CQI",
                                    cv2x_FfMacScheduler::PUSCH_UL_CQI, "PUSCH_UL_CQI",
                                    cv2x_FfMacScheduler::ALL_UL_CQI, "ALL_UL_CQI"))
    ;
  return tid;
}


} // namespace ns3




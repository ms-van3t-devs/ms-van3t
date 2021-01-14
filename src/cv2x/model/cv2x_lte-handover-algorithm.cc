/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Budiarto Herman
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#include "cv2x_lte-handover-algorithm.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteHandoverAlgorithm");

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteHandoverAlgorithm);


cv2x_LteHandoverAlgorithm::cv2x_LteHandoverAlgorithm ()
{
}


cv2x_LteHandoverAlgorithm::~cv2x_LteHandoverAlgorithm ()
{
}


TypeId
cv2x_LteHandoverAlgorithm::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::cv2x_LteHandoverAlgorithm")
    .SetParent<Object> ()
    .SetGroupName("Lte")
  ;
  return tid;
}


void
cv2x_LteHandoverAlgorithm::DoDispose ()
{
}



} // end of namespace ns3

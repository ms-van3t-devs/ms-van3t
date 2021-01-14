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
 *
 */

#include "cv2x_lte-ue-component-carrier-manager.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteUeComponentCarrierManager");
NS_OBJECT_ENSURE_REGISTERED (cv2x_LteUeComponentCarrierManager);

cv2x_LteUeComponentCarrierManager::cv2x_LteUeComponentCarrierManager ()
{
}

cv2x_LteUeComponentCarrierManager::~cv2x_LteUeComponentCarrierManager ()
{
}

TypeId
cv2x_LteUeComponentCarrierManager::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::cv2x_LteUeComponentCarrierManager")
                      .SetParent<Object> ()
                      .SetGroupName("Lte");
  return tid;
}

void
cv2x_LteUeComponentCarrierManager::DoDispose ()
{
}

bool 
cv2x_LteUeComponentCarrierManager::SetComponentCarrierMacSapProviders (uint8_t componentCarrierId, cv2x_LteMacSapProvider* sap)
{
  NS_LOG_FUNCTION (this);
  bool result = false;
  std::map <uint8_t, cv2x_LteMacSapProvider*>::iterator it;
  it=m_macSapProvidersMap.find (componentCarrierId);
  if (componentCarrierId > m_noOfComponentCarriers)
    {
      NS_FATAL_ERROR ("Inconsistent componentCarrierId or you didn't call SetNumberOfComponentCarriers before calling this method");
    }
  if (it != m_macSapProvidersMap.end ())
    {
      NS_FATAL_ERROR ("Tried to allocated an existing componentCarrierId");
    }
  else
    {
      m_macSapProvidersMap.insert (std::pair<uint8_t, cv2x_LteMacSapProvider*>(componentCarrierId, sap));
      result = true;
    }
  return result;
  
}

void
cv2x_LteUeComponentCarrierManager::SetNumberOfComponentCarriers (uint8_t noOfComponentCarriers)
{
  NS_LOG_FUNCTION (this);
  m_noOfComponentCarriers = noOfComponentCarriers;
}


} // end of namespace ns3

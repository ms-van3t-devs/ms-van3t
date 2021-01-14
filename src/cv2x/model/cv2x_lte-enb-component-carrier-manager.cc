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

#include "cv2x_lte-enb-component-carrier-manager.h"
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteEnbComponentCarrierManager");
NS_OBJECT_ENSURE_REGISTERED (cv2x_LteEnbComponentCarrierManager);


cv2x_LteEnbComponentCarrierManager::cv2x_LteEnbComponentCarrierManager ()
{

}

cv2x_LteEnbComponentCarrierManager::~cv2x_LteEnbComponentCarrierManager ()
{
}

TypeId
cv2x_LteEnbComponentCarrierManager::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::cv2x_LteEnbComponentCarrierManager")
    .SetParent<Object> ()
    .SetGroupName("Lte")
  ;
  return tid;
}

void
cv2x_LteEnbComponentCarrierManager::DoDispose ()
{
}

void
cv2x_LteEnbComponentCarrierManager::SetLteCcmRrcSapUser (cv2x_LteCcmRrcSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ccmRrcSapUser = s;
}

cv2x_LteCcmRrcSapProvider*
cv2x_LteEnbComponentCarrierManager::GetLteCcmRrcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ccmRrcSapProvider;
}

cv2x_LteMacSapProvider*
cv2x_LteEnbComponentCarrierManager::GetLteMacSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_macSapProvider;
}

cv2x_LteCcmMacSapUser*
cv2x_LteEnbComponentCarrierManager::GetLteCcmMacSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_ccmMacSapUser;
}

bool 
cv2x_LteEnbComponentCarrierManager::SetMacSapProvider (uint8_t componentCarrierId, cv2x_LteMacSapProvider* sap)
{
  NS_LOG_FUNCTION (this);
  bool res = false;
  std::map <uint8_t, cv2x_LteMacSapProvider*>::iterator it = m_macSapProvidersMap.find (componentCarrierId);
  if ((uint16_t) componentCarrierId > m_noOfComponentCarriers)
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
      res = true;
    }
  return res;
  
}

bool 
cv2x_LteEnbComponentCarrierManager::SetCcmMacSapProviders (uint8_t componentCarrierId, cv2x_LteCcmMacSapProvider* sap)
{
  NS_LOG_FUNCTION (this);
  bool res = false;
  std::map< uint8_t,cv2x_LteCcmMacSapProvider*>::iterator it =  m_ccmMacSapProviderMap.find (componentCarrierId);
  
  if (it == m_ccmMacSapProviderMap.end ())
    {
      m_ccmMacSapProviderMap.insert (std::pair <uint8_t,cv2x_LteCcmMacSapProvider*> (componentCarrierId, sap));
    }
 
  res = true;
  return res;
  
}

void
cv2x_LteEnbComponentCarrierManager::SetNumberOfComponentCarriers (uint16_t noOfComponentCarriers)
{
  NS_LOG_FUNCTION (this);
  m_noOfComponentCarriers = noOfComponentCarriers;
}

void
cv2x_LteEnbComponentCarrierManager::SetRrc (const Ptr<cv2x_LteEnbRrc> rrc)
{
  m_rrc = rrc;
}

} // end of namespace ns3

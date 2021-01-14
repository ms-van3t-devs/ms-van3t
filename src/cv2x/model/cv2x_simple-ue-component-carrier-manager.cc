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

#include "cv2x_simple-ue-component-carrier-manager.h"
#include <ns3/log.h>
#include <ns3/cv2x_lte-ue-mac.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_SimpleUeComponentCarrierManager");

NS_OBJECT_ENSURE_REGISTERED (cv2x_SimpleUeComponentCarrierManager);

///////////////////////////////////////////////////////////
// SAP forwarders
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// MAC SAP PROVIDER SAP forwarders
///////////////////////////////////////////////////////////

/// cv2x_SimpleUeCcmMacSapProvider class
class cv2x_SimpleUeCcmMacSapProvider : public cv2x_LteMacSapProvider
{ 
public:
  /**
   * Constructor
   *
   * \param mac the component carrier manager
   */
  cv2x_SimpleUeCcmMacSapProvider (cv2x_SimpleUeComponentCarrierManager* mac);

  // inherited from cv2x_LteMacSapProvider
  virtual void TransmitPdu (cv2x_LteMacSapProvider::TransmitPduParameters params);
  virtual void ReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params);

private:
  cv2x_SimpleUeComponentCarrierManager* m_mac; ///< the component carrier manager
};

cv2x_SimpleUeCcmMacSapProvider::cv2x_SimpleUeCcmMacSapProvider (cv2x_SimpleUeComponentCarrierManager* mac)
  : m_mac (mac)
{
}

void
cv2x_SimpleUeCcmMacSapProvider::TransmitPdu (TransmitPduParameters params)
{
  m_mac->DoTransmitPdu (params);
}


void
cv2x_SimpleUeCcmMacSapProvider::ReportBufferStatus (ReportBufferStatusParameters params)
{
  m_mac->DoReportBufferStatus (params);
}

///////////////////////////////////////////////////////////
// MAC SAP USER SAP forwarders
/////////////// ////////////////////////////////////////////

/// cv2x_SimpleUeCcmMacSapUser class
class cv2x_SimpleUeCcmMacSapUser : public cv2x_LteMacSapUser
{ 
public:
  /**
   * Constructor
   *
   * \param mac the component carrier manager
   */
  cv2x_SimpleUeCcmMacSapUser  (cv2x_SimpleUeComponentCarrierManager* mac);

  // inherited from cv2x_LteMacSapUser
  virtual void NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid);
  virtual void ReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid);
  virtual void NotifyHarqDeliveryFailure ();


private:
  cv2x_SimpleUeComponentCarrierManager* m_mac; ///< the component carrier manager
};

cv2x_SimpleUeCcmMacSapUser::cv2x_SimpleUeCcmMacSapUser (cv2x_SimpleUeComponentCarrierManager* mac)
  : m_mac (mac)
{
}

void
cv2x_SimpleUeCcmMacSapUser::NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid)
{
  NS_LOG_INFO ("cv2x_SimpleUeCcmMacSapUser::NotifyTxOpportunity for ccId:"<<(uint32_t)componentCarrierId);
  m_mac->DoNotifyTxOpportunity (bytes, layer, harqId, componentCarrierId, rnti, lcid);
}


void
cv2x_SimpleUeCcmMacSapUser::ReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid)
{
  m_mac->DoReceivePdu (p, rnti, lcid);
}

void
cv2x_SimpleUeCcmMacSapUser::NotifyHarqDeliveryFailure ()
{
  m_mac->DoNotifyHarqDeliveryFailure ();
}

//////////////////////////////////////////////////////////
// cv2x_SimpleUeComponentCarrierManager methods
///////////////////////////////////////////////////////////

cv2x_SimpleUeComponentCarrierManager::cv2x_SimpleUeComponentCarrierManager ()
: m_ccmRrcSapUser (0)
{
  NS_LOG_FUNCTION (this);
  m_ccmRrcSapProvider = new cv2x_MemberLteUeCcmRrcSapProvider<cv2x_SimpleUeComponentCarrierManager> (this);
  m_noOfComponentCarriersEnabled = 1;
  m_ccmMacSapUser = new cv2x_SimpleUeCcmMacSapUser (this);
  m_ccmMacSapProvider = new cv2x_SimpleUeCcmMacSapProvider (this);
}


cv2x_SimpleUeComponentCarrierManager::~cv2x_SimpleUeComponentCarrierManager ()
{
  NS_LOG_FUNCTION (this);

}


void
cv2x_SimpleUeComponentCarrierManager::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_ccmMacSapUser;
  delete m_ccmMacSapProvider;
  delete m_ccmRrcSapProvider;
}


TypeId
cv2x_SimpleUeComponentCarrierManager::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::cv2x_SimpleUeComponentCarrierManager")
    .SetParent<cv2x_LteUeComponentCarrierManager> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_SimpleUeComponentCarrierManager> ()
    ;
  return tid;
}

cv2x_LteMacSapProvider*
cv2x_SimpleUeComponentCarrierManager::GetLteMacSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ccmMacSapProvider;
}

void
cv2x_SimpleUeComponentCarrierManager::SetLteCcmRrcSapUser (cv2x_LteUeCcmRrcSapUser* s)
{
  NS_LOG_FUNCTION (this << s);
  m_ccmRrcSapUser = s;
}


cv2x_LteUeCcmRrcSapProvider*
cv2x_SimpleUeComponentCarrierManager::GetLteCcmRrcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_ccmRrcSapProvider;
}


void
cv2x_SimpleUeComponentCarrierManager::DoInitialize ()
{
  NS_LOG_FUNCTION (this);
  cv2x_LteUeComponentCarrierManager::DoInitialize ();
}


void
cv2x_SimpleUeComponentCarrierManager::DoReportUeMeas (uint16_t rnti,
                                                 cv2x_LteRrcSap::MeasResults measResults)
{
  NS_LOG_FUNCTION (this << rnti << (uint16_t) measResults.measId);
}


void
cv2x_SimpleUeComponentCarrierManager::DoTransmitPdu (cv2x_LteMacSapProvider::TransmitPduParameters params)
{
  NS_LOG_FUNCTION (this);
   std::map <uint8_t, cv2x_LteMacSapProvider*>::iterator it =  m_macSapProvidersMap.find (params.componentCarrierId);
  NS_ASSERT_MSG (it != m_macSapProvidersMap.end (), "could not find Sap for cv2x_ComponentCarrier " << (uint16_t) params.componentCarrierId);
  // with this algorithm all traffic is on Primary Carrier
  it->second->TransmitPdu (params);
}

void
cv2x_SimpleUeComponentCarrierManager::DoReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this);
  std::map <uint8_t, cv2x_LteMacSapProvider*>::iterator it =  m_macSapProvidersMap.find (0);
  NS_ASSERT_MSG (it != m_macSapProvidersMap.end (), "could not find Sap for cv2x_ComponentCarrier ");
  it->second->ReportBufferStatus (params);
}

void 
cv2x_SimpleUeComponentCarrierManager::DoNotifyHarqDeliveryFailure ()
{
 NS_LOG_FUNCTION (this);
}


void 
cv2x_SimpleUeComponentCarrierManager::DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid)
{
  NS_LOG_FUNCTION (this);
  std::map<uint8_t, cv2x_LteMacSapUser*>::iterator lcidIt = m_lcAttached.find (lcid);
  NS_ASSERT_MSG (lcidIt != m_lcAttached.end (), "could not find LCID" << lcid);
  NS_LOG_DEBUG (this << " lcid= " << (uint32_t) lcid << " layer= " << (uint16_t) layer << " componentCarierId " << (uint16_t) componentCarrierId << " rnti " << rnti);
  (*lcidIt).second->NotifyTxOpportunity (bytes, layer, harqId, componentCarrierId, rnti, lcid);
}
void
cv2x_SimpleUeComponentCarrierManager::DoReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid)
{
  NS_LOG_FUNCTION (this);
  std::map<uint8_t, cv2x_LteMacSapUser*>::iterator lcidIt = m_lcAttached.find (lcid);
  if (lcidIt != m_lcAttached.end ())
    {
      (*lcidIt).second->ReceivePdu (p, rnti, lcid);
    }
}

///////////////////////////////////////////////////////////
// Ue CCM RRC SAP PROVIDER SAP forwarders
///////////////////////////////////////////////////////////
std::vector<uint16_t>
cv2x_SimpleUeComponentCarrierManager::DoRemoveLc (uint8_t lcid)
{
  NS_LOG_FUNCTION (this << " lcId" << lcid);
  std::vector<uint16_t> res;
  NS_ASSERT_MSG (m_lcAttached.find (lcid) != m_lcAttached.end (), "could not find LCID " << lcid);
  m_lcAttached.erase (lcid);
  // send back all the configuration to the componentCarrier where we want to remove the Lc
  std::map<uint8_t, std::map<uint8_t, cv2x_LteMacSapProvider*> >::iterator it =  m_componentCarrierLcMap.begin ();
  while (it != m_componentCarrierLcMap.end ())
    {
      std::map<uint8_t, cv2x_LteMacSapProvider*>::iterator lcToRemove = it->second.find (lcid);
      if (lcToRemove != it->second.end ())
        {
          res.insert (res.end (), it->first);
        }
      it++;
    }
  NS_ASSERT_MSG (res.size () != 0, "Not found in the cv2x_ComponentCarrierManager maps the LCID " << lcid);

  return res; 

}

std::vector<cv2x_LteUeCcmRrcSapProvider::LcsConfig>
cv2x_SimpleUeComponentCarrierManager::DoAddLc (uint8_t lcId,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu)
{
  NS_LOG_FUNCTION (this);
  std::vector<cv2x_LteUeCcmRrcSapProvider::LcsConfig> res;
  std::map<uint8_t, cv2x_LteMacSapUser*>::iterator it = m_lcAttached.find (lcId);
  NS_ASSERT_MSG (it == m_lcAttached.end (), "Warning, LCID " << lcId << " already exist");
  m_lcAttached.insert (std::pair<uint8_t, cv2x_LteMacSapUser*> (lcId, msu));
  cv2x_LteUeCcmRrcSapProvider::LcsConfig elem;
  std::map <uint8_t, std::map<uint8_t, cv2x_LteMacSapProvider*> >::iterator ccLcMapIt;
  for (uint8_t ncc = 0; ncc < m_noOfComponentCarriersEnabled; ncc++)
    {
      elem.componentCarrierId = ncc;
      elem.lcConfig = &lcConfig;
      elem.msu = m_ccmMacSapUser;
      res.insert (res.end (), elem);
      
      ccLcMapIt = m_componentCarrierLcMap.find (ncc);
      if (ccLcMapIt != m_componentCarrierLcMap.end ())
        {
          ccLcMapIt->second.insert (std::pair <uint8_t, cv2x_LteMacSapProvider*> (lcId, m_macSapProvidersMap.at (ncc)));
        }
      else
        {
          std::map<uint8_t, cv2x_LteMacSapProvider*> empty;
          std::pair <std::map <uint8_t, std::map<uint8_t, cv2x_LteMacSapProvider*> >::iterator, bool>
            ret = m_componentCarrierLcMap.insert (std::pair <uint8_t,  std::map<uint8_t, cv2x_LteMacSapProvider*> >
                                                  (ncc, empty));
          NS_ASSERT_MSG (ret.second, "element already present, cv2x_ComponentCarrierId already existed");
          ccLcMapIt = m_componentCarrierLcMap.find (ncc);
          ccLcMapIt->second.insert (std::pair <uint8_t, cv2x_LteMacSapProvider*> (lcId, m_macSapProvidersMap.at (ncc)));
        }
    }
  
  return res;  
}

void
cv2x_SimpleUeComponentCarrierManager::DoNotifyConnectionReconfigurationMsg ()
{
  NS_LOG_FUNCTION (this);
  // this method need to be extended, now support only up to 2 cv2x_ComponentCarrier Simulations

  if (m_noOfComponentCarriersEnabled < m_noOfComponentCarriers)
   {
     // new cv2x_ComponentCarrierConfiguration Requested
     m_noOfComponentCarriersEnabled++;
     std::vector<uint8_t> res;
     res.insert (res.end (), m_noOfComponentCarriersEnabled);
     //here the code to update all the Lc, since now  those should be mapped on all cv2x_ComponentCarriers
     m_ccmRrcSapUser->cv2x_ComponentCarrierEnabling (res);
   }
  
}
cv2x_LteMacSapUser*
cv2x_SimpleUeComponentCarrierManager::DoConfigureSignalBearer (uint8_t lcid,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu)
{
  NS_LOG_FUNCTION (this);
  std::map<uint8_t, cv2x_LteMacSapUser*>::iterator it = m_lcAttached.find (lcid);
  //NS_ASSERT_MSG (it == m_lcAttached.end (), "Warning, LCID " << (uint8_t) lcid << " already exist");
  if (it != m_lcAttached.end ())
    {
      // This line will remove the former SignalBearer. It is needed in case of handover
      // since an update of the signal bearer performed.
      // Now it points on the right cv2x_LteMacSapUser
      m_lcAttached.erase (it); 
    }
  m_lcAttached.insert (std::pair<uint8_t, cv2x_LteMacSapUser*> (lcid, msu));

  return m_ccmMacSapUser;
 } 

} // end of namespace ns3

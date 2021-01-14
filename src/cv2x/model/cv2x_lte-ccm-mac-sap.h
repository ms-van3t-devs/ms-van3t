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

#ifndef CV2X_LTE_CCM_MAC_SAP_H
#define CV2X_LTE_CCM_MAC_SAP_H

#include <ns3/cv2x_lte-rrc-sap.h>
#include <ns3/cv2x_eps-bearer.h>
#include <ns3/cv2x_lte-enb-cmac-sap.h>
#include <ns3/cv2x_lte-mac-sap.h>
#include <ns3/cv2x_ff-mac-common.h>


namespace ns3 {
/**
 * \ingroup lte
 *
 * \brief Service Access Point (SAP) offered by the component carrier manager (CCM) 
 *  by MAC to CCM.
 *
 * This is the *Component Carrier Manager SAP Provider*, i.e., the part of the SAP
 * that contains the MAC methods called by the eNodeB CCM
 * instance.
 */
class cv2x_LteCcmMacSapProvider
{
 
public:
  virtual ~cv2x_LteCcmMacSapProvider ();

  /**
   * \brief Add the Buffer Status Report to the list.
   * \param bsr cv2x_LteEnbComponentCarrierManager used this function to
   *  send back an uplink BSR to some of the MAC instances
   */
  virtual void ReportMacCeToScheduler (cv2x_MacCeListElement_s bsr) = 0;

}; // end of class cv2x_LteCcmMacSapProvider


/**
 * \ingroup lte
 *
 * \brief Service Access Point (SAP) offered by MAC to the 
 *        component carrier manager (CCM).
 *  
 *
 * This is the *CCM MAC SAP User*, i.e., the part of the SAP
 * that contains the component carrier manager methods called 
 * by the eNodeB MAC instance.
 */
class cv2x_LteCcmMacSapUser : public cv2x_LteMacSapUser
{
public:
  virtual ~cv2x_LteCcmMacSapUser ();
  /**
   * \brief When the Primary Component carrier receive a buffer status report 
   *  it is sent to the CCM.
   * \param bsr Buffer Status Report received from a Ue
   * \param componentCarrierId
   */
  virtual void UlReceiveMacCe (cv2x_MacCeListElement_s bsr, uint8_t componentCarrierId) = 0;

  /**
   * \brief Notifies component carrier manager about physical resource block occupancy
   * \param prbOccupancy The physical resource block occupancy
   * \param componentCarrierId The component carrier id
   */
  virtual void NotifyPrbOccupancy (double prbOccupancy, uint8_t componentCarrierId) = 0;

}; // end of class cv2x_LteCcmMacSapUser

/// cv2x_MemberLteCcmMacSapProvider class
template <class C>
class cv2x_MemberLteCcmMacSapProvider : public cv2x_LteCcmMacSapProvider
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteCcmMacSapProvider (C* owner);
  // inherited from cv2x_LteCcmRrcSapProvider
  virtual void ReportMacCeToScheduler (cv2x_MacCeListElement_s bsr);

private:
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteCcmMacSapProvider<C>::cv2x_MemberLteCcmMacSapProvider (C* owner)
  : m_owner (owner)
{
}
 
template <class C>
void cv2x_MemberLteCcmMacSapProvider<C>::ReportMacCeToScheduler (cv2x_MacCeListElement_s bsr)
{
  m_owner->DoReportMacCeToScheduler (bsr);
}


/// cv2x_MemberLteCcmMacSapUser class
template <class C>
class cv2x_MemberLteCcmMacSapUser : public cv2x_LteCcmMacSapUser
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteCcmMacSapUser (C* owner);
  // inherited from cv2x_LteCcmRrcSapUser
  virtual void UlReceiveMacCe (cv2x_MacCeListElement_s bsr, uint8_t componentCarrierId);
  virtual void NotifyPrbOccupancy (double prbOccupancy, uint8_t componentCarrierId);
  // inherited from cv2x_LteMacSapUser
  virtual void NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid);
  virtual void ReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid);
  virtual void NotifyHarqDeliveryFailure ();


private:
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteCcmMacSapUser<C>::cv2x_MemberLteCcmMacSapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
void cv2x_MemberLteCcmMacSapUser<C>::UlReceiveMacCe (cv2x_MacCeListElement_s bsr, uint8_t componentCarrierId)
{
  m_owner->DoUlReceiveMacCe (bsr, componentCarrierId);
}

template <class C>
void cv2x_MemberLteCcmMacSapUser<C>::NotifyPrbOccupancy (double prbOccupancy, uint8_t componentCarrierId)
{
  m_owner->DoNotifyPrbOccupancy (prbOccupancy, componentCarrierId);
}

template <class C>
void cv2x_MemberLteCcmMacSapUser<C>::NotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid)
{
  m_owner->DoNotifyTxOpportunity (bytes, layer, harqId, componentCarrierId, rnti, lcid);
}

template <class C>
void cv2x_MemberLteCcmMacSapUser<C>::ReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid)
{
  m_owner->DoReceivePdu (p, rnti, lcid);
}

template <class C>
void cv2x_MemberLteCcmMacSapUser<C>::NotifyHarqDeliveryFailure ()
{
  m_owner->DoNotifyHarqDeliveryFailure ();
}

  
} // end of namespace ns3


#endif /* CV2X_LTE_CCM_MAC_SAP_H */


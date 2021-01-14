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

#ifndef CV2X_LTE_UE_CCM_RRC_SAP_H
#define CV2X_LTE_UE_CCM_RRC_SAP_H

#include <ns3/cv2x_lte-rrc-sap.h>
#include <ns3/cv2x_eps-bearer.h>
#include <ns3/cv2x_lte-ue-cmac-sap.h>
#include <ns3/cv2x_lte-mac-sap.h>
#include <map>


namespace ns3 {
  class cv2x_LteUeCmacSapProvider;
  class cv2x_LteMacSapUser;

  /**
 * \brief Service Access Point (SAP) offered by the UE component carrier manager 
 *  to the UE RRC.
 *
 * This is the *Component Carrier Management SAP Provider*, i.e., the part of the SAP
 * that contains the component carrier manager methods called by the Ue RRC
 * instance.
 */
class cv2x_LteUeCcmRrcSapProvider
{

/// allow  cv2x_LteMacSapUser class friend access
friend class cv2x_LteMacSapUser;
 
public:

  virtual ~cv2x_LteUeCcmRrcSapProvider ();
  /// LcsConfig structure
  struct LcsConfig
  {
    uint8_t componentCarrierId; ///< component carrier ID
    cv2x_LteUeCmacSapProvider::LogicalChannelConfig *lcConfig; ///< logical channel config
    cv2x_LteMacSapUser *msu; ///< MSU
  };

  /** 
   * add a new Logical Channel (LC) 
   * 
   * \param lcId is the Logical Channel Id
   * \param lcConfig is a single structure contains logical Channel Id, Logical Channel config and Component Carrier Id
   * \param msu is the pointer to cv2x_LteMacSapUser related to the Rlc instance
   * \return vector of LcsConfig contains the lc configuration for each Mac
   *                the size of the vector is equal to the number of component
   *                carrier enabled.
   *
   * The Logical Channel configurations for each component carrier depend on the 
   * algorithm used to split the traffic between the component carriers themself.
   */
  virtual  std::vector<cv2x_LteUeCcmRrcSapProvider::LcsConfig> AddLc (uint8_t lcId,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu) = 0;

  /**
   * \brief Remove an existing Logical Channel for a Ue in the cv2x_LteUeComponentCarrierManager
   * \param lcid the Logical Channel Id
   * \return vector of integer the componentCarrierId of the componentCarrier
   *                where the bearer is enabled
   */
  virtual std::vector<uint16_t> RemoveLc (uint8_t lcid) = 0;
  /// Notify reconfiguration msg function
  virtual void NotifyConnectionReconfigurationMsg () = 0;

  
  /**
   * \brief Add the Signal Bearer for a specific Ue in LteUeComponenCarrierManager
   * \param lcid the Logical Channel Id
   * \param lcConfig this structure it is hard-coded in the cv2x_LteEnbRrc
   * \param msu it is the MacSapUser of the Rlc istance
   * \return the cv2x_LteMacSapUser of the cv2x_ComponentCarrierManager
   *
   */
  virtual cv2x_LteMacSapUser* ConfigureSignalBearer (uint8_t lcid,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu) = 0;

}; // end of class cv2x_LteUeCcmRrcSapProvider

/// cv2x_MemberLteUeCcmRrcSapProvider class
template <class C>
class cv2x_MemberLteUeCcmRrcSapProvider : public cv2x_LteUeCcmRrcSapProvider
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteUeCcmRrcSapProvider (C* owner);

  // inherited from cv2x_LteUeCcmRrcSapProvider
  virtual std::vector<uint16_t> RemoveLc (uint8_t lcid);
  virtual std::vector<cv2x_LteUeCcmRrcSapProvider::LcsConfig> AddLc (uint8_t lcId,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu);
  virtual void NotifyConnectionReconfigurationMsg ();
  virtual cv2x_LteMacSapUser* ConfigureSignalBearer (uint8_t lcid,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu);
  
private:
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteUeCcmRrcSapProvider<C>::cv2x_MemberLteUeCcmRrcSapProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
std::vector<uint16_t> cv2x_MemberLteUeCcmRrcSapProvider<C>::RemoveLc (uint8_t lcid)
{
  return m_owner->DoRemoveLc (lcid);
}

template <class C>
std::vector<cv2x_LteUeCcmRrcSapProvider::LcsConfig> cv2x_MemberLteUeCcmRrcSapProvider<C>::AddLc (uint8_t lcId,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu)
{
  return m_owner->DoAddLc (lcId, lcConfig, msu);
}

template <class C>
void cv2x_MemberLteUeCcmRrcSapProvider<C>::NotifyConnectionReconfigurationMsg ()
{
  m_owner->DoNotifyConnectionReconfigurationMsg ();
}


template <class C>
cv2x_LteMacSapUser* cv2x_MemberLteUeCcmRrcSapProvider<C>::ConfigureSignalBearer (uint8_t lcid,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu)
{
  return m_owner->DoConfigureSignalBearer (lcid, lcConfig, msu);
}


/**
 * \brief Service Access Point (SAP) offered by the UE RRC to the UE CCM.
 *
 * This is the *Component Carrier Management SAP User*, i.e., the part of the SAP
 * that contains the UE RRC methods called by the UE CCM instance.
 */
class cv2x_LteUeCcmRrcSapUser
{
 
public:
  virtual ~cv2x_LteUeCcmRrcSapUser ();

  /**
   * this function will be used after the RRC notify to cv2x_ComponentCarrierManager
   * that a reconfiguration message with Secondary component carrier (SCc) arrived or not
   * the method it is called only if the SCc wasn't set up
   * \param componentCarrierList component carrier list
   */
  virtual void cv2x_ComponentCarrierEnabling (std::vector<uint8_t> componentCarrierList) = 0;

}; // end of class cv2x_LteUeCcmRrcSapUser

  /// MemberLteUeCcmRrcSapUser class
  template <class C>
  class MemberLteUeCcmRrcSapUser : public cv2x_LteUeCcmRrcSapUser
  {
  public:
    /**
     * Constructor
     *
     * \param owner the owner class
     */
    MemberLteUeCcmRrcSapUser (C* owner);
    //inherited from cv2x_LteUeCcmRrcSapUser
    virtual void cv2x_ComponentCarrierEnabling (std::vector<uint8_t> componentCarrierList);

  private:
  C* m_owner; ///< the owner class
};

template <class C>
MemberLteUeCcmRrcSapUser<C>::MemberLteUeCcmRrcSapUser (C* owner)
  : m_owner (owner)
{
}
  
template <class C>
void MemberLteUeCcmRrcSapUser<C>::cv2x_ComponentCarrierEnabling (std::vector<uint8_t> componentCarrierList)
{
  m_owner->DoComponentCarrierEnabling (componentCarrierList);
}
  
} // end of namespace ns3


#endif /* CV2X_LTE_UE_CCM_RRC_SAP_H */


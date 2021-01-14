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

#ifndef CV2X_LTE_CCM_RRC_SAP_H
#define CV2X_LTE_CCM_RRC_SAP_H

#include <ns3/cv2x_lte-rrc-sap.h>
#include <ns3/cv2x_eps-bearer.h>
#include <ns3/cv2x_lte-enb-cmac-sap.h>
#include <ns3/cv2x_lte-mac-sap.h>
#include <map>


namespace ns3 {
  class cv2x_LteUeCmacSapProvider;
  class cv2x_UeManager;
  class cv2x_LteEnbCmacSapProvider;
  class cv2x_LteMacSapUser;
  class cv2x_LteRrcSap;

/**
 * \brief Service Access Point (SAP) offered by the Component Carrier Manager (CCM)
 * instance to the eNodeB RRC instance.
 *
 * This is the *Component Carrier Manager SAP Provider*, i.e., the part of the SAP
 * that contains the CCM methods called by the eNodeB RRC instance.
 */
class cv2x_LteCcmRrcSapProvider
{

/// allow cv2x_UeManager class friend access
friend class cv2x_UeManager;
/// allow cv2x_LteMacSapUser class friend access
friend class cv2x_LteMacSapUser;
 
public:
  
  virtual ~cv2x_LteCcmRrcSapProvider ();
  
  /// LcsConfig structure
  struct LcsConfig
  {
    uint16_t componentCarrierId; ///< component carrier ID
    cv2x_LteEnbCmacSapProvider::LcInfo lc; ///< LC info
    cv2x_LteMacSapUser *msu; ///< MSU
  };

  /**
   * \brief Reports UE measurements to the component carrier manager.
   * \param rnti Radio Network Temporary Identity, an integer identifying 
   * the UE where the measurement report originates from.
   * \param measResults a single report of one measurement identity
   *
   * The received measurement report is a result of the UE measurements configuration
   * previously configured by calling cv2x_LteCcmRrcSapProvider::AddUeMeasReportConfigForComponentCarrier.
   * The report may be stored and utilized for the purpose of making decision if and when
   * to use the secondary carriers.
   */
  virtual void ReportUeMeas (uint16_t rnti, cv2x_LteRrcSap::MeasResults measResults) = 0;

  /**
   * \brief Add a new UE in the cv2x_LteEnbComponentCarrierManager.
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE.
   * \param state The current rrc state of the UE.
   */
  virtual void AddUe (uint16_t rnti, uint8_t state) = 0;

  /**
   * \brief Add a new logical channel.
   * \param lcInfo - information about newly created logical channel
   * \param msu - pointer to corresponding rlc interface
   *
   */
  virtual void AddLc (cv2x_LteEnbCmacSapProvider::LcInfo lcInfo, cv2x_LteMacSapUser* msu) = 0;

  /**
   * \brief Remove an existing UE.
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   */
  virtual void RemoveUe (uint16_t rnti) = 0;

  /**
   * \brief Add a new Bearer for the Ue in the cv2x_LteEnbComponentCarrierManager.
   * \param bearer a pointer to the cv2x_EpsBearer object
   * \param bearerId a unique identifier for the bearer
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   * \param lcid the Logical Channel id
   * \param lcGroup the Logical Channel group
   * \param msu a pointer to the cv2x_LteMacSapUser, the cv2x_LteEnbComponentCarrierManager
   *             has to store a cv2x_LteMacSapUser for each Rlc istance, in order to 
   *             properly redirect the packet
   * \return vector of LcsConfig contains the lc configuration for each Mac
   *                the size of the vector is equal to the number of component
   *                carrier enabled.
   *
   * The Logical Channel configurations for each component carrier depend on the 
   * algorithm used to split the traffic between the component carriers themself.
   */  
  virtual std::vector<cv2x_LteCcmRrcSapProvider::LcsConfig> SetupDataRadioBearer (cv2x_EpsBearer bearer, uint8_t bearerId, uint16_t rnti, uint8_t lcid, uint8_t lcGroup, cv2x_LteMacSapUser *msu) = 0;

   /**
   * \brief Release an existing Data Radio Bearer for a Ue in the cv2x_LteEnbComponentCarrierManager
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   * \param lcid the Logical Channel Id
   * \return vector of integer the componentCarrierId of the componentCarrier
   *                where the bearer is enabled
   */

  virtual std::vector<uint8_t> ReleaseDataRadioBearer (uint16_t rnti, uint8_t lcid) = 0;

  /**
   * \brief Add the Signal Bearer for a specific Ue in LteEnbComponenCarrierManager
   * \param lcInfo this structure it is hard-coded in the cv2x_LteEnbRrc
   * \param rlcMacSapUser it is the MacSapUser of the Rlc istance
   * \return the cv2x_LteMacSapUser of the cv2x_ComponentCarrierManager
   *
   */
  virtual cv2x_LteMacSapUser* ConfigureSignalBearer(cv2x_LteEnbCmacSapProvider::LcInfo lcInfo,  cv2x_LteMacSapUser* rlcMacSapUser) = 0;

}; // end of class cv2x_LteCcmRrcSapProvider


/**
 * \brief Service Access Point (SAP) offered by the eNodeB RRC instance to the
 *        component carrier manager (CCM) instance.
 *
 * This is the *Component Carrier Management SAP User*, i.e., the part of the SAP that
 * contains the eNodeB RRC methods called by the CCM.
 */
class cv2x_LteCcmRrcSapUser
{
  /// allow cv2x_LteEnbRrc class friend access
  friend class cv2x_LteEnbRrc;
public:
  virtual ~cv2x_LteCcmRrcSapUser ();

  /**
   * \brief Request a certain reporting configuration to be fulfilled by the UEs
   *        attached to the eNodeB entity.
   * \param reportConfig the UE measurement reporting configuration
   * \return the measurement identity associated with this newly added
   *         reporting configuration
   *
   * The eNodeB RRC entity is expected to configure the same reporting
   * configuration in each of the attached UEs. When later in the simulation a
   * UE measurement report is received from a UE as a result of this
   * configuration, the eNodeB RRC entity shall forward this report to the
   * cv2x_ComponentCarrier algorithm through the cv2x_LteCcmRrcSapProvider::ReportUeMeas
   * SAP function.
   *
   * \note This function is only valid before the simulation begins.
   */
  virtual uint8_t AddUeMeasReportConfigForComponentCarrier (cv2x_LteRrcSap::ReportConfigEutra reportConfig) = 0;

  /**
   * \brief Instruct the eNodeB RRC entity to prepare a component carrier.
   * \param rnti Radio Network Temporary Identity, an integer identifying the
   *             UE which shall perform the cv2x_ComponentCarrier
   * \param targetCellId the cell ID of the target eNodeB
   *
   * This function is used by the cv2x_ComponentCarrier manager when a decision on 
   * component carriers configurations.
   *
   * The process to produce the decision is up to the implementation of cv2x_ComponentCarrier
   * algorithm. It is typically based on the reported UE measurements, which are
   * received through the cv2x_LteCcmRrcSapProvider::ReportUeMeas function.
   */
  virtual void TriggerComponentCarrier (uint16_t rnti, uint16_t targetCellId) = 0;

  /** 
   * add a new Logical Channel (LC) 
   * 
   * \param lcConfig is a single structure contains logical Channel Id, Logical Channel config and Component Carrier Id
   */
  virtual void AddLcs (std::vector<cv2x_LteEnbRrcSapProvider::LogicalChannelConfig> lcConfig) = 0;

  /** 
   * remove an existing LC
   * 
   * \param rnti 
   * \param lcid
   */
  virtual void ReleaseLcs (uint16_t rnti, uint8_t lcid) = 0;

  /**
   * Get UE manager by RNTI
   *
   * \param rnti RNTI
   * \return UE manager
   */
  virtual Ptr<cv2x_UeManager> GetUeManager (uint16_t rnti) = 0;

}; // end of class cv2x_LteCcmRrcSapUser

/// cv2x_MemberLteCcmRrcSapProvider class
template <class C>
class cv2x_MemberLteCcmRrcSapProvider : public cv2x_LteCcmRrcSapProvider
{
public:
  /**
   * Constructor
   * 
   * \param owner the owner class
   */
  cv2x_MemberLteCcmRrcSapProvider (C* owner);

  // inherited from cv2x_LteCcmRrcSapProvider
  virtual void ReportUeMeas (uint16_t rnti, cv2x_LteRrcSap::MeasResults measResults);
  virtual void AddUe (uint16_t rnti, uint8_t state);
  virtual void AddLc (cv2x_LteEnbCmacSapProvider::LcInfo lcInfo, cv2x_LteMacSapUser* msu);
  virtual void RemoveUe (uint16_t rnti);
  virtual std::vector<cv2x_LteCcmRrcSapProvider::LcsConfig> SetupDataRadioBearer (cv2x_EpsBearer bearer, uint8_t bearerId, uint16_t rnti, uint8_t lcid, uint8_t lcGroup, cv2x_LteMacSapUser *msu);
  virtual std::vector<uint8_t> ReleaseDataRadioBearer (uint16_t rnti, uint8_t lcid);
  virtual cv2x_LteMacSapUser* ConfigureSignalBearer(cv2x_LteEnbCmacSapProvider::LcInfo lcInfo,  cv2x_LteMacSapUser* rlcMacSapUser);

private:
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteCcmRrcSapProvider<C>::cv2x_MemberLteCcmRrcSapProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
void cv2x_MemberLteCcmRrcSapProvider<C>::ReportUeMeas (uint16_t rnti, cv2x_LteRrcSap::MeasResults measResults)
{
  m_owner->DoReportUeMeas (rnti, measResults);
}

template <class C>
void cv2x_MemberLteCcmRrcSapProvider<C>::AddUe (uint16_t rnti, uint8_t state)
{
  m_owner->DoAddUe (rnti, state);
}

template <class C>
void cv2x_MemberLteCcmRrcSapProvider<C>::AddLc (cv2x_LteEnbCmacSapProvider::LcInfo lcInfo, cv2x_LteMacSapUser* msu)
{
  m_owner->DoAddLc (lcInfo, msu);
}

template <class C>
void cv2x_MemberLteCcmRrcSapProvider<C>::RemoveUe (uint16_t rnti)
{
  m_owner->DoRemoveUe (rnti);
}

template <class C>
std::vector<cv2x_LteCcmRrcSapProvider::LcsConfig> cv2x_MemberLteCcmRrcSapProvider<C>::SetupDataRadioBearer (cv2x_EpsBearer bearer, uint8_t bearerId, uint16_t rnti, uint8_t lcid, uint8_t lcGroup, cv2x_LteMacSapUser *msu)
{
  return m_owner->DoSetupDataRadioBearer (bearer, bearerId, rnti, lcid, lcGroup, msu);
}

template <class C>
std::vector<uint8_t> cv2x_MemberLteCcmRrcSapProvider<C>::ReleaseDataRadioBearer (uint16_t rnti, uint8_t lcid)
{
  return m_owner->DoReleaseDataRadioBearer (rnti, lcid);
}

template <class C>
cv2x_LteMacSapUser* cv2x_MemberLteCcmRrcSapProvider<C>::ConfigureSignalBearer(cv2x_LteEnbCmacSapProvider::LcInfo lcInfo,  cv2x_LteMacSapUser* rlcMacSapUser)
{
  return m_owner->DoConfigureSignalBearer (lcInfo, rlcMacSapUser);
}


/// cv2x_MemberLteCcmRrcSapUser class
template <class C>
class cv2x_MemberLteCcmRrcSapUser : public cv2x_LteCcmRrcSapUser
{
public:
  /**
   * Constructor
   * 
   * \param owner the owner class
   */
  cv2x_MemberLteCcmRrcSapUser (C* owner);

  // inherited from cv2x_LteCcmRrcSapUser
  virtual void AddLcs (std::vector<cv2x_LteEnbRrcSapProvider::LogicalChannelConfig> lcConfig);
  virtual void ReleaseLcs (uint16_t rnti, uint8_t lcid);
  virtual uint8_t AddUeMeasReportConfigForComponentCarrier (cv2x_LteRrcSap::ReportConfigEutra reportConfig);
  virtual void TriggerComponentCarrier (uint16_t rnti, uint16_t targetCellId);
  virtual Ptr<cv2x_UeManager> GetUeManager (uint16_t rnti);

private:
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteCcmRrcSapUser<C>::cv2x_MemberLteCcmRrcSapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
void cv2x_MemberLteCcmRrcSapUser<C>::AddLcs (std::vector<cv2x_LteEnbRrcSapProvider::LogicalChannelConfig> lcConfig)
{
  NS_FATAL_ERROR ("Function should not be called because it is not implemented.");
  //m_owner->DoAddLcs (lcConfig);
}

template <class C>
void cv2x_MemberLteCcmRrcSapUser<C>::ReleaseLcs (uint16_t rnti, uint8_t lcid)
{
  NS_FATAL_ERROR ("Function should not be called because it is not implemented.");
  //m_owner->DoReleaseLcs (rnti, lcid);

}

template <class C>
uint8_t
cv2x_MemberLteCcmRrcSapUser<C>::AddUeMeasReportConfigForComponentCarrier (cv2x_LteRrcSap::ReportConfigEutra reportConfig)
{
  return m_owner->DoAddUeMeasReportConfigForComponentCarrier (reportConfig);
}


template <class C>
void
cv2x_MemberLteCcmRrcSapUser<C>::TriggerComponentCarrier (uint16_t rnti, uint16_t targetCellId)
{
  NS_FATAL_ERROR ("Function should not be called because it is not implemented.");
}

template <class C>
Ptr<cv2x_UeManager>
cv2x_MemberLteCcmRrcSapUser<C>::GetUeManager (uint16_t rnti)
{
  return m_owner->GetUeManager (rnti);
}

} // end of namespace ns3


#endif /* CV2X_LTE_CCM_RRC_SAP_H */


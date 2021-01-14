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

#ifndef CV2X_SIMPLE_UE_COMPONENT_CARRIER_MANAGER_H
#define CV2X_SIMPLE_UE_COMPONENT_CARRIER_MANAGER_H

#include <ns3/cv2x_lte-ue-component-carrier-manager.h>
#include <ns3/cv2x_lte-ue-ccm-rrc-sap.h>
#include <ns3/cv2x_lte-rrc-sap.h>
#include <map>

namespace ns3 {
  class cv2x_LteUeCcmRrcSapProvider;

/**
 * \brief Component carrier manager implementation which simply does nothing.
 *
 * Selecting this component carrier selection algorithm is equivalent to disabling automatic
 * triggering of component carrier selection. This is the default choice.
 *
 */
class cv2x_SimpleUeComponentCarrierManager : public cv2x_LteUeComponentCarrierManager
{
public:
  /// Creates a No-op CCS algorithm instance.
  cv2x_SimpleUeComponentCarrierManager ();

  virtual ~cv2x_SimpleUeComponentCarrierManager ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  // inherited from LteComponentCarrierManager
  virtual void SetLteCcmRrcSapUser (cv2x_LteUeCcmRrcSapUser* s);
  virtual cv2x_LteUeCcmRrcSapProvider* GetLteCcmRrcSapProvider ();
  virtual cv2x_LteMacSapProvider* GetLteMacSapProvider ();



  /// let the forwarder class access the protected and private members
  friend class cv2x_MemberLteUeCcmRrcSapProvider<cv2x_SimpleUeComponentCarrierManager>;
  //friend class MemberLteUeCcmRrcSapUser<cv2x_SimpleUeComponentCarrierManager>;
  
  /// allow cv2x_SimpleUeCcmMacSapProvider class friend access
  friend class cv2x_SimpleUeCcmMacSapProvider;
  /// allow cv2x_SimpleUeCcmMacSapUser class friend access
  friend class cv2x_SimpleUeCcmMacSapUser;

protected:

  // inherited from Object
  virtual void DoInitialize ();
  virtual void DoDispose ();
  // inherited from LteCcsAlgorithm as a Component Carrier Management SAP implementation
  /**
   * \brief Report Ue Measure function
   * \param rnti the RNTI
   * \param measResults the measure results
   */
  void DoReportUeMeas (uint16_t rnti, cv2x_LteRrcSap::MeasResults measResults);
  // forwarded from cv2x_LteMacSapProvider
  /**
   * \brief Transmit PDU function
   * \param params cv2x_LteMacSapProvider::TransmitPduParameters
   */
  void DoTransmitPdu (cv2x_LteMacSapProvider::TransmitPduParameters params);
  /**
   * \brief Report buffer status function
   * \param params cv2x_LteMacSapProvider::ReportBufferStatusParameters
   */
  void DoReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params);
  /// Notify HARQ deliver failure
  void DoNotifyHarqDeliveryFailure ();
  // forwarded from cv2x_LteMacSapUser
  /**
   * \brief Notify TX opportunity function
   * \param bytes the number of bytes
   * \param layer the layer
   * \param harqId the HARQ ID
   * \param componentCarrierId the component carrier ID
   * \param rnti the RNTI
   * \param lcid the LCID
   */
  void DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid);
  /**
   * \brief Receive PDU function
   * \param p the packet
   * \param rnti the RNTI
   * \param lcid the LCID
   */
  void DoReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid);
  //forwarded from cv2x_LteUeCcmRrcSapProvider
  /**
   * \brief Add LC function
   * \param lcId the LCID
   * \param lcConfig the logical channel config
   * \param msu the MSU
   * \returns updated LC config list
   */
  std::vector<cv2x_LteUeCcmRrcSapProvider::LcsConfig> DoAddLc (uint8_t lcId,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu);
  /**
   * \brief Remove LC function
   * \param lcid the LCID
   * \returns updated LC list
   */
  std::vector<uint16_t> DoRemoveLc (uint8_t lcid);
  /// Notify connection reconfiguration message
  void DoNotifyConnectionReconfigurationMsg ();
  /**
   * \brief Configure signal bearer function
   * \param lcId the LCID
   * \param lcConfig the logical channel config
   * \param msu the MSU
   * \returns cv2x_LteMacSapUser *
   */
  cv2x_LteMacSapUser* DoConfigureSignalBearer (uint8_t lcId,  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu);
  
private:
  
  cv2x_LteUeCcmRrcSapUser* m_ccmRrcSapUser;//!< Interface to the eNodeB RRC instance.
  cv2x_LteUeCcmRrcSapProvider* m_ccmRrcSapProvider; //!< Receive API calls from the eNodeB RRC instance.
  cv2x_LteMacSapUser* m_ccmMacSapUser;//!< Interface to the eNodeB RLC instance.
  cv2x_LteMacSapProvider* m_ccmMacSapProvider; //!< Receive API calls from the eNodeB RLC instance

}; // end of class cv2x_SimpleUeComponentCarrierManager


} // end of namespace ns3


#endif /* CV2X_SIMPLE_UE_COMPONENT_CARRIER_MANAGER_H */

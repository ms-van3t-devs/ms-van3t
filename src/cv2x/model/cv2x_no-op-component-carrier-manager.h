/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Danilo Abrignani
 * Copyright (c) 2016 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Authors: Danilo Abrignani <danilo.abrignani@unibo.it>
 *          Biljana Bojovic <biljana.bojovic@cttc.es>
 */

#ifndef CV2X_NO_OP_COMPONENT_CARRIER_MANAGER_H
#define CV2X_NO_OP_COMPONENT_CARRIER_MANAGER_H

#include <ns3/cv2x_lte-enb-component-carrier-manager.h>
#include <ns3/cv2x_lte-ccm-rrc-sap.h>
#include <ns3/cv2x_lte-rrc-sap.h>
#include <map>

namespace ns3 {

class cv2x_UeManager;
class cv2x_LteCcmRrcSapProvider;

/**
 * \brief The default component carrier manager that forwards all traffic, the uplink and the downlink,
 *  over the primary carrier, and will not use secondary carriers. To enable carrier aggregation
 *  feature, select another component carrier manager class, i.e., some of child classes of
 *  cv2x_LteEnbComponentCarrierManager of cv2x_NoOpComponentCarrierManager.
 */

class cv2x_NoOpComponentCarrierManager : public cv2x_LteEnbComponentCarrierManager
{
  /// allow cv2x_EnbMacMemberLteMacSapProvider<cv2x_NoOpComponentCarrierManager> class friend access
  friend class cv2x_EnbMacMemberLteMacSapProvider<cv2x_NoOpComponentCarrierManager>;
  /// allow cv2x_MemberLteCcmRrcSapProvider<cv2x_NoOpComponentCarrierManager> class friend access
  friend class cv2x_MemberLteCcmRrcSapProvider<cv2x_NoOpComponentCarrierManager>;
  /// allow cv2x_MemberLteCcmRrcSapUser<cv2x_NoOpComponentCarrierManager> class friend access
  friend class cv2x_MemberLteCcmRrcSapUser<cv2x_NoOpComponentCarrierManager>;
  /// allow cv2x_MemberLteCcmMacSapUser<cv2x_NoOpComponentCarrierManager> class friend access
  friend class cv2x_MemberLteCcmMacSapUser<cv2x_NoOpComponentCarrierManager>;

public:

  cv2x_NoOpComponentCarrierManager ();
  virtual ~cv2x_NoOpComponentCarrierManager ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

protected:
  // Inherited methods
  virtual void DoInitialize ();
  virtual void DoDispose ();
  virtual void DoReportUeMeas (uint16_t rnti, cv2x_LteRrcSap::MeasResults measResults);
  /**
   * \brief Add UE.
   * \param rnti the RNTI
   * \param state the state
   */
  virtual void DoAddUe (uint16_t rnti, uint8_t state);
  /**
   * \brief Add LC.
   * \param lcInfo the LC info
   * \param msu the MSU
   */
  virtual void DoAddLc (cv2x_LteEnbCmacSapProvider::LcInfo lcInfo, cv2x_LteMacSapUser* msu);
  /**
   * \brief Setup data radio bearer.
   * \param bearer the radio bearer
   * \param bearerId the bearerID
   * \param rnti the RNTI
   * \param lcid the LCID
   * \param lcGroup the LC group
   * \param msu the MSU
   * \returns std::vector<cv2x_LteCcmRrcSapProvider::LcsConfig> 
   */
  virtual std::vector<cv2x_LteCcmRrcSapProvider::LcsConfig> DoSetupDataRadioBearer (cv2x_EpsBearer bearer, uint8_t bearerId, uint16_t rnti, uint8_t lcid, uint8_t lcGroup, cv2x_LteMacSapUser* msu);
  /**
   * \brief Transmit PDU.
   * \param params the transmit PDU parameters
   */
  virtual void DoTransmitPdu (cv2x_LteMacSapProvider::TransmitPduParameters params);
  /**
   * \brief Report buffer status.
   * \param params the report buffer status parameters
   */
  virtual void DoReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params);
  /**
   * \brief Notify transmit opportunity.
   * \param bytes the number of bytes
   * \param layer the layer
   * \param harqId the HARQ ID
   * \param componentCarrierId the component carrier ID
   * \param rnti the RNTI
   * \param lcid the LCID
   */
  virtual void DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid);
  /**
   * \brief Receive PDU.
   * \param p the packet
   * \param rnti the RNTI
   * \param lcid the LCID
   */
  virtual void DoReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid);
  /// Notify HARQ delivery failure
  virtual void DoNotifyHarqDeliveryFailure ();
  /**
   * \brief Remove UE.
   * \param rnti the RNTI
   */
  virtual void DoRemoveUe (uint16_t rnti);
  /**
   * \brief Release data radio bearer.
   * \param rnti the RNTI
   * \param lcid the LCID
   * \returns updated data radio bearer list
   */
  virtual std::vector<uint8_t> DoReleaseDataRadioBearer (uint16_t rnti, uint8_t lcid);
  /**
   * \brief Configure the signal bearer.
   * \param lcinfo the cv2x_LteEnbCmacSapProvider::LcInfo
   * \param msu the MSU
   * \returns updated data radio bearer list
   */
  virtual cv2x_LteMacSapUser* DoConfigureSignalBearer(cv2x_LteEnbCmacSapProvider::LcInfo lcinfo,  cv2x_LteMacSapUser* msu);
  /**
   * \brief Forwards uplink BSR to CCM, called by MAC through CCM SAP interface.
   * \param bsr the BSR
   * \param componentCarrierId the component carrier ID
   */
  virtual void DoUlReceiveMacCe (cv2x_MacCeListElement_s bsr, uint8_t componentCarrierId);
  /**
   * \brief Function implements the function of the SAP interface of CCM instance which is used by MAC
   * to notify the PRB occupancy reported by scheduler.
   * \param prbOccupancy the PRB occupancy
   * \param componentCarrierId the component carrier ID
   */
  virtual void DoNotifyPrbOccupancy (double prbOccupancy, uint8_t componentCarrierId);

protected:

  std::map <uint8_t, double > m_ccPrbOccupancy;//!< The physical resource block occupancy per carrier.

}; // end of class cv2x_NoOpComponentCarrierManager


/*
 * \brief Component carrier manager implementation that splits traffic equally among carriers.
 */
class cv2x_RrComponentCarrierManager : public cv2x_NoOpComponentCarrierManager
{
public:

  cv2x_RrComponentCarrierManager ();
  virtual ~cv2x_RrComponentCarrierManager ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

protected:

  // Inherited methods
  virtual void DoReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params);
  virtual void DoUlReceiveMacCe (cv2x_MacCeListElement_s bsr, uint8_t componentCarrierId);

}; // end of class cv2x_RrComponentCarrierManager

} // end of namespace ns3


#endif /* CV2X_NO_OP_COMPONENT_CARRIER_MANAGER_H */

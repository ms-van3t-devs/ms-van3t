/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Modified by: NIST (D2D)
 */


#ifndef CV2X_LTE_RRC_PROTOCOL_IDEAL_H
#define CV2X_LTE_RRC_PROTOCOL_IDEAL_H

#include <stdint.h>
#include <map>

#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/cv2x_lte-rrc-sap.h>

namespace ns3 {

class cv2x_LteUeRrcSapProvider;
class cv2x_LteUeRrcSapUser;
class cv2x_LteEnbRrcSapProvider;
class cv2x_LteUeRrc;


/**
 * \ingroup lte
 *
 * Models the transmission of RRC messages from the UE to the eNB in
 * an ideal fashion, without errors and without consuming any radio
 * resources. 
 * 
 */
class cv2x_LteUeRrcProtocolIdeal : public Object
{
  /// allow cv2x_MemberLteUeRrcSapUser<cv2x_LteUeRrcProtocolIdeal> class friend access
  friend class cv2x_MemberLteUeRrcSapUser<cv2x_LteUeRrcProtocolIdeal>;

public:

  cv2x_LteUeRrcProtocolIdeal ();
  virtual ~cv2x_LteUeRrcProtocolIdeal ();

  // inherited from Object
  virtual void DoDispose (void);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Set LTE UE RRC SAP provider function
   *
   * \param p LTE UE RRC SAP provider
   */
  void SetLteUeRrcSapProvider (cv2x_LteUeRrcSapProvider* p);
  /**
   * Get LTE UE RRC SAP user function
   *
   * \returns LTE UE RRC SAP user
   */
  cv2x_LteUeRrcSapUser* GetLteUeRrcSapUser ();
  
  /**
   * Set LTE UE RRC  function
   *
   * \param rrc LTE UE RRC 
   */
  void SetUeRrc (Ptr<cv2x_LteUeRrc> rrc);
  

private:

  // methods forwarded from cv2x_LteUeRrcSapUser
  /**
   * Setup function
   *
   * \param params cv2x_LteUeRrcSapUser::SetupParameters 
   */
  void DoSetup (cv2x_LteUeRrcSapUser::SetupParameters params);
  /**
   * Send RRC connection request function
   *
   * \param msg cv2x_LteRrcSap::RrcConnectionRequest 
   */
  void DoSendRrcConnectionRequest (cv2x_LteRrcSap::RrcConnectionRequest msg);
  /**
   * Send RRC connection setup completed function
   *
   * \param msg cv2x_LteRrcSap::RrcConnectionSetupCompleted 
   */
  void DoSendRrcConnectionSetupCompleted (cv2x_LteRrcSap::RrcConnectionSetupCompleted msg);
  /**
   * Send RRC connection reconfiguration completed function
   *
   * \param msg cv2x_LteRrcSap::RrcConnectionReconfigurationCompleted 
   */
  void DoSendRrcConnectionReconfigurationCompleted (cv2x_LteRrcSap::RrcConnectionReconfigurationCompleted msg);
  /**
   * Send RRC connection reestablishment request function
   *
   * \param msg cv2x_LteRrcSap::RrcConnectionReestablishmentRequest 
   */
  void DoSendRrcConnectionReestablishmentRequest (cv2x_LteRrcSap::RrcConnectionReestablishmentRequest msg);
  /**
   * Send RRC connection reestablishment complete function
   *
   * \param msg cv2x_LteRrcSap::RrcConnectionReestablishmentRequest 
   */
  void DoSendRrcConnectionReestablishmentComplete (cv2x_LteRrcSap::RrcConnectionReestablishmentComplete msg);
  /**
   * Send measurement report function
   *
   * \param msg cv2x_LteRrcSap::MeasurementReport 
   */
  void DoSendMeasurementReport (cv2x_LteRrcSap::MeasurementReport msg);
  /**
   * Send sidelink ue information function
   *
   * \param msg cv2x_LteRrcSap::SidelinkUeInformation
   */
  void DoSendSidelinkUeInformation (cv2x_LteRrcSap::SidelinkUeInformation msg);


  /// Set ENB RRC SAP provider
  void SetEnbRrcSapProvider ();

  Ptr<cv2x_LteUeRrc> m_rrc; ///< the RRC
  uint16_t m_rnti; ///< the RNTI
  cv2x_LteUeRrcSapProvider* m_ueRrcSapProvider; ///< the UE RRC SAP provider
  cv2x_LteUeRrcSapUser* m_ueRrcSapUser; ///< the RRC SAP user
  cv2x_LteEnbRrcSapProvider* m_enbRrcSapProvider; ///< the ENB RRC SAP provider
  
};


/**
 * Models the transmission of RRC messages from the UE to the eNB in
 * an ideal fashion, without errors and without consuming any radio
 * resources. 
 * 
 */
class cv2x_LteEnbRrcProtocolIdeal : public Object
{
  /// allow cv2x_MemberLteEnbRrcSapUser<cv2x_LteEnbRrcProtocolIdeal> class friend access
  friend class cv2x_MemberLteEnbRrcSapUser<cv2x_LteEnbRrcProtocolIdeal>;

public:

  cv2x_LteEnbRrcProtocolIdeal ();
  virtual ~cv2x_LteEnbRrcProtocolIdeal ();

  // inherited from Object
  virtual void DoDispose (void);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * Set LTE ENB RRC SAP provider function
   *
   * \param p the LTE ENB RRC SAP provider 
   */
  void SetLteEnbRrcSapProvider (cv2x_LteEnbRrcSapProvider* p);
  /**
   * Get LTE ENB RRC SAP user function
   *
   * \returns LTE ENB RRC SAP user
   */
  cv2x_LteEnbRrcSapUser* GetLteEnbRrcSapUser ();

  /**
   * Set the cell ID function
   *
   * \param cellId the cell ID 
   */
  void SetCellId (uint16_t cellId);

  /**
   * Get LTE UE RRC SAP provider function
   *
   * \param rnti the RNTI
   * \returns LTE UE RRC SAP provider
   */
  cv2x_LteUeRrcSapProvider* GetUeRrcSapProvider (uint16_t rnti);
  /**
   * Set UE RRC SAP provider function
   *
   * \param rnti the RNTI
   * \param p the UE RRC SAP provider 
   */
  void SetUeRrcSapProvider (uint16_t rnti, cv2x_LteUeRrcSapProvider* p);

private:

  // methods forwarded from cv2x_LteEnbRrcSapUser
  /**
   * Setup UE function
   *
   * \param rnti the RNTI
   * \param params cv2x_LteEnbRrcSapUser::SetupUeParameters 
   */
  void DoSetupUe (uint16_t rnti, cv2x_LteEnbRrcSapUser::SetupUeParameters params);
  /**
   * Remove UE function
   *
   * \param rnti the RNTI
   */
  void DoRemoveUe (uint16_t rnti);
  /**
   * Send system information function
   *
   * \param cellId cell ID
   * \param msg cv2x_LteRrcSap::SystemInformation
   */
  void DoSendSystemInformation (uint16_t cellId, cv2x_LteRrcSap::SystemInformation msg);
  /**
   * Send system information function
   *
   * \param msg cv2x_LteRrcSap::SystemInformation
   */
  void SendSystemInformation (cv2x_LteRrcSap::SystemInformation msg);
  /**
   * Send RRC connection setup function
   *
   * \param rnti the RNTI
   * \param msg cv2x_LteRrcSap::RrcConnectionSetup
   */
  void DoSendRrcConnectionSetup (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionSetup msg);
  /**
   * Send RRC connection reconfiguration function
   *
   * \param rnti the RNTI
   * \param msg cv2x_LteRrcSap::RrcConnectionReconfiguration
   */
  void DoSendRrcConnectionReconfiguration (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReconfiguration msg);
  /**
   * Send RRC connection reestablishment function
   *
   * \param rnti the RNTI
   * \param msg cv2x_LteRrcSap::RrcConnectionReestablishment
   */
  void DoSendRrcConnectionReestablishment (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReestablishment msg);
  /**
   * Send RRC connection reestablishment reject function
   *
   * \param rnti the RNTI
   * \param msg cv2x_LteRrcSap::RrcConnectionReestablishmentReject
   */
  void DoSendRrcConnectionReestablishmentReject (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReestablishmentReject msg);
  /**
   * Send RRC connection release function
   *
   * \param rnti the RNTI
   * \param msg cv2x_LteRrcSap::RrcConnectionRelease
   */
  void DoSendRrcConnectionRelease (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionRelease msg);
  /**
   * Send RRC connection reject function
   *
   * \param rnti the RNTI
   * \param msg cv2x_LteRrcSap::RrcConnectionReject
   */
  void DoSendRrcConnectionReject (uint16_t rnti, cv2x_LteRrcSap::RrcConnectionReject msg);
  /**
   * Encode handover preparation information function
   *
   * \param msg cv2x_LteRrcSap::HandoverPreparationInfo
   * \returns the packet
   */
  Ptr<Packet> DoEncodeHandoverPreparationInformation (cv2x_LteRrcSap::HandoverPreparationInfo msg);
  /**
   * Encode handover preparation information function
   *
   * \param p the packet
   * \returns cv2x_LteRrcSap::HandoverPreparationInfo
   */
  cv2x_LteRrcSap::HandoverPreparationInfo DoDecodeHandoverPreparationInformation (Ptr<Packet> p);
  /**
   * Encode handover command function
   *
   * \param msg cv2x_LteRrcSap::RrcConnectionReconfiguration
   * \returns rnti the RNTI
   */
  Ptr<Packet> DoEncodeHandoverCommand (cv2x_LteRrcSap::RrcConnectionReconfiguration msg);
  /**
   * Decode handover command function
   *
   * \param p the packet
   * \returns cv2x_LteRrcSap::RrcConnectionReconfiguration
   */
  cv2x_LteRrcSap::RrcConnectionReconfiguration DoDecodeHandoverCommand (Ptr<Packet> p);


  uint16_t m_rnti; ///< the RNTI
  uint16_t m_cellId; ///< the cell ID
  cv2x_LteEnbRrcSapProvider* m_enbRrcSapProvider; ///< the ENB RRC SAP provider
  cv2x_LteEnbRrcSapUser* m_enbRrcSapUser; ///< the ENB RRC SAP user
  std::map<uint16_t, cv2x_LteUeRrcSapProvider*> m_enbRrcSapProviderMap; ///< the LTE UE RRC SAP provider
  
};



}


#endif // CV2X_LTE_RRC_PROTOCOL_IDEAL_H

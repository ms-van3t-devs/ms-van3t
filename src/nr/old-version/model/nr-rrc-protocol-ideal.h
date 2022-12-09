/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License version 2 as
*   published by the Free Software Foundation;
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*/



#ifndef NR_RRC_PROTOCOL_IDEAL_H
#define NR_RRC_PROTOCOL_IDEAL_H

#include <stdint.h>
#include <map>

#include <ns3/ptr.h>
#include <ns3/object.h>
#include <ns3/lte-rrc-sap.h>

namespace ns3 {

class LteUeRrcSapProvider;
class LteUeRrcSapUser;
class LteEnbRrcSapProvider;
class LteUeRrc;


/**
 * \ingroup ue
 * \ingroup gnb
 *
 * \brief RRC message passing from the UE to the GNB
 *
 * Models the transmission of RRC messages from the UE to the gNB in
 * an ideal fashion, without errors and without consuming any radio
 * resources.
 *
 */
class nrUeRrcProtocolIdeal : public Object
{
  friend class MemberLteUeRrcSapUser<nrUeRrcProtocolIdeal>;

public:
  /**
   * \brief nrUeRrcProtocolIdeal constructor
   */
  nrUeRrcProtocolIdeal ();
  /**
   * \brief ~nrUeRrcProtocolIdeal
   */
  virtual ~nrUeRrcProtocolIdeal ();

  // inherited from Object
  virtual void DoDispose (void) override;
  /**
   * \brief GetTypeId
   * \return the type id of the object
   */
  static TypeId GetTypeId (void);

  /**
   * \brief SetLteUeRrcSapProvider
   * \param p
   */
  void SetLteUeRrcSapProvider (LteUeRrcSapProvider* p);
  /**
   * \brief GetLteUeRrcSapUser
   * \return
   */
  LteUeRrcSapUser* GetLteUeRrcSapUser ();

  /**
   * \brief SetUeRrc
   * \param rrc
   */
  void SetUeRrc (Ptr<LteUeRrc> rrc);
private:
  // methods forwarded from LteUeRrcSapUser
  void DoSetup (LteUeRrcSapUser::SetupParameters params);
  void DoSendRrcConnectionRequest (LteRrcSap::RrcConnectionRequest msg);
  void DoSendRrcConnectionSetupCompleted (LteRrcSap::RrcConnectionSetupCompleted msg);
  void DoSendRrcConnectionReconfigurationCompleted (LteRrcSap::RrcConnectionReconfigurationCompleted msg);
  void DoSendRrcConnectionReestablishmentRequest (LteRrcSap::RrcConnectionReestablishmentRequest msg);
  void DoSendRrcConnectionReestablishmentComplete (LteRrcSap::RrcConnectionReestablishmentComplete msg);
  void DoSendMeasurementReport (LteRrcSap::MeasurementReport msg);
  /**
   * \brief Send Ideal UE context remove request function
   *
   * Notify eNodeB to release UE context once radio link failure
   * or random access failure is detected. It is needed since no
   * RLF detection mechanism at eNodeB is implemented
   *
   * \param rnti the RNTI of the UE
   */
  void DoSendIdealUeContextRemoveRequest (uint16_t rnti);

  void SetEnbRrcSapProvider ();

  Ptr<LteUeRrc> m_rrc;
  uint16_t m_rnti;
  LteUeRrcSapProvider* m_ueRrcSapProvider;
  LteUeRrcSapUser* m_ueRrcSapUser;
  LteEnbRrcSapProvider* m_enbRrcSapProvider;

};


/**
 * Models the transmission of RRC messages from the UE to the gNB in
 * an ideal fashion, without errors and without consuming any radio
 * resources.
 *
 */
class NrGnbRrcProtocolIdeal : public Object
{
  friend class MemberLteEnbRrcSapUser<NrGnbRrcProtocolIdeal>;

public:
  NrGnbRrcProtocolIdeal ();
  virtual ~NrGnbRrcProtocolIdeal ();

  // inherited from Object
  virtual void DoDispose (void);
  static TypeId GetTypeId (void);

  void SetLteEnbRrcSapProvider (LteEnbRrcSapProvider* p);
  LteEnbRrcSapUser* GetLteEnbRrcSapUser ();

  LteUeRrcSapProvider* GetUeRrcSapProvider (uint16_t rnti);
  void SetUeRrcSapProvider (uint16_t rnti, LteUeRrcSapProvider* p);

private:
  // methods forwarded from LteEnbRrcSapUser
  void DoSetupUe (uint16_t rnti, LteEnbRrcSapUser::SetupUeParameters params);
  void DoRemoveUe (uint16_t rnti);
  void DoSendSystemInformation (uint16_t cellId, LteRrcSap::SystemInformation msg);
  void SendSystemInformation (uint16_t cellId, LteRrcSap::SystemInformation msg);
  void DoSendRrcConnectionSetup (uint16_t rnti, LteRrcSap::RrcConnectionSetup msg);
  void DoSendRrcConnectionReconfiguration (uint16_t rnti, LteRrcSap::RrcConnectionReconfiguration msg);
  void DoSendRrcConnectionReestablishment (uint16_t rnti, LteRrcSap::RrcConnectionReestablishment msg);
  void DoSendRrcConnectionReestablishmentReject (uint16_t rnti, LteRrcSap::RrcConnectionReestablishmentReject msg);
  void DoSendRrcConnectionRelease (uint16_t rnti, LteRrcSap::RrcConnectionRelease msg);
  void DoSendRrcConnectionReject (uint16_t rnti, LteRrcSap::RrcConnectionReject msg);
  Ptr<Packet> DoEncodeHandoverPreparationInformation (LteRrcSap::HandoverPreparationInfo msg);
  LteRrcSap::HandoverPreparationInfo DoDecodeHandoverPreparationInformation (Ptr<Packet> p);
  Ptr<Packet> DoEncodeHandoverCommand (LteRrcSap::RrcConnectionReconfiguration msg);
  LteRrcSap::RrcConnectionReconfiguration DoDecodeHandoverCommand (Ptr<Packet> p);


  uint16_t m_rnti;
  LteEnbRrcSapProvider* m_enbRrcSapProvider;
  LteEnbRrcSapUser* m_enbRrcSapUser;
  std::map<uint16_t, LteUeRrcSapProvider*> m_enbRrcSapProviderMap;

};



}


#endif // NR_RRC_PROTOCOL_IDEAL_H

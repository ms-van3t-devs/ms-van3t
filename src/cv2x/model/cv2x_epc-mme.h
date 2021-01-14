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
 */

#ifndef CV2X_EPC_MME_H
#define CV2X_EPC_MME_H

#include <ns3/object.h>
#include <ns3/cv2x_epc-s1ap-sap.h>
#include <ns3/cv2x_epc-s11-sap.h>

#include <map>
#include <list>

namespace ns3 {

class Node;
class NetDevice;

/**
 * \brief This object implements the MME functionality.
 *
 */
class cv2x_EpcMme : public Object
{

  /// allow cv2x_MemberEpcS1apSapMme<cv2x_EpcMme> class friend access
  friend class cv2x_MemberEpcS1apSapMme<cv2x_EpcMme>;
  /// allow cv2x_MemberEpcS11SapMme<cv2x_EpcMme> class friend access
  friend class cv2x_MemberEpcS11SapMme<cv2x_EpcMme>;
  
public:
  
  /** 
   * Constructor
   */
  cv2x_EpcMme ();

  /** 
   * Destructor
   */  
  virtual ~cv2x_EpcMme ();
  
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
protected:
  virtual void DoDispose ();

public:


  /** 
   * 
   * \return the MME side of the S1-AP SAP 
   */
  cv2x_EpcS1apSapMme* GetS1apSapMme ();

  /** 
   * Set the SGW side of the S11 SAP 
   * 
   * \param s the SGW side of the S11 SAP 
   */
  void SetS11SapSgw (cv2x_EpcS11SapSgw * s);

  /** 
   * 
   * \return the MME side of the S11 SAP 
   */
  cv2x_EpcS11SapMme* GetS11SapMme ();

  /** 
   * Add a new ENB to the MME. 
   * \param ecgi E-UTRAN Cell Global ID, the unique identifier of the eNodeB
   * \param enbS1UAddr address of the eNB for S1-U communications
   * \param enbS1apSap the ENB side of the S1-AP SAP 
   */
  void AddEnb (uint16_t ecgi, Ipv4Address enbS1UAddr, cv2x_EpcS1apSapEnb* enbS1apSap);
  
  /** 
   * Add a new UE to the MME. This is the equivalent of storing the UE
   * credentials before the UE is ever turned on. 
   * 
   * \param imsi the unique identifier of the UE
   */
  void AddUe (uint64_t imsi);

  /** 
   * Add an EPS bearer to the list of bearers to be activated for this
   * UE. The bearer will be activated when the UE enters the ECM
   * connected state.
   * 
   * \param imsi UE identifier
   * \param tft traffic flow template of the bearer
   * \param bearer QoS characteristics of the bearer
   * \returns bearer ID 
   */
  uint8_t AddBearer (uint64_t imsi, Ptr<cv2x_EpcTft> tft, cv2x_EpsBearer bearer);


private:

  // S1-AP SAP MME forwarded methods
  /**
   * Initial UE Message function 
   * \param mmeUeS1Id the MME UE S1 ID
   * \param enbUeS1Id the ENB UE S1 ID
   * \param imsi the IMSI
   * \param ecgi the ECGI
   */
  void DoInitialUeMessage (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, uint64_t imsi, uint16_t ecgi);
  /**
   * Initial Context Setup Response function 
   * \param mmeUeS1Id the MME UE S1 ID
   * \param enbUeS1Id the ENB UE S1 ID
   * \param erabSetupList the ERAB setup list
   */
  void DoInitialContextSetupResponse (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<cv2x_EpcS1apSapMme::ErabSetupItem> erabSetupList);
  /**
   * Path Switch Request function
   * \param mmeUeS1Id the MME UE S1 ID
   * \param enbUeS1Id the ENB UE S1 ID
   * \param cgi the CGI
   * \param erabToBeSwitchedInDownlinkList the ERAB to be switched in downlink list
   */
  void DoPathSwitchRequest (uint64_t enbUeS1Id, uint64_t mmeUeS1Id, uint16_t cgi, std::list<cv2x_EpcS1apSapMme::ErabSwitchedInDownlinkItem> erabToBeSwitchedInDownlinkList);
  /**
   * ERAB Release Indication function
   * \param mmeUeS1Id the MME UE S1 ID
   * \param enbUeS1Id the ENB UE S1 ID
   * \param erabToBeReleaseIndication the ERAB to be release indication list
   */
  void DoErabReleaseIndication (uint64_t mmeUeS1Id, uint16_t enbUeS1Id, std::list<cv2x_EpcS1apSapMme::ErabToBeReleasedIndication> erabToBeReleaseIndication);

  // S11 SAP MME forwarded methods
  /**
   * Create Session Response function
   * \param msg cv2x_EpcS11SapMme::CreateSessionResponseMessage
   */
  void DoCreateSessionResponse (cv2x_EpcS11SapMme::CreateSessionResponseMessage msg);
  /**
   * Modify Bearer Response function
   * \param msg cv2x_EpcS11SapMme::ModifyBearerResponseMessage
   */
  void DoModifyBearerResponse (cv2x_EpcS11SapMme::ModifyBearerResponseMessage msg);
  /**
   * Delete Bearer Request function
   * \param msg cv2x_EpcS11SapMme::DeleteBearerRequestMessage
   */
  void DoDeleteBearerRequest (cv2x_EpcS11SapMme::DeleteBearerRequestMessage msg);


  /**
   * Hold info on an EPS bearer to be activated
   * 
   */
  struct BearerInfo
  {
    Ptr<cv2x_EpcTft> tft;  ///< traffic flow template
    cv2x_EpsBearer bearer; ///< bearer QOS characteristics 
    uint8_t bearerId; ///< bearer ID
  };
  
  /**
   * Hold info on a UE
   * 
   */
  struct UeInfo : public SimpleRefCount<UeInfo>
  {
    uint64_t mmeUeS1Id; ///< mmeUeS1Id
    uint16_t enbUeS1Id; ///< enbUeS1Id
    uint64_t imsi; ///< UE identifier
    uint16_t cellId; ///< cell ID
    std::list<BearerInfo> bearersToBeActivated; ///< list of bearers to be activated
    uint16_t bearerCounter; ///< bearer counter
  };

  /**
   * UeInfo stored by IMSI
   * 
   */  
  std::map<uint64_t, Ptr<UeInfo> > m_ueInfoMap;

  /**
   * \brief This Function erases all contexts of bearer from MME side
   * \param ueInfo UE information pointer
   * \param epsBearerId Bearer Id which need to be removed corresponding to UE
   */
  void RemoveBearer (Ptr<UeInfo> ueInfo, uint8_t epsBearerId);

  /**
   * Hold info on a ENB
   * 
   */
  struct EnbInfo : public SimpleRefCount<EnbInfo>
  {
    uint16_t gci; ///< GCI
    Ipv4Address s1uAddr; ///< IP address
    cv2x_EpcS1apSapEnb* s1apSapEnb; ///< cv2x_EpcS1apSapEnb
  };

  /**
   * EnbInfo stored by EGCI
   * 
   */
  std::map<uint16_t, Ptr<EnbInfo> > m_enbInfoMap;


  

  cv2x_EpcS1apSapMme* m_s1apSapMme; ///< cv2x_EpcS1apSapMme

  cv2x_EpcS11SapMme* m_s11SapMme; ///< cv2x_EpcS11SapMme
  cv2x_EpcS11SapSgw* m_s11SapSgw; ///< cv2x_EpcS11SapSgw
  
};




} // namespace ns3

#endif // CV2X_EPC_MME_H

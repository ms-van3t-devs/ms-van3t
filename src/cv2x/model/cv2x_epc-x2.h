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
 * Author: Manuel Requena <manuel.requena@cttc.es>
 */

#ifndef CV2X_EPC_X2_H
#define CV2X_EPC_X2_H

#include "ns3/socket.h"
#include "ns3/callback.h"
#include "ns3/ptr.h"
#include "ns3/object.h"

#include "ns3/cv2x_epc-x2-sap.h"

#include <map>

namespace ns3 {


/**
 * cv2x_X2IfaceInfo
 */
class cv2x_X2IfaceInfo : public SimpleRefCount<cv2x_X2IfaceInfo>
{
public:
  /**
   * Constructor
   *
   * \param remoteIpAddr remote IP address
   * \param localCtrlPlaneSocket control plane socket
   * \param localUserPlaneSocket user plane socket
   */
  cv2x_X2IfaceInfo (Ipv4Address remoteIpAddr, Ptr<Socket> localCtrlPlaneSocket, Ptr<Socket> localUserPlaneSocket);
  virtual ~cv2x_X2IfaceInfo (void);

  /**
   * Assignment operator
   *
   * \returns cv2x_X2IfaceInfo& 
   */
  cv2x_X2IfaceInfo& operator= (const cv2x_X2IfaceInfo &);

public:
  Ipv4Address   m_remoteIpAddr; ///< remote IP address
  Ptr<Socket>   m_localCtrlPlaneSocket; ///< local control plane socket
  Ptr<Socket>   m_localUserPlaneSocket; ///< local user plane socket
};


/**
 * cv2x_X2CellInfo
 */
class cv2x_X2CellInfo : public SimpleRefCount<cv2x_X2CellInfo>
{
public:
  /**
   * Constructor
   *
   * \param localCellId local cell ID
   * \param remoteCellId remote cell ID
   */
  cv2x_X2CellInfo (uint16_t localCellId, uint16_t remoteCellId);
  virtual ~cv2x_X2CellInfo (void);

  /**
   * Assignment operator
   *
   * \returns cv2x_X2CellInfo&  
   */
  cv2x_X2CellInfo& operator= (const cv2x_X2CellInfo &);

public:
  uint16_t m_localCellId; ///< local cell ID
  uint16_t m_remoteCellId; ///< remote cell ID
};


/**
 * \ingroup lte
 *
 * This entity is installed inside an eNB and provides the functionality for the X2 interface
 */
class cv2x_EpcX2 : public Object
{
  /// allow cv2x_EpcX2SpecificEpcX2SapProvider<cv2x_EpcX2> class friend access
  friend class cv2x_EpcX2SpecificEpcX2SapProvider<cv2x_EpcX2>;

public:
  /** 
   * Constructor
   */
  cv2x_EpcX2 ();

  /**
   * Destructor
   */
  virtual ~cv2x_EpcX2 (void);

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose (void);


  /**
   * \param s the X2 SAP User to be used by this EPC X2 entity
   */
  void SetEpcX2SapUser (cv2x_EpcX2SapUser * s);

  /**
   * \return the X2 SAP Provider interface offered by this EPC X2 entity
   */
  cv2x_EpcX2SapProvider* GetEpcX2SapProvider ();


  /**
   * Add an X2 interface to this EPC X2 entity
   * \param enb1CellId the cell ID of the current eNodeB
   * \param enb1X2Address the address of the current eNodeB
   * \param enb2CellId the cell ID of the neighbouring eNodeB
   * \param enb2X2Address the address of the neighbouring eNodeB
   */
  void AddX2Interface (uint16_t enb1CellId, Ipv4Address enb1X2Address,
                       uint16_t enb2CellId, Ipv4Address enb2X2Address);


  /** 
   * Method to be assigned to the recv callback of the X2-C (X2 Control Plane) socket.
   * It is called when the eNB receives a packet from the peer eNB of the X2-C interface
   * 
   * \param socket socket of the X2-C interface
   */
  void RecvFromX2cSocket (Ptr<Socket> socket);

  /** 
   * Method to be assigned to the recv callback of the X2-U (X2 User Plane) socket.
   * It is called when the eNB receives a packet from the peer eNB of the X2-U interface
   * 
   * \param socket socket of the X2-U interface
   */
  void RecvFromX2uSocket (Ptr<Socket> socket);


protected:
  // Interface provided by cv2x_EpcX2SapProvider
  /**
   * Send handover request function
   * \param params the send handover request parameters
   */
  virtual void DoSendHandoverRequest (cv2x_EpcX2SapProvider::HandoverRequestParams params);
  /**
   * Send handover request ack function
   * \param params the send handover request ack parameters
   */
  virtual void DoSendHandoverRequestAck (cv2x_EpcX2SapProvider::HandoverRequestAckParams params);
  /**
   * Send handover preparation failure function
   * \param params the handover preparation failure parameters
   */
  virtual void DoSendHandoverPreparationFailure (cv2x_EpcX2SapProvider::HandoverPreparationFailureParams params);
  /**
   * Send SN status transfer function
   * \param params the SN status transfer parameters 
   */
  virtual void DoSendSnStatusTransfer (cv2x_EpcX2SapProvider::SnStatusTransferParams params);
  /**
   * Send UE context release function
   * \param params the UE context release parameters
   */
  virtual void DoSendUeContextRelease (cv2x_EpcX2SapProvider::UeContextReleaseParams params);
  /**
   * Send load information function
   * \param params the send load information parameters
   */
  virtual void DoSendLoadInformation (cv2x_EpcX2SapProvider::LoadInformationParams params);
  /**
   * Send resource status update function
   * \param params the send resource status update parameters
   */
  virtual void DoSendResourceStatusUpdate (cv2x_EpcX2SapProvider::ResourceStatusUpdateParams params);
  /**
   * Send UE data function
   *
   * \param params cv2x_EpcX2SapProvider::UeDataParams
   */
  virtual void DoSendUeData (cv2x_EpcX2SapProvider::UeDataParams params);

  cv2x_EpcX2SapUser* m_x2SapUser; ///< X2 SAP user
  cv2x_EpcX2SapProvider* m_x2SapProvider; ///< X2 SAP provider


private:

  /**
   * Map the targetCellId to the corresponding (sourceSocket, remoteIpAddr) to be used
   * to send the X2 message
   */
  std::map < uint16_t, Ptr<cv2x_X2IfaceInfo> > m_x2InterfaceSockets;

  /**
   * Map the localSocket (the one receiving the X2 message) 
   * to the corresponding (sourceCellId, targetCellId) associated with the X2 interface
   */
  std::map < Ptr<Socket>, Ptr<cv2x_X2CellInfo> > m_x2InterfaceCellIds;

  /**
   * UDP ports to be used for the X2-C interface
   */
  uint16_t m_x2cUdpPort;
  /**
   * UDP ports to be used for the X2-U interface
   */
  uint16_t m_x2uUdpPort;

};

} //namespace ns3

#endif // CV2X_EPC_X2_H

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Modified by: NIST (D2D)
 */

#ifndef CV2X_LTE_PDCP_SAP_H
#define CV2X_LTE_PDCP_SAP_H

#include "ns3/packet.h"

namespace ns3 {

/**
 * Service Access Point (SAP) offered by the PDCP entity to the RRC entity
 * See 3GPP 36.323 Packet Data Convergence Protocol (PDCP) specification
 *
 * This is the PDCP SAP Provider
 * (i.e. the part of the SAP that contains the PDCP methods called by the RRC)
 */
class cv2x_LtePdcpSapProvider
{
public:
  virtual ~cv2x_LtePdcpSapProvider ();

  /**
   * Parameters for cv2x_LtePdcpSapProvider::TransmitPdcpSdu
   */
  struct TransmitPdcpSduParameters
  {
    Ptr<Packet> pdcpSdu;  /**< the RRC PDU */
    uint16_t    rnti; /**< the C-RNTI identifying the UE */
    uint8_t     lcid; /**< the logical channel id corresponding to the sending RLC instance */
    /* Additional identifier for sidelink */
    uint32_t    srcL2Id;  /**< Source L2 ID (24 bits) */
    uint32_t    dstL2Id;  /**< Destination L2 ID (24 bits) */
  };

  /**
   * Send a RRC PDU to the RDCP for transmission
   * This method is to be called
   * when upper RRC entity has a RRC PDU ready to send   
   * 
   * \param params 
   */
  virtual void TransmitPdcpSdu (TransmitPdcpSduParameters params) = 0;
};


/**
 * Service Access Point (SAP) offered by the PDCP entity to the RRC entity
 * See 3GPP 36.323 Packet Data Convergence Protocol (PDCP) specification
 *
 * This is the PDCP SAP User
 * (i.e. the part of the SAP that contains the RRC methods called by the PDCP)
 */
class cv2x_LtePdcpSapUser
{
public:
  virtual ~cv2x_LtePdcpSapUser ();

  /**
   * Parameters for cv2x_LtePdcpSapUser::ReceivePdcpSdu
   */
  struct ReceivePdcpSduParameters
  {
    Ptr<Packet> pdcpSdu;  /**< the RRC PDU */
    uint16_t    rnti; /**< the C-RNTI identifying the UE */
    uint8_t     lcid; /**< the logical channel id corresponding to the sending RLC instance */
    /* Additional identifier for sidelink */
    uint32_t    srcL2Id;  /**< Source L2 ID (24 bits) */
    uint32_t    dstL2Id;  /**< Destination L2 ID (24 bits) */
  };

  /**
  * Called by the PDCP entity to notify the RRC entity of the reception of a new RRC PDU
  *
  * \param params
  */
  virtual void ReceivePdcpSdu (ReceivePdcpSduParameters params) = 0;
};


/// cv2x_LtePdcpSpecificLtePdcpSapProvider class
template <class C>
class cv2x_LtePdcpSpecificLtePdcpSapProvider : public cv2x_LtePdcpSapProvider
{
public:
  /**
   * Constructor
   * 
   * \param pdcp PDCP
   */
  cv2x_LtePdcpSpecificLtePdcpSapProvider (C* pdcp);

  // Interface implemented from cv2x_LtePdcpSapProvider
  virtual void TransmitPdcpSdu (TransmitPdcpSduParameters params);

private:
  cv2x_LtePdcpSpecificLtePdcpSapProvider ();
  C* m_pdcp; ///< the PDCP
};

template <class C>
cv2x_LtePdcpSpecificLtePdcpSapProvider<C>::cv2x_LtePdcpSpecificLtePdcpSapProvider (C* pdcp)
  : m_pdcp (pdcp)
{
}

template <class C>
cv2x_LtePdcpSpecificLtePdcpSapProvider<C>::cv2x_LtePdcpSpecificLtePdcpSapProvider ()
{
}

template <class C>
void cv2x_LtePdcpSpecificLtePdcpSapProvider<C>::TransmitPdcpSdu (TransmitPdcpSduParameters params)
{
  m_pdcp->DoTransmitPdcpSdu (params.pdcpSdu);
}


/// cv2x_LtePdcpSpecificLtePdcpSapUser class
template <class C>
class cv2x_LtePdcpSpecificLtePdcpSapUser : public cv2x_LtePdcpSapUser
{
public:
  /**
   * Constructor
   *
   * \param rrc RRC
   */
  cv2x_LtePdcpSpecificLtePdcpSapUser (C* rrc);

  // Interface implemented from cv2x_LtePdcpSapUser
  virtual void ReceivePdcpSdu (ReceivePdcpSduParameters params);

private:
  cv2x_LtePdcpSpecificLtePdcpSapUser ();
  C* m_rrc; ///< RRC
};

template <class C>
cv2x_LtePdcpSpecificLtePdcpSapUser<C>::cv2x_LtePdcpSpecificLtePdcpSapUser (C* rrc)
  : m_rrc (rrc)
{
}

template <class C>
cv2x_LtePdcpSpecificLtePdcpSapUser<C>::cv2x_LtePdcpSpecificLtePdcpSapUser ()
{
}

template <class C>
void cv2x_LtePdcpSpecificLtePdcpSapUser<C>::ReceivePdcpSdu (ReceivePdcpSduParameters params)
{
  m_rrc->DoReceivePdcpSdu (params);
}


} // namespace ns3

#endif // CV2X_LTE_PDCP_SAP_H

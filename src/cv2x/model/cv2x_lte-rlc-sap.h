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

#ifndef CV2X_LTE_RLC_SAP_H
#define CV2X_LTE_RLC_SAP_H

#include "ns3/packet.h"

namespace ns3 {

/**
 * Service Access Point (SAP) offered by the UM-RLC and AM-RLC entities to the PDCP entity
 * See 3GPP 36.322 Radio Link Control (RLC) protocol specification
 *
 * This is the RLC SAP Provider
 * (i.e. the part of the SAP that contains the RLC methods called by the PDCP)
 */
class cv2x_LteRlcSapProvider
{
public:
  virtual ~cv2x_LteRlcSapProvider ();

  /**
   * Parameters for cv2x_LteRlcSapProvider::TransmitPdcpPdu
   */
  struct TransmitPdcpPduParameters
  {
    Ptr<Packet> pdcpPdu;  /**< the PDCP PDU */
    uint16_t    rnti; /**< the C-RNTI identifying the UE */
    uint8_t     lcid; /**< the logical channel id corresponding to the sending RLC instance */
    /* Additional identifier for sidelink */
    uint32_t    srcL2Id;  /**< Source L2 ID (24 bits) */
    uint32_t    dstL2Id;  /**< Destination L2 ID (24 bits) */
  };

  /**
   * Send a PDCP PDU to the RLC for transmission
   * This method is to be called
   * when upper PDCP entity has a PDCP PDU ready to send
   * \param params the TransmitPdcpPduParameters
   */
  virtual void TransmitPdcpPdu (TransmitPdcpPduParameters params) = 0;
};


/**
 * Service Access Point (SAP) offered by the UM-RLC and AM-RLC entities to the PDCP entity
 * See 3GPP 36.322 Radio Link Control (RLC) protocol specification
 *
 * This is the RLC SAP User
 * (i.e. the part of the SAP that contains the PDCP methods called by the RLC)
 */
class cv2x_LteRlcSapUser
{
public:
  virtual ~cv2x_LteRlcSapUser ();

  /**
  * Called by the RLC entity to notify the PDCP entity of the reception of a new PDCP PDU
  *
  * \param p the PDCP PDU
  */
  virtual void ReceivePdcpPdu (Ptr<Packet> p) = 0;
};


/// cv2x_LteRlcSpecificLteRlcSapProvider
template <class C>
class cv2x_LteRlcSpecificLteRlcSapProvider : public cv2x_LteRlcSapProvider
{
public:
  /**
   * Constructor
   *
   * \param rlc the RLC
   */
  cv2x_LteRlcSpecificLteRlcSapProvider (C* rlc);

  /**
   * Interface implemented from cv2x_LteRlcSapProvider
   * \param params the TransmitPdcpPduParameters
   */
  virtual void TransmitPdcpPdu (TransmitPdcpPduParameters params);

private:
  cv2x_LteRlcSpecificLteRlcSapProvider ();
  C* m_rlc; ///< the RLC
};

template <class C>
cv2x_LteRlcSpecificLteRlcSapProvider<C>::cv2x_LteRlcSpecificLteRlcSapProvider (C* rlc)
  : m_rlc (rlc)
{
}

template <class C>
cv2x_LteRlcSpecificLteRlcSapProvider<C>::cv2x_LteRlcSpecificLteRlcSapProvider ()
{
}

template <class C>
void cv2x_LteRlcSpecificLteRlcSapProvider<C>::TransmitPdcpPdu (TransmitPdcpPduParameters params)
{
  m_rlc->DoTransmitPdcpPdu (params.pdcpPdu);
}


/// cv2x_LteRlcSpecificLteRlcSapUser class
template <class C>
class cv2x_LteRlcSpecificLteRlcSapUser : public cv2x_LteRlcSapUser
{
public:
  /**
   * Constructor
   *
   * \param pdcp the PDCP
   */
  cv2x_LteRlcSpecificLteRlcSapUser (C* pdcp);

  // Interface implemented from cv2x_LteRlcSapUser
  virtual void ReceivePdcpPdu (Ptr<Packet> p);

private:
  cv2x_LteRlcSpecificLteRlcSapUser ();
  C* m_pdcp; ///< the PDCP
};

template <class C>
cv2x_LteRlcSpecificLteRlcSapUser<C>::cv2x_LteRlcSpecificLteRlcSapUser (C* pdcp)
  : m_pdcp (pdcp)
{
}

template <class C>
cv2x_LteRlcSpecificLteRlcSapUser<C>::cv2x_LteRlcSpecificLteRlcSapUser ()
{
}

template <class C>
void cv2x_LteRlcSpecificLteRlcSapUser<C>::ReceivePdcpPdu (Ptr<Packet> p)
{
  m_pdcp->DoReceivePdcpPdu (p);
}


} // namespace ns3

#endif // CV2X_LTE_RLC_SAP_H

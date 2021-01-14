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
 * Author: Nicola Baldo <nbaldo@cttc.es>
 * Modified by: NIST (D2D)
 */

#ifndef CV2X_LTE_RADIO_BEARER_INFO_H
#define CV2X_LTE_RADIO_BEARER_INFO_H

#include <ns3/object.h>
#include <ns3/pointer.h>
#include <ns3/cv2x_eps-bearer.h>
#include <ns3/cv2x_lte-rrc-sap.h>
#include <ns3/ipv4-address.h>

namespace ns3 {

class cv2x_LteRlc;
class cv2x_LtePdcp;

/**
 * store information on active radio bearer instance
 * 
 */
class cv2x_LteRadioBearerInfo : public Object
{

public:
  cv2x_LteRadioBearerInfo (void);
  virtual ~cv2x_LteRadioBearerInfo (void);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  Ptr<cv2x_LteRlc> m_rlc; ///< RLC
  Ptr<cv2x_LtePdcp> m_pdcp; ///< PDCP
};


/**
 * store information on active signaling radio bearer instance
 * 
 */
class cv2x_LteSignalingRadioBearerInfo : public cv2x_LteRadioBearerInfo
{

public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  uint8_t m_srbIdentity; ///< SRB indentity
  cv2x_LteRrcSap::LogicalChannelConfig m_logicalChannelConfig; ///< logical channel config  
};


/**
 * store information on active data radio bearer instance
 * 
 */
class cv2x_LteDataRadioBearerInfo : public cv2x_LteRadioBearerInfo
{

public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  cv2x_EpsBearer m_epsBearer; ///< EPS bearer
  uint8_t m_epsBearerIdentity; ///< EPS bearer identity
  uint8_t m_drbIdentity; ///< DRB identity
  cv2x_LteRrcSap::RlcConfig m_rlcConfig; ///< RLC config
  uint8_t m_logicalChannelIdentity; ///< logical channel identity
  cv2x_LteRrcSap::LogicalChannelConfig m_logicalChannelConfig; ///< logical channel config
  uint32_t m_gtpTeid; /**< S1-bearer GTP tunnel endpoint identifier, see 36.423 9.2.1 */
  Ipv4Address m_transportLayerAddress; /**< IP Address of the SGW, see 36.423 9.2.1 */
};


/**
 * store information on active sidelink data radio bearer instance
 * 
 */
class cv2x_LteSidelinkRadioBearerInfo : public cv2x_LteRadioBearerInfo
{

public:
  static TypeId GetTypeId (void);

  //cv2x_LteRrcSap::RlcConfig m_rlcConfig;
  uint8_t m_logicalChannelIdentity;
  cv2x_LteRrcSap::LogicalChannelConfig m_logicalChannelConfig;
  uint32_t m_sourceL2Id;
  uint32_t m_destinationL2Id;
  //bool m_tx;
  //bool m_rx;
};



} // namespace ns3


#endif // CV2X_LTE_RADIO_BEARER_INFO_H

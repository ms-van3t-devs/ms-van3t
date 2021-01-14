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
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#ifndef CV2X_LTE_UE_CMAC_SAP_H
#define CV2X_LTE_UE_CMAC_SAP_H

#include <ns3/packet.h>
#include <ns3/cv2x_ff-mac-common.h>
#include <ns3/cv2x_eps-bearer.h>
#include <ns3/cv2x_lte-common.h>
#include <ns3/cv2x_sl-pool.h>
#include <ns3/cv2x_lte-control-messages.h>

namespace ns3 {



class cv2x_LteMacSapUser;

/**
 * Service Access Point (SAP) offered by the UE MAC to the UE RRC
 *
 * This is the MAC SAP Provider, i.e., the part of the SAP that contains the MAC methods called by the RRC
 */
class cv2x_LteUeCmacSapProvider
{
public:
  virtual ~cv2x_LteUeCmacSapProvider ();

  /// RachConfig structure
  struct RachConfig
  {
    uint8_t numberOfRaPreambles; ///< number of RA preambles
    uint8_t preambleTransMax; ///< preamble transmit maximum
    uint8_t raResponseWindowSize; ///< RA response window size
  };
  
  /** 
   * Configure RACH function 
   *
   * \param rc the RACH config
   */
  virtual void ConfigureRach (RachConfig rc) = 0;

  /** 
   * tell the MAC to start a contention-based random access procedure,
   * e.g., to perform RRC connection establishment 
   * 
   */
  virtual void StartContentionBasedRandomAccessProcedure () = 0;

  /** 
   * tell the MAC to start a non-contention-based random access
   * procedure, e.g., as a consequence of handover
   * 
   * \param rnti
   * \param rapId Random Access Preamble Identifier
   * \param prachMask 
   */
  virtual void StartNonContentionBasedRandomAccessProcedure (uint16_t rnti, uint8_t rapId, uint8_t prachMask) = 0;


  /// LogicalChannelConfig structure
  struct LogicalChannelConfig
  {
    uint8_t priority; ///< priority
    uint16_t prioritizedBitRateKbps; ///< prioritize bit rate Kbps
    uint16_t bucketSizeDurationMs; ///< bucket size duration ms
    uint8_t logicalChannelGroup; ///< logical channel group
  };
  
  /** 
   * add a new Logical Channel (LC) 
   * 
   * \param lcId the ID of the LC
   * \param lcConfig the LC configuration provided by the RRC
   * \param msu the corresponding cv2x_LteMacSapUser
   */
  virtual void AddLc (uint8_t lcId, LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu) = 0;

  /**
   * add a new Logical Channel (LC) used for Sidelink communication
   *
   * \param lcId the ID of the LC
   * \param lcConfig the LC configuration provided by the RRC
   * \param msu the corresponding cv2x_LteMacSapUser
   */
  virtual void AddLc (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id, LogicalChannelConfig lcConfig, cv2x_LteMacSapUser* msu) = 0;
  

  /** 
   * remove an existing LC
   * 
   * \param lcId 
   */
  virtual void RemoveLc (uint8_t lcId) = 0;

  /**
   * remove an existing LC
   *
   * \param lcId
   */
  virtual void RemoveLc (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id) = 0;
  

  /** 
   * reset the MAC
   * 
   */
  virtual void Reset () = 0;
  
  /**
   *
   * \param rnti the cell-specific UE identifier
   */
  virtual void SetRnti (uint16_t rnti) = 0;

  /**
   * add a sidelink transmission pool for the given destination
   * \param dstL2Id the destination
   * \param pool the transmission pool
   */
  virtual void AddSlTxPool (uint32_t dstL2Id, Ptr<SidelinkTxCommResourcePool> pool) = 0;

  /**
   * add a sidelink V2X transmission pool for the given destination
   * \param dstL2Id the destination
   * \param pool the transmission pool
   */
  virtual void AddSlV2xTxPool(uint32_t dstL2Id, Ptr<SidelinkTxCommResourcePoolV2x> pool) = 0;

  /**
   * remove the sidelink transmission pool for the given destination
   * \param dstL2Id the destination
   */
  virtual void RemoveSlTxPool (uint32_t dstL2Id) = 0;

  /**
   * remove the sidelink V2X transmission pool for the given destination
   * \param dstL2Id the destination
   */
  virtual void RemoveSlV2xTxPool (uint32_t dstL2Id) = 0;

  /**
   * set the sidelink receiving pools
   * \param pools the sidelink receiving pools
   */
  virtual void SetSlRxPools (std::list<Ptr<SidelinkRxCommResourcePool> > pools) = 0;

  /**
   * set the sidelink V2X receiving pools
   * \param pools the sidelink receiving pools
   */
  virtual void SetSlV2xRxPools (std::list<Ptr<SidelinkRxCommResourcePoolV2x> > pools) = 0;

  /**
   * add a new destination to listen for
   * \param a destination to listen for
   */
  virtual void AddSlDestination (uint32_t destination) = 0;

  /**
   * remove a destination to listen for
   * \param destination the destination that is no longer of interest
   */
  virtual void RemoveSlDestination (uint32_t destination) = 0;

  /**
   * add a sidelink discovery pool 
   * \param res discovery resources
   * \param pool the transmission pool
   */  
  virtual void AddSlTxPool (Ptr<SidelinkTxDiscResourcePool> pool) = 0;

  /**
   * remove the sidelink discovery pool 
   */
  virtual void RemoveSlTxPool () = 0;

  /**
   * set the sidelink discovery receiving pools
   * \param pools the sidelink discovery receiving pools
   */
  virtual void SetSlRxPools (std::list<Ptr<SidelinkRxDiscResourcePool> > pools) = 0;
  
  /**
   * Push announcing applications to MAC 
   * \param apps applications to announce
   */
  virtual void ModifyDiscTxApps (std::list<uint32_t> apps) = 0;
 
  /**
   * Push monitoring applications to MAC 
   * \params apps applications to monitor
   */
  virtual void ModifyDiscRxApps (std::list<uint32_t> apps) = 0;

  /**
   *  added function to handle priority in UL scheduling 
   *
   */
  virtual void AddLCPriority (uint8_t rnti, uint8_t lcid ,uint8_t  priority) = 0;

};



/**
 * Service Access Point (SAP) offered by the UE MAC to the UE RRC
 *
 * This is the MAC SAP User, i.e., the part of the SAP that contains the RRC methods called by the MAC
 */
class cv2x_LteUeCmacSapUser
{
public:

  virtual ~cv2x_LteUeCmacSapUser ();

  /** 
   * 
   * 
   * \param rnti the T-C-RNTI, which will eventually become the C-RNTI after contention resolution
   */
  virtual void SetTemporaryCellRnti (uint16_t rnti) = 0;

  /** 
   * Notify the RRC that the MAC Random Access procedure completed successfully
   * 
   */
  virtual void NotifyRandomAccessSuccessful () = 0;

  /** 
   * Notify the RRC that the MAC Random Access procedure failed
   * 
   */
  virtual void NotifyRandomAccessFailed () = 0;

  /**
   * Notify the RRC that the MAC has detected a new incoming flow for sidelink reception
   */
  virtual void NotifySidelinkReception (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id) = 0;

  /**
   * Notify the RRC that the MAC has data to send in the PSSCH
   */
  virtual void NotifyMacHasSlDataToSend () = 0;
  /**
   * Notify the RRC that the MAC does not have data to send in the PSSCH
   */
  virtual void NotifyMacHasNotSlDataToSend () = 0;

  /**
   * Notify the RRC that the MAC has detected a new incoming flow for discovery reception
   */
  virtual void NotifyDiscoveryReception (Ptr<cv2x_LteControlMessage> msg) = 0;
};




} // namespace ns3


#endif // CV2X_LTE_UE_CMAC_SAP_H

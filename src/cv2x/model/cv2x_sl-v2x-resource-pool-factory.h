/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This software was developed at the National Institute of Standards and
 * Technology by employees of the Federal Government in the course of
 * their official duties. Pursuant to titleElement 17 Section 105 of the United
 * States Code this software is not subject to copyright protection and
 * is in the public domain.
 * NIST assumes no responsibility whatsoever for its use by other parties,
 * and makes no guarantees, expressed or implied, about its quality,
 * reliability, or any other characteristic.

 * We would appreciate acknowledgement if the software is used.

 * NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 * DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 * FROM THE USE OF THIS SOFTWARE.
 *
 * Modified by: NIST
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#ifndef CV2X_SL_V2X_RESOURCE_POOL_FACTORY_H
#define CV2X_SL_V2X_RESOURCE_POOL_FACTORY_H

#include "cv2x_lte-rrc-sap.h"
#include "cv2x_sl-pool-factory.h"

namespace ns3 {
  
/** Class to configure and generate resource pools */
class cv2x_SlV2xResourcePoolFactory : public  cv2x_SlPoolFactory 
{
public:
  cv2x_SlV2xResourcePoolFactory(); 

  /**
   * Creates a new resource pool based on the factory configuration
   * \return The new resource pool
   */
  cv2x_LteRrcSap::SlCommResourcePoolV2x CreatePool ();

  /**
   * Sets the alpha parameter for the power control algorithm in the shared channel
   * \param alpha The alpha parameter for the power control algorithm
   */
  void SetDataTxAlpha (double alpha);

  /**
   * Sets the p0 parameter for the power control algorithm in the shared channel
   * \param p0 The p0 parameter for the power control algorithm
   */  
  void SetDataTxP0 (int16_t p0);

  /**
   * Sets the SL OffsetIndicator 
   * \param slOffsetIndicator
   */ 
  void SetSlOffsetIndicator(uint16_t slOffsetIndicator);

  /**
   * Sets the SL subframe  bitmap of the resource pool, which is defined by repeating 
   * the bitmap within a SFN cycle (see TS 36.213)
   * \param slSubframe
   */ 
  void SetSlSubframe(std::bitset<20> slSubframe); 
  
  /**
   * Sets the adjacency parameter which indicates whether a UE shall always transmit 
   * PSCCH and PSSCH in adjacent RBs (indicated by TRUE) or in non-adjacent RBs
   * (indicated by FALSE) (see TS 36.213)
   * \param adjacencyPSCCHPSSCH
   */ 
  void SetAdjacencyPscchPssch(bool adjacencyPscchPssch);

  /**
   * Sets the subchannel size which indicates the number of PRBs of each subchannel in
   * the corresponding resource pool (see TS 36.213).The value n5 denotes 5 PRBs; 
   * n6 denotes 6 PRBs and so on. E-UTRAN configures values n5, n6, n10, n15, n20, n25, 
   * n50, n75 and n100 in the case of adjacencyPSCCH-PSSCH set to TRUE; otherwise, E-UTRAN 
   * configures values n4, n5, n6, n8, n9, n10, n12, n15, n16, n18, n20, n30, n48, n72 and 
   * n96 in the case of adjacencyPSCCH-PSSCH set to FALSE,
   * \param sizeSubchannel
   */ 
  void SetSizeSubchannel(uint16_t sizeSubchannel);

  /**
   * Sets the number of subchannels in the corresponding resource pool (see TS 36.213)
   * \param numSubchannel
   */ 
  void SetNumSubchannel(uint16_t numSubchannel);

  /**
   * Sets the lowest RB index of the subchannel with the lowest index (see TS 36.213)
   * \param startRbSubchannel
   */ 
  void SetStartRbSubchannel(uint16_t startRbSubchannel);

  /**
   * Sets the lowest RB index of tthe PSSCH pool (see TS 36.213). This field is absent when 
   * a pool is (pre)configured such that a UE always transmits SC and data in adjacent RBs
   * in the same subframe. 
   * \param startRbPscchPool
   */ 
  void SetStartRbPscchPool(uint16_t startRbPscchPool);

  /**
   * Sets the S-RSSI threshold for determing the contribution 
   * \param thresSRSSICBR
   */ 
  void SetThreshSrssiCbr(uint16_t thresSrssiCbr);

  /**
   * Sets the zone ID for which the UE shall use this resource pool. This field is absent 
   * in v2x-CommTxPoolExceptional, p2x-CommTxPoolNormalCommon and v2x-CommRxPool in SIB21 
   * or in mobilityControlInfoV2x.
   * \param zoneId
   */ 
  void SetZoneId(uint16_t zoneId);

  /** 
   * Indicates if the configuration is for a UE selected pool
   * \param ueSelected true if the connfiguration is for a UE selected pool 
   */
  void SetHaveUeSelectedResourceConfig (bool ueSelected);


private: 
  cv2x_LteRrcSap::SlCommResourcePoolV2x m_pool; 

protected: 

  bool m_ueSelected; //!< indicates if the pool if for UE selected mode
  double m_dataAlpha; //!< the alpha parameter for power control in the shared channel
  int16_t m_dataP0; //!< the p0 parameter for the power control in the shared channel

  // V2X specific parameter
  uint16_t m_slOffsetIndicator;             //!< indicates the offset of the first subframe of a resource pool
  std::bitset<20> m_slSubframe;             //!< determines PSSCH subframe pool: bitmap with acceptable sizes
  bool m_adjacencyPscchPssch;               //!< indicates if PSCCH and PSSCH are adjacent in the frequecy domain
  uint16_t m_sizeSubchannel;             //!< indicates the number of PRBs of each subchannel in the corresponding resource pool
  uint16_t m_numSubchannel;              //!< indicates the number of subchannels in the corresponding resource pool 
  uint16_t m_startRbSubchannel;              //!< indicates the lowest RB index of the subchannel with the lowest index
  uint16_t m_startRbPscchPool;               //!< indicates the lowest RB index of the PSCCH pool. This field is irrelevant if a UE always transmits control and data in adjacent RBs in the same subframe 
  uint16_t m_zoneId;                         //!< indicates the zone ID for which the UE shall use this resource pool
  uint16_t m_threshSrssiCbr;                 //!< indicates the S-RSSI threshold for determining the contribution of a sub-channel to the CBR measurement as specified in TS 36.214
  //uint8_t m_syncConfigIndex;              //!< indicates the synchronisation configuration that is associated with a reception pool, by means of an index to the corresponding entry of v2x-SyncConfig in SystemInformationblockType21 for V2X sidelink communication
  //std::string m_syncAllowedGnssSync;        //!< indicates the allowed the synchronization references for a transmission resource pool for V2X sidelink communication
  //std::string m_syncAllowedEnbSync;        
  //std::string m_syncAllowedUeSync;
  // TODO: add missing optional parameter

};

}

#endif

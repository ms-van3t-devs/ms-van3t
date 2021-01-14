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
 * 
 */

#ifndef CV2X_SL_PRECONFIG_POOL_FACTORY_H
#define CV2X_SL_PRECONFIG_POOL_FACTORY_H

#include "cv2x_lte-rrc-sap.h"
#include "cv2x_sl-resource-pool-factory.h"

namespace ns3 {

  /** Class to generate preconfigured pool configuration */
class cv2x_SlPreconfigPoolFactory : public cv2x_SlResourcePoolFactory
{
public:
  cv2x_SlPreconfigPoolFactory ();

  /**
   * Generate a preconfigure resource pool based on the factory configuration
   * \return a new preconfigured resource pool
   */
  cv2x_LteRrcSap::SlPreconfigCommPool CreatePool ();

private:
  cv2x_LteRrcSap::SlPreconfigCommPool m_pool;
};

} // namespace ns3

#endif

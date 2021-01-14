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
 */

#ifndef CV2X_LTE_SL_TFT_H
#define CV2X_LTE_SL_TFT_H


#include <ns3/simple-ref-count.h>
#include <ns3/ipv4-address.h>

#include <list>

namespace ns3 {

/**
 * 
 * \brief  Traffic flow template used by sidelink bearers.
 *
 * This class implements a variant of LTE TFT for sidelink
 *
 */
class cv2x_LteSlTft : public SimpleRefCount<cv2x_LteSlTft>
{
public:

  /**
   * Indicates the direction of the traffic that is to be classified. 
   */
  enum Direction {TRANSMIT = 1,
		  RECEIVE = 2,
		  BIDIRECTIONAL = 3};

  /**
   * Constructor
   * \param d The direction
   * \param groupIp The group IP address
   * \param groupL2 The group layer 2 address
   */
  cv2x_LteSlTft (Direction d, Ipv4Address groupIp, uint32_t groupL2);

  /**
   * Clone the TFT
   * \return a copy of this cv2x_LteSlTft
   */
  Ptr<cv2x_LteSlTft> Copy();
  
  /** 
   * Function to evaluate if the SL TFT matches the remote IP address
   * \param remoteAddress 
   * 
   * \return true if the TFT matches with the
   * parameters, false otherwise.
   */
  bool Matches (Ipv4Address remoteAddress);

  /**
   * Gets the Group L2 address associated with the TFT
   * \return the Group L2 address associated with the TFT
   */
  uint32_t GetGroupL2Address ();

  /**
   * Indicates if the TFT is for an incoming sidelink bearer
   * \return true if the TFT is for an incoming sidelink bearer
   */
  bool isReceive ();

  /**
   * Indicates if the TFT is for an outgoing sidelink bearer
   * \return true if the TFT is for an outgoing sidelink bearer
   */
  bool isTransmit ();
  
 private:

  
  
  Direction m_direction; /**< whether the filter needs to be applied
			  to uplink / downlink only, or in both cases*/
  
  Ipv4Address m_groupAddress;     /**< IPv4 address of the remote host  */
  uint32_t m_groupL2Address;      /** 24 bit MAC address  **/

};

} // namespace ns3

#endif /* CV2X_LTE_SL_TFT_H */

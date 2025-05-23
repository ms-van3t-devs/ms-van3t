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

#include "cv2x_lte-sl-tft.h"
#include "ns3/abort.h"
#include "ns3/log.h"

namespace ns3 {
  
  NS_LOG_COMPONENT_DEFINE ("cv2x_LteSlTft");

  cv2x_LteSlTft::cv2x_LteSlTft (Direction d, Ipv4Address groupIp, uint32_t groupL2)
  {
    NS_LOG_FUNCTION (this);
    m_direction = d;
    m_groupAddress = groupIp;
    NS_ASSERT_MSG ((groupL2 & 0xFF000000) == 0, "Group L2 address must be 24 bits");
    m_groupL2Address = groupL2;
  }

  Ptr<cv2x_LteSlTft>
  cv2x_LteSlTft::Copy ()
  {
    Ptr<cv2x_LteSlTft> tft = Create<cv2x_LteSlTft> (m_direction, m_groupAddress, m_groupL2Address);
    return tft;
  }
  
  /** 
   * 
   * \param remoteAddress 
   * 
   * \return true if the TFT matches with the
   * parameters, false otherwise.
   */
  bool
  cv2x_LteSlTft::Matches (Ipv4Address remoteAddress)
  {
    return remoteAddress == m_groupAddress;
  }


  uint32_t
  cv2x_LteSlTft::GetGroupL2Address ()
  {
    return m_groupL2Address;
  }

  bool
  cv2x_LteSlTft::isReceive ()
  {
    //receiving if RECEIVE or BIDIRECTIONAL
    return m_direction != cv2x_LteSlTft::TRANSMIT;
  }

  bool
  cv2x_LteSlTft::isTransmit ()
  {
    //transmitting if TRANSMIT or BIDIRECTIONAL
    return m_direction != cv2x_LteSlTft::RECEIVE;
  }

  
}

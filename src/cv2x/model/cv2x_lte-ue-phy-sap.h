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
 * Author: Marco Miozzo <mmiozzo@cttc.es>
 * Modified by: NIST
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */



#ifndef CV2X_LTE_UE_PHY_SAP_H
#define CV2X_LTE_UE_PHY_SAP_H

#include <ns3/packet.h>

namespace ns3 {

class cv2x_LteControlMessage;

/**
* Service Access Point (SAP) offered by the UE-PHY to the UE-MAC
*
* This is the PHY SAP Provider, i.e., the part of the SAP that contains
* the PHY methods called by the MAC
*/
class cv2x_LteUePhySapProvider
{
public:
  virtual ~cv2x_LteUePhySapProvider ();

  /**
  * \brief Send the MAC PDU to the channel
  * \param p the MAC PDU to send
  * \return true if
  */
  virtual void SendMacPdu (Ptr<Packet> p) = 0;

  /**
  * \brief Send SendLteControlMessage (PDCCH map, CQI feedbacks) using the ideal control channel
  * \param msg the Ideal Control Message to send
  */
  virtual void SendLteControlMessage (Ptr<cv2x_LteControlMessage> msg) = 0;

  /** 
   * send a preamble on the PRACH
   * 
   * \param prachId the ID of the preamble
   * \param raRnti the RA rnti
   */
  virtual void SendRachPreamble (uint32_t prachId, uint32_t raRnti) = 0;

};


/**
* Service Access Point (SAP) offered by the PHY to the MAC
*
* This is the PHY SAP User, i.e., the part of the SAP that contains the MAC
* methods called by the PHY
*/
class cv2x_LteUePhySapUser 
{
public:
  virtual ~cv2x_LteUePhySapUser ();


  /**
  * Called by the Phy to notify the MAC of the reception of a new PHY-PDU
  *
  * \param p
  */
  virtual void ReceivePhyPdu (Ptr<Packet> p) = 0;

  /**
  * \brief Trigger the start from a new frame (input from Phy layer)
  * \param frameNo frame number
  * \param subframeNo subframe number
  */
  virtual void SubframeIndication (uint32_t frameNo, uint32_t subframeNo) = 0;

  /**
  * \brief Receive SendLteControlMessage (PDCCH map, CQI feedbacks) using the ideal control channel
  * \param msg the Ideal Control Message to receive
  */
  virtual void ReceiveLteControlMessage (Ptr<cv2x_LteControlMessage> msg) = 0;

  /**
  * Notify the MAC that the PHY changed of timing as consequence of a change of SyncRef
  * Adjust the MAC subframe indication
  * \param frameNo the current PHY frame number
  * \param subframeNo the current PHY subframe number
  */
  virtual void NotifyChangeOfTiming (uint32_t frameNo, uint32_t subframeNo) = 0;

  /**
   * Pass the sensing information from PHY to MAC
   * \param frameNo the current PHY frame number
   * \param subframeNo the current PHY subframe number
   * \param rbstart the start resource block 
   * \param rbLen the length of transmitted data
   * \param rsrpVal the measured RSRP value over the used resource blocks
   */
  virtual void PassSensingData (uint32_t frameNo, uint32_t subframeNo, uint16_t pRsvp, uint8_t rbStart, uint8_t rbLen, uint8_t prio, double slRsrp, double slRssi) = 0;
};


} // namespace ns3


#endif // CV2X_LTE_UE_PHY_SAP_H

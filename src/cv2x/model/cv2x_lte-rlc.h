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

#ifndef CV2X_LTE_RLC_H
#define CV2X_LTE_RLC_H

#include <ns3/simple-ref-count.h>
#include <ns3/packet.h>
#include "ns3/uinteger.h"
#include "ns3/traced-value.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/nstime.h"

#include "ns3/object.h"

#include "ns3/cv2x_lte-rlc-sap.h"
#include "ns3/cv2x_lte-mac-sap.h"

namespace ns3 {


// class cv2x_LteRlcSapProvider;
// class cv2x_LteRlcSapUser;
// 
// class cv2x_LteMacSapProvider;
// class cv2x_LteMacSapUser;

/**
 * This abstract base class defines the API to interact with the Radio Link Control
 * (LTE_RLC) in LTE, see 3GPP TS 36.322
 *
 */
class cv2x_LteRlc : public Object // SimpleRefCount<cv2x_LteRlc>
{
  /// allow cv2x_LteRlcSpecificLteMacSapUser class friend access
  friend class cv2x_LteRlcSpecificLteMacSapUser;
  /// allow cv2x_LteRlcSpecificLteRlcSapProvider<cv2x_LteRlc> class friend access
  friend class cv2x_LteRlcSpecificLteRlcSapProvider<cv2x_LteRlc>;
public:
  cv2x_LteRlc ();
  virtual ~cv2x_LteRlc ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  /**
   *
   *
   * \param rnti
   */
  void SetRnti (uint16_t rnti);

  /**
   *
   *
   * \param lcId
   */
  void SetLcId (uint8_t lcId);

  /**
   * Sets the source L2 Id for sidelink identification of the PDCP entity
   * \param src
   */
  void SetSourceL2Id (uint32_t src);
  
  /**
   * Sets the destination L2 Id for sidelink identification of the PDCP entity
   * \param src
   */
  void SetDestinationL2Id (uint32_t dst);

  /**
   *
   *
   * \param s the RLC SAP user to be used by this LTE_RLC
   */
  void SetLteRlcSapUser (cv2x_LteRlcSapUser * s);

  /**
   *
   *
   * \return the RLC SAP Provider interface offered to the PDCP by this LTE_RLC
   */
  cv2x_LteRlcSapProvider* GetLteRlcSapProvider ();

  /**
   *
   *
   * \param s the MAC SAP Provider to be used by this LTE_RLC
   */
  void SetLteMacSapProvider (cv2x_LteMacSapProvider * s);

  /**
   *
   *
   * \return the MAC SAP User interface offered to the MAC by this LTE_RLC
   */
  cv2x_LteMacSapUser* GetLteMacSapUser ();


  /**
   * TracedCallback signature for NotifyTxOpportunity events.
   *
   * \param [in] rnti C-RNTI scheduled.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] bytes The number of bytes to transmit
   */
  typedef void (* NotifyTxTracedCallback)
    (uint16_t rnti, uint8_t lcid, uint32_t bytes);

  /**
   * TracedCallback signature for
   *
   * \param [in] rnti C-RNTI scheduled.
   * \param [in] lcid The logical channel id corresponding to
   *             the sending RLC instance.
   * \param [in] bytes The packet size.
   * \param [in] delay Delay since sender timestamp, in ns.
   */
  typedef void (* ReceiveTracedCallback)
    (uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t delay);

  /// \todo MRE What is the sense to duplicate all the interfaces here???
  // NB to avoid the use of multiple inheritance
  
protected:
  // Interface forwarded by cv2x_LteRlcSapProvider
  /**
   * Transmit PDCP PDU
   * 
   * \param p packet
   */
  virtual void DoTransmitPdcpPdu (Ptr<Packet> p) = 0;

  cv2x_LteRlcSapUser* m_rlcSapUser; ///< RLC SAP user
  cv2x_LteRlcSapProvider* m_rlcSapProvider; ///< RLC SAP provider

  // Interface forwarded by cv2x_LteMacSapUser
  /**
   * Notify transmit opportunity
   *
   * \param bytes number of bytes
   * \param layer the layer
   * \param harqId the HARQ ID
   * \param componentCarrierId component carrier ID
   * \param rnti the RNTI
   * \param lcid the LCID
   */ 
  virtual void DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid) = 0;
  /**
   * Notify HARQ delivery failure
   */ 
  virtual void DoNotifyHarqDeliveryFailure () = 0;
  /**
   * Receive PDU function
   *
   * \param p the packet
   * \param rnti the RNTI
   * \param lcid the LCID
   */ 
  virtual void DoReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid) = 0;

  cv2x_LteMacSapUser* m_macSapUser; ///< MAC SAP user
  cv2x_LteMacSapProvider* m_macSapProvider; ///< MAC SAP provider

  uint16_t m_rnti; ///< RNTI
  uint8_t m_lcid; ///< LCID
  /* Additional identifier for sidelink */
  uint32_t m_srcL2Id; ///< Source L2 ID (24 bits)
  uint32_t m_dstL2Id; ///< Destination L2 ID (24 bits)

  /**
   * Used to inform of a PDU delivery to the MAC SAP provider
   */
  TracedCallback<uint16_t, uint8_t, uint32_t> m_txPdu;
  /**
   * Used to inform of a PDU reception from the MAC SAP user
   */
  TracedCallback<uint16_t, uint8_t, uint32_t, uint64_t> m_rxPdu;

};



/**
 * LTE_RLC Saturation Mode (SM): simulation-specific mode used for
 * experiments that do not need to consider the layers above the LTE_RLC.
 * The LTE_RLC SM, unlike the standard LTE_RLC modes, it does not provide
 * data delivery services to upper layers; rather, it just generates a
 * new LTE_RLC PDU whenever the MAC notifies a transmission opportunity.
 *
 */
class cv2x_LteRlcSm : public cv2x_LteRlc
{
public:
  cv2x_LteRlcSm ();
  virtual ~cv2x_LteRlcSm ();
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual void DoInitialize ();
  virtual void DoDispose ();

  virtual void DoTransmitPdcpPdu (Ptr<Packet> p);
  virtual void DoNotifyTxOpportunity (uint32_t bytes, uint8_t layer, uint8_t harqId, uint8_t componentCarrierId, uint16_t rnti, uint8_t lcid);
  virtual void DoNotifyHarqDeliveryFailure ();
  virtual void DoReceivePdu (Ptr<Packet> p, uint16_t rnti, uint8_t lcid);



private:
  /// Report buffer status
  void ReportBufferStatus ();

};




// /**
//  * Implements LTE_RLC Transparent Mode (TM), see  3GPP TS 36.322
//  *
//  */
// class cv2x_LteRlcTm : public cv2x_LteRlc
// {
// public:
//   virtual ~cv2x_LteRlcTm ();

// };


// /**
//  * Implements LTE_RLC Unacknowledged Mode (UM), see  3GPP TS 36.322
//  *
//  */
// class cv2x_LteRlcUm : public cv2x_LteRlc
// {
// public:
//   virtual ~cv2x_LteRlcUm ();

// };

// /**
//  * Implements LTE_RLC Acknowledged Mode (AM), see  3GPP TS 36.322
//  *
//  */

// class cv2x_LteRlcAm : public cv2x_LteRlc
// {
// public:
//   virtual ~cv2x_LteRlcAm ();
// };





} // namespace ns3

#endif // CV2X_LTE_RLC_H

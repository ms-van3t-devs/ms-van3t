/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef NR_UE_NET_DEVICE_H
#define NR_UE_NET_DEVICE_H

#include "nr-net-device.h"

namespace ns3 {

class Packet;
class PacketBurst;
class Node;
class NrUePhy;
class NrUeMac;
class LteUeComponentCarrierManager;
class EpcUeNas;
class LteUeRrc;
class NrGnbNetDevice;
class BandwidthPartUe;
class BwpManagerUe;

/**
 * \ingroup ue
 * \brief The User Equipment NetDevice
 *
 * This class represent the netdevice of the UE. This class is the contact
 * point between the TCP/IP part (from internet and network modules) and the
 * NR part.
 */
class NrUeNetDevice : public NrNetDevice
{
public:
  /**
   * \brief GetTypeId
   * \return
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrUeNetDevice
   */
  NrUeNetDevice (void);

  /**
   * \brief ~NrUeNetDevice
   */
  virtual ~NrUeNetDevice (void);

  /**
   * \brief GetCsgId ?
   * \return ?
   */
  uint32_t GetCsgId () const;

  /**
   * \brief SetCsgId ?
   * \param csgId ?
   */
  void SetCsgId (uint32_t csgId);

  /**
   * \brief Obtain a pointer to the PHY at the index specified
   * \param index bandwidth part index
   * \return the pointer to the PHY selected
   */
  Ptr<NrUePhy> GetPhy (uint8_t index) const;

  /**
   * \brief Obtain a pointer to the MAC at the index specified
   * \param index bandwidth part index
   * \return the pointer to the MAC selected
   */
  Ptr<NrUeMac> GetMac (uint8_t index) const;

  /**
   * \brief Get the bandwidth part manager
   * \return a pointer to the BWP manager
   */
  Ptr<BwpManagerUe> GetBwpManager (void) const;

  /**
   * \brief Get the Imsi
   * \return UE imsi
   */
  uint64_t GetImsi () const;

  /**
   * \brief Get the CellId
   * \return cell ID
   */
  uint16_t GetCellId () const;

  /**
   * \brief Get a pointer to the Nas
   * \return the NAS pointer
   */
  Ptr<EpcUeNas> GetNas (void) const;

  /**
   * \brief Get a Rrc pointer
   * \return RRC pointer
   */
  Ptr<LteUeRrc> GetRrc () const;

  /**
   * \brief Set the GNB to which this UE is attached to
   * \param enb GNB to attach to
   *
   * This method may change once we implement handover.
   */
  void SetTargetEnb (Ptr<NrGnbNetDevice> enb);

  /**
   * \brief Obtain a pointer to the target enb
   * \return a pointer to the target enb
   */
  Ptr<const NrGnbNetDevice> GetTargetEnb (void) const;

  /**
   * \brief Set the ComponentCarrier Map for the UE
   * \param ccm the map of ComponentCarrierUe
   */
  void SetCcMap (std::map< uint8_t, Ptr<BandwidthPartUe> > ccm);

  /**
   * \brief Get the ComponentCarrier Map for the UE
   * \returns the map of ComponentCarrierUe
   */
  std::map< uint8_t, Ptr<BandwidthPartUe> >  GetCcMap (void);

  /**
   * \brief Get the size of the component carriers map
   * \return the number of cc that we have
   */
  uint32_t GetCcMapSize () const;

  /**
   * \brief Spectrum has calculated the HarqFeedback for one DL transmission,
   * and give it to the NetDevice of the UE.
   *
   * The NetDevice find the best BWP to forward the Harq Feedback, and then
   * forward it to the PHY of the selected BWP.
   *
   * \param m feedback
   */
  void EnqueueDlHarqFeedback (const DlHarqInfo &m) const;

  /**
   * \brief The UE received a CTRL message list.
   *
   * The UE should divide the messages to the BWP they pertain to.
   *
   * \param msgList Message list
   * \param sourceBwpId BWP Id from which the list originated
   */
  void RouteIngoingCtrlMsgs (const std::list<Ptr<NrControlMessage> > &msgList, uint8_t sourceBwpId);

  /**
   * \brief Route the outgoing messages to the right BWP
   * \param msgList the list of messages
   * \param sourceBwpId the source bwp of the messages
   */
  void RouteOutgoingCtrlMsgs (const std::list<Ptr<NrControlMessage> > &msgList, uint8_t sourceBwpId);

  /**
   * \brief Update the RRC config. Must be called only once.
   */
  void UpdateConfig (void);

protected:
  // inherited from Object
  virtual void DoInitialize (void);
  virtual void DoDispose ();

  // inherited from NetDevice
  virtual bool DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

private:
  Ptr<NrGnbNetDevice> m_targetEnb;   //!< GNB pointer
  Ptr<LteUeRrc> m_rrc;                   //!< RRC pointer
  Ptr<EpcUeNas> m_nas;                   //!< NAS pointer
  uint64_t m_imsi;                       //!< UE IMSI
  uint32_t m_csgId;                      //!< ?_?

  std::map < uint8_t, Ptr<BandwidthPartUe> > m_ccMap; ///< component carrier map
  Ptr<LteUeComponentCarrierManager> m_componentCarrierManager; ///< the component carrier manager

};

}
#endif /* NR_UE_NET_DEVICE_H */

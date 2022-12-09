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

#ifndef NR_ENB_NET_DEVICE_H
#define NR_ENB_NET_DEVICE_H

#include "nr-net-device.h"

namespace ns3 {

class Packet;
class PacketBurst;
class Node;
class NrGnbPhy;
class NrGnbMac;
class LteEnbRrc;
class BandwidthPartGnb;
class LteEnbComponentCarrierManager;
class BwpManagerGnb;
class NrMacScheduler;

/**
 * \ingroup gnb
 * \brief The NrGnbNetDevice class
 *
 * This class represent the GNB NetDevice.
 */
class NrGnbNetDevice : public NrNetDevice
{
public:
  static TypeId GetTypeId (void);

  NrGnbNetDevice ();

  virtual ~NrGnbNetDevice (void);

  Ptr<NrMacScheduler> GetScheduler (uint8_t index) const;

  Ptr<NrGnbMac> GetMac (uint8_t index) const;

  Ptr<NrGnbPhy> GetPhy (uint8_t index) const;

  Ptr<BwpManagerGnb> GetBwpManager () const;

  uint16_t GetBwpId (uint8_t index) const;

  /**
   * \return the cell id
   */
  uint16_t GetCellId () const;

  /**
   * \return the cell ids belonging to this gNB
   */
  std::vector<uint16_t> GetCellIds () const;

  /**
   * \brief Set this gnb cell id
   * \param cellId the cell id
   */
  void SetCellId (uint16_t cellId);

  uint16_t GetEarfcn (uint8_t index) const;

  void SetRrc (Ptr<LteEnbRrc> rrc);

  Ptr<LteEnbRrc> GetRrc (void);

  void SetCcMap (const std::map<uint8_t, Ptr<BandwidthPartGnb> > &ccm);

  /**
   * \brief Get the size of the component carriers map
   * \return the number of cc that we have
   */
  uint32_t GetCcMapSize () const;

  /**
   * \brief The gNB received a CTRL message list.
   *
   * The gNB should divide the messages to the BWP they pertain to.
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
  void UpdateConfig ();

protected:
  virtual void DoInitialize (void);

  virtual void DoDispose (void);
  virtual bool DoSend (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);

private:
  Ptr<LteEnbRrc> m_rrc;

  uint16_t m_cellId; //!< Cell ID. Set by the helper.

  std::map<uint8_t, Ptr<BandwidthPartGnb> > m_ccMap; /**< ComponentCarrier map */

  Ptr<LteEnbComponentCarrierManager> m_componentCarrierManager; ///< the component carrier manager of this eNb

};

}

#endif /* NR_ENB_NET_DEVICE_H */

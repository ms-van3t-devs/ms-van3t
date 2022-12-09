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
#ifndef BWPMANAGERUE_H
#define BWPMANAGERUE_H

#include <ns3/simple-ue-component-carrier-manager.h>
#include <ns3/nr-phy-mac-common.h>

namespace ns3 {

class BwpManagerAlgorithm;
class NrControlMessage;

/**
 * \ingroup ue-bwp
 * \brief The BwpManagerUe class
 */
class BwpManagerUe : public SimpleUeComponentCarrierManager
{
public:
  /**
   * \brief GetTypeId
   * \return the type id for the object
   */
  static TypeId GetTypeId ();

  /**
   * \brief BwpManagerUe constructor
   */
  BwpManagerUe ();
  /**
   * \brief ~BwpManagerUe
   */
  virtual ~BwpManagerUe () override;

  /**
   * \brief Set the algorithm
   * \param algorithm pointer to the algorithm
   */
  void SetBwpManagerAlgorithm (const Ptr<BwpManagerAlgorithm> &algorithm);

  /**
   * \brief The UE received a HARQ feedback from spectrum. Where this feedback
   * should be forwarded?
   *
   * \param m the feedback
   * \return the BWP index in which the feedback can be transmitted to the gNB.
   */
  uint8_t RouteDlHarqFeedback (const DlHarqInfo &m) const;

  /**
   * \brief Decide the BWP for the control message received.
   * \param msg Message
   * \param sourceBwpId BWP Id from which this message come from.
   *
   * The routing is made following the bandwidth part reported in the message.
   *
   * \return the BWP Id to which this message should be routed to.
   */
  uint8_t RouteIngoingCtrlMsg (const Ptr<NrControlMessage> & msg, uint8_t sourceBwpId) const;

  /**
   * \brief Route the outgoing messages to the right BWP
   * \param msg the message
   * \param sourceBwpId the source bwp of the message
   *
   * The routing is made by following the mapping provided through the function
   * SetOutputLink. If no mapping has been installed, or if the sourceBwpId
   * provided is not in the mapping, then forward the message back to the
   * originating BWP.
   *
   * \see SetOutputLink
   *
   * \return the bwp to which the ctrl messages should be redirected
   */
  uint8_t RouteOutgoingCtrlMsg (const Ptr<NrControlMessage> &msg, uint8_t sourceBwpId) const;

  /**
   * \brief Set a mapping between two BWP.
   * \param sourceBwp The messages that comes from this value...
   * \param outputBwp ... will get routed in this bandwidth part.
   *
   * Call it for each mapping you want to install.
   */
  void SetOutputLink (uint32_t sourceBwp, uint32_t outputBwp);

protected:
  virtual void DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params) override;
  virtual std::vector<LteUeCcmRrcSapProvider::LcsConfig> DoAddLc (uint8_t lcId,  LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu) override;
  virtual LteMacSapUser* DoConfigureSignalBearer (uint8_t lcId,  LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser* msu) override;
  Ptr<BwpManagerAlgorithm> m_algorithm;

private:
  std::unordered_map<uint8_t, EpsBearer::Qci> m_lcToBearerMap; //!< Map from LCID to bearer ID

  std::unordered_map <uint32_t, uint32_t> m_outputLinks; //!< Mapping between BWP.
};

} // namespace ns3
#endif // BWPMANAGERUE_H

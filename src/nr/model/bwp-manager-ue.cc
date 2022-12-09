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
#include "bwp-manager-ue.h"
#include "bwp-manager-algorithm.h"
#include <ns3/log.h>
#include <ns3/pointer.h>
#include "nr-control-messages.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("BwpManagerUe");
NS_OBJECT_ENSURE_REGISTERED (BwpManagerUe);

BwpManagerUe::BwpManagerUe () : SimpleUeComponentCarrierManager ()
{
  NS_LOG_FUNCTION (this);
}

BwpManagerUe::~BwpManagerUe ()
{
  NS_LOG_FUNCTION (this);
}

void
BwpManagerUe::SetBwpManagerAlgorithm(const Ptr<BwpManagerAlgorithm> &algorithm)
{
  NS_LOG_FUNCTION (this);
  m_algorithm = algorithm;
}


TypeId
BwpManagerUe::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::BwpManagerUe")
    .SetParent<SimpleUeComponentCarrierManager> ()
    .SetGroupName ("nr")
    .AddConstructor<BwpManagerUe> ()
    .AddAttribute ("BwpManagerAlgorithm",
                   "The algorithm pointer",
                   PointerValue (),
                   MakePointerAccessor (&BwpManagerUe::m_algorithm),
                   MakePointerChecker <BwpManagerAlgorithm> ())
  ;
  return tid;
}

void
BwpManagerUe::DoReportBufferStatus (LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_algorithm != nullptr);

  uint8_t bwpIndex = m_algorithm->GetBwpForEpsBearer (m_lcToBearerMap.at (params.lcid));

  NS_LOG_DEBUG ("BSR of size " << params.txQueueSize << " from RLC for LCID = " <<
                static_cast<uint32_t> (params.lcid) << " traffic type " <<
                m_lcToBearerMap.at (params.lcid) << " reported to CcId " <<
                static_cast<uint32_t> (bwpIndex));

  m_componentCarrierLcMap.at (bwpIndex).at (params.lcid)->ReportBufferStatus (params);
}

std::vector<LteUeCcmRrcSapProvider::LcsConfig>
BwpManagerUe::DoAddLc (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser *msu)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("For LC ID " << static_cast<uint32_t> (lcId) << " bearer qci " <<
               static_cast<uint32_t> (lcConfig.priority) <<
               " from priority " << static_cast<uint32_t> (lcConfig.priority));

  // see lte-enb-rrc.cc:453
  m_lcToBearerMap.insert (std::make_pair (lcId, static_cast<EpsBearer::Qci> (lcConfig.priority)));

  return SimpleUeComponentCarrierManager::DoAddLc (lcId, lcConfig, msu);
}

LteMacSapUser
*BwpManagerUe::DoConfigureSignalBearer (uint8_t lcId, LteUeCmacSapProvider::LogicalChannelConfig lcConfig, LteMacSapUser *msu)
{
  NS_LOG_FUNCTION (this);

  // Ignore signaling bearers for the moment. These are for an advanced use.
  // m_lcToBearerMap.insert (std::make_pair (lcId, EpsBearer::FromPriority (lcConfig.priority).qci));

  return SimpleUeComponentCarrierManager::DoConfigureSignalBearer (lcId, lcConfig, msu);
}

uint8_t
BwpManagerUe::RouteDlHarqFeedback (const DlHarqInfo &m) const
{
  NS_LOG_FUNCTION (this);

  return m.m_bwpIndex;
}

void
BwpManagerUe::SetOutputLink(uint32_t sourceBwp, uint32_t outputBwp)
{
  NS_LOG_FUNCTION (this);
  m_outputLinks.insert (std::make_pair (sourceBwp, outputBwp));
}

uint8_t
BwpManagerUe::RouteOutgoingCtrlMsg (const Ptr<NrControlMessage> &msg, uint8_t sourceBwpId) const
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Msg type " << msg->GetMessageType () << " that wants to go out from UE");

  if (m_outputLinks.empty ())
    {
      NS_LOG_INFO ("No linked BWP, routing outgoing msg to the source: " << +sourceBwpId);
      return sourceBwpId;
    }

  auto it = m_outputLinks.find (sourceBwpId);
  if (it == m_outputLinks.end ())
    {
      NS_LOG_INFO ("Source BWP not in the map, routing outgoing msg to itself: " << +sourceBwpId);
      return sourceBwpId;
    }

  NS_LOG_INFO ("routing outgoing msg to bwp: " << +it->second);
  return it->second;
}

uint8_t
BwpManagerUe::RouteIngoingCtrlMsg (const Ptr<NrControlMessage> &msg, uint8_t sourceBwpId) const
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("Msg type " << msg->GetMessageType () << " comes from BWP " <<
               +sourceBwpId << " that wants to go in the UE, goes in BWP " << msg->GetSourceBwp ());
  return msg->GetSourceBwp ();
}

} // namespace ns3

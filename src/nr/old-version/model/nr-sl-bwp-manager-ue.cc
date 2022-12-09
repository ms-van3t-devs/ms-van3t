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

#include "nr-sl-bwp-manager-ue.h"

#include <ns3/log.h>
#include "bwp-manager-algorithm.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrSlBwpManagerUe");
NS_OBJECT_ENSURE_REGISTERED (NrSlBwpManagerUe);

TypeId
NrSlBwpManagerUe::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::NrSlBwpManagerUe")
    .SetParent<BwpManagerUe> ()
    .SetGroupName ("nr")
    .AddConstructor<NrSlBwpManagerUe> ()
  ;
  return tid;
}

NrSlBwpManagerUe::NrSlBwpManagerUe () : BwpManagerUe ()
{
  NS_LOG_FUNCTION (this);
  m_nrSlUeBwpmRrcSapProvider = new MemberNrSlUeBwpmRrcSapProvider<NrSlBwpManagerUe> (this);
  m_nrSlBwpmUeMacSapProvider = new MemberNrSlMacSapProvider <NrSlBwpManagerUe> (this);
  m_nrSlBwpmUeMacSapUser = new MemberNrSlMacSapUser <NrSlBwpManagerUe> (this);
}


NrSlBwpManagerUe::~NrSlBwpManagerUe ()
{
  NS_LOG_FUNCTION (this);
}


void
NrSlBwpManagerUe::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  delete m_nrSlUeBwpmRrcSapProvider;
  delete m_nrSlBwpmUeMacSapProvider;
  delete m_nrSlBwpmUeMacSapUser;
  BwpManagerUe::DoDispose ();
}




NrSlUeBwpmRrcSapProvider*
NrSlBwpManagerUe::GetNrSlUeBwpmRrcSapProvider () const
{
  return m_nrSlUeBwpmRrcSapProvider;
}

void
NrSlBwpManagerUe::SetNrSlUeBwpmRrcSapUser (NrSlUeBwpmRrcSapUser* nrSlUeBwpmRrcSapUser)
{
  m_nrSlUeBwpmRrcSapUser = nrSlUeBwpmRrcSapUser;
}


void
NrSlBwpManagerUe::DoTransmitNrSlRlcPdu (const NrSlMacSapProvider::NrSlRlcPduParameters &params)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("NR SL BWP manager forwarding SL RLC PDU to MAC");
  std::map<uint8_t, std::map<NrSlUeBwpLcIdentifier, NrSlMacSapProvider*> >::iterator it;
  it =  m_nrSlBwpLcMap.find (params.bwpId);
  NS_ABORT_MSG_IF (it == m_nrSlBwpLcMap.end (), "could not find Sap for Sidelink BWP "
                   << +params.bwpId);
  NrSlUeBwpLcIdentifier slLcIdentifier;
  slLcIdentifier.lcId = params.lcid;
  slLcIdentifier.srcL2Id = params.srcL2Id;
  slLcIdentifier.dstL2Id = params.dstL2Id;
  std::map<NrSlUeBwpLcIdentifier, NrSlMacSapProvider*>::iterator itSlLc;
  //PDU is forwarded to the MAC of respective BWP/CC
  itSlLc = it->second.find (slLcIdentifier);
  NS_ASSERT_MSG (itSlLc != it->second.end (), "could not find Sap for SL LC id " << +params.lcid);
  itSlLc->second->TransmitNrSlRlcPdu (params);
}

void
NrSlBwpManagerUe::DoReportNrSlBufferStatus (const NrSlMacSapProvider::NrSlReportBufferStatusParameters &params)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_algorithm != nullptr);
  NrSlUeBwpLcIdentifier slLcIdentifier;
  slLcIdentifier.lcId = params.lcid;
  slLcIdentifier.srcL2Id = params.srcL2Id;
  slLcIdentifier.dstL2Id = params.dstL2Id;

  std::map<NrSlUeBwpLcIdentifier, EpsBearer::Qci>::iterator itQci;
  itQci = m_slLcToBearerMap.find (slLcIdentifier);
  NS_ABORT_MSG_IF (itQci == m_slLcToBearerMap.end (), "could not find QCI for Sidelink LC id " << +slLcIdentifier.lcId);
  uint8_t bwpIndex = m_algorithm->GetBwpForEpsBearer (itQci->second);

  NS_LOG_DEBUG ("BSR of size " << params.txQueueSize << " from RLC for LCID = " <<
                static_cast<uint16_t> (params.lcid) << " traffic type " <<
                m_slLcToBearerMap.at (slLcIdentifier) << " reported to BwpId " <<
                static_cast<uint16_t> (bwpIndex));

  std::map<uint8_t, std::map<NrSlUeBwpLcIdentifier, NrSlMacSapProvider*> >::iterator it;
  it =  m_nrSlBwpLcMap.find (bwpIndex);
  NS_ABORT_MSG_IF (it == m_nrSlBwpLcMap.end (), "could not find BWP id " << +bwpIndex);

  std::map<NrSlUeBwpLcIdentifier, NrSlMacSapProvider*>::iterator itSlLc;
  itSlLc = it->second.find (slLcIdentifier);
  NS_ASSERT_MSG (itSlLc != it->second.end (), "could not find Sap for SL LC id " << +params.lcid);
  itSlLc->second->ReportNrSlBufferStatus (params);
}


void
NrSlBwpManagerUe::DoNotifyNrSlTxOpportunity (const NrSlMacSapUser::NrSlTxOpportunityParameters &txOpParams)
{
  NS_LOG_FUNCTION (this);

  NrSlUeBwpLcIdentifier slLcIdentifier;
  slLcIdentifier.lcId = txOpParams.lcid;
  slLcIdentifier.srcL2Id = txOpParams.srcL2Id;
  slLcIdentifier.dstL2Id = txOpParams.dstL2Id;

  std::map <NrSlUeBwpLcIdentifier, NrSlMacSapUser*>::iterator slLcIdIt = m_nrSlDrbLcInfoMap.find (slLcIdentifier);

  NS_ABORT_MSG_IF (slLcIdIt == m_nrSlDrbLcInfoMap.end (), "cannot find SL LCID " << +txOpParams.lcid
                                                                                 << ", srcL2Id " << txOpParams.srcL2Id << ", dstL2Id " << txOpParams.dstL2Id);

  NS_LOG_DEBUG (this << " SL lcid = " << +txOpParams.lcid << " layer= "
                     << +txOpParams.layer << " componentCarierId " << +txOpParams.bwpId
                     << " rnti " << txOpParams.rnti);

  NS_LOG_DEBUG (this << " MAC is asking BWP id = " << +txOpParams.bwpId
                     << " with SL lcid = " << +txOpParams.lcid << " to transmit " << txOpParams.bytes << " bytes");

  slLcIdIt->second->NotifyNrSlTxOpportunity (txOpParams);
}
void
NrSlBwpManagerUe::DoReceiveNrSlRlcPdu (NrSlMacSapUser::NrSlReceiveRlcPduParameters rxPduParams)
{
  NS_LOG_FUNCTION (this);

  NrSlUeBwpLcIdentifier slLcIdentifier;
  slLcIdentifier.lcId = rxPduParams.lcid;
  slLcIdentifier.srcL2Id = rxPduParams.srcL2Id;
  slLcIdentifier.dstL2Id = rxPduParams.dstL2Id;

  std::map <NrSlUeBwpLcIdentifier, NrSlMacSapUser*>::iterator slLcIdIt = m_nrSlDrbLcInfoMap.find (slLcIdentifier);

  NS_ABORT_MSG_IF (slLcIdIt == m_nrSlDrbLcInfoMap.end (), "cannot find SL LCID " << +rxPduParams.lcid
                                                                                 << ", srcL2Id " << rxPduParams.srcL2Id << ", dstL2Id " << rxPduParams.dstL2Id);
  slLcIdIt->second->ReceiveNrSlRlcPdu (rxPduParams);
}



///////////////////////////////////////////////////////////
// NR SL UE BWP Manager RRC SAP PROVIDER SAP forwarders
///////////////////////////////////////////////////////////

std::vector<NrSlUeBwpmRrcSapProvider::SlLcInfoBwpm>
NrSlBwpManagerUe::DoAddNrSlDrbLc (const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo &lcInfo, NrSlMacSapUser* msu)
{
  NS_LOG_FUNCTION (this);

  //SL DRB LC starts from 4
  NS_ASSERT_MSG (lcInfo.lcId > 3, "Hey! I can only add the LC for data radio bearers.");
  std::vector<NrSlUeBwpmRrcSapProvider::SlLcInfoBwpm> res;
  NrSlUeBwpLcIdentifier slLcIdentifier;
  slLcIdentifier.lcId = lcInfo.lcId;
  slLcIdentifier.srcL2Id = lcInfo.srcL2Id;
  slLcIdentifier.dstL2Id = lcInfo.dstL2Id;

  NS_LOG_DEBUG ("UE CCM Adding SL LcId = " << +lcInfo.lcId << " srcL2Id = " << lcInfo.srcL2Id << " dstL2Id = " << lcInfo.dstL2Id);

  NS_ASSERT_MSG (m_nrSlDrbLcInfoMap.find (slLcIdentifier) == m_nrSlDrbLcInfoMap.end (), "cannot add channel because LCID " << +lcInfo.lcId
                                                                                                                           << ", srcL2Id " << lcInfo.srcL2Id << ", dstL2Id " << lcInfo.dstL2Id << " is already present");

  m_nrSlDrbLcInfoMap.insert (std::make_pair (slLcIdentifier, msu));

  m_slLcToBearerMap.insert (std::make_pair (slLcIdentifier, static_cast<EpsBearer::Qci> (lcInfo.pqi)));


  NrSlUeBwpmRrcSapProvider::SlLcInfoBwpm elem;
  std::map <uint8_t, std::map<NrSlUeBwpLcIdentifier, NrSlMacSapProvider*> >::iterator slCcLcMapIt;
  NS_ASSERT_MSG (m_slBwpIds.size () != 0, "Did you forget to set BWP container from RRC");
  for (const auto &itBwpIt : m_slBwpIds)
    {
      elem.bwpId =  itBwpIt;
      elem.lcInfo = lcInfo;
      elem.msu = m_nrSlBwpmUeMacSapUser;
      res.insert (res.end (), elem);

      slCcLcMapIt = m_nrSlBwpLcMap.find (elem.bwpId);
      if (slCcLcMapIt != m_nrSlBwpLcMap.end ())
        {
          slCcLcMapIt->second.insert (std::make_pair (slLcIdentifier, m_nrSlMacSapProvidersMap.at (elem.bwpId)));
        }
      else
        {
          std::map<NrSlUeBwpLcIdentifier, NrSlMacSapProvider*> empty;
          std::pair <std::map <uint8_t, std::map<NrSlUeBwpLcIdentifier, NrSlMacSapProvider*> >::iterator, bool> ret;
          ret = m_nrSlBwpLcMap.insert (std::make_pair (elem.bwpId, empty));
          ret.first->second.insert (std::make_pair (slLcIdentifier, m_nrSlMacSapProvidersMap.at (elem.bwpId)));
        }
    }
  return res;
}

std::vector<uint8_t>
NrSlBwpManagerUe::DoRemoveNrSlDrbLc (uint8_t slLcId, uint32_t srcL2Id, uint32_t dstL2Id)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG (slLcId > 3, "Hey! I can delete only the LC for data radio bearers.");
  std::vector<uint8_t> res;

  //block RLC to reach MAC
  NrSlUeBwpLcIdentifier slLcIdentifier;
  slLcIdentifier.lcId = slLcId;
  slLcIdentifier.srcL2Id = srcL2Id;
  slLcIdentifier.dstL2Id = dstL2Id;

  std::map<uint8_t, std::map<NrSlUeBwpLcIdentifier, NrSlMacSapProvider*> >::iterator it = m_nrSlBwpLcMap.begin ();
  std::map<NrSlUeBwpLcIdentifier, NrSlMacSapProvider*>::iterator itSlLc;

  while (it != m_nrSlBwpLcMap.end ())
    {
      itSlLc = it->second.find (slLcIdentifier);
      if (itSlLc != it->second.end () && itSlLc->first.lcId > 3) //SL DRB LC starts from 4
        {
          res.push_back (it->first);
          it->second.erase (itSlLc);
        }
      else
        {
          it++;
        }
    }

  //block MAC to reach RLC
  std::map <NrSlUeBwpLcIdentifier, NrSlMacSapUser*>::iterator itToRlc;
  itToRlc = m_nrSlDrbLcInfoMap.find (slLcIdentifier);
  if (itToRlc != m_nrSlDrbLcInfoMap.end () && itToRlc->first.lcId > 3) //SL DRB LC starts from 4
    {
      m_nrSlDrbLcInfoMap.erase (itToRlc);
    }

  return res;
}


void
NrSlBwpManagerUe::DoResetNrSlDrbLcMap ()
{
  NS_LOG_FUNCTION (this);
  m_nrSlDrbLcInfoMap.erase (m_nrSlDrbLcInfoMap.begin (), m_nrSlDrbLcInfoMap.end ());
  m_nrSlBwpLcMap.erase (m_nrSlBwpLcMap.begin (), m_nrSlBwpLcMap.end ());
}

void
NrSlBwpManagerUe::DoSetBwpIdContainer (const std::set<uint8_t> &bwpIdVec)
{
  NS_LOG_FUNCTION (this);
  m_slBwpIds = bwpIdVec;
}

bool
NrSlBwpManagerUe::SetNrSlMacSapProviders (uint8_t bwpId, NrSlMacSapProvider* sap)
{
  NS_LOG_FUNCTION (this);
  bool result = false;
  std::map <uint8_t, NrSlMacSapProvider*>::iterator it;
  it = m_nrSlMacSapProvidersMap.find (bwpId);
  if (it != m_nrSlMacSapProvidersMap.end ())
    {
      NS_FATAL_ERROR ("BWP id " << +bwpId << " already exists");
    }
  else
    {
      m_nrSlMacSapProvidersMap.insert (std::make_pair (bwpId, sap));
      result = true;
    }
  return result;
}

NrSlMacSapProvider*
NrSlBwpManagerUe::GetNrSlMacSapProviderFromBwpm ()
{
  NS_LOG_FUNCTION (this);
  return m_nrSlBwpmUeMacSapProvider;
}

} // end of namespace ns3

/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Authors: Nicola Baldo <nbaldo@cttc.es>
 *          Budiarto Herman <budiarto.herman@magister.fi>
 * Modified by:
 *          Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 *          Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)
 *          NIST (D2D)
 *          Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *          Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#include "cv2x_lte-ue-rrc.h"

#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <ns3/object-map.h>
#include <ns3/object-factory.h>
#include <ns3/simulator.h>

#include <ns3/cv2x_lte-rlc.h>
#include <ns3/cv2x_lte-rlc-tm.h>
#include <ns3/cv2x_lte-rlc-um.h>
#include <ns3/cv2x_lte-rlc-am.h>
#include <ns3/cv2x_lte-pdcp.h>
#include <ns3/cv2x_lte-radio-bearer-info.h>
#include <ns3/cv2x_sl-pool.h>
#include <ns3/random-variable-stream.h>

#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteUeRrc");

///////////////////////////////////////////
// cv2x_LteEnbRrcSl
///////////////////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteUeRrcSl);

cv2x_LteUeRrcSl::cv2x_LteUeRrcSl ()
  : m_slEnabled (false)
{
}

void
cv2x_LteUeRrcSl::DoInitialize ()
{
}

cv2x_LteUeRrcSl::~cv2x_LteUeRrcSl (void)
{
}
  
void
cv2x_LteUeRrcSl::DoDispose ()
{
}

TypeId cv2x_LteUeRrcSl::GetTypeId (void)
{
  static TypeId  tid = TypeId ("ns3::cv2x_LteUeRrcSl")
    .SetParent<Object> ()
    .AddConstructor<cv2x_LteUeRrcSl> ()
    
    ;
  return tid;
}

/**
 * \brief makes a copy of the sidelink configuration
 * \return a copy of the sidelink configuration
 */
Ptr<cv2x_LteUeRrcSl>
cv2x_LteUeRrcSl::Copy ()
{
  NS_LOG_FUNCTION (this);
  Ptr<cv2x_LteUeRrcSl> copy = CreateObject<cv2x_LteUeRrcSl> ();
  //copy internal values
  copy->m_slEnabled = m_slEnabled;
  copy->m_slMap = m_slMap;
  return copy;
}

void
cv2x_LteUeRrcSl::SetSlEnabled (bool status)
{
  NS_LOG_FUNCTION (this);
  m_slEnabled = status;
}
    
bool
cv2x_LteUeRrcSl::IsSlEnabled ()
{
  NS_LOG_FUNCTION (this);
  return m_slEnabled;
}

void
cv2x_LteUeRrcSl::SetDiscEnabled (bool status)
{
  NS_LOG_FUNCTION (this);
  m_discEnabled = status;
}
    
bool
cv2x_LteUeRrcSl::IsDiscEnabled ()
{
  NS_LOG_FUNCTION (this);
  return m_discEnabled;
}

void
cv2x_LteUeRrcSl::SetSlPreconfiguration (cv2x_LteRrcSap::SlPreconfiguration preconfiguration)
{
  NS_LOG_FUNCTION (this);
  m_preconfiguration = preconfiguration;
}

void
cv2x_LteUeRrcSl::SetV2xEnabled (bool status)
{
  NS_LOG_FUNCTION (this);
  m_v2xEnabled = status;
}
    
bool
cv2x_LteUeRrcSl::IsV2xEnabled ()
{
  NS_LOG_FUNCTION (this);
  return m_v2xEnabled;
}

void
cv2x_LteUeRrcSl::SetSlV2xPreconfiguration (cv2x_LteRrcSap::SlV2xPreconfiguration preconfiguration)
{
  NS_LOG_FUNCTION (this);
  m_preconfigurationV2x = preconfiguration; 
}

cv2x_LteRrcSap::SlPreconfiguration
cv2x_LteUeRrcSl::GetSlPreconfiguration ()
{
  NS_LOG_FUNCTION (this);
  return m_preconfiguration;
}

cv2x_LteRrcSap::SlV2xPreconfiguration
cv2x_LteUeRrcSl::GetSlV2xPreconfiguration ()
{
  NS_LOG_FUNCTION (this);
  return m_preconfigurationV2x;
}

void
cv2x_LteUeRrcSl::SetSourceL2Id (uint32_t src)
{
  NS_LOG_FUNCTION (this);
  m_sourceL2Id = src;
}
  
bool
cv2x_LteUeRrcSl::IsTxInterested ()
{
  NS_LOG_FUNCTION (this);

  //Loop through each bearer to see if one is interested to transmit
  std::map <uint32_t, std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> > >::iterator srcIt = m_slrbMap.find (m_sourceL2Id);
  if (srcIt == m_slrbMap.end ()) {
    return false;
  } 
  return m_slrbMap[m_sourceL2Id].size () >0;
}

bool
cv2x_LteUeRrcSl::IsRxInterested ()
{
  NS_LOG_FUNCTION (this);
  
  return m_rxGroup.size() > 0;
}

bool
cv2x_LteUeRrcSl::IsMonitoringInterested ()
{
  NS_LOG_FUNCTION (this);
  return m_monitorApps.size () > 0;
}

bool
cv2x_LteUeRrcSl::IsAnnouncingInterested ()
{
  NS_LOG_FUNCTION (this);
  return m_announceApps.size () > 0;
}

std::list<uint32_t>
cv2x_LteUeRrcSl::GetTxDestinations ()
{
  std::list<uint32_t> destinations;

  //Loop through each bearer to see if one is interested to transmit
  std::map <uint32_t, std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> > >::iterator srcIt = m_slrbMap.find (m_sourceL2Id);
  if (srcIt != m_slrbMap.end ()) {
    //Loop through each bearer to see if one is interested to transmit
    std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> >::iterator it;
    for (it = m_slrbMap[m_sourceL2Id].begin(); it != m_slrbMap[m_sourceL2Id].end(); it++) {
      destinations.push_back (it->second->m_destinationL2Id);
    }
  }
  return destinations;
}

uint8_t 
cv2x_LteUeRrcSl::GetDiscTxResources ()
{
  return m_discTxResources;
}

void
cv2x_LteUeRrcSl::SetDiscTxResources (uint8_t nb)
{
  m_discTxResources = nb;  
}

uint16_t 
cv2x_LteUeRrcSl::GetDiscInterFreq ()
{
  return m_discInterFreq;
}
void
cv2x_LteUeRrcSl::SetDiscInterFreq (uint16_t ulEarfcn)
{
  m_discInterFreq = ulEarfcn;
}

bool
cv2x_LteUeRrcSl::AddSidelinkRadioBearer (Ptr<cv2x_LteSidelinkRadioBearerInfo> slb)
{
  std::map <uint32_t, std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> > >::iterator srcIt = m_slrbMap.find (slb->m_sourceL2Id);
  if (srcIt == m_slrbMap.end ()) {
    //must insert map
    std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> > empty;
    
    m_slrbMap.insert (std::pair <uint32_t, std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> > > (slb->m_sourceL2Id, empty));
    NS_LOG_LOGIC ("First SLRB for source " << slb->m_sourceL2Id);
  }

  std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> >::iterator groupIt = m_slrbMap[slb->m_sourceL2Id].find (slb->m_destinationL2Id);
  NS_ASSERT (groupIt == m_slrbMap[slb->m_sourceL2Id].end());
  NS_LOG_LOGIC ("Adding SLRB " << slb->m_sourceL2Id << "->" << slb->m_destinationL2Id);
  m_slrbMap[slb->m_sourceL2Id].insert (std::pair<uint32_t,Ptr<cv2x_LteSidelinkRadioBearerInfo> > (slb->m_destinationL2Id, slb));

  return true;
}

bool
cv2x_LteUeRrcSl::DeleteSidelinkRadioBearer (uint32_t src, uint32_t group)
{
  bool deleted = m_slrbMap[src].erase (group) >0;
  if (m_slrbMap[src].size() == 0) {
    //can delete the source as well
    m_slrbMap.erase (src);
  }
  //remove from receiving group if present
  m_rxGroup.remove (group);
  
  return deleted;
}
  
Ptr<cv2x_LteSidelinkRadioBearerInfo>
cv2x_LteUeRrcSl::GetSidelinkRadioBearer (uint32_t src, uint32_t group)
{
  Ptr<cv2x_LteSidelinkRadioBearerInfo> slrb = NULL;
  
  NS_LOG_LOGIC ("Searching SLRB " << src << "->" << group);

   //Print list of SLRB
   {
     NS_LOG_LOGIC ("Configured SLRBs "); 
     std::map <uint32_t, std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> > >::iterator srcIt;
     for (srcIt = m_slrbMap.begin (); srcIt != m_slrbMap.end () ; srcIt++) {
       std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> >::iterator dstIt;
       for (dstIt = srcIt->second.begin() ; dstIt != srcIt->second.end (); dstIt++) {
         NS_LOG_LOGIC ("SLRB " << dstIt->second->m_sourceL2Id << "->" << dstIt->second->m_destinationL2Id);
       }
     }
   }
  
  std::map <uint32_t, std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> > >::iterator srcIt = m_slrbMap.find (src);
  if (srcIt != m_slrbMap.end ()) {
    std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> >::iterator srcIt2 = (*srcIt).second.find (group);
    if (srcIt2 != (*srcIt).second.end()) {
      slrb = m_slrbMap[src][group];      
    }
  }
  return slrb;
}

Ptr<cv2x_LteSidelinkRadioBearerInfo>
cv2x_LteUeRrcSl::GetSidelinkRadioBearer (uint32_t group)
{
  return GetSidelinkRadioBearer (m_sourceL2Id, group);
}

void 
cv2x_LteUeRrcSl::AddDiscoveryApps (std::list<uint32_t> apps, bool rxtx)
{
  for (std::list<uint32_t>::iterator it = apps.begin (); it != apps.end (); ++it)
  {
    if (rxtx)
    {
      m_announceApps.push_back (*it);
    }
    else 
    {
      m_monitorApps.push_back (*it);
    }
  }
}


void 
cv2x_LteUeRrcSl::RemoveDiscoveryApps (std::list<uint32_t> apps, bool rxtx)
{
  for (std::list<uint32_t>::iterator it = apps.begin (); it != apps.end (); ++it)
  {
    if (rxtx)
    {
      m_announceApps.remove (*it);
    }
    else 
    {
      m_monitorApps.remove (*it);
    }
  }
}

void
cv2x_LteUeRrcSl::RecordTransmissionOfSidelinkUeInformation ()
{
  m_lastSidelinkUeInformationTime = Simulator::Now ();
}

double
cv2x_LteUeRrcSl::GetTimeSinceLastTransmissionOfSidelinkUeInformation ()
{
  return (Simulator::Now() - m_lastSidelinkUeInformationTime).GetSeconds();
}

uint8_t
cv2x_LteUeRrcSl::GetNextLcid ()
{
  //find unused the LCID
  bool found = true;
  uint8_t lcid;
  
  for (lcid = 1; lcid < 11; lcid++) {
    found = false;
    std::map <uint32_t, Ptr<cv2x_LteSidelinkRadioBearerInfo> >::iterator it;
    for (it = m_slrbMap[m_sourceL2Id].begin(); it != m_slrbMap[m_sourceL2Id].end(); it++) {
      if (it->second->m_logicalChannelIdentity == lcid) {
        found = true;
        break;
      }
    }
    if (!found) {
      break; //avoid increasing lcid
    }
  }
  NS_ASSERT (!found);
  return lcid;
}

bool
cv2x_LteUeRrcSl::IsCellBroadcastingSIB18 (uint16_t cellId)
{
  std::map <uint16_t, LteSlCellConfiguration>::iterator it = m_slMap.find (cellId);
  return (it != m_slMap.end() && it->second.haveSib18);
}

bool
cv2x_LteUeRrcSl::IsCellBroadcastingSIB19 (uint16_t cellId)
{
  std::map <uint16_t, LteSlCellConfiguration>::iterator it = m_slMap.find (cellId);
  return (it != m_slMap.end() && it->second.haveSib19);
}


/////////////////////////////
// CMAC SAP forwarder
/////////////////////////////

/// cv2x_UeMemberLteUeCmacSapUser class
class cv2x_UeMemberLteUeCmacSapUser : public cv2x_LteUeCmacSapUser
{
public:
  /**
   * Constructor
   *
   * \param rrc the RRC class
   */
  cv2x_UeMemberLteUeCmacSapUser (cv2x_LteUeRrc* rrc);

  virtual void SetTemporaryCellRnti (uint16_t rnti);
  virtual void NotifyRandomAccessSuccessful ();
  virtual void NotifyRandomAccessFailed ();
  //communication
  virtual void NotifySidelinkReception (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id);
  virtual void NotifyMacHasSlDataToSend ();
  virtual void NotifyMacHasNotSlDataToSend ();
  //discovery
  virtual void NotifyDiscoveryReception (Ptr<cv2x_LteControlMessage> msg);


private:
  cv2x_LteUeRrc* m_rrc; ///< the RRC class
};

cv2x_UeMemberLteUeCmacSapUser::cv2x_UeMemberLteUeCmacSapUser (cv2x_LteUeRrc* rrc)
  : m_rrc (rrc)
{
}

void
cv2x_UeMemberLteUeCmacSapUser::SetTemporaryCellRnti (uint16_t rnti)
{
  m_rrc->DoSetTemporaryCellRnti (rnti);
}


void
cv2x_UeMemberLteUeCmacSapUser::NotifyRandomAccessSuccessful ()
{
  m_rrc->DoNotifyRandomAccessSuccessful ();
}

void
cv2x_UeMemberLteUeCmacSapUser::NotifyRandomAccessFailed ()
{
  m_rrc->DoNotifyRandomAccessFailed ();
}

void
cv2x_UeMemberLteUeCmacSapUser::NotifySidelinkReception (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id)
{
  m_rrc->DoNotifySidelinkReception (lcId, srcL2Id, dstL2Id);
}

void
cv2x_UeMemberLteUeCmacSapUser::NotifyMacHasSlDataToSend ()
{
  m_rrc->DoNotifyMacHasSlDataToSend ();
}

void
cv2x_UeMemberLteUeCmacSapUser::NotifyMacHasNotSlDataToSend ()
{
  m_rrc->DoNotifyMacHasNotSlDataToSend ();
}

void
cv2x_UeMemberLteUeCmacSapUser::NotifyDiscoveryReception (Ptr<cv2x_LteControlMessage> msg)
{
  m_rrc->DoNotifyDiscoveryReception (msg);
}

/// Map each of UE RRC states to its string representation.
static const std::string g_ueRrcStateName[cv2x_LteUeRrc::NUM_STATES] =
{
  "IDLE_START",
  "IDLE_CELL_SEARCH",
  "IDLE_WAIT_MIB_SIB1",
  "IDLE_WAIT_MIB",
  "IDLE_WAIT_SIB1",
  "IDLE_CAMPED_NORMALLY",
  "IDLE_WAIT_SIB2",
  "IDLE_RANDOM_ACCESS",
  "IDLE_CONNECTING",
  "CONNECTED_NORMALLY",
  "CONNECTED_HANDOVER",
  "CONNECTED_PHY_PROBLEM",
  "CONNECTED_REESTABLISHING"
};

/**
 * \param s The UE RRC state.
 * \return The string representation of the given state.
 */
static const std::string & ToString (cv2x_LteUeRrc::State s)
{
  return g_ueRrcStateName[s];
}


/////////////////////////////
// ue RRC methods
/////////////////////////////

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteUeRrc);


cv2x_LteUeRrc::cv2x_LteUeRrc ()
  : m_cmacSapProvider (0),
    m_rrcSapUser (0),
    m_macSapProvider (0),
    m_asSapUser (0),
    m_ccmRrcSapProvider (0),
    m_state (IDLE_START),
    m_imsi (0),
    m_rnti (0),
    m_cellId (0),
    m_useRlcSm (true),
    m_connectionPending (false),
    m_hasReceivedMib (false),
    m_hasReceivedSib1 (false),
    m_hasReceivedSib2 (false),
    m_csgWhiteList (0),
    m_sidelinkConfiguration (0),
    m_slssTransmissionActive(false),
    m_txSlSyncOffsetIndicator(0),
    m_hasSyncRef (false),
    m_inCoverage(false),
    m_slssId(0),
    m_currSubframeNo(0),
    m_currFrameNo(0),
    m_hasDataToTransmit(false),
    m_inInnerCellOfSyncRef(false),
    m_slssTxTime(Seconds(0)),
    m_numberOfComponentCarriers (MIN_NO_CC)
{
  NS_LOG_FUNCTION (this);
  m_cphySapUser.push_back (new cv2x_MemberLteUeCphySapUser<cv2x_LteUeRrc> (this));
  m_cmacSapUser.push_back (new cv2x_UeMemberLteUeCmacSapUser (this));
  m_cphySapProvider.push_back(0);
  m_cmacSapProvider.push_back(0);
  m_rrcSapProvider = new cv2x_MemberLteUeRrcSapProvider<cv2x_LteUeRrc> (this);
  m_drbPdcpSapUser = new cv2x_LtePdcpSpecificLtePdcpSapUser<cv2x_LteUeRrc> (this);
  m_asSapProvider = new cv2x_MemberLteAsSapProvider<cv2x_LteUeRrc> (this);
  m_ccmRrcSapUser = new MemberLteUeCcmRrcSapUser<cv2x_LteUeRrc> (this);
}

cv2x_LteUeRrc::~cv2x_LteUeRrc ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteUeRrc::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  for ( uint16_t i = 0; i < m_numberOfComponentCarriers; i++)
   {
      delete m_cphySapUser.at(i);
      delete m_cmacSapUser.at(i);
   }
  m_cphySapUser.clear ();
  m_cmacSapUser.clear ();
  delete m_rrcSapProvider;
  delete m_drbPdcpSapUser;
  delete m_asSapProvider;
  delete m_ccmRrcSapUser;
  m_cphySapProvider.erase(m_cphySapProvider.begin(), m_cphySapProvider.end());
  m_cphySapProvider.clear();
  m_cmacSapProvider.erase(m_cmacSapProvider.begin(), m_cmacSapProvider.end());
  m_cmacSapProvider.clear();
  m_drbMap.clear ();
}

TypeId
cv2x_LteUeRrc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteUeRrc")
    .SetParent<Object> ()
    .SetGroupName ("Lte")
    .AddConstructor<cv2x_LteUeRrc> ()
    .AddAttribute ("DataRadioBearerMap", "List of UE RadioBearerInfo for Data Radio Bearers by LCID.",
                   ObjectMapValue (),
                   MakeObjectMapAccessor (&cv2x_LteUeRrc::m_drbMap),
                   MakeObjectMapChecker<cv2x_LteDataRadioBearerInfo> ())
    .AddAttribute ("Srb0", "SignalingRadioBearerInfo for SRB0",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteUeRrc::m_srb0),
                   MakePointerChecker<cv2x_LteSignalingRadioBearerInfo> ())
    .AddAttribute ("Srb1", "SignalingRadioBearerInfo for SRB1",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteUeRrc::m_srb1),
                   MakePointerChecker<cv2x_LteSignalingRadioBearerInfo> ())
    .AddAttribute ("CellId",
                   "Serving cell identifier",
                   UintegerValue (0), // unused, read-only attribute
                   MakeUintegerAccessor (&cv2x_LteUeRrc::GetCellId),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("C-RNTI",
                   "Cell Radio Network Temporary Identifier",
                   UintegerValue (0), // unused, read-only attribute
                   MakeUintegerAccessor (&cv2x_LteUeRrc::GetRnti),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("T300",
                   "Timer for the RRC Connection Establishment procedure "
                   "(i.e., the procedure is deemed as failed if it takes longer than this)",
                   TimeValue (MilliSeconds (100)),
                   MakeTimeAccessor (&cv2x_LteUeRrc::m_t300),
                   MakeTimeChecker ())
    //Add accessor to sidelink configuration
    .AddAttribute ("SidelinkConfiguration",
                   "The sidelink configuration associated to this cv2x_LtePhy",
                   PointerValue (),
                   MakePointerAccessor (&cv2x_LteUeRrc::m_sidelinkConfiguration),
                   MakePointerChecker <cv2x_LteUeRrcSl> ())
    .AddTraceSource ("MibReceived",
                     "trace fired upon reception of Master Information Block",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_mibReceivedTrace),
                     "ns3::cv2x_LteUeRrc::MibSibHandoverTracedCallback")
    .AddTraceSource ("Sib1Received",
                     "trace fired upon reception of System Information Block Type 1",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_sib1ReceivedTrace),
                     "ns3::cv2x_LteUeRrc::MibSibHandoverTracedCallback")
    .AddTraceSource ("Sib2Received",
                     "trace fired upon reception of System Information Block Type 2",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_sib2ReceivedTrace),
                     "ns3::cv2x_LteUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("StateTransition",
                     "trace fired upon every UE RRC state transition",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_stateTransitionTrace),
                     "ns3::cv2x_LteUeRrc::StateTracedCallback")
    .AddTraceSource ("InitialCellSelectionEndOk",
                     "trace fired upon successful initial cell selection procedure",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_initialCellSelectionEndOkTrace),
                     "ns3::cv2x_LteUeRrc::CellSelectionTracedCallback")
    .AddTraceSource ("InitialCellSelectionEndError",
                     "trace fired upon failed initial cell selection procedure",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_initialCellSelectionEndErrorTrace),
                     "ns3::cv2x_LteUeRrc::CellSelectionTracedCallback")
    .AddTraceSource ("RandomAccessSuccessful",
                     "trace fired upon successful completion of the random access procedure",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_randomAccessSuccessfulTrace),
                     "ns3::cv2x_LteUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("RandomAccessError",
                     "trace fired upon failure of the random access procedure",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_randomAccessErrorTrace),
                     "ns3::cv2x_LteUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("ConnectionEstablished",
                     "trace fired upon successful RRC connection establishment",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_connectionEstablishedTrace),
                     "ns3::cv2x_LteUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("ConnectionTimeout",
                     "trace fired upon timeout RRC connection establishment because of T300",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_connectionTimeoutTrace),
                     "ns3::cv2x_LteUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("ConnectionReconfiguration",
                     "trace fired upon RRC connection reconfiguration",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_connectionReconfigurationTrace),
                     "ns3::cv2x_LteUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("HandoverStart",
                     "trace fired upon start of a handover procedure",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_handoverStartTrace),
                     "ns3::cv2x_LteUeRrc::MibSibHandoverTracedCallback")
    .AddTraceSource ("HandoverEndOk",
                     "trace fired upon successful termination of a handover procedure",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_handoverEndOkTrace),
                     "ns3::cv2x_LteUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("HandoverEndError",
                     "trace fired upon failure of a handover procedure",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_handoverEndErrorTrace),
                     "ns3::cv2x_LteUeRrc::ImsiCidRntiTracedCallback")
    .AddTraceSource ("SCarrierConfigured",
                     "trace fired after configuring secondary carriers",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_sCarrierConfiguredTrace),
                     "ns3::cv2x_LteUeRrc::SCarrierConfiguredTracedCallback")
    .AddAttribute ("UeSlssTransmissionEnabled",
             "If True, the UE transmits SLSSs when required as part of the sidelink synchronization protocol",
              BooleanValue(false),
              MakeBooleanAccessor (&cv2x_LteUeRrc::m_slssTransmissionEnabled),
              MakeBooleanChecker())
    .AddAttribute ("MinSrsrp",
              "The minimum S-RSRP required to consider a SyncRef detectable",
              DoubleValue(-125),
              MakeDoubleAccessor (&cv2x_LteUeRrc::m_minSrsrp),
              MakeDoubleChecker<double>())
    .AddTraceSource ("ChangeOfSyncRef",
                     "trace fired upon report of a change of SyncRef",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_ChangeOfSyncRefTrace),
                     "ns3::cv2x_LteUeRrc::ChangeOfSyncRefTracedCallback")
    .AddTraceSource ("SendSLSS",
                     "trace fired upon send of a SLSS",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_SendSlssTrace),
                     "ns3::cv2x_LteUeRrc::SendSLSSTracedCallback")
    // Added to trace the reception of discovery message
    .AddTraceSource ("DiscoveryMonitoring",
                     "trace to track the monitoring of discovery messages",
                     MakeTraceSourceAccessor (&cv2x_LteUeRrc::m_discoveryMonitoringTrace),
                     "ns3::cv2x_LteUeRrc::DiscoveryMonitoringTracedCallback")
  ;
  return tid;
}


void
cv2x_LteUeRrc::SetLteUeCphySapProvider (cv2x_LteUeCphySapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_cphySapProvider.at(0) = s;
}

void
cv2x_LteUeRrc::SetLteUeCphySapProvider (cv2x_LteUeCphySapProvider * s, uint8_t index)
{
  NS_LOG_FUNCTION (this << s);
  m_cphySapProvider.at(index) = s;
}

cv2x_LteUeCphySapUser*
cv2x_LteUeRrc::GetLteUeCphySapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_cphySapUser.at(0);
}

cv2x_LteUeCphySapUser*
cv2x_LteUeRrc::GetLteUeCphySapUser (uint8_t index)
{
  NS_LOG_FUNCTION (this);
  return m_cphySapUser.at(index);
}

void
cv2x_LteUeRrc::SetLteUeCmacSapProvider (cv2x_LteUeCmacSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_cmacSapProvider.at (0) = s;
}

void
cv2x_LteUeRrc::SetLteUeCmacSapProvider (cv2x_LteUeCmacSapProvider * s, uint8_t index)
{
  NS_LOG_FUNCTION (this << s);
  m_cmacSapProvider.at (index) = s;
}

cv2x_LteUeCmacSapUser*
cv2x_LteUeRrc::GetLteUeCmacSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_cmacSapUser.at (0);
}

cv2x_LteUeCmacSapUser*
cv2x_LteUeRrc::GetLteUeCmacSapUser (uint8_t index)
{
  NS_LOG_FUNCTION (this);
  return m_cmacSapUser.at (index);
}

void
cv2x_LteUeRrc::SetLteUeRrcSapUser (cv2x_LteUeRrcSapUser * s)
{
  NS_LOG_FUNCTION (this << s);
  m_rrcSapUser = s;
}

cv2x_LteUeRrcSapProvider*
cv2x_LteUeRrc::GetLteUeRrcSapProvider ()
{
  NS_LOG_FUNCTION (this);
  return m_rrcSapProvider;
}

void
cv2x_LteUeRrc::SetLteMacSapProvider (cv2x_LteMacSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_macSapProvider = s;
}

void
cv2x_LteUeRrc::SetLteCcmRrcSapProvider (cv2x_LteUeCcmRrcSapProvider * s)
{
  NS_LOG_FUNCTION (this << s);
  m_ccmRrcSapProvider = s;
}

cv2x_LteUeCcmRrcSapUser*
cv2x_LteUeRrc::GetLteCcmRrcSapUser ()
{
  NS_LOG_FUNCTION (this);
  return m_ccmRrcSapUser;
}

void
cv2x_LteUeRrc::SetAsSapUser (cv2x_LteAsSapUser* s)
{
  m_asSapUser = s;
}

cv2x_LteAsSapProvider* 
cv2x_LteUeRrc::GetAsSapProvider ()
{
  return m_asSapProvider;
}

void 
cv2x_LteUeRrc::SetImsi (uint64_t imsi)
{
  NS_LOG_FUNCTION (this << imsi);
  m_imsi = imsi;
}

uint64_t
cv2x_LteUeRrc::GetImsi (void) const
{
  return m_imsi;
}

uint16_t
cv2x_LteUeRrc::GetRnti () const
{
  NS_LOG_FUNCTION (this);
  return m_rnti;
}

uint16_t
cv2x_LteUeRrc::GetCellId () const
{
  NS_LOG_FUNCTION (this);
  return m_cellId;
}


uint8_t 
cv2x_LteUeRrc::GetUlBandwidth () const
{
  NS_LOG_FUNCTION (this);
  return m_ulBandwidth;
}

uint8_t 
cv2x_LteUeRrc::GetDlBandwidth () const
{
  NS_LOG_FUNCTION (this);
  return m_dlBandwidth;
}

uint32_t
cv2x_LteUeRrc::GetDlEarfcn () const
{
  return m_dlEarfcn;
}

uint32_t 
cv2x_LteUeRrc::GetUlEarfcn () const
{
  NS_LOG_FUNCTION (this);
  return m_ulEarfcn;
}

cv2x_LteUeRrc::State
cv2x_LteUeRrc::GetState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_state;
}

void
cv2x_LteUeRrc::SetUseRlcSm (bool val) 
{
  NS_LOG_FUNCTION (this);
  m_useRlcSm = val;
}


void
cv2x_LteUeRrc::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);

  // setup the UE side of SRB0
  uint8_t lcid = 0;

  Ptr<cv2x_LteRlc> rlc = CreateObject<cv2x_LteRlcTm> ()->GetObject<cv2x_LteRlc> ();
  rlc->SetLteMacSapProvider (m_macSapProvider);
  rlc->SetRnti (m_rnti);
  rlc->SetLcId (lcid);

  m_srb0 = CreateObject<cv2x_LteSignalingRadioBearerInfo> ();
  m_srb0->m_rlc = rlc;
  m_srb0->m_srbIdentity = 0;
  cv2x_LteUeRrcSapUser::SetupParameters ueParams;
  ueParams.srb0SapProvider = m_srb0->m_rlc->GetLteRlcSapProvider ();
  ueParams.srb1SapProvider = 0;
  m_rrcSapUser->Setup (ueParams);

  // CCCH (LCID 0) is pre-configured, here is the hardcoded configuration:
  cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig;
  lcConfig.priority = 0; // highest priority
  lcConfig.prioritizedBitRateKbps = 65535; // maximum
  lcConfig.bucketSizeDurationMs = 65535; // maximum
  lcConfig.logicalChannelGroup = 0; // all SRBs mapped to LCG 0
  m_cmacSapProvider.at(0)->AddLc (lcid, lcConfig, rlc->GetLteMacSapUser ());
}

void
cv2x_LteUeRrc::InitializeSap (void)
{
  if (m_numberOfComponentCarriers < MIN_NO_CC || m_numberOfComponentCarriers > MAX_NO_CC)
    {
      // this check is needed in order to maintain backward compatibility with scripts and tests
      // if case lte-helper is not used (like in several tests) the m_numberOfComponentCarriers
      // is not set and then an error is raised
      // In this case m_numberOfComponentCarriers is set to 1
      m_numberOfComponentCarriers = MIN_NO_CC;
    }
  if (m_numberOfComponentCarriers > MIN_NO_CC )
    {
      for ( uint16_t i = 1; i < m_numberOfComponentCarriers; i++)
        {
          m_cphySapUser.push_back(new cv2x_MemberLteUeCphySapUser<cv2x_LteUeRrc> (this));
          m_cmacSapUser.push_back(new cv2x_UeMemberLteUeCmacSapUser (this));
          m_cphySapProvider.push_back(0);
          m_cmacSapProvider.push_back(0);
        }
    }
}


void
cv2x_LteUeRrc::DoSendData (Ptr<Packet> packet, uint8_t bid)
{
  NS_LOG_FUNCTION (this << packet);

  uint8_t drbid = Bid2Drbid (bid);

  if (drbid != 0)
    {
  std::map<uint8_t, Ptr<cv2x_LteDataRadioBearerInfo> >::iterator it =   m_drbMap.find (drbid);
  NS_ASSERT_MSG (it != m_drbMap.end (), "could not find bearer with drbid == " << drbid);

  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters params;
  params.pdcpSdu = packet;
  params.rnti = m_rnti;
  params.lcid = it->second->m_logicalChannelIdentity;
  params.dstL2Id = 0;
  params.srcL2Id = 0;


  NS_LOG_LOGIC (this << " RNTI=" << m_rnti << " sending packet " << packet
                     << " on DRBID " << (uint32_t) drbid
                     << " (LCID " << (uint32_t) params.lcid << ")"
                     << " (" << packet->GetSize () << " bytes)");
  it->second->m_pdcp->GetLtePdcpSapProvider ()->TransmitPdcpSdu (params);
    }
}

void
cv2x_LteUeRrc::DoSendSidelinkData (Ptr<Packet> packet, uint32_t group)
{
  NS_LOG_FUNCTION (this << packet << "for sidelink group " <<group);
  //Find the PDCP for sidelink transmission
  Ptr<cv2x_LteSidelinkRadioBearerInfo> slrb = m_sidelinkConfiguration->GetSidelinkRadioBearer (group);
  NS_ASSERT_MSG (slrb, "could not find sidelink bearer for group == " << group); //the NAS should be aware about the existance of the bearer or not

  cv2x_LtePdcpSapProvider::TransmitPdcpSduParameters params;
  params.pdcpSdu = packet;
  params.rnti = m_rnti;
  params.lcid = slrb->m_logicalChannelIdentity;
  params.dstL2Id = 0;
  params.srcL2Id = 0;
  
  NS_LOG_LOGIC (this << " RNTI=" << m_rnti << " sending packet " << packet
                     << " on SLRBBID " << (uint32_t) group
                     << " (LCID " << (uint32_t) params.lcid << ")"
                     << " (" << packet->GetSize () << " bytes)");
  slrb->m_pdcp->GetLtePdcpSapProvider ()->TransmitPdcpSdu (params);
}

void
cv2x_LteUeRrc::DoDisconnect ()
{
  NS_LOG_FUNCTION (this);

  switch (m_state)
    {
    case IDLE_START:
    case IDLE_CELL_SEARCH:
    case IDLE_WAIT_MIB_SIB1:
    case IDLE_WAIT_MIB:
    case IDLE_WAIT_SIB1:
    case IDLE_CAMPED_NORMALLY:
      NS_LOG_INFO ("already disconnected");
      break;

    case IDLE_WAIT_SIB2:
    case IDLE_CONNECTING:
      NS_FATAL_ERROR ("cannot abort connection setup procedure");
      break;

    case CONNECTED_NORMALLY:
    case CONNECTED_HANDOVER:
    case CONNECTED_PHY_PROBLEM:
    case CONNECTED_REESTABLISHING:
      LeaveConnectedMode ();
      break;

    default: // i.e. IDLE_RANDOM_ACCESS
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void
cv2x_LteUeRrc::DoActivateSidelinkRadioBearer (uint32_t group, bool tx, bool rx)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT_MSG(m_sidelinkConfiguration->GetSidelinkRadioBearer (m_sidelinkConfiguration->m_sourceL2Id, group) == NULL,
                "Sidelink bearer with src="<< m_sidelinkConfiguration->m_sourceL2Id << " and group="<< group <<" is already installed.");
  
  switch (m_state)
    {
    case IDLE_START:
    case IDLE_CELL_SEARCH:
    case IDLE_WAIT_MIB_SIB1:
    case IDLE_WAIT_MIB:
    case IDLE_WAIT_SIB1:
    case IDLE_CAMPED_NORMALLY:
      NS_LOG_INFO ("Considering out of network");
      NS_LOG_INFO ("IMSI " << m_imsi << " considered out of network");

      if (m_rnti == 0)
        {
          //the RNTI was not configured, this is the first call to sidelink configuration
          NS_LOG_INFO (this << " Setting RNTI to " <<  (uint16_t) (m_imsi & 0xFFFF));
          //preconfigure the RNTI to the IMSI's 16 LSB for uniqueness
          DoSetTemporaryCellRnti ( (uint16_t) (m_imsi & 0xFFFF));
          //propagate to the MAC (normally the MAC indicates the RNTI when receiving message from the eNodeB)
          m_cmacSapProvider.at (0)->SetRnti (m_rnti);
          //since it is first time, configure the physical layer as well
          if (!m_sidelinkConfiguration->IsV2xEnabled ())
            {
              m_cphySapProvider.at (0)->ConfigureUplink (m_sidelinkConfiguration->GetSlPreconfiguration().preconfigGeneral.carrierFreq, m_sidelinkConfiguration->GetSlPreconfiguration().preconfigGeneral.slBandwidth);
            }
          else
            {
              m_cphySapProvider.at (0)->ConfigureUplink (m_sidelinkConfiguration->GetSlV2xPreconfiguration().v2xPreconfigFreqList.freq[0].v2xCommPreconfigGeneral.carrierFreq, m_sidelinkConfiguration->GetSlV2xPreconfiguration().v2xPreconfigFreqList.freq[0].v2xCommPreconfigGeneral.slBandwidth);
            }
        }

      if (tx)
        {
          Ptr<cv2x_LteSidelinkRadioBearerInfo> slbInfo = AddSlrb (m_sidelinkConfiguration->m_sourceL2Id, group, m_sidelinkConfiguration->GetNextLcid ());
          NS_LOG_INFO ("Created new TX SLRB for group " << group << " LCID=" << (slbInfo->m_logicalChannelIdentity & 0xF));
        }
      if (rx)
        {
          NS_LOG_LOGIC (this << " rx: AddSlDestination " << group);
          //Add to the list of group to monitor for sidelink
          m_sidelinkConfiguration->m_rxGroup.push_back (group);
          //tell the phy to listen for the group
          m_cphySapProvider.at (0)->AddSlDestination (group);
          m_cmacSapProvider.at (0)->AddSlDestination (group);
        }

      NS_ASSERT (
          m_sidelinkConfiguration->GetSlPreconfiguration().preconfigComm.nbPools >0 ||
          (
            m_sidelinkConfiguration->GetSlV2xPreconfiguration().v2xPreconfigFreqList.freq[0].v2xCommTxPoolList.nbPools >0 &&
            m_sidelinkConfiguration->GetSlV2xPreconfiguration().v2xPreconfigFreqList.freq[0].v2xCommRxPoolList.nbPools >0
          )
        );

      //Activate bearer using preconfiguration if available
      if (tx) 
        {
          NS_LOG_INFO ("Configuring Tx pool");

          if (!m_sidelinkConfiguration->IsV2xEnabled ()) 
            {
              Ptr<SidelinkTxCommResourcePool> txPool = CreateObject<SidelinkTxCommResourcePool>();
              txPool->SetPool (m_sidelinkConfiguration->GetSlPreconfiguration().preconfigComm.pools[0]);

              std::list <uint32_t>::iterator it;
              std::list <uint32_t> destinations = m_sidelinkConfiguration->GetTxDestinations ();
              //int index = 0;
              //currently we can only use one pool so all groups will use the same one
              
              //for (it = destinations.begin() ; it != destinations.end() ; it++) {
              //m_cmacSapProvider.at (0)->AddSlTxPool (*it, txPool);
              m_cmacSapProvider.at (0)->AddSlTxPool (group, txPool);
              //if this is the first group setup, register pool with physical layer, otherwise it has already been done
              if (destinations.size() == 1) 
                {
                  //inform PHY about pool
                  m_cphySapProvider.at (0)->SetSlTxPool (txPool);
                }
            }
          else 
            {
              Ptr<SidelinkTxCommResourcePoolV2x> txPool = CreateObject<SidelinkTxCommResourcePoolV2x>();
              txPool->SetPool (m_sidelinkConfiguration->GetSlV2xPreconfiguration().v2xPreconfigFreqList.freq[0].v2xCommTxPoolList.pools[0]);

              std::list <uint32_t>::iterator it;
              std::list <uint32_t> destinations = m_sidelinkConfiguration->GetTxDestinations ();
              //currently we can only use one pool so all groups will use the same one
              m_cmacSapProvider.at (0)->AddSlV2xTxPool (group, txPool);
              //if this is the first group setup, register pool with physical layer, otherwise it has already been done
              if (destinations.size() == 1) 
                {
                  //inform PHY about pool
                  m_cphySapProvider.at (0)->SetSlV2xTxPool (txPool);
                }
            }        
        }
      if (tx || rx)
        {
          //Configure receiving pool
          if (!m_sidelinkConfiguration->IsV2xEnabled ())
            {
              std::list< Ptr<SidelinkRxCommResourcePool> > pools;
              Ptr<SidelinkRxCommResourcePool> pool = CreateObject <SidelinkRxCommResourcePool> ();
              pool->SetPool (m_sidelinkConfiguration->GetSlPreconfiguration().preconfigComm.pools[0]);
              //must find ways to store Rx pool though it is in different format
              //m_sidelinkConfiguration->rxPools.push_back (std::make_pair(msg.sib18.commConfig.commRxPool.pools[i], pool));
              pools.push_back (pool);
              m_cmacSapProvider.at (0)->SetSlRxPools (pools);
              m_cphySapProvider.at (0)->SetSlRxPools (pools);
            }
          else
            {
              std::list< Ptr<SidelinkRxCommResourcePoolV2x> > pools;
              Ptr<SidelinkRxCommResourcePoolV2x> pool = CreateObject <SidelinkRxCommResourcePoolV2x> ();
              pool->SetPool (m_sidelinkConfiguration->GetSlV2xPreconfiguration().v2xPreconfigFreqList.freq[0].v2xCommRxPoolList.pools[0]);
              //must find ways to store Rx pool though it is in different format
              pools.push_back (pool);
              m_cmacSapProvider.at (0)->SetSlV2xRxPools (pools);
              m_cphySapProvider.at (0)->SetSlV2xRxPools (pools);
            }
        }
      //for testing, just indicate it is ok
      m_asSapUser->NotifySidelinkRadioBearerActivated (group);
      break;
      
    case IDLE_WAIT_SIB2:
    case IDLE_CONNECTING:
      NS_LOG_INFO ("Connecting, must wait to send message");
      break;
      
    case CONNECTED_NORMALLY:
    case CONNECTED_HANDOVER:
    case CONNECTED_PHY_PROBLEM:
    case CONNECTED_REESTABLISHING:
      NS_LOG_INFO ("Considering in coverage");
      if (tx)
        {
          Ptr<cv2x_LteSidelinkRadioBearerInfo> slbInfo = AddSlrb (m_sidelinkConfiguration->m_sourceL2Id, group, m_sidelinkConfiguration->GetNextLcid ());
          NS_LOG_INFO ("Created new TX SLRB for group " << group << " LCID=" << (slbInfo->m_logicalChannelIdentity & 0xF));
        }
      if (rx)
        {
          NS_LOG_LOGIC (this << " rx: AddSlDestination " << group);
          //Add to the list of group to monitor for sidelink
          m_sidelinkConfiguration->m_rxGroup.push_back (group);
          //tell the phy to listen for the group
          m_cphySapProvider.at (0)->AddSlDestination (group);
          m_cmacSapProvider.at (0)->AddSlDestination (group);
        }
      //Try to send to eNodeB
      SendSidelinkUeInformation ();
      break;
        
    default: // i.e. IDLE_RANDOM_ACCESS
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void
cv2x_LteUeRrc::DoDeactivateSidelinkRadioBearer (uint32_t group)
{
  NS_LOG_FUNCTION (this);

  //TODO: need handling of SLRBs for the group coming from other UEs as well.
  
  if (m_sidelinkConfiguration->DeleteSidelinkRadioBearer (m_sidelinkConfiguration->m_sourceL2Id, group))
    {
      switch (m_state)
      {
      case IDLE_START:
      case IDLE_CELL_SEARCH:
      case IDLE_WAIT_MIB_SIB1:
      case IDLE_WAIT_MIB:
      case IDLE_WAIT_SIB1:
      case IDLE_CAMPED_NORMALLY:
        NS_LOG_INFO ("Considering out of network");
        //Activate bearer using preconfiguration if available
        //TBD        
        break;                
      case CONNECTED_NORMALLY:
      case CONNECTED_HANDOVER:
      case CONNECTED_PHY_PROBLEM:
      case CONNECTED_REESTABLISHING:
        NS_LOG_INFO ("Considering in coverage");
        //Try to send to eNodeB
        SendSidelinkUeInformation ();
        break;
        
      default: // i.e. IDLE_RANDOM_ACCESS
        NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
        break;
      }
    }
}

Ptr<cv2x_LteSidelinkRadioBearerInfo>
cv2x_LteUeRrc::AddSlrb (uint32_t source, uint32_t destination, uint8_t lcid)
{
  Ptr<cv2x_LteSidelinkRadioBearerInfo> slbInfo = CreateObject <cv2x_LteSidelinkRadioBearerInfo> ();
    slbInfo->m_sourceL2Id = source;
    slbInfo->m_destinationL2Id = destination;
    slbInfo->m_logicalChannelIdentity = lcid;
    //slbInfo->m_tx = tx;
    //slbInfo->m_rx = rx;
    m_sidelinkConfiguration->AddSidelinkRadioBearer (slbInfo);

    //create PDCP/RLC stack
    ObjectFactory rlcObjectFactory;
    rlcObjectFactory.SetTypeId (cv2x_LteRlcUm::GetTypeId ());
    Ptr<cv2x_LteRlc> rlc = rlcObjectFactory.Create ()->GetObject<cv2x_LteRlc> ();
    rlc->SetLteMacSapProvider (m_macSapProvider);
    rlc->SetRnti (m_rnti); //??
    rlc->SetLcId (slbInfo->m_logicalChannelIdentity);
    rlc->SetSourceL2Id (slbInfo->m_sourceL2Id);
    rlc->SetDestinationL2Id (slbInfo->m_destinationL2Id);
    
    slbInfo->m_rlc = rlc;
        
    Ptr<cv2x_LtePdcp> pdcp = CreateObject<cv2x_LtePdcp> ();
    pdcp->SetRnti (m_rnti);
    pdcp->SetLcId (slbInfo->m_logicalChannelIdentity);
    pdcp->SetSourceL2Id (slbInfo->m_sourceL2Id);
    pdcp->SetDestinationL2Id (slbInfo->m_destinationL2Id);
    pdcp->SetLtePdcpSapUser (m_drbPdcpSapUser);
    pdcp->SetLteRlcSapProvider (rlc->GetLteRlcSapProvider ());
    rlc->SetLteRlcSapUser (pdcp->GetLteRlcSapUser ());
    slbInfo->m_pdcp = pdcp;


    struct cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig;
    lcConfig.priority = 9;
    lcConfig.prioritizedBitRateKbps = 65535;
    lcConfig.bucketSizeDurationMs = 65535;
    lcConfig.logicalChannelGroup = 3; //"11" as per 36.321      

    m_cmacSapProvider.at (0)->AddLc (slbInfo->m_logicalChannelIdentity,
                              slbInfo->m_sourceL2Id,
                              slbInfo->m_destinationL2Id,
                              lcConfig,
                              rlc->GetLteMacSapUser ());
    //rlc->Initialize ();

    return slbInfo;
}

void
cv2x_LteUeRrc::DoAddDiscoveryApps (std::list<uint32_t> apps, bool rxtx)
{
  NS_LOG_FUNCTION (this);
  
  switch (m_state)
    {
    case IDLE_START:
    case IDLE_CELL_SEARCH:
    case IDLE_WAIT_MIB_SIB1:
    case IDLE_WAIT_MIB:
    case IDLE_WAIT_SIB1:
    case IDLE_CAMPED_NORMALLY:
      NS_LOG_INFO ("Considering out of network");

      if (m_rnti == 0)
        {
          NS_LOG_INFO (this << " Setting RNTI to " <<  (uint16_t) (m_imsi & 0xFFFF));
          //preconfigure the RNTI to the IMSI's 16 LSB for uniqueness
          DoSetTemporaryCellRnti ( (uint16_t) (m_imsi & 0xFFFF));
          //propagate to the MAC (normally the MAC indicates the RNTI when receiving message from the eNodeB)
          m_cmacSapProvider.at (0)->SetRnti (m_rnti);
          //since it is first time, configure the physical layer as well
          m_cphySapProvider.at (0)->ConfigureUplink (m_sidelinkConfiguration->GetSlPreconfiguration().preconfigGeneral.carrierFreq, m_sidelinkConfiguration->GetSlPreconfiguration().preconfigGeneral.slBandwidth);
        }
          
      m_sidelinkConfiguration->AddDiscoveryApps (apps, rxtx);
      NS_LOG_INFO ("Created new discovery Applications for UE " << m_rnti );

      //Inform MAC about new disc apps
      m_cmacSapProvider.at (0)->ModifyDiscTxApps (m_sidelinkConfiguration->m_announceApps);
      m_cmacSapProvider.at (0)->ModifyDiscRxApps (m_sidelinkConfiguration->m_monitorApps);

      //Set pool using preconfigured one if it exits
      if (m_sidelinkConfiguration->GetSlPreconfiguration().preconfigDisc.nbPools >0)
      {
        //announce: Tx
        if (rxtx)
        {
          NS_LOG_INFO ("Configuring Tx preconfigured pool");
          Ptr<SidelinkTxDiscResourcePool> txPool = CreateObject<SidelinkTxDiscResourcePool>();
          txPool->SetPool (m_sidelinkConfiguration->GetSlPreconfiguration().preconfigDisc.pools[0]);

          //inform MAC and PHY about the pool
          m_cmacSapProvider.at (0)->AddSlTxPool (txPool);
          m_cphySapProvider.at (0)->SetSlTxPool (txPool);
        }
        //monitor: Rx
        else 
        {
          std::list< Ptr<SidelinkRxDiscResourcePool> > pools;
          Ptr<SidelinkRxDiscResourcePool> pool = CreateObject <SidelinkRxDiscResourcePool> ();
          pool->SetPool (m_sidelinkConfiguration->GetSlPreconfiguration().preconfigDisc.pools[0]);
          pools.push_back (pool);
          m_cmacSapProvider.at (0)->SetSlRxPools (pools);
          m_cphySapProvider.at (0)->SetSlRxPools (pools);
        }
      }

      break;
      
    case IDLE_WAIT_SIB2:
    case IDLE_CONNECTING:
      NS_LOG_INFO ("Connecting, must wait to send message");
      break;
      
    case CONNECTED_NORMALLY:
    case CONNECTED_HANDOVER:
    case CONNECTED_PHY_PROBLEM:
    case CONNECTED_REESTABLISHING:
      NS_LOG_INFO ("Considering in coverage");
      m_sidelinkConfiguration->AddDiscoveryApps (apps, rxtx);
      NS_LOG_INFO ("Created new discovery Applications for UE " << m_rnti );
      //Inform MAC about new disc apps
      m_cmacSapProvider.at (0)->ModifyDiscTxApps (m_sidelinkConfiguration->m_announceApps);
      m_cmacSapProvider.at (0)->ModifyDiscRxApps (m_sidelinkConfiguration->m_monitorApps);
      //Try to send to eNodeB
      SendSidelinkUeInformation ();
      break;
        
    default: // i.e. IDLE_RANDOM_ACCESS
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void
cv2x_LteUeRrc::DoRemoveDiscoveryApps (std::list<uint32_t> apps, bool rxtx)
{
  NS_LOG_FUNCTION (this);

  m_sidelinkConfiguration->RemoveDiscoveryApps (apps, rxtx);
  NS_LOG_INFO ("deleting new discovery Applications for UE " << m_rnti );
  // Inform MAC
  m_cmacSapProvider.at (0)->ModifyDiscTxApps (m_sidelinkConfiguration->m_announceApps);
  m_cmacSapProvider.at (0)->ModifyDiscRxApps (m_sidelinkConfiguration->m_monitorApps);

      switch (m_state)
      {
      case IDLE_START:
      case IDLE_CELL_SEARCH:
      case IDLE_WAIT_MIB_SIB1:
      case IDLE_WAIT_MIB:
      case IDLE_WAIT_SIB1:
      case IDLE_CAMPED_NORMALLY:
        NS_LOG_INFO ("Considering out of network");
        //Activate bearer using preconfiguration if available
        //TBD    
        break;                
      case CONNECTED_NORMALLY:
      case CONNECTED_HANDOVER:
      case CONNECTED_PHY_PROBLEM:
      case CONNECTED_REESTABLISHING:
        NS_LOG_INFO ("Considering in coverage");
        //Try to send to eNodeB
        SendSidelinkUeInformation ();
        break;
        
      default: // i.e. IDLE_RANDOM_ACCESS
        NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
        break;
      }
}  

void
cv2x_LteUeRrc::DoReceivePdcpSdu (cv2x_LtePdcpSapUser::ReceivePdcpSduParameters params)
{
  NS_LOG_FUNCTION (this);
  m_asSapUser->RecvData (params.pdcpSdu);
}


void
cv2x_LteUeRrc::DoSetTemporaryCellRnti (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);
  m_rnti = rnti;
  m_srb0->m_rlc->SetRnti (m_rnti);
  m_cphySapProvider.at(0)->SetRnti (m_rnti);
}

void
cv2x_LteUeRrc::DoNotifyRandomAccessSuccessful ()
{
  NS_LOG_FUNCTION (this << m_imsi << ToString (m_state));
  m_randomAccessSuccessfulTrace (m_imsi, m_cellId, m_rnti);

  switch (m_state)
    {
    case IDLE_RANDOM_ACCESS:
      {
        // we just received a RAR with a T-C-RNTI and an UL grant
        // send RRC connection request as message 3 of the random access procedure 
        SwitchToState (IDLE_CONNECTING);
        cv2x_LteRrcSap::RrcConnectionRequest msg;
        msg.ueIdentity = m_imsi;
        m_rrcSapUser->SendRrcConnectionRequest (msg); 
        m_connectionTimeout = Simulator::Schedule (m_t300,
                                                   &cv2x_LteUeRrc::ConnectionTimeout,
                                                   this);
      }
      break;

    case CONNECTED_HANDOVER:
      {
        cv2x_LteRrcSap::RrcConnectionReconfigurationCompleted msg;
        msg.rrcTransactionIdentifier = m_lastRrcTransactionIdentifier;
        m_rrcSapUser->SendRrcConnectionReconfigurationCompleted (msg);

        // 3GPP TS 36.331 section 5.5.6.1 Measurements related actions upon handover
        std::map<uint8_t, cv2x_LteRrcSap::MeasIdToAddMod>::iterator measIdIt;
        for (measIdIt = m_varMeasConfig.measIdList.begin ();
             measIdIt != m_varMeasConfig.measIdList.end ();
             ++measIdIt)
          {
            VarMeasReportListClear (measIdIt->second.measId);
          }
        //if target cell sending SIB 18 or 19 and SidelinkUEInformation was sent in the last second
        //with interest/resource request,
        //initiate transmission of the SidelinkUEInformation message in accordance with 5.X.2.3
        if ((m_sidelinkConfiguration->IsCellBroadcastingSIB18 (m_cellId) || m_sidelinkConfiguration->IsCellBroadcastingSIB19 (m_cellId)) && m_sidelinkConfiguration->GetTimeSinceLastTransmissionOfSidelinkUeInformation () < 1.0) {
          SendSidelinkUeInformation ();
        }

        SwitchToState (CONNECTED_NORMALLY);
        m_handoverEndOkTrace (m_imsi, m_cellId, m_rnti);
      }
      break;

    default:
      NS_FATAL_ERROR ("unexpected event in state " << ToString (m_state));
      break; 
    }
}

void
cv2x_LteUeRrc::DoNotifyRandomAccessFailed ()
{
  NS_LOG_FUNCTION (this << m_imsi << ToString (m_state));
  m_randomAccessErrorTrace (m_imsi, m_cellId, m_rnti);

  switch (m_state)
    {
    case IDLE_RANDOM_ACCESS:
      {
        SwitchToState (IDLE_CAMPED_NORMALLY);
        m_asSapUser->NotifyConnectionFailed ();
      }
      break;

    case CONNECTED_HANDOVER:
      {
        m_handoverEndErrorTrace (m_imsi, m_cellId, m_rnti);
        /**
         * \todo After a handover failure because of a random access failure,
         *       send an RRC Connection Re-establishment and switch to
         *       CONNECTED_REESTABLISHING state.
         */
      }
      break;

    default:
      NS_FATAL_ERROR ("unexpected event in state " << ToString (m_state));
      break;
    }
}


void
cv2x_LteUeRrc::DoSetCsgWhiteList (uint32_t csgId)
{
  NS_LOG_FUNCTION (this << m_imsi << csgId);
  m_csgWhiteList = csgId;
}

void 
cv2x_LteUeRrc::DoStartCellSelection (uint32_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << m_imsi << dlEarfcn);
  NS_ASSERT_MSG (m_state == IDLE_START,
                 "cannot start cell selection from state " << ToString (m_state));
  m_dlEarfcn = dlEarfcn;
  m_cphySapProvider.at(0)->StartCellSearch (dlEarfcn);
  SwitchToState (IDLE_CELL_SEARCH);
}

void 
cv2x_LteUeRrc::DoForceCampedOnEnb (uint16_t cellId, uint32_t dlEarfcn)
{
  NS_LOG_FUNCTION (this << m_imsi << cellId << dlEarfcn);

  switch (m_state)
    {
    case IDLE_START:
      m_cellId = cellId;
      m_dlEarfcn = dlEarfcn;
      m_cphySapProvider.at(0)->SynchronizeWithEnb (m_cellId, m_dlEarfcn);
      SwitchToState (IDLE_WAIT_MIB);
      break;

    case IDLE_CELL_SEARCH:
    case IDLE_WAIT_MIB_SIB1:
    case IDLE_WAIT_SIB1:
      NS_FATAL_ERROR ("cannot abort cell selection " << ToString (m_state));
      break;

    case IDLE_WAIT_MIB:
      NS_LOG_INFO ("already forced to camp to cell " << m_cellId);
      break;

    case IDLE_CAMPED_NORMALLY:
    case IDLE_WAIT_SIB2:
    case IDLE_RANDOM_ACCESS:
    case IDLE_CONNECTING:
      NS_LOG_INFO ("already camped to cell " << m_cellId);
      break;

    case CONNECTED_NORMALLY:
    case CONNECTED_HANDOVER:
    case CONNECTED_PHY_PROBLEM:
    case CONNECTED_REESTABLISHING:
      NS_LOG_INFO ("already connected to cell " << m_cellId);
      break;

    default:
      NS_FATAL_ERROR ("unexpected event in state " << ToString (m_state));
      break;
    }

}

void
cv2x_LteUeRrc::DoConnect ()
{
  NS_LOG_FUNCTION (this << m_imsi);

  switch (m_state)
    {
    case IDLE_START:
    case IDLE_CELL_SEARCH:
    case IDLE_WAIT_MIB_SIB1:
    case IDLE_WAIT_SIB1:
    case IDLE_WAIT_MIB:
      m_connectionPending = true;
      break;

    case IDLE_CAMPED_NORMALLY:
      m_connectionPending = true;
      SwitchToState (IDLE_WAIT_SIB2);
      break;

    case IDLE_WAIT_SIB2:
    case IDLE_RANDOM_ACCESS:
    case IDLE_CONNECTING:
      NS_LOG_INFO ("already connecting");
      break;

    case CONNECTED_NORMALLY:
    case CONNECTED_REESTABLISHING:
    case CONNECTED_HANDOVER:
      NS_LOG_INFO ("already connected");
      break;

    default:
      NS_FATAL_ERROR ("unexpected event in state " << ToString (m_state));
      break;
    }
}



// CPHY SAP methods

void
cv2x_LteUeRrc::DoRecvMasterInformationBlock (uint16_t cellId,
                                        cv2x_LteRrcSap::MasterInformationBlock msg)
{ 
  m_dlBandwidth = msg.dlBandwidth;
  m_cphySapProvider.at(0)->SetDlBandwidth (msg.dlBandwidth);
  m_hasReceivedMib = true;
  m_mibReceivedTrace (m_imsi, m_cellId, m_rnti, cellId);

  switch (m_state)
    {
    case IDLE_WAIT_MIB:
      // manual attachment
      SwitchToState (IDLE_CAMPED_NORMALLY);
      break;

    case IDLE_WAIT_MIB_SIB1:
      // automatic attachment from Idle mode cell selection
      SwitchToState (IDLE_WAIT_SIB1);
      break;

    default:
      // do nothing extra
      break;
    }
}

void
cv2x_LteUeRrc::DoRecvSystemInformationBlockType1 (uint16_t cellId,
                                             cv2x_LteRrcSap::SystemInformationBlockType1 msg)
{
  NS_LOG_FUNCTION (this);
  switch (m_state)
    {
    case IDLE_WAIT_SIB1:
      NS_ASSERT_MSG (cellId == msg.cellAccessRelatedInfo.cellIdentity,
                     "Cell identity in SIB1 does not match with the originating cell");
      m_hasReceivedSib1 = true;
      m_lastSib1 = msg;
      m_sib1ReceivedTrace (m_imsi, m_cellId, m_rnti, cellId);
      EvaluateCellForSelection ();
      break;

    case IDLE_CAMPED_NORMALLY:
    case IDLE_RANDOM_ACCESS:
    case IDLE_CONNECTING:
    case CONNECTED_NORMALLY:
    case CONNECTED_HANDOVER:
    case CONNECTED_PHY_PROBLEM:
    case CONNECTED_REESTABLISHING:
      NS_ASSERT_MSG (cellId == msg.cellAccessRelatedInfo.cellIdentity,
                     "Cell identity in SIB1 does not match with the originating cell");
      m_hasReceivedSib1 = true;
      m_lastSib1 = msg;
      m_sib1ReceivedTrace (m_imsi, m_cellId, m_rnti, cellId);
      break;

    case IDLE_WAIT_MIB_SIB1:
      // MIB has not been received, so ignore this SIB1
      break;

    default: // e.g. IDLE_START, IDLE_CELL_SEARCH, IDLE_WAIT_MIB, IDLE_WAIT_SIB2
      // do nothing
      break;
    }
}

void
cv2x_LteUeRrc::DoReportUeMeasurements (cv2x_LteUeCphySapUser::UeMeasurementsParameters params)
{
  NS_LOG_FUNCTION (this);

  // layer 3 filtering does not apply in IDLE mode
  bool useLayer3Filtering = (m_state == CONNECTED_NORMALLY);
  bool triggering = true;
  std::vector <cv2x_LteUeCphySapUser::UeMeasurementsElement>::iterator newMeasIt;
  for (newMeasIt = params.m_ueMeasurementsList.begin ();
       newMeasIt != params.m_ueMeasurementsList.end (); ++newMeasIt)
    {
      if (params.m_componentCarrierId != 0)
        {
          triggering = false; // report is triggered only when an event is on the primary carrier
          // in this case the measurement received is related to secondary carriers
          // measurements related to secondary carriers are saved on a different portion of memory
          SaveScellUeMeasurements (newMeasIt->m_cellId, newMeasIt->m_rsrp,
                                   newMeasIt->m_rsrq, useLayer3Filtering, 
                                   params.m_componentCarrierId );
        }
      else
        {
          SaveUeMeasurements (newMeasIt->m_cellId, newMeasIt->m_rsrp,
                              newMeasIt->m_rsrq, useLayer3Filtering);
        }
    }

  if (m_state == IDLE_CELL_SEARCH)
    {
      // start decoding BCH
      SynchronizeToStrongestCell ();
    }
  else
    {
      if (triggering)
        {
          std::map<uint8_t, cv2x_LteRrcSap::MeasIdToAddMod>::iterator measIdIt;
          for (measIdIt = m_varMeasConfig.measIdList.begin ();
               measIdIt != m_varMeasConfig.measIdList.end (); ++measIdIt)
            {
              MeasurementReportTriggering (measIdIt->first);
             }
        }
    }

} // end of cv2x_LteUeRrc::DoReportUeMeasurements



// RRC SAP methods

void
cv2x_LteUeRrc::DoCompleteSetup (cv2x_LteUeRrcSapProvider::CompleteSetupParameters params)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);
  m_srb0->m_rlc->SetLteRlcSapUser (params.srb0SapUser);
  if (m_srb1)
    {
      m_srb1->m_pdcp->SetLtePdcpSapUser (params.srb1SapUser);
    }
}


void 
cv2x_LteUeRrc::DoRecvSystemInformation (cv2x_LteRrcSap::SystemInformation msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);

  if (msg.haveSib2)
    {
      switch (m_state)
        {
        case IDLE_CAMPED_NORMALLY:
        case IDLE_WAIT_SIB2:
        case IDLE_RANDOM_ACCESS:
        case IDLE_CONNECTING:
        case CONNECTED_NORMALLY:
        case CONNECTED_HANDOVER:
        case CONNECTED_PHY_PROBLEM:
        case CONNECTED_REESTABLISHING:
          m_hasReceivedSib2 = true;
          m_ulBandwidth = msg.sib2.freqInfo.ulBandwidth;
          m_ulEarfcn = msg.sib2.freqInfo.ulCarrierFreq;
          m_sib2ReceivedTrace (m_imsi, m_cellId, m_rnti);
          cv2x_LteUeCmacSapProvider::RachConfig rc;
          rc.numberOfRaPreambles = msg.sib2.radioResourceConfigCommon.rachConfigCommon.preambleInfo.numberOfRaPreambles;
          rc.preambleTransMax = msg.sib2.radioResourceConfigCommon.rachConfigCommon.raSupervisionInfo.preambleTransMax;
          rc.raResponseWindowSize = msg.sib2.radioResourceConfigCommon.rachConfigCommon.raSupervisionInfo.raResponseWindowSize;
          m_cmacSapProvider.at (0)->ConfigureRach (rc);
          m_cphySapProvider.at (0)->ConfigureUplink (m_ulEarfcn, m_ulBandwidth);
          m_cphySapProvider.at (0)->ConfigureReferenceSignalPower (msg.sib2.radioResourceConfigCommon.pdschConfigCommon.referenceSignalPower);
          if (m_state == IDLE_WAIT_SIB2)
            {
              NS_ASSERT (m_connectionPending);
              StartConnection ();
            }
          break;

        default: // IDLE_START, IDLE_CELL_SEARCH, IDLE_WAIT_MIB, IDLE_WAIT_MIB_SIB1, IDLE_WAIT_SIB1
          // do nothing
          break;
        }
    }
  if (msg.haveSib18 && m_sidelinkConfiguration != NULL && m_sidelinkConfiguration->IsSlEnabled())
    {
      switch (m_state)
        {
        case IDLE_CAMPED_NORMALLY:
        case CONNECTED_NORMALLY:
          {
            NS_LOG_LOGIC (this << " processing SIB18 in state " << m_state);
            //update sidelink information for the cell
            std::map <uint16_t, cv2x_LteUeRrcSl::LteSlCellConfiguration>::iterator it = m_sidelinkConfiguration->m_slMap.find (m_cellId);
            if (it == m_sidelinkConfiguration->m_slMap.end ()) {
              NS_LOG_LOGIC (this << " adding SIB18 for cell " << m_cellId);
              cv2x_LteUeRrcSl::LteSlCellConfiguration cellConfig;
              cellConfig.cellId = m_cellId;
              cellConfig.haveSib18 = true;
              cellConfig.sib18 = msg.sib18;
              m_sidelinkConfiguration->m_slMap.insert (std::pair<uint16_t, cv2x_LteUeRrcSl::LteSlCellConfiguration> (m_cellId, cellConfig));
            } else {
              NS_LOG_LOGIC (this << " updating SIB18 for cell " << m_cellId);
              it->second.haveSib18 = true;
              it->second.sib18 = msg.sib18;
            }
            if (m_sidelinkConfiguration->IsTxInterested() || m_sidelinkConfiguration->IsRxInterested ()) {
              //Inform MAC about the pools to be used
              //TBD
              std::list< Ptr<SidelinkRxCommResourcePool> > pools;
              for (int i = 0 ; i < msg.sib18.commConfig.commRxPool.nbPools; i++)
                {
                  Ptr<SidelinkRxCommResourcePool> pool = NULL;
                  for (uint32_t j = 0 ; j < m_sidelinkConfiguration->rxPools.size() && pool == NULL; j++)
                    {
                      
                      if (msg.sib18.commConfig.commRxPool.pools[i]== m_sidelinkConfiguration->rxPools.at(j).first)
                        {
                          pool = m_sidelinkConfiguration->rxPools.at(j).second;
                        }
                    }
                  if (pool == NULL)
                    {
                      pool = CreateObject <SidelinkRxCommResourcePool> ();
                      pool->SetPool (msg.sib18.commConfig.commRxPool.pools[i]);
                      m_sidelinkConfiguration->rxPools.push_back (std::make_pair(msg.sib18.commConfig.commRxPool.pools[i], pool));
                    }
                  pools.push_back (pool);
                }
              m_cmacSapProvider.at (0)->SetSlRxPools (pools);
              m_cphySapProvider.at (0)->SetSlRxPools (pools);
            }
          }
          break;
        default: // IDLE_START, IDLE_CELL_SEARCH, IDLE_WAIT_MIB, IDLE_WAIT_MIB_SIB1, IDLE_WAIT_SIB1
          // do nothing
          break;
        }
    }

  if (msg.haveSib19 && m_sidelinkConfiguration != NULL && m_sidelinkConfiguration->IsDiscEnabled())
    {
      switch (m_state)
      {
        case IDLE_CAMPED_NORMALLY:
        case CONNECTED_NORMALLY:
        {
          NS_LOG_LOGIC (this << " processing SIB19 in state " << m_state);
          //update sidelink information for the cell
          std::map <uint16_t, cv2x_LteUeRrcSl::LteSlCellConfiguration>::iterator it = m_sidelinkConfiguration->m_slMap.find (m_cellId);
          if (it == m_sidelinkConfiguration->m_slMap.end ()) {
            NS_LOG_LOGIC (this << " adding SIB19 for cell " << m_cellId);
            cv2x_LteUeRrcSl::LteSlCellConfiguration cellConfig;
            cellConfig.cellId = m_cellId;
            cellConfig.haveSib19 = true;
            cellConfig.sib19 = msg.sib19;
            m_sidelinkConfiguration->m_slMap.insert (std::pair<uint16_t, cv2x_LteUeRrcSl::LteSlCellConfiguration> (m_cellId, cellConfig));
          } 
          else 
          {
            NS_LOG_LOGIC (this << " updating SIB19 for cell " << m_cellId);
            it->second.haveSib19 = true;
            it->second.sib19 = msg.sib19;
          }
          if (m_sidelinkConfiguration->IsMonitoringInterested() || m_sidelinkConfiguration->IsAnnouncingInterested ()) 
          {
            
            std::list< Ptr<SidelinkRxDiscResourcePool> > pools;
            for (int i = 0 ; i < msg.sib19.discConfig.discRxPool.nbPools; i++)
            {
              Ptr<SidelinkRxDiscResourcePool> pool = NULL;
              for (uint32_t j = 0 ; j < m_sidelinkConfiguration->monitorPools.size() && pool == NULL; j++)
              {
                if (msg.sib19.discConfig.discRxPool.pools[i]== m_sidelinkConfiguration->monitorPools.at(j).first)
                {
                  pool = m_sidelinkConfiguration->monitorPools.at(j).second;
                }
              }                
              if (pool == NULL)
              {
                pool = CreateObject <SidelinkRxDiscResourcePool> ();
                pool->SetPool (msg.sib19.discConfig.discRxPool.pools[i]);
                m_sidelinkConfiguration->monitorPools.push_back (std::make_pair(msg.sib19.discConfig.discRxPool.pools[i], pool));
              }
              pools.push_back (pool);
            }
          m_cmacSapProvider.at (0)->SetSlRxPools (pools);
          m_cphySapProvider.at (0)->SetSlRxPools (pools);
          }
        }//end case
          break;
        default: // IDLE_START, IDLE_CELL_SEARCH, IDLE_WAIT_MIB, IDLE_WAIT_MIB_SIB1, IDLE_WAIT_SIB1
            // do nothing
          break;
      }//end switch
    }//end if
}


void 
cv2x_LteUeRrc::DoRecvRrcConnectionSetup (cv2x_LteRrcSap::RrcConnectionSetup msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);
  switch (m_state)
    {
    case IDLE_CONNECTING:
      {
        ApplyRadioResourceConfigDedicated (msg.radioResourceConfigDedicated);
        m_connectionTimeout.Cancel ();
        SwitchToState (CONNECTED_NORMALLY);
        cv2x_LteRrcSap::RrcConnectionSetupCompleted msg2;
        msg2.rrcTransactionIdentifier = msg.rrcTransactionIdentifier;
        m_rrcSapUser->SendRrcConnectionSetupCompleted (msg2);
        m_asSapUser->NotifyConnectionSuccessful ();
        m_connectionEstablishedTrace (m_imsi, m_cellId, m_rnti);
      }
      break;

    default:
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void
cv2x_LteUeRrc::DoRecvRrcConnectionReconfiguration (cv2x_LteRrcSap::RrcConnectionReconfiguration msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);
  NS_LOG_INFO ("DoRecvRrcConnectionReconfiguration haveNonCriticalExtension:" << msg.haveNonCriticalExtension );
  switch (m_state)
    {
    case CONNECTED_NORMALLY:
      if (msg.haveMobilityControlInfo)
        {
          NS_LOG_INFO ("haveMobilityControlInfo == true");
          SwitchToState (CONNECTED_HANDOVER);
          const cv2x_LteRrcSap::MobilityControlInfo& mci = msg.mobilityControlInfo;
          m_handoverStartTrace (m_imsi, m_cellId, m_rnti, mci.targetPhysCellId);
          m_cmacSapProvider.at(0)->Reset ();
          m_cphySapProvider.at(0)->Reset ();
          m_cellId = mci.targetPhysCellId;
          NS_ASSERT (mci.haveCarrierFreq);
          NS_ASSERT (mci.haveCarrierBandwidth);
          m_cphySapProvider.at(0)->SynchronizeWithEnb (m_cellId, mci.carrierFreq.dlCarrierFreq);
          m_cphySapProvider.at(0)->SetDlBandwidth ( mci.carrierBandwidth.dlBandwidth);
          m_cphySapProvider.at(0)->ConfigureUplink (mci.carrierFreq.ulCarrierFreq, mci.carrierBandwidth.ulBandwidth);
          m_rnti = msg.mobilityControlInfo.newUeIdentity;
          m_srb0->m_rlc->SetRnti (m_rnti);
          NS_ASSERT_MSG (mci.haveRachConfigDedicated, "handover is only supported with non-contention-based random access procedure");
          m_cmacSapProvider.at(0)->StartNonContentionBasedRandomAccessProcedure (m_rnti, mci.rachConfigDedicated.raPreambleIndex, mci.rachConfigDedicated.raPrachMaskIndex);
          m_cphySapProvider.at(0)->SetRnti (m_rnti);
          m_lastRrcTransactionIdentifier = msg.rrcTransactionIdentifier;
          NS_ASSERT (msg.haveRadioResourceConfigDedicated);

          // we re-establish SRB1 by creating a new entity
          // note that we can't dispose the old entity now, because
          // it's in the current stack, so we would corrupt the stack
          // if we did so. Hence we schedule it for later disposal
          m_srb1Old = m_srb1;
          Simulator::ScheduleNow (&cv2x_LteUeRrc::DisposeOldSrb1, this);
          m_srb1 = 0; // new instance will be be created within ApplyRadioResourceConfigDedicated

          m_drbMap.clear (); // dispose all DRBs
          ApplyRadioResourceConfigDedicated (msg.radioResourceConfigDedicated);

          if (msg.haveMeasConfig)
            {
              ApplyMeasConfig (msg.measConfig);
            }
          // RRC connection reconfiguration completed will be sent
          // after handover is complete
        }
      else
        {
          NS_LOG_INFO ("haveMobilityControlInfo == false");
          if (msg.haveNonCriticalExtension)
            {
              ApplyRadioResourceConfigDedicatedSecondaryCarrier (msg.nonCriticalExtension);
              NS_LOG_FUNCTION ( this << "RNTI " << m_rnti << " Configured for CA" );
            }
          if (msg.haveRadioResourceConfigDedicated)
            {
              ApplyRadioResourceConfigDedicated (msg.radioResourceConfigDedicated);
            } 
          if (msg.haveMeasConfig)
            {
              ApplyMeasConfig (msg.measConfig);
            }
          //check if it has sidelink resource information
          if (msg.haveSlCommConfig)
            {
              ApplySidelinkDedicatedConfiguration (msg.slCommConfig);
            }
          //check if it has discovery resource information
          if (msg.haveSlDiscConfig)
            {
              ApplySidelinkDedicatedConfiguration (msg.slDiscConfig);
            }
          cv2x_LteRrcSap::RrcConnectionReconfigurationCompleted msg2;
          msg2.rrcTransactionIdentifier = msg.rrcTransactionIdentifier;
          m_rrcSapUser->SendRrcConnectionReconfigurationCompleted (msg2);
          m_connectionReconfigurationTrace (m_imsi, m_cellId, m_rnti);
        }
      break;

    default:
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void 
cv2x_LteUeRrc::DoRecvRrcConnectionReestablishment (cv2x_LteRrcSap::RrcConnectionReestablishment msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);
  switch (m_state)
    {
    case CONNECTED_REESTABLISHING:
      {
        /**
         * \todo After receiving RRC Connection Re-establishment, stop timer
         *       T301, fire a new trace source, reply with RRC Connection
         *       Re-establishment Complete, and finally switch to
         *       CONNECTED_NORMALLY state. See Section 5.3.7.5 of 3GPP TS
         *       36.331.
         */

        //if target cell sending SIB 18 or 19 and SidelinkUEInformation was sent in the last second
        //with interest/resource request,
        //initiate transmission of the SidelinkUEInformation message in accordance with 5.X.2.3
        if ((m_sidelinkConfiguration->IsCellBroadcastingSIB18 (m_cellId) || m_sidelinkConfiguration->IsCellBroadcastingSIB19 (m_cellId)) && m_sidelinkConfiguration->GetTimeSinceLastTransmissionOfSidelinkUeInformation () < 1.0)
          {
            SendSidelinkUeInformation ();
          }
      }
      break;

    default:
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void 
cv2x_LteUeRrc::DoRecvRrcConnectionReestablishmentReject (cv2x_LteRrcSap::RrcConnectionReestablishmentReject msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);
  switch (m_state)
    {
    case CONNECTED_REESTABLISHING:
      {
        /**
         * \todo After receiving RRC Connection Re-establishment Reject, stop
         *       timer T301. See Section 5.3.7.8 of 3GPP TS 36.331.
         */
        LeaveConnectedMode ();
      }
      break;

    default:
      NS_FATAL_ERROR ("method unexpected in state " << ToString (m_state));
      break;
    }
}

void 
cv2x_LteUeRrc::DoRecvRrcConnectionRelease (cv2x_LteRrcSap::RrcConnectionRelease msg)
{
  NS_LOG_FUNCTION (this << " RNTI " << m_rnti);
  /// \todo Currently not implemented, see Section 5.3.8 of 3GPP TS 36.331.
}

void 
cv2x_LteUeRrc::DoRecvRrcConnectionReject (cv2x_LteRrcSap::RrcConnectionReject msg)
{
  NS_LOG_FUNCTION (this);
  m_connectionTimeout.Cancel ();

  m_cmacSapProvider.at (0)->Reset ();       // reset the MAC
  m_hasReceivedSib2 = false;         // invalidate the previously received SIB2
  SwitchToState (IDLE_CAMPED_NORMALLY);
  m_asSapUser->NotifyConnectionFailed ();  // inform upper layer
}



void
cv2x_LteUeRrc::SynchronizeToStrongestCell ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_state == IDLE_CELL_SEARCH);

  uint16_t maxRsrpCellId = 0;
  double maxRsrp = -std::numeric_limits<double>::infinity ();

  std::map<uint16_t, MeasValues>::iterator it;
  for (it = m_storedMeasValues.begin (); it != m_storedMeasValues.end (); it++)
    {
      /*
       * This block attempts to find a cell with strongest RSRP and has not
       * yet been identified as "acceptable cell".
       */
      if (maxRsrp < it->second.rsrp)
        {
          std::set<uint16_t>::const_iterator itCell;
          itCell = m_acceptableCell.find (it->first);
          if (itCell == m_acceptableCell.end ())
            {
              maxRsrpCellId = it->first;
              maxRsrp = it->second.rsrp;
            }
        }
    }

  if (maxRsrpCellId == 0)
    {
      NS_LOG_WARN (this << " Cell search is unable to detect surrounding cell to attach to");
    }
  else
    {
      NS_LOG_LOGIC (this << " cell " << maxRsrpCellId
                         << " is the strongest untried surrounding cell");
      m_cphySapProvider.at(0)->SynchronizeWithEnb (maxRsrpCellId, m_dlEarfcn);
      SwitchToState (IDLE_WAIT_MIB_SIB1);
    }

} // end of void cv2x_LteUeRrc::SynchronizeToStrongestCell ()


void
cv2x_LteUeRrc::EvaluateCellForSelection ()
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_state == IDLE_WAIT_SIB1);
  NS_ASSERT (m_hasReceivedMib);
  NS_ASSERT (m_hasReceivedSib1);
  uint16_t cellId = m_lastSib1.cellAccessRelatedInfo.cellIdentity;

  // Cell selection criteria evaluation

  bool isSuitableCell = false;
  bool isAcceptableCell = false;
  std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.find (cellId);
  double qRxLevMeas = storedMeasIt->second.rsrp;
  double qRxLevMin = cv2x_EutranMeasurementMapping::IeValue2ActualQRxLevMin (m_lastSib1.cellSelectionInfo.qRxLevMin);
  NS_LOG_LOGIC (this << " cell selection to cellId=" << cellId
                     << " qrxlevmeas=" << qRxLevMeas << " dBm"
                     << " qrxlevmin=" << qRxLevMin << " dBm");

  if (qRxLevMeas - qRxLevMin > 0)
    {
      isAcceptableCell = true;

      uint32_t cellCsgId = m_lastSib1.cellAccessRelatedInfo.csgIdentity;
      bool cellCsgIndication = m_lastSib1.cellAccessRelatedInfo.csgIndication;

      isSuitableCell = (cellCsgIndication == false) || (cellCsgId == m_csgWhiteList);

      NS_LOG_LOGIC (this << " csg(ue/cell/indication)=" << m_csgWhiteList << "/"
                         << cellCsgId << "/" << cellCsgIndication);
    }

  // Cell selection decision

  if (isSuitableCell)
    {
      m_cellId = cellId;
      m_cphySapProvider.at(0)->SynchronizeWithEnb (cellId, m_dlEarfcn);
      m_cphySapProvider.at(0)->SetDlBandwidth (m_dlBandwidth);
      m_initialCellSelectionEndOkTrace (m_imsi, cellId);
      SwitchToState (IDLE_CAMPED_NORMALLY);
    }
  else
    {
      // ignore the MIB and SIB1 received from this cell
      m_hasReceivedMib = false;
      m_hasReceivedSib1 = false;

      m_initialCellSelectionEndErrorTrace (m_imsi, cellId);

      if (isAcceptableCell)
        {
          /*
           * The cells inserted into this list will not be considered for
           * subsequent cell search attempt.
           */
          m_acceptableCell.insert (cellId);
        }

      SwitchToState (IDLE_CELL_SEARCH);
      SynchronizeToStrongestCell (); // retry to a different cell
    }

} // end of void cv2x_LteUeRrc::EvaluateCellForSelection ()


void
cv2x_LteUeRrc::ApplyRadioResourceConfigDedicatedSecondaryCarrier (cv2x_LteRrcSap::NonCriticalExtensionConfiguration nonCec)
{
  NS_LOG_FUNCTION (this);

  m_sCellToAddModList = nonCec.sCellsToAddModList;

  for(std::list<cv2x_LteRrcSap::SCellToAddMod>::iterator it = nonCec.sCellsToAddModList.begin(); it!=nonCec.sCellsToAddModList.end(); it++)
    {
      cv2x_LteRrcSap::SCellToAddMod scell = *it;
      uint8_t ccId = scell.sCellIndex;


      uint16_t physCellId = scell.cellIdentification.physCellId;
      uint8_t ulBand = scell.radioResourceConfigCommonSCell.ulConfiguration.ulFreqInfo.ulBandwidth;
      uint32_t ulEarfcn = scell.radioResourceConfigCommonSCell.ulConfiguration.ulFreqInfo.ulCarrierFreq;
      uint8_t dlBand = scell.radioResourceConfigCommonSCell.nonUlConfiguration.dlBandwidth;
      uint32_t dlEarfcn = scell.cellIdentification.dlCarrierFreq;
      uint8_t txMode = scell.radioResourceConfigDedicateSCell.physicalConfigDedicatedSCell.antennaInfo.transmissionMode;
      uint8_t srsIndex = scell.radioResourceConfigDedicateSCell.physicalConfigDedicatedSCell.soundingRsUlConfigDedicated.srsConfigIndex;

      m_cphySapProvider.at (ccId)->SynchronizeWithEnb (physCellId, dlEarfcn);
      m_cphySapProvider.at (ccId)->SetDlBandwidth (dlBand);
      m_cphySapProvider.at (ccId)->ConfigureUplink (ulEarfcn, ulBand);
      m_cphySapProvider.at (ccId)->ConfigureReferenceSignalPower (scell.radioResourceConfigCommonSCell.nonUlConfiguration.pdschConfigCommon.referenceSignalPower);
      m_cphySapProvider.at (ccId)->SetTransmissionMode (txMode);
      m_cphySapProvider.at (ccId)->SetRnti(m_rnti);
      m_cmacSapProvider.at (ccId)->SetRnti(m_rnti);
      // update PdschConfigDedicated (i.e. P_A value)
      cv2x_LteRrcSap::PdschConfigDedicated pdschConfigDedicated = scell.radioResourceConfigDedicateSCell.physicalConfigDedicatedSCell.pdschConfigDedicated;
      double paDouble = cv2x_LteRrcSap::ConvertPdschConfigDedicated2Double (pdschConfigDedicated);
      m_cphySapProvider.at (ccId)->SetPa (paDouble);
      m_cphySapProvider.at (ccId)->SetSrsConfigurationIndex (srsIndex);
    }

  m_sCarrierConfiguredTrace (this, m_sCellToAddModList);
}

void 
cv2x_LteUeRrc::ApplyRadioResourceConfigDedicated (cv2x_LteRrcSap::RadioResourceConfigDedicated rrcd)
{
  NS_LOG_FUNCTION (this);
  const struct cv2x_LteRrcSap::PhysicalConfigDedicated& pcd = rrcd.physicalConfigDedicated;

  if (pcd.haveAntennaInfoDedicated)
    {
      m_cphySapProvider.at(0)->SetTransmissionMode (pcd.antennaInfo.transmissionMode);
    }
  if (pcd.haveSoundingRsUlConfigDedicated)
    {
      m_cphySapProvider.at(0)->SetSrsConfigurationIndex (pcd.soundingRsUlConfigDedicated.srsConfigIndex);
    }

  if (pcd.havePdschConfigDedicated)
    {
      // update PdschConfigDedicated (i.e. P_A value)
      m_pdschConfigDedicated = pcd.pdschConfigDedicated;
      double paDouble = cv2x_LteRrcSap::ConvertPdschConfigDedicated2Double (m_pdschConfigDedicated);
      m_cphySapProvider.at(0)->SetPa (paDouble);
    }

  std::list<cv2x_LteRrcSap::SrbToAddMod>::const_iterator stamIt = rrcd.srbToAddModList.begin ();
  if (stamIt != rrcd.srbToAddModList.end ())
    {
      if (m_srb1 == 0)
        {
          // SRB1 not setup yet        
          NS_ASSERT_MSG ((m_state == IDLE_CONNECTING) || (m_state == CONNECTED_HANDOVER), 
                         "unexpected state " << ToString (m_state));
          NS_ASSERT_MSG (stamIt->srbIdentity == 1, "only SRB1 supported");

          const uint8_t lcid = 1; // fixed LCID for SRB1

          Ptr<cv2x_LteRlc> rlc = CreateObject<cv2x_LteRlcAm> ();
          rlc->SetLteMacSapProvider (m_macSapProvider);
          rlc->SetRnti (m_rnti);
          rlc->SetLcId (lcid);      

          Ptr<cv2x_LtePdcp> pdcp = CreateObject<cv2x_LtePdcp> ();
          pdcp->SetRnti (m_rnti);
          pdcp->SetLcId (lcid);
          pdcp->SetLtePdcpSapUser (m_drbPdcpSapUser);
          pdcp->SetLteRlcSapProvider (rlc->GetLteRlcSapProvider ());
          rlc->SetLteRlcSapUser (pdcp->GetLteRlcSapUser ());

          m_srb1 = CreateObject<cv2x_LteSignalingRadioBearerInfo> ();
          m_srb1->m_rlc = rlc;
          m_srb1->m_pdcp = pdcp;
          m_srb1->m_srbIdentity = 1;
          
          m_srb1->m_logicalChannelConfig.priority = stamIt->logicalChannelConfig.priority;
          m_srb1->m_logicalChannelConfig.prioritizedBitRateKbps = stamIt->logicalChannelConfig.prioritizedBitRateKbps;
          m_srb1->m_logicalChannelConfig.bucketSizeDurationMs = stamIt->logicalChannelConfig.bucketSizeDurationMs;
          m_srb1->m_logicalChannelConfig.logicalChannelGroup = stamIt->logicalChannelConfig.logicalChannelGroup;

          struct cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig;
          lcConfig.priority = stamIt->logicalChannelConfig.priority;
          lcConfig.prioritizedBitRateKbps = stamIt->logicalChannelConfig.prioritizedBitRateKbps;
          lcConfig.bucketSizeDurationMs = stamIt->logicalChannelConfig.bucketSizeDurationMs;
          lcConfig.logicalChannelGroup = stamIt->logicalChannelConfig.logicalChannelGroup;
          m_cmacSapProvider.at (0)->AddLc (lcid, lcConfig, rlc->GetLteMacSapUser ());
          ++stamIt;
          NS_ASSERT_MSG (stamIt == rrcd.srbToAddModList.end (), "at most one SrbToAdd supported");
          
          cv2x_LteUeRrcSapUser::SetupParameters ueParams;
          ueParams.srb0SapProvider = m_srb0->m_rlc->GetLteRlcSapProvider ();
          ueParams.srb1SapProvider = m_srb1->m_pdcp->GetLtePdcpSapProvider ();
          m_rrcSapUser->Setup (ueParams);
        }
      else
        {
          NS_LOG_INFO ("request to modify SRB1 (skipping as currently not implemented)");
          // would need to modify m_srb1, and then propagate changes to the MAC
        }
    }


  std::list<cv2x_LteRrcSap::DrbToAddMod>::const_iterator dtamIt;
  for (dtamIt = rrcd.drbToAddModList.begin ();
       dtamIt != rrcd.drbToAddModList.end ();
       ++dtamIt)
    {
      NS_LOG_INFO (this << " IMSI " << m_imsi << " adding/modifying DRBID " << (uint32_t) dtamIt->drbIdentity << " LC " << (uint32_t) dtamIt->logicalChannelIdentity);
      NS_ASSERT_MSG (dtamIt->logicalChannelIdentity > 2, "LCID value " << dtamIt->logicalChannelIdentity << " is reserved for SRBs");

      std::map<uint8_t, Ptr<cv2x_LteDataRadioBearerInfo> >::iterator drbMapIt = m_drbMap.find (dtamIt->drbIdentity);
      if (drbMapIt == m_drbMap.end ())
        {
          NS_LOG_INFO ("New Data Radio Bearer");
        
          TypeId rlcTypeId;
          if (m_useRlcSm)
            {
              rlcTypeId = cv2x_LteRlcSm::GetTypeId ();
            }
          else
            {
              switch (dtamIt->rlcConfig.choice)
                {
                case cv2x_LteRrcSap::RlcConfig::AM: 
                  rlcTypeId = cv2x_LteRlcAm::GetTypeId ();
                  break;
          
                case cv2x_LteRrcSap::RlcConfig::UM_BI_DIRECTIONAL: 
                  rlcTypeId = cv2x_LteRlcUm::GetTypeId ();
                  break;
          
                default:
                  NS_FATAL_ERROR ("unsupported RLC configuration");
                  break;                
                }
            }
  
          ObjectFactory rlcObjectFactory;
          rlcObjectFactory.SetTypeId (rlcTypeId);
          Ptr<cv2x_LteRlc> rlc = rlcObjectFactory.Create ()->GetObject<cv2x_LteRlc> ();
          rlc->SetLteMacSapProvider (m_macSapProvider);
          rlc->SetRnti (m_rnti);
          rlc->SetLcId (dtamIt->logicalChannelIdentity);

          Ptr<cv2x_LteDataRadioBearerInfo> drbInfo = CreateObject<cv2x_LteDataRadioBearerInfo> ();
          drbInfo->m_rlc = rlc;
          drbInfo->m_epsBearerIdentity = dtamIt->epsBearerIdentity;
          drbInfo->m_logicalChannelIdentity = dtamIt->logicalChannelIdentity;
          drbInfo->m_drbIdentity = dtamIt->drbIdentity;
 
          // we need PDCP only for real RLC, i.e., RLC/UM or RLC/AM
          // if we are using RLC/SM we don't care of anything above RLC
          if (rlcTypeId != cv2x_LteRlcSm::GetTypeId ())
            {
              Ptr<cv2x_LtePdcp> pdcp = CreateObject<cv2x_LtePdcp> ();
              pdcp->SetRnti (m_rnti);
              pdcp->SetLcId (dtamIt->logicalChannelIdentity);
              pdcp->SetLtePdcpSapUser (m_drbPdcpSapUser);
              pdcp->SetLteRlcSapProvider (rlc->GetLteRlcSapProvider ());
              rlc->SetLteRlcSapUser (pdcp->GetLteRlcSapUser ());
              drbInfo->m_pdcp = pdcp;
            }

          m_bid2DrbidMap[dtamIt->epsBearerIdentity] = dtamIt->drbIdentity;
  
          m_drbMap.insert (std::pair<uint8_t, Ptr<cv2x_LteDataRadioBearerInfo> > (dtamIt->drbIdentity, drbInfo));
  

          struct cv2x_LteUeCmacSapProvider::LogicalChannelConfig lcConfig;
          lcConfig.priority = dtamIt->logicalChannelConfig.priority;
          lcConfig.prioritizedBitRateKbps = dtamIt->logicalChannelConfig.prioritizedBitRateKbps;
          lcConfig.bucketSizeDurationMs = dtamIt->logicalChannelConfig.bucketSizeDurationMs;
          lcConfig.logicalChannelGroup = dtamIt->logicalChannelConfig.logicalChannelGroup;      

          for (uint32_t i = 0; i < m_numberOfComponentCarriers; i++)
          {
            m_cmacSapProvider.at (i)->AddLc (dtamIt->logicalChannelIdentity,
                                    lcConfig,
                                    rlc->GetLteMacSapUser ());
            // added function to handle priority in the lac layer of UE 
            m_cmacSapProvider.at (i)->AddLCPriority (m_rnti,
                                                     dtamIt->logicalChannelIdentity,
                                                     dtamIt->logicalChannelConfig.priority);

          }
          rlc->Initialize ();
        }
      else
        {
          NS_LOG_INFO ("request to modify existing DRBID");
          Ptr<cv2x_LteDataRadioBearerInfo> drbInfo = drbMapIt->second;
          /// \todo currently not implemented. Would need to modify drbInfo, and then propagate changes to the MAC
        }
    }
  
  std::list<uint8_t>::iterator dtdmIt;
  for (dtdmIt = rrcd.drbToReleaseList.begin ();
       dtdmIt != rrcd.drbToReleaseList.end ();
       ++dtdmIt)
    {
      uint8_t drbid = *dtdmIt;
      NS_LOG_INFO (this << " IMSI " << m_imsi << " releasing DRB " << (uint32_t) drbid << drbid);
      std::map<uint8_t, Ptr<cv2x_LteDataRadioBearerInfo> >::iterator it =   m_drbMap.find (drbid);
      NS_ASSERT_MSG (it != m_drbMap.end (), "could not find bearer with given lcid");
      m_drbMap.erase (it);      
      m_bid2DrbidMap.erase (drbid);
      //Remove LCID
      for (uint32_t i = 0; i < m_numberOfComponentCarriers; i++)
       {
         m_cmacSapProvider.at (i)->RemoveLc (drbid + 2);
       }
    }
}


void 
cv2x_LteUeRrc::ApplyMeasConfig (cv2x_LteRrcSap::MeasConfig mc)
{
  NS_LOG_FUNCTION (this);

  // perform the actions specified in 3GPP TS 36.331 section 5.5.2.1 

  // 3GPP TS 36.331 section 5.5.2.4 Measurement object removal
  for (std::list<uint8_t>::iterator it = mc.measObjectToRemoveList.begin ();
       it !=  mc.measObjectToRemoveList.end ();
       ++it)
    {
      uint8_t measObjectId = *it;
      NS_LOG_LOGIC (this << " deleting measObjectId " << (uint32_t)  measObjectId);
      m_varMeasConfig.measObjectList.erase (measObjectId);
      std::map<uint8_t, cv2x_LteRrcSap::MeasIdToAddMod>::iterator measIdIt = m_varMeasConfig.measIdList.begin ();
      while (measIdIt != m_varMeasConfig.measIdList.end ())
        {
          if (measIdIt->second.measObjectId == measObjectId)
            {
              uint8_t measId = measIdIt->second.measId;
              NS_ASSERT (measId == measIdIt->first);
              NS_LOG_LOGIC (this << " deleting measId " << (uint32_t) measId << " because referring to measObjectId " << (uint32_t)  measObjectId);
              // note: postfix operator preserves iterator validity
              m_varMeasConfig.measIdList.erase (measIdIt++);
              VarMeasReportListClear (measId);
            }
          else
            {
              ++measIdIt;
            }
        }

    }

  // 3GPP TS 36.331 section 5.5.2.5  Measurement object addition/ modification
  for (std::list<cv2x_LteRrcSap::MeasObjectToAddMod>::iterator it = mc.measObjectToAddModList.begin ();
       it !=  mc.measObjectToAddModList.end ();
       ++it)
    {
      // simplifying assumptions
      NS_ASSERT_MSG (it->measObjectEutra.cellsToRemoveList.empty (), "cellsToRemoveList not supported");
      NS_ASSERT_MSG (it->measObjectEutra.cellsToAddModList.empty (), "cellsToAddModList not supported");
      NS_ASSERT_MSG (it->measObjectEutra.cellsToRemoveList.empty (), "blackCellsToRemoveList not supported");
      NS_ASSERT_MSG (it->measObjectEutra.blackCellsToAddModList.empty (), "blackCellsToAddModList not supported");
      NS_ASSERT_MSG (it->measObjectEutra.haveCellForWhichToReportCGI == false, "cellForWhichToReportCGI is not supported");

      uint8_t measObjectId = it->measObjectId;
      std::map<uint8_t, cv2x_LteRrcSap::MeasObjectToAddMod>::iterator measObjectIt = m_varMeasConfig.measObjectList.find (measObjectId);
      if (measObjectIt != m_varMeasConfig.measObjectList.end ())
        {
          NS_LOG_LOGIC ("measObjectId " << (uint32_t) measObjectId << " exists, updating entry");
          measObjectIt->second = *it;
          for (std::map<uint8_t, cv2x_LteRrcSap::MeasIdToAddMod>::iterator measIdIt 
                 = m_varMeasConfig.measIdList.begin ();
               measIdIt != m_varMeasConfig.measIdList.end ();
               ++measIdIt)
            {
              if (measIdIt->second.measObjectId == measObjectId)
                {
                  uint8_t measId = measIdIt->second.measId;
                  NS_LOG_LOGIC (this << " found measId " << (uint32_t) measId << " referring to measObjectId " << (uint32_t)  measObjectId);
                  VarMeasReportListClear (measId);
                }
            }
        }
      else
        {
          NS_LOG_LOGIC ("measObjectId " << (uint32_t) measObjectId << " is new, adding entry");
          m_varMeasConfig.measObjectList[measObjectId] = *it;
        }

    }

  // 3GPP TS 36.331 section 5.5.2.6 Reporting configuration removal
  for (std::list<uint8_t>::iterator it = mc.reportConfigToRemoveList.begin ();
       it !=  mc.reportConfigToRemoveList.end ();
       ++it)
    {
      uint8_t reportConfigId = *it;
      NS_LOG_LOGIC (this << " deleting reportConfigId " << (uint32_t)  reportConfigId);
      m_varMeasConfig.reportConfigList.erase (reportConfigId);
      std::map<uint8_t, cv2x_LteRrcSap::MeasIdToAddMod>::iterator measIdIt = m_varMeasConfig.measIdList.begin ();
      while (measIdIt != m_varMeasConfig.measIdList.end ())
        {
          if (measIdIt->second.reportConfigId == reportConfigId)
            {
              uint8_t measId = measIdIt->second.measId;
              NS_ASSERT (measId == measIdIt->first);
              NS_LOG_LOGIC (this << " deleting measId " << (uint32_t) measId << " because referring to reportConfigId " << (uint32_t)  reportConfigId);
              // note: postfix operator preserves iterator validity
              m_varMeasConfig.measIdList.erase (measIdIt++);
              VarMeasReportListClear (measId);
            }
          else
            {
              ++measIdIt;
            }
        }

    }

  // 3GPP TS 36.331 section 5.5.2.7 Reporting configuration addition/ modification
  for (std::list<cv2x_LteRrcSap::ReportConfigToAddMod>::iterator it = mc.reportConfigToAddModList.begin ();
       it !=  mc.reportConfigToAddModList.end ();
       ++it)
    {
      // simplifying assumptions
      NS_ASSERT_MSG (it->reportConfigEutra.triggerType == cv2x_LteRrcSap::ReportConfigEutra::EVENT,
                     "only trigger type EVENT is supported");

      uint8_t reportConfigId = it->reportConfigId;
      std::map<uint8_t, cv2x_LteRrcSap::ReportConfigToAddMod>::iterator reportConfigIt = m_varMeasConfig.reportConfigList.find (reportConfigId);
      if (reportConfigIt != m_varMeasConfig.reportConfigList.end ())
        {
          NS_LOG_LOGIC ("reportConfigId " << (uint32_t) reportConfigId << " exists, updating entry");
          m_varMeasConfig.reportConfigList[reportConfigId] = *it;
          for (std::map<uint8_t, cv2x_LteRrcSap::MeasIdToAddMod>::iterator measIdIt 
                 = m_varMeasConfig.measIdList.begin ();
               measIdIt != m_varMeasConfig.measIdList.end ();
               ++measIdIt)
            {
              if (measIdIt->second.reportConfigId == reportConfigId)
                {
                  uint8_t measId = measIdIt->second.measId;
                  NS_LOG_LOGIC (this << " found measId " << (uint32_t) measId << " referring to reportConfigId " << (uint32_t)  reportConfigId);
                  VarMeasReportListClear (measId);
                }
            }
        }
      else
        {
          NS_LOG_LOGIC ("reportConfigId " << (uint32_t) reportConfigId << " is new, adding entry");
          m_varMeasConfig.reportConfigList[reportConfigId] = *it;
        }

    }

  // 3GPP TS 36.331 section 5.5.2.8 Quantity configuration
  if (mc.haveQuantityConfig)
    {
      NS_LOG_LOGIC (this << " setting quantityConfig");
      m_varMeasConfig.quantityConfig = mc.quantityConfig;
      // we calculate here the coefficient a used for Layer 3 filtering, see 3GPP TS 36.331 section 5.5.3.2
      m_varMeasConfig.aRsrp = std::pow (0.5, mc.quantityConfig.filterCoefficientRSRP / 4.0);
      m_varMeasConfig.aRsrq = std::pow (0.5, mc.quantityConfig.filterCoefficientRSRQ / 4.0);
      NS_LOG_LOGIC (this << " new filter coefficients: aRsrp=" << m_varMeasConfig.aRsrp << ", aRsrq=" << m_varMeasConfig.aRsrq);

      for (std::map<uint8_t, cv2x_LteRrcSap::MeasIdToAddMod>::iterator measIdIt
             = m_varMeasConfig.measIdList.begin ();
           measIdIt != m_varMeasConfig.measIdList.end ();
           ++measIdIt)
        {
          VarMeasReportListClear (measIdIt->second.measId);
        }
    }

  // 3GPP TS 36.331 section 5.5.2.2 Measurement identity removal
  for (std::list<uint8_t>::iterator it = mc.measIdToRemoveList.begin ();
       it !=  mc.measIdToRemoveList.end ();
       ++it)
    {
      uint8_t measId = *it;
      NS_LOG_LOGIC (this << " deleting measId " << (uint32_t) measId);
      m_varMeasConfig.measIdList.erase (measId);
      VarMeasReportListClear (measId);

      // removing time-to-trigger queues
      m_enteringTriggerQueue.erase (measId);
      m_leavingTriggerQueue.erase (measId);
    }

  // 3GPP TS 36.331 section 5.5.2.3 Measurement identity addition/ modification
  for (std::list<cv2x_LteRrcSap::MeasIdToAddMod>::iterator it = mc.measIdToAddModList.begin ();
       it !=  mc.measIdToAddModList.end ();
       ++it)
    {
      NS_LOG_LOGIC (this << " measId " << (uint32_t) it->measId
                         << " (measObjectId=" << (uint32_t) it->measObjectId
                         << ", reportConfigId=" << (uint32_t) it->reportConfigId
                         << ")");
      NS_ASSERT (m_varMeasConfig.measObjectList.find (it->measObjectId)
                 != m_varMeasConfig.measObjectList.end ());
      NS_ASSERT (m_varMeasConfig.reportConfigList.find (it->reportConfigId)
                 != m_varMeasConfig.reportConfigList.end ());
      m_varMeasConfig.measIdList[it->measId] = *it; // side effect: create new entry if not exists
      std::map<uint8_t, VarMeasReport>::iterator measReportIt = m_varMeasReportList.find (it->measId);
      if (measReportIt != m_varMeasReportList.end ())
        {
          measReportIt->second.periodicReportTimer.Cancel ();
          m_varMeasReportList.erase (measReportIt);
        }
      NS_ASSERT (m_varMeasConfig.reportConfigList.find (it->reportConfigId)
                 ->second.reportConfigEutra.triggerType != cv2x_LteRrcSap::ReportConfigEutra::PERIODICAL);

      // new empty queues for time-to-trigger
      std::list<PendingTrigger_t> s;
      m_enteringTriggerQueue[it->measId] = s;
      m_leavingTriggerQueue[it->measId] = s;
    }

  if (mc.haveMeasGapConfig)
    {
      NS_FATAL_ERROR ("measurement gaps are currently not supported");
    }

  if (mc.haveSmeasure)
    {
      NS_FATAL_ERROR ("s-measure is currently not supported");
    }

  if (mc.haveSpeedStatePars)
    {
      NS_FATAL_ERROR ("SpeedStatePars are currently not supported");
    }
}

void
cv2x_LteUeRrc::SaveUeMeasurements (uint16_t cellId, double rsrp, double rsrq,
                              bool useLayer3Filtering)
{
  NS_LOG_FUNCTION (this << cellId << rsrp << rsrq << useLayer3Filtering);

  std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.find (cellId);

  if (storedMeasIt != m_storedMeasValues.end ())
    {
      if (useLayer3Filtering)
        {
          // F_n = (1-a) F_{n-1} + a M_n
          storedMeasIt->second.rsrp = (1 - m_varMeasConfig.aRsrp) * storedMeasIt->second.rsrp
            + m_varMeasConfig.aRsrp * rsrp;

          if (std::isnan (storedMeasIt->second.rsrq))
            {
              // the previous RSRQ measurements provided UE PHY are invalid
              storedMeasIt->second.rsrq = rsrq; // replace it with unfiltered value
            }
          else
            {
              storedMeasIt->second.rsrq = (1 - m_varMeasConfig.aRsrq) * storedMeasIt->second.rsrq
                + m_varMeasConfig.aRsrq * rsrq;
            }
        }
      else
        {
          storedMeasIt->second.rsrp = rsrp;
          storedMeasIt->second.rsrq = rsrq;
        }
    }
  else
    {
      // first value is always unfiltered
      MeasValues v;
      v.rsrp = rsrp;
      v.rsrq = rsrq;
      std::pair<uint16_t, MeasValues> val (cellId, v);
      std::pair<std::map<uint16_t, MeasValues>::iterator, bool>
        ret = m_storedMeasValues.insert (val);
      NS_ASSERT_MSG (ret.second == true, "element already existed");
      storedMeasIt = ret.first;
    }

  NS_LOG_DEBUG (this << " IMSI " << m_imsi << " state " << ToString (m_state)
                     << ", measured cell " << m_cellId
                     << ", new RSRP " << rsrp << " stored " << storedMeasIt->second.rsrp
                     << ", new RSRQ " << rsrq << " stored " << storedMeasIt->second.rsrq);
  storedMeasIt->second.timestamp = Simulator::Now ();

} // end of void SaveUeMeasurements

void
cv2x_LteUeRrc::MeasurementReportTriggering (uint8_t measId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);

  std::map<uint8_t, cv2x_LteRrcSap::MeasIdToAddMod>::iterator measIdIt =
    m_varMeasConfig.measIdList.find (measId);
  NS_ASSERT (measIdIt != m_varMeasConfig.measIdList.end ());
  NS_ASSERT (measIdIt->first == measIdIt->second.measId);

  std::map<uint8_t, cv2x_LteRrcSap::ReportConfigToAddMod>::iterator
    reportConfigIt = m_varMeasConfig.reportConfigList.find (measIdIt->second.reportConfigId);
  NS_ASSERT (reportConfigIt != m_varMeasConfig.reportConfigList.end ());
  cv2x_LteRrcSap::ReportConfigEutra& reportConfigEutra = reportConfigIt->second.reportConfigEutra;

  std::map<uint8_t, cv2x_LteRrcSap::MeasObjectToAddMod>::iterator
    measObjectIt = m_varMeasConfig.measObjectList.find (measIdIt->second.measObjectId);
  NS_ASSERT (measObjectIt != m_varMeasConfig.measObjectList.end ());
  cv2x_LteRrcSap::MeasObjectEutra& measObjectEutra = measObjectIt->second.measObjectEutra;

  std::map<uint8_t, VarMeasReport>::iterator
    measReportIt = m_varMeasReportList.find (measId);
  bool isMeasIdInReportList = (measReportIt != m_varMeasReportList.end ());

  // we don't check the purpose field, as it is only included for
  // triggerType == periodical, which is not supported
  NS_ASSERT_MSG (reportConfigEutra.triggerType
                 == cv2x_LteRrcSap::ReportConfigEutra::EVENT,
                 "only triggerType == event is supported");
  // only EUTRA is supported, no need to check for it

  NS_LOG_LOGIC (this << " considering measId " << (uint32_t) measId);
  bool eventEntryCondApplicable = false;
  bool eventLeavingCondApplicable = false;
  ConcernedCells_t concernedCellsEntry;
  ConcernedCells_t concernedCellsLeaving;

  switch (reportConfigEutra.eventId)
    {
    case cv2x_LteRrcSap::ReportConfigEutra::EVENT_A1:
      {
        /*
         * Event A1 (Serving becomes better than threshold)
         * Please refer to 3GPP TS 36.331 Section 5.5.4.2
         */

        double ms; // Ms, the measurement result of the serving cell
        double thresh; // Thresh, the threshold parameter for this event
        // Hys, the hysteresis parameter for this event.
        double hys = cv2x_EutranMeasurementMapping::IeValue2ActualHysteresis (reportConfigEutra.hysteresis);

        switch (reportConfigEutra.triggerQuantity)
          {
          case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
            ms = m_storedMeasValues[m_cellId].rsrp;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            thresh = cv2x_EutranMeasurementMapping::RsrpRange2Dbm (reportConfigEutra.threshold1.range);
            break;
          case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
            ms = m_storedMeasValues[m_cellId].rsrq;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            thresh = cv2x_EutranMeasurementMapping::RsrqRange2Db (reportConfigEutra.threshold1.range);
            break;
          default:
            NS_FATAL_ERROR ("unsupported triggerQuantity");
            break;
          }

        // Inequality A1-1 (Entering condition): Ms - Hys > Thresh
        bool entryCond = ms - hys > thresh;

        if (entryCond)
          {
            if (!isMeasIdInReportList)
              {
                concernedCellsEntry.push_back (m_cellId);
                eventEntryCondApplicable = true;
              }
            else
              {
                /*
                 * This is to check that the triggered cell recorded in the
                 * VarMeasReportList is the serving cell.
                 */
                NS_ASSERT (measReportIt->second.cellsTriggeredList.find (m_cellId)
                           != measReportIt->second.cellsTriggeredList.end ());
              }
          }
        else if (reportConfigEutra.timeToTrigger > 0)
          {
            CancelEnteringTrigger (measId);
          }

        // Inequality A1-2 (Leaving condition): Ms + Hys < Thresh
        bool leavingCond = ms + hys < thresh;

        if (leavingCond)
          {
            if (isMeasIdInReportList)
              {
                /*
                 * This is to check that the triggered cell recorded in the
                 * VarMeasReportList is the serving cell.
                 */
                NS_ASSERT (measReportIt->second.cellsTriggeredList.find (m_cellId)
                           != measReportIt->second.cellsTriggeredList.end ());
                concernedCellsLeaving.push_back (m_cellId);
                eventLeavingCondApplicable = true;
              }
          }
        else if (reportConfigEutra.timeToTrigger > 0)
          {
            CancelLeavingTrigger (measId);
          }

        NS_LOG_LOGIC (this << " event A1: serving cell " << m_cellId
                           << " ms=" << ms << " thresh=" << thresh
                           << " entryCond=" << entryCond
                           << " leavingCond=" << leavingCond);

      } // end of case cv2x_LteRrcSap::ReportConfigEutra::EVENT_A1

      break;

    case cv2x_LteRrcSap::ReportConfigEutra::EVENT_A2:
      {
        /*
         * Event A2 (Serving becomes worse than threshold)
         * Please refer to 3GPP TS 36.331 Section 5.5.4.3
         */

        double ms; // Ms, the measurement result of the serving cell
        double thresh; // Thresh, the threshold parameter for this event
        // Hys, the hysteresis parameter for this event.
        double hys = cv2x_EutranMeasurementMapping::IeValue2ActualHysteresis (reportConfigEutra.hysteresis);

        switch (reportConfigEutra.triggerQuantity)
          {
          case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
            ms = m_storedMeasValues[m_cellId].rsrp;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            thresh = cv2x_EutranMeasurementMapping::RsrpRange2Dbm (reportConfigEutra.threshold1.range);
            break;
          case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
            ms = m_storedMeasValues[m_cellId].rsrq;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            thresh = cv2x_EutranMeasurementMapping::RsrqRange2Db (reportConfigEutra.threshold1.range);
            break;
          default:
            NS_FATAL_ERROR ("unsupported triggerQuantity");
            break;
          }

        // Inequality A2-1 (Entering condition): Ms + Hys < Thresh
        bool entryCond = ms + hys < thresh;

        if (entryCond)
          {
            if (!isMeasIdInReportList)
              {
                concernedCellsEntry.push_back (m_cellId);
                eventEntryCondApplicable = true;
              }
            else
              {
                /*
                 * This is to check that the triggered cell recorded in the
                 * VarMeasReportList is the serving cell.
                 */
                NS_ASSERT (measReportIt->second.cellsTriggeredList.find (m_cellId)
                           != measReportIt->second.cellsTriggeredList.end ());
              }
          }
        else if (reportConfigEutra.timeToTrigger > 0)
          {
            CancelEnteringTrigger (measId);
          }

        // Inequality A2-2 (Leaving condition): Ms - Hys > Thresh
        bool leavingCond = ms - hys > thresh;

        if (leavingCond)
          {
            if (isMeasIdInReportList)
              {
                /*
                 * This is to check that the triggered cell recorded in the
                 * VarMeasReportList is the serving cell.
                 */
                NS_ASSERT (measReportIt->second.cellsTriggeredList.find (m_cellId)
                           != measReportIt->second.cellsTriggeredList.end ());
                concernedCellsLeaving.push_back (m_cellId);
                eventLeavingCondApplicable = true;
              }
          }
        else if (reportConfigEutra.timeToTrigger > 0)
          {
            CancelLeavingTrigger (measId);
          }

        NS_LOG_LOGIC (this << " event A2: serving cell " << m_cellId
                           << " ms=" << ms << " thresh=" << thresh
                           << " entryCond=" << entryCond
                           << " leavingCond=" << leavingCond);

      } // end of case cv2x_LteRrcSap::ReportConfigEutra::EVENT_A2

      break;

    case cv2x_LteRrcSap::ReportConfigEutra::EVENT_A3:
      {
        /*
         * Event A3 (Neighbour becomes offset better than PCell)
         * Please refer to 3GPP TS 36.331 Section 5.5.4.4
         */

        double mn; // Mn, the measurement result of the neighbouring cell
        double ofn = measObjectEutra.offsetFreq; // Ofn, the frequency specific offset of the frequency of the
        double ocn = 0.0; // Ocn, the cell specific offset of the neighbour cell
        double mp; // Mp, the measurement result of the PCell
        double ofp = measObjectEutra.offsetFreq; // Ofp, the frequency specific offset of the primary frequency
        double ocp = 0.0; // Ocp, the cell specific offset of the PCell
        // Off, the offset parameter for this event.
        double off = cv2x_EutranMeasurementMapping::IeValue2ActualA3Offset (reportConfigEutra.a3Offset);
        // Hys, the hysteresis parameter for this event.
        double hys = cv2x_EutranMeasurementMapping::IeValue2ActualHysteresis (reportConfigEutra.hysteresis);

        switch (reportConfigEutra.triggerQuantity)
          {
          case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
            mp = m_storedMeasValues[m_cellId].rsrp;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            break;
          case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
            mp = m_storedMeasValues[m_cellId].rsrq;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            break;
          default:
            NS_FATAL_ERROR ("unsupported triggerQuantity");
            break;
          }

        for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
             storedMeasIt != m_storedMeasValues.end ();
             ++storedMeasIt)
          {
            uint16_t cellId = storedMeasIt->first;
            if (cellId == m_cellId)
              {
                continue;
              }

            switch (reportConfigEutra.triggerQuantity)
              {
              case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
                mn = storedMeasIt->second.rsrp;
                break;
              case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
                mn = storedMeasIt->second.rsrq;
                break;
              default:
                NS_FATAL_ERROR ("unsupported triggerQuantity");
                break;
              }

            bool hasTriggered = isMeasIdInReportList
              && (measReportIt->second.cellsTriggeredList.find (cellId)
                  != measReportIt->second.cellsTriggeredList.end ());

            // Inequality A3-1 (Entering condition): Mn + Ofn + Ocn - Hys > Mp + Ofp + Ocp + Off
            bool entryCond = mn + ofn + ocn - hys > mp + ofp + ocp + off;

            if (entryCond)
              {
                if (!hasTriggered)
                  {
                    concernedCellsEntry.push_back (cellId);
                    eventEntryCondApplicable = true;
                  }
              }
            else if (reportConfigEutra.timeToTrigger > 0)
              {
                CancelEnteringTrigger (measId, cellId);
              }

            // Inequality A3-2 (Leaving condition): Mn + Ofn + Ocn + Hys < Mp + Ofp + Ocp + Off
            bool leavingCond = mn + ofn + ocn + hys < mp + ofp + ocp + off;

            if (leavingCond)
              {
                if (hasTriggered)
                  {
                    concernedCellsLeaving.push_back (cellId);
                    eventLeavingCondApplicable = true;
                  }
              }
            else if (reportConfigEutra.timeToTrigger > 0)
              {
                CancelLeavingTrigger (measId, cellId);
              }

            NS_LOG_LOGIC (this << " event A3: neighbor cell " << cellId
                               << " mn=" << mn << " mp=" << mp << " offset=" << off
                               << " entryCond=" << entryCond
                               << " leavingCond=" << leavingCond);

          } // end of for (storedMeasIt)

      } // end of case cv2x_LteRrcSap::ReportConfigEutra::EVENT_A3

      break;

    case cv2x_LteRrcSap::ReportConfigEutra::EVENT_A4:
      {
        /*
         * Event A4 (Neighbour becomes better than threshold)
         * Please refer to 3GPP TS 36.331 Section 5.5.4.5
         */

        double mn; // Mn, the measurement result of the neighbouring cell
        double ofn = measObjectEutra.offsetFreq; // Ofn, the frequency specific offset of the frequency of the
        double ocn = 0.0; // Ocn, the cell specific offset of the neighbour cell
        double thresh; // Thresh, the threshold parameter for this event
        // Hys, the hysteresis parameter for this event.
        double hys = cv2x_EutranMeasurementMapping::IeValue2ActualHysteresis (reportConfigEutra.hysteresis);

        switch (reportConfigEutra.triggerQuantity)
          {
          case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            thresh = cv2x_EutranMeasurementMapping::RsrpRange2Dbm (reportConfigEutra.threshold1.range);
            break;
          case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            thresh = cv2x_EutranMeasurementMapping::RsrqRange2Db (reportConfigEutra.threshold1.range);
            break;
          default:
            NS_FATAL_ERROR ("unsupported triggerQuantity");
            break;
          }

        for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
             storedMeasIt != m_storedMeasValues.end ();
             ++storedMeasIt)
          {
            uint16_t cellId = storedMeasIt->first;
            if (cellId == m_cellId)
              {
                continue;
              }

            switch (reportConfigEutra.triggerQuantity)
              {
              case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
                mn = storedMeasIt->second.rsrp;
                break;
              case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
                mn = storedMeasIt->second.rsrq;
                break;
              default:
                NS_FATAL_ERROR ("unsupported triggerQuantity");
                break;
              }

            bool hasTriggered = isMeasIdInReportList
              && (measReportIt->second.cellsTriggeredList.find (cellId)
                  != measReportIt->second.cellsTriggeredList.end ());

            // Inequality A4-1 (Entering condition): Mn + Ofn + Ocn - Hys > Thresh
            bool entryCond = mn + ofn + ocn - hys > thresh;

            if (entryCond)
              {
                if (!hasTriggered)
                  {
                    concernedCellsEntry.push_back (cellId);
                    eventEntryCondApplicable = true;
                  }
              }
            else if (reportConfigEutra.timeToTrigger > 0)
              {
                CancelEnteringTrigger (measId, cellId);
              }

            // Inequality A4-2 (Leaving condition): Mn + Ofn + Ocn + Hys < Thresh
            bool leavingCond = mn + ofn + ocn + hys < thresh;

            if (leavingCond)
              {
                if (hasTriggered)
                  {
                    concernedCellsLeaving.push_back (cellId);
                    eventLeavingCondApplicable = true;
                  }
              }
            else if (reportConfigEutra.timeToTrigger > 0)
              {
                CancelLeavingTrigger (measId, cellId);
              }

            NS_LOG_LOGIC (this << " event A4: neighbor cell " << cellId
                               << " mn=" << mn << " thresh=" << thresh
                               << " entryCond=" << entryCond
                               << " leavingCond=" << leavingCond);

          } // end of for (storedMeasIt)

      } // end of case cv2x_LteRrcSap::ReportConfigEutra::EVENT_A4

      break;

    case cv2x_LteRrcSap::ReportConfigEutra::EVENT_A5:
      {
        /*
         * Event A5 (PCell becomes worse than threshold1 and neighbour
         * becomes better than threshold2)
         * Please refer to 3GPP TS 36.331 Section 5.5.4.6
         */

        double mp; // Mp, the measurement result of the PCell
        double mn; // Mn, the measurement result of the neighbouring cell
        double ofn = measObjectEutra.offsetFreq; // Ofn, the frequency specific offset of the frequency of the
        double ocn = 0.0; // Ocn, the cell specific offset of the neighbour cell
        double thresh1; // Thresh1, the threshold parameter for this event
        double thresh2; // Thresh2, the threshold parameter for this event
        // Hys, the hysteresis parameter for this event.
        double hys = cv2x_EutranMeasurementMapping::IeValue2ActualHysteresis (reportConfigEutra.hysteresis);

        switch (reportConfigEutra.triggerQuantity)
          {
          case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
            mp = m_storedMeasValues[m_cellId].rsrp;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            NS_ASSERT (reportConfigEutra.threshold2.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRP);
            thresh1 = cv2x_EutranMeasurementMapping::RsrpRange2Dbm (reportConfigEutra.threshold1.range);
            thresh2 = cv2x_EutranMeasurementMapping::RsrpRange2Dbm (reportConfigEutra.threshold2.range);
            break;
          case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
            mp = m_storedMeasValues[m_cellId].rsrq;
            NS_ASSERT (reportConfigEutra.threshold1.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            NS_ASSERT (reportConfigEutra.threshold2.choice
                       == cv2x_LteRrcSap::ThresholdEutra::THRESHOLD_RSRQ);
            thresh1 = cv2x_EutranMeasurementMapping::RsrqRange2Db (reportConfigEutra.threshold1.range);
            thresh2 = cv2x_EutranMeasurementMapping::RsrqRange2Db (reportConfigEutra.threshold2.range);
            break;
          default:
            NS_FATAL_ERROR ("unsupported triggerQuantity");
            break;
          }

        // Inequality A5-1 (Entering condition 1): Mp + Hys < Thresh1
        bool entryCond = mp + hys < thresh1;

        if (entryCond)
          {
            for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
                 storedMeasIt != m_storedMeasValues.end ();
                 ++storedMeasIt)
              {
                uint16_t cellId = storedMeasIt->first;
                if (cellId == m_cellId)
                  {
                    continue;
                  }

                switch (reportConfigEutra.triggerQuantity)
                  {
                  case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
                    mn = storedMeasIt->second.rsrp;
                    break;
                  case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
                    mn = storedMeasIt->second.rsrq;
                    break;
                  default:
                    NS_FATAL_ERROR ("unsupported triggerQuantity");
                    break;
                  }

                bool hasTriggered = isMeasIdInReportList
                  && (measReportIt->second.cellsTriggeredList.find (cellId)
                      != measReportIt->second.cellsTriggeredList.end ());

                // Inequality A5-2 (Entering condition 2): Mn + Ofn + Ocn - Hys > Thresh2

                entryCond = mn + ofn + ocn - hys > thresh2;

                if (entryCond)
                  {
                    if (!hasTriggered)
                      {
                        concernedCellsEntry.push_back (cellId);
                        eventEntryCondApplicable = true;
                      }
                  }
                else if (reportConfigEutra.timeToTrigger > 0)
                  {
                    CancelEnteringTrigger (measId, cellId);
                  }

                NS_LOG_LOGIC (this << " event A5: neighbor cell " << cellId
                                   << " mn=" << mn << " mp=" << mp
                                   << " thresh2=" << thresh2
                                   << " thresh1=" << thresh1
                                   << " entryCond=" << entryCond);

              } // end of for (storedMeasIt)

          } // end of if (entryCond)
        else
          {
            NS_LOG_LOGIC (this << " event A5: serving cell " << m_cellId
                               << " mp=" << mp << " thresh1=" << thresh1
                               << " entryCond=" << entryCond);

            if (reportConfigEutra.timeToTrigger > 0)
              {
                CancelEnteringTrigger (measId);
              }
          }

        if (isMeasIdInReportList)
          {
            // Inequality A5-3 (Leaving condition 1): Mp - Hys > Thresh1
            bool leavingCond = mp - hys > thresh1;

            if (leavingCond)
              {
                if (reportConfigEutra.timeToTrigger == 0)
                  {
                    // leaving condition #2 does not have to be checked

                    for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
                         storedMeasIt != m_storedMeasValues.end ();
                         ++storedMeasIt)
                      {
                        uint16_t cellId = storedMeasIt->first;
                        if (cellId == m_cellId)
                          {
                            continue;
                          }

                        if (measReportIt->second.cellsTriggeredList.find (cellId)
                            != measReportIt->second.cellsTriggeredList.end ())
                          {
                            concernedCellsLeaving.push_back (cellId);
                            eventLeavingCondApplicable = true;
                          }
                      }
                  } // end of if (reportConfigEutra.timeToTrigger == 0)
                else
                  {
                    // leaving condition #2 has to be checked to cancel time-to-trigger

                    for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
                         storedMeasIt != m_storedMeasValues.end ();
                         ++storedMeasIt)
                      {
                        uint16_t cellId = storedMeasIt->first;
                        if (cellId == m_cellId)
                          {
                            continue;
                          }

                        if (measReportIt->second.cellsTriggeredList.find (cellId)
                            != measReportIt->second.cellsTriggeredList.end ())
                          {
                            switch (reportConfigEutra.triggerQuantity)
                              {
                              case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
                                mn = storedMeasIt->second.rsrp;
                                break;
                              case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
                                mn = storedMeasIt->second.rsrq;
                                break;
                              default:
                                NS_FATAL_ERROR ("unsupported triggerQuantity");
                                break;
                              }

                            // Inequality A5-4 (Leaving condition 2): Mn + Ofn + Ocn + Hys < Thresh2

                            leavingCond = mn + ofn + ocn + hys < thresh2;

                            if (!leavingCond)
                              {
                                CancelLeavingTrigger (measId, cellId);
                              }

                            /*
                             * Whatever the result of leaving condition #2, this
                             * cell is still "in", because leaving condition #1
                             * is already true.
                             */
                            concernedCellsLeaving.push_back (cellId);
                            eventLeavingCondApplicable = true;

                            NS_LOG_LOGIC (this << " event A5: neighbor cell " << cellId
                                               << " mn=" << mn << " mp=" << mp
                                               << " thresh2=" << thresh2
                                               << " thresh1=" << thresh1
                                               << " leavingCond=" << leavingCond);

                          } // end of if (measReportIt->second.cellsTriggeredList.find (cellId)
                            //            != measReportIt->second.cellsTriggeredList.end ())

                      } // end of for (storedMeasIt)

                  } // end of else of if (reportConfigEutra.timeToTrigger == 0)

                NS_LOG_LOGIC (this << " event A5: serving cell " << m_cellId
                                   << " mp=" << mp << " thresh1=" << thresh1
                                   << " leavingCond=" << leavingCond);

              } // end of if (leavingCond)
            else
              {
                if (reportConfigEutra.timeToTrigger > 0)
                  {
                    CancelLeavingTrigger (measId);
                  }

                // check leaving condition #2

                for (std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedMeasValues.begin ();
                     storedMeasIt != m_storedMeasValues.end ();
                     ++storedMeasIt)
                  {
                    uint16_t cellId = storedMeasIt->first;
                    if (cellId == m_cellId)
                      {
                        continue;
                      }

                    if (measReportIt->second.cellsTriggeredList.find (cellId)
                        != measReportIt->second.cellsTriggeredList.end ())
                      {
                        switch (reportConfigEutra.triggerQuantity)
                          {
                          case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
                            mn = storedMeasIt->second.rsrp;
                            break;
                          case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
                            mn = storedMeasIt->second.rsrq;
                            break;
                          default:
                            NS_FATAL_ERROR ("unsupported triggerQuantity");
                            break;
                          }

                        // Inequality A5-4 (Leaving condition 2): Mn + Ofn + Ocn + Hys < Thresh2
                        leavingCond = mn + ofn + ocn + hys < thresh2;

                        if (leavingCond)
                          {
                            concernedCellsLeaving.push_back (cellId);
                            eventLeavingCondApplicable = true;
                          }

                        NS_LOG_LOGIC (this << " event A5: neighbor cell " << cellId
                                           << " mn=" << mn << " mp=" << mp
                                           << " thresh2=" << thresh2
                                           << " thresh1=" << thresh1
                                           << " leavingCond=" << leavingCond);

                      } // end of if (measReportIt->second.cellsTriggeredList.find (cellId)
                        //            != measReportIt->second.cellsTriggeredList.end ())

                  } // end of for (storedMeasIt)

              } // end of else of if (leavingCond)

          } // end of if (isMeasIdInReportList)

      } // end of case cv2x_LteRrcSap::ReportConfigEutra::EVENT_A5

      break;

    default:
      NS_FATAL_ERROR ("unsupported eventId " << reportConfigEutra.eventId);
      break;

    } // switch (event type)

  NS_LOG_LOGIC (this << " eventEntryCondApplicable=" << eventEntryCondApplicable
                     << " eventLeavingCondApplicable=" << eventLeavingCondApplicable);

  if (eventEntryCondApplicable)
    {
      if (reportConfigEutra.timeToTrigger == 0)
        {
          VarMeasReportListAdd (measId, concernedCellsEntry);
        }
      else
        {
          PendingTrigger_t t;
          t.measId = measId;
          t.concernedCells = concernedCellsEntry;
          t.timer = Simulator::Schedule (MilliSeconds (reportConfigEutra.timeToTrigger),
                                         &cv2x_LteUeRrc::VarMeasReportListAdd, this,
                                         measId, concernedCellsEntry);
          std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
            enteringTriggerIt = m_enteringTriggerQueue.find (measId);
          NS_ASSERT (enteringTriggerIt != m_enteringTriggerQueue.end ());
          enteringTriggerIt->second.push_back (t);
        }
    }

  if (eventLeavingCondApplicable)
    {
      // reportOnLeave will only be set when eventId = eventA3
      bool reportOnLeave = (reportConfigEutra.eventId == cv2x_LteRrcSap::ReportConfigEutra::EVENT_A3)
        && reportConfigEutra.reportOnLeave;

      if (reportConfigEutra.timeToTrigger == 0)
        {
          VarMeasReportListErase (measId, concernedCellsLeaving, reportOnLeave);
        }
      else
        {
          PendingTrigger_t t;
          t.measId = measId;
          t.concernedCells = concernedCellsLeaving;
          t.timer = Simulator::Schedule (MilliSeconds (reportConfigEutra.timeToTrigger),
                                         &cv2x_LteUeRrc::VarMeasReportListErase, this,
                                         measId, concernedCellsLeaving, reportOnLeave);
          std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
            leavingTriggerIt = m_leavingTriggerQueue.find (measId);
          NS_ASSERT (leavingTriggerIt != m_leavingTriggerQueue.end ());
          leavingTriggerIt->second.push_back (t);
        }
    }

} // end of void cv2x_LteUeRrc::MeasurementReportTriggering (uint8_t measId)

void
cv2x_LteUeRrc::CancelEnteringTrigger (uint8_t measId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    it1 = m_enteringTriggerQueue.find (measId);
  NS_ASSERT (it1 != m_enteringTriggerQueue.end ());

  if (!it1->second.empty ())
    {
      std::list<PendingTrigger_t>::iterator it2;
      for (it2 = it1->second.begin (); it2 != it1->second.end (); ++it2)
        {
          NS_ASSERT (it2->measId == measId);
          NS_LOG_LOGIC (this << " canceling entering time-to-trigger event at "
                             << Simulator::GetDelayLeft (it2->timer).GetSeconds ());
          Simulator::Cancel (it2->timer);
        }

      it1->second.clear ();
    }
}

void
cv2x_LteUeRrc::CancelEnteringTrigger (uint8_t measId, uint16_t cellId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId << cellId);

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    it1 = m_enteringTriggerQueue.find (measId);
  NS_ASSERT (it1 != m_enteringTriggerQueue.end ());

  std::list<PendingTrigger_t>::iterator it2 = it1->second.begin ();
  while (it2 != it1->second.end ())
    {
      NS_ASSERT (it2->measId == measId);

      ConcernedCells_t::iterator it3;
      for (it3 = it2->concernedCells.begin ();
           it3 != it2->concernedCells.end (); ++it3)
        {
          if (*it3 == cellId)
            {
              it3 = it2->concernedCells.erase (it3);
            }
        }

      if (it2->concernedCells.empty ())
        {
          NS_LOG_LOGIC (this << " canceling entering time-to-trigger event at "
                             << Simulator::GetDelayLeft (it2->timer).GetSeconds ());
          Simulator::Cancel (it2->timer);
          it2 = it1->second.erase (it2);
        }
      else
        {
          it2++;
        }
    }
}

void
cv2x_LteUeRrc::CancelLeavingTrigger (uint8_t measId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    it1 = m_leavingTriggerQueue.find (measId);
  NS_ASSERT (it1 != m_leavingTriggerQueue.end ());

  if (!it1->second.empty ())
    {
      std::list<PendingTrigger_t>::iterator it2;
      for (it2 = it1->second.begin (); it2 != it1->second.end (); ++it2)
        {
          NS_ASSERT (it2->measId == measId);
          NS_LOG_LOGIC (this << " canceling leaving time-to-trigger event at "
                             << Simulator::GetDelayLeft (it2->timer).GetSeconds ());
          Simulator::Cancel (it2->timer);
        }

      it1->second.clear ();
    }
}

void
cv2x_LteUeRrc::CancelLeavingTrigger (uint8_t measId, uint16_t cellId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId << cellId);

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    it1 = m_leavingTriggerQueue.find (measId);
  NS_ASSERT (it1 != m_leavingTriggerQueue.end ());

  std::list<PendingTrigger_t>::iterator it2 = it1->second.begin ();
  while (it2 != it1->second.end ())
    {
      NS_ASSERT (it2->measId == measId);

      ConcernedCells_t::iterator it3;
      for (it3 = it2->concernedCells.begin ();
           it3 != it2->concernedCells.end (); ++it3)
        {
          if (*it3 == cellId)
            {
              it3 = it2->concernedCells.erase (it3);
            }
        }

      if (it2->concernedCells.empty ())
        {
          NS_LOG_LOGIC (this << " canceling leaving time-to-trigger event at "
                             << Simulator::GetDelayLeft (it2->timer).GetSeconds ());
          Simulator::Cancel (it2->timer);
          it2 = it1->second.erase (it2);
        }
      else
        {
          it2++;
        }
    }
}

void
cv2x_LteUeRrc::VarMeasReportListAdd (uint8_t measId, ConcernedCells_t enteringCells)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);
  NS_ASSERT (!enteringCells.empty ());

  std::map<uint8_t, VarMeasReport>::iterator
    measReportIt = m_varMeasReportList.find (measId);

  if (measReportIt == m_varMeasReportList.end ())
    {
      VarMeasReport r;
      r.measId = measId;
      std::pair<uint8_t, VarMeasReport> val (measId, r);
      std::pair<std::map<uint8_t, VarMeasReport>::iterator, bool>
        ret = m_varMeasReportList.insert (val);
      NS_ASSERT_MSG (ret.second == true, "element already existed");
      measReportIt = ret.first;
    }

  NS_ASSERT (measReportIt != m_varMeasReportList.end ());

  for (ConcernedCells_t::const_iterator it = enteringCells.begin ();
       it != enteringCells.end ();
       ++it)
    {
      measReportIt->second.cellsTriggeredList.insert (*it);
    }

  NS_ASSERT (!measReportIt->second.cellsTriggeredList.empty ());
  measReportIt->second.numberOfReportsSent = 0;
  measReportIt->second.periodicReportTimer
    = Simulator::Schedule (UE_MEASUREMENT_REPORT_DELAY,
                           &cv2x_LteUeRrc::SendMeasurementReport,
                           this, measId);

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    enteringTriggerIt = m_enteringTriggerQueue.find (measId);
  NS_ASSERT (enteringTriggerIt != m_enteringTriggerQueue.end ());
  if (!enteringTriggerIt->second.empty ())
    {
      /*
       * Assumptions at this point:
       *  - the call to this function was delayed by time-to-trigger;
       *  - the time-to-trigger delay is fixed (not adaptive/dynamic); and
       *  - the first element in the list is associated with this function call.
       */
      enteringTriggerIt->second.pop_front ();

      if (!enteringTriggerIt->second.empty ())
        {
          /*
           * To prevent the same set of cells triggering again in the future,
           * we clean up the time-to-trigger queue. This case might occur when
           * time-to-trigger > 200 ms.
           */
          for (ConcernedCells_t::const_iterator it = enteringCells.begin ();
               it != enteringCells.end (); ++it)
            {
              CancelEnteringTrigger (measId, *it);
            }
        }

    } // end of if (!enteringTriggerIt->second.empty ())

} // end of cv2x_LteUeRrc::VarMeasReportListAdd

void
cv2x_LteUeRrc::VarMeasReportListErase (uint8_t measId, ConcernedCells_t leavingCells,
                                  bool reportOnLeave)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);
  NS_ASSERT (!leavingCells.empty ());

  std::map<uint8_t, VarMeasReport>::iterator
    measReportIt = m_varMeasReportList.find (measId);
  NS_ASSERT (measReportIt != m_varMeasReportList.end ());

  for (ConcernedCells_t::const_iterator it = leavingCells.begin ();
       it != leavingCells.end ();
       ++it)
    {
      measReportIt->second.cellsTriggeredList.erase (*it);
    }

  if (reportOnLeave)
    {
      // runs immediately without UE_MEASUREMENT_REPORT_DELAY
      SendMeasurementReport (measId);
    }

  if (measReportIt->second.cellsTriggeredList.empty ())
    {
      measReportIt->second.periodicReportTimer.Cancel ();
      m_varMeasReportList.erase (measReportIt);
    }

  std::map<uint8_t, std::list<PendingTrigger_t> >::iterator
    leavingTriggerIt = m_leavingTriggerQueue.find (measId);
  NS_ASSERT (leavingTriggerIt != m_leavingTriggerQueue.end ());
  if (!leavingTriggerIt->second.empty ())
    {
      /*
       * Assumptions at this point:
       *  - the call to this function was delayed by time-to-trigger; and
       *  - the time-to-trigger delay is fixed (not adaptive/dynamic); and
       *  - the first element in the list is associated with this function call.
       */
      leavingTriggerIt->second.pop_front ();

      if (!leavingTriggerIt->second.empty ())
        {
          /*
           * To prevent the same set of cells triggering again in the future,
           * we clean up the time-to-trigger queue. This case might occur when
           * time-to-trigger > 200 ms.
           */
          for (ConcernedCells_t::const_iterator it = leavingCells.begin ();
               it != leavingCells.end (); ++it)
            {
              CancelLeavingTrigger (measId, *it);
            }
        }

    } // end of if (!leavingTriggerIt->second.empty ())

} // end of cv2x_LteUeRrc::VarMeasReportListErase

void
cv2x_LteUeRrc::VarMeasReportListClear (uint8_t measId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);

  // remove the measurement reporting entry for this measId from the VarMeasReportList
  std::map<uint8_t, VarMeasReport>::iterator
    measReportIt = m_varMeasReportList.find (measId);
  if (measReportIt != m_varMeasReportList.end ())
    {
      NS_LOG_LOGIC (this << " deleting existing report for measId " << (uint16_t) measId);
      measReportIt->second.periodicReportTimer.Cancel ();
      m_varMeasReportList.erase (measReportIt);
    }

  CancelEnteringTrigger (measId);
  CancelLeavingTrigger (measId);
}

void 
cv2x_LteUeRrc::SendMeasurementReport (uint8_t measId)
{
  NS_LOG_FUNCTION (this << (uint16_t) measId);
  //  3GPP TS 36.331 section 5.5.5 Measurement reporting

  std::map<uint8_t, cv2x_LteRrcSap::MeasIdToAddMod>::iterator 
    measIdIt = m_varMeasConfig.measIdList.find (measId);
  NS_ASSERT (measIdIt != m_varMeasConfig.measIdList.end ());

  std::map<uint8_t, cv2x_LteRrcSap::ReportConfigToAddMod>::iterator 
    reportConfigIt = m_varMeasConfig.reportConfigList.find (measIdIt->second.reportConfigId);
  NS_ASSERT (reportConfigIt != m_varMeasConfig.reportConfigList.end ());
  cv2x_LteRrcSap::ReportConfigEutra& reportConfigEutra = reportConfigIt->second.reportConfigEutra;

  cv2x_LteRrcSap::MeasurementReport measurementReport;
  cv2x_LteRrcSap::MeasResults& measResults = measurementReport.measResults;
  measResults.measId = measId;

  std::map<uint16_t, MeasValues>::iterator servingMeasIt = m_storedMeasValues.find (m_cellId);
  NS_ASSERT (servingMeasIt != m_storedMeasValues.end ());
  measResults.rsrpResult = cv2x_EutranMeasurementMapping::Dbm2RsrpRange (servingMeasIt->second.rsrp);
  measResults.rsrqResult = cv2x_EutranMeasurementMapping::Db2RsrqRange (servingMeasIt->second.rsrq);
  NS_LOG_INFO (this << " reporting serving cell "
               "RSRP " << (uint32_t) measResults.rsrpResult << " (" << servingMeasIt->second.rsrp << " dBm) "
               "RSRQ " << (uint32_t) measResults.rsrqResult << " (" << servingMeasIt->second.rsrq << " dB)");
  measResults.haveMeasResultNeighCells = false;
  std::map<uint8_t, VarMeasReport>::iterator measReportIt = m_varMeasReportList.find (measId);
  if (measReportIt == m_varMeasReportList.end ())
    {
      NS_LOG_ERROR ("no entry found in m_varMeasReportList for measId " << (uint32_t) measId);
    }
  else
    {
      if (!(measReportIt->second.cellsTriggeredList.empty ()))
        {
          std::multimap<double, uint16_t> sortedNeighCells;
          for (std::set<uint16_t>::iterator cellsTriggeredIt = measReportIt->second.cellsTriggeredList.begin ();
               cellsTriggeredIt != measReportIt->second.cellsTriggeredList.end ();
               ++cellsTriggeredIt)
            {
              uint16_t cellId = *cellsTriggeredIt;
              if (cellId != m_cellId)
                {
                  std::map<uint16_t, MeasValues>::iterator neighborMeasIt = m_storedMeasValues.find (cellId);
                  double triggerValue;
                  switch (reportConfigEutra.triggerQuantity)
                    {
                    case cv2x_LteRrcSap::ReportConfigEutra::RSRP:
                      triggerValue = neighborMeasIt->second.rsrp;
                      break;
                    case cv2x_LteRrcSap::ReportConfigEutra::RSRQ:
                      triggerValue = neighborMeasIt->second.rsrq;
                      break;
                    default:
                      NS_FATAL_ERROR ("unsupported triggerQuantity");
                      break;
                    }
                  sortedNeighCells.insert (std::pair<double, uint16_t> (triggerValue, cellId));
                }
            }

          std::multimap<double, uint16_t>::reverse_iterator sortedNeighCellsIt;
          uint32_t count;
          for (sortedNeighCellsIt = sortedNeighCells.rbegin (), count = 0;
               sortedNeighCellsIt != sortedNeighCells.rend () && count < reportConfigEutra.maxReportCells;
               ++sortedNeighCellsIt, ++count)
            {
              uint16_t cellId = sortedNeighCellsIt->second;
              std::map<uint16_t, MeasValues>::iterator neighborMeasIt = m_storedMeasValues.find (cellId);
              NS_ASSERT (neighborMeasIt != m_storedMeasValues.end ());
              cv2x_LteRrcSap::MeasResultEutra measResultEutra;
              measResultEutra.physCellId = cellId;
              measResultEutra.haveCgiInfo = false;
              measResultEutra.haveRsrpResult = true;
              measResultEutra.rsrpResult = cv2x_EutranMeasurementMapping::Dbm2RsrpRange (neighborMeasIt->second.rsrp);
              measResultEutra.haveRsrqResult = true;
              measResultEutra.rsrqResult = cv2x_EutranMeasurementMapping::Db2RsrqRange (neighborMeasIt->second.rsrq);
              NS_LOG_INFO (this << " reporting neighbor cell " << (uint32_t) measResultEutra.physCellId 
                                << " RSRP " << (uint32_t) measResultEutra.rsrpResult
                                << " (" << neighborMeasIt->second.rsrp << " dBm)"
                                << " RSRQ " << (uint32_t) measResultEutra.rsrqResult
                                << " (" << neighborMeasIt->second.rsrq << " dB)");
              measResults.measResultListEutra.push_back (measResultEutra);
              measResults.haveMeasResultNeighCells = true;
            }
        }
      else
        {
          NS_LOG_WARN (this << " cellsTriggeredList is empty");
        }

      measResults.haveScellsMeas = false;
      std::map<uint16_t, MeasValues>::iterator sCellsMeasIt =  m_storedScellMeasValues.begin ();
      if (sCellsMeasIt != m_storedScellMeasValues.end ())
        {
          measResults.haveScellsMeas = true;
          measResults.measScellResultList.haveMeasurementResultsServingSCells = true;
          measResults.measScellResultList.haveMeasurementResultsNeighCell = false;


          for ( sCellsMeasIt = m_storedScellMeasValues.begin (); 
                sCellsMeasIt != m_storedScellMeasValues.end (); ++sCellsMeasIt)
            {
              cv2x_LteRrcSap::MeasResultScell measResultScell;
              measResultScell.servFreqId = sCellsMeasIt->first;
              measResultScell.haveRsrpResult =  true;
              measResultScell.haveRsrqResult =  true;
              measResultScell.rsrpResult = cv2x_EutranMeasurementMapping::Dbm2RsrpRange (sCellsMeasIt->second.rsrp);
              measResultScell.rsrqResult = cv2x_EutranMeasurementMapping::Db2RsrqRange (sCellsMeasIt->second.rsrq); 
              measResults.measScellResultList.measResultScell.push_back (measResultScell);
            }
        }

      /*
       * The current cv2x_LteRrcSap implementation is broken in that it does not
       * allow for infinite values of reportAmount, which is probably the most
       * reasonable setting. So we just always assume infinite reportAmount.
       */
      measReportIt->second.numberOfReportsSent++;
      measReportIt->second.periodicReportTimer.Cancel ();

      Time reportInterval;
      switch (reportConfigEutra.reportInterval)
        {
        case cv2x_LteRrcSap::ReportConfigEutra::MS120:
          reportInterval = MilliSeconds (120);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MS240:
          reportInterval = MilliSeconds (240);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MS480:
          reportInterval = MilliSeconds (480);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MS640:
          reportInterval = MilliSeconds (640);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MS1024:
          reportInterval = MilliSeconds (1024);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MS2048:
          reportInterval = MilliSeconds (2048);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MS5120:
          reportInterval = MilliSeconds (5120);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MS10240:
          reportInterval = MilliSeconds (10240);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MIN1:
          reportInterval = Seconds (60);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MIN6:
          reportInterval = Seconds (360);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MIN12:
          reportInterval = Seconds (720);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MIN30:
          reportInterval = Seconds (1800);
          break;
        case cv2x_LteRrcSap::ReportConfigEutra::MIN60:
          reportInterval = Seconds (3600);
          break;
        default:
          NS_FATAL_ERROR ("Unsupported reportInterval " << (uint16_t) reportConfigEutra.reportInterval);
          break;
        }

      // schedule the next measurement reporting
      measReportIt->second.periodicReportTimer 
        = Simulator::Schedule (reportInterval,
                               &cv2x_LteUeRrc::SendMeasurementReport,
                               this, measId);

      // send the measurement report to eNodeB
      m_rrcSapUser->SendMeasurementReport (measurementReport);
    } 
}

void 
cv2x_LteUeRrc::StartConnection ()
{
  NS_LOG_FUNCTION (this << m_imsi);
  NS_ASSERT (m_hasReceivedMib);
  NS_ASSERT (m_hasReceivedSib2);
  m_connectionPending = false; // reset the flag
  SwitchToState (IDLE_RANDOM_ACCESS);
  m_cmacSapProvider.at (0)->StartContentionBasedRandomAccessProcedure ();
}

void 
cv2x_LteUeRrc::LeaveConnectedMode ()
{
  NS_LOG_FUNCTION (this << m_imsi);
  m_asSapUser->NotifyConnectionReleased ();
  m_cmacSapProvider.at (0)->RemoveLc (1);
  std::map<uint8_t, Ptr<cv2x_LteDataRadioBearerInfo> >::iterator it;
  for (it = m_drbMap.begin (); it != m_drbMap.end (); ++it)
    {
      m_cmacSapProvider.at (0)->RemoveLc (it->second->m_logicalChannelIdentity);
    }
  m_drbMap.clear ();
  m_bid2DrbidMap.clear ();
  m_srb1 = 0;
  SwitchToState (IDLE_CAMPED_NORMALLY);
}

void
cv2x_LteUeRrc::ConnectionTimeout ()
{
  NS_LOG_FUNCTION (this << m_imsi);
  m_cmacSapProvider.at (0)->Reset ();       // reset the MAC
  m_hasReceivedSib2 = false;         // invalidate the previously received SIB2
  SwitchToState (IDLE_CAMPED_NORMALLY);
  m_connectionTimeoutTrace (m_imsi, m_cellId, m_rnti);
  m_asSapUser->NotifyConnectionFailed ();  // inform upper layer
}

void
cv2x_LteUeRrc::DisposeOldSrb1 ()
{
  NS_LOG_FUNCTION (this);
  m_srb1Old = 0;
}

uint8_t 
cv2x_LteUeRrc::Bid2Drbid (uint8_t bid)
{
  std::map<uint8_t, uint8_t>::iterator it = m_bid2DrbidMap.find (bid);
  //NS_ASSERT_MSG (it != m_bid2DrbidMap.end (), "could not find BID " << bid);
  if (it == m_bid2DrbidMap.end ())
    {
      return 0;
    }
  else
    {
  return it->second;
    }
}

void 
cv2x_LteUeRrc::SwitchToState (State newState)
{
  NS_LOG_FUNCTION (this << ToString (newState));
  State oldState = m_state;
  m_state = newState;
  NS_LOG_INFO (this << " IMSI " << m_imsi << " RNTI " << m_rnti << " UeRrc "
                    << ToString (oldState) << " --> " << ToString (newState));
  m_stateTransitionTrace (m_imsi, m_cellId, m_rnti, oldState, newState);

  switch (newState)
    {
    case IDLE_START:
      NS_FATAL_ERROR ("cannot switch to an initial state");
      break;

    case IDLE_CELL_SEARCH:
    case IDLE_WAIT_MIB_SIB1:
    case IDLE_WAIT_MIB:
    case IDLE_WAIT_SIB1:
      break;

    case IDLE_CAMPED_NORMALLY:
      if (m_connectionPending)
        {
          SwitchToState (IDLE_WAIT_SIB2);
        }
      break;

    case IDLE_WAIT_SIB2:
      if (m_hasReceivedSib2)
        {
          NS_ASSERT (m_connectionPending);
          StartConnection ();
        }
      break;

    case IDLE_RANDOM_ACCESS:
    case IDLE_CONNECTING:
    case CONNECTED_NORMALLY:
    case CONNECTED_HANDOVER:
    case CONNECTED_PHY_PROBLEM:
    case CONNECTED_REESTABLISHING:
      break;
 
    default:
      break;
    }
}

void
cv2x_LteUeRrc::DoComponentCarrierEnabling (std::vector<uint8_t> res)
  {
    NS_LOG_INFO (this);
  }

void
cv2x_LteUeRrc::SaveScellUeMeasurements (uint16_t sCellId, double rsrp, double rsrq,
                                   bool useLayer3Filtering, uint16_t componentCarrierId)
{
  NS_LOG_FUNCTION (this << sCellId << componentCarrierId << rsrp << rsrq << useLayer3Filtering);
  if (sCellId == m_cellId)
    {

      std::map<uint16_t, MeasValues>::iterator storedMeasIt = m_storedScellMeasValues.find (componentCarrierId);

      if (storedMeasIt != m_storedScellMeasValues.end ())
        {
          if (useLayer3Filtering)
            {
              // F_n = (1-a) F_{n-1} + a M_n
              storedMeasIt->second.rsrp = (1 - m_varMeasConfig.aRsrp) * storedMeasIt->second.rsrp
                + m_varMeasConfig.aRsrp * rsrp;

              if (std::isnan (storedMeasIt->second.rsrq))
                {
                  // the previous RSRQ measurements provided UE PHY are invalid
                  storedMeasIt->second.rsrq = rsrq;   // replace it with unfiltered value
                }
              else
                {
                  storedMeasIt->second.rsrq = (1 - m_varMeasConfig.aRsrq) * storedMeasIt->second.rsrq
                    + m_varMeasConfig.aRsrq * rsrq;
                }
            }
          else
            {
              storedMeasIt->second.rsrp = rsrp;
              storedMeasIt->second.rsrq = rsrq;
            }
        }
      else
        {
          // first value is always unfiltered
          MeasValues v;
          v.rsrp = rsrp;
          v.rsrq = rsrq;
          std::pair<uint16_t, MeasValues> val (componentCarrierId, v);
          std::pair<std::map<uint16_t, MeasValues>::iterator, bool>
            ret = m_storedScellMeasValues.insert (val);
          NS_ASSERT_MSG (ret.second == true, "element already existed");
          storedMeasIt = ret.first;
        }

      NS_LOG_DEBUG (this << " IMSI " << m_imsi << " state " << ToString (m_state)
                    << ", measured cell " << sCellId
                    << ", carrier component Id " << componentCarrierId
                    << ", new RSRP " << rsrp << " stored " << storedMeasIt->second.rsrp
                    << ", new RSRQ " << rsrq << " stored " << storedMeasIt->second.rsrq);
      storedMeasIt->second.timestamp = Simulator::Now ();
    }
  else 
    {
      NS_LOG_DEBUG (this << " IMSI " << m_imsi << "measurement on SCC from not serving cell ");
    }

}   // end of void SaveUeMeasurements


/**
 * Process dedicated sidelink configuration
 * \param config The sidelink configuration
 */
void
cv2x_LteUeRrc::ApplySidelinkDedicatedConfiguration (cv2x_LteRrcSap::SlCommConfig config)
{
  NS_LOG_FUNCTION (this);

  //TODO: implement procedures described in 5.3.10.X
  // 1>	if the RRCConnectionReconfiguration message includes the sl-CommConfig:
  //   2>	if commTxResources is included and set to setup:
  //     3>	from the next SC period use the resources indicated by commTxResources for sidelink direct communication transmission, as specified in 5.X.4;
  // 2>	else if commTxResources is included and set to release:
  //     3>	from the next SC period, release the resources allocated for sidelink direct communication transmission previously configured by commTxResources;

  //pools are allocated per UE, not per destinations. A UE may have multiple pools
  //and we need to maintain the list of pools
  //pass information to MAC to change scheduling
  if (config.commTxResources == cv2x_LteRrcSap::SlCommConfig::SETUP) {
    NS_LOG_INFO ("Received dedicated resources " << config.setup.setup);

    Ptr<SidelinkTxCommResourcePool> txPool = CreateObject<SidelinkTxCommResourcePool>();
    if (config.setup.setup == cv2x_LteRrcSap::SlCommTxResourcesSetup::SCHEDULED) {
      txPool->SetPool (config.setup.scheduled.commTxConfig);
      txPool->SetScheduledTxParameters (config.setup.scheduled.crnti, config.setup.scheduled.macMainConfig, config.setup.scheduled.commTxConfig, 0, config.setup.scheduled.mcs);
    } else {
      NS_ASSERT (config.setup.ueSelected.havePoolToAdd);
      //configure lower layers to transmit the sidelink control information and the corresponding data using the pool of resources indicated by the first entry in commTxPoolNormalDedicated;
      txPool->SetPool (config.setup.ueSelected.poolToAddModList.pools[0].pool);
    }
    
    std::list <uint32_t>::iterator it;
    std::list <uint32_t> destinations = m_sidelinkConfiguration->GetTxDestinations ();
    //int index = 0;
    //currently we can only use one pool so all groups will use the same one
    for (it = destinations.begin() ; it != destinations.end() ; it++) {
      m_cmacSapProvider.at (0)->AddSlTxPool (*it, txPool);
    }
    //inform PHY about pool
    m_cphySapProvider.at (0)->SetSlTxPool (txPool);
    
    //indicate NAS that bearer was established
    //TODO: we should only indicate this once per bearer
    for (std::list <uint32_t>::iterator it = destinations.begin() ; it != destinations.end() ; it++) {
      m_asSapUser->NotifySidelinkRadioBearerActivated (*it);
    }
    
  } else {
    //release all tranmission pools
    m_cphySapProvider.at (0)->RemoveSlTxPool ();
  }

}

void
cv2x_LteUeRrc::ApplySidelinkDedicatedConfiguration (cv2x_LteRrcSap::SlDiscConfig config)
{
  NS_LOG_FUNCTION (this);
  if (config.discTxResources == cv2x_LteRrcSap::SlDiscConfig::SETUP) {
    NS_LOG_INFO ("Received dedicated resources (discovery) " << config.setup.setup);

    Ptr<SidelinkTxDiscResourcePool> txPool = CreateObject<SidelinkTxDiscResourcePool>();

    if (config.setup.setup == cv2x_LteRrcSap::SlDiscTxResourcesSetup::SCHEDULED) {
      txPool->SetPool (config.setup.scheduled.discTxConfig);
      txPool->SetScheduledTxParameters (config.setup.scheduled.discTxConfig, config.setup.scheduled.discTfIndexList, config.setup.scheduled.discHoppingConfigDisc);
    } 
    else {
      NS_ASSERT (config.setup.ueSelected.havePoolToAdd && config.setup.ueSelected.poolToAddModList.nbPools > 0);

      // all pools in the same list have the same criteria for poolSelection
      if (config.setup.ueSelected.poolToAddModList.pools[0].pool.txParameters.ueSelectedResourceConfig.poolSelection.selection == cv2x_LteRrcSap::PoolSelection::RSRPBASED) {

        uint8_t i = 0;
        bool poolFound = false;
        while ((i < config.setup.ueSelected.poolToAddModList.nbPools) and (!poolFound)) {

          // retrieve upper and lower RSRP bounds
          // make sure that the current rsrp vlaue is in-between threshLow and threshHigh;
          // otherwise go to the next pool
          uint32_t lowRsrp = config.setup.ueSelected.poolToAddModList.pools[i].pool.txParameters.ueSelectedResourceConfig.poolSelectionRsrpBased.threshLow;
          uint32_t highRsrp = config.setup.ueSelected.poolToAddModList.pools[i].pool.txParameters.ueSelectedResourceConfig.poolSelectionRsrpBased.threshHigh;

          // check the rsrp values : Value 0 corresponds to -infinity, value 1 to -110dBm, 
          // value 2 to -100dBm, and so on (i.e. in steps of 10dBm) until value 6, 
          // which corresponds to -60dBm, while value 7 corresponds to +infinity.
          NS_ASSERT_MSG (lowRsrp <= highRsrp, "Invalid Rsrp limits : lower bound is greater than upper bound");
          NS_ASSERT_MSG ((lowRsrp != 7) and (highRsrp != 0), "invalid RSRP limits values");

          // apply the layer 3 filter before checking the pool
          SaveUeMeasurements (m_cellId, m_storedMeasValues[m_cellId].rsrp, m_storedMeasValues[m_cellId].rsrq, true);

          //low = -infinity
          if (lowRsrp == 0) {
            //high = + infinity
            if (highRsrp == 7) {
              // any rsrp value would be ok : select first pool
              txPool->SetPool (config.setup.ueSelected.poolToAddModList.pools[0].pool);
              poolFound = true;
            }
            //high could be : -110 dBm to -60 dBm 
            else {
              // check if rsrp <= highRsrp
              if (m_storedMeasValues[m_cellId].rsrp <= cv2x_LteRrcSap::RsrpValueDbm (highRsrp)) {
                txPool->SetPool (config.setup.ueSelected.poolToAddModList.pools[i].pool);
                poolFound = true;
              }
            }
          }
          // low could be : -110 dBm to -60 dBm 
          else {
            //high = + infinity
            if (highRsrp == 7) {
              // check if rsrp >= lowRsrp
              if (m_storedMeasValues[m_cellId].rsrp >= cv2x_LteRrcSap::RsrpValueDbm (lowRsrp)) {
                txPool->SetPool (config.setup.ueSelected.poolToAddModList.pools[i].pool);
                poolFound = true;
              }
            }
            //high could be : -110 dBm to -60 dBm
            else {
              // check if lowRsrp <= rsrp <= highRsrp
              if (((m_storedMeasValues[m_cellId].rsrp >= cv2x_LteRrcSap::RsrpValueDbm (lowRsrp)) and (m_storedMeasValues[m_cellId].rsrp <= cv2x_LteRrcSap::RsrpValueDbm (highRsrp))))  {
                txPool->SetPool (config.setup.ueSelected.poolToAddModList.pools[i].pool);
                poolFound = true;
              }
            }
          }         
          //increment; so if the pool is not good, go to the next one
          i++;
        }//end while
        NS_ASSERT_MSG (poolFound, "No pool match the RSRP-based selection");
      }//end if (rsrp)

      else {
      // ue selected : randomly selected using a uniform distribution
      txPool->SetPool (config.setup.ueSelected.poolToAddModList.pools[m_sidelinkConfiguration->m_rand->GetInteger (0, config.setup.ueSelected.poolToAddModList.nbPools - 1)].pool);
      }
    }//end if (ue selected)
        
    //inform MAC about the pool
    m_cmacSapProvider.at (0)->AddSlTxPool (txPool);
    //inform PHY about the pool
    m_cphySapProvider.at (0)->SetSlTxPool (txPool); 

  }//end if (setup)

  else {
    //release all tranmission pools
    m_cphySapProvider.at (0)->RemoveSlTxPool (true); // add bool value so that we can use the function for discovery
  }
}



/**
 * Transmits a SidelinkUEInformation message to the eNodeB
 */
void
cv2x_LteUeRrc::SendSidelinkUeInformation ()
{
  NS_LOG_FUNCTION (this);

  cv2x_LteRrcSap::SidelinkUeInformation sidelinkUeInformation;
  sidelinkUeInformation.haveCommRxInterestedFreq = false;
  sidelinkUeInformation.haveCommTxResourceReq = false;
  sidelinkUeInformation.haveDiscRxInterest = false; 
  sidelinkUeInformation.haveDiscTxResourceReq = false; 

  std::map <uint16_t, cv2x_LteUeRrcSl::LteSlCellConfiguration>::iterator it = m_sidelinkConfiguration->m_slMap.find (m_cellId);
  if (it != m_sidelinkConfiguration->m_slMap.end ()) {
    //if SIB 18 received
    //  if configured to receive sidelink communication
    //    set commRxInterestedFreq with sidelink frequency
    
    //  if configuration to transmit sidelink communication
    //    set commTxResourceReq with sidelink frequency and list of destination
    if (it->second.haveSib18) {
      if (m_sidelinkConfiguration->IsRxInterested ()) {
        sidelinkUeInformation.haveCommRxInterestedFreq = true;
        sidelinkUeInformation.commRxInterestedFreq = GetUlEarfcn ();
      }
      if (m_sidelinkConfiguration->IsTxInterested ()) {
        std::list <uint32_t> destinations = m_sidelinkConfiguration->GetTxDestinations ();
        sidelinkUeInformation.haveCommTxResourceReq = true;
        sidelinkUeInformation.slCommTxResourceReq.carrierFreq = GetUlEarfcn ();
        sidelinkUeInformation.slCommTxResourceReq.slDestinationInfoList.nbDestinations = destinations.size ();
        std::list <uint32_t>::iterator it;
        int index = 0;
        for (it = destinations.begin() ; it != destinations.end() ; it++) {
          sidelinkUeInformation.slCommTxResourceReq.slDestinationInfoList.SlDestinationIdentity[index++] = *it;
        }
      }
    }                         

    // similar for SIB 19
    if (it->second.haveSib19) {

      // UE interested in monitoring discovery announcements
      if  ((m_sidelinkConfiguration->IsMonitoringInterested ()) and (m_sidelinkConfiguration->GetDiscInterFreq () ==  GetUlEarfcn ())) 
      {
        sidelinkUeInformation.haveDiscRxInterest = true;
        sidelinkUeInformation.discRxInterest = true;
      }
      // UE interested in transmit discovery announcements
      if  (m_sidelinkConfiguration->IsAnnouncingInterested ()) {        
        sidelinkUeInformation.haveDiscTxResourceReq = true;      
        NS_ASSERT_MSG (m_sidelinkConfiguration->GetDiscTxResources ()>0, "can't have 0 or negative resources for the discovery announcement. Check if DiscTxResources is defined for in-coverage or eNBs disabled for ou-of-coverage");
        sidelinkUeInformation.discTxResourceReq = m_sidelinkConfiguration->GetDiscTxResources ();
      }
    }

    // Record time
    m_sidelinkConfiguration->RecordTransmissionOfSidelinkUeInformation ();
    // send the message to eNodeB
    m_rrcSapUser->SendSidelinkUeInformation (sidelinkUeInformation);
  }
} 

void cv2x_LteUeRrc::DoNotifySidelinkReception (uint8_t lcId, uint32_t srcL2Id, uint32_t dstL2Id)
{
  NS_LOG_FUNCTION (this << (uint16_t)lcId << srcL2Id << dstL2Id);

  //add LC
  Ptr<cv2x_LteSidelinkRadioBearerInfo> slbInfo = AddSlrb (srcL2Id, dstL2Id, lcId);
  NS_LOG_INFO ("Created new RX SLRB for group " << dstL2Id << " LCID=" << (slbInfo->m_logicalChannelIdentity & 0xF));
}

void cv2x_LteUeRrc::DoNotifyDiscoveryReception (Ptr<cv2x_LteControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  Ptr<cv2x_SlDiscMessage> msg2 = DynamicCast<cv2x_SlDiscMessage> (msg);
  cv2x_SlDiscMsg disc = msg2->GetSlDiscMessage ();
  for (std::list<uint32_t>::iterator it = m_sidelinkConfiguration->m_monitorApps.begin (); it != m_sidelinkConfiguration->m_monitorApps.end (); ++it)
  {
    if ((std::bitset <184>)*it == disc.m_proSeAppCode)
    {
      NS_LOG_INFO ("discovery message received by " << m_rnti << ", proSeAppCode = " << *it);
      m_discoveryMonitoringTrace (m_imsi, m_cellId, m_rnti, *it);
    }
  }
}

void
cv2x_LteUeRrc::SetSlssid(uint64_t slssid){
  NS_LOG_FUNCTION (this << slssid);
  m_slssId = slssid;
  //Pass it to lower layers as well
  m_cphySapProvider.at (0)->SetSlssId(slssid);
}

uint64_t
cv2x_LteUeRrc::GetSlssid()
{
  NS_LOG_FUNCTION (this);
  return m_slssId;
}

uint64_t
cv2x_LteUeRrc::GetFrameNumber()
{
  NS_LOG_FUNCTION (this);
  return m_currFrameNo;
}

uint64_t
cv2x_LteUeRrc::GetSubFrameNumber()
{
  NS_LOG_FUNCTION (this);
  return m_currSubframeNo;
}

void
cv2x_LteUeRrc::InitiateSlssTransmission()
{
  NS_LOG_FUNCTION (this);

  //Try to initiate SLSS transmissions only once ending an off-data period (didn't have data to transmit before)
  if (!m_hasDataToTransmit)
    {
      NS_LOG_LOGIC (this << " the UE has data to transmit");
      m_hasDataToTransmit = true;
      if (m_hasSyncRef && IsInTheInnerPartOfTheSyncRefCell(m_currSyncRef.slssid,m_currSyncRef.rxOffset))
        {
          m_inInnerCellOfSyncRef = true;
          NS_LOG_LOGIC (this << " the UE is in the inner cell of the selected SyncRef, no SLSS transmissions initiated");
        }
      else
        {
          NS_LOG_LOGIC (this << " the UE is in the outer cell of the selected SyncRef, SLSS transmissions initiated");
          ActivateSlssTransmission();
        }
    }
}

void
cv2x_LteUeRrc::StopSlssTransmission()
{
  NS_LOG_FUNCTION (this);

  //Try to stop SLSS transmissions only once ending an on-data period (had data to transmit before)
  if(m_hasDataToTransmit)
    {
      m_hasDataToTransmit = false;
      NS_LOG_LOGIC (this << " the UE has no data to transmit, stopping SLSS transmission");
      DeactivateSlssTransmission();
    }
}

void cv2x_LteUeRrc::ActivateSlssTransmission ()
{
  NS_LOG_FUNCTION (this);

  if (!m_slssTransmissionActive)
    {
      NS_LOG_LOGIC (this << " the SLSS transmission was not active, configuring it and activating it");
      m_slssTransmissionActive = true;
      cv2x_LteRrcSap::SlPreconfiguration preconf = m_sidelinkConfiguration->GetSlPreconfiguration ();
      //Configure own  SLSSID and SyncOffsetIndicator
      if(m_hasSyncRef || m_currSyncRef.slssid != 0)
        {
          NS_LOG_LOGIC (this << " the UE has a selected SyncRef, using its SLSSID and the other offset indicator");
          //Use the values from the SyncRef
          m_slssId = m_currSyncRef.slssid;
          uint16_t currSyncRefSyncOffsetIndicator = (10*m_currSyncRef.directFrameNo + m_currSyncRef.directSubframeNo) % 40;
          if(currSyncRefSyncOffsetIndicator == preconf.preconfigSync.syncOffsetIndicator1 )
            {
              m_txSlSyncOffsetIndicator = preconf.preconfigSync.syncOffsetIndicator2;
            } else{
                m_txSlSyncOffsetIndicator = preconf.preconfigSync.syncOffsetIndicator1;
            }
        }
      else
        {
          NS_LOG_LOGIC (this << " the UE does not have a selected SyncRef, selecting SLSSID and offset indicator randomly");
          //Choose SLSSID Randomly between 168 and 335
          SlChangeOfSyncRefStatParameters param;

           param.imsi=m_imsi;
           param.prevSlssid =m_slssId;
           param.prevRxOffset=0;
           param.prevFrameNo=m_currFrameNo;
           param.prevSubframeNo=m_currSubframeNo;

           m_slssId= m_imsi*10;

           param.currSlssid=m_slssId;
           param.currRxOffset=0;
           param.currFrameNo=m_currFrameNo;
           param.currSubframeNo=m_currSubframeNo;

           m_ChangeOfSyncRefTrace(param);

          //Choose SyncOffsetIndicator randomly between the two preconfigured values
          Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
          if(x->GetInteger(0,1))
            {
              m_txSlSyncOffsetIndicator = preconf.preconfigSync.syncOffsetIndicator1;
            }
          else
            {
              m_txSlSyncOffsetIndicator = preconf.preconfigSync.syncOffsetIndicator2;
            }
        }
      //Schedule the sending of the first MIB-SL according the selected SyncOffsetIndicator
      uint16_t nextSLSS=0;
      uint16_t currOffset = (m_currFrameNo*10 + m_currSubframeNo) % 40 ;
      if ( currOffset < m_txSlSyncOffsetIndicator)
        {
          nextSLSS = m_txSlSyncOffsetIndicator - currOffset;
        }
      else
        {
          nextSLSS = 40 - currOffset + m_txSlSyncOffsetIndicator;
        }
      NS_LOG_INFO (this << " UE IMSI "<<m_imsi <<" activating SLSS transmission with SLSSID " << m_slssId <<", first SLSS in "<< nextSLSS<< "ms");


      Simulator::Schedule (MilliSeconds(nextSLSS), &cv2x_LteUeRrc::SendSlss, this);
      m_slssTxTime = MilliSeconds(Simulator::Now().GetMilliSeconds()+nextSLSS);
    }
}

void cv2x_LteUeRrc::DeactivateSlssTransmission ()
{
  NS_LOG_FUNCTION (this);

  if (m_slssTransmissionActive)
    {
      m_slssTransmissionActive = false;
    }
}

void cv2x_LteUeRrc::SaveSubframeIndication(uint16_t frameNo, uint16_t subFrameNo)
{
  //NS_LOG_FUNCTION (this << frameNo << subFrameNo ); //To much overhead as it is called every ms
  m_currFrameNo = frameNo;
  m_currSubframeNo = subFrameNo;
}

void
cv2x_LteUeRrc::SendSlss(){

  NS_LOG_FUNCTION (this);

  if(m_slssTransmissionActive)
    {
      NS_LOG_LOGIC (this << " the SLSS transmission is active");
      //Send a SLSS if the UE is supposed to send it now (e.g. it didn't change of timing/SyncRef)
      if((m_currFrameNo*10 + m_currSubframeNo) % 40 == m_txSlSyncOffsetIndicator)
        {
          NS_LOG_LOGIC (this << " the UE is supposed to send the SLSS now");

          cv2x_LteRrcSap::MasterInformationBlockSL mibSl;
          cv2x_LteRrcSap::SlPreconfiguration preconf = m_sidelinkConfiguration->GetSlPreconfiguration ();

          mibSl.slBandwidth = preconf.preconfigGeneral.slBandwidth;
          mibSl.inCoverage = m_inCoverage;
          mibSl.directFrameNo = m_currFrameNo;
          mibSl.directSubframeNo = m_currSubframeNo ;
          mibSl.creationTimestamp = Simulator::Now();

          if(!m_hasSyncRef)
            {
              mibSl.slssid = m_slssId;
            }
          else
            {
              mibSl.slssid = m_currSyncRef.slssid;
            }

          //Send the SLSS
          NS_LOG_INFO (this <<" UE IMSI "<<m_imsi <<" sending SLSS");
          NS_LOG_INFO (this << " mibSl.slBandwidth "<<mibSl.slBandwidth
                       <<" mibSl.inCoverage "<<mibSl.inCoverage
                       <<" mibSl.directFrameNo "<<mibSl.directFrameNo
                       <<" mibSl.directSubframeNo "<<mibSl.directSubframeNo
                       <<" mibSl.slssid "<<mibSl.slssid);

          m_SendSlssTrace(m_imsi,mibSl.slssid,m_txSlSyncOffsetIndicator,mibSl.inCoverage,mibSl.directFrameNo,mibSl.directSubframeNo);

          m_cphySapProvider.at (0)->SendSlss(mibSl);
        }

      //Calculate when the next SLSS is supposed to be send
      uint16_t nextSLSS=0;
      uint16_t currOffset = (m_currFrameNo*10 + m_currSubframeNo) % 40 ;
      if ( currOffset < m_txSlSyncOffsetIndicator)
        {
          nextSLSS = m_txSlSyncOffsetIndicator - currOffset;
        }
      else
        {
          nextSLSS = 40 - currOffset + m_txSlSyncOffsetIndicator;
        }

      //Schedule the sent of the SLSS if it wasn't scheduled already
      if (m_slssTxTime.GetMilliSeconds() == Simulator::Now().GetMilliSeconds())
        {
          Simulator::Schedule (MilliSeconds(nextSLSS), &cv2x_LteUeRrc::SendSlss, this);
          m_slssTxTime = MilliSeconds(Simulator::Now().GetMilliSeconds()+nextSLSS);
          NS_LOG_LOGIC (this << " UE IMSI "<<m_imsi <<" scheduled a SLSS to be sent in "<<nextSLSS<<" ms");
        }
      else
        {
          NS_LOG_LOGIC (this << " UE IMSI "<<m_imsi <<" Duplicated SLSS scheduling, ignoring");
        }
    }
}

void
cv2x_LteUeRrc::SaveSlssMeasurements (uint16_t slssid, uint16_t offset, double srsrp, bool useLayer3Filtering)
{
  NS_LOG_FUNCTION (this << slssid << offset << srsrp << useLayer3Filtering);

  std::map <std::pair<uint16_t,uint16_t>, SlssMeasValues>::iterator storedMeasIt = m_storedSlssMeasValues.find (std::pair<uint16_t,uint16_t>(slssid,offset));

  if (storedMeasIt != m_storedSlssMeasValues.end ())
    {
      NS_LOG_LOGIC (this << " This SyncRef was measured in the past");

      if (useLayer3Filtering)
        {
          cv2x_LteRrcSap::SlPreconfiguration preconf = m_sidelinkConfiguration->GetSlPreconfiguration ();
          NS_LOG_LOGIC (this << " Using L3 filtering with preconfigured filterCoefficient: "<< preconf.preconfigSync.filterCoefficient);

          bool valid = false;

          //Converting stored S-RSRP to linear units
          double storedSrsrp_W = std::pow(10.0,storedMeasIt->second.srsrp/10.0)/1000.0;

          double Fn_W;
          if (srsrp > m_minSrsrp)
            {
              NS_LOG_LOGIC (this << " The new SyncRef S-RSRP is above the minimum required, applying filter");

              double newSrsrp_W = std::pow(10.0,srsrp/10.0)/1000.0;

              //The filter to be used according to //TS 36.331 5.5.3.2 is
              //      F_n = (1-a)*F_{n-1} + a*M_n
              //and it is defined for an input rate of 200 ms.
              //If the input rate is different, the filter needs to be adapted such that
              //its time characteristics are preserved.
              //This is achieved using:
              //      F_n = (1-a)^(nP) F_{n-1} + a M_n
              //where nP is the number of 200 ms periods that have elapsed since last time
              //the quantity was filtered and stored

              double aSrsrp = std::pow (0.5, preconf.preconfigSync.filterCoefficient / 4.0); //TS 36.331 5.5.3.2 is
              double nP = (Simulator::Now().GetMilliSeconds() -  storedMeasIt->second.timestamp.GetMilliSeconds()) / 200;
              //std::cout<< Simulator::Now().GetMilliSeconds()<<" imsi: "<<m_imsi<< " slssid: "<< slssid << " offset: "<<offset <<" srsrp: "<< srsrp<<
              //    "timestamp: "<<storedMeasIt->second.timestamp.GetMilliSeconds()<< std::endl;
              Fn_W = std::pow ((1 - aSrsrp),nP) * storedSrsrp_W + aSrsrp * newSrsrp_W;
              valid=true;
            }
          else
            {
              NS_LOG_LOGIC (this << " The new SyncRef S-RSRP is below or equal to the minimum required... Ignoring measurement");
            }

          if (valid)
            {
            //Converting filtered value to dBm
            double Fn_dBm = 10 * log10 (1000 * (Fn_W));

            //If after the filtering, it decays below the minimum required, use the minimum required
            if(Fn_dBm <= m_minSrsrp)
              {
                NS_LOG_LOGIC (this << " The L3 filtered SyncRef S-RSRP is below or equal to the minimum required, storing minimum required");
                storedMeasIt->second.srsrp = m_minSrsrp;
                storedMeasIt->second.timestamp = Simulator::Now ();
              }
            else
              {
                NS_LOG_LOGIC (this << " The L3 filtered SyncRef S-RSRP is above the minimum required, storing it");
                storedMeasIt->second.srsrp = Fn_dBm;
                storedMeasIt->second.timestamp = Simulator::Now ();
              }
            }
        }
      else
        {
          NS_LOG_LOGIC (this << " Not using L3 filtering");
          storedMeasIt->second.srsrp = srsrp;
          storedMeasIt->second.timestamp = Simulator::Now ();
        }
    }
  else
    {
      NS_LOG_LOGIC (this << " First time measuring this SyncRef, storing first value");
      SlssMeasValues v;
      v.srsrp = srsrp;
      v.timestamp = Simulator::Now ();
      m_storedSlssMeasValues.insert(std::pair< std::pair<uint16_t,uint16_t>, SlssMeasValues>(std::pair<uint16_t,uint16_t>(slssid,offset), v));
      m_knownSlssidList.push_back(std::pair<uint16_t,uint16_t>(slssid,offset));
    }
  NS_LOG_INFO (this <<" IMSI " << m_imsi << " measured SyncRef with slssid " << slssid << " and offset " << offset <<" New S-RSRP "
               << srsrp << " Stored S-RSRP " << storedMeasIt->second.srsrp);
}

void
cv2x_LteUeRrc::SynchronizeToStrongestSyncRef ()
{
  NS_LOG_FUNCTION (this);

  uint64_t maxSrsrpSlssid = 0;
  uint64_t maxSrsrpOffset = 0;

  double maxSrsrp = -std::numeric_limits<double>::infinity ();

  //Determine the strongest SyncRef
  std::vector <std::pair<uint16_t,uint16_t> >::iterator RepIt;
  for (RepIt = m_lastReportedSlssidList.begin (); RepIt != m_lastReportedSlssidList.end (); ++RepIt)
    {
      std::map <std::pair<uint16_t,uint16_t>, SlssMeasValues>::iterator it = m_storedSlssMeasValues.find(std::pair<uint16_t,uint16_t>(*RepIt));
      if (it == m_storedSlssMeasValues.end ())
        {
          NS_LOG_ERROR(" The UE cannot find the S-RSRP measurements for the detected SyncRef SLSSID " << RepIt->first <<" offset " <<RepIt->second );
        }
      else
        {
          if (maxSrsrp < it->second.srsrp)
            {
              maxSrsrpSlssid = it->first.first ;
              maxSrsrpOffset = it->first.second;
              maxSrsrp = it->second.srsrp;
            }
        }
    }

  NS_LOG_INFO (this <<" The strongest SyncRef SLSSID "<< maxSrsrpSlssid << " offset "<<maxSrsrpOffset <<" has a S-RSRP of "<<maxSrsrp<<" dBm" );

  cv2x_LteRrcSap::SlPreconfiguration preconf = m_sidelinkConfiguration->GetSlPreconfiguration ();

  if (maxSrsrpSlssid == 0 || maxSrsrp - m_minSrsrp <  preconf.preconfigSync.syncRefMinHyst)
    {
      NS_LOG_LOGIC (this << " Unable to find suitable SyncRef");
      if(!m_slssTransmissionActive && m_hasDataToTransmit)
        {
          NS_LOG_LOGIC (this <<" The UE is transmitting sidelink communication... Activating own SLSS transmission");
          ActivateSlssTransmission();
        }
    }
  else
    {
      NS_LOG_LOGIC (this << " The strongest SyncRef is suitable");

      //Verify we have the mib for the strongest SyncRef
      std::map <std::pair<uint16_t,uint16_t>, cv2x_LteRrcSap::MasterInformationBlockSL>::iterator itMap
      = m_latestMibSlReceived.find (std::pair<uint16_t,uint16_t>(maxSrsrpSlssid,maxSrsrpOffset ));
      if (itMap == m_latestMibSlReceived.end ())
        {
          NS_LOG_ERROR(" The UE is unable to find the MIB-SL of the strongest SyncRef");
        }
      else
        {
          NS_LOG_LOGIC (this << " The MIB-SL of the strongest SyncRef was found... Instructing synchronization");

          NS_LOG_INFO (this <<" UE IMSI "<<m_imsi <<" selected SyncRef slssid "<<maxSrsrpSlssid << " offset " << maxSrsrpOffset);
          NS_LOG_INFO (this <<" mibSl.slBandwidth "<<(*itMap).second.slBandwidth
                       <<" mibSl.inCoverage "<<(*itMap).second.inCoverage
                       <<" mibSl.directFrameNo "<<(*itMap).second.directFrameNo
                       <<" mibSl.directSubframeNo "<<(*itMap).second.directSubframeNo
                       <<" mibSl.slssid "<<(*itMap).second.slssid
                       <<" mibSl.creationTimestamp "<<(*itMap).second.creationTimestamp.GetMilliSeconds()
                       <<" mibSl.rxTimestamp "<<(*itMap).second.rxTimestamp.GetMilliSeconds());

          //Instruct the phy to syncronize with maxSrsrpSlssid
          m_cphySapProvider.at (0)->SynchronizeToSyncRef((*itMap).second);
        }
    }
}

bool
cv2x_LteUeRrc::IsInTheInnerPartOfTheSyncRefCell(uint16_t slssid, uint16_t offset)
{
  NS_LOG_FUNCTION (this << slssid << offset);

  cv2x_LteRrcSap::SlPreconfiguration preconf = m_sidelinkConfiguration->GetSlPreconfiguration ();

  std::map <std::pair<uint16_t,uint16_t>, SlssMeasValues>::iterator itMap = m_storedSlssMeasValues.find(std::pair<uint16_t,uint16_t>(slssid,offset));
  if (itMap == m_storedSlssMeasValues.end ())
    {
      NS_LOG_ERROR(" The UE cannot find the S-RSRP measurements for the selected SyncRef SLSSID " << slssid <<" offset " << offset);
      return false;
    }
  else
    {
      if ((*itMap).second.srsrp >= preconf.preconfigSync.syncTxThreshOoC)
        {
          NS_LOG_LOGIC (this << " The SyncRef S-RSRP is above or equal to the configured syncTxThreshOoC, i.e., the UE is in the inner part of the SyncRef cell");
          return true;
        }
      else
        {
          NS_LOG_LOGIC (this << " The SyncRef S-RSRP is below the configured syncTxThreshOoC, i.e., the UE is in the outer part of the SyncRef cell");
          return false;
        }
    }
}

void cv2x_LteUeRrc::DoReceiveMibSL (cv2x_LteRrcSap::MasterInformationBlockSL mibSl)
{

  NS_LOG_FUNCTION (this);

  mibSl.rxTimestamp = Simulator::Now();
  mibSl.rxOffset = Simulator::Now().GetMilliSeconds() % 40;

  NS_LOG_INFO (this <<" UE IMSI "<<m_imsi <<" received MIB-SL ");
  NS_LOG_INFO (this <<" mibSl.slBandwidth "<<mibSl.slBandwidth
               <<", mibSl.inCoverage "<<mibSl.inCoverage
               <<", mibSl.directFrameNo "<<mibSl.directFrameNo
               <<", mibSl.directSubframeNo "<<mibSl.directSubframeNo
               <<", mibSl.creationTimestamp "<< mibSl.creationTimestamp.GetMilliSeconds()<<" (ms)"
               <<", mibSl.rxTimestamp "<<mibSl.rxTimestamp.GetMilliSeconds()<<" (ms)"
               <<", mibSl.slssid "<<mibSl.slssid
               <<", mibSl.rxOffset "<<mibSl.rxOffset);


  //Store the mib
  std::map <std::pair<uint16_t,uint16_t>, cv2x_LteRrcSap::MasterInformationBlockSL>::iterator itMap
  = m_latestMibSlReceived.find (std::pair<uint16_t,uint16_t>(mibSl.slssid, mibSl.rxOffset));

  if (itMap == m_latestMibSlReceived.end ())
    {
      //Insert new entry
      NS_LOG_LOGIC (this << " First received MIB-SL for SyncRef with SLSSID "<<mibSl.slssid << " offset " <<mibSl.rxOffset);
      m_latestMibSlReceived.insert (std::pair <std::pair<uint16_t,uint16_t>, cv2x_LteRrcSap::MasterInformationBlockSL> (std::pair<uint16_t,uint16_t>(mibSl.slssid, mibSl.rxOffset), mibSl));
    }
  else{
      //Replace the entry
      NS_LOG_LOGIC (this << " Updating stored MIB-SL for SyncRef with SLSSID "<<mibSl.slssid << " offset " <<mibSl.rxOffset);
      (*itMap).second.slBandwidth = mibSl.slBandwidth;
      (*itMap).second.inCoverage = mibSl.inCoverage;
      (*itMap).second.directFrameNo = mibSl.directFrameNo;
      (*itMap).second.directSubframeNo = mibSl.directSubframeNo;
      (*itMap).second.slssid = mibSl.slssid;
      (*itMap).second.rxTimestamp= mibSl.rxTimestamp;
      (*itMap).second.creationTimestamp= mibSl.creationTimestamp;
  }
  //Verify if it is a MIB-SL from the current SyncRef
  if(m_hasSyncRef && mibSl.slssid == m_currSyncRef.slssid){
      NS_LOG_LOGIC (this << " The received MIB-SL is from the selected SyncRef (SLSSID "<<mibSl.slssid << " offset " <<mibSl.rxOffset<<")");
  }
}

void cv2x_LteUeRrc::DoReportSlssMeasurements ( cv2x_LteUeCphySapUser::UeSlssMeasurementsParameters params, uint64_t slssid, uint16_t offset )
{
  NS_LOG_FUNCTION (this);

  bool useLayer3Filtering = false;
  cv2x_LteRrcSap::SlPreconfiguration preconf = m_sidelinkConfiguration->GetSlPreconfiguration ();
  if (preconf.preconfigSync.filterCoefficient != 0){
      NS_LOG_LOGIC (this << " Using L3 filtering");
      useLayer3Filtering = true;
  }

  if (m_knownSlssidList.size() > 0 && slssid == 0){ //Measurement for SyncRef selection report

      NS_LOG_LOGIC (this << " The report corresponds to the measurement sub-process");

      //Storing the measurements of the known SLSSIDs
      std::vector <std::pair<uint16_t,uint16_t> >::iterator knownSlssidIit;
      for (knownSlssidIit = m_knownSlssidList.begin ();
          knownSlssidIit != m_knownSlssidList.end (); ++knownSlssidIit)
        {
          bool found = false;
          std::vector <cv2x_LteUeCphySapUser::UeSlssMeasurementReportElement>::iterator newMeasIt;
          for (newMeasIt = params.m_ueSlssMeasurementsList.begin ();
              newMeasIt != params.m_ueSlssMeasurementsList.end (); newMeasIt++)
            {
              if ( (*knownSlssidIit).first ==  newMeasIt->m_slssid && (*knownSlssidIit).second ==  newMeasIt->m_offset)
                {
                  NS_LOG_LOGIC (this <<" UE IMSI "<<m_imsi<< " reported measurements from known SincRef with SLSSID "<<newMeasIt->m_slssid<<" offset "<<newMeasIt->m_offset<<" S-RSRP "<<newMeasIt->m_srsrp );
                  SaveSlssMeasurements(newMeasIt->m_slssid,newMeasIt->m_offset,newMeasIt->m_srsrp,useLayer3Filtering);
                  m_lastReportedSlssidList.push_back(std::pair<uint16_t,uint16_t>(newMeasIt->m_slssid,newMeasIt->m_offset));
                  found = true;
                  break;
                }
            }
          if (!found)
            {
              NS_LOG_LOGIC (this <<" UE IMSI "<<m_imsi<< " did not report measurements from known SincRef with SLSSID"<<(*knownSlssidIit).first << " offset "<<(*knownSlssidIit).second);
            }
        }
      //Adding the measures of the not known SLSSIDs
      std::vector <cv2x_LteUeCphySapUser::UeSlssMeasurementReportElement>::iterator newMeasIt;
      for (newMeasIt = params.m_ueSlssMeasurementsList.begin ();
          newMeasIt != params.m_ueSlssMeasurementsList.end (); newMeasIt++)
        {
          bool found = false;
          std::vector <std::pair<uint16_t,uint16_t> >::iterator knownSlssidIit;
          for (knownSlssidIit = m_knownSlssidList.begin ();
              knownSlssidIit != m_knownSlssidList.end (); ++knownSlssidIit)
            {
              if ( (*knownSlssidIit).first ==  newMeasIt->m_slssid && (*knownSlssidIit).second ==  newMeasIt->m_offset)
                {
                  found = true;
                  break;
                }
            }
          if(!found)
            {
              NS_LOG_LOGIC (this <<" UE IMSI "<<m_imsi<< " reported measurements from not known SincRef with SLSSID "<<newMeasIt->m_slssid<< " offset "<<newMeasIt->m_offset<<" S-RSRP "<<newMeasIt->m_srsrp);
              SaveSlssMeasurements(newMeasIt->m_slssid,newMeasIt->m_offset, newMeasIt->m_srsrp,useLayer3Filtering);
              m_lastReportedSlssidList.push_back(std::pair<uint16_t,uint16_t>(newMeasIt->m_slssid,newMeasIt->m_offset));
              found = true;
            }
        }
  }
  else if (m_knownSlssidList.size() == 0) //The first report with detected SyncRefs
    {
      NS_LOG_LOGIC (this << " The report corresponds to the first measurement sub-process with detected SyncRefs");

      std::vector <cv2x_LteUeCphySapUser::UeSlssMeasurementReportElement>::iterator newMeasIt;
      for (newMeasIt = params.m_ueSlssMeasurementsList.begin ();
          newMeasIt != params.m_ueSlssMeasurementsList.end (); ++newMeasIt)
        {
          NS_LOG_LOGIC (this <<" UE IMSI "<<m_imsi<< " reported measurements from not known SincRef with SLSSID "<<newMeasIt->m_slssid<< " offset "<<newMeasIt->m_offset<<" S-RSRP "<<newMeasIt->m_srsrp);
          SaveSlssMeasurements(newMeasIt->m_slssid,newMeasIt->m_offset, newMeasIt->m_srsrp,useLayer3Filtering);
          m_lastReportedSlssidList.push_back(std::pair<uint16_t,uint16_t>(newMeasIt->m_slssid,newMeasIt->m_offset));

        }
    }
  else if (slssid > 0)//Specific SLSSID report (i.e., evaluation of selected SyncRef)
    {
      NS_LOG_LOGIC (this << " The report corresponds to the evaluation sub-process");

      std::vector <cv2x_LteUeCphySapUser::UeSlssMeasurementReportElement>::iterator newMeasIt;
      if (params.m_ueSlssMeasurementsList.size() == 1)
        {
          newMeasIt = params.m_ueSlssMeasurementsList.begin();
          if(newMeasIt->m_slssid == slssid)
            {
              NS_LOG_LOGIC (this <<" UE IMSI "<<m_imsi<< " reported measurements from selected SincRef with SLSSID "<<newMeasIt->m_slssid<< " offset "<<newMeasIt->m_offset<<" S-RSRP "<<newMeasIt->m_srsrp);
              SaveSlssMeasurements(newMeasIt->m_slssid,newMeasIt->m_offset, newMeasIt->m_srsrp,useLayer3Filtering);
              m_lastReportedSlssidList.push_back(std::pair<uint16_t,uint16_t>(slssid,offset));
            }
          else
            {
              NS_LOG_ERROR(" The S-RSRP measurements does not correspond to the intended SyncRef with SLSSID "<<slssid<<" offset "<<offset);
            }
        }
      else if (params.m_ueSlssMeasurementsList.size() == 0) //The SyncRef disappeared
        {
          NS_LOG_LOGIC (this <<" The UE is unable to find the S-RSRP measurements of the selected SyncRef with SLSSID "<<slssid<<" offset "<<offset);
        }
      else if (params.m_ueSlssMeasurementsList.size() > 1)
        {
          NS_LOG_ERROR(" The S-RSRP measurement report intended for evaluation of selected SyncRef has more than one SyncRef reported ");
        }
    }

  //Selection of the SyncRef
  if(m_hasSyncRef && slssid == 0)
    {
      NS_LOG_LOGIC (this <<" Evaluating validity of current SyncRef");

      cv2x_LteRrcSap::SlPreconfiguration preconf = m_sidelinkConfiguration->GetSlPreconfiguration ();
      //Select strongest candidate
      uint16_t maxSrsrpSlssid = 0;
      uint64_t maxSrsrpOffset = 0;
      bool syncRefDetected = false;
      double maxSrsrp = -std::numeric_limits<double>::infinity ();
      std::vector <std::pair<uint16_t,uint16_t> >::iterator RepIt;
      for (RepIt = m_lastReportedSlssidList.begin (); RepIt != m_lastReportedSlssidList.end (); ++RepIt)
        {
          if ( (*RepIt).first == m_currSyncRef.slssid && (*RepIt).second == m_currSyncRef.rxOffset){
              syncRefDetected = true;
          }
          std::map <std::pair<uint16_t,uint16_t>, SlssMeasValues>::iterator it = m_storedSlssMeasValues.find(std::pair<uint16_t,uint16_t>(*RepIt));
          if (it == m_storedSlssMeasValues.end ())
            {
              NS_LOG_ERROR(" The UE cannot find the S-RSRP measurements for the detected SyncRef SLSSID " << RepIt->first <<" offset " <<RepIt->second );
            }
          else
            {
              if (maxSrsrp < it->second.srsrp)
                {
                  maxSrsrpSlssid = it->first.first;
                  maxSrsrpOffset = it->first.second;
                  maxSrsrp = it->second.srsrp;
                }
            }
        }

      NS_LOG_INFO (this <<" The strongest SyncRef SLSSID "<< maxSrsrpSlssid << " offset "<<maxSrsrpOffset <<" has a S-RSRP of "<<maxSrsrp<<" dBm" );
      //Strongest candidate exceeds the minimum required?
      if (maxSrsrpSlssid != 0 && maxSrsrp - m_minSrsrp >  preconf.preconfigSync.syncRefMinHyst)
        {
          //Yes
          NS_LOG_LOGIC(this <<" The strongest candidate S-RSRP exceeds the minimum required by the corresponding threshold syncRefMinHyst" );

          //The selected SyncRef was detected?
          if (!syncRefDetected)
            {
              //No... consider the selected SyncRef not valid
              NS_LOG_LOGIC(this <<" The selected SyncRef was not detected... Considering it not valid" );
              m_hasSyncRef = false;
            }
          else
            {
              //Yes
              std::map <std::pair<uint16_t,uint16_t>, SlssMeasValues>::iterator itMap = m_storedSlssMeasValues.find (std::pair<uint16_t,uint16_t>(m_currSyncRef.slssid,m_currSyncRef.rxOffset));
              if (itMap == m_storedSlssMeasValues.end ())
                {
                  NS_LOG_LOGIC(this <<" The UE cannot find the S-RSRP measurements for the selected SyncRef... Considering it not valid"  );
                  m_hasSyncRef = false;
                }
              else
                {
                  //Strongest candidate better than the selected SyncRef?
                  if (maxSrsrp - (*itMap).second.srsrp > preconf.preconfigSync.syncRefDiffHyst)
                    {
                      //Yes... Consider not valid the selected SyncRef
                      NS_LOG_LOGIC(this <<" The strongest candidate S-RSRP exceeds the selected SyncRef one by the corresponding threshold syncRefDiffHyst... Considering the selected SyncRef not valid" );
                      m_hasSyncRef = false;
                    }
                  else
                    {
                      //No... Keep the selected SyncRef
                      NS_LOG_LOGIC(this <<" The strongest candidate S-RSRP does not exceed the selected SyncRef one by the corresponding threshold syncRefDiffHyst... Keeping the selected SyncRef" );
                    }
                }
            }
        }
      else
        {
          //No... Keep the selected SyncRef
          NS_LOG_LOGIC(this <<" The strongest candidate S-RSRP does not exceed the minimum required by the corresponding threshold syncRefMinHyst" );
        }
    }
  if(!m_hasSyncRef && slssid == 0)
    {
      NS_LOG_LOGIC(this <<" The UE does not have a (valid) selected SyncRef... Selecting the strongest candidate if approppriate" );
      //Evaluate candidates for selection of SyncRef
      SynchronizeToStrongestSyncRef ();
    }

  if (m_hasSyncRef && slssid > 0)
    {
      NS_LOG_LOGIC(this <<" Evaluating S-RSRP measurements of the selected SyncRef" );

      if(IsInTheInnerPartOfTheSyncRefCell(m_currSyncRef.slssid, m_currSyncRef.rxOffset))
        {
          NS_LOG_LOGIC (this << " the UE is in the inner cell of the selected SyncRef");
          if(m_slssTransmissionActive == true )
            {
              NS_LOG_LOGIC (this << " the UE was transmitting SLSS... Deactivating the SLSS transmission");
              DeactivateSlssTransmission ();
            }
          m_inInnerCellOfSyncRef = true;
        }
      else
        {
          NS_LOG_LOGIC (this << " the UE is in the outer cell of the selected SyncRef");

          if(m_inInnerCellOfSyncRef && m_hasDataToTransmit)
            {
              // It was in the inner cell, but not anymore

              NS_LOG_LOGIC (this << " the UE was in the outer cell of the selected SyncRef (but not anymore) "
                  "and it is transmitting sidelink communication... Activating SLSS transmission");
             ActivateSlssTransmission ();
            }
          m_inInnerCellOfSyncRef = false;
        }
    }
  m_lastReportedSlssidList.clear();
}

void
cv2x_LteUeRrc::DoReportChangeOfSyncRef (cv2x_LteRrcSap::MasterInformationBlockSL mibSl, uint16_t frameNo, uint16_t subFrameNo)
{
  NS_LOG_FUNCTION (this);

  m_hasSyncRef = true;

  uint16_t previousFrameNo = m_currFrameNo;
  uint16_t previousSubFrameNo = m_currSubframeNo;

  //Save the current subframe indication
  SaveSubframeIndication(frameNo, subFrameNo);

  SlChangeOfSyncRefStatParameters param;
  param.imsi=m_imsi;
  param.prevSlssid =m_slssId;
  param.prevRxOffset=m_currSyncRef.rxOffset;
  param.prevFrameNo=previousFrameNo;
  param.prevSubframeNo=previousSubFrameNo;

  //Storing the value of the MIB (Note this is not the current frameNo and subframeNo)
  m_currSyncRef.directFrameNo = mibSl.directFrameNo;
  m_currSyncRef.directSubframeNo = mibSl.directSubframeNo;
  m_currSyncRef.inCoverage = mibSl.inCoverage;
  m_currSyncRef.rxTimestamp = mibSl.rxTimestamp;
  m_currSyncRef.creationTimestamp = mibSl.creationTimestamp;
  m_currSyncRef.slBandwidth = mibSl.slBandwidth;
  m_currSyncRef.slssid = mibSl.slssid;
  m_currSyncRef.rxOffset = mibSl.rxOffset;

  m_slssId = mibSl.slssid;

  param.currSlssid=m_slssId;
  param.currRxOffset=mibSl.rxOffset;
  param.currFrameNo=m_currFrameNo;
  param.currSubframeNo=m_currSubframeNo;

  m_ChangeOfSyncRefTrace(param);

  NS_LOG_INFO (this <<" UE IMSI "<<m_imsi <<" reported successful change of SyncRef, selected SyncRef SLSSID "<< mibSl.slssid <<"offset "<< mibSl.rxOffset);
}

void cv2x_LteUeRrc::DoReportSubframeIndication(uint16_t frameNo, uint16_t subFrameNo)
{
  //NS_LOG_FUNCTION (this << frameNo << subFrameNo ); // Too much overhead as it is called every ms
  SaveSubframeIndication(frameNo, subFrameNo);
}

void
cv2x_LteUeRrc::DoNotifyMacHasSlDataToSend()
{
  NS_LOG_FUNCTION (this);
  if (m_slssTransmissionEnabled){
      NS_LOG_LOGIC(this <<" The MAC notified that it has data to send, and SLSS transmissions are enabled... Initiating SLSS transmissions" );
      InitiateSlssTransmission();
  }
}

void
cv2x_LteUeRrc::DoNotifyMacHasNotSlDataToSend()
{
  NS_LOG_FUNCTION (this);
  if (m_slssTransmissionEnabled){
      NS_LOG_LOGIC(this <<" The MAC notified that it does not have anymore data to send, and SLSS transmissions are enabled... Stoping SLSS transmissions" );
      StopSlssTransmission();
  }
}


} // namespace ns3


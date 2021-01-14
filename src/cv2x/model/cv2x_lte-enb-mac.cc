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
 * Author: Marco Miozzo <marco.miozzo@cttc.es>
 *         Nicola Baldo  <nbaldo@cttc.es>
 * Modified by:
 *          Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 *          Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)
 *          NIST (D2D)
 */


#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>

#include "cv2x_lte-amc.h"
#include "cv2x_lte-control-messages.h"
#include "cv2x_lte-enb-net-device.h"
#include "cv2x_lte-ue-net-device.h"

#include <ns3/cv2x_lte-enb-mac.h>
#include <ns3/cv2x_lte-radio-bearer-tag.h>
#include <ns3/cv2x_lte-ue-phy.h>

#include "ns3/cv2x_lte-mac-sap.h"
#include "ns3/cv2x_lte-enb-cmac-sap.h"
#include <ns3/cv2x_lte-common.h>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteEnbMac");

NS_OBJECT_ENSURE_REGISTERED (cv2x_LteEnbMac);



// //////////////////////////////////////
// member SAP forwarders
// //////////////////////////////////////


/// cv2x_EnbMacMemberLteEnbCmacSapProvider class
class cv2x_EnbMacMemberLteEnbCmacSapProvider : public cv2x_LteEnbCmacSapProvider
{
public:
  /**
   * Constructor
   *
   * \param mac the MAC
   */
  cv2x_EnbMacMemberLteEnbCmacSapProvider (cv2x_LteEnbMac* mac);

  // inherited from cv2x_LteEnbCmacSapProvider
  virtual void ConfigureMac (uint8_t ulBandwidth, uint8_t dlBandwidth);
  virtual void AddUe (uint16_t rnti);
  virtual void RemoveUe (uint16_t rnti);
  virtual void AddLc (LcInfo lcinfo, cv2x_LteMacSapUser* msu);
  virtual void ReconfigureLc (LcInfo lcinfo);
  virtual void ReleaseLc (uint16_t rnti, uint8_t lcid);
  virtual void UeUpdateConfigurationReq (UeConfig params);
  virtual void AddPool (uint32_t group, Ptr<SidelinkCommResourcePool> pool);
  virtual void RemovePool (uint32_t group);
  virtual RachConfig GetRachConfig ();
  virtual AllocateNcRaPreambleReturnValue AllocateNcRaPreamble (uint16_t rnti);
  

private:
  cv2x_LteEnbMac* m_mac; ///< the MAC
};


cv2x_EnbMacMemberLteEnbCmacSapProvider::cv2x_EnbMacMemberLteEnbCmacSapProvider (cv2x_LteEnbMac* mac)
  : m_mac (mac)
{
}

void
cv2x_EnbMacMemberLteEnbCmacSapProvider::ConfigureMac (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  m_mac->DoConfigureMac (ulBandwidth, dlBandwidth);
}

void
cv2x_EnbMacMemberLteEnbCmacSapProvider::AddUe (uint16_t rnti)
{
  m_mac->DoAddUe (rnti);
}

void
cv2x_EnbMacMemberLteEnbCmacSapProvider::RemoveUe (uint16_t rnti)
{
  m_mac->DoRemoveUe (rnti);
}

void
cv2x_EnbMacMemberLteEnbCmacSapProvider::AddLc (LcInfo lcinfo, cv2x_LteMacSapUser* msu)
{
  m_mac->DoAddLc (lcinfo, msu);
}

void
cv2x_EnbMacMemberLteEnbCmacSapProvider::ReconfigureLc (LcInfo lcinfo)
{
  m_mac->DoReconfigureLc (lcinfo);
}

void
cv2x_EnbMacMemberLteEnbCmacSapProvider::ReleaseLc (uint16_t rnti, uint8_t lcid)
{
  m_mac->DoReleaseLc (rnti, lcid);
}

void
cv2x_EnbMacMemberLteEnbCmacSapProvider::UeUpdateConfigurationReq (UeConfig params)
{
  m_mac->DoUeUpdateConfigurationReq (params);
}

void
cv2x_EnbMacMemberLteEnbCmacSapProvider::AddPool (uint32_t group, Ptr<SidelinkCommResourcePool> pool)
{
  m_mac->DoAddPool (group, pool);
}

void
cv2x_EnbMacMemberLteEnbCmacSapProvider::RemovePool (uint32_t group)
{
  m_mac->DoRemovePool (group);
}

cv2x_LteEnbCmacSapProvider::RachConfig 
cv2x_EnbMacMemberLteEnbCmacSapProvider::GetRachConfig ()
{
  return m_mac->DoGetRachConfig ();
}
 
cv2x_LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue 
cv2x_EnbMacMemberLteEnbCmacSapProvider::AllocateNcRaPreamble (uint16_t rnti)
{
  return m_mac->DoAllocateNcRaPreamble (rnti);
}


/// cv2x_EnbMacMemberFfMacSchedSapUser class
class cv2x_EnbMacMemberFfMacSchedSapUser : public cv2x_FfMacSchedSapUser
{
public:
  /**
   * Constructor
   * 
   * \param mac the MAC
   */
  cv2x_EnbMacMemberFfMacSchedSapUser (cv2x_LteEnbMac* mac);


  virtual void SchedDlConfigInd (const struct SchedDlConfigIndParameters& params);
  virtual void SchedUlConfigInd (const struct SchedUlConfigIndParameters& params);
private:
  cv2x_LteEnbMac* m_mac; ///< the MAC
};


cv2x_EnbMacMemberFfMacSchedSapUser::cv2x_EnbMacMemberFfMacSchedSapUser (cv2x_LteEnbMac* mac)
  : m_mac (mac)
{
}


void
cv2x_EnbMacMemberFfMacSchedSapUser::SchedDlConfigInd (const struct SchedDlConfigIndParameters& params)
{
  m_mac->DoSchedDlConfigInd (params);
}



void
cv2x_EnbMacMemberFfMacSchedSapUser::SchedUlConfigInd (const struct SchedUlConfigIndParameters& params)
{
  m_mac->DoSchedUlConfigInd (params);
}


/// cv2x_EnbMacMemberFfMacCschedSapUser class
class cv2x_EnbMacMemberFfMacCschedSapUser : public cv2x_FfMacCschedSapUser
{
public:
  /**
   * Constructor
   *
   * \param mac the MAC
   */
  cv2x_EnbMacMemberFfMacCschedSapUser (cv2x_LteEnbMac* mac);

  virtual void CschedCellConfigCnf (const struct CschedCellConfigCnfParameters& params);
  virtual void CschedUeConfigCnf (const struct CschedUeConfigCnfParameters& params);
  virtual void CschedLcConfigCnf (const struct CschedLcConfigCnfParameters& params);
  virtual void CschedLcReleaseCnf (const struct CschedLcReleaseCnfParameters& params);
  virtual void CschedUeReleaseCnf (const struct CschedUeReleaseCnfParameters& params);
  virtual void CschedUeConfigUpdateInd (const struct CschedUeConfigUpdateIndParameters& params);
  virtual void CschedCellConfigUpdateInd (const struct CschedCellConfigUpdateIndParameters& params);

private:
  cv2x_LteEnbMac* m_mac; ///< the MAC
};


cv2x_EnbMacMemberFfMacCschedSapUser::cv2x_EnbMacMemberFfMacCschedSapUser (cv2x_LteEnbMac* mac)
  : m_mac (mac)
{
}

void
cv2x_EnbMacMemberFfMacCschedSapUser::CschedCellConfigCnf (const struct CschedCellConfigCnfParameters& params)
{
  m_mac->DoCschedCellConfigCnf (params);
}

void
cv2x_EnbMacMemberFfMacCschedSapUser::CschedUeConfigCnf (const struct CschedUeConfigCnfParameters& params)
{
  m_mac->DoCschedUeConfigCnf (params);
}

void
cv2x_EnbMacMemberFfMacCschedSapUser::CschedLcConfigCnf (const struct CschedLcConfigCnfParameters& params)
{
  m_mac->DoCschedLcConfigCnf (params);
}

void
cv2x_EnbMacMemberFfMacCschedSapUser::CschedLcReleaseCnf (const struct CschedLcReleaseCnfParameters& params)
{
  m_mac->DoCschedLcReleaseCnf (params);
}

void
cv2x_EnbMacMemberFfMacCschedSapUser::CschedUeReleaseCnf (const struct CschedUeReleaseCnfParameters& params)
{
  m_mac->DoCschedUeReleaseCnf (params);
}

void
cv2x_EnbMacMemberFfMacCschedSapUser::CschedUeConfigUpdateInd (const struct CschedUeConfigUpdateIndParameters& params)
{
  m_mac->DoCschedUeConfigUpdateInd (params);
}

void
cv2x_EnbMacMemberFfMacCschedSapUser::CschedCellConfigUpdateInd (const struct CschedCellConfigUpdateIndParameters& params)
{
  m_mac->DoCschedCellConfigUpdateInd (params);
}



/// ---------- PHY-SAP
class cv2x_EnbMacMemberLteEnbPhySapUser : public cv2x_LteEnbPhySapUser
{
public:
  /**
   * Constructor
   *
   * \param mac the MAC
   */
  cv2x_EnbMacMemberLteEnbPhySapUser (cv2x_LteEnbMac* mac);

  // inherited from cv2x_LteEnbPhySapUser
  virtual void ReceivePhyPdu (Ptr<Packet> p);
  virtual void SubframeIndication (uint32_t frameNo, uint32_t subframeNo);
  virtual void ReceiveLteControlMessage (Ptr<cv2x_LteControlMessage> msg);
  virtual void ReceiveRachPreamble (uint32_t prachId);
  virtual void UlCqiReport (cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi);
  virtual void UlInfoListElementHarqFeeback (cv2x_UlInfoListElement_s params);
  virtual void DlInfoListElementHarqFeeback (cv2x_DlInfoListElement_s params);

private:
  cv2x_LteEnbMac* m_mac; ///< the MAC
};

cv2x_EnbMacMemberLteEnbPhySapUser::cv2x_EnbMacMemberLteEnbPhySapUser (cv2x_LteEnbMac* mac) : m_mac (mac)
{
}


void
cv2x_EnbMacMemberLteEnbPhySapUser::ReceivePhyPdu (Ptr<Packet> p)
{
  m_mac->DoReceivePhyPdu (p);
}

void
cv2x_EnbMacMemberLteEnbPhySapUser::SubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  m_mac->DoSubframeIndication (frameNo, subframeNo);
}

void
cv2x_EnbMacMemberLteEnbPhySapUser::ReceiveLteControlMessage (Ptr<cv2x_LteControlMessage> msg)
{
  m_mac->DoReceiveLteControlMessage (msg);
}

void
cv2x_EnbMacMemberLteEnbPhySapUser::ReceiveRachPreamble (uint32_t prachId)
{
  m_mac->DoReceiveRachPreamble (prachId);
}

void
cv2x_EnbMacMemberLteEnbPhySapUser::UlCqiReport (cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi)
{
  m_mac->DoUlCqiReport (ulcqi);
}

void
cv2x_EnbMacMemberLteEnbPhySapUser::UlInfoListElementHarqFeeback (cv2x_UlInfoListElement_s params)
{
  m_mac->DoUlInfoListElementHarqFeeback (params);
}

void
cv2x_EnbMacMemberLteEnbPhySapUser::DlInfoListElementHarqFeeback (cv2x_DlInfoListElement_s params)
{
  m_mac->DoDlInfoListElementHarqFeeback (params);
}


// //////////////////////////////////////
// generic cv2x_LteEnbMac methods
// //////////////////////////////////////


TypeId
cv2x_LteEnbMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_LteEnbMac")
    .SetParent<Object> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_LteEnbMac> ()
    .AddAttribute ("NumberOfRaPreambles",
                   "how many random access preambles are available for the contention based RACH process",
                   UintegerValue (52),
                   MakeUintegerAccessor (&cv2x_LteEnbMac::m_numberOfRaPreambles),
                   MakeUintegerChecker<uint8_t> (4, 64))
    .AddAttribute ("PreambleTransMax",
                   "Maximum number of random access preamble transmissions",
                   UintegerValue (50),
                   MakeUintegerAccessor (&cv2x_LteEnbMac::m_preambleTransMax),
                   MakeUintegerChecker<uint8_t> (3, 200))
    .AddAttribute ("RaResponseWindowSize",
                   "length of the window (in TTIs) for the reception of the random access response (RAR); the resulting RAR timeout is this value + 3 ms",
                   UintegerValue (3),
                   MakeUintegerAccessor (&cv2x_LteEnbMac::m_raResponseWindowSize),
                   MakeUintegerChecker<uint8_t> (2, 10))
    .AddTraceSource ("DlScheduling",
                     "Information regarding DL scheduling.",
                     MakeTraceSourceAccessor (&cv2x_LteEnbMac::m_dlScheduling),
                     "ns3::cv2x_LteEnbMac::DlSchedulingTracedCallback")
    .AddTraceSource ("UlScheduling",
                     "Information regarding UL scheduling.",
                     MakeTraceSourceAccessor (&cv2x_LteEnbMac::m_ulScheduling),
                     "ns3::cv2x_LteEnbMac::UlSchedulingTracedCallback")
    .AddAttribute ("cv2x_ComponentCarrierId",
                   "cv2x_ComponentCarrier Id, needed to reply on the appropriate sap.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_LteEnbMac::m_componentCarrierId),
                   MakeUintegerChecker<uint8_t> (0,4))
  ;

  return tid;
}


cv2x_LteEnbMac::cv2x_LteEnbMac ():
m_ccmMacSapUser (0)
{
  NS_LOG_FUNCTION (this);
  m_macSapProvider = new cv2x_EnbMacMemberLteMacSapProvider<cv2x_LteEnbMac> (this);
  m_cmacSapProvider = new cv2x_EnbMacMemberLteEnbCmacSapProvider (this);
  m_schedSapUser = new cv2x_EnbMacMemberFfMacSchedSapUser (this);
  m_cschedSapUser = new cv2x_EnbMacMemberFfMacCschedSapUser (this);
  m_enbPhySapUser = new cv2x_EnbMacMemberLteEnbPhySapUser (this);
  m_ccmMacSapProvider = new cv2x_MemberLteCcmMacSapProvider<cv2x_LteEnbMac> (this);
}


cv2x_LteEnbMac::~cv2x_LteEnbMac ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteEnbMac::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_dlCqiReceived.clear ();
  m_ulCqiReceived.clear ();
  m_ulCeReceived.clear ();
  m_dlInfoListReceived.clear ();
  m_ulInfoListReceived.clear ();
  m_miDlHarqProcessesPackets.clear ();
  delete m_macSapProvider;
  delete m_cmacSapProvider;
  delete m_schedSapUser;
  delete m_cschedSapUser;
  delete m_enbPhySapUser;
  delete m_ccmMacSapProvider;
}

void
cv2x_LteEnbMac::SetComponentCarrierId (uint8_t index)
{
  m_componentCarrierId = index;
}

void
cv2x_LteEnbMac::SetFfMacSchedSapProvider (cv2x_FfMacSchedSapProvider* s)
{
  m_schedSapProvider = s;
}

cv2x_FfMacSchedSapUser*
cv2x_LteEnbMac::GetFfMacSchedSapUser (void)
{
  return m_schedSapUser;
}

void
cv2x_LteEnbMac::SetFfMacCschedSapProvider (cv2x_FfMacCschedSapProvider* s)
{
  m_cschedSapProvider = s;
}

cv2x_FfMacCschedSapUser*
cv2x_LteEnbMac::GetFfMacCschedSapUser (void)
{
  return m_cschedSapUser;
}



void
cv2x_LteEnbMac::SetLteMacSapUser (cv2x_LteMacSapUser* s)
{
  m_macSapUser = s;
}

cv2x_LteMacSapProvider*
cv2x_LteEnbMac::GetLteMacSapProvider (void)
{
  return m_macSapProvider;
}

void
cv2x_LteEnbMac::SetLteEnbCmacSapUser (cv2x_LteEnbCmacSapUser* s)
{
  m_cmacSapUser = s;
}

cv2x_LteEnbCmacSapProvider*
cv2x_LteEnbMac::GetLteEnbCmacSapProvider (void)
{
  return m_cmacSapProvider;
}

void
cv2x_LteEnbMac::SetLteEnbPhySapProvider (cv2x_LteEnbPhySapProvider* s)
{
  m_enbPhySapProvider = s;
}


cv2x_LteEnbPhySapUser*
cv2x_LteEnbMac::GetLteEnbPhySapUser ()
{
  return m_enbPhySapUser;
}

void
cv2x_LteEnbMac::SetLteCcmMacSapUser (cv2x_LteCcmMacSapUser* s)
{
  m_ccmMacSapUser = s;
}


cv2x_LteCcmMacSapProvider*
cv2x_LteEnbMac::GetLteCcmMacSapProvider ()
{
  return m_ccmMacSapProvider;
}

void
cv2x_LteEnbMac::DoSubframeIndication (uint32_t frameNo, uint32_t subframeNo)
{
  NS_LOG_FUNCTION (this << " EnbMac - frame " << frameNo << " subframe " << subframeNo);

  // Store current frame / subframe number
  m_frameNo = frameNo;
  m_subframeNo = subframeNo;


  // --- DOWNLINK ---
  // Send Dl-CQI info to the scheduler
  if (m_dlCqiReceived.size () > 0)
    {
      cv2x_FfMacSchedSapProvider::SchedDlCqiInfoReqParameters dlcqiInfoReq;
      dlcqiInfoReq.m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & subframeNo);
      dlcqiInfoReq.m_cqiList.insert (dlcqiInfoReq.m_cqiList.begin (), m_dlCqiReceived.begin (), m_dlCqiReceived.end ());
      m_dlCqiReceived.erase (m_dlCqiReceived.begin (), m_dlCqiReceived.end ());
      m_schedSapProvider->SchedDlCqiInfoReq (dlcqiInfoReq);
    }

  if (!m_receivedRachPreambleCount.empty ())
    {
      // process received RACH preambles and notify the scheduler
      cv2x_FfMacSchedSapProvider::SchedDlRachInfoReqParameters rachInfoReqParams;
      NS_ASSERT (subframeNo > 0 && subframeNo <= 10); // subframe in 1..10
      for (std::map<uint8_t, uint32_t>::const_iterator it = m_receivedRachPreambleCount.begin ();
           it != m_receivedRachPreambleCount.end ();
           ++it)
        {
          NS_LOG_INFO (this << " preambleId " << (uint32_t) it->first << ": " << it->second << " received");
          NS_ASSERT (it->second != 0);
          if (it->second > 1)
            {
              NS_LOG_INFO ("preambleId " << (uint32_t) it->first << ": collision");
              // in case of collision we assume that no preamble is
              // successfully received, hence no RAR is sent 
            }
          else
            {
              uint16_t rnti;
              std::map<uint8_t, NcRaPreambleInfo>::iterator jt = m_allocatedNcRaPreambleMap.find (it->first);
              if (jt != m_allocatedNcRaPreambleMap.end ())
                {
                  rnti = jt->second.rnti;
                  NS_LOG_INFO ("preambleId previously allocated for NC based RA, RNTI =" << (uint32_t) rnti << ", sending RAR");
                  
                }
              else
                {
                  rnti = m_cmacSapUser->AllocateTemporaryCellRnti ();
                  NS_LOG_INFO ("preambleId " << (uint32_t) it->first << ": allocated T-C-RNTI " << (uint32_t) rnti << ", sending RAR");
                }

              cv2x_RachListElement_s rachLe;
              rachLe.m_rnti = rnti;
              rachLe.m_estimatedSize = 144; // to be confirmed
              rachInfoReqParams.m_rachList.push_back (rachLe);
              m_rapIdRntiMap.insert (std::pair <uint16_t, uint32_t> (rnti, it->first));
            }
        }
      m_schedSapProvider->SchedDlRachInfoReq (rachInfoReqParams);
      m_receivedRachPreambleCount.clear ();
    }
  // Get downlink transmission opportunities
  uint32_t dlSchedFrameNo = m_frameNo;
  uint32_t dlSchedSubframeNo = m_subframeNo;
  //   NS_LOG_DEBUG (this << " sfn " << frameNo << " sbfn " << subframeNo);
  if (dlSchedSubframeNo + m_macChTtiDelay > 10)
    {
      dlSchedFrameNo++;
      dlSchedSubframeNo = (dlSchedSubframeNo + m_macChTtiDelay) % 10;
    }
  else
    {
      dlSchedSubframeNo = dlSchedSubframeNo + m_macChTtiDelay;
    }
  cv2x_FfMacSchedSapProvider::SchedDlTriggerReqParameters dlparams;
  dlparams.m_sfnSf = ((0x3FF & dlSchedFrameNo) << 4) | (0xF & dlSchedSubframeNo);

  // Forward DL HARQ feebacks collected during last TTI
  if (m_dlInfoListReceived.size () > 0)
    {
      dlparams.m_dlInfoList = m_dlInfoListReceived;
      // empty local buffer
      m_dlInfoListReceived.clear ();
    }

  m_schedSapProvider->SchedDlTriggerReq (dlparams);


  // --- UPLINK ---
  // Send UL-CQI info to the scheduler
  for (uint16_t i = 0; i < m_ulCqiReceived.size (); i++)
    {
      if (subframeNo > 1)
        {        
          m_ulCqiReceived.at (i).m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & (subframeNo - 1));
        }
      else
        {
          m_ulCqiReceived.at (i).m_sfnSf = ((0x3FF & (frameNo - 1)) << 4) | (0xF & 10);
        }
      m_schedSapProvider->SchedUlCqiInfoReq (m_ulCqiReceived.at (i));
    }
    m_ulCqiReceived.clear ();
  
  // Send BSR reports to the scheduler
  if (m_ulCeReceived.size () > 0)
    {
      cv2x_FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters ulMacReq;
      ulMacReq.m_sfnSf = ((0x3FF & frameNo) << 4) | (0xF & subframeNo);
      ulMacReq.m_macCeList.insert (ulMacReq.m_macCeList.begin (), m_ulCeReceived.begin (), m_ulCeReceived.end ());
      m_ulCeReceived.erase (m_ulCeReceived.begin (), m_ulCeReceived.end ());
      m_schedSapProvider->SchedUlMacCtrlInfoReq (ulMacReq);
    }


  // Get uplink transmission opportunities
  uint32_t ulSchedFrameNo = m_frameNo;
  uint32_t ulSchedSubframeNo = m_subframeNo;
  //   NS_LOG_DEBUG (this << " sfn " << frameNo << " sbfn " << subframeNo);
  if (ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY) > 10)
    {
      ulSchedFrameNo++;
      ulSchedSubframeNo = (ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY)) % 10;
    }
  else
    {
      ulSchedSubframeNo = ulSchedSubframeNo + (m_macChTtiDelay + UL_PUSCH_TTIS_DELAY);
    }
  cv2x_FfMacSchedSapProvider::SchedUlTriggerReqParameters ulparams;
  ulparams.m_sfnSf = ((0x3FF & ulSchedFrameNo) << 4) | (0xF & ulSchedSubframeNo);

  // Forward DL HARQ feebacks collected during last TTI
  if (m_ulInfoListReceived.size () > 0)
    {
     ulparams.m_ulInfoList = m_ulInfoListReceived;
      // empty local buffer
      m_ulInfoListReceived.clear ();
    }

  m_schedSapProvider->SchedUlTriggerReq (ulparams);

}


void
cv2x_LteEnbMac::DoReceiveLteControlMessage  (Ptr<cv2x_LteControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);
  if (msg->GetMessageType () == cv2x_LteControlMessage::DL_CQI)
    {
      Ptr<cv2x_DlCqiLteControlMessage> dlcqi = DynamicCast<cv2x_DlCqiLteControlMessage> (msg);
      ReceiveDlCqiLteControlMessage (dlcqi);
    }
  else if (msg->GetMessageType () == cv2x_LteControlMessage::BSR)
    {
      Ptr<cv2x_BsrLteControlMessage> bsr = DynamicCast<cv2x_BsrLteControlMessage> (msg);
      ReceiveBsrMessage (bsr->GetBsr ());
    }
  else if (msg->GetMessageType () == cv2x_LteControlMessage::DL_HARQ)
    {
      Ptr<cv2x_DlHarqFeedbackLteControlMessage> dlharq = DynamicCast<cv2x_DlHarqFeedbackLteControlMessage> (msg);
      DoDlInfoListElementHarqFeeback (dlharq->GetDlHarqFeedback ());
    }
  else
    {
      NS_LOG_LOGIC (this << " cv2x_LteControlMessage type " << msg->GetMessageType () << " not recognized");
    }
}

void
cv2x_LteEnbMac::DoReceiveRachPreamble  (uint8_t rapId)
{
  NS_LOG_FUNCTION (this << (uint32_t) rapId);
  // just record that the preamble has been received; it will be processed later
  ++m_receivedRachPreambleCount[rapId]; // will create entry if not exists
}

void
cv2x_LteEnbMac::DoUlCqiReport (cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters ulcqi)
{ 
  if (ulcqi.m_ulCqi.m_type == cv2x_UlCqi_s::PUSCH)
    {
      NS_LOG_DEBUG (this << " eNB rxed an PUSCH UL-CQI");
    }
  else if (ulcqi.m_ulCqi.m_type == cv2x_UlCqi_s::SRS)
    {
      NS_LOG_DEBUG (this << " eNB rxed an SRS UL-CQI");
    }
  m_ulCqiReceived.push_back (ulcqi);
}


void
cv2x_LteEnbMac::ReceiveDlCqiLteControlMessage  (Ptr<cv2x_DlCqiLteControlMessage> msg)
{
  NS_LOG_FUNCTION (this << msg);

  cv2x_CqiListElement_s dlcqi = msg->GetDlCqi ();
  NS_LOG_LOGIC (this << "Enb Received DL-CQI rnti" << dlcqi.m_rnti);
  NS_ASSERT (dlcqi.m_rnti != 0);
  m_dlCqiReceived.push_back (dlcqi);

}


void
cv2x_LteEnbMac::ReceiveBsrMessage  (cv2x_MacCeListElement_s bsr)
{
  NS_LOG_FUNCTION (this);
  m_ccmMacSapUser->UlReceiveMacCe (bsr, m_componentCarrierId);
}

void
cv2x_LteEnbMac::DoReportMacCeToScheduler (cv2x_MacCeListElement_s bsr)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG (this << " bsr Size " << (uint16_t) m_ulCeReceived.size ());
  //send to cv2x_LteCcmMacSapUser
  m_ulCeReceived.push_back (bsr); // this to called when LteUlCcmSapProvider::ReportMacCeToScheduler is called
  NS_LOG_DEBUG (this << " bsr Size after push_back " << (uint16_t) m_ulCeReceived.size ());
}


void
cv2x_LteEnbMac::DoReceivePhyPdu (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this);
  cv2x_LteRadioBearerTag tag;
  p->RemovePacketTag (tag);

  // store info of the packet received

//   std::map <uint16_t,cv2x_UlInfoListElement_s>::iterator it;
//   u_int rnti = tag.GetRnti ();
//  u_int lcid = tag.GetLcid ();
//   it = m_ulInfoListElements.find (tag.GetRnti ());
//   if (it == m_ulInfoListElements.end ())
//     {
//       // new RNTI
//       cv2x_UlInfoListElement_s ulinfonew;
//       ulinfonew.m_rnti = tag.GetRnti ();
//       // always allocate full size of ulReception vector, initializing all elements to 0
//       ulinfonew.m_ulReception.assign (MAX_LC_LIST+1, 0);
//       // set the element for the current LCID
//       ulinfonew.m_ulReception.at (tag.GetLcid ()) = p->GetSize ();
//       ulinfonew.m_receptionStatus = cv2x_UlInfoListElement_s::Ok;
//       ulinfonew.m_tpc = 0; // Tx power control not implemented at this stage
//       m_ulInfoListElements.insert (std::pair<uint16_t, cv2x_UlInfoListElement_s > (tag.GetRnti (), ulinfonew));
// 
//     }
//   else
//     {
//       // existing RNTI: we just set the value for the current
//       // LCID. Note that the corresponding element had already been
//       // allocated previously.
//       NS_ASSERT_MSG ((*it).second.m_ulReception.at (tag.GetLcid ()) == 0, "would overwrite previously written ulReception element");
//       (*it).second.m_ulReception.at (tag.GetLcid ()) = p->GetSize ();
//       (*it).second.m_receptionStatus = cv2x_UlInfoListElement_s::Ok;
//     }



  // forward the packet to the correspondent RLC
  uint16_t rnti = tag.GetRnti ();
  uint8_t lcid = tag.GetLcid ();
  std::map <uint16_t, std::map<uint8_t, cv2x_LteMacSapUser*> >::iterator rntiIt = m_rlcAttached.find (rnti);
  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "could not find RNTI" << rnti);
  std::map<uint8_t, cv2x_LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (lcid);
  //NS_ASSERT_MSG (lcidIt != rntiIt->second.end (), "could not find LCID" << lcid);

  //Receive PDU only if LCID is found
  if (lcidIt != rntiIt->second.end ())
    {
      (*lcidIt).second->ReceivePdu (p, rnti, lcid);
    }
}



// ////////////////////////////////////////////
// CMAC SAP
// ////////////////////////////////////////////

void
cv2x_LteEnbMac::DoConfigureMac (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  NS_LOG_FUNCTION (this << " ulBandwidth=" << (uint16_t) ulBandwidth << " dlBandwidth=" << (uint16_t) dlBandwidth);
  cv2x_FfMacCschedSapProvider::CschedCellConfigReqParameters params;
  // Configure the subset of parameters used by cv2x_FfMacScheduler
  params.m_ulBandwidth = ulBandwidth;
  params.m_dlBandwidth = dlBandwidth;
  m_macChTtiDelay = m_enbPhySapProvider->GetMacChTtiDelay ();
  // ...more parameters can be configured
  m_cschedSapProvider->CschedCellConfigReq (params);
}


void
cv2x_LteEnbMac::DoAddUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << " rnti=" << rnti);
  std::map<uint8_t, cv2x_LteMacSapUser*> empty;
  std::pair <std::map <uint16_t, std::map<uint8_t, cv2x_LteMacSapUser*> >::iterator, bool> 
    ret = m_rlcAttached.insert (std::pair <uint16_t,  std::map<uint8_t, cv2x_LteMacSapUser*> > 
                                (rnti, empty));
  NS_ASSERT_MSG (ret.second, "element already present, RNTI already existed");

  cv2x_FfMacCschedSapProvider::CschedUeConfigReqParameters params;
  params.m_rnti = rnti;
  params.m_transmissionMode = 0; // set to default value (SISO) for avoiding random initialization (valgrind error)

  m_cschedSapProvider->CschedUeConfigReq (params);

  // Create DL transmission HARQ buffers
  std::vector < Ptr<PacketBurst> > dlHarqLayer0pkt;
  dlHarqLayer0pkt.resize (8);
  for (uint8_t i = 0; i < 8; i++)
    {
      Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
      dlHarqLayer0pkt.at (i) = pb;
    }
  std::vector < Ptr<PacketBurst> > dlHarqLayer1pkt;
  dlHarqLayer1pkt.resize (8);
  for (uint8_t i = 0; i < 8; i++)
    {
      Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
      dlHarqLayer1pkt.at (i) = pb;
    }
  DlHarqProcessesBuffer_t buf;
  buf.push_back (dlHarqLayer0pkt);
  buf.push_back (dlHarqLayer1pkt);
  m_miDlHarqProcessesPackets.insert (std::pair <uint16_t, DlHarqProcessesBuffer_t> (rnti, buf));
}

void
cv2x_LteEnbMac::DoRemoveUe (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << " rnti=" << rnti);
  cv2x_FfMacCschedSapProvider::CschedUeReleaseReqParameters params;
  params.m_rnti = rnti;
  m_cschedSapProvider->CschedUeReleaseReq (params);
  m_rlcAttached.erase (rnti);
  m_miDlHarqProcessesPackets.erase (rnti);
}

void
cv2x_LteEnbMac::DoAddLc (cv2x_LteEnbCmacSapProvider::LcInfo lcinfo, cv2x_LteMacSapUser* msu)
{
  NS_LOG_FUNCTION (this << lcinfo.rnti << (uint16_t) lcinfo.lcId);

  std::map <cv2x_LteFlowId_t, cv2x_LteMacSapUser* >::iterator it;
  
  cv2x_LteFlowId_t flow (lcinfo.rnti, lcinfo.lcId);
  
  std::map <uint16_t, std::map<uint8_t, cv2x_LteMacSapUser*> >::iterator rntiIt = m_rlcAttached.find (lcinfo.rnti);
  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "RNTI not found");
  std::map<uint8_t, cv2x_LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (lcinfo.lcId);
  if (lcidIt == rntiIt->second.end ())
    {
      rntiIt->second.insert (std::pair<uint8_t, cv2x_LteMacSapUser*> (lcinfo.lcId, msu));
    }
  else
    {
      NS_LOG_ERROR ("LC already exists");
    }

  // CCCH (LCID 0) is pre-configured 
  // see FF LTE MAC Scheduler
  // Interface Specification v1.11, 
  // 4.3.4 logicalChannelConfigListElement
  if (lcinfo.lcId != 0)
    {
      struct cv2x_FfMacCschedSapProvider::CschedLcConfigReqParameters params;
      params.m_rnti = lcinfo.rnti;
      params.m_reconfigureFlag = false;

      struct cv2x_LogicalChannelConfigListElement_s lccle;
      lccle.m_logicalChannelIdentity = lcinfo.lcId;
      lccle.m_logicalChannelGroup = lcinfo.lcGroup;
      lccle.m_direction = cv2x_LogicalChannelConfigListElement_s::DIR_BOTH;
      lccle.m_qosBearerType = lcinfo.isGbr ? cv2x_LogicalChannelConfigListElement_s::QBT_GBR : cv2x_LogicalChannelConfigListElement_s::QBT_NON_GBR;
      lccle.m_qci = lcinfo.qci;
      lccle.m_eRabMaximulBitrateUl = lcinfo.mbrUl;
      lccle.m_eRabMaximulBitrateDl = lcinfo.mbrDl;
      lccle.m_eRabGuaranteedBitrateUl = lcinfo.gbrUl;
      lccle.m_eRabGuaranteedBitrateDl = lcinfo.gbrDl;
      params.m_logicalChannelConfigList.push_back (lccle);

      m_cschedSapProvider->CschedLcConfigReq (params);
    }
}

void
cv2x_LteEnbMac::DoReconfigureLc (cv2x_LteEnbCmacSapProvider::LcInfo lcinfo)
{
  NS_FATAL_ERROR ("not implemented");
}

void
cv2x_LteEnbMac::DoReleaseLc (uint16_t rnti, uint8_t lcid)
{
  NS_LOG_FUNCTION (this);

  //Find user based on rnti and then erase lcid stored against the same
  std::map <uint16_t, std::map<uint8_t, cv2x_LteMacSapUser*> >::iterator rntiIt = m_rlcAttached.find (rnti);
  rntiIt->second.erase (lcid);

  struct cv2x_FfMacCschedSapProvider::CschedLcReleaseReqParameters params;
  params.m_rnti = rnti;
  params.m_logicalChannelIdentity.push_back (lcid);
  m_cschedSapProvider->CschedLcReleaseReq (params);
}

void
cv2x_LteEnbMac::DoUeUpdateConfigurationReq (cv2x_LteEnbCmacSapProvider::UeConfig params)
{
  NS_LOG_FUNCTION (this);

  // propagates to scheduler
  cv2x_FfMacCschedSapProvider::CschedUeConfigReqParameters req;
  req.m_rnti = params.m_rnti;
  req.m_transmissionMode = params.m_transmissionMode;
  req.m_slDestinations = params.m_slDestinations;
  NS_LOG_DEBUG ("sidelink: adding " << params.m_slDestinations.size () << " destinations for UE with RNTI " << params.m_rnti);
  req.m_reconfigureFlag = true;
  m_cschedSapProvider->CschedUeConfigReq (req);
}

void
cv2x_LteEnbMac::DoAddPool (uint32_t group, Ptr<SidelinkCommResourcePool> pool)
{
  NS_LOG_FUNCTION (this);

  // propagates to scheduler
  cv2x_FfMacCschedSapProvider::CschedPoolConfigReqParameters req;
  req.m_group = group;
  req.m_pool = pool;
  m_cschedSapProvider->CschedPoolConfigReq (req);
}

void
cv2x_LteEnbMac::DoRemovePool (uint32_t group)
{
  NS_LOG_FUNCTION (this);

  // propagates to scheduler
  cv2x_FfMacCschedSapProvider::CschedPoolReleaseReqParameters req;
  req.m_group = group;
  m_cschedSapProvider->CschedPoolReleaseReq (req);
}

cv2x_LteEnbCmacSapProvider::RachConfig 
cv2x_LteEnbMac::DoGetRachConfig ()
{
  struct cv2x_LteEnbCmacSapProvider::RachConfig rc;
  rc.numberOfRaPreambles = m_numberOfRaPreambles;
  rc.preambleTransMax = m_preambleTransMax;
  rc.raResponseWindowSize = m_raResponseWindowSize;
  return rc;
}
 
cv2x_LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue 
cv2x_LteEnbMac::DoAllocateNcRaPreamble (uint16_t rnti)
{
  bool found = false;
  uint8_t preambleId;
  for (preambleId = m_numberOfRaPreambles; preambleId < 64; ++preambleId)
    {
      std::map<uint8_t, NcRaPreambleInfo>::iterator it = m_allocatedNcRaPreambleMap.find (preambleId);
      if ((it ==  m_allocatedNcRaPreambleMap.end ())
          || (it->second.expiryTime < Simulator::Now ()))
        {
          found = true;
          NcRaPreambleInfo preambleInfo;
          uint32_t expiryIntervalMs = (uint32_t) m_preambleTransMax * ((uint32_t) m_raResponseWindowSize + 5); 
          
          preambleInfo.expiryTime = Simulator::Now () + MilliSeconds (expiryIntervalMs);
          preambleInfo.rnti = rnti;
          NS_LOG_INFO ("allocated preamble for NC based RA: preamble " << preambleId << ", RNTI " << preambleInfo.rnti << ", exiryTime " << preambleInfo.expiryTime);
          m_allocatedNcRaPreambleMap[preambleId] = preambleInfo; // create if not exist, update otherwise
          break;
        }
    }
  cv2x_LteEnbCmacSapProvider::AllocateNcRaPreambleReturnValue ret;
  if (found)
    {
      ret.valid = true;
      ret.raPreambleId = preambleId;
      ret.raPrachMaskIndex = 0;
    }
  else
    {
      ret.valid = false;
      ret.raPreambleId = 0;
      ret.raPrachMaskIndex = 0;
    }
  return ret;
}



// ////////////////////////////////////////////
// MAC SAP
// ////////////////////////////////////////////


void
cv2x_LteEnbMac::DoTransmitPdu (cv2x_LteMacSapProvider::TransmitPduParameters params)
{
  NS_LOG_FUNCTION (this);
  cv2x_LteRadioBearerTag tag (params.rnti, params.lcid, params.layer);
  params.pdu->AddPacketTag (tag);
  params.componentCarrierId = m_componentCarrierId;
  // Store pkt in HARQ buffer
  std::map <uint16_t, DlHarqProcessesBuffer_t>::iterator it =  m_miDlHarqProcessesPackets.find (params.rnti);
  NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
  NS_LOG_DEBUG (this << " LAYER " << (uint16_t)tag.GetLayer () << " HARQ ID " << (uint16_t)params.harqProcessId);
  
  //(*it).second.at (params.layer).at (params.harqProcessId) = params.pdu;//->Copy ();
  (*it).second.at (params.layer).at (params.harqProcessId)->AddPacket (params.pdu);
  m_enbPhySapProvider->SendMacPdu (params.pdu);
}

void
cv2x_LteEnbMac::DoReportBufferStatus (cv2x_LteMacSapProvider::ReportBufferStatusParameters params)
{
  NS_LOG_FUNCTION (this);
  cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters req;
  req.m_rnti = params.rnti;
  req.m_logicalChannelIdentity = params.lcid;
  req.m_rlcTransmissionQueueSize = params.txQueueSize;
  req.m_rlcTransmissionQueueHolDelay = params.txQueueHolDelay;
  req.m_rlcRetransmissionQueueSize = params.retxQueueSize;
  req.m_rlcRetransmissionHolDelay = params.retxQueueHolDelay;
  req.m_rlcStatusPduSize = params.statusPduSize;
  m_schedSapProvider->SchedDlRlcBufferReq (req);
}



// ////////////////////////////////////////////
// SCHED SAP
// ////////////////////////////////////////////



void
cv2x_LteEnbMac::DoSchedDlConfigInd (cv2x_FfMacSchedSapUser::SchedDlConfigIndParameters ind)
{
  NS_LOG_FUNCTION (this);
  // Create DL PHY PDU
  Ptr<PacketBurst> pb = CreateObject<PacketBurst> ();
  std::map <cv2x_LteFlowId_t, cv2x_LteMacSapUser* >::iterator it;

  for (unsigned int i = 0; i < ind.m_buildDataList.size (); i++)
    {
      for (uint16_t layer = 0; layer < ind.m_buildDataList.at (i).m_dci.m_ndi.size (); layer++)
        {
          if (ind.m_buildDataList.at (i).m_dci.m_ndi.at (layer) == 1)
            {
              // new data -> force emptying correspondent harq pkt buffer
              std::map <uint16_t, DlHarqProcessesBuffer_t>::iterator it = m_miDlHarqProcessesPackets.find (ind.m_buildDataList.at (i).m_rnti);
              NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
              for (uint16_t lcId = 0; lcId < (*it).second.size (); lcId++)
                {
                  Ptr<PacketBurst> pb = CreateObject <PacketBurst> ();
                  (*it).second.at (lcId).at (ind.m_buildDataList.at (i).m_dci.m_harqProcess) = pb;
                }
            }
        }
      for (unsigned int j = 0; j < ind.m_buildDataList.at (i).m_rlcPduList.size (); j++)
        {
          for (uint16_t k = 0; k < ind.m_buildDataList.at (i).m_rlcPduList.at (j).size (); k++)
            {
              if (ind.m_buildDataList.at (i).m_dci.m_ndi.at (k) == 1)
                {
                  // New Data -> retrieve it from RLC
                  uint16_t rnti = ind.m_buildDataList.at (i).m_rnti;
                  uint8_t lcid = ind.m_buildDataList.at (i).m_rlcPduList.at (j).at (k).m_logicalChannelIdentity;
                  std::map <uint16_t, std::map<uint8_t, cv2x_LteMacSapUser*> >::iterator rntiIt = m_rlcAttached.find (rnti);
                  NS_ASSERT_MSG (rntiIt != m_rlcAttached.end (), "could not find RNTI" << rnti);
                  std::map<uint8_t, cv2x_LteMacSapUser*>::iterator lcidIt = rntiIt->second.find (lcid);
                  NS_ASSERT_MSG (lcidIt != rntiIt->second.end (), "could not find LCID" << (uint32_t)lcid<<" carrier id:"<<(uint16_t)m_componentCarrierId);
                  NS_LOG_DEBUG (this << " rnti= " << rnti << " lcid= " << (uint32_t) lcid << " layer= " << k);
                  (*lcidIt).second->NotifyTxOpportunity (ind.m_buildDataList.at (i).m_rlcPduList.at (j).at (k).m_size, k, ind.m_buildDataList.at (i).m_dci.m_harqProcess, m_componentCarrierId, rnti, lcid);
                }
              else
                {
                  if (ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (k) > 0)
                    {
                      // HARQ retransmission -> retrieve TB from HARQ buffer
                      std::map <uint16_t, DlHarqProcessesBuffer_t>::iterator it = m_miDlHarqProcessesPackets.find (ind.m_buildDataList.at (i).m_rnti);
                      NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
                      Ptr<PacketBurst> pb = (*it).second.at (k).at ( ind.m_buildDataList.at (i).m_dci.m_harqProcess);
                      for (std::list<Ptr<Packet> >::const_iterator j = pb->Begin (); j != pb->End (); ++j)
                        {
                          Ptr<Packet> pkt = (*j)->Copy ();
                          m_enbPhySapProvider->SendMacPdu (pkt);
                        }
                    }
                }
            }
        }
      // send the relative DCI
      Ptr<cv2x_DlDciLteControlMessage> msg = Create<cv2x_DlDciLteControlMessage> ();
      msg->SetDci (ind.m_buildDataList.at (i).m_dci);
      m_enbPhySapProvider->SendLteControlMessage (msg);
    }

  // Fire the trace with the DL information
  for (  uint32_t i  = 0; i < ind.m_buildDataList.size (); i++ )
    {
      // Only one TB used
      if (ind.m_buildDataList.at (i).m_dci.m_tbsSize.size () == 1)
        {
          cv2x_DlSchedulingCallbackInfo dlSchedulingCallbackInfo;
          dlSchedulingCallbackInfo.frameNo = m_frameNo;
          dlSchedulingCallbackInfo.subframeNo = m_subframeNo;
          dlSchedulingCallbackInfo.rnti = ind.m_buildDataList.at (i).m_dci.m_rnti;
          dlSchedulingCallbackInfo.mcsTb1=ind.m_buildDataList.at (i).m_dci.m_mcs.at (0);
          dlSchedulingCallbackInfo.sizeTb1 = ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (0);
          dlSchedulingCallbackInfo.mcsTb2 = 0;
          dlSchedulingCallbackInfo.sizeTb2 = 0;
          dlSchedulingCallbackInfo.componentCarrierId = m_componentCarrierId;
          m_dlScheduling(dlSchedulingCallbackInfo);
        }
      // Two TBs used
      else if (ind.m_buildDataList.at (i).m_dci.m_tbsSize.size () == 2)
        {
          cv2x_DlSchedulingCallbackInfo dlSchedulingCallbackInfo;
          dlSchedulingCallbackInfo.frameNo = m_frameNo;
          dlSchedulingCallbackInfo.subframeNo = m_subframeNo;
          dlSchedulingCallbackInfo.rnti = ind.m_buildDataList.at (i).m_dci.m_rnti;
          dlSchedulingCallbackInfo.mcsTb1=ind.m_buildDataList.at (i).m_dci.m_mcs.at (0);
          dlSchedulingCallbackInfo.sizeTb1 = ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (0);
          dlSchedulingCallbackInfo.mcsTb2 = ind.m_buildDataList.at (i).m_dci.m_mcs.at (1);
          dlSchedulingCallbackInfo.sizeTb2 = ind.m_buildDataList.at (i).m_dci.m_tbsSize.at (1);
          dlSchedulingCallbackInfo.componentCarrierId = m_componentCarrierId;
          m_dlScheduling(dlSchedulingCallbackInfo);
        }
      else
        {
          NS_FATAL_ERROR ("Found element with more than two transport blocks");
        }
    }

  // Random Access procedure: send RARs
  Ptr<cv2x_RarLteControlMessage> rarMsg = Create<cv2x_RarLteControlMessage> ();
  // see TS 36.321 5.1.4;  preambles were sent two frames ago
  // (plus 3GPP counts subframes from 0, not 1)
  uint16_t raRnti;
  if (m_subframeNo < 3)
    {
      raRnti = m_subframeNo + 7; // equivalent to +10-3
    }
  else
    {
      raRnti = m_subframeNo - 3;
    }
  rarMsg->SetRaRnti (raRnti);
  for (unsigned int i = 0; i < ind.m_buildRarList.size (); i++)
    {
      std::map <uint8_t, uint32_t>::iterator itRapId = m_rapIdRntiMap.find (ind.m_buildRarList.at (i).m_rnti);
      if (itRapId == m_rapIdRntiMap.end ())
        {
          NS_FATAL_ERROR ("Unable to find rapId of RNTI " << ind.m_buildRarList.at (i).m_rnti);
        }
      cv2x_RarLteControlMessage::Rar rar;
      rar.rapId = itRapId->second;
      rar.rarPayload = ind.m_buildRarList.at (i);
      rarMsg->AddRar (rar);
      NS_LOG_INFO (this << " Send RAR message to RNTI " << ind.m_buildRarList.at (i).m_rnti << " rapId " << itRapId->second);
    }
  if (ind.m_buildRarList.size () > 0)
    {
      m_enbPhySapProvider->SendLteControlMessage (rarMsg);
    }
  m_rapIdRntiMap.clear ();
}


void
cv2x_LteEnbMac::DoSchedUlConfigInd (cv2x_FfMacSchedSapUser::SchedUlConfigIndParameters ind)
{
  NS_LOG_FUNCTION (this);

  for (unsigned int i = 0; i < ind.m_dciList.size (); i++)
    {
      // send the correspondent ul dci
      Ptr<cv2x_UlDciLteControlMessage> msg = Create<cv2x_UlDciLteControlMessage> ();
      msg->SetDci (ind.m_dciList.at (i));
      m_enbPhySapProvider->SendLteControlMessage (msg);
    }

  // Fire the trace with the UL information
  for (  uint32_t i  = 0; i < ind.m_dciList.size (); i++ )
    {
      m_ulScheduling (m_frameNo, m_subframeNo, ind.m_dciList.at (i).m_rnti,
                      ind.m_dciList.at (i).m_mcs, ind.m_dciList.at (i).m_tbSize, m_componentCarrierId);
    }

  if (ind.m_sldciList.size () > 0)
    {
      NS_LOG_DEBUG ("Sending " << ind.m_sldciList.size () << " SL_DCI messages");
    }
  for (unsigned int i = 0; i < ind.m_sldciList.size (); i++)
    {
      NS_LOG_DEBUG ("i=" << i << " rnti=" << (uint32_t) (ind.m_sldciList.at (i).m_rnti));
      // send the correspondent sl dci
      Ptr<cv2x_SlDciLteControlMessage> msg = Create<cv2x_SlDciLteControlMessage> ();
      msg->SetDci (ind.m_sldciList.at (i));
      m_enbPhySapProvider->SendLteControlMessage (msg);
    }


}




// ////////////////////////////////////////////
// CSCHED SAP
// ////////////////////////////////////////////


void
cv2x_LteEnbMac::DoCschedCellConfigCnf (cv2x_FfMacCschedSapUser::CschedCellConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteEnbMac::DoCschedUeConfigCnf (cv2x_FfMacCschedSapUser::CschedUeConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteEnbMac::DoCschedLcConfigCnf (cv2x_FfMacCschedSapUser::CschedLcConfigCnfParameters params)
{
  NS_LOG_FUNCTION (this);
  // Call the CSCHED primitive
  // m_cschedSap->LcConfigCompleted();
}

void
cv2x_LteEnbMac::DoCschedLcReleaseCnf (cv2x_FfMacCschedSapUser::CschedLcReleaseCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteEnbMac::DoCschedUeReleaseCnf (cv2x_FfMacCschedSapUser::CschedUeReleaseCnfParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteEnbMac::DoCschedUeConfigUpdateInd (cv2x_FfMacCschedSapUser::CschedUeConfigUpdateIndParameters params)
{
  NS_LOG_FUNCTION (this);
  // propagates to RRC
  cv2x_LteEnbCmacSapUser::UeConfig ueConfigUpdate;
  ueConfigUpdate.m_rnti = params.m_rnti;
  ueConfigUpdate.m_transmissionMode = params.m_transmissionMode;
  m_cmacSapUser->RrcConfigurationUpdateInd (ueConfigUpdate);
}

void
cv2x_LteEnbMac::DoCschedCellConfigUpdateInd (cv2x_FfMacCschedSapUser::CschedCellConfigUpdateIndParameters params)
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_LteEnbMac::DoUlInfoListElementHarqFeeback (cv2x_UlInfoListElement_s params)
{
  NS_LOG_FUNCTION (this);
  m_ulInfoListReceived.push_back (params);
}

void
cv2x_LteEnbMac::DoDlInfoListElementHarqFeeback (cv2x_DlInfoListElement_s params)
{
  NS_LOG_FUNCTION (this);
  // Update HARQ buffer
  std::map <uint16_t, DlHarqProcessesBuffer_t>::iterator it =  m_miDlHarqProcessesPackets.find (params.m_rnti);
  NS_ASSERT (it != m_miDlHarqProcessesPackets.end ());
  for (uint8_t layer = 0; layer < params.m_harqStatus.size (); layer++)
    {
      if (params.m_harqStatus.at (layer) == cv2x_DlInfoListElement_s::ACK)
        {
          // discard buffer
          Ptr<PacketBurst> emptyBuf = CreateObject <PacketBurst> ();
          (*it).second.at (layer).at (params.m_harqProcessId) = emptyBuf;
          NS_LOG_DEBUG (this << " HARQ-ACK UE " << params.m_rnti << " harqId " << (uint16_t)params.m_harqProcessId << " layer " << (uint16_t)layer);
        }
      else if (params.m_harqStatus.at (layer) == cv2x_DlInfoListElement_s::NACK)
        {
          NS_LOG_DEBUG (this << " HARQ-NACK UE " << params.m_rnti << " harqId " << (uint16_t)params.m_harqProcessId << " layer " << (uint16_t)layer);
        }
      else
        {
          NS_FATAL_ERROR (" HARQ functionality not implemented");
        }
    }
  m_dlInfoListReceived.push_back (params);
}


} // namespace ns3

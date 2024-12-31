/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Modified by: NIST (D2D)
 */

#include <ns3/log.h>
#include <ns3/pointer.h>
#include <ns3/math.h>
#include <cfloat>
#include <set>
#include <climits>

#include <ns3/cv2x_lte-amc.h>
#include <ns3/cv2x_rr-ff-mac-scheduler.h>
#include <ns3/simulator.h>
#include <ns3/cv2x_lte-common.h>
#include <ns3/cv2x_lte-vendor-specific-parameters.h>
#include <ns3/boolean.h>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_RrFfMacScheduler");

/// Type 0 allocation RBG
static const int Type0AllocationRbg[4] = {
  10,       // RGB size 1
  26,       // RGB size 2
  63,       // RGB size 3
  110       // RGB size 4
};  // see table 7.1.6.1-1 of 36.213




NS_OBJECT_ENSURE_REGISTERED (cv2x_RrFfMacScheduler);


cv2x_RrFfMacScheduler::cv2x_RrFfMacScheduler ()
  :   m_cschedSapUser (0),
    m_schedSapUser (0),
    m_nextRntiDl (0),
    m_nextRntiUl (0)
{
  m_amc = CreateObject <cv2x_LteAmc> ();
  m_cschedSapProvider = new cv2x_MemberCschedSapProvider<cv2x_RrFfMacScheduler> (this);
  m_schedSapProvider = new cv2x_MemberSchedSapProvider<cv2x_RrFfMacScheduler> (this);
}

cv2x_RrFfMacScheduler::~cv2x_RrFfMacScheduler ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_RrFfMacScheduler::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_dlHarqProcessesDciBuffer.clear ();
  m_dlHarqProcessesTimer.clear ();
  m_dlHarqProcessesRlcPduListBuffer.clear ();
  m_dlInfoListBuffered.clear ();
  m_ulHarqCurrentProcessId.clear ();
  m_ulHarqProcessesStatus.clear ();
  m_ulHarqProcessesDciBuffer.clear ();
  delete m_cschedSapProvider;
  delete m_schedSapProvider;
}

TypeId
cv2x_RrFfMacScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_RrFfMacScheduler")
    .SetParent<cv2x_FfMacScheduler> ()
    .SetGroupName("Lte")
    .AddConstructor<cv2x_RrFfMacScheduler> ()
    .AddAttribute ("CqiTimerThreshold",
                   "The number of TTIs a CQI is valid (default 1000 - 1 sec.)",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&cv2x_RrFfMacScheduler::m_cqiTimersThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("HarqEnabled",
                   "Activate/Deactivate the HARQ [by default is active].",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_RrFfMacScheduler::m_harqOn),
                   MakeBooleanChecker ())
    .AddAttribute ("UlGrantMcs",
                   "The MCS of the UL grant, must be [0..15] (default 0)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_RrFfMacScheduler::m_ulGrantMcs),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}



void
cv2x_RrFfMacScheduler::SetFfMacCschedSapUser (cv2x_FfMacCschedSapUser* s)
{
  m_cschedSapUser = s;
}

void
cv2x_RrFfMacScheduler::SetFfMacSchedSapUser (cv2x_FfMacSchedSapUser* s)
{
  m_schedSapUser = s;
}

cv2x_FfMacCschedSapProvider*
cv2x_RrFfMacScheduler::GetFfMacCschedSapProvider ()
{
  return m_cschedSapProvider;
}

cv2x_FfMacSchedSapProvider*
cv2x_RrFfMacScheduler::GetFfMacSchedSapProvider ()
{
  return m_schedSapProvider;
}

void
cv2x_RrFfMacScheduler::SetLteFfrSapProvider (cv2x_LteFfrSapProvider* s)
{
  m_ffrSapProvider = s;
}

cv2x_LteFfrSapUser*
cv2x_RrFfMacScheduler::GetLteFfrSapUser ()
{
  return m_ffrSapUser;
}

void
cv2x_RrFfMacScheduler::DoCschedCellConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedCellConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  // Read the subset of parameters used
  m_cschedCellConfig = params;
  m_rachAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);
  cv2x_FfMacCschedSapUser::CschedUeConfigCnfParameters cnf;
  cnf.m_result = cv2x_SUCCESS;
  m_cschedSapUser->CschedUeConfigCnf (cnf);
  return;
}

void
cv2x_RrFfMacScheduler::DoCschedUeConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedUeConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this << " RNTI " << params.m_rnti << " txMode " << (uint16_t)params.m_transmissionMode);
  std::map <uint16_t,uint8_t>::iterator it = m_uesTxMode.find (params.m_rnti);
  if (it == m_uesTxMode.end ())
    {
      m_uesTxMode.insert (std::pair <uint16_t, double> (params.m_rnti, params.m_transmissionMode));
      // generate HARQ buffers
      m_dlHarqCurrentProcessId.insert (std::pair <uint16_t,uint8_t > (params.m_rnti, 0));
      DlHarqProcessesStatus_t dlHarqPrcStatus;
      dlHarqPrcStatus.resize (8,0);
      m_dlHarqProcessesStatus.insert (std::pair <uint16_t, DlHarqProcessesStatus_t> (params.m_rnti, dlHarqPrcStatus));
      DlHarqProcessesTimer_t dlHarqProcessesTimer;
      dlHarqProcessesTimer.resize (8,0);
      m_dlHarqProcessesTimer.insert (std::pair <uint16_t, DlHarqProcessesTimer_t> (params.m_rnti, dlHarqProcessesTimer));
      cv2x_DlHarqProcessesDciBuffer_t dlHarqdci;
      dlHarqdci.resize (8);
      m_dlHarqProcessesDciBuffer.insert (std::pair <uint16_t, cv2x_DlHarqProcessesDciBuffer_t> (params.m_rnti, dlHarqdci));
      cv2x_DlHarqRlcPduListBuffer_t dlHarqRlcPdu;
      dlHarqRlcPdu.resize (2);
      dlHarqRlcPdu.at (0).resize (8);
      dlHarqRlcPdu.at (1).resize (8);
      m_dlHarqProcessesRlcPduListBuffer.insert (std::pair <uint16_t, cv2x_DlHarqRlcPduListBuffer_t> (params.m_rnti, dlHarqRlcPdu));
      m_ulHarqCurrentProcessId.insert (std::pair <uint16_t,uint8_t > (params.m_rnti, 0));
      cv2x_UlHarqProcessesStatus_t ulHarqPrcStatus;
      ulHarqPrcStatus.resize (8,0);
      m_ulHarqProcessesStatus.insert (std::pair <uint16_t, cv2x_UlHarqProcessesStatus_t> (params.m_rnti, ulHarqPrcStatus));
      cv2x_UlHarqProcessesDciBuffer_t ulHarqdci;
      ulHarqdci.resize (8);
      m_ulHarqProcessesDciBuffer.insert (std::pair <uint16_t, cv2x_UlHarqProcessesDciBuffer_t> (params.m_rnti, ulHarqdci));
    }
  else
    {
      (*it).second = params.m_transmissionMode;
    }
  return;
}

void
cv2x_RrFfMacScheduler::DoCschedLcConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedLcConfigReqParameters& params)
{
  // NS_LOG_FUNCTION (this);
  // Not used at this stage (LCs updated by DoSchedDlRlcBufferReq)
  // return;
  NS_LOG_FUNCTION (this << " New LC, rnti: "  << params.m_rnti);
  NS_LOG_FUNCTION ("LC configuration. Number of LCs:"<<params.m_logicalChannelConfigList.size ());

  // m_reconfigureFlat indicates if this is a reconfiguration or new UE is added, table  4.1.5 in LTE MAC scheduler specification
  if (params.m_reconfigureFlag)
    {
      std::vector <struct cv2x_LogicalChannelConfigListElement_s>::const_iterator lcit;

      for(lcit = params.m_logicalChannelConfigList.begin (); lcit!= params.m_logicalChannelConfigList.end (); lcit++)
        {
          cv2x_LteFlowId_t flowid = cv2x_LteFlowId_t (params.m_rnti,lcit->m_logicalChannelIdentity);

          if (m_ueLogicalChannelsConfigList.find (flowid) == m_ueLogicalChannelsConfigList.end ())
            {
              NS_LOG_ERROR ("UE logical channels can not be reconfigured because it was not configured before.");
            }
          else
            {
              m_ueLogicalChannelsConfigList.find (flowid)->second = *lcit;
            }
        }
    }    // else new UE is added
  else
    {
      std::vector <struct cv2x_LogicalChannelConfigListElement_s>::const_iterator lcit;

      for (lcit = params.m_logicalChannelConfigList.begin (); lcit != params.m_logicalChannelConfigList.end (); lcit++)
        {
          cv2x_LteFlowId_t flowId = cv2x_LteFlowId_t (params.m_rnti,lcit->m_logicalChannelIdentity);
          m_ueLogicalChannelsConfigList.insert (std::pair<cv2x_LteFlowId_t, cv2x_LogicalChannelConfigListElement_s>(flowId,*lcit));
        }
    }
}

void
cv2x_RrFfMacScheduler::DoCschedLcReleaseReq (const struct cv2x_FfMacCschedSapProvider::CschedLcReleaseReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  std::vector <uint8_t>::const_iterator it;

  for (it = params.m_logicalChannelIdentity.begin (); it != params.m_logicalChannelIdentity.end (); it++)
    {
      cv2x_LteFlowId_t flowId = cv2x_LteFlowId_t (params.m_rnti, *it);

      // find the logical channel with the same Logical Channel Identity in the current list, release it
      if (m_ueLogicalChannelsConfigList.find (flowId)!= m_ueLogicalChannelsConfigList.end ())
        {
          m_ueLogicalChannelsConfigList.erase (flowId);
        }
      else
        {
          NS_FATAL_ERROR ("Logical channels cannot be released because it can not be found in the list of active LCs");
        }
    }
  for (uint16_t i = 0; i < params.m_logicalChannelIdentity.size (); i++)
    {
      std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it = m_rlcBufferReq.begin ();
      std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator temp;
      while (it!=m_rlcBufferReq.end ())
        {
          if (((*it).first.m_rnti == params.m_rnti) && ((*it).first.m_lcId == params.m_logicalChannelIdentity.at (i)))
            {
              temp = it;
              it++;
              m_rlcBufferReq.erase (temp);
            }
          else
            {
              it++;
            }
        }
    }
  return;
}

void
cv2x_RrFfMacScheduler::DoCschedUeReleaseReq (const struct cv2x_FfMacCschedSapProvider::CschedUeReleaseReqParameters& params)
{
  NS_LOG_FUNCTION (this << " Release RNTI " << params.m_rnti);
  
  for (int i=0; i < MAX_LC_LIST; i++)
    {
      cv2x_LteFlowId_t flowId = cv2x_LteFlowId_t (params.m_rnti,i);
      // find the logical channel with the same Logical Channel Identity in the current list, release it
      if (m_ueLogicalChannelsConfigList.find (flowId)!= m_ueLogicalChannelsConfigList.end ())
        {
          m_ueLogicalChannelsConfigList.erase (flowId);
        }
    }

  m_uesTxMode.erase (params.m_rnti);
  m_dlHarqCurrentProcessId.erase (params.m_rnti);
  m_dlHarqProcessesStatus.erase  (params.m_rnti);
  m_dlHarqProcessesTimer.erase (params.m_rnti);
  m_dlHarqProcessesDciBuffer.erase  (params.m_rnti);
  m_dlHarqProcessesRlcPduListBuffer.erase  (params.m_rnti);
  m_ulHarqCurrentProcessId.erase  (params.m_rnti);
  m_ulHarqProcessesStatus.erase  (params.m_rnti);
  m_ulHarqProcessesDciBuffer.erase  (params.m_rnti);
  m_ceBsrRxed.erase (params.m_rnti);
  std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it = m_rlcBufferReq.begin ();
  std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator temp;
  while (it != m_rlcBufferReq.end ())
    {
      if ((*it).first.m_rnti == params.m_rnti)
        {
          temp = it;
          it++;
          m_rlcBufferReq.erase (temp);
        }
      else
        {
          it++;
        }
    }
  if (m_nextRntiUl == params.m_rnti)
    {
      m_nextRntiUl = 0;
    }

  if (m_nextRntiDl == params.m_rnti)
    {
      m_nextRntiDl = 0;
    }
    
  return;
}


void
cv2x_RrFfMacScheduler::DoSchedDlRlcBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this << params.m_rnti << (uint32_t) params.m_logicalChannelIdentity);
  // API generated by RLC for updating RLC parameters on a LC (tx and retx queues)
  std::map <cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;

  cv2x_LteFlowId_t flow (params.m_rnti, params.m_logicalChannelIdentity);

  it =  m_rlcBufferReq.find (flow);

  if (it == m_rlcBufferReq.end ())
    {
      m_rlcBufferReq.insert (std::pair <cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters> (flow, params));

      //m_p10CqiRxed.insert ( std::pair<uint16_t, uint8_t > (params.m_rnti, 1)); // only codeword 0 at this stage (SISO)
      // initialized to 1 (i.e., the lowest value for transmitting a signal)
      //m_p10CqiTimers.insert ( std::pair<uint16_t, uint32_t > (params.m_rnti, m_cqiTimersThreshold));
    }
  else
    {
      (*it).second = params;
    }

  return;

  NS_LOG_INFO (this << " RNTI " << params.m_rnti << " LC " << (uint16_t)params.m_logicalChannelIdentity << " RLC tx size " << params.m_rlcTransmissionQueueHolDelay << " RLC retx size " << params.m_rlcRetransmissionQueueSize << " RLC stat size " <<  params.m_rlcStatusPduSize);
  
  return;
}

void
cv2x_RrFfMacScheduler::DoSchedDlPagingBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlPagingBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("method not implemented");
  return;
}

void
cv2x_RrFfMacScheduler::DoSchedDlMacBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlMacBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("method not implemented");
  return;
}

int
cv2x_RrFfMacScheduler::GetRbgSize (int dlbandwidth)
{
  for (int i = 0; i < 4; i++)
    {
      if (dlbandwidth < Type0AllocationRbg[i])
        {
          return (i + 1);
        }
    }

  return (-1);
}

bool
cv2x_RrFfMacScheduler::SortRlcBufferReq (cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters i,cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters j)
{
  return (i.m_rnti < j.m_rnti);
}

int
cv2x_RrFfMacScheduler::LcActivePerFlow (uint16_t rnti)
{
  std::map <cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;
  int lcActive = 0;
  for (it = m_rlcBufferReq.begin (); it != m_rlcBufferReq.end (); it++)
    {
      if (((*it).first.m_rnti == rnti) && (((*it).second.m_rlcTransmissionQueueSize > 0)
                                           || ((*it).second.m_rlcRetransmissionQueueSize > 0)
                                           || ((*it).second.m_rlcStatusPduSize > 0) ))
        {
          lcActive++;
        }
      if ((*it).first.m_rnti > rnti)
        {
          break;
        }
    }
  return (lcActive);
}

uint8_t
cv2x_RrFfMacScheduler::HarqProcessAvailability (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);

  std::map <uint16_t, uint8_t>::iterator it = m_dlHarqCurrentProcessId.find (rnti);
  if (it == m_dlHarqCurrentProcessId.end ())
    {
      NS_FATAL_ERROR ("No Process Id found for this RNTI " << rnti);
    }
  std::map <uint16_t, DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find (rnti);
  if (itStat == m_dlHarqProcessesStatus.end ())
    {
      NS_FATAL_ERROR ("No Process Id Statusfound for this RNTI " << rnti);
    }
  uint8_t i = (*it).second;
  do
    {
      i = (i + 1) % HARQ_PROC_NUM;
    }
  while ( ((*itStat).second.at (i) != 0)&&(i != (*it).second));
  if ((*itStat).second.at (i) == 0)
    {
      return (true);
    }
  else
    {
      return (false); // return a not valid harq proc id
    }
}



uint8_t
cv2x_RrFfMacScheduler::UpdateHarqProcessId (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);


  if (m_harqOn == false)
    {
      return (0);
    }

  std::map <uint16_t, uint8_t>::iterator it = m_dlHarqCurrentProcessId.find (rnti);
  if (it == m_dlHarqCurrentProcessId.end ())
    {
      NS_FATAL_ERROR ("No Process Id found for this RNTI " << rnti);
    }
  std::map <uint16_t, DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find (rnti);
  if (itStat == m_dlHarqProcessesStatus.end ())
    {
      NS_FATAL_ERROR ("No Process Id Statusfound for this RNTI " << rnti);
    }
  uint8_t i = (*it).second;
  do
    {
      i = (i + 1) % HARQ_PROC_NUM;
    }
  while ( ((*itStat).second.at (i) != 0)&&(i != (*it).second));
  if ((*itStat).second.at (i) == 0)
    {
      (*it).second = i;
      (*itStat).second.at (i) = 1;
    }
  else
    {
      return (9); // return a not valid harq proc id
    }

  return ((*it).second);
}


void
cv2x_RrFfMacScheduler::RefreshHarqProcesses ()
{
  NS_LOG_FUNCTION (this);

  std::map <uint16_t, DlHarqProcessesTimer_t>::iterator itTimers;
  for (itTimers = m_dlHarqProcessesTimer.begin (); itTimers != m_dlHarqProcessesTimer.end (); itTimers ++)
    {
      for (uint16_t i = 0; i < HARQ_PROC_NUM; i++)
        {
          if ((*itTimers).second.at (i) == HARQ_DL_TIMEOUT)
            {
              // reset HARQ process

              NS_LOG_INFO (this << " Reset HARQ proc " << i << " for RNTI " << (*itTimers).first);
              std::map <uint16_t, DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find ((*itTimers).first);
              if (itStat == m_dlHarqProcessesStatus.end ())
                {
                  NS_FATAL_ERROR ("No Process Id Status found for this RNTI " << (*itTimers).first);
                }
              (*itStat).second.at (i) = 0;
              (*itTimers).second.at (i) = 0;
              //Bug 737: clear associated metadata
              std::map <uint16_t, cv2x_DlHarqRlcPduListBuffer_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduListBuffer.find ((*itTimers).first);
              if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end ())
                {
                  NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << (*itTimers).first);
                }
              for (uint16_t k = 0; k < (*itRlcPdu).second.size (); k++)
                {
                  (*itRlcPdu).second.at (k).at (i).clear ();
                }
            }
          else
            {
              (*itTimers).second.at (i)++;
            }
        }
    }

}



void
cv2x_RrFfMacScheduler::DoSchedDlTriggerReq (const struct cv2x_FfMacSchedSapProvider::SchedDlTriggerReqParameters& params)
{
  //std::cout<<" --------  ENTER RR DL SCHEDULER ---------"<<std::endl;
  
  NS_LOG_FUNCTION (this << " DL Frame no. " << (params.m_sfnSf >> 4) << " subframe no. " << (0xF & params.m_sfnSf));
  // API generated by RLC for triggering the scheduling of a DL subframe

  RefreshDlCqiMaps ();
  int rbgSize = GetRbgSize (m_cschedCellConfig.m_dlBandwidth);
  int rbgNum = m_cschedCellConfig.m_dlBandwidth / rbgSize;
  cv2x_FfMacSchedSapUser::SchedDlConfigIndParameters ret;

  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > allocationMapPerRntiPerLCId;
  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itMap;
  allocationMapPerRntiPerLCId.clear ();

  // Generate RBGs map
  std::vector <bool> rbgMap;
  uint16_t rbgAllocatedNum = 0;
  std::set <uint16_t> rntiAllocated;
  rbgMap.resize (m_cschedCellConfig.m_dlBandwidth / rbgSize, false);

  //   update UL HARQ proc id
  std::map <uint16_t, uint8_t>::iterator itProcId;
  for (itProcId = m_ulHarqCurrentProcessId.begin (); itProcId != m_ulHarqCurrentProcessId.end (); itProcId++)
    {
      (*itProcId).second = ((*itProcId).second + 1) % HARQ_PROC_NUM;
    }

  // RACH Allocation
  m_rachAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);
  uint16_t rbStart = 0;
  std::vector <struct cv2x_RachListElement_s>::iterator itRach;
  for (itRach = m_rachList.begin (); itRach != m_rachList.end (); itRach++)
    {
      NS_ASSERT_MSG (m_amc->GetUlTbSizeFromMcs (m_ulGrantMcs, m_cschedCellConfig.m_ulBandwidth) > (*itRach).m_estimatedSize, " Default UL Grant MCS does not allow to send RACH messages");
      cv2x_BuildRarListElement_s newRar;
      newRar.m_rnti = (*itRach).m_rnti;
      // DL-RACH Allocation
      // Ideal: no needs of configuring m_dci
      // UL-RACH Allocation
      newRar.m_grant.m_rnti = newRar.m_rnti;
      newRar.m_grant.m_mcs = m_ulGrantMcs;
      uint16_t rbLen = 1;
      uint16_t tbSizeBits = 0;
      // find lowest TB size that fits UL grant estimated size
      while ((tbSizeBits < (*itRach).m_estimatedSize) && (rbStart + rbLen < m_cschedCellConfig.m_ulBandwidth))
        {
          rbLen++;
          tbSizeBits = m_amc->GetUlTbSizeFromMcs (m_ulGrantMcs, rbLen);
        }
      if (tbSizeBits < (*itRach).m_estimatedSize)
        {
          // no more allocation space: finish allocation
          break;
        }
      newRar.m_grant.m_rbStart = rbStart;
      newRar.m_grant.m_rbLen = rbLen;
      newRar.m_grant.m_tbSize = tbSizeBits / 8;
      newRar.m_grant.m_hopping = false;
      newRar.m_grant.m_tpc = 0;
      newRar.m_grant.m_cqiRequest = false;
      newRar.m_grant.m_ulDelay = false;
      NS_LOG_INFO (this << " UL grant allocated to RNTI " << (*itRach).m_rnti << " rbStart " << rbStart << " rbLen " << rbLen << " MCS " << (uint16_t) m_ulGrantMcs << " tbSize " << newRar.m_grant.m_tbSize);
      for (uint16_t i = rbStart; i < rbStart + rbLen; i++)
        {
          m_rachAllocationMap.at (i) = (*itRach).m_rnti;
        }

      if (m_harqOn == true)
        {
          // generate UL-DCI for HARQ retransmissions
          cv2x_UlDciListElement_s uldci;
          uldci.m_rnti = newRar.m_rnti;
          uldci.m_rbLen = rbLen;
          uldci.m_rbStart = rbStart;
          uldci.m_mcs = m_ulGrantMcs;
          uldci.m_tbSize = tbSizeBits / 8;
          uldci.m_ndi = 1;
          uldci.m_cceIndex = 0;
          uldci.m_aggrLevel = 1;
          uldci.m_ueTxAntennaSelection = 3; // antenna selection OFF
          uldci.m_hopping = false;
          uldci.m_n2Dmrs = 0;
          uldci.m_tpc = 0; // no power control
          uldci.m_cqiRequest = false; // only period CQI at this stage
          uldci.m_ulIndex = 0; // TDD parameter
          uldci.m_dai = 1; // TDD parameter
          uldci.m_freqHopping = 0;
          uldci.m_pdcchPowerOffset = 0; // not used

          uint8_t harqId = 0;
          std::map <uint16_t, uint8_t>::iterator itProcId;
          itProcId = m_ulHarqCurrentProcessId.find (uldci.m_rnti);
          if (itProcId == m_ulHarqCurrentProcessId.end ())
            {
              NS_FATAL_ERROR ("No info find in HARQ buffer for UE " << uldci.m_rnti);
            }
          harqId = (*itProcId).second;
          std::map <uint16_t, cv2x_UlHarqProcessesDciBuffer_t>::iterator itDci = m_ulHarqProcessesDciBuffer.find (uldci.m_rnti);
          if (itDci == m_ulHarqProcessesDciBuffer.end ())
            {
              NS_FATAL_ERROR ("Unable to find RNTI entry in UL DCI HARQ buffer for RNTI " << uldci.m_rnti);
            }
          (*itDci).second.at (harqId) = uldci;
        }

      rbStart = rbStart + rbLen;
      ret.m_buildRarList.push_back (newRar);
    }
  m_rachList.clear ();

  // Process DL HARQ feedback
  RefreshHarqProcesses ();
  // retrieve past HARQ retx buffered
  if (m_dlInfoListBuffered.size () > 0)
    {
      if (params.m_dlInfoList.size () > 0)
        {
          NS_LOG_INFO (this << " Received DL-HARQ feedback");
          m_dlInfoListBuffered.insert (m_dlInfoListBuffered.end (), params.m_dlInfoList.begin (), params.m_dlInfoList.end ());
        }
    }
  else
    {
      if (params.m_dlInfoList.size () > 0)
        {
          m_dlInfoListBuffered = params.m_dlInfoList;
        }
    }
  if (m_harqOn == false)
    {
      // Ignore HARQ feedback
      m_dlInfoListBuffered.clear ();
    }
  std::vector <struct cv2x_DlInfoListElement_s> dlInfoListUntxed;
  for (uint16_t i = 0; i < m_dlInfoListBuffered.size (); i++)
    {
      std::set <uint16_t>::iterator itRnti = rntiAllocated.find (m_dlInfoListBuffered.at (i).m_rnti);
      if (itRnti != rntiAllocated.end ())
        {
          // RNTI already allocated for retx
          continue;
        }
      uint8_t nLayers = m_dlInfoListBuffered.at (i).m_harqStatus.size ();
      std::vector <bool> retx;
      NS_LOG_INFO (this << " Processing DLHARQ feedback");
      if (nLayers == 1)
        {
          retx.push_back (m_dlInfoListBuffered.at (i).m_harqStatus.at (0) == cv2x_DlInfoListElement_s::NACK);
          retx.push_back (false);
        }
      else
        {
          retx.push_back (m_dlInfoListBuffered.at (i).m_harqStatus.at (0) == cv2x_DlInfoListElement_s::NACK);
          retx.push_back (m_dlInfoListBuffered.at (i).m_harqStatus.at (1) == cv2x_DlInfoListElement_s::NACK);
        }
      if (retx.at (0) || retx.at (1))
        {
          // retrieve HARQ process information
          uint16_t rnti = m_dlInfoListBuffered.at (i).m_rnti;
          uint8_t harqId = m_dlInfoListBuffered.at (i).m_harqProcessId;
          NS_LOG_INFO (this << " HARQ retx RNTI " << rnti << " harqId " << (uint16_t)harqId);
          std::map <uint16_t, cv2x_DlHarqProcessesDciBuffer_t>::iterator itHarq = m_dlHarqProcessesDciBuffer.find (rnti);
          if (itHarq == m_dlHarqProcessesDciBuffer.end ())
            {
              NS_FATAL_ERROR ("No info find in HARQ buffer for UE " << rnti);
            }

          cv2x_DlDciListElement_s dci = (*itHarq).second.at (harqId);
          int rv = 0;
          if (dci.m_rv.size () == 1)
            {
              rv = dci.m_rv.at (0);
            }
          else
            {
              rv = (dci.m_rv.at (0) > dci.m_rv.at (1) ? dci.m_rv.at (0) : dci.m_rv.at (1));
            }

          if (rv == 3)
            {
              // maximum number of retx reached -> drop process
              NS_LOG_INFO ("Max number of retransmissions reached -> drop process");
              std::map <uint16_t, DlHarqProcessesStatus_t>::iterator it = m_dlHarqProcessesStatus.find (rnti);
              if (it == m_dlHarqProcessesStatus.end ())
                {
                  NS_LOG_ERROR ("No info find in HARQ buffer for UE (might change eNB) " << m_dlInfoListBuffered.at (i).m_rnti);
                }
              (*it).second.at (harqId) = 0;
              std::map <uint16_t, cv2x_DlHarqRlcPduListBuffer_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduListBuffer.find (rnti);
              if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end ())
                {
                  NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << m_dlInfoListBuffered.at (i).m_rnti);
                }
              for (uint16_t k = 0; k < (*itRlcPdu).second.size (); k++)
                {
                  (*itRlcPdu).second.at (k).at (harqId).clear ();
                }
              continue;
            }
          // check the feasibility of retransmitting on the same RBGs
          // translate the DCI to Spectrum framework
          std::vector <int> dciRbg;
          uint32_t mask = 0x1;
          NS_LOG_INFO ("Original RBGs " << dci.m_rbBitmap << " rnti " << dci.m_rnti);
          for (int j = 0; j < 32; j++)
            {
              if (((dci.m_rbBitmap & mask) >> j) == 1)
                {
                  dciRbg.push_back (j);
                  NS_LOG_INFO ("\t" << j);
                }
              mask = (mask << 1);
            }
          bool free = true;
          for (uint8_t j = 0; j < dciRbg.size (); j++)
            {
              if (rbgMap.at (dciRbg.at (j)) == true)
                {
                  free = false;
                  break;
                }
            }
          if (free)
            {
              // use the same RBGs for the retx
              // reserve RBGs
              for (uint8_t j = 0; j < dciRbg.size (); j++)
                {
                  rbgMap.at (dciRbg.at (j)) = true;
                  NS_LOG_INFO ("RBG " << dciRbg.at (j) << " assigned");
                  rbgAllocatedNum++;
                }

              NS_LOG_INFO (this << " Send retx in the same RBGs");
            }
          else
            {
              // find RBGs for sending HARQ retx
              uint8_t j = 0;
              uint8_t rbgId = (dciRbg.at (dciRbg.size () - 1) + 1) % rbgNum;
              uint8_t startRbg = dciRbg.at (dciRbg.size () - 1);
              std::vector <bool> rbgMapCopy = rbgMap;
              while ((j < dciRbg.size ())&&(startRbg != rbgId))
                {
                  if (rbgMapCopy.at (rbgId) == false)
                    {
                      rbgMapCopy.at (rbgId) = true;
                      dciRbg.at (j) = rbgId;
                      j++;
                    }
                  rbgId = (rbgId + 1) % rbgNum;
                }
              if (j == dciRbg.size ())
                {
                  // find new RBGs -> update DCI map
                  uint64_t rbgMask = 0;
                  for (uint16_t k = 0; k < dciRbg.size (); k++)
                    {
                      rbgMask = rbgMask + (0x1 << dciRbg.at (k));
                      NS_LOG_INFO (this << " New allocated RBG " << dciRbg.at (k));
                      rbgAllocatedNum++;
                    }
                  dci.m_rbBitmap = rbgMask;
                  rbgMap = rbgMapCopy;
                }
              else
                {
                  // HARQ retx cannot be performed on this TTI -> store it
                  dlInfoListUntxed.push_back (m_dlInfoListBuffered.at (i));
                  NS_LOG_INFO (this << " No resource for this retx -> buffer it");
                }
            }
          // retrieve RLC PDU list for retx TBsize and update DCI
          cv2x_BuildDataListElement_s newEl;
          std::map <uint16_t, cv2x_DlHarqRlcPduListBuffer_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduListBuffer.find (rnti);
          if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end ())
            {
              NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << rnti);
            }
          for (uint8_t j = 0; j < nLayers; j++)
            {
              if (retx.at (j))
                {
                  if (j >= dci.m_ndi.size ())
                    {
                      // for avoiding errors in MIMO transient phases
                      dci.m_ndi.push_back (0);
                      dci.m_rv.push_back (0);
                      dci.m_mcs.push_back (0);
                      dci.m_tbsSize.push_back (0);
                      NS_LOG_INFO (this << " layer " << (uint16_t)j << " no txed (MIMO transition)");

                    }
                  else
                    {
                      dci.m_ndi.at (j) = 0;
                      dci.m_rv.at (j)++;
                      (*itHarq).second.at (harqId).m_rv.at (j)++;
                      NS_LOG_INFO (this << " layer " << (uint16_t)j << " RV " << (uint16_t)dci.m_rv.at (j));
                    }
                }
              else
                {
                  // empty TB of layer j
                  dci.m_ndi.at (j) = 0;
                  dci.m_rv.at (j) = 0;
                  dci.m_mcs.at (j) = 0;
                  dci.m_tbsSize.at (j) = 0;
                  NS_LOG_INFO (this << " layer " << (uint16_t)j << " no retx");
                }
            }

          for (uint16_t k = 0; k < (*itRlcPdu).second.at (0).at (dci.m_harqProcess).size (); k++)
            {
              std::vector <struct cv2x_RlcPduListElement_s> rlcPduListPerLc;
              for (uint8_t j = 0; j < nLayers; j++)
                {
                  if (retx.at (j))
                    {
                      if (j < dci.m_ndi.size ())
                        {
                          NS_LOG_INFO (" layer " << (uint16_t)j << " tb size " << dci.m_tbsSize.at (j));
                          rlcPduListPerLc.push_back ((*itRlcPdu).second.at (j).at (dci.m_harqProcess).at (k));
                        }
                    }
                  else
                    { // if no retx needed on layer j, push an cv2x_RlcPduListElement_s object with m_size=0 to keep the size of rlcPduListPerLc vector = 2 in case of MIMO
                      NS_LOG_INFO (" layer " << (uint16_t)j << " tb size "<<dci.m_tbsSize.at (j));
                      cv2x_RlcPduListElement_s emptyElement;
                      emptyElement.m_logicalChannelIdentity = (*itRlcPdu).second.at (j).at (dci.m_harqProcess).at (k).m_logicalChannelIdentity;
                      emptyElement.m_size = 0;
                      rlcPduListPerLc.push_back (emptyElement);
                    }
                }

              if (rlcPduListPerLc.size () > 0)
                {
                  newEl.m_rlcPduList.push_back (rlcPduListPerLc);
                }
            }
          newEl.m_rnti = rnti;
          newEl.m_dci = dci;
          (*itHarq).second.at (harqId).m_rv = dci.m_rv;
          // refresh timer
          std::map <uint16_t, DlHarqProcessesTimer_t>::iterator itHarqTimer = m_dlHarqProcessesTimer.find (rnti);
          if (itHarqTimer== m_dlHarqProcessesTimer.end ())
            {
              NS_FATAL_ERROR ("Unable to find HARQ timer for RNTI " << (uint16_t)rnti);
            }
          (*itHarqTimer).second.at (harqId) = 0;
          ret.m_buildDataList.push_back (newEl);
          rntiAllocated.insert (rnti);
        }
      else
        {
          // update HARQ process status
          NS_LOG_INFO (this << " HARQ ACK UE " << m_dlInfoListBuffered.at (i).m_rnti);
          std::map <uint16_t, DlHarqProcessesStatus_t>::iterator it = m_dlHarqProcessesStatus.find (m_dlInfoListBuffered.at (i).m_rnti);
          if (it == m_dlHarqProcessesStatus.end ())
            {
              NS_FATAL_ERROR ("No info find in HARQ buffer for UE " << m_dlInfoListBuffered.at (i).m_rnti);
            }
          (*it).second.at (m_dlInfoListBuffered.at (i).m_harqProcessId) = 0;
          std::map <uint16_t, cv2x_DlHarqRlcPduListBuffer_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduListBuffer.find (m_dlInfoListBuffered.at (i).m_rnti);
          if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end ())
            {
              NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << m_dlInfoListBuffered.at (i).m_rnti);
            }
          for (uint16_t k = 0; k < (*itRlcPdu).second.size (); k++)
            {
              (*itRlcPdu).second.at (k).at (m_dlInfoListBuffered.at (i).m_harqProcessId).clear ();
            }
        }
    }
  m_dlInfoListBuffered.clear ();
  m_dlInfoListBuffered = dlInfoListUntxed;

  if (rbgAllocatedNum == rbgNum)
    {
      // all the RBGs are already allocated -> exit
      if ((ret.m_buildDataList.size () > 0) || (ret.m_buildRarList.size () > 0))
        {
          m_schedSapUser->SchedDlConfigInd (ret);
        }
      return;
    }

std::vector <cv2x_LteFlowId_t> satisfiedFlows;      // structure to save satisfied flows 
  double bytesTxedF = 0;                             // accumulated data for each flows             
 
  std::map <cv2x_LteFlowId_t,struct cv2x_LogicalChannelConfigListElement_s>::iterator itLogicalChannels;
  std::map <cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itF;

  std::vector <uint16_t> tempV;      // structure to save the RBGs assigned to each flow 

  std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itDL= m_rlcBufferReq.begin();

  
  if ( m_nextRntiDl == 0 )
   {
     for ( itDL= m_rlcBufferReq.begin(); itDL != m_rlcBufferReq.end(); itDL ++ )
      {
        if ( (*itDL).first.m_rnti != 0 ) 
         {
            m_nextRntiDl = (*itDL).first.m_rnti;
            m_nextLcidDl = (*itDL).first.m_lcId;
         }
      }
   }

  if ( m_nextRntiDl == 0 ) 
   {
     ret.m_nrOfPdcchOfdmSymbols = 1;   /// \todo check correct value according the DCIs txed
     m_schedSapUser->SchedDlConfigInd (ret);
     return;
   }
    
  //std::cout<<" m_nextRntiDl = "<<(int)m_nextRntiDl<<" m_nextLcidDl = "<<(int)m_nextLcidDl<<std::endl;

  std::map <cv2x_LteFlowId_t,struct cv2x_LogicalChannelConfigListElement_s>::iterator itTemp1;
  for ( itTemp1= m_ueLogicalChannelsConfigList.begin(); itTemp1 != m_ueLogicalChannelsConfigList.end(); itTemp1 ++ )
   { 
      //std::cout<<" rnti = "<<(int) (*itTemp1).first.m_rnti<<" lcid = "<<(int)(*itTemp1).first.m_lcId<<std::endl;
   }

  //std::cout<<"\t display rlc buffer ; "<<std::endl;
  std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itBuffer;
  for ( itBuffer= m_rlcBufferReq.begin(); itBuffer != m_rlcBufferReq.end(); itBuffer ++ ) 
   {
      //std::cout<<"\t rnti = "<<(int)itBuffer->first.m_rnti<<" Lcid = "<<(int)itBuffer->first.m_lcId<<" data = "<<(int) (*itBuffer).second.m_rlcTransmissionQueueSize<<std::endl;
   }

  unsigned int nbHarq = 0 ;
  for (int i = 0; i < rbgNum; i++)
   {
      NS_LOG_INFO (this << " ALLOCATION for RBG " << i << " of " << rbgNum);
      //std::cout<<"\t ALLOCATION for RBG " << i << " of " << rbgNum<<std::endl;
    
      if (rbgMap.at (i) == false)    // this RBG is not allocated
        {
          std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itLogicalChannels;
          std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itMax = m_rlcBufferReq.end ();
          
          cv2x_LteFlowId_t m_next_flow = cv2x_LteFlowId_t (m_nextRntiDl, m_nextLcidDl);
          itLogicalChannels = m_rlcBufferReq.find (m_next_flow);
          if ( itLogicalChannels == m_rlcBufferReq.end()) 
            itLogicalChannels = m_rlcBufferReq.begin();

          unsigned int nbLoop = 0 ;

          do 
            {
              nbLoop ++;

              std::set <uint16_t>::iterator itRnti = rntiAllocated.find (itLogicalChannels->first.m_rnti);
              if ((itRnti != rntiAllocated.end ())||(!HarqProcessAvailability (itLogicalChannels->first.m_rnti)))
               {
                 // UE already allocated for HARQ or without HARQ process available -> drop it
                 if (itRnti != rntiAllocated.end ())
                 {
                   NS_LOG_DEBUG (this << " RNTI discared for HARQ tx" << (uint16_t)(itLogicalChannels->first.m_rnti));
                 }
                 if (!HarqProcessAvailability (itLogicalChannels->first.m_rnti))
                 {
                   NS_LOG_DEBUG (this << " RNTI discared for HARQ id" << (uint16_t)(itLogicalChannels->first.m_rnti));
                 }
             
                 nbHarq ++;
                 itLogicalChannels ++;
                 if ( itLogicalChannels == m_rlcBufferReq.end ()) 
                     itLogicalChannels = m_rlcBufferReq.begin ();  

                 m_nextRntiDl = itLogicalChannels->first.m_rnti;
                 m_nextLcidDl = itLogicalChannels->first.m_lcId;
                 m_next_flow = cv2x_LteFlowId_t (m_nextRntiDl, m_nextLcidDl);
                 continue;
               }
   
              cv2x_LteFlowId_t flowId1 ;
              flowId1.m_rnti= itLogicalChannels->first.m_rnti;
              flowId1.m_lcId= itLogicalChannels->first.m_lcId;
              std::vector <cv2x_LteFlowId_t>::iterator Ft;
              bool findF=false;
              //trying to find the current flow in the satisfied flows 
              for(Ft=satisfiedFlows.begin();Ft!=satisfiedFlows.end();Ft++)
                {
                  if(((*Ft).m_rnti==flowId1.m_rnti) && ((*Ft).m_lcId==flowId1.m_lcId))
                    {
                     findF=true;
                     break;
                    }
                }
              if (findF==false) // this flow doesn't exist in the satisfied flows --> needs more RBGs to transmit its data  
               {
                 std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itQ;
                 cv2x_LteFlowId_t flowId = cv2x_LteFlowId_t (itLogicalChannels->first.m_rnti, itLogicalChannels->second.m_logicalChannelIdentity);
                 itQ = m_rlcBufferReq.find(flowId);
                 if (itQ!=m_rlcBufferReq.end())
                  {
                    if (((*itQ).second.m_rlcTransmissionQueueSize > 0))
                      {          
                         itMax = itLogicalChannels;
                      }
                     else 
                      {
                         itLogicalChannels ++;
                         if ( itLogicalChannels == m_rlcBufferReq.end ()) 
                            itLogicalChannels = m_rlcBufferReq.begin (); 

                         m_nextRntiDl = itLogicalChannels->first.m_rnti;
                         m_nextLcidDl = itLogicalChannels->first.m_lcId;  
                         m_next_flow = cv2x_LteFlowId_t (m_nextRntiDl, m_nextLcidDl);   // added ! 
                         continue;
                      }

                  }  //end if itQ!=m_rlcBufferReq.end()
               }  //end findF==false
 
            itLogicalChannels ++;
            if ( itLogicalChannels == m_rlcBufferReq.end ()) 
               itLogicalChannels = m_rlcBufferReq.begin ();   
            
            } //end for itLogicalchannels
          while ( (itLogicalChannels->first.m_rnti ==  m_next_flow.m_rnti) && ( itLogicalChannels->first.m_lcId == m_next_flow.m_lcId) && (nbHarq < m_rlcBufferReq.size()) && (nbLoop <=  m_rlcBufferReq.size()) );


          if (itMax == m_rlcBufferReq.end ())
            {
              // no UE available for this RB
              NS_LOG_INFO (this << " any UE found");
            }
           else
            {
                //std::cout<<" it max , rnti = "<<(int) itMax->first.m_rnti<<" lcid = "<<(int)itMax->first.m_lcId<<std::endl;
                std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itMap;
                itMap = allocationMapPerRntiPerLCId.find ((uint16_t)(itMax->first.m_rnti));
                if (itMap == allocationMapPerRntiPerLCId.end ())
                  {
                    // insert new element
                    std::map<uint8_t, std::vector <uint16_t> > tempMap;
                    
                    tempV.push_back(i);
                    tempMap.insert (std::pair<uint8_t, std::vector <uint16_t> > (itMax->first.m_lcId, tempV));
                    allocationMapPerRntiPerLCId.insert (std::pair <uint16_t, std::map<uint8_t, std::vector <uint16_t> > > (itMax->first.m_rnti, tempMap));
                    rbgMap.at (i) = true;
                   
                    tempV.clear();

                     /*********      testing if the flow is satisfied from the first RBG allocated :  *****************************/
                    std::map <uint16_t,uint8_t>::iterator itTxMode;
                    itTxMode = m_uesTxMode.find (itMax->first.m_rnti);
                    if (itTxMode == m_uesTxMode.end ())
                     {
                       NS_FATAL_ERROR ("No Transmission Mode info on user " << itMax->first.m_rnti);
                     }
                    int nLayer = cv2x_TransmissionModesLayers::TxMode2LayerNum ((*itTxMode).second);  // nbr of layers for MIMO transmissions
                       
                    int b=1;
                    int mcsF=0;
                    std::map <uint16_t,uint8_t>::iterator itCqi = m_p10CqiRxed.find (itMax->first.m_rnti);
                    if (itCqi == m_p10CqiRxed.end ())
                     {
                       mcsF=0; // no info on this user -> lowest MCS
                     }
                    else
                     {
                       mcsF=m_amc->GetMcsFromCqi ((*itCqi).second);
                     }
                    bytesTxedF=(m_amc->GetDlTbSizeFromMcs (mcsF, b * rbgSize) / 8) * nLayer;   //multiply by nLayer for MIMO 
                    std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itQF;
                    cv2x_LteFlowId_t flowIdF = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->first.m_lcId);
                    itQF = m_rlcBufferReq.find(flowIdF);
                    //std::cout<<" \t first allocation rnti = "<<(int)flowIdF.m_rnti<<" lcid = "<<(int) flowIdF.m_lcId<< " mcs = "<<(int) mcsF<<" data to transmit = " <<(int)(*itQF).second.m_rlcTransmissionQueueSize <<" allocated without n = " <<(int)(m_amc->GetDlTbSizeFromMcs (mcsF, b * rbgSize) / 8)<<" allocated with n = " <<(int) bytesTxedF<<std::endl;
                       
                    if ((*itQF).second.m_rlcTransmissionQueueSize <= bytesTxedF )
                     {
                        //this flow is  satisfied with this first allocation --> add it to satisfied flows 
                        satisfiedFlows.push_back(flowIdF);
                        //std::cout<<"\t --> this flow is satisfied "<<std::endl;
                        std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itLogicalChannels2;
                        cv2x_LteFlowId_t flow2 = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->first.m_lcId);
                                 
                        itLogicalChannels2 = m_rlcBufferReq.find(flow2);
                        itLogicalChannels2 ++;
                        if ( itLogicalChannels2 == m_rlcBufferReq.end())
                          itLogicalChannels2 = m_rlcBufferReq.begin();

                        m_nextRntiDl = itLogicalChannels2->first.m_rnti;
                        m_nextLcidDl = itLogicalChannels2->first.m_lcId;
                     }
                    else if ( i == (rbgNum -1))
                     {
                       // flow still not satisfied --> mark it as the next flow 
                       m_nextRntiDl = itMax->first.m_rnti;
                       m_nextLcidDl = itMax->first.m_lcId;
                       //std::cout<<" \t update next flow : m_nextRntiDl = "<<(int)m_nextRntiDl<<" m_nextLcidDl = "<<(int)m_nextLcidDl<<std::endl;
                     }
                    /***********************************************************************************/
                  }
                else
                  {
                    std::map <uint8_t, std::vector <uint16_t> >::iterator itSMap;
                    itSMap=itMap->second.find ((uint8_t)itMax->first.m_lcId);
                    if (itSMap == itMap->second.end ())
                     {
                       //insert new flow id element 
                       tempV.push_back(i);
                       itMap->second.insert (std::pair<uint8_t, std::vector <uint16_t> > (itMax->first.m_lcId, tempV));
                       rbgMap.at (i) = true;
                      
                       tempV.clear();

                        /*********      testing if the flow is satisfied from the first RBG allocated :  *****************************/
                       std::map <uint16_t,uint8_t>::iterator itTxMode;
                       itTxMode = m_uesTxMode.find (itMax->first.m_rnti);
                       if (itTxMode == m_uesTxMode.end ())
                        {
                          NS_FATAL_ERROR ("No Transmission Mode info on user " << itMax->first.m_rnti);
                        }
                       int nLayer = cv2x_TransmissionModesLayers::TxMode2LayerNum ((*itTxMode).second);  // nbr of layers for MIMO transmissions
                       
                       int b=1;
                       int mcsF=0;
                       std::map <uint16_t,uint8_t>::iterator itCqi = m_p10CqiRxed.find (itMax->first.m_rnti);
                       if (itCqi == m_p10CqiRxed.end ())
                        {
                          mcsF=0; // no info on this user -> lowest MCS
                        }
                       else
                        {
                          mcsF=m_amc->GetMcsFromCqi ((*itCqi).second);
                        }
                       bytesTxedF=(m_amc->GetDlTbSizeFromMcs (mcsF, b * rbgSize) / 8) * nLayer;   //multiply by nLayer for MIMO 
                       std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itQF;
                       cv2x_LteFlowId_t flowIdF = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->first.m_lcId);
                       itQF = m_rlcBufferReq.find(flowIdF);
                       //std::cout<<" \t first allocation rnti = "<<(int)flowIdF.m_rnti<<" lcid = "<<(int) flowIdF.m_lcId<< " data to transmit = " <<(int)(*itQF).second.m_rlcTransmissionQueueSize <<" allocated without n = " <<(int)(m_amc->GetDlTbSizeFromMcs (mcsF, b * rbgSize) / 8)<<" allocated with n = " <<(int) bytesTxedF<<std::endl;
                       
                       if ((*itQF).second.m_rlcTransmissionQueueSize <= bytesTxedF )
                        {
                           //this flow is  satisfied with this first allocation --> add it to satisfied flows 
                           satisfiedFlows.push_back(flowIdF);
                           //std::cout<<"\t --> this flow is satisfied "<<std::endl;

                           std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itLogicalChannels2;
                           cv2x_LteFlowId_t flow2 = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->first.m_lcId);
                                 
                           itLogicalChannels2 = m_rlcBufferReq.find(flow2);
                           itLogicalChannels2 ++;
                           if ( itLogicalChannels2 == m_rlcBufferReq.end())
                              itLogicalChannels2 = m_rlcBufferReq.begin();

                           m_nextRntiDl = itLogicalChannels2->first.m_rnti;
                           m_nextLcidDl = itLogicalChannels2->first.m_lcId;

                        }
                       else if ( i == (rbgNum -1))
                        {
                          // flow still not satisfied --> mark it as the next flow 
                          m_nextRntiDl = itMax->first.m_rnti;
                          m_nextLcidDl = itMax->first.m_lcId;
                          //std::cout<<" \t update next flow : m_nextRntiDl = "<<(int)m_nextRntiDl<<" m_nextLcidDl = "<<(int)m_nextLcidDl<<std::endl;
                        }
                    /***********************************************************************************/
                     }
                     else 
                     {
                       //std::cout<<" update floew "<<std::endl;
                       int mcsF=0;
                       std::map <uint16_t,uint8_t>::iterator itTxMode;
                       itTxMode = m_uesTxMode.find (itMax->first.m_rnti);
                       if (itTxMode == m_uesTxMode.end ())
                        {
                          NS_FATAL_ERROR ("No Transmission Mode info on user " << itMax->first.m_rnti);
                        }
                       int nLayer = cv2x_TransmissionModesLayers::TxMode2LayerNum ((*itTxMode).second);  // nbr of layers for MIMO transmissions
                       
                       int b=1;
                       b=b+(*itSMap).second.size();
                       std::map <uint16_t,uint8_t>::iterator itCqi = m_p10CqiRxed.find (itMax->first.m_rnti);
                       if (itCqi == m_p10CqiRxed.end ())
                        {
                          mcsF=0; // no info on this user -> lowest MCS
                        }
                       else
                        {
                          mcsF=m_amc->GetMcsFromCqi ((*itCqi).second);
                        }
                        //std::cout<<"\t mcs = "<<(int) mcsF <<std::endl;
                        bytesTxedF=(m_amc->GetDlTbSizeFromMcs (mcsF, b * rbgSize) / 8) * nLayer;   //multiply by nLayer for MIMO 

                        std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itQF;
                        cv2x_LteFlowId_t flowIdF = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->first.m_lcId);
                        itQF = m_rlcBufferReq.find(flowIdF);
                        //std::cout<<" \t additional allocation rnti = "<<(int)flowIdF.m_rnti<<" lcid = "<<(int) flowIdF.m_lcId<< " data to transmit = " <<(int)(*itQF).second.m_rlcTransmissionQueueSize <<" allocated without n = " <<(int)(m_amc->GetDlTbSizeFromMcs (mcsF, b * rbgSize) / 8)<<" allocated with n = " <<(int) bytesTxedF<<std::endl;
                       
                        if ((*itQF).second.m_rlcTransmissionQueueSize <= bytesTxedF )
                         {             
                           (*itSMap).second.push_back (i);
                           rbgMap.at (i) = true;
               
                           //this flow is  satisfied with this new allocation --> add it to satisfied flows 
                           std::vector <cv2x_LteFlowId_t>::iterator Ft;
                           bool findF2=false;
                           for(Ft=satisfiedFlows.begin();Ft!=satisfiedFlows.end();Ft++)
                            {
                              if(((*Ft).m_rnti==flowIdF.m_rnti) && ((*Ft).m_lcId==flowIdF.m_lcId))
                               {
                                 findF2=true;
                                 break;
                               }
                            }
                           if (findF2==false)
                            {
                              satisfiedFlows.push_back(flowIdF);
                              //std::cout<<"\t --> this flow is satisfied "<<std::endl;

                              std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itLogicalChannels2;
                              cv2x_LteFlowId_t flow2 = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->first.m_lcId);
                                 
                              itLogicalChannels2 = m_rlcBufferReq.find(flow2);
                              itLogicalChannels2 ++;
                              if ( itLogicalChannels2 == m_rlcBufferReq.end())
                                 itLogicalChannels2 = m_rlcBufferReq.begin();

                              m_nextRntiDl = itLogicalChannels2->first.m_rnti;
                              m_nextLcidDl = itLogicalChannels2->first.m_lcId;
                            }  
                         }
                         else 
                         {
                           (*itSMap).second.push_back (i);
                           rbgMap.at (i) = true;
                      
                           if ( i == (rbgNum -1))
                            {
                               // flow still not satisfied --> mark it as the next flow 
                               //calculate how many RBGs are allocated for this flow 
                               int nRbg = (*itSMap).second.size() ;
                               if ( nRbg == rbgNum)
                                {
                                  std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itLogicalChannels2;
                                  cv2x_LteFlowId_t flow2 = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->first.m_lcId);
                                 
                                  itLogicalChannels2 = m_rlcBufferReq.find(flow2);
                                  itLogicalChannels2 ++;
                                  if ( itLogicalChannels2 == m_rlcBufferReq.end())
                                    itLogicalChannels2 = m_rlcBufferReq.begin();

                                  m_nextRntiDl = itLogicalChannels2->first.m_rnti;
                                  m_nextLcidDl = itLogicalChannels2->first.m_lcId;
                                }
                               else
                                {
                                  m_nextRntiDl = itMax->first.m_rnti;
                                  m_nextLcidDl = itMax->first.m_lcId;
                                }
                             }
                         }
                     }   //end if itSMap == itMap->second.end ()
                  }  //end if itMap == allocationMapPerRntiPerLCId.end ()
                NS_LOG_INFO (this << " UE assigned " << (uint16_t)(itLogicalChannels->first.m_rnti));
              }
        } // end for RBG free
    } // end for RBGs

   tempV.clear();

  // reorganize the allocation map to be contiguous allocated for each UE 
  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > CopyAllocatedMap;
  CopyAllocatedMap.insert ( allocationMapPerRntiPerLCId.begin(), allocationMapPerRntiPerLCId.end());
  allocationMapPerRntiPerLCId.clear ();
  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itCopy2 = CopyAllocatedMap.begin ();
  unsigned int comp=0;
  while (itCopy2 != CopyAllocatedMap.end ())
   {
      std::map <uint8_t, std::vector <uint16_t> > mapFlow=itCopy2->second;
      std::map <uint8_t, std::vector <uint16_t> >::iterator itCopy3= mapFlow.begin();
      
      while (itCopy3!=mapFlow.end())
       {
          std::map<uint8_t, std::vector <uint16_t> > tempMap; 
          std::vector <uint16_t> tempV;
          for (unsigned int j=0; j< (*itCopy3).second.size ();j++,comp++) 
           {
              // insert new element   
              tempV.push_back(comp);
           }
          std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itFind = allocationMapPerRntiPerLCId.find((*itCopy2).first);
          if ( itFind == allocationMapPerRntiPerLCId.end()) 
           {
             tempMap.insert (std::pair<uint8_t, std::vector <uint16_t> > (itCopy3->first, tempV));
             allocationMapPerRntiPerLCId.insert (std::pair <uint16_t, std::map<uint8_t, std::vector <uint16_t> > > (itCopy2->first, tempMap));
           }
          else
           {
             //rnti already in the map 
             itFind->second.insert (std::pair<uint8_t, std::vector <uint16_t> > (itCopy3->first, tempV));       
           }
          tempV.clear();
          itCopy3++;
       }
      itCopy2++;
    }

  itMap = allocationMapPerRntiPerLCId.begin ();

  while (itMap != allocationMapPerRntiPerLCId.end ())
    {
      // create new cv2x_BuildDataListElement_s for this LC
      cv2x_BuildDataListElement_s newEl;
      newEl.m_rnti = (*itMap).first;
      
      cv2x_DlDciListElement_s newDci;
      newDci.m_rnti = (*itMap).first;
      newDci.m_harqProcess = UpdateHarqProcessId ((*itMap).first);
      uint16_t lcActives = LcActivePerFlow ((*itMap).first);
      
      if (lcActives == 0)
        {
          // Set to max value, to avoid divide by 0 below
          lcActives = 1; // UINT16_MAX;
        }

      std::map <uint16_t,uint8_t>::iterator itTxMode;
      itTxMode = m_uesTxMode.find ((*itMap).first);
      if (itTxMode == m_uesTxMode.end ())
        {
          NS_FATAL_ERROR ("No Transmission Mode info on user " << (*itMap).first);
        }
      int nLayer = cv2x_TransmissionModesLayers::TxMode2LayerNum ((*itTxMode).second);

       std::map <uint16_t,uint8_t>::iterator itCqi = m_p10CqiRxed.find (newEl.m_rnti);
      for (uint8_t i = 0; i < nLayer; i++)
        {
          if (itCqi == m_p10CqiRxed.end ())
            {
              newDci.m_mcs.push_back (0); // no info on this user -> lowest MCS
            }
          else
            {
              newDci.m_mcs.push_back ( m_amc->GetMcsFromCqi ((*itCqi).second) );
            }
        }
        std::map <uint8_t, std::vector <uint16_t> > MapAux;
        MapAux=itMap->second;
        std::map <uint8_t, std::vector <uint16_t> >::iterator itFF=MapAux.begin();
        uint16_t RgbPerRnti=0;
        for (itFF=MapAux.begin();itFF!=MapAux.end();itFF++)
         {
           RgbPerRnti=RgbPerRnti+(*itFF).second.size();
         }

        for (uint8_t j = 0; j < nLayer; j++)
         {
           int tbSize = (m_amc->GetDlTbSizeFromMcs (newDci.m_mcs.at (j), RgbPerRnti * rbgSize) / 8);
           newDci.m_tbsSize.push_back (tbSize);
         }

        newDci.m_resAlloc = 0;  // only allocation type 0 at this stage
        newDci.m_rbBitmap = 0; // TBD (32 bit bitmap see 7.1.6 of 36.213)
        uint64_t rbgMask = 0;

      
        for (itFF=MapAux.begin();itFF!=MapAux.end();itFF++)
         {
           for (uint16_t k = 0; k < (*itFF).second.size (); k++)
            {
              rbgMask = rbgMask + (0x1 << (*itFF).second.at (k));
              NS_LOG_INFO (this << " Allocated RBG " << (*itFF).second.at (k));
            }
         }
  
        newDci.m_rbBitmap = rbgMask;
      
      std::map <cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itBufReq; 

      // updating the rlc buffer : code modified by richard to find correctly the flow to update  
      for (itFF=MapAux.begin();itFF!=MapAux.end();itFF++) 
        {
          cv2x_LteFlowId_t flowId ((*itMap).first, (*itFF).first);
          itBufReq = m_rlcBufferReq.find (flowId);
          if (itBufReq == m_rlcBufferReq.end ())
            {
              NS_FATAL_ERROR ("Cannot find flow");
            }
          else if ( ((*itBufReq).second.m_rlcTransmissionQueueSize > 0)
                   || ((*itBufReq).second.m_rlcRetransmissionQueueSize > 0)
                   || ((*itBufReq).second.m_rlcStatusPduSize > 0) )
            {              
              uint16_t RgbPerFlow=(*itFF).second.size();
              
              std::vector <struct cv2x_RlcPduListElement_s> newRlcPduLe;
              uint32_t bytesTxed = 0;
              for (uint8_t j = 0; j < nLayer; j++)
                {
                  cv2x_RlcPduListElement_s newRlcEl;     // RLC element per flow 
                  newRlcEl.m_logicalChannelIdentity = (*itBufReq).first.m_lcId;
                  newRlcEl.m_size = (m_amc->GetDlTbSizeFromMcs (newDci.m_mcs.at (j), RgbPerFlow * rbgSize) / 8);   //transport block for this flow 
                  NS_LOG_INFO (this << " LCID " << (uint32_t) newRlcEl.m_logicalChannelIdentity << " size " << newRlcEl.m_size << " layer " << (uint16_t)j);
                  bytesTxed += newRlcEl.m_size ;
                  newRlcPduLe.push_back (newRlcEl);
                  UpdateDlRlcBufferInfo (newDci.m_rnti, newRlcEl.m_logicalChannelIdentity, newRlcEl.m_size);
                  //std::cout<<"\t Update DL RLC buffer , rnti = "<<(int)newDci.m_rnti<<" lcid = "<<(int)newRlcEl.m_logicalChannelIdentity<< " size = "<<(int)newRlcEl.m_size<<std::endl;
                  
                  if (m_harqOn == true)
                    {
                      // store RLC PDU list for HARQ
                      std::map <uint16_t, cv2x_DlHarqRlcPduListBuffer_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduListBuffer.find ((*itMap).first);
                      if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end ())
                        {
                          NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << (*itMap).first);
                        }
                      (*itRlcPdu).second.at (j).at (newDci.m_harqProcess).push_back (newRlcEl);
                    }
                } //end for 
              newEl.m_rlcPduList.push_back (newRlcPduLe);                  

            } // end if buffer 
        } // end for buffer 

      for (uint8_t j = 0; j < nLayer; j++)
        {
          newDci.m_ndi.push_back (1);
          newDci.m_rv.push_back (0);
        }

      newDci.m_tpc = 1; //1 is mapped to 0 in Accumulated Mode and to -1 in Absolute Mode
      newEl.m_dci = newDci;
      if (m_harqOn == true)
        {
          // store DCI for HARQ
          std::map <uint16_t, cv2x_DlHarqProcessesDciBuffer_t>::iterator itDci = m_dlHarqProcessesDciBuffer.find (newEl.m_rnti);
          if (itDci == m_dlHarqProcessesDciBuffer.end ())
            {
              NS_FATAL_ERROR ("Unable to find RNTI entry in DCI HARQ buffer for RNTI " << newEl.m_rnti);
            }
          (*itDci).second.at (newDci.m_harqProcess) = newDci;
          // refresh timer
          std::map <uint16_t, DlHarqProcessesTimer_t>::iterator itHarqTimer =  m_dlHarqProcessesTimer.find (newEl.m_rnti);
          if (itHarqTimer== m_dlHarqProcessesTimer.end ())
            {
              NS_FATAL_ERROR ("Unable to find HARQ timer for RNTI " << (uint16_t)newEl.m_rnti);
            }
          (*itHarqTimer).second.at (newDci.m_harqProcess) = 0;
        }
      // ...more parameters -> ingored in this version

      ret.m_buildDataList.push_back (newEl);
      itMap++;
    } // end while allocation

  ret.m_nrOfPdcchOfdmSymbols = 1;   /// \todo check correct value according the DCIs txed
 
  //print allocation map
  {
    uint16_t usedRBGs = 0;
    std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator allocMapRntiIt;
    for (allocMapRntiIt = allocationMapPerRntiPerLCId.begin() ; allocMapRntiIt != allocationMapPerRntiPerLCId.end(); allocMapRntiIt++) 
      {
        //uint16_t rnti = allocMapRntiIt->first;
        std::map <uint8_t, std::vector <uint16_t> >::iterator allocMapLcidIt;
        for (allocMapLcidIt = allocMapRntiIt->second.begin() ; allocMapLcidIt != allocMapRntiIt->second.end() ; allocMapLcidIt++) 
          {
            //uint8_t lcid = allocMapLcidIt->first;
            usedRBGs += allocMapLcidIt->second.size();
            //std::cout << Simulator::Now ().GetSeconds () << " ALLOC RNTI " << rnti << " LCID " << (uint16_t) lcid << " RBGs " << allocMapLcidIt->second.size() << std::endl;          
          }
      }
    //std::cout << Simulator::Now ().GetSeconds () << " TOTAL DL USED " << usedRBGs << std::endl;       
  }

  m_schedSapUser->SchedDlConfigInd (ret);
  return;
}

void
cv2x_RrFfMacScheduler::DoSchedDlRachInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedDlRachInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  
  m_rachList = params.m_rachList;

  return;
}

void
cv2x_RrFfMacScheduler::DoSchedDlCqiInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  std::map <uint16_t,uint8_t>::iterator it;
  for (unsigned int i = 0; i < params.m_cqiList.size (); i++)
    {
      if ( params.m_cqiList.at (i).m_cqiType == cv2x_CqiListElement_s::P10 )
        {
          NS_LOG_LOGIC ("wideband CQI " <<  (uint32_t) params.m_cqiList.at (i).m_wbCqi.at (0) << " reported");
          std::map <uint16_t,uint8_t>::iterator it;
          uint16_t rnti = params.m_cqiList.at (i).m_rnti;
          it = m_p10CqiRxed.find (rnti);
          if (it == m_p10CqiRxed.end ())
            {
              // create the new entry
              m_p10CqiRxed.insert ( std::pair<uint16_t, uint8_t > (rnti, params.m_cqiList.at (i).m_wbCqi.at (0)) ); // only codeword 0 at this stage (SISO)
              // generate correspondent timer
              m_p10CqiTimers.insert ( std::pair<uint16_t, uint32_t > (rnti, m_cqiTimersThreshold));
            }
          else
            {
              // update the CQI value
              (*it).second = params.m_cqiList.at (i).m_wbCqi.at (0);
              // update correspondent timer
              std::map <uint16_t,uint32_t>::iterator itTimers;
              itTimers = m_p10CqiTimers.find (rnti);
              (*itTimers).second = m_cqiTimersThreshold;
            }
        }
      else if ( params.m_cqiList.at (i).m_cqiType == cv2x_CqiListElement_s::A30 )
        {
          // subband CQI reporting high layer configured
          // Not used by RR Scheduler
        }
      else
        {
          NS_LOG_ERROR (this << " CQI type unknown");
        }
    }

  return;
}

double
cv2x_RrFfMacScheduler::EstimateUlSinr (uint16_t rnti, uint16_t rb)
{
  std::map <uint16_t, std::vector <double> >::iterator itCqi = m_ueCqi.find (rnti);
  if (itCqi == m_ueCqi.end ())
    {
      // no cqi info about this UE
      return (NO_SINR);
    }
  else
    {
      // take the average SINR value among the available
      double sinrSum = 0;
      unsigned int sinrNum = 0;
      for (uint32_t i = 0; i < m_cschedCellConfig.m_ulBandwidth; i++)
        {
          double sinr = (*itCqi).second.at (i);
          if (sinr != NO_SINR)
            {
              sinrSum += sinr;
              sinrNum++;
            }
        }
      double estimatedSinr;
      //Changed
      if (sinrNum != 0) 
      	estimatedSinr=(sinrSum / sinrNum);
  	  else
  	 	estimatedSinr=DBL_MAX;
      // store the value
      (*itCqi).second.at (rb) = estimatedSinr;
      return (estimatedSinr);
    }
}

void
cv2x_RrFfMacScheduler::DoSchedUlTriggerReq (const struct cv2x_FfMacSchedSapProvider::SchedUlTriggerReqParameters& params)
{
  //std::cout<<" ENTER cv2x_RrFfMacScheduler::DoSchedUlTriggerReq"<<std::endl;
  //Make sure we reset the allocation map in case it is present. This can happen if CQI are reported
  //by PUSCH and the eNode B did not receive any packet from the UE (due toloss for example)
  std::map <uint16_t, std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > >::iterator itMap = m_allocationMaps.find (params.m_sfnSf);
  if (itMap != m_allocationMaps.end ()) 
   {
    m_allocationMaps.erase (itMap);
   }

  uint16_t rbAllocatedNum = 0;
  NS_LOG_FUNCTION (this << " UL - Frame no. " << (params.m_sfnSf >> 4) << " subframe no. " << (0xF & params.m_sfnSf) << " size " << params.m_ulInfoList.size ());

  RefreshUlCqiMaps ();
  uint16_t rbNum= m_cschedCellConfig.m_ulBandwidth;
  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > allocationMapPerRntiPerLCId;     //allocation map for the UL scheduler : per UE per LCID 
  // Generate RBs map
  cv2x_FfMacSchedSapUser::SchedUlConfigIndParameters ret;
  std::vector <bool> rbMap;
  std::set <uint16_t> rntiAllocated;
  m_rachAllocationMap.clear ();
  m_rachAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);

  rbMap.resize (m_cschedCellConfig.m_ulBandwidth, false);
  // remove RACH allocation
  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itTemp1 = allocationMapPerRntiPerLCId.begin ();

  //fill the rbmap if there is some RBGs assigned to HARQ 
  while (itTemp1 != allocationMapPerRntiPerLCId.end ())
   {
      std::map <uint8_t, std::vector <uint16_t> > mapFlow=itTemp1->second;
      std::map <uint8_t, std::vector <uint16_t> >::iterator itTemp2= mapFlow.begin();
      while (itTemp2!=mapFlow.end())
       {
         for (unsigned int j=0; j< (*itTemp2).second.size () ;j++) 
           {
             rbMap.at ((*itTemp2).second.at(j)) = true;
           }
          itTemp2++;
        }
      itTemp1++;
    }
 
   if (m_harqOn == true)
    {
      //   Process UL HARQ feedback
      for (uint16_t i = 0; i < params.m_ulInfoList.size (); i++)
        {
          if (params.m_ulInfoList.at (i).m_receptionStatus == cv2x_UlInfoListElement_s::NotOk)
            {
              // retx correspondent block: retrieve the UL-DCI
              uint16_t rnti = params.m_ulInfoList.at (i).m_rnti;
              std::map <uint16_t, uint8_t>::iterator itProcId = m_ulHarqCurrentProcessId.find (rnti);
              if (itProcId == m_ulHarqCurrentProcessId.end ())
                {
                  NS_LOG_ERROR ("No info find in HARQ buffer for UE (might change eNB) " << rnti);
                }
              uint8_t harqId = (uint8_t)((*itProcId).second - HARQ_PERIOD) % HARQ_PROC_NUM;
              NS_LOG_INFO (this << " UL-HARQ retx RNTI " << rnti << " harqId " << (uint16_t)harqId << " i " << i << " size "  << params.m_ulInfoList.size ());
              std::map <uint16_t, cv2x_UlHarqProcessesDciBuffer_t>::iterator itHarq = m_ulHarqProcessesDciBuffer.find (rnti);
              if (itHarq == m_ulHarqProcessesDciBuffer.end ())
                {
                  NS_LOG_ERROR ("No info find in HARQ buffer for UE (might change eNB) " << rnti);
                  continue;
                }
              cv2x_UlDciListElement_s dci = (*itHarq).second.at (harqId);
              std::map <uint16_t, cv2x_UlHarqProcessesStatus_t>::iterator itStat = m_ulHarqProcessesStatus.find (rnti);
              if (itStat == m_ulHarqProcessesStatus.end ())
                {
                  NS_LOG_ERROR ("No info find in HARQ buffer for UE (might change eNB) " << rnti);
                }
              if ((*itStat).second.at (harqId) >= 3)
                {
                  NS_LOG_INFO ("Max number of retransmissions reached (UL)-> drop process");
                  continue;
                }
              bool free = true;

              for (int j = dci.m_rbStart; j < dci.m_rbStart + dci.m_rbLen; j++)
                {
                  if (rbMap.at (j) == true)
                    {
                      free = false;
                      NS_LOG_INFO (this << " BUSY " << j);
                    }
                }
              if (free)
                {
                  // retx on the same RBs
                  for (int j = dci.m_rbStart; j < dci.m_rbStart + dci.m_rbLen; j++)
                    {
                      std::vector <uint16_t> tempV;
                      rbMap.at (j) = true;
                      std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itMap;
                      itMap = allocationMapPerRntiPerLCId.find (dci.m_rnti);
                      if (itMap == allocationMapPerRntiPerLCId.end ())
                       {        
                         // insert new element
                         std::map<uint8_t, std::vector <uint16_t> > tempMap;   
                         tempV.push_back(j);
                         tempMap.insert (std::pair<uint8_t, std::vector <uint16_t> > (4, tempV));
                         allocationMapPerRntiPerLCId.insert (std::pair <uint16_t, std::map<uint8_t, std::vector <uint16_t> > > (dci.m_rnti, tempMap));                     
                         tempV.clear();
                       }
                      else
                       {
                         std::map <uint8_t, std::vector <uint16_t> >::iterator itSMap;
                         itSMap=itMap->second.find (4);
                         if (itSMap == itMap->second.end ())
                          {
                            //insert new flow id element 
                            tempV.push_back(j);  
                            itMap->second.insert (std::pair<uint8_t, std::vector <uint16_t> > (4, tempV));
                            tempV.clear();
                          }
                         else 
                          {
                            (*itSMap).second.push_back (j);
                          }
                        }
                       
                      NS_LOG_INFO ("\tRB " << j);
                      rbAllocatedNum++;
                    }
                  NS_LOG_INFO (this << " Send retx in the same RBs " << (uint16_t)dci.m_rbStart << " to " << dci.m_rbStart + dci.m_rbLen << " RV " << (*itStat).second.at (harqId) + 1);
                }
              else
                {
                  NS_LOG_INFO ("Cannot allocate retx due to RACH allocations for UE " << rnti);
                  continue;
                }
              dci.m_ndi = 0;
              // Update HARQ buffers with new HarqId
              (*itStat).second.at ((*itProcId).second) = (*itStat).second.at (harqId) + 1;
              (*itStat).second.at (harqId) = 0;
              (*itHarq).second.at ((*itProcId).second) = dci;
              ret.m_dciList.push_back (dci);
              rntiAllocated.insert (dci.m_rnti);
            }
          else
            {
              NS_LOG_INFO (this << " HARQ-ACK feedback from RNTI " << params.m_ulInfoList.at (i).m_rnti);
            }
        }
    }

  int nflows = 0;
  std::multimap <uint16_t, std::map <uint8_t, uint32_t> >::iterator it;
  for (it = m_ceBsrRxed.begin (); it != m_ceBsrRxed.end (); it++)
    {
      std::set <uint16_t>::iterator itRnti = rntiAllocated.find ((*it).first);
      // select UEs with queues not empty and not yet allocated for HARQ
      std::map <uint8_t, uint32_t> mapLC=it->second;
      std::map <uint8_t, uint32_t>::iterator itLC;
      for (itLC=mapLC.begin(); itLC!=mapLC.end(); itLC++) 
       {  
         if (((*itLC).second > 0)&&(itRnti == rntiAllocated.end ()))
          {
            nflows++;
          }
       }
   }

  if (nflows == 0)
    {
      //print allocation map
      {
        uint32_t totalRbs = 0;
        std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator allocMapRntiIt;
        for (allocMapRntiIt = allocationMapPerRntiPerLCId.begin() ; allocMapRntiIt != allocationMapPerRntiPerLCId.end(); allocMapRntiIt++) 
          {
            uint16_t rnti = allocMapRntiIt->first;
            std::map <uint8_t, std::vector <uint16_t> >::iterator allocMapLcidIt;
            for (allocMapLcidIt = allocMapRntiIt->second.begin() ; allocMapLcidIt != allocMapRntiIt->second.end() ; allocMapLcidIt++) 
              {
                uint8_t lcid = allocMapLcidIt->first;
                NS_LOG_DEBUG ("ALLOC RNTI " << rnti << " LCID " << (uint16_t) lcid << " RBs " << allocMapLcidIt->second.size());   
                totalRbs+= allocMapLcidIt->second.size();
              }
          }
        //std::cout << Simulator::Now ().GetSeconds () << " TOTAL UL USED " << totalRbs << std::endl;   
      }

      if (ret.m_dciList.size () > 0)
        {
          m_allocationMaps.insert (std::pair <uint16_t, std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > > (params.m_sfnSf, allocationMapPerRntiPerLCId));
          m_schedSapUser->SchedUlConfigInd (ret);
        }

      return;  // no flows to be scheduled
    }

  //printf BSR buffer status
  {
    std::multimap <uint16_t, std::map <uint8_t, uint32_t> >::iterator bsrIt;
    for (bsrIt = m_ceBsrRxed.begin() ; bsrIt != m_ceBsrRxed.end () ; bsrIt++)
      {
        uint16_t rnti = bsrIt->first;
        std::map <uint8_t, uint32_t> copyBsrRnti = bsrIt->second;
        std::map <uint8_t, uint32_t>::iterator lcidIt;
        for (lcidIt = copyBsrRnti.begin() ; lcidIt != copyBsrRnti.end() ; lcidIt++)
          {
            NS_LOG_DEBUG ("BSR status: RNTI " << rnti << " LCID " << (uint16_t) lcidIt->first << " BUFFER " << lcidIt->second);
            //std::cout<<"\t BSR status: RNTI " << (int) rnti << " LCID " << (int)lcidIt->first << " BUFFER " << (int)lcidIt->second<<std::endl;
          }
      }
  }

 if (rbAllocatedNum == rbNum)
  {
    // all the RBs are already allocated -> exit
    if (ret.m_dciList.size () > 0)
     {
       m_allocationMaps.insert (std::pair <uint16_t, std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > > (params.m_sfnSf, allocationMapPerRntiPerLCId));
       m_schedSapUser->SchedUlConfigInd (ret);
     }
    return;
  }

  std::vector <bool> rbMap1=rbMap;  
  std::vector <cv2x_LteFlowId_t> satisfiedFlows;       // structure to save satisfied flows 
  double bytesTxedF = 0;                          // accumulated data for each flows            

  std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itUl = m_ceBsrRxed.begin() ;

  if ( m_nextRntiUl == 0 )
   {
     bool findLc = false;
     for ( itUl= m_ceBsrRxed.begin(); itUl != m_ceBsrRxed.end(); itUl ++ )
      {
        if ( (*itUl).first != 0 ) 
         {
            m_nextRntiUl = (*itUl).first;
            std::map <uint8_t, uint32_t>::iterator itLC = (*itUl).second.begin();
            for ( itLC = (*itUl).second.begin(); itLC != (*itUl).second.end(); itLC ++)
             {
                if ( itLC ->second > 0 ) 
                 {
                    //std::cout<<" lc = "<<(int)(*itLC).first<<" data = " <<(int)itLC ->second <<std::endl;
                    m_nextLcidUl = (*itLC).first;
                    findLc= true;
                    break;
                 }
             }
         }
        if ( findLc == true )
         break;
      }
   }

  if ( m_nextRntiUl == 0 ) 
   {
     m_schedSapUser->SchedUlConfigInd (ret);
     return;
   }

  //compute intial allocated Rbs for each flow 

  std::map< cv2x_LteFlowId_t, uint32_t> InitialAllocatedRbs;

   std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator it1;
   for ( it1= allocationMapPerRntiPerLCId.begin(); it1 != allocationMapPerRntiPerLCId.end(); it1 ++)
    {
         std::map <uint8_t, std::vector <uint16_t> > maptemp = it1->second;
         std::map <uint8_t, std::vector <uint16_t> >::iterator it2;
         for ( it2= maptemp.begin(); it2 != maptemp.end(); it2 ++)
          {
             cv2x_LteFlowId_t flow = cv2x_LteFlowId_t (it1->first, it2->first);
             InitialAllocatedRbs.insert ( std::pair <cv2x_LteFlowId_t, uint32_t>  (flow, it2->second.size() ));
          }
    }
  
  //std::cout<<" \t m_nextRntiUl = "<<(int)m_nextRntiUl<<" m_nextLcidUl = "<<(int)m_nextLcidUl<<std::endl;
  std::vector <uint16_t> tempV;
  int i=0;
  // start the allocation procedure 
  while (i<rbNum) 
   {
     NS_LOG_INFO (this << " ALLOCATION for RB " << i << " of " << rbNum);
     //std::cout<<"\t ALLOCATION for RB " << (int)i << " of " << (int)rbNum<<std::endl;
     if (rbMap.at (i) == false)
      {
        std::map <cv2x_LteFlowId_t,struct cv2x_LogicalChannelConfigListElement_s>:: iterator itLogicalChannels;
        std::map <cv2x_LteFlowId_t,struct cv2x_LogicalChannelConfigListElement_s>:: iterator itMax = m_ueLogicalChannelsConfigList.end ();
        
        cv2x_LteFlowId_t m_next_Ul_flow = cv2x_LteFlowId_t (m_nextRntiUl, m_nextLcidUl);
        itLogicalChannels = m_ueLogicalChannelsConfigList.find (m_next_Ul_flow);
        if ( itLogicalChannels == m_ueLogicalChannelsConfigList.end()) 
          itLogicalChannels = m_ueLogicalChannelsConfigList.begin();
        do 
         {
           cv2x_LteFlowId_t flowId1 ;
           flowId1.m_rnti= itLogicalChannels->first.m_rnti;
           flowId1.m_lcId= itLogicalChannels->second.m_logicalChannelIdentity;
           std::vector <cv2x_LteFlowId_t>::iterator Ft;
           bool findF=false;
           // check if this flow is already in the satisfied flows 
           for(Ft=satisfiedFlows.begin();Ft!=satisfiedFlows.end();Ft++)
            {
              if(((*Ft).m_rnti==flowId1.m_rnti) && ((*Ft).m_lcId==flowId1.m_lcId))
               {
                 findF=true;
                 break;
               }
            }
           if (findF==false)
            { 
              //std::cout<<" rnti = "<<(int)itLogicalChannels->first.m_rnti<<" lcid = "<<(int)itLogicalChannels->second.m_logicalChannelIdentity<<" in not satisfied "<<std::endl;
              // this flows doesn't exist in the satisfied flows --> needs more RBs to transmit           
              // try to find this flow in the m_ceBsrRxed
              std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator it;     
              it=m_ceBsrRxed.find(itLogicalChannels->first.m_rnti);
              if (it!=m_ceBsrRxed.end())
               {
                 std::map <uint8_t, uint32_t> mapLC=it->second;
                 std::map <uint8_t, uint32_t>::iterator itLC=mapLC.find(itLogicalChannels->second.m_logicalChannelIdentity);
                 if (itLC!=mapLC.end())      
                  {
                    if ((*itLC).second > 0)
                     {
                        itMax = itLogicalChannels;
                     } 
                    else
                     {
                        //std::cout<<"\t this flow not having data "<<std::endl;
                        itLC ++;
                        if ( itLC == mapLC.end())
                         {
                           it ++;
                           if ( it == m_ceBsrRxed.end() ) 
                             it = m_ceBsrRxed.begin();
                     
                           itLC = it->second.begin();
                         }
                        m_nextRntiUl = it->first;
                        m_nextLcidUl = itLC->first; 
                        
                        m_next_Ul_flow = cv2x_LteFlowId_t (m_nextRntiUl, m_nextLcidUl);   // added ! 
                        itLogicalChannels= m_ueLogicalChannelsConfigList.find(m_next_Ul_flow); // added ! 
                       continue;
                      }
                  } 
                } 
            }  //end findF==false
           itLogicalChannels ++;
           if ( itLogicalChannels == m_ueLogicalChannelsConfigList.end ()) 
               itLogicalChannels = m_ueLogicalChannelsConfigList.begin ();   

         } //end for itLogicalchannels
       while ( (itLogicalChannels->first.m_rnti ==  m_next_Ul_flow.m_rnti) && ( itLogicalChannels->second.m_logicalChannelIdentity == m_next_Ul_flow.m_lcId) );

       if (itMax == m_ueLogicalChannelsConfigList.end ())
        {
          // no UE available for this RB
          NS_LOG_INFO (this << " any UE found");
        }
       else
        {
          //std::cout<<"\t selected flow , rnti = "<<(int)itMax->first.m_rnti<<" lcid = "<<(int)itMax->second.m_logicalChannelIdentity<<std::endl;
          std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itMap;
          itMap = allocationMapPerRntiPerLCId.find ((uint16_t)(itMax->first.m_rnti));
            
          if (itMap == allocationMapPerRntiPerLCId.end ())
           {
             // first time allocation for this flow 
             //std::cout<<"\t first time allocation "<<std::endl;
             std::map<uint8_t, std::vector <uint16_t> > tempMap;
                    
             tempV.push_back(i);
             tempMap.insert (std::pair<uint8_t, std::vector <uint16_t> > (itMax->second.m_logicalChannelIdentity, tempV));
             allocationMapPerRntiPerLCId.insert (std::pair <uint16_t, std::map<uint8_t, std::vector <uint16_t> > > (itMax->first.m_rnti, tempMap));
             rbMap.at (i) = true; 
             tempV.clear();
             /************ trying to see if this flow is satisfied by this first allocation   ***************************/ 
                std::map <uint16_t, std::vector <double> >::iterator itCqi = m_ueCqi.find (itMax->first.m_rnti);  
                int cqi = 0;
                int mcsF=0;
                if (itCqi == m_ueCqi.end ())
                 {
                   cqi =1 ;
                 }
                else 
                 {
                    double maxSinr = 0;
                    for (unsigned int i = 0; i < (*itCqi).second.size(); i++)
                     {
                       if ((*itCqi).second.at (i) > maxSinr)
                        {
                          maxSinr = (*itCqi).second.at (i);
                        }
                     }
                   double s = log2 ( 1 + (
                   std::pow (10, maxSinr / 10 )  /
                          ( (-std::log (5.0 * 0.00005 )) / 1.5) ));

                   cqi = m_amc->GetCqiFromSpectralEfficiency (s);
                 }

                if (cqi > 0 ) // CQI == 0 means "out of range" (see table 7.2.3-1 of 36.213)
                 {
                   std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itUl;
                   itUl=m_ceBsrRxed.find(itMax->first.m_rnti);
                   std::map <uint8_t, uint32_t> mapUl=itUl->second;
                   std::map <uint8_t, uint32_t>:: iterator itmapUl;
                   int dataUl=0;
                   for (itmapUl=mapUl.begin(); itmapUl!=mapUl.end(); itmapUl++)
                    {
                      dataUl=dataUl+(*itmapUl).second;
                    }
                   if (dataUl > 0)             
                    {                          
                      // this UE has data to transmit   
                      mcsF = m_amc->GetMcsFromCqi (cqi);
                      //std::cout<<" MCS: "<<(int)mcsF<<std::endl;
                    }                       
                 }    
                bytesTxedF=(m_amc->GetUlTbSizeFromMcs (mcsF, 1) / 8) ; 
                
                cv2x_LteFlowId_t flowIdF = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->second.m_logicalChannelIdentity);
                // try to find this flow in the m_ceBsrRxed
                std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itTemp;
                itTemp=m_ceBsrRxed.find(itMax->first.m_rnti);  
                std::map <uint8_t, uint32_t> mapLCTemp=itTemp->second;
                std::map <uint8_t, uint32_t>::iterator itLCTemp=mapLCTemp.find(itMax->second.m_logicalChannelIdentity);
                //std::cout<<" rnti = "<<(int) itMax->first.m_rnti<<" lcid = "<<(int)itMax->second.m_logicalChannelIdentity<<" mcs = "<<(int) mcsF<<" data to send = "<<(int)(*itLCTemp).second<<" allcated data = "<<(int)bytesTxedF<<std::endl;

                if ((*itLCTemp).second <= bytesTxedF )
                 {
                   //std::cout<<"\t --> flow satisfied "<<std::endl;
                   //this flow is  satisfied with this new allocation --> add it to satisfied flows 
                   satisfiedFlows.push_back(flowIdF);

                   std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itLogicalChannels2;
                   itLogicalChannels2 = m_ceBsrRxed.find(itMax->first.m_rnti);
                   std::map <uint8_t, uint32_t> mapLc = itLogicalChannels2->second;
                   std::map <uint8_t, uint32_t>:: iterator itMapLc = mapLc.find(itMax->second.m_logicalChannelIdentity);
                   
                   itMapLc ++;
                   if ( itMapLc == mapLc.end())
                    {
                      itLogicalChannels2 ++;
                      if ( itLogicalChannels2 == m_ceBsrRxed.end() ) 
                         itLogicalChannels2 = m_ceBsrRxed.begin();
                     
                      itMapLc = itLogicalChannels2->second.begin();
                    }

                   m_nextRntiUl = itLogicalChannels2->first;
                   m_nextLcidUl = itMapLc->first;
                 }
                else if ( i == (rbNum -1))
                 {
                   // flow still not satisfied --> mark it as the next flow 
                   m_nextRntiUl = itMax->first.m_rnti;
                   m_nextLcidUl = itMax->second.m_logicalChannelIdentity;
                 }
           }
          else
           {
             // rnti already exist in the allocation map, check the lcid 
             std::set <uint16_t>::iterator itCheckHarq = rntiAllocated.find( itMax->first.m_rnti);
             if ( itCheckHarq != rntiAllocated.end () ) // check if rnti is already allocated for UL HARQ 
              {
                  std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itLogicalChannels2;
                  itLogicalChannels2 = m_ceBsrRxed.find(itMax->first.m_rnti);
                  std::map <uint8_t, uint32_t> mapLc = itLogicalChannels2->second;
                  std::map <uint8_t, uint32_t>:: iterator itMapLc ;
                   
                  itLogicalChannels2 ++;
                  if ( itLogicalChannels2 == m_ceBsrRxed.end() ) 
                     itLogicalChannels2 = m_ceBsrRxed.begin();
                     
                  itMapLc = itLogicalChannels2->second.begin();

                  m_nextRntiUl = itLogicalChannels2->first;
                  m_nextLcidUl = itMapLc->first;
              }
             else 
              {
             std::map <uint8_t, std::vector <uint16_t> >::iterator itSMap;
             itSMap=itMap->second.find ((uint8_t)itMax->second.m_logicalChannelIdentity);
             if (itSMap == itMap->second.end ())
              {
                //std::cout<<"\t first time allocation for this lcid "<<std::endl;
                //insert new lc id element 
                tempV.push_back(i);    
                itMap->second.insert (std::pair<uint8_t, std::vector <uint16_t> > (itMax->second.m_logicalChannelIdentity, tempV));
                rbMap.at (i) = true; 
                tempV.clear();
                /************ trying to see if this flow is satisfied by this first allocation   ***************************/ 
                std::map <uint16_t, std::vector <double> >::iterator itCqi = m_ueCqi.find (itMax->first.m_rnti);  
                int cqi = 0;
                int mcsF=0;
                if (itCqi == m_ueCqi.end ())
                 {
                   cqi =1 ;
                 }
                else 
                 {
                    // take the lowest CQI value (worst RB)
                    double maxSinr = 0;
                    for (unsigned int i = 0; i < (*itCqi).second.size(); i++)
                     {
                       if ((*itCqi).second.at (i) > maxSinr)
                        {
                          maxSinr = (*itCqi).second.at (i);
                        }
                     }
                   /*****************************************************************/
                   double s = log2 ( 1 + (
                   std::pow (10, maxSinr / 10 )  /
                          ( (-std::log (5.0 * 0.00005 )) / 1.5) ));

                   cqi = m_amc->GetCqiFromSpectralEfficiency (s);
                 }

                if (cqi > 0 ) // CQI == 0 means "out of range" (see table 7.2.3-1 of 36.213)
                 {
                   std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itUl;
                   itUl=m_ceBsrRxed.find(itMax->first.m_rnti);
                   std::map <uint8_t, uint32_t> mapUl=itUl->second;
                   std::map <uint8_t, uint32_t>:: iterator itmapUl;
                   int dataUl=0;
                   for (itmapUl=mapUl.begin(); itmapUl!=mapUl.end(); itmapUl++)
                    {
                      dataUl=dataUl+(*itmapUl).second;
                    }
                   if (dataUl > 0)             
                    {                          
                      // this UE has data to transmit   
                      mcsF = m_amc->GetMcsFromCqi (cqi);
                      //std::cout<<" MCS: "<<(int)mcsF<<std::endl;
                    }                       
                 }    
                bytesTxedF=(m_amc->GetUlTbSizeFromMcs (mcsF, 1) / 8) ; 
                cv2x_LteFlowId_t flowIdF = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->second.m_logicalChannelIdentity);
                // try to find this flow in the m_ceBsrRxed
                std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itTemp;
                itTemp=m_ceBsrRxed.find(itMax->first.m_rnti);  
                std::map <uint8_t, uint32_t> mapLCTemp=itTemp->second;
                std::map <uint8_t, uint32_t>::iterator itLCTemp=mapLCTemp.find(itMax->second.m_logicalChannelIdentity);

                 //std::cout<<" rnti = "<<(int) itMax->first.m_rnti<<" lcid = "<<(int)itMax->second.m_logicalChannelIdentity<<" mcs = "<<(int) mcsF<<" data to send = "<<(int)(*itLCTemp).second<<" allcated data = "<<(int)bytesTxedF<<std::endl;

                if ((*itLCTemp).second <= bytesTxedF )
                 {
                   //this flow is  satisfied with this new allocation --> add it to satisfied flows 
                   //std::cout<<"\t --> flow satisfied "<<std::endl;
                   satisfiedFlows.push_back(flowIdF);

                   std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itLogicalChannels2;
                   itLogicalChannels2 = m_ceBsrRxed.find(itMax->first.m_rnti);
                   std::map <uint8_t, uint32_t> mapLc = itLogicalChannels2->second;
                   std::map <uint8_t, uint32_t>:: iterator itMapLc = mapLc.find(itMax->second.m_logicalChannelIdentity);
                   
                   
                   itMapLc ++;
                   if ( itMapLc == mapLc.end())
                    {
                      itLogicalChannels2 ++;
                      if ( itLogicalChannels2 == m_ceBsrRxed.end() ) 
                         itLogicalChannels2 = m_ceBsrRxed.begin();
                     
                      itMapLc = itLogicalChannels2->second.begin();
                      
                    }

                   m_nextRntiUl = itLogicalChannels2->first;
                   m_nextLcidUl = itMapLc->first;
                 }
                else if ( i == (rbNum -1))
                 {
                   // flow still not satisfied --> mark it as the next flow 
                   m_nextRntiUl = itMax->first.m_rnti;
                   m_nextLcidUl = itMax->second.m_logicalChannelIdentity;
                 }
              }
             else 
              {
                // the lc id already exists --> check if it needs more RBs or not     
                //std::cout<<"\t need for addtional allocation for this lcid "<<std::endl; 
                cv2x_LteFlowId_t flowInitial = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->second.m_logicalChannelIdentity);
                std::map< cv2x_LteFlowId_t, uint32_t>::iterator itInittial= InitialAllocatedRbs.find( flowInitial );
                uint32_t initialRbAllocated= itInittial->second; 
                if ( initialRbAllocated > 0 )  // flow already allocsted in UL HARQ --> skip it 
                 {
                    std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itLogicalChannels2;
                    itLogicalChannels2 = m_ceBsrRxed.find(itMax->first.m_rnti);
                    std::map <uint8_t, uint32_t> mapLc = itLogicalChannels2->second;
                    std::map <uint8_t, uint32_t>:: iterator itMapLc ;
                   
                    itLogicalChannels2 ++;
                    if ( itLogicalChannels2 == m_ceBsrRxed.end() ) 
                       itLogicalChannels2 = m_ceBsrRxed.begin();
                     
                    itMapLc = itLogicalChannels2->second.begin();

                    m_nextRntiUl = itLogicalChannels2->first;
                    m_nextLcidUl = itMapLc->first;
                    
                 }
                else 
                 { 
                  //std::cout<<"\t this flow has initial rb allocated = "<<(int) initialRbAllocated<<std::endl;
                  int b=1;
                  b=b+(*itSMap).second.size()- initialRbAllocated;   // number of allocated RBs + 1 (the current RB) 
                  std::map <uint16_t, std::vector <double> >::iterator itCqi = m_ueCqi.find (itMax->first.m_rnti);  
                  int cqi = 0;
                  int mcsF=0;
                  if (itCqi == m_ueCqi.end ())
                  {
                    cqi =1 ;
                  }
                  else 
                  {
                      double maxSinr = 0;
                      for (unsigned int i = 0; i < (*itCqi).second.size(); i++)
                      {
                        if ((*itCqi).second.at (i) > maxSinr)
                          {
                            maxSinr = (*itCqi).second.at (i);
                          }
                      }
                    double s = log2 ( 1 + (
                    std::pow (10, maxSinr / 10 )  /
                            ( (-std::log (5.0 * 0.00005 )) / 1.5) ));

                    cqi = m_amc->GetCqiFromSpectralEfficiency (s);
                  }

                  if (cqi > 0 ) // CQI == 0 means "out of range" (see table 7.2.3-1 of 36.213)
                  {
                    std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itUl;
                    itUl=m_ceBsrRxed.find(itMax->first.m_rnti);
                    std::map <uint8_t, uint32_t> mapUl=itUl->second;
                    std::map <uint8_t, uint32_t>:: iterator itmapUl;
                    int dataUl=0;
                    for (itmapUl=mapUl.begin(); itmapUl!=mapUl.end(); itmapUl++)
                      {
                        dataUl=dataUl+(*itmapUl).second;
                      }
                    if (dataUl > 0)             
                      {                          
                        // this UE has data to transmit   
                        mcsF = m_amc->GetMcsFromCqi (cqi);
                        //std::cout<<" MCS: "<<(int)mcsF<<std::endl;
                      }                       
                  }    
                
                  bytesTxedF=(m_amc->GetUlTbSizeFromMcs (mcsF, b * 1) / 8) ; 
                  cv2x_LteFlowId_t flowIdF = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->second.m_logicalChannelIdentity);
                  // try to find this flow in the m_ceBsrRxed
                  std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itTemp;
                  itTemp=m_ceBsrRxed.find(itMax->first.m_rnti);  
                  std::map <uint8_t, uint32_t> mapLCTemp=itTemp->second;
                  std::map <uint8_t, uint32_t>::iterator itLCTemp=mapLCTemp.find(itMax->second.m_logicalChannelIdentity);
                  
                  //std::cout<<" rnti = "<<(int) itMax->first.m_rnti<<" lcid = "<<(int)itMax->second.m_logicalChannelIdentity<<" mcs = "<<(int) mcsF<<" data to send = "<<(int)(*itLCTemp).second<<" allcated data = "<<(int)bytesTxedF<<std::endl;

                  if ((*itLCTemp).second <= bytesTxedF )
                  {
                    (*itSMap).second.push_back (i);
                    rbMap.at (i) = true;
                
                    //this flow is  satisfied with this new allocation --> add it to satisfied flows 
                    std::vector <cv2x_LteFlowId_t>::iterator Ft;
                    bool findF2=false;
                    for(Ft=satisfiedFlows.begin();Ft!=satisfiedFlows.end();Ft++)
                      {
                        if(((*Ft).m_rnti==flowIdF.m_rnti) && ((*Ft).m_lcId==flowIdF.m_lcId))
                        {
                          findF2=true;
                          break;
                        }
                      }
                      if (findF2==false)
                      {
                        //std::cout<<"\t --> flow satisfied "<<std::endl;
                        satisfiedFlows.push_back(flowIdF);

                        std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itLogicalChannels2;
                        itLogicalChannels2 = m_ceBsrRxed.find(itMax->first.m_rnti);
                        std::map <uint8_t, uint32_t> mapLc = itLogicalChannels2->second;
                        std::map <uint8_t, uint32_t>:: iterator itMapLc = mapLc.find(itMax->second.m_logicalChannelIdentity);
                    
                    
                        itMapLc ++;
                        if ( itMapLc == mapLc.end())
                          {
                            itLogicalChannels2 ++;
                            if ( itLogicalChannels2 == m_ceBsrRxed.end() ) 
                              itLogicalChannels2 = m_ceBsrRxed.begin();
                      
                            itMapLc = itLogicalChannels2->second.begin();
                        
                          }

                        m_nextRntiUl = itLogicalChannels2->first;
                        m_nextLcidUl = itMapLc->first;
                      }
                    }
                    else 
                    {
                            
                      (*itSMap).second.push_back (i);
                      rbMap.at (i) = true;
                      if ( i == (rbNum -1))
                        {
                          // flow still not satisfied --> mark it as the next flow 
                          //calculate how many RBGs are allocated for this flow 
                          int nRbg = (*itSMap).second.size() ;
                          if ( nRbg == rbNum)
                            {
                              std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itLogicalChannels2;
                              itLogicalChannels2 = m_ceBsrRxed.find(itMax->first.m_rnti);
                              std::map <uint8_t, uint32_t> mapLc = itLogicalChannels2->second;
                              std::map <uint8_t, uint32_t>:: iterator itMapLc = mapLc.find(itMax->second.m_logicalChannelIdentity);
                    
                              itMapLc ++;
                              if ( itMapLc == mapLc.end())
                                {
                                  itLogicalChannels2 ++;
                                  if ( itLogicalChannels2 == m_ceBsrRxed.end() ) 
                                    itLogicalChannels2 = m_ceBsrRxed.begin();
                      
                                  itMapLc = itLogicalChannels2->second.begin();
                        
                                }

                              m_nextRntiUl = itLogicalChannels2->first;
                              m_nextLcidUl = itMapLc->first;
                            }
                          else
                            {
                              m_nextRntiUl = itMax->first.m_rnti;
                              m_nextLcidUl = itMax->first.m_lcId;
                            }
                      }
                    }
                  }   // flow already allocated in UL HARQ 
                 }   //end if itSMap == itMap->second.end ()
                }  // end if check if rnti already allocated for  UL HARQ 
               }  //end if itMap == allocationMapPerRntiPerLCId.end ()
                NS_LOG_INFO (this << " UE assigned " << (uint16_t)(itLogicalChannels->first.m_rnti));
            }
        } // end for RBG free
     i++;
   } // end while i < rbgNum 


  // reorganize the allocation map to be contiguous allocated for each UE 
  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > CopyAllocatedMap;
  CopyAllocatedMap.insert ( allocationMapPerRntiPerLCId.begin(), allocationMapPerRntiPerLCId.end());
  allocationMapPerRntiPerLCId.clear ();
  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itCopy2 = CopyAllocatedMap.begin ();
  unsigned int comp=0;
  while (itCopy2 != CopyAllocatedMap.end ())
   {
      std::map <uint8_t, std::vector <uint16_t> > mapFlow=itCopy2->second;
      std::map <uint8_t, std::vector <uint16_t> >::iterator itCopy3= mapFlow.begin();
      
      while (itCopy3!=mapFlow.end())
       {
          std::map<uint8_t, std::vector <uint16_t> > tempMap; 
          std::vector <uint16_t> tempV;
          for (unsigned int j=0; j< (*itCopy3).second.size ();j++,comp++) 
           {
              // insert new element   
              tempV.push_back(comp);
           }
          std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itFind = allocationMapPerRntiPerLCId.find((*itCopy2).first);
          if ( itFind == allocationMapPerRntiPerLCId.end()) 
           {
             tempMap.insert (std::pair<uint8_t, std::vector <uint16_t> > (itCopy3->first, tempV));
             allocationMapPerRntiPerLCId.insert (std::pair <uint16_t, std::map<uint8_t, std::vector <uint16_t> > > (itCopy2->first, tempMap));
           }
          else
           {
             //rnti already in the map 
             itFind->second.insert (std::pair<uint8_t, std::vector <uint16_t> > (itCopy3->first, tempV));       
           }
          tempV.clear();
          itCopy3++;
       }
      itCopy2++;
    }             

  uint16_t rbAllocated = 0;
  //std::multimap <uint16_t, std::map <uint8_t, uint32_t> >::iterator it;
  //int nflows = 0;
  std::vector<uint16_t> RntiAssigned;

   if (m_nextRntiUl != 0)
    {
      for (it = m_ceBsrRxed.begin (); it !=m_ceBsrRxed.end (); it++)
        {
          if ((*it).first == m_nextRntiUl)
            {
              break;
            }
        }
      if (it == m_ceBsrRxed.end())
        {
          NS_LOG_ERROR (this << " no user found");
        }
    }
  else
    {
      it =  m_ceBsrRxed.begin ();
      m_nextRntiUl = (*it).first;
    }

    uint16_t rbRnti=0;
    std::vector<uint16_t> UETreated;
   do
    {
      std::set <uint16_t>::iterator itRnti = rntiAllocated.find ((*it).first);  
      
      std::map <uint8_t, uint32_t> mapLC=it->second;
      std::map <uint8_t, uint32_t>::iterator itLC;
      double data=0;
      for (itLC=mapLC.begin(); itLC!=mapLC.end();itLC++)
       {
         data=data+(*itLC).second;   
       }
      //std::cout<<" \t RNTI : "<< (*it).first<< " data to send = "<<(int) data <<std::endl;
      // RBs per RNTI :
      rbRnti=0;
      std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itTmp1 = allocationMapPerRntiPerLCId.find ((*it).first);
      if (itTmp1!=allocationMapPerRntiPerLCId.end())
       {
        std::map <uint8_t, std::vector <uint16_t> > mapFlow1=itTmp1->second;
        std::map <uint8_t, std::vector <uint16_t> >::iterator itTmp2;
        for (itTmp2=mapFlow1.begin(); itTmp2!=mapFlow1.end(); itTmp2++)
         {
           // check already allocated rbs 
           cv2x_LteFlowId_t flowInitial = cv2x_LteFlowId_t ((*it).first, (*itTmp2).first);
           std::map< cv2x_LteFlowId_t, uint32_t>::iterator itInittial= InitialAllocatedRbs.find( flowInitial );
           uint32_t initialRbAllocated= itInittial->second;
           rbRnti=rbRnti+ (*itTmp2).second.size() - initialRbAllocated;
         }
       }

      
       
       //std::cout<<" \t RNTI : "<< (*it).first<< " rb allocated = "<<(int)rbRnti<<" data to send = "<<(int) data <<std::endl;
     
      bool findUETreated=false;
      for (unsigned int k = 0; k < UETreated.size(); k++)
       {
          if ( UETreated.at(k)==(*it).first)
             findUETreated=true;    
       }

      if ( (rbRnti > 0) && (findUETreated==true) )
        {
           //std::cout<<" findUETreated==true --> exit loop "<<std::endl;
           rbRnti = 0;
           m_nextRntiUl = (*it).first;
           continue;
        }
      if ((itRnti != rntiAllocated.end ())||(data == 0) || (rbRnti==0))
       {
         // UE already allocated for UL-HARQ -> skip it
         it++;
         if (it == m_ceBsrRxed.end ())
          {
            
            // restart from the first
            it = m_ceBsrRxed.begin ();
            //std::cout<<" reach bsrEnd , first bsr begin = "<<(int)(*it).first<<std::endl;
          }
         //std::cout<<" \t m_nextRntiUl = "<<(int)m_nextRntiUl<<" it rnti = "<<(int)(*it).first<<std::endl;
         continue;
       } 

      
      if (rbAllocated + rbRnti - 1 > m_cschedCellConfig.m_ulBandwidth)
       {
         // limit to physical resources last resource assignment
         rbRnti = m_cschedCellConfig.m_ulBandwidth - rbAllocated;
         // at least 3 rbg per flow to ensure TxOpportunity >= 7 bytes
         if (rbRnti < 3)
          {
            // terminate allocation
            rbRnti = 0;     
          }
       }

      NS_LOG_INFO (this << " try to allocate Rnti: " << (*it).first);
      //std::cout<<" \t try to allocate Rnti: " << (int)(*it).first<<std::endl;
      cv2x_UlDciListElement_s uldci;
      uldci.m_rnti = (*it).first;
      uldci.m_rbLen = rbRnti;
      bool allocated = false;
      while ((!allocated)&&((rbAllocated + rbRnti - m_cschedCellConfig.m_ulBandwidth) < 1) && (rbRnti != 0))
       {
         // check availability
         bool free = true; 
         for (uint16_t j = rbAllocated; j < rbAllocated + rbRnti; j++)
           {
             if (rbMap1.at (j) == true)
               {
                 free = false;
                 break;
               }
           }
         if (free)
           {
             uldci.m_rbStart = rbAllocated;

             for (uint16_t j = rbAllocated; j < rbAllocated + rbRnti; j++)
               {
                 rbMap1.at (j) = true;                 
               }
             rbAllocated += rbRnti;
             allocated = true;
             break;
           }
         rbAllocated++;
         if (rbAllocated + rbRnti - 1 > m_cschedCellConfig.m_ulBandwidth)
           {
             // limit to physical resources last resource assignment
             rbRnti = m_cschedCellConfig.m_ulBandwidth - rbAllocated;
             // at least 3 rbg per flow to ensure TxOpportunity >= 7 bytes
             if (rbRnti < 3)
               {
                 // terminate allocation
                 rbRnti = 0;
               }
           }
       } // end while
        
      if (!allocated)
       {
         // unable to allocate new resource: finish scheduling
         m_nextRntiUl = (*it).first;
         if (ret.m_dciList.size () > 0)
           {
             m_schedSapUser->SchedUlConfigInd (ret);
           }
         m_allocationMaps.insert (std::pair <uint16_t, std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > > (params.m_sfnSf, allocationMapPerRntiPerLCId));
         return;
       }

     std::map <uint16_t, std::vector <double> >::iterator itCqi = m_ueCqi.find ((*it).first);
     int cqi = 0;
     if (itCqi == m_ueCqi.end ())
       {
         // no cqi info about this UE
         uldci.m_mcs = 0; // MCS 0 -> UL-AMC TBD
       }
     else
       {
          double maxSinr = 0;
          for (unsigned int i = 0; i < (*itCqi).second.size(); i++)
           {
              if ((*itCqi).second.at (i) > maxSinr)
               {
                  maxSinr = (*itCqi).second.at (i);
               }
            }
 
          double s = log2 ( 1 + (
          std::pow (10, maxSinr / 10 )  /
                   ( (-std::log (5.0 * 0.00005 )) / 1.5) ));

          cqi = m_amc->GetCqiFromSpectralEfficiency (s);
          
         uldci.m_mcs = m_amc->GetMcsFromCqi (cqi);
       } // end 
        
     uldci.m_tbSize = (m_amc->GetUlTbSizeFromMcs (uldci.m_mcs, rbRnti) / 8); // MCS 0 -> UL-AMC TBD


     std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itTmpBuffer = allocationMapPerRntiPerLCId.find ((*it).first);
      if (itTmpBuffer!=allocationMapPerRntiPerLCId.end())
       {
        int rbFlow=0;
        std::map <uint8_t, std::vector <uint16_t> > mapFlow1=itTmpBuffer->second;
        std::map <uint8_t, std::vector <uint16_t> >::iterator itTmpBuffer2;
        for (itTmpBuffer2=mapFlow1.begin(); itTmpBuffer2!=mapFlow1.end(); itTmpBuffer2++)
         {
           rbFlow=(*itTmpBuffer2).second.size();
           UpdateUlRlcBufferInfo (uldci.m_rnti, (*itTmpBuffer2).first, (m_amc->GetUlTbSizeFromMcs (uldci.m_mcs, rbFlow) / 8));    
           //std::cout<<" \t Update UL RLC : rnti = "<<(int)uldci.m_rnti<<" lcid = "<<(int)(*itTmpBuffer2).first<<" bytes = " <<(int)(m_amc->GetUlTbSizeFromMcs (uldci.m_mcs, rbFlow) / 8)<<std::endl;
         }
        UETreated.push_back(uldci.m_rnti);
       }
  
     uldci.m_ndi = 1;
     uldci.m_cceIndex = 0;
     uldci.m_aggrLevel = 1;
     uldci.m_ueTxAntennaSelection = 3; // antenna selection OFF
     uldci.m_hopping = false;
     uldci.m_n2Dmrs = 0;
     uldci.m_tpc = 0; // no power control
     uldci.m_cqiRequest = false; // only period CQI at this stage
     uldci.m_ulIndex = 0; // TDD parameter
     uldci.m_dai = 1; // TDD parameter
     uldci.m_freqHopping = 0;
     uldci.m_pdcchPowerOffset = 0; // not used
     ret.m_dciList.push_back (uldci);
 
     RntiAssigned.push_back ((*it).first);
   
     // store DCI for HARQ_PERIOD
     uint8_t harqId = 0;
     if (m_harqOn == true)
      {
        std::map <uint16_t, uint8_t>::iterator itProcId;
        itProcId = m_ulHarqCurrentProcessId.find (uldci.m_rnti);
        if (itProcId == m_ulHarqCurrentProcessId.end ())
          {
            NS_FATAL_ERROR ("No info find in HARQ buffer for UE " << uldci.m_rnti);
          }
        harqId = (*itProcId).second;
        std::map <uint16_t, cv2x_UlHarqProcessesDciBuffer_t>::iterator itDci = m_ulHarqProcessesDciBuffer.find (uldci.m_rnti);
        if (itDci == m_ulHarqProcessesDciBuffer.end ())
         {
            NS_FATAL_ERROR ("Unable to find RNTI entry in UL DCI HARQ buffer for RNTI " << uldci.m_rnti);
         }
           (*itDci).second.at (harqId) = uldci;
      } // end if (m_harqOn == true)
      
      it++;
      if (it == m_ceBsrRxed.end ())
        {
          // restart from the first
          it = m_ceBsrRxed.begin ();
        }

      rbRnti=0;
      itTmp1 = allocationMapPerRntiPerLCId.find ((*it).first);
      if (itTmp1!=allocationMapPerRntiPerLCId.end())
       {
         std::map <uint8_t, std::vector <uint16_t> > mapFlow1=itTmp1->second;
         std::map <uint8_t, std::vector <uint16_t> >::iterator itTmp2;
         for (itTmp2=mapFlow1.begin(); itTmp2!=mapFlow1.end(); itTmp2++)
          {
            rbRnti=rbRnti+ (*itTmp2).second.size();
          }
       }
      if (RntiAssigned.size()==m_ceBsrRxed.size()) 
       {
         // all RNTI are assigned: no need to continue scheduling 
         m_nextRntiUl = (*it).first;
         break;
       }
    }

  while (((*it).first != m_nextRntiUl) || (rbRnti!=0));

 
  m_allocationMaps.insert (std::pair <uint16_t, std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > > (params.m_sfnSf, allocationMapPerRntiPerLCId));

  m_schedSapUser->SchedUlConfigInd (ret);

  //printf BSR buffer status
  {
    std::multimap <uint16_t, std::map <uint8_t, uint32_t> >::iterator bsrIt;
    for (bsrIt = m_ceBsrRxed.begin() ; bsrIt != m_ceBsrRxed.end () ; bsrIt++)
      {
        uint16_t rnti = bsrIt->first;
        std::map <uint8_t, uint32_t> copyBsrRnti = bsrIt->second;
        std::map <uint8_t, uint32_t>::iterator lcidIt;
        for (lcidIt = copyBsrRnti.begin() ; lcidIt != copyBsrRnti.end() ; lcidIt++)
          {
            NS_LOG_DEBUG ("BSR2 status: RNTI " << rnti << " LCID " << (uint16_t) lcidIt->first << " BUFFER " << lcidIt->second);
            //std::cout<<"BSR2 status: RNTI " << (int)rnti << " LCID " << (int) lcidIt->first << " BUFFER " << (int)lcidIt->second<<std::endl;
            if (lcidIt->second == 0) 
              {
                //erase lc information
                //bsrIt->second.erase (lcidIt->first);
              }
          }
      }
    //clear up
    std::multimap <uint16_t, std::map <uint8_t, uint32_t> > cpBsrRxed = m_ceBsrRxed;
    for (bsrIt = cpBsrRxed.begin() ; bsrIt != cpBsrRxed.end () ; bsrIt++)
      {
        if (bsrIt->second.size() == 0)
          {
            //no more LCID for this RNTI, remove it
            //m_ceBsrRxed.erase (bsrIt->first);
          }
      }
  }

  //print allocation map
   {
     uint16_t usedRBGs = 0;
     std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator allocMapRntiIt;
     for (allocMapRntiIt = allocationMapPerRntiPerLCId.begin() ; allocMapRntiIt != allocationMapPerRntiPerLCId.end(); allocMapRntiIt++) 
       {
  //       uint16_t rnti = allocMapRntiIt->first;
         std::map <uint8_t, std::vector <uint16_t> >::iterator allocMapLcidIt;
         for (allocMapLcidIt = allocMapRntiIt->second.begin() ; allocMapLcidIt != allocMapRntiIt->second.end() ; allocMapLcidIt++) 
           {
  //           uint8_t lcid = allocMapLcidIt->first;
             usedRBGs += allocMapLcidIt->second.size();
  //           std::cout << Simulator::Now ().GetSeconds () << " UL ALLOC RNTI " << rnti << " LCID " << (uint16_t) lcid << " RBGs " << allocMapLcidIt->second.size() << std::endl;          
           }
       }
     //std::cout << Simulator::Now ().GetSeconds () << " TOTAL UL USED " << usedRBGs << std::endl;          
   }

  return;
       
}

void
cv2x_RrFfMacScheduler::DoSchedUlNoiseInterferenceReq (const struct cv2x_FfMacSchedSapProvider::SchedUlNoiseInterferenceReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  return;
}

void
cv2x_RrFfMacScheduler::DoSchedUlSrInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlSrInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  return;
}

void
cv2x_RrFfMacScheduler::DoSchedUlMacCtrlInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
{
  //std::cout<<" ENTER cv2x_RrFfMacScheduler::DoSchedUlMacCtrlInfoReq "<<std::endl;
  NS_LOG_FUNCTION (this);
  std::multimap <uint16_t, std::map <uint8_t, uint32_t> >::iterator it;
  for (unsigned int i = 0; i < params.m_macCeList.size (); i++)
    {
      if ( params.m_macCeList.at (i).m_macCeType == cv2x_MacCeListElement_s::BSR )
        {
          // buffer status report
          // note that this scheduler does not differentiate the
          // allocation according to which LCGs have more/less bytes
          // to send.
          // Hence the BSR of different LCGs are just summed up to get
          // a total queue size that is used for allocation purposes.

          uint32_t buffer = 0;
          uint16_t rnti = params.m_macCeList.at (i).m_rnti;
          std::map <uint8_t, std::vector <uint8_t> > mapBsr=params.m_macCeList.at (i).m_macCeValue.m_SlBufferStatus;
          std::map <uint8_t, std::vector <uint8_t> >::iterator itBsr=mapBsr.begin();
          
          for (itBsr=mapBsr.begin(); itBsr!=mapBsr.end(); itBsr++)  // modification to handle the new structure of the m_macCeList
           {  
             buffer=0;                                                     
             uint8_t lcId=(*itBsr).first;
             it = m_ceBsrRxed.find (rnti);
             if (it == m_ceBsrRxed.end ())
               { 
                 //create a new entry 
                 std::map <uint8_t, uint32_t> BsrTemp;
                 for (uint8_t lcg = 0; lcg < 4; ++lcg)
                  {
                    uint8_t bsrId=(*itBsr).second.at (lcg);
                    buffer += cv2x_BufferSizeLevelBsr::BsrId2BufferSize (bsrId);
                  }
                 BsrTemp.insert (std::pair <uint8_t, uint32_t> (lcId, buffer));
                 m_ceBsrRxed.insert (std::pair <uint16_t, std::map <uint8_t, uint32_t> > (rnti,  BsrTemp));
                 NS_LOG_DEBUG ("BSR Rx RNTI " << rnti << " LCID " << (uint16_t) lcId << " BUFFER " << buffer);
                 //std::cout<<"\t create new entry bsr , BSR Rx RNTI " << (int) rnti << " LCID " << (int) lcId << " BUFFER " << (int) buffer;
               }
              else
               {
                 //update the buffer size value
                  std::map <uint8_t, uint32_t>::iterator itMap;
                  itMap=it->second.find ((uint8_t)lcId);
                  if (itMap == it->second.end ())
                   {
                      //insert new LC id element 
                      for (uint8_t lcg = 0; lcg < 4; ++lcg)
                       {
                         uint8_t bsrId=(*itBsr).second.at (lcg);  
                         buffer += cv2x_BufferSizeLevelBsr::BsrId2BufferSize (bsrId);               
                       }    
                      //std::cout<<"\t buffer = "<<(int)buffer<<std::endl;     
                      it->second.insert (std::pair <uint8_t, uint32_t> (lcId, buffer));
                      //std::cout<<" insert new lcid in bsr , BSR Rx RNTI " << (int) rnti << " LCID " << (int) lcId << " BUFFER " << (int) buffer;
                   }
                  else 
                   {   
                     for (uint8_t lcg = 0; lcg < 4; ++lcg) 
                      {
                         uint8_t bsrId=(*itBsr).second.at (lcg);
                         buffer += cv2x_BufferSizeLevelBsr::BsrId2BufferSize (bsrId);
                      }  
                     //std::cout<<"\t buffer = "<<(int)buffer<<std::endl; 
                     (*itMap).second=buffer;
                   }   
                  NS_LOG_DEBUG ("BSR Rx RNTI " << rnti << " LCID " << (uint16_t) lcId << " BUFFER " << buffer); 
                  //std::cout<<" update  lcid in bsr , BSR Rx RNTI " << (int) rnti << " LCID " << (int) lcId << " BUFFER " << (int) buffer;
               } // end else 
           } //end if (it == m_ceBsrRxed.end ())
        } // end for for (itBsr=mapBsr.begin(); itBsr!=mapBsr.end(); itBsr++)
    } //end for 
  return; 
}

void
cv2x_RrFfMacScheduler::DoSchedUlCqiInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  //std::cout<<"ENTER DoSchedUlCqiInfoReq " <<std::endl;
  NS_LOG_FUNCTION (this);
  // retrieve the allocation for this subframe
  
  switch (m_ulCqiFilter)
    {
    case cv2x_FfMacScheduler::SRS_UL_CQI:
      {
        // filter all the CQIs that are not SRS based
        if (params.m_ulCqi.m_type != cv2x_UlCqi_s::SRS)
          {
            return;
          }
      }
      break;
    case cv2x_FfMacScheduler::PUSCH_UL_CQI:
      {
        // filter all the CQIs that are not SRS based
        if (params.m_ulCqi.m_type != cv2x_UlCqi_s::PUSCH)
          {
            return;
          }
      }
    case cv2x_FfMacScheduler::ALL_UL_CQI:
      break;

    default:
      NS_FATAL_ERROR ("Unknown UL CQI type");
    }

  switch (params.m_ulCqi.m_type)
    {
    case cv2x_UlCqi_s::PUSCH:
      {
        std::map <uint16_t, std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > >::iterator itMap=m_allocationMaps.begin();
        std::map <uint16_t, std::vector <double> >::iterator itCqi;
        NS_LOG_DEBUG (this << " Collect PUSCH CQIs of Frame no. " << (params.m_sfnSf >> 4) << " subframe no. " << (0xF & params.m_sfnSf));

        // added code
        std::map<uint16_t, std::vector <uint16_t> > allocationMap;
        std::map<uint16_t, std::vector <uint16_t> >::iterator itallocationMap=allocationMap.begin();

        uint16_t snsf=(*itMap).first;
        std::vector <uint16_t> vTemp;

        std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > mapRntiLcid = itMap->second;
        std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itmapRntiLcid;
        for (itmapRntiLcid=mapRntiLcid.begin(); itmapRntiLcid!= mapRntiLcid.end(); itmapRntiLcid++)
         {
           std::map <uint8_t, std::vector <uint16_t> > mapLcid=itmapRntiLcid->second;
           std::map <uint8_t, std::vector <uint16_t> >::iterator itLcid;
           for (itLcid=mapLcid.begin(); itLcid!=mapLcid.end(); itLcid ++)
            {
              vTemp.push_back ((*itmapRntiLcid).first);
            }
         }
         
        allocationMap.insert ( std::pair <uint16_t, std::vector <uint16_t> > (snsf, vTemp));
        vTemp.clear();
        itallocationMap = allocationMap.find (params.m_sfnSf);
        if (itallocationMap == allocationMap.end ())
          {
            return;
          }
 
        bool s_found=false;

        for (uint32_t i = 0; i < (*itallocationMap).second.size(); i++)
          {
            s_found=true;
            // convert from fixed point notation Sxxxxxxxxxxx.xxx to double
            double sinr = cv2x_LteFfConverter::fpS11dot3toDouble (params.m_ulCqi.m_sinr.at (i));
            itCqi = m_ueCqi.find ((*itallocationMap).second.at (i));
            if (itCqi == m_ueCqi.end ())
              {
                // create a new entry
                std::vector <double> newCqi;
                for (uint32_t j = 0; j < m_cschedCellConfig.m_ulBandwidth; j++)
                  {
                    if (i == j)
                      {
                        newCqi.push_back (sinr);
                      }
                    else
                      {
                        // initialize with NO_SINR value.
                        newCqi.push_back (NO_SINR);
                      }
                  }
                m_ueCqi.insert (std::pair <uint16_t, std::vector <double> > ((*itallocationMap).second.at (i), newCqi));
                // generate correspondent timer
                m_ueCqiTimers.insert (std::pair <uint16_t, uint32_t > ((*itallocationMap).second.at (i), m_cqiTimersThreshold));
              }
            else
              {
                // update the value
                (*itCqi).second.at (i) = sinr;
                NS_LOG_DEBUG (this << " RNTI " << (*itallocationMap).second.at (i) << " RB " << i << " SINR " << sinr);
                // update correspondent timer
                std::map <uint16_t, uint32_t>::iterator itTimers;
                itTimers = m_ueCqiTimers.find ((*itallocationMap).second.at (i));
                (*itTimers).second = m_cqiTimersThreshold;
              }
          }
        // remove obsolete info on allocation
        if (s_found==true)
        m_allocationMaps.erase (itMap);
      }
      break;
    case cv2x_UlCqi_s::SRS:
      {
        NS_LOG_DEBUG (this << " Collect SRS CQIs of Frame no. " << (params.m_sfnSf >> 4) << " subframe no. " << (0xF & params.m_sfnSf));
        // get the RNTI from vendor specific parameters
        uint16_t rnti = 0;
        NS_ASSERT (params.m_vendorSpecificList.size () > 0);
        for (uint16_t i = 0; i < params.m_vendorSpecificList.size (); i++)
          {
            if (params.m_vendorSpecificList.at (i).m_type == SRS_CQI_RNTI_VSP)
              {
                Ptr<cv2x_SrsCqiRntiVsp> vsp = DynamicCast<cv2x_SrsCqiRntiVsp> (params.m_vendorSpecificList.at (i).m_value);
                rnti = vsp->GetRnti ();
              }
          }
        std::map <uint16_t, std::vector <double> >::iterator itCqi;
        itCqi = m_ueCqi.find (rnti);
        if (itCqi == m_ueCqi.end ())
          {
            // create a new entry
            std::vector <double> newCqi;
            for (uint32_t j = 0; j < m_cschedCellConfig.m_ulBandwidth; j++)
              {
                double sinr = cv2x_LteFfConverter::fpS11dot3toDouble (params.m_ulCqi.m_sinr.at (j));
                newCqi.push_back (sinr);
                NS_LOG_INFO (this << " RNTI " << rnti << " new SRS-CQI for RB  " << j << " value " << sinr);
              }
            m_ueCqi.insert (std::pair <uint16_t, std::vector <double> > (rnti, newCqi));
            // generate correspondent timer
            m_ueCqiTimers.insert (std::pair <uint16_t, uint32_t > (rnti, m_cqiTimersThreshold));
          }
        else
          {
            // update the values
            for (uint32_t j = 0; j < m_cschedCellConfig.m_ulBandwidth; j++)
              {
                double sinr = cv2x_LteFfConverter::fpS11dot3toDouble (params.m_ulCqi.m_sinr.at (j));
                (*itCqi).second.at (j) = sinr;
                NS_LOG_INFO (this << " RNTI " << rnti << " update SRS-CQI for RB  " << j << " value " << sinr);
              }
            // update correspondent timer
            std::map <uint16_t, uint32_t>::iterator itTimers;
            itTimers = m_ueCqiTimers.find (rnti);
            (*itTimers).second = m_cqiTimersThreshold;
          }
      }
      break;
    case cv2x_UlCqi_s::PUCCH_1:
    case cv2x_UlCqi_s::PUCCH_2:
    case cv2x_UlCqi_s::PRACH:
      {
        NS_FATAL_ERROR ("MtFfMacScheduler supports only PUSCH and SRS UL-CQIs");
      }
      break;
    default:
      NS_FATAL_ERROR ("Unknown type of UL-CQI");
    }
  return;
}


void
cv2x_RrFfMacScheduler::RefreshDlCqiMaps (void)
{
  NS_LOG_FUNCTION (this << m_p10CqiTimers.size ());
  // refresh DL CQI P01 Map
  std::map <uint16_t,uint32_t>::iterator itP10 = m_p10CqiTimers.begin ();
  while (itP10 != m_p10CqiTimers.end ())
    {
      NS_LOG_INFO (this << " P10-CQI for user " << (*itP10).first << " is " << (uint32_t)(*itP10).second << " thr " << (uint32_t)m_cqiTimersThreshold);
      if ((*itP10).second == 0)
        {
          // delete correspondent entries
          std::map <uint16_t,uint8_t>::iterator itMap = m_p10CqiRxed.find ((*itP10).first);
          NS_ASSERT_MSG (itMap != m_p10CqiRxed.end (), " Does not find CQI report for user " << (*itP10).first);
          NS_LOG_INFO (this << " P10-CQI exired for user " << (*itP10).first);
          m_p10CqiRxed.erase (itMap);
          std::map <uint16_t,uint32_t>::iterator temp = itP10;
          itP10++;
          m_p10CqiTimers.erase (temp);
        }
      else
        {
          (*itP10).second--;
          itP10++;
        }
    }

  return;
}


void
cv2x_RrFfMacScheduler::RefreshUlCqiMaps (void)
{
  // refresh UL CQI  Map
  std::map <uint16_t,uint32_t>::iterator itUl = m_ueCqiTimers.begin ();
  while (itUl != m_ueCqiTimers.end ())
    {
      NS_LOG_INFO (this << " UL-CQI for user " << (*itUl).first << " is " << (uint32_t)(*itUl).second << " thr " << (uint32_t)m_cqiTimersThreshold);
      if ((*itUl).second == 0)
        {
          // delete correspondent entries
          std::map <uint16_t, std::vector <double> >::iterator itMap = m_ueCqi.find ((*itUl).first);
          NS_ASSERT_MSG (itMap != m_ueCqi.end (), " Does not find CQI report for user " << (*itUl).first);
          NS_LOG_INFO (this << " UL-CQI exired for user " << (*itUl).first);
          (*itMap).second.clear ();
          m_ueCqi.erase (itMap);
          std::map <uint16_t,uint32_t>::iterator temp = itUl;
          itUl++;
          m_ueCqiTimers.erase (temp);
        }
      else
        {
          (*itUl).second--;
          itUl++;
        }
    }

  return;
}

void
cv2x_RrFfMacScheduler::UpdateDlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size)
{
  //std::cout<<" ENTER cv2x_RrFfMacScheduler::UpdateDlRlcBufferInfo "<<std::endl;
  //std::cout<<" \t rnti = "<<(int)rnti <<" lcid = " <<(int)lcid <<" tb size = "<<(int) size <<std::endl;
  std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;
  cv2x_LteFlowId_t flow (rnti, lcid);
  it = m_rlcBufferReq.find (flow);
  if (it != m_rlcBufferReq.end ())
    {
      NS_LOG_INFO (this << " UE " << rnti << " LC " << (uint16_t)lcid << " txqueue " << (*it).second.m_rlcTransmissionQueueSize << " retxqueue " << (*it).second.m_rlcRetransmissionQueueSize << " status " << (*it).second.m_rlcStatusPduSize << " decrease " << size);

      //std::cout << " UE " << rnti << " LC " << (int)lcid << " txqueue " << (int) (*it).second.m_rlcTransmissionQueueSize << " retxqueue " << (int)(*it).second.m_rlcRetransmissionQueueSize << " status " << (int)(*it).second.m_rlcStatusPduSize << " decrease " << (int) size << std::endl;

      // Update queues: RLC tx order Status, ReTx, Tx
      // Update status queue
      if (((*it).second.m_rlcStatusPduSize > 0) && (size >= (*it).second.m_rlcStatusPduSize))
        {
          (*it).second.m_rlcStatusPduSize = 0;
        }
      else if (((*it).second.m_rlcRetransmissionQueueSize > 0) && (size >= (*it).second.m_rlcRetransmissionQueueSize))
        {
          (*it).second.m_rlcRetransmissionQueueSize = 0;
        }
      else if ((*it).second.m_rlcTransmissionQueueSize > 0)
        {
          uint32_t rlcOverhead;
          if (lcid == 1)
            {
              // for SRB1 (using RLC AM) it's better to
              // overestimate RLC overhead rather than
              // underestimate it and risk unneeded
              // segmentation which increases delay 
              rlcOverhead = 4;
            }
          else
            {
              // minimum RLC overhead due to header
              rlcOverhead = 2;
            }
          // update transmission queue
          if ((*it).second.m_rlcTransmissionQueueSize <= size - rlcOverhead)
            {
              (*it).second.m_rlcTransmissionQueueSize = 0;
            }
          else
            {
              (*it).second.m_rlcTransmissionQueueSize -= size - rlcOverhead;
            }
        }
    }
  else
    {
      NS_LOG_ERROR (this << " Does not find DL RLC Buffer Report of UE " << rnti);
    }
}


void
cv2x_RrFfMacScheduler::UpdateUlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size)
{

  size = size - 2; // remove the minimum RLC overhead
  std::multimap <uint16_t, std::map <uint8_t, uint32_t> >::iterator it=m_ceBsrRxed.find (rnti);
  
  if (it != m_ceBsrRxed.end ())
    {
      std::map <uint8_t, uint32_t>::iterator itMap=it->second.find (lcid);
      if (itMap!=it->second.end())
       {
         NS_LOG_INFO (this << " UE " << rnti << " lcid "<< (uint16_t) lcid << " size " << size << " BSR " << (*itMap).second);
         //std::cout<<"\t UE " << (int) rnti << " lcid "<< (int)lcid << " size " << (int) size << " BSR " << (int) (*itMap).second<<std::endl;
         if ((*itMap).second >= size)
          {
            (*itMap).second -= size;
            //std::cout<<"\t updated data in m_bsr =" <<(int)(*itMap).second<<std::endl;
          }
         else
          {
            (*itMap).second = 0;
            //std::cout<<"\t updated data in m_bsr =" <<(int)(*itMap).second<<std::endl;
          }
       }
      else 
       {
         NS_LOG_ERROR (this << " Does not find BSR report info of UE LCID " << lcid);
         //std::cout<<" \t LCID not found in m_bsr "<<std::endl;
       }
    }  
  else
    {
      //std::cout<<"\t error : Does not find BSR report info of UE"<<std::endl;
      NS_LOG_ERROR (this << " Does not find BSR report info of UE " << rnti);
    }
}

void
cv2x_RrFfMacScheduler::TransmissionModeConfigurationUpdate (uint16_t rnti, uint8_t txMode)
{
  NS_LOG_FUNCTION (this << " RNTI " << rnti << " txMode " << (uint16_t)txMode);
  cv2x_FfMacCschedSapUser::CschedUeConfigUpdateIndParameters params;
  params.m_rnti = rnti;
  params.m_transmissionMode = txMode;
  m_cschedSapUser->CschedUeConfigUpdateInd (params);
}



}

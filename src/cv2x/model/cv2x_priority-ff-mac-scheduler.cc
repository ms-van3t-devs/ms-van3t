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

#include <ns3/simulator.h>
#include <ns3/cv2x_lte-amc.h>
#include <ns3/cv2x_priority-ff-mac-scheduler.h>
#include <ns3/cv2x_lte-vendor-specific-parameters.h>
#include <ns3/boolean.h>
#include <cfloat>
#include <set>
#include <map>
#include <algorithm>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_PriorityFfMacScheduler");

static const int PriorityType0AllocationRbg[4] = {
  10,       // RGB size 1
  26,       // RGB size 2
  63,       // RGB size 3
  110       // RGB size 4
};  // see table 7.1.6.1-1 of 36.213


NS_OBJECT_ENSURE_REGISTERED (cv2x_PriorityFfMacScheduler);



class cv2x_PrioritySchedulerMemberCschedSapProvider : public cv2x_FfMacCschedSapProvider
{
public:
  cv2x_PrioritySchedulerMemberCschedSapProvider (cv2x_PriorityFfMacScheduler* scheduler);

  // inherited from cv2x_FfMacCschedSapProvider
  virtual void CschedCellConfigReq (const struct CschedCellConfigReqParameters& params);
  virtual void CschedUeConfigReq (const struct CschedUeConfigReqParameters& params);
  virtual void CschedLcConfigReq (const struct CschedLcConfigReqParameters& params);
  virtual void CschedLcReleaseReq (const struct CschedLcReleaseReqParameters& params);
  virtual void CschedUeReleaseReq (const struct CschedUeReleaseReqParameters& params);

private:
  cv2x_PrioritySchedulerMemberCschedSapProvider ();
  cv2x_PriorityFfMacScheduler* m_scheduler;
};

cv2x_PrioritySchedulerMemberCschedSapProvider::cv2x_PrioritySchedulerMemberCschedSapProvider ()
{
}

cv2x_PrioritySchedulerMemberCschedSapProvider::cv2x_PrioritySchedulerMemberCschedSapProvider (cv2x_PriorityFfMacScheduler* scheduler) : m_scheduler (scheduler)
{
}


void
cv2x_PrioritySchedulerMemberCschedSapProvider::CschedCellConfigReq (const struct CschedCellConfigReqParameters& params)
{
  m_scheduler->DoCschedCellConfigReq (params);
}

void
cv2x_PrioritySchedulerMemberCschedSapProvider::CschedUeConfigReq (const struct CschedUeConfigReqParameters& params)
{
  m_scheduler->DoCschedUeConfigReq (params);
}


void
cv2x_PrioritySchedulerMemberCschedSapProvider::CschedLcConfigReq (const struct CschedLcConfigReqParameters& params)
{
  m_scheduler->DoCschedLcConfigReq (params);
}

void
cv2x_PrioritySchedulerMemberCschedSapProvider::CschedLcReleaseReq (const struct CschedLcReleaseReqParameters& params)
{
  m_scheduler->DoCschedLcReleaseReq (params);
}

void
cv2x_PrioritySchedulerMemberCschedSapProvider::CschedUeReleaseReq (const struct CschedUeReleaseReqParameters& params)
{
  m_scheduler->DoCschedUeReleaseReq (params);
}




class cv2x_PrioritySchedulerMemberSchedSapProvider : public cv2x_FfMacSchedSapProvider
{
public:
  cv2x_PrioritySchedulerMemberSchedSapProvider (cv2x_PriorityFfMacScheduler* scheduler);

  // inherited from cv2x_FfMacSchedSapProvider
  virtual void SchedDlRlcBufferReq (const struct SchedDlRlcBufferReqParameters& params);
  virtual void SchedDlPagingBufferReq (const struct SchedDlPagingBufferReqParameters& params);
  virtual void SchedDlMacBufferReq (const struct SchedDlMacBufferReqParameters& params);
  virtual void SchedDlTriggerReq (const struct SchedDlTriggerReqParameters& params);
  virtual void SchedDlRachInfoReq (const struct SchedDlRachInfoReqParameters& params);
  virtual void SchedDlCqiInfoReq (const struct SchedDlCqiInfoReqParameters& params);
  virtual void SchedUlTriggerReq (const struct SchedUlTriggerReqParameters& params);
  virtual void SchedUlNoiseInterferenceReq (const struct SchedUlNoiseInterferenceReqParameters& params);
  virtual void SchedUlSrInfoReq (const struct SchedUlSrInfoReqParameters& params);
  virtual void SchedUlMacCtrlInfoReq (const struct SchedUlMacCtrlInfoReqParameters& params);
  virtual void SchedUlCqiInfoReq (const struct SchedUlCqiInfoReqParameters& params);


private:
  cv2x_PrioritySchedulerMemberSchedSapProvider ();
  cv2x_PriorityFfMacScheduler* m_scheduler;
};



cv2x_PrioritySchedulerMemberSchedSapProvider::cv2x_PrioritySchedulerMemberSchedSapProvider ()
{
}


cv2x_PrioritySchedulerMemberSchedSapProvider::cv2x_PrioritySchedulerMemberSchedSapProvider (cv2x_PriorityFfMacScheduler* scheduler)
  : m_scheduler (scheduler)
{
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedDlRlcBufferReq (const struct SchedDlRlcBufferReqParameters& params)
{
  m_scheduler->DoSchedDlRlcBufferReq (params);
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedDlPagingBufferReq (const struct SchedDlPagingBufferReqParameters& params)
{
  m_scheduler->DoSchedDlPagingBufferReq (params);
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedDlMacBufferReq (const struct SchedDlMacBufferReqParameters& params)
{
  m_scheduler->DoSchedDlMacBufferReq (params);
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedDlTriggerReq (const struct SchedDlTriggerReqParameters& params)
{
  m_scheduler->DoSchedDlTriggerReq (params);
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedDlRachInfoReq (const struct SchedDlRachInfoReqParameters& params)
{
  m_scheduler->DoSchedDlRachInfoReq (params);
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedDlCqiInfoReq (const struct SchedDlCqiInfoReqParameters& params)
{
  m_scheduler->DoSchedDlCqiInfoReq (params);
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedUlTriggerReq (const struct SchedUlTriggerReqParameters& params)
{
  m_scheduler->DoSchedUlTriggerReq (params);
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedUlNoiseInterferenceReq (const struct SchedUlNoiseInterferenceReqParameters& params)
{
  m_scheduler->DoSchedUlNoiseInterferenceReq (params);
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedUlSrInfoReq (const struct SchedUlSrInfoReqParameters& params)
{
  m_scheduler->DoSchedUlSrInfoReq (params);
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedUlMacCtrlInfoReq (const struct SchedUlMacCtrlInfoReqParameters& params)
{
  m_scheduler->DoSchedUlMacCtrlInfoReq (params);
}

void
cv2x_PrioritySchedulerMemberSchedSapProvider::SchedUlCqiInfoReq (const struct SchedUlCqiInfoReqParameters& params)
{
  m_scheduler->DoSchedUlCqiInfoReq (params);
}





cv2x_PriorityFfMacScheduler::cv2x_PriorityFfMacScheduler ()
  :   m_cschedSapUser (0),
    m_schedSapUser (0),
    m_timeWindow (99.0),
    m_nextRntiUl (0)
{
  m_amc = CreateObject <cv2x_LteAmc> ();
  m_cschedSapProvider = new cv2x_PrioritySchedulerMemberCschedSapProvider (this);
  m_schedSapProvider = new cv2x_PrioritySchedulerMemberSchedSapProvider (this);
  m_ffrSapProvider = 0;
  m_ffrSapUser = new cv2x_MemberLteFfrSapUser<cv2x_PriorityFfMacScheduler> (this);
}

cv2x_PriorityFfMacScheduler::~cv2x_PriorityFfMacScheduler ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_PriorityFfMacScheduler::DoDispose ()
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
  delete m_ffrSapUser;
}

TypeId
cv2x_PriorityFfMacScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_PriorityFfMacScheduler")
    .SetParent<cv2x_FfMacScheduler> ()
    .AddConstructor<cv2x_PriorityFfMacScheduler> ()
    .AddAttribute ("CqiTimerThreshold",
                   "The number of TTIs a CQI is valid (default 1000 - 1 sec.)",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&cv2x_PriorityFfMacScheduler::m_cqiTimersThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("HarqEnabled",
                   "Activate/Deactivate the HARQ [by default is active].",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_PriorityFfMacScheduler::m_harqOn),
                   MakeBooleanChecker ())
    .AddAttribute ("UlGrantMcs",
                   "The MCS of the UL grant, must be [0..15] (default 0)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_PriorityFfMacScheduler::m_ulGrantMcs),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}



void
cv2x_PriorityFfMacScheduler::SetFfMacCschedSapUser (cv2x_FfMacCschedSapUser* s)
{
  m_cschedSapUser = s;
}

void
cv2x_PriorityFfMacScheduler::SetFfMacSchedSapUser (cv2x_FfMacSchedSapUser* s)
{
  m_schedSapUser = s;
}

cv2x_FfMacCschedSapProvider*
cv2x_PriorityFfMacScheduler::GetFfMacCschedSapProvider ()
{
  return m_cschedSapProvider;
}

cv2x_FfMacSchedSapProvider*
cv2x_PriorityFfMacScheduler::GetFfMacSchedSapProvider ()
{
  return m_schedSapProvider;
}

void
cv2x_PriorityFfMacScheduler::SetLteFfrSapProvider (cv2x_LteFfrSapProvider* s)
{
  m_ffrSapProvider = s;
}

cv2x_LteFfrSapUser*
cv2x_PriorityFfMacScheduler::GetLteFfrSapUser ()
{
  return m_ffrSapUser;
}

void
cv2x_PriorityFfMacScheduler::DoCschedCellConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedCellConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  // Read the subset of parameters used
  m_cschedCellConfig = params;
  m_rachAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);
  cv2x_FfMacCschedSapUser::CschedUeConfigCnfParameters cnf;
  cnf.m_result = SUCCESS;
  m_cschedSapUser->CschedUeConfigCnf (cnf);
  return;
}

void
cv2x_PriorityFfMacScheduler::DoCschedUeConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedUeConfigReqParameters& params)
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
      DlHarqProcessesDciBuffer_t dlHarqdci;
      dlHarqdci.resize (8);
      m_dlHarqProcessesDciBuffer.insert (std::pair <uint16_t, DlHarqProcessesDciBuffer_t> (params.m_rnti, dlHarqdci));
      DlHarqRlcPduListBuffer_t dlHarqRlcPdu;
      dlHarqRlcPdu.resize (2);
      dlHarqRlcPdu.at (0).resize (8);
      dlHarqRlcPdu.at (1).resize (8);
      m_dlHarqProcessesRlcPduListBuffer.insert (std::pair <uint16_t, DlHarqRlcPduListBuffer_t> (params.m_rnti, dlHarqRlcPdu));
      m_ulHarqCurrentProcessId.insert (std::pair <uint16_t,uint8_t > (params.m_rnti, 0));
      UlHarqProcessesStatus_t ulHarqPrcStatus;
      ulHarqPrcStatus.resize (8,0);
      m_ulHarqProcessesStatus.insert (std::pair <uint16_t, UlHarqProcessesStatus_t> (params.m_rnti, ulHarqPrcStatus));
      UlHarqProcessesDciBuffer_t ulHarqdci;
      ulHarqdci.resize (8);
      m_ulHarqProcessesDciBuffer.insert (std::pair <uint16_t, UlHarqProcessesDciBuffer_t> (params.m_rnti, ulHarqdci));
    }
  else
    {
      (*it).second = params.m_transmissionMode;
    }
  return;
}

void
cv2x_PriorityFfMacScheduler::DoCschedLcConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedLcConfigReqParameters& params)
{

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


  std::map <uint16_t, cv2x_PrioritysFlowPerf_t>::iterator it;

  for (uint16_t i = 0; i < params.m_logicalChannelConfigList.size (); i++)
    {
      it = m_flowStatsDl.find (params.m_rnti);

      if (it == m_flowStatsDl.end ())
        {
          double tbrDlInBytes = params.m_logicalChannelConfigList.at (i).m_eRabGuaranteedBitrateDl / 8;   // byte/s
          double tbrUlInBytes = params.m_logicalChannelConfigList.at (i).m_eRabGuaranteedBitrateUl / 8;   // byte/s

          cv2x_PrioritysFlowPerf_t flowStatsDl;
          flowStatsDl.flowStart = Simulator::Now ();
          flowStatsDl.totalBytesTransmitted = 0;
          flowStatsDl.lastTtiBytesTransmitted = 0;
          flowStatsDl.lastAveragedThroughput = 1;
          flowStatsDl.secondLastAveragedThroughput = 1;
          flowStatsDl.targetThroughput = tbrDlInBytes;
          m_flowStatsDl.insert (std::pair<uint16_t, cv2x_PrioritysFlowPerf_t> (params.m_rnti, flowStatsDl));
          cv2x_PrioritysFlowPerf_t flowStatsUl;
          flowStatsUl.flowStart = Simulator::Now ();
          flowStatsUl.totalBytesTransmitted = 0;
          flowStatsUl.lastTtiBytesTransmitted = 0;
          flowStatsUl.lastAveragedThroughput = 1;
          flowStatsUl.secondLastAveragedThroughput = 1;
          flowStatsUl.targetThroughput = tbrUlInBytes;
          m_flowStatsUl.insert (std::pair<uint16_t, cv2x_PrioritysFlowPerf_t> (params.m_rnti, flowStatsUl));
        }
      else
        {
          // update GBR from cv2x_UeManager::SetupDataRadioBearer ()
          double tbrDlInBytes = params.m_logicalChannelConfigList.at (i).m_eRabGuaranteedBitrateDl / 8;   // byte/s
          double tbrUlInBytes = params.m_logicalChannelConfigList.at (i).m_eRabGuaranteedBitrateUl / 8;   // byte/s
          m_flowStatsDl[(*it).first].targetThroughput = tbrDlInBytes;
          m_flowStatsUl[(*it).first].targetThroughput = tbrUlInBytes;

        }
    }

  return;
  
}

void
cv2x_PriorityFfMacScheduler::DoCschedLcReleaseReq (const struct cv2x_FfMacCschedSapProvider::CschedLcReleaseReqParameters& params)
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
cv2x_PriorityFfMacScheduler::DoCschedUeReleaseReq (const struct cv2x_FfMacCschedSapProvider::CschedUeReleaseReqParameters& params)
{
 NS_LOG_FUNCTION (this);
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
  m_flowStatsDl.erase  (params.m_rnti);
  m_flowStatsUl.erase  (params.m_rnti);
  m_ceBsrRxed.erase (params.m_rnti);
  std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it = m_rlcBufferReq.begin ();
  std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator temp;
  while (it!=m_rlcBufferReq.end ())
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

  return;
}


void
cv2x_PriorityFfMacScheduler::DoSchedDlRlcBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this << params.m_rnti << (uint32_t) params.m_logicalChannelIdentity);
  // API generated by RLC for updating RLC parameters on a LC (tx and retx queues)

  std::map <cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;

  cv2x_LteFlowId_t flow (params.m_rnti, params.m_logicalChannelIdentity);

  it =  m_rlcBufferReq.find (flow);

  if (it == m_rlcBufferReq.end ())
    {
      m_rlcBufferReq.insert (std::pair <cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters> (flow, params));
    }
  else
    {
      (*it).second = params;
    }

  return;
}

void
cv2x_PriorityFfMacScheduler::DoSchedDlPagingBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlPagingBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("method not implemented");
  return;
}

void
cv2x_PriorityFfMacScheduler::DoSchedDlMacBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlMacBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("method not implemented");
  return;
}

int
cv2x_PriorityFfMacScheduler::GetRbgSize (int dlbandwidth)
{
  for (int i = 0; i < 4; i++)
    {
      if (dlbandwidth < PriorityType0AllocationRbg[i])
        {
          return (i + 1);
        }
    }

  return (-1);
}


int
cv2x_PriorityFfMacScheduler::LcActivePerFlow (uint16_t rnti)
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
cv2x_PriorityFfMacScheduler::HarqProcessAvailability (uint16_t rnti)
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
cv2x_PriorityFfMacScheduler::UpdateHarqProcessId (uint16_t rnti)
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
      NS_FATAL_ERROR ("No HARQ process available for RNTI " << rnti << " check before update with HarqProcessAvailability");
    }

  return ((*it).second);
}


void
cv2x_PriorityFfMacScheduler::RefreshHarqProcesses ()
{
  NS_LOG_FUNCTION (this);

  std::map <uint16_t, DlHarqProcessesTimer_t>::iterator itTimers;
  for (itTimers = m_dlHarqProcessesTimer.begin (); itTimers != m_dlHarqProcessesTimer.end (); itTimers++)
    {
      for (uint16_t i = 0; i < HARQ_PROC_NUM; i++)
        {
          if ((*itTimers).second.at (i) == HARQ_DL_TIMEOUT)
            {
              // reset HARQ process

              NS_LOG_DEBUG (this << " Reset HARQ proc " << i << " for RNTI " << (*itTimers).first);
              std::map <uint16_t, DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find ((*itTimers).first);
              if (itStat == m_dlHarqProcessesStatus.end ())
                {
                  NS_FATAL_ERROR ("No Process Id Status found for this RNTI " << (*itTimers).first);
                }
              (*itStat).second.at (i) = 0;
              (*itTimers).second.at (i) = 0;
            }
          else
            {
              (*itTimers).second.at (i)++;
            }
        }
    }

}


void
cv2x_PriorityFfMacScheduler::DoSchedDlTriggerReq (const struct cv2x_FfMacSchedSapProvider::SchedDlTriggerReqParameters& params)
{

 NS_LOG_FUNCTION (this << " Frame no. " << (params.m_sfnSf >> 4) << " subframe no. " << (0xF & params.m_sfnSf));
  // API generated by RLC for triggering the scheduling of a DL subframe
  // evaluate the relative channel quality indicator for each UE per each RBG
  // (since we are using allocation type 0 the small unit of allocation is RBG)
  // Resource allocation type 0 (see sec 7.1.6.1 of 36.213)
  
  /**************************************** Priority-based DL Scheduler  ***************************************/
  /* - A DL Scheduler that prioritizes flows based on their QCI Value                                          */
  /* - Each QCI Value is mapped to a priority value                                                            */
  /* - The flow having the highest priority ( little value of P ) will be prioritized                          */
  /* - Only required RBGs are assigned to each flow to transmit it pending data                                */
  /*___________________________________________________________________________________________________________*/ 
 
 RefreshDlCqiMaps ();

  int rbgSize = GetRbgSize (m_cschedCellConfig.m_dlBandwidth);
  int numberOfRBGs = m_cschedCellConfig.m_dlBandwidth / rbgSize;
  
  std::vector <bool> rbgMap;  // global RBGs map
  uint16_t rbgAllocatedNum = 0;
  std::set <uint16_t> rntiAllocated;
  rbgMap.resize (m_cschedCellConfig.m_dlBandwidth / rbgSize, false);

  rbgMap = m_ffrSapProvider->GetAvailableDlRbg ();
  for (std::vector<bool>::iterator it = rbgMap.begin (); it != rbgMap.end (); it++)
    {
      if ((*it) == true )
        {
          rbgAllocatedNum++;
        }
    }

  cv2x_FfMacSchedSapUser::SchedDlConfigIndParameters ret;

  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > allocationMapPerRntiPerLCId;   //allocation map per rnti per LC 
  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itMap;
  allocationMapPerRntiPerLCId.clear ();

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
      NS_ASSERT_MSG (m_amc->GetTbSizeFromMcs (m_ulGrantMcs, m_cschedCellConfig.m_ulBandwidth) > (*itRach).m_estimatedSize, " Default UL Grant MCS does not allow to send RACH messages");
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
          tbSizeBits = m_amc->GetTbSizeFromMcs (m_ulGrantMcs, rbLen);
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
      NS_LOG_INFO (this << " UL grant allocated to RNTI " << (*itRach).m_rnti << " rbStart " << rbStart << " rbLen " << rbLen << " MCS " << m_ulGrantMcs << " tbSize " << newRar.m_grant.m_tbSize);
      for (uint16_t i = rbStart; i < rbStart + rbLen; i++)
        {
          m_rachAllocationMap.at (i) = (*itRach).m_rnti;
        }
      rbStart = rbStart + rbLen;
      
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
          std::map <uint16_t, UlHarqProcessesDciBuffer_t>::iterator itDci = m_ulHarqProcessesDciBuffer.find (uldci.m_rnti);
          if (itDci == m_ulHarqProcessesDciBuffer.end ())
            {
              NS_FATAL_ERROR ("Unable to find RNTI entry in UL DCI HARQ buffer for RNTI " << uldci.m_rnti);
            }
          (*itDci).second.at (harqId) = uldci;
        }
      
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
          std::map <uint16_t, DlHarqProcessesDciBuffer_t>::iterator itHarq = m_dlHarqProcessesDciBuffer.find (rnti);
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
              NS_LOG_INFO ("Maximum number of retransmissions reached -> drop process");
              std::map <uint16_t, DlHarqProcessesStatus_t>::iterator it = m_dlHarqProcessesStatus.find (rnti);
              if (it == m_dlHarqProcessesStatus.end ())
                {
                  NS_LOG_ERROR ("No info find in HARQ buffer for UE (might change eNB) " << m_dlInfoListBuffered.at (i).m_rnti);
                }
              (*it).second.at (harqId) = 0;
              std::map <uint16_t, DlHarqRlcPduListBuffer_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduListBuffer.find (rnti);
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
              uint8_t rbgId = (dciRbg.at (dciRbg.size () - 1) + 1) % numberOfRBGs;
              uint8_t startRbg = dciRbg.at (dciRbg.size () - 1);
              std::vector <bool> rbgMapCopy = rbgMap;
              while ((j < dciRbg.size ())&&(startRbg != rbgId) )    //change third condition : occurs when changing MIMO Model; && (rbgId < numberOfRBGs)
                {
                  if (rbgMapCopy.at (rbgId) == false)
                    {
                      rbgMapCopy.at (rbgId) = true;
                      dciRbg.at (j) = rbgId;
                      j++;
                    }
                  rbgId++;
                }
              if (j == dciRbg.size ())
                {
                  // find new RBGs -> update DCI map
                  uint32_t rbgMask = 0;
                  for (uint16_t k = 0; k < dciRbg.size (); k++)
                    {
                      rbgMask = rbgMask + (0x1 << dciRbg.at (k));
                      rbgAllocatedNum++;
                    }
                  dci.m_rbBitmap = rbgMask;
                  rbgMap = rbgMapCopy;
                  NS_LOG_INFO (this << " Move retx in RBGs " << dciRbg.size ());
                }
              else
                {
                  // HARQ retx cannot be performed on this TTI -> store it
                  dlInfoListUntxed.push_back (params.m_dlInfoList.at (i));
                  NS_LOG_INFO (this << " No resource for this retx -> buffer it");
                }
            }
          // retrieve RLC PDU list for retx TBsize and update DCI
          cv2x_BuildDataListElement_s newEl;
          std::map <uint16_t, DlHarqRlcPduListBuffer_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduListBuffer.find (rnti);
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
                          rlcPduListPerLc.push_back ((*itRlcPdu).second.at (j).at (dci.m_harqProcess).at (k));
                        }
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
          NS_LOG_INFO (this << " HARQ received ACK for UE " << m_dlInfoListBuffered.at (i).m_rnti);
          std::map <uint16_t, DlHarqProcessesStatus_t>::iterator it = m_dlHarqProcessesStatus.find (m_dlInfoListBuffered.at (i).m_rnti);
          if (it == m_dlHarqProcessesStatus.end ())
            {
              NS_FATAL_ERROR ("No info find in HARQ buffer for UE " << m_dlInfoListBuffered.at (i).m_rnti);
            }
          (*it).second.at (m_dlInfoListBuffered.at (i).m_harqProcessId) = 0;
          std::map <uint16_t, DlHarqRlcPduListBuffer_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduListBuffer.find (m_dlInfoListBuffered.at (i).m_rnti);
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


  std::vector <cv2x_LteFlowId_t> satisfiedFlows;  //structure to save the satisfied flows: flows that have obtained the required RBGs to transmit their pending data  
  double bytesTxedF = 0;                         // accumulated data for each flows             
  int mcsF=0;                                    //MCS to calculate accumulated data   
 
  std::vector <uint16_t> tempV;  // vector to save the RBGs that are assigned to each flow 

  for (int i = 0; i < numberOfRBGs; i++)
    {
      NS_LOG_INFO (this << " ALLOCATION for RBG " << i << " of " << numberOfRBGs);
     
      if (rbgMap.at (i) == false)
        {
          std::map <cv2x_LteFlowId_t,struct cv2x_LogicalChannelConfigListElement_s>::iterator itLogicalChannels;
          std::map <cv2x_LteFlowId_t,struct cv2x_LogicalChannelConfigListElement_s>::iterator itMax = m_ueLogicalChannelsConfigList.end ();

          int PriorityMin=10;
          
          for (itLogicalChannels = m_ueLogicalChannelsConfigList.begin (); itLogicalChannels != m_ueLogicalChannelsConfigList.end (); itLogicalChannels++)
            {
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
                continue;
              }

              cv2x_LteFlowId_t flowId1 ;
              flowId1.m_rnti= itLogicalChannels->first.m_rnti;
              flowId1.m_lcId= itLogicalChannels->second.m_logicalChannelIdentity;
              std::vector <cv2x_LteFlowId_t>::iterator Ft;
              bool findF=false;

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
               // this flow is not present in the satisfied flows --> it needs more RBGs to transmit data 
               int QciValue=(int)itLogicalChannels->second.m_qci;  // QCI value of this flows 
               std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itQ;
               cv2x_LteFlowId_t flowId = cv2x_LteFlowId_t (itLogicalChannels->first.m_rnti, itLogicalChannels->second.m_logicalChannelIdentity);
               itQ = m_rlcBufferReq.find(flowId);
               if (itQ!=m_rlcBufferReq.end()) //this flow has data to send 
                {
                if ((*itQ).second.m_rlcTransmissionQueueSize > 0) 
                 {
                   int Priority=0;  // priority value obtained based on the QCI value 
                   switch (QciValue)
                    {
                      case 1:
                         Priority=2;
                         break;
                      case 2:
                         Priority=4;
                         break;
                      case 3:
                         Priority=3;
                         break;
                      case 4:
                         Priority=5;
                         break;
                      case 5:
                         Priority=1;
                         break;
                      case 6:
                         Priority=6;
                         break;
                      case 7:
                         Priority=7;
                         break;
                      case 8:
                         Priority=8;
                         break;
                      case 9:
                         Priority=9;
                         break;
                    }
                   if ( Priority < PriorityMin)
                    {
                      PriorityMin= Priority;
                      itMax=itLogicalChannels;
                    }
                } //end if LcActiveperFlow
               } //end if itQ!=m_rlcBufferReq.end()
              }  //end findF==false

             } //end for itLogicalchannels

           if (itMax == m_ueLogicalChannelsConfigList.end ())
            {
              // no UE available for this RB
              NS_LOG_INFO (this << " any UE found");
            }
           else
            {
              // insert the flow in the allocation map 

              std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itMap;      
              itMap = allocationMapPerRntiPerLCId.find ((uint16_t)(itMax->first.m_rnti));
              if (itMap == allocationMapPerRntiPerLCId.end ())        // the flow doesn't exist in the allocation map --> create new entry with the rnti id and the lc id 
               {
                 // insert new element
                 std::map<uint8_t, std::vector <uint16_t> > tempMap;  
                 tempV.push_back(i);
                 tempMap.insert (std::pair<uint8_t, std::vector <uint16_t> > (itMax->second.m_logicalChannelIdentity, tempV));
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
                 
                    bytesTxedF=(m_amc->GetTbSizeFromMcs (mcsF, b * rbgSize) / 8) * nLayer;   //multiply by nLayer for MIMO 
                    std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itQF;
                    cv2x_LteFlowId_t flowIdF = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->second.m_logicalChannelIdentity);
                    itQF = m_rlcBufferReq.find(flowIdF);
                    //std::cout<<" \t first allocation rnti = "<<(int)flowIdF.m_rnti<<" lcid = "<<(int) flowIdF.m_lcId<< " data to transmit = " <<(int)(*itQF).second.m_rlcTransmissionQueueSize <<" allocated without n = " <<(int)(m_amc->GetTbSizeFromMcs (mcsF, b * rbgSize) / 8)<<" allocated with n = " <<(int) bytesTxedF<<std::endl;
                       
                    if ((*itQF).second.m_rlcTransmissionQueueSize <= bytesTxedF )
                     {
                        //this flow is  satisfied with this first allocation --> add it to satisfied flows 
                        satisfiedFlows.push_back(flowIdF);
                     }

                    /***********************************************************************************/

               }
              else                                                    // the rnti id already exist in the allocation map --> try to see if the lc id already exists or not 
               {
                 std::map <uint8_t, std::vector <uint16_t> >::iterator itSMap;
                 itSMap=itMap->second.find ((uint8_t)itMax->second.m_logicalChannelIdentity);
                 if (itSMap == itMap->second.end ())                  //  the lcid does't exist in the allocation map --> create new entry for it 
                  {
                    //insert new flow id element 
                    tempV.push_back(i);         
                    itMap->second.insert (std::pair<uint8_t, std::vector <uint16_t> > (itMax->second.m_logicalChannelIdentity, tempV));
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
                 
                       bytesTxedF=(m_amc->GetTbSizeFromMcs (mcsF, b * rbgSize) / 8) * nLayer;   //multiply by nLayer for MIMO 
                       std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itQF;
                       cv2x_LteFlowId_t flowIdF = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->second.m_logicalChannelIdentity);
                       itQF = m_rlcBufferReq.find(flowIdF);
                       //std::cout<<" \t first allocation rnti = "<<(int)flowIdF.m_rnti<<" lcid = "<<(int) flowIdF.m_lcId<< " data to transmit = " <<(int)(*itQF).second.m_rlcTransmissionQueueSize <<" allocated without n = " <<(int)(m_amc->GetTbSizeFromMcs (mcsF, b * rbgSize) / 8)<<" allocated with n = " <<(int) bytesTxedF<<std::endl;
                       
                       if ((*itQF).second.m_rlcTransmissionQueueSize <= bytesTxedF )
                        {
                           //this flow is  satisfied with this first allocation --> add it to satisfied flows 
                           satisfiedFlows.push_back(flowIdF);
                        }

                    /***********************************************************************************/
                   

                  }
                  else                                              // the lc id is already in the allocation map --> try to see if it needs more RBGs to transmit it data or not 
                   {
                     // added for MIMO layers
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
                 
                     bytesTxedF=(m_amc->GetTbSizeFromMcs (mcsF, b * rbgSize) / 8) * nLayer;   //multiply by nLayer for MIMO 
                     std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itQF;
                     cv2x_LteFlowId_t flowIdF = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->second.m_logicalChannelIdentity);
                     itQF = m_rlcBufferReq.find(flowIdF);
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
                         }                         
                      }
                     else 
                      { 
                        (*itSMap).second.push_back (i);
                        rbgMap.at (i) = true;
                      }

                    }   //end if itSMap == itMap->second.end ()
                 }  //end if itMap == allocationMapPerRntiPerLCId.end ()
                NS_LOG_INFO (this << " UE assigned " << (uint16_t)(itLogicalChannels->first.m_rnti));
             }
        } // end for RBG free
    } // end for RBGsm_ulGrantMcs

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


  // generate the transmission opportunities by grouping the RBGs of the same RNTI and
  // creating the correspondent DCIs
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

     // display queue size of each flow 
     std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;
     //it = m_rlcBufferReq.begin();

     std::map <uint16_t,uint8_t>::iterator itTxMode;
     itTxMode = m_uesTxMode.find ((*itMap).first);
     if (itTxMode == m_uesTxMode.end ())
      {
        NS_FATAL_ERROR ("No Transmission Mode info on user " << (*itMap).first);
      }
     int nLayer = cv2x_TransmissionModesLayers::TxMode2LayerNum ((*itTxMode).second);

     // find the ncs of this flow 
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
   
     //calculate RgbPerRnti for each flow 
     std::map <uint8_t, std::vector <uint16_t> > MapAux;
     MapAux=itMap->second;
     std::map <uint8_t, std::vector <uint16_t> >::iterator itFF=MapAux.begin();
     uint16_t RgbPerRnti=0;
     for (itFF=MapAux.begin();itFF!=MapAux.end();itFF++)
      {
        RgbPerRnti=RgbPerRnti+(*itFF).second.size();
      }

     // calculate the transport block size per rnti 
     for (uint8_t j = 0; j < nLayer; j++)
      {
        int tbSize = (m_amc->GetTbSizeFromMcs (newDci.m_mcs.at (j), RgbPerRnti * rbgSize) / 8);
        newDci.m_tbsSize.push_back (tbSize);
      }

     newDci.m_resAlloc = 0;  // only allocation type 0 at this stage
     newDci.m_rbBitmap = 0; // TBD (32 bit bitmap see 7.1.6 of 36.213)
     uint32_t rbgMask = 0;
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
              for (uint8_t j = 0; j < nLayer; j++)
                {
                  cv2x_RlcPduListElement_s newRlcEl;     // RLC element per flow 
                  newRlcEl.m_logicalChannelIdentity = (*itBufReq).first.m_lcId;
                  newRlcEl.m_size = (m_amc->GetTbSizeFromMcs (newDci.m_mcs.at (j), RgbPerFlow * rbgSize) / 8);   //transport block for this flow 
                  NS_LOG_INFO (this << " LCID " << (uint32_t) newRlcEl.m_logicalChannelIdentity << " size " << newRlcEl.m_size << " layer " << (uint16_t)j);
                  newRlcPduLe.push_back (newRlcEl);
                  UpdateDlRlcBufferInfo (newDci.m_rnti, newRlcEl.m_logicalChannelIdentity, newRlcEl.m_size);
                  
                  if (m_harqOn == true)
                    {
                      // store RLC PDU list for HARQ
                      std::map <uint16_t, DlHarqRlcPduListBuffer_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduListBuffer.find ((*itMap).first);
                      if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end ())
                        {
                          NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << (*itMap).first);
                        }
                      (*itRlcPdu).second.at (j).at (newDci.m_harqProcess).push_back (newRlcEl);
                    }
                } //end for 
              newEl.m_rlcPduList.push_back (newRlcPduLe);
            } // end if buffer 
          
        } // end for each flow 

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
        std::map <uint16_t, DlHarqProcessesDciBuffer_t>::iterator itDci = m_dlHarqProcessesDciBuffer.find (newEl.m_rnti);
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
  m_schedSapUser->SchedDlConfigInd (ret);

  return;
}

void
cv2x_PriorityFfMacScheduler::DoSchedDlRachInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedDlRachInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);

  m_rachList = params.m_rachList;

  return;
}

void
cv2x_PriorityFfMacScheduler::DoSchedDlCqiInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  m_ffrSapProvider->ReportDlCqiInfo (params);

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
              // update the CQI value and refresh correspondent timer
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
          std::map <uint16_t,cv2x_SbMeasResult_s>::iterator it;
          uint16_t rnti = params.m_cqiList.at (i).m_rnti;
          it = m_a30CqiRxed.find (rnti);
          if (it == m_a30CqiRxed.end ())
            {
              // create the new entry
              m_a30CqiRxed.insert ( std::pair<uint16_t, cv2x_SbMeasResult_s > (rnti, params.m_cqiList.at (i).m_sbMeasResult) );
              m_a30CqiTimers.insert ( std::pair<uint16_t, uint32_t > (rnti, m_cqiTimersThreshold));
            }
          else
            {
              // update the CQI value and refresh correspondent timer
              (*it).second = params.m_cqiList.at (i).m_sbMeasResult;
              std::map <uint16_t,uint32_t>::iterator itTimers;
              itTimers = m_a30CqiTimers.find (rnti);
              (*itTimers).second = m_cqiTimersThreshold;
            }
        }
      else
        {
          NS_LOG_ERROR (this << " CQI type unknown");
        }
    }

  return;
}


double
cv2x_PriorityFfMacScheduler::EstimateUlSinr (uint16_t rnti, uint16_t rb)
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
      int sinrNum = 0;
      for (uint32_t i = 0; i < m_cschedCellConfig.m_ulBandwidth; i++)
        {
          double sinr = (*itCqi).second.at (i);
          if (sinr != NO_SINR)
            {
              sinrSum += sinr;
              sinrNum++;
            }
        }
      double estimatedSinr = (sinrNum > 0) ? (sinrSum / sinrNum) : DBL_MAX;
      // store the value
      (*itCqi).second.at (rb) = estimatedSinr;
      return (estimatedSinr);
    }
}

void
cv2x_PriorityFfMacScheduler::DoSchedUlTriggerReq (const struct cv2x_FfMacSchedSapProvider::SchedUlTriggerReqParameters& params)
{
/*********************************** Priority based UL scheduler **************************************************/
  /*   - Basic scheduler that assign RBs to the flow having the highest priority                                    */
  /*   - Only required RBs to transmit data are assigned to each flow                                               */
  /*   - Priority of each flow is based on the QCI value of each bearer                                             */
  /*________________________________________________________________________________________________________________*/


  // Make sure we reset the allocation map in case it is present. This can happen if CQI are reported
  // by PUSCH and the eNode B did not receive any packet from the UE (due toloss for example)
  std::map <uint16_t, std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > >::iterator itMap = m_allocationMaps.find (params.m_sfnSf);
  if (itMap != m_allocationMaps.end ()) 
   {
    m_allocationMaps.erase (itMap);
   }

  NS_LOG_FUNCTION (this << " UL - Frame no. " << (params.m_sfnSf >> 4) << " subframe no. " << (0xF & params.m_sfnSf) << " size " << params.m_ulInfoList.size ());

  uint16_t rbAllocatedNum = 0;
  RefreshUlCqiMaps ();
  uint16_t rbNum= m_cschedCellConfig.m_ulBandwidth;
  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > allocationMapPerRntiPerLCId;  //rbg allocation map to allocate the flows in UL 
  cv2x_FfMacSchedSapUser::SchedUlConfigIndParameters ret;
  std::vector <bool> rbMap;
  std::set <uint16_t> rntiAllocated;
  m_rachAllocationMap.clear ();
  m_rachAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);

  rbMap.resize (m_cschedCellConfig.m_ulBandwidth, false);
  
  //fill the rbmap with true value if there are some RBGs already allocated for HARQ
  std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itTemp1 = allocationMapPerRntiPerLCId.begin ();
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
           std::map <uint16_t, UlHarqProcessesDciBuffer_t>::iterator itHarq = m_ulHarqProcessesDciBuffer.find (rnti);
           if (itHarq == m_ulHarqProcessesDciBuffer.end ())
            {
              NS_LOG_ERROR ("No info find in HARQ buffer for UE (might change eNB) " << rnti);
              continue;
            }
           cv2x_UlDciListElement_s dci = (*itHarq).second.at (harqId);
           std::map <uint16_t, UlHarqProcessesStatus_t>::iterator itStat = m_ulHarqProcessesStatus.find (rnti);
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
                     tempMap.insert (std::pair<uint8_t, std::vector <uint16_t> > (0, tempV));
                     allocationMapPerRntiPerLCId.insert (std::pair <uint16_t, std::map<uint8_t, std::vector <uint16_t> > > (dci.m_rnti, tempMap));                     
                     tempV.clear();
                   }
                  else
                   {
                     std::map <uint8_t, std::vector <uint16_t> >::iterator itSMap;
                     itSMap=itMap->second.find (0);
                     if (itSMap == itMap->second.end ())
                      {
                        //insert new flow id element 
                        tempV.push_back(j);  
                        itMap->second.insert (std::pair<uint8_t, std::vector <uint16_t> > (0, tempV));
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



  std::vector <bool> rbMap1=rbMap;                     // this is to conserve the rbMap to use it in the DCI construction 
  std::vector <cv2x_LteFlowId_t> satisfiedFlows;        //structure to save satisfied flows 
  double bytesTxedF = 0;                               // accumulated data for each flows             

  std::map <cv2x_LteFlowId_t,struct cv2x_LogicalChannelConfigListElement_s>::iterator itLogicalChannels;

  std::vector <uint16_t> tempV;   // vector to store the RBs for each flow
  int i=0;
  while (i<rbNum) 
   {
     NS_LOG_INFO (this << " ALLOCATION for RB " << i << " of " << rbNum);
     //std::cout<<" ALLOCATION for RB "<<(int) i << " of "<<(int)rbNum<<std::endl;
     if (rbMap.at (i) == false)
      {
        std::map <cv2x_LteFlowId_t,struct cv2x_LogicalChannelConfigListElement_s>::iterator itLogicalChannels;
        std::map <cv2x_LteFlowId_t,struct cv2x_LogicalChannelConfigListElement_s>::iterator itMax = m_ueLogicalChannelsConfigList.end ();

        int PriorityMin=10;            // The minimum priority of all present flows, initialised by 10 

        for (itLogicalChannels = m_ueLogicalChannelsConfigList.begin (); itLogicalChannels != m_ueLogicalChannelsConfigList.end (); itLogicalChannels++)
         {
           std::set <uint16_t>::iterator itRnti = rntiAllocated.find (itLogicalChannels->first.m_rnti);
          
           /********************** not allocate if MCS == 0 ******************************************************************* 
           int MCSTest=0;    
           std::map <uint16_t,uint8_t>::iterator itTxModeT;
           itTxModeT = m_uesTxMode.find (itLogicalChannels->first.m_rnti);
           if (itTxModeT == m_uesTxMode.end ())
            {
              NS_FATAL_ERROR ("No Transmission Mode info on user " << itLogicalChannels->first.m_rnti);
            }
           int nLayer = cv2x_TransmissionModesLayers::TxMode2LayerNum ((*itTxModeT).second);  // nbr of layers for MIMO transmissions           
           std::cout<<std::endl;
           std::map <uint16_t, std::vector <double> >::iterator itCqiT = m_ueCqi.find (itLogicalChannels->first.m_rnti);  
           int cqiT = 0;
           std::vector <uint8_t> sbCqiT;
           if (itCqiT == m_ueCqi.end ())
            {
              for (uint8_t j = 0; j < nLayer; j++) 
               {
                 sbCqiT.push_back (1);  // start with lowest value
               }
            }
           else 
            {
              double MaxSinr = -4096;
              for (unsigned int k=0; k< (*itCqiT).second.size();k++)
               {
                 if (MaxSinr < (*itCqiT).second.at(k))
                  MaxSinr=(*itCqiT).second.at(k);
               }          
              //  translate SINR -> cqi: WILD ACK: same as DL
              double s = log2 ( 1 + (
                       std::pow (10, MaxSinr / 10 )  /
                       ( (-std::log (5.0 * 0.00005 )) / 1.5) ));

              cqiT = m_amc->GetCqiFromSpectralEfficiency (s);
              sbCqiT.push_back (cqiT);
             }
             
            uint8_t cqi1T = sbCqiT.at (0);
            std::cout<<"Cqi1 = "<<(int) cqi1T<<std::endl;
            uint8_t cqi2T = 1;
            // std::map <uint16_t,uint32_t>::iterator itUl=m_ceBsrRxed.find(*it);
            if (sbCqiT.size () > 1)
             {
               cqi2T = sbCqiT.at (1);
             }
            if ((cqi1T > 0)||(cqi2T > 0)) // CQI == 0 means "out of range" (see table 7.2.3-1 of 36.213)
             {
               std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itUl;
               itUl=m_ceBsrRxed.find(itLogicalChannels->first.m_rnti);
               if ( itUl!=m_ceBsrRxed.end() )
                {
                  std::map <uint8_t, uint32_t> mapUl=itUl->second;
                  std::map <uint8_t, uint32_t>:: iterator itmapUl;
                  int dataUl=0;
                  for (itmapUl=mapUl.begin(); itmapUl!=mapUl.end(); itmapUl++)
                   {
                     dataUl=dataUl+(*itmapUl).second;
                   } 
                  if (dataUl > 0)     //
                   {                          //
                     // this UE has data to transmit    
                     for (uint8_t k = 0; k < nLayer; k++)
                      {
                        if (sbCqiT.size () > k)
                         {
                           std::cout<<"k="<<(int) k<<std::endl;
                           std::cout<<"sbcqi="<<(int)sbCqiT.at (k)<<std::endl;
                           MCSTest = m_amc->GetMcsFromCqi (sbCqiT.at (k));
                           std::cout<<" MCSTest= "<<(int)MCSTest<<std::endl;
                          }
                         else
                          {
                            // no info on this subband -> worst MCS
                            MCSTest = 0;
                            std::cout<<" MCSTest: 0"<<std::endl;
                          }
                      }
                    }                          //
                 } // end if itUl!=m_ceBsrRxed.end()
               }
              if (MCSTest== 0 ) 
               { 
                 std::cout<<"*MCSTest=0"<<std::endl;
                 continue;
               }
          **********************************************************************************************************************/           

           cv2x_LteFlowId_t flowId1 ;      //trying to find this flow in the satisfied flows 
           flowId1.m_rnti= itLogicalChannels->first.m_rnti;
           flowId1.m_lcId= itLogicalChannels->second.m_logicalChannelIdentity;
           std::vector <cv2x_LteFlowId_t>::iterator Ft;
           bool findF=false;
           for(Ft=satisfiedFlows.begin();Ft!=satisfiedFlows.end();Ft++)
            {
              if(((*Ft).m_rnti==flowId1.m_rnti) && ((*Ft).m_lcId==flowId1.m_lcId))
               {
                 findF=true;
                 break;
               }
            }
           if (findF==false)    // this flow is nor present in the satisfied flows --> we can assign it the required RBs 
            {
              //std::cout<<" rnti = "<<(int) flowId1.m_rnti<< " lcid = "<<(int)flowId1.m_lcId<<std::endl;
              int QciValue=(int)itLogicalChannels->second.m_qci;  
              // try to find this flow in the m_ceBsrRxed
              std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator it;                  
              it=m_ceBsrRxed.find(itLogicalChannels->first.m_rnti);
              if (it!=m_ceBsrRxed.end())
               {
                 std::map <uint8_t, uint32_t> mapLC=it->second;
                 std::map <uint8_t, uint32_t>::iterator itLC=mapLC.find(itLogicalChannels->second.m_logicalChannelIdentity); // trying to find this LC in mapLC 
                 if (itLC!=mapLC.end())      
                  {
                    if ((*itLC).second > 0)  // this flow has data to send
                     {         
                       int Priority=0; 
                       switch (QciValue)
                        {
                          case 1:
                            Priority=2;
                            break;
                          case 2:
                            Priority=4;
                            break;
                          case 3:
                            Priority=3;
                            break;
                          case 4:
                            Priority=5;
                            break;
                          case 5:
                            Priority=1;
                            break;
                          case 6:
                            Priority=6;
                            break;
                          case 7:
                            Priority=7;
                            break;
                          case 8:
                            Priority=8;
                            break;
                          case 9:
                            Priority=9;
                            break;
                         }

                       if ( Priority < PriorityMin)
                        {
                          PriorityMin= Priority;
                          itMax=itLogicalChannels;
                        }

                      } //((*itLC).second > 0
                    } //itLC!=mapLC.end()
                  } // end if it!=m_ceBsrRxed.end()
                }  //end findF==false

              } //end for itLogicalchannels
              
            if (itMax == m_ueLogicalChannelsConfigList.end ())
             {
               // no UE available for this RB
               NS_LOG_INFO (this << " any UE found");
             }
            else
             {
               //std::cout<<" priority max , rnti = "<<(int)itMax->first.m_rnti<<" lcid = "<<(int)itMax->second.m_logicalChannelIdentity<<std::endl;
               // insert this flow in the allocation map : allocationMapPerRntiPerLCId
               std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itMap;
               itMap = allocationMapPerRntiPerLCId.find ((uint16_t)(itMax->first.m_rnti));
            
               if (itMap == allocationMapPerRntiPerLCId.end ())
                {
                  // insert new element
                  //std::cout<<" first insert of this rnti "<<std::endl;
                  std::map<uint8_t, std::vector <uint16_t> > tempMap;          
                  tempV.push_back(i);
                  tempMap.insert (std::pair<uint8_t, std::vector <uint16_t> > (itMax->second.m_logicalChannelIdentity, tempV));
                  allocationMapPerRntiPerLCId.insert (std::pair <uint16_t, std::map<uint8_t, std::vector <uint16_t> > > (itMax->first.m_rnti, tempMap));
                  rbMap.at (i) = true; 
                  tempV.clear();
                }
               else
                {

                  std::map <uint8_t, std::vector <uint16_t> >::iterator itSMap;
                  itSMap=itMap->second.find ((uint8_t)itMax->second.m_logicalChannelIdentity);
                  if (itSMap == itMap->second.end ())
                   {
                     //insert new flow id element 
                     //std::cout<<" first insert of this lcid "<<std::endl;
                     tempV.push_back(i);
                       
                     itMap->second.insert (std::pair<uint8_t, std::vector <uint16_t> > (itMax->second.m_logicalChannelIdentity, tempV));
                     rbMap.at (i) = true;
                      
                     tempV.clear();
                   }
                  else 
                   { 
                     int b=1;
                     b=b+(*itSMap).second.size(); 
                     std::map <uint16_t, std::vector <double> >::iterator itCqi = m_ueCqi.find (itMax->first.m_rnti);  
                     int cqi = 0;
                     int mcsF=0;
                     if (itCqi == m_ueCqi.end ())
                      {
                         cqi = 1 ;  // start with lowest value
                      }
                     else 
                      {
                        std::vector <double> vec=(*itCqi).second;
                        sort( vec.begin(), vec.end() );
                        vec.erase( unique( vec.begin(), vec.end() ), vec.end() );
                        double max_element=0;
                        int max_occurance =0;
                        for ( unsigned int k=0; k < vec.size(); k++)
                         {
                           int repeated=0;
                           for (unsigned int j=0; j < (*itCqi).second.size() ; j++ ) 
                            {
                              if (vec.at(k)== (*itCqi).second.at(j) )
                               { 
                                 repeated++;
                               }    
                            }
                           if ( repeated > max_occurance )
                            {
                              max_occurance = repeated;
                              max_element= vec.at(k);
                            }
                         }
                        double s = log2 ( 1 + (
                        std::pow (10, max_element / 10 )  /
                          ( (-std::log (5.0 * 0.00005 )) / 1.5) ));

                        cqi = m_amc->GetCqiFromSpectralEfficiency (s);
                      }
             
                     if (cqi > 0) // CQI == 0 means "out of range" (see table 7.2.3-1 of 36.213)
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
                            mcsF = m_amc->GetMcsFromCqi (cqi);
                         }                       
                      }
                     //std::cout <<"At time " << Simulator::Now ().GetSeconds ()<< "s  RNTI = "<<(int) itMax->first.m_rnti<< " MCS = "<<(int)mcsF<<std::endl;
                     bytesTxedF=(m_amc->GetTbSizeFromMcs (mcsF, b * 1) / 8) ;
                     cv2x_LteFlowId_t flowIdF = cv2x_LteFlowId_t (itMax->first.m_rnti, itMax->second.m_logicalChannelIdentity);
                     // try to find this flow in the m_ceBsrRxed
                     std::multimap <uint16_t, std::map <uint8_t, uint32_t> >:: iterator itTemp;
                     itTemp=m_ceBsrRxed.find(itMax->first.m_rnti);    
                     std::map <uint8_t, uint32_t> mapLCTemp=itTemp->second;
                     std::map <uint8_t, uint32_t>::iterator itLCTemp=mapLCTemp.find(itMax->second.m_logicalChannelIdentity);

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
                              satisfiedFlows.push_back(flowIdF);
                            }
                      }
                     else 
                      {   
                        (*itSMap).second.push_back (i);
                        rbMap.at (i) = true;
                      }
                    }   //end if itSMap == itMap->second.end ()
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
  std::multimap <uint16_t, std::map <uint8_t, uint32_t> >::iterator it;
  int nflows = 0;
  std::vector<uint16_t> RntiAssigned;
  
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
      if (ret.m_dciList.size () > 0)
        {
          m_allocationMaps.insert (std::pair <uint16_t, std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > > (params.m_sfnSf, allocationMapPerRntiPerLCId));
          m_schedSapUser->SchedUlConfigInd (ret);
        }
      return;  // no flows to be scheduled
    }

   std::map <uint16_t, cv2x_PrioritysFlowPerf_t>::iterator itStats;
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
      // RBs per RNTI :
      rbRnti=0;
      std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itTmp1 = allocationMapPerRntiPerLCId.find ((*it).first);
      if (itTmp1!=allocationMapPerRntiPerLCId.end())
       {
        std::map <uint8_t, std::vector <uint16_t> > mapFlow1=itTmp1->second;
        std::map <uint8_t, std::vector <uint16_t> >::iterator itTmp2;
        for (itTmp2=mapFlow1.begin(); itTmp2!=mapFlow1.end(); itTmp2++)
         {
           rbRnti=rbRnti+ (*itTmp2).second.size();
         }
       }
      if ((itRnti != rntiAllocated.end ())||(data == 0) || (rbRnti==0))
       {
         // UE already allocated for UL-HARQ -> skip it
         //std::cout<<" entere in the loop "<<std::endl;
         it++;
         if (it == m_ceBsrRxed.end ())
          {
            // restart from the first
            it = m_ceBsrRxed.begin ();
           
            rbRnti=0;
            std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itTmp1 = allocationMapPerRntiPerLCId.find ((*it).first);
            if (itTmp1!=allocationMapPerRntiPerLCId.end())
             {
               std::map <uint8_t, std::vector <uint16_t> > mapFlow1=itTmp1->second;
               std::map <uint8_t, std::vector <uint16_t> >::iterator itTmp2;
               for (itTmp2=mapFlow1.begin(); itTmp2!=mapFlow1.end(); itTmp2++)
                {
                  rbRnti=rbRnti+ (*itTmp2).second.size();
                }
             }
          }

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
      cv2x_UlDciListElement_s uldci;
      uldci.m_rnti = (*it).first;
      uldci.m_rbLen = rbRnti;
      bool allocated = false;
      while ((!allocated)&&((rbAllocated + rbRnti - m_cschedCellConfig.m_ulBandwidth) < 1) && (rbRnti != 0))
       {
         // check availability
         bool free = true;
         //std::cout<<"rbAllocated ="<<(int)rbAllocated<<std::endl;
          
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
         std::vector <double> vec=(*itCqi).second;
         sort( vec.begin(), vec.end() );
         vec.erase( unique( vec.begin(), vec.end() ), vec.end() );
         double max_element=0;
         int max_occurance =0;
         for ( unsigned int k=0; k < vec.size(); k++)
           {
             int repeated=0;
             for (unsigned int j=0; j < (*itCqi).second.size() ; j++ ) 
              {
                if (vec.at(k)== (*itCqi).second.at(j) )
                  { 
                    repeated++;
                  }    
              }
             if ( repeated > max_occurance )
              {
                 max_occurance = repeated;
                 max_element= vec.at(k);
              }
           }
          double s = log2 ( 1 + (
          std::pow (10, max_element / 10 )  /
                   ( (-std::log (5.0 * 0.00005 )) / 1.5) ));

          cqi = m_amc->GetCqiFromSpectralEfficiency (s);
          
         uldci.m_mcs = m_amc->GetMcsFromCqi (cqi);
       } // end 
        
     uldci.m_tbSize = (m_amc->GetTbSizeFromMcs (uldci.m_mcs, rbRnti) / 8); // MCS 0 -> UL-AMC TBD
     
     std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > >::iterator itTmpBuffer = allocationMapPerRntiPerLCId.find ((*it).first);
      if (itTmpBuffer!=allocationMapPerRntiPerLCId.end())
       {
        int rbFlow=0;
        std::map <uint8_t, std::vector <uint16_t> > mapFlow1=itTmpBuffer->second;
        std::map <uint8_t, std::vector <uint16_t> >::iterator itTmpBuffer2;
        for (itTmpBuffer2=mapFlow1.begin(); itTmpBuffer2!=mapFlow1.end(); itTmpBuffer2++)
         {
           rbFlow=(*itTmpBuffer2).second.size();
           UpdateUlRlcBufferInfo (uldci.m_rnti, (*itTmpBuffer2).first, (m_amc->GetTbSizeFromMcs (uldci.m_mcs, rbFlow) / 8));    
         }
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
        std::map <uint16_t, UlHarqProcessesDciBuffer_t>::iterator itDci = m_ulHarqProcessesDciBuffer.find (uldci.m_rnti);
        if (itDci == m_ulHarqProcessesDciBuffer.end ())
         {
            NS_FATAL_ERROR ("Unable to find RNTI entry in UL DCI HARQ buffer for RNTI " << uldci.m_rnti);
         }
           (*itDci).second.at (harqId) = uldci;
      } // end if (m_harqOn == true)
      

      // update TTI  UE stats
      itStats = m_flowStatsUl.find ((*it).first);
      if (itStats != m_flowStatsUl.end ())
       {
         (*itStats).second.lastTtiBytesTransmitted = uldci.m_tbSize;
       }
      else
       {
         NS_LOG_DEBUG (this << " No Stats for this allocated UE");
       }

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

  // Update global UE stats
  // update UEs stats
  for (itStats = m_flowStatsUl.begin (); itStats != m_flowStatsUl.end (); itStats++)
    {
      (*itStats).second.totalBytesTransmitted += (*itStats).second.lastTtiBytesTransmitted;
      // update average throughput (see eq. 12.3 of Sec 12.3.1.2 of LTE – The UMTS Long Term Evolution, Ed Wiley)
      (*itStats).second.lastAveragedThroughput = ((1.0 - (1.0 / m_timeWindow)) * (*itStats).second.lastAveragedThroughput) + ((1.0 / m_timeWindow) * (double)((*itStats).second.lastTtiBytesTransmitted / 0.001));
      NS_LOG_INFO (this << " UE total bytes " << (*itStats).second.totalBytesTransmitted);
      NS_LOG_INFO (this << " UE average throughput " << (*itStats).second.lastAveragedThroughput);
      (*itStats).second.lastTtiBytesTransmitted = 0;
    }
  m_allocationMaps.insert (std::pair <uint16_t, std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > > (params.m_sfnSf, allocationMapPerRntiPerLCId));
  m_schedSapUser->SchedUlConfigInd (ret);

  return;
}

void
cv2x_PriorityFfMacScheduler::DoSchedUlNoiseInterferenceReq (const struct cv2x_FfMacSchedSapProvider::SchedUlNoiseInterferenceReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  return;
}

void
cv2x_PriorityFfMacScheduler::DoSchedUlSrInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlSrInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  return;
}

void
cv2x_PriorityFfMacScheduler::DoSchedUlMacCtrlInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
{
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
                      it->second.insert (std::pair <uint8_t, uint32_t> (lcId, buffer));
                   }
                  else 
                   {   
                     for (uint8_t lcg = 0; lcg < 4; ++lcg) 
                      {
                         uint8_t bsrId=(*itBsr).second.at (lcg);
                         buffer += cv2x_BufferSizeLevelBsr::BsrId2BufferSize (bsrId);
                      }  
                     (*itMap).second=buffer;
                   }    
               } // end else 
           } //end if (it == m_ceBsrRxed.end ())
        } // end for for (itBsr=mapBsr.begin(); itBsr!=mapBsr.end(); itBsr++)
    } //end for 
  return; 
         
}

void
cv2x_PriorityFfMacScheduler::DoSchedUlCqiInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);


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
        NS_FATAL_ERROR ("cv2x_PriorityFfMacScheduler supports only PUSCH and SRS UL-CQIs");
      }
      break;
    default:
      NS_FATAL_ERROR ("Unknown type of UL-CQI");
    }
  return;
}

void
cv2x_PriorityFfMacScheduler::RefreshDlCqiMaps (void)
{
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
          NS_LOG_INFO (this << " P10-CQI expired for user " << (*itP10).first);
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

  // refresh DL CQI A30 Map
  std::map <uint16_t,uint32_t>::iterator itA30 = m_a30CqiTimers.begin ();
  while (itA30 != m_a30CqiTimers.end ())
    {
      NS_LOG_INFO (this << " A30-CQI for user " << (*itA30).first << " is " << (uint32_t)(*itA30).second << " thr " << (uint32_t)m_cqiTimersThreshold);
      if ((*itA30).second == 0)
        {
          // delete correspondent entries
          std::map <uint16_t,cv2x_SbMeasResult_s>::iterator itMap = m_a30CqiRxed.find ((*itA30).first);
          NS_ASSERT_MSG (itMap != m_a30CqiRxed.end (), " Does not find CQI report for user " << (*itA30).first);
          NS_LOG_INFO (this << " A30-CQI expired for user " << (*itA30).first);
          m_a30CqiRxed.erase (itMap);
          std::map <uint16_t,uint32_t>::iterator temp = itA30;
          itA30++;
          m_a30CqiTimers.erase (temp);
        }
      else
        {
          (*itA30).second--;
          itA30++;
        }
    }

  return;
}


void
cv2x_PriorityFfMacScheduler::RefreshUlCqiMaps (void)
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
cv2x_PriorityFfMacScheduler::UpdateDlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size)
{
  std::map<cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;
  cv2x_LteFlowId_t flow (rnti, lcid);
  it = m_rlcBufferReq.find (flow);
  if (it != m_rlcBufferReq.end ())
    {
      NS_LOG_INFO (this << " UE " << rnti << " LC " << (uint16_t)lcid << " txqueue " << (*it).second.m_rlcTransmissionQueueSize << " retxqueue " << (*it).second.m_rlcRetransmissionQueueSize << " status " << (*it).second.m_rlcStatusPduSize << " decrease " << size);
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
cv2x_PriorityFfMacScheduler::UpdateUlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size)
{

 //std::cout<<"At time: "<< Simulator::Now().GetSeconds ()<<" ENTER cv2x_PriorityFfMacScheduler::UpdateUlRlcBufferInfo " <<std::endl;
  size = size - 2; // remove the minimum RLC overhead
  std::multimap <uint16_t, std::map <uint8_t, uint32_t> >::iterator it=m_ceBsrRxed.find (rnti);
  
  if (it != m_ceBsrRxed.end ())
    {
      std::map <uint8_t, uint32_t>::iterator itMap=it->second.find (lcid);

      if (itMap!=it->second.end())
       {
         NS_LOG_INFO (this << " UE " << rnti << "lcid "<< lcid << " size " << size << " BSR " << (*itMap).second);
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
      NS_LOG_ERROR (this << " Does not find BSR report info of UE " << rnti);
    }

}

void
cv2x_PriorityFfMacScheduler::TransmissionModeConfigurationUpdate (uint16_t rnti, uint8_t txMode)
{
  NS_LOG_FUNCTION (this << " RNTI " << rnti << " txMode " << (uint16_t)txMode);
  cv2x_FfMacCschedSapUser::CschedUeConfigUpdateIndParameters params;
  params.m_rnti = rnti;
  params.m_transmissionMode = txMode;
  m_cschedSapUser->CschedUeConfigUpdateInd (params);
}


}

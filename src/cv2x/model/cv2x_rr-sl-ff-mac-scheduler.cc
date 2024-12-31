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
#include <ns3/cv2x_rr-sl-ff-mac-scheduler.h>
#include <ns3/simulator.h>
#include <ns3/cv2x_lte-common.h>
#include <ns3/cv2x_lte-vendor-specific-parameters.h>
#include <ns3/boolean.h>
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_RrSlFfMacScheduler");

static const int Type0AllocationRbg[4] = {
  10,       // RGB size 1
  26,       // RGB size 2
  63,       // RGB size 3
  110       // RGB size 4
};  // see table 7.1.6.1-1 of 36.213




NS_OBJECT_ENSURE_REGISTERED (cv2x_RrSlFfMacScheduler);


class cv2x_RrSlSchedulerMemberCschedSapProvider : public cv2x_FfMacCschedSapProvider
{
public:
  cv2x_RrSlSchedulerMemberCschedSapProvider (cv2x_RrSlFfMacScheduler* scheduler);

  // inherited from cv2x_FfMacCschedSapProvider
  virtual void CschedCellConfigReq (const struct CschedCellConfigReqParameters& params);
  virtual void CschedUeConfigReq (const struct CschedUeConfigReqParameters& params);
  virtual void CschedLcConfigReq (const struct CschedLcConfigReqParameters& params);
  virtual void CschedLcReleaseReq (const struct CschedLcReleaseReqParameters& params);
  virtual void CschedUeReleaseReq (const struct CschedUeReleaseReqParameters& params);
  virtual void CschedPoolConfigReq (const struct CschedPoolConfigReqParameters& params);
  virtual void CschedPoolReleaseReq (const struct CschedPoolReleaseReqParameters& params);

private:
  cv2x_RrSlSchedulerMemberCschedSapProvider ();
  cv2x_RrSlFfMacScheduler* m_scheduler;
};

cv2x_RrSlSchedulerMemberCschedSapProvider::cv2x_RrSlSchedulerMemberCschedSapProvider ()
{
}

cv2x_RrSlSchedulerMemberCschedSapProvider::cv2x_RrSlSchedulerMemberCschedSapProvider (cv2x_RrSlFfMacScheduler* scheduler) : m_scheduler (scheduler)
{
}


void
cv2x_RrSlSchedulerMemberCschedSapProvider::CschedCellConfigReq (const struct CschedCellConfigReqParameters& params)
{
  m_scheduler->DoCschedCellConfigReq (params);
}

void
cv2x_RrSlSchedulerMemberCschedSapProvider::CschedUeConfigReq (const struct CschedUeConfigReqParameters& params)
{
  m_scheduler->DoCschedUeConfigReq (params);
}


void
cv2x_RrSlSchedulerMemberCschedSapProvider::CschedLcConfigReq (const struct CschedLcConfigReqParameters& params)
{
  m_scheduler->DoCschedLcConfigReq (params);
}

void
cv2x_RrSlSchedulerMemberCschedSapProvider::CschedLcReleaseReq (const struct CschedLcReleaseReqParameters& params)
{
  m_scheduler->DoCschedLcReleaseReq (params);
}

void
cv2x_RrSlSchedulerMemberCschedSapProvider::CschedUeReleaseReq (const struct CschedUeReleaseReqParameters& params)
{
  m_scheduler->DoCschedUeReleaseReq (params);
}

void
cv2x_RrSlSchedulerMemberCschedSapProvider::CschedPoolConfigReq (const struct CschedPoolConfigReqParameters& params)
{
  m_scheduler->DoCschedPoolConfigReq (params);
}
  
void
cv2x_RrSlSchedulerMemberCschedSapProvider::CschedPoolReleaseReq (const struct CschedPoolReleaseReqParameters& params)
{
  m_scheduler->DoCschedPoolReleaseReq (params);
}



class cv2x_RrSlSchedulerMemberSchedSapProvider : public cv2x_FfMacSchedSapProvider
{
public:
  cv2x_RrSlSchedulerMemberSchedSapProvider (cv2x_RrSlFfMacScheduler* scheduler);

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
  cv2x_RrSlSchedulerMemberSchedSapProvider ();
  cv2x_RrSlFfMacScheduler* m_scheduler;
};



cv2x_RrSlSchedulerMemberSchedSapProvider::cv2x_RrSlSchedulerMemberSchedSapProvider ()
{
}


cv2x_RrSlSchedulerMemberSchedSapProvider::cv2x_RrSlSchedulerMemberSchedSapProvider (cv2x_RrSlFfMacScheduler* scheduler)
  : m_scheduler (scheduler)
{
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedDlRlcBufferReq (const struct SchedDlRlcBufferReqParameters& params)
{
  m_scheduler->DoSchedDlRlcBufferReq (params);
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedDlPagingBufferReq (const struct SchedDlPagingBufferReqParameters& params)
{
  m_scheduler->DoSchedDlPagingBufferReq (params);
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedDlMacBufferReq (const struct SchedDlMacBufferReqParameters& params)
{
  m_scheduler->DoSchedDlMacBufferReq (params);
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedDlTriggerReq (const struct SchedDlTriggerReqParameters& params)
{
  m_scheduler->DoSchedDlTriggerReq (params);
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedDlRachInfoReq (const struct SchedDlRachInfoReqParameters& params)
{
  m_scheduler->DoSchedDlRachInfoReq (params);
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedDlCqiInfoReq (const struct SchedDlCqiInfoReqParameters& params)
{
  m_scheduler->DoSchedDlCqiInfoReq (params);
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedUlTriggerReq (const struct SchedUlTriggerReqParameters& params)
{
  m_scheduler->DoSchedUlTriggerReq (params);
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedUlNoiseInterferenceReq (const struct SchedUlNoiseInterferenceReqParameters& params)
{
  m_scheduler->DoSchedUlNoiseInterferenceReq (params);
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedUlSrInfoReq (const struct SchedUlSrInfoReqParameters& params)
{
  m_scheduler->DoSchedUlSrInfoReq (params);
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedUlMacCtrlInfoReq (const struct SchedUlMacCtrlInfoReqParameters& params)
{
  m_scheduler->DoSchedUlMacCtrlInfoReq (params);
}

void
cv2x_RrSlSchedulerMemberSchedSapProvider::SchedUlCqiInfoReq (const struct SchedUlCqiInfoReqParameters& params)
{
  m_scheduler->DoSchedUlCqiInfoReq (params);
}





cv2x_RrSlFfMacScheduler::cv2x_RrSlFfMacScheduler ()
  :   m_cschedSapUser (0),
    m_schedSapUser (0),
    m_nextRntiDl (0),
    m_nextLcidDl (0),
    m_nextRntiUl (0)
{
  m_amc = CreateObject <cv2x_LteAmc> ();
  m_cschedSapProvider = new cv2x_RrSlSchedulerMemberCschedSapProvider (this);
  m_schedSapProvider = new cv2x_RrSlSchedulerMemberSchedSapProvider (this);
}

cv2x_RrSlFfMacScheduler::~cv2x_RrSlFfMacScheduler ()
{
  NS_LOG_FUNCTION (this);
}

void
cv2x_RrSlFfMacScheduler::DoDispose ()
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
cv2x_RrSlFfMacScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::cv2x_RrSlFfMacScheduler")
    .SetParent<cv2x_FfMacScheduler> ()
    .AddConstructor<cv2x_RrSlFfMacScheduler> ()
    .AddAttribute ("CqiTimerThreshold",
                   "The number of TTIs a CQI is valid (default 1000 - 1 sec.)",
                   UintegerValue (1000),
                   MakeUintegerAccessor (&cv2x_RrSlFfMacScheduler::m_cqiTimersThreshold),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("HarqEnabled",
                   "Activate/Deactivate the HARQ [by default is active].",
                   BooleanValue (true),
                   MakeBooleanAccessor (&cv2x_RrSlFfMacScheduler::m_harqOn),
                   MakeBooleanChecker ())
    .AddAttribute ("UlGrantMcs",
                   "The MCS of the UL grant, must be [0..15] (default 0)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_RrSlFfMacScheduler::m_ulGrantMcs),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("Itrp",
                   "The repetition for PSSCH. Default = 0",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_RrSlFfMacScheduler::m_slItrp),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("SlGrantMcs",
                   "The MCS of the SL grant, must be [0..15] (default 0)",
                   UintegerValue (0),
                   MakeUintegerAccessor (&cv2x_RrSlFfMacScheduler::m_slGrantMcs),
                   MakeUintegerChecker<uint8_t> ())
    .AddAttribute ("SlGrantSize",
                   "The number of RBs allocated per UE for sidelink (default 1)",
                   UintegerValue (1),
                   MakeUintegerAccessor (&cv2x_RrSlFfMacScheduler::m_slGrantSize),
                   MakeUintegerChecker<uint8_t> ())
                   
  ;
  return tid;
}



void
cv2x_RrSlFfMacScheduler::SetFfMacCschedSapUser (cv2x_FfMacCschedSapUser* s)
{
  m_cschedSapUser = s;
}

void
cv2x_RrSlFfMacScheduler::SetFfMacSchedSapUser (cv2x_FfMacSchedSapUser* s)
{
  m_schedSapUser = s;
}

cv2x_FfMacCschedSapProvider*
cv2x_RrSlFfMacScheduler::GetFfMacCschedSapProvider ()
{
  return m_cschedSapProvider;
}

cv2x_FfMacSchedSapProvider*
cv2x_RrSlFfMacScheduler::GetFfMacSchedSapProvider ()
{
  return m_schedSapProvider;
}

void
cv2x_RrSlFfMacScheduler::SetLteFfrSapProvider (cv2x_LteFfrSapProvider* s)
{
  m_ffrSapProvider = s;
}

cv2x_LteFfrSapUser*
cv2x_RrSlFfMacScheduler::GetLteFfrSapUser ()
{
  return m_ffrSapUser;
}

void
cv2x_RrSlFfMacScheduler::DoCschedCellConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedCellConfigReqParameters& params)
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
cv2x_RrSlFfMacScheduler::DoCschedUeConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedUeConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this << " RNTI " << params.m_rnti << " txMode " << (uint16_t)params.m_transmissionMode);
  std::map <uint16_t,uint8_t>::iterator it = m_uesTxMode.find (params.m_rnti);
  if (it == m_uesTxMode.end ())
    {
      m_uesTxMode.insert (std::pair <uint16_t, double> (params.m_rnti, params.m_transmissionMode));
      // generate HARQ buffers
      m_dlHarqCurrentProcessId.insert (std::pair <uint16_t,uint8_t > (params.m_rnti, 0));
      cv2x_DlHarqProcessesStatus_t dlHarqPrcStatus;
      dlHarqPrcStatus.resize (8,0);
      m_dlHarqProcessesStatus.insert (std::pair <uint16_t, cv2x_DlHarqProcessesStatus_t> (params.m_rnti, dlHarqPrcStatus));
      cv2x_DlHarqProcessesTimer_t dlHarqProcessesTimer;
      dlHarqProcessesTimer.resize (8,0);
      m_dlHarqProcessesTimer.insert (std::pair <uint16_t, cv2x_DlHarqProcessesTimer_t> (params.m_rnti, dlHarqProcessesTimer));
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

  //configure/update sidelink destinations
  std::map <uint16_t,std::vector <uint32_t> >::iterator ueIt = m_uesDestinations.find (params.m_rnti);
  if (ueIt == m_uesDestinations.end ()) {
    m_uesDestinations.insert (std::pair <uint16_t,std::vector <uint32_t> > (params.m_rnti, params.m_slDestinations));
  } else {
    (*ueIt).second = params.m_slDestinations;
  }

  
  return;
}

void
cv2x_RrSlFfMacScheduler::DoCschedLcConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedLcConfigReqParameters& params)
{
  //NS_LOG_FUNCTION (this);
  // Not used at this stage (LCs updated by DoSchedDlRlcBufferReq)

  //added
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


  std::map <uint16_t, cv2x_RrsFlowPerf_t>::iterator it;

  for (uint16_t i = 0; i < params.m_logicalChannelConfigList.size (); i++)
    {
      it = m_flowStatsDl.find (params.m_rnti);

      if (it == m_flowStatsDl.end ())
        {
          double tbrDlInBytes = params.m_logicalChannelConfigList.at (i).m_eRabGuaranteedBitrateDl / 8;   // byte/s
          double tbrUlInBytes = params.m_logicalChannelConfigList.at (i).m_eRabGuaranteedBitrateUl / 8;   // byte/s

          cv2x_RrsFlowPerf_t flowStatsDl;
          flowStatsDl.flowStart = Simulator::Now ();
          flowStatsDl.totalBytesTransmitted = 0;
          flowStatsDl.lastTtiBytesTransmitted = 0;
          flowStatsDl.lastAveragedThroughput = 1;
          flowStatsDl.secondLastAveragedThroughput = 1;
          flowStatsDl.targetThroughput = tbrDlInBytes;
          m_flowStatsDl.insert (std::pair<uint16_t, cv2x_RrsFlowPerf_t> (params.m_rnti, flowStatsDl));
          cv2x_RrsFlowPerf_t flowStatsUl;
          flowStatsUl.flowStart = Simulator::Now ();
          flowStatsUl.totalBytesTransmitted = 0;
          flowStatsUl.lastTtiBytesTransmitted = 0;
          flowStatsUl.lastAveragedThroughput = 1;
          flowStatsUl.secondLastAveragedThroughput = 1;
          flowStatsUl.targetThroughput = tbrUlInBytes;
          m_flowStatsUl.insert (std::pair<uint16_t, cv2x_RrsFlowPerf_t> (params.m_rnti, flowStatsUl));
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
cv2x_RrSlFfMacScheduler::DoCschedLcReleaseReq (const struct cv2x_FfMacCschedSapProvider::CschedLcReleaseReqParameters& params)
{

  NS_LOG_FUNCTION (this);
    for (uint16_t i = 0; i < params.m_logicalChannelIdentity.size (); i++)
    {
     std::list<cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it = m_rlcBufferReq.begin ();
      while (it!=m_rlcBufferReq.end ())
        {
          if (((*it).m_rnti == params.m_rnti)&&((*it).m_logicalChannelIdentity == params.m_logicalChannelIdentity.at (i)))
            {
              it = m_rlcBufferReq.erase (it);
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
cv2x_RrSlFfMacScheduler::DoCschedUeReleaseReq (const struct cv2x_FfMacCschedSapProvider::CschedUeReleaseReqParameters& params)
{
  NS_LOG_FUNCTION (this << " Release RNTI " << params.m_rnti);
  
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
  //clear requests from this UE on all the pools
  std::map <uint32_t, PoolInfo>::iterator poolIt;
  for (poolIt = m_poolAllocations.begin() ; poolIt != m_poolAllocations.end() ; poolIt++)
    {
      poolIt->second.m_ceSlBsrRxed.erase (params.m_rnti);
    }
  
  std::list<cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it = m_rlcBufferReq.begin ();
  while (it != m_rlcBufferReq.end ())
    {
      if ((*it).m_rnti == params.m_rnti)
        {
          NS_LOG_INFO (this << " Erase RNTI " << (*it).m_rnti << " LC " << (uint16_t)(*it).m_logicalChannelIdentity);
          it = m_rlcBufferReq.erase (it);
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
cv2x_RrSlFfMacScheduler::DoCschedPoolConfigReq (const struct  cv2x_FfMacCschedSapProvider::CschedPoolConfigReqParameters& params)
{
  NS_LOG_FUNCTION (this << params.m_group);

  PoolInfo info;
  info.m_pool = params.m_pool;
  info.m_init = false;

  std::map <uint32_t, PoolInfo>::iterator it = m_poolAllocations.find (params.m_group);
  NS_ASSERT_MSG (it == m_poolAllocations.end(), "Cannot add existing pool");
  m_poolAllocations.insert (std::pair <uint32_t, PoolInfo> (params.m_group, info));
}
  
void
cv2x_RrSlFfMacScheduler::DoCschedPoolReleaseReq (const struct  cv2x_FfMacCschedSapProvider::CschedPoolReleaseReqParameters& params)
{
  NS_LOG_FUNCTION (this << params.m_group);
  std::map <uint32_t, PoolInfo>::iterator it = m_poolAllocations.find (params.m_group);
  NS_ASSERT_MSG (it != m_poolAllocations.end(), "Cannot remove non existing pool");
  m_poolAllocations.erase (it);
}

  

void
cv2x_RrSlFfMacScheduler::DoSchedDlRlcBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
{

  NS_LOG_FUNCTION (this << params.m_rnti << (uint32_t) params.m_logicalChannelIdentity);
  // API generated by RLC for updating RLC parameters on a LC (tx and retx queues)
  std::list<cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it = m_rlcBufferReq.begin ();
  bool newLc = true;
  while (it != m_rlcBufferReq.end ())
    {
      // remove old entries of this UE-LC
      if (((*it).m_rnti == params.m_rnti)&&((*it).m_logicalChannelIdentity == params.m_logicalChannelIdentity))
        {
          it = m_rlcBufferReq.erase (it);
          newLc = false;
        }
      else
        {
          ++it;
        }
    }
  // add the new parameters
  m_rlcBufferReq.insert (it, params);
  NS_LOG_INFO (this << " RNTI " << params.m_rnti << " LC " << (uint16_t)params.m_logicalChannelIdentity << " RLC tx size " << params.m_rlcTransmissionQueueHolDelay << " RLC retx size " << params.m_rlcRetransmissionQueueSize << " RLC stat size " <<  params.m_rlcStatusPduSize);
  // initialize statistics of the flow in case of new flows
  if (newLc == true)
    {
      m_p10CqiRxed.insert ( std::pair<uint16_t, uint8_t > (params.m_rnti, 1)); // only codeword 0 at this stage (SISO)
      // initialized to 1 (i.e., the lowest value for transmitting a signal)
      m_p10CqiTimers.insert ( std::pair<uint16_t, uint32_t > (params.m_rnti, m_cqiTimersThreshold));
    }

  return;


}

void
cv2x_RrSlFfMacScheduler::DoSchedDlPagingBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlPagingBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("method not implemented");
  return;
}

void
cv2x_RrSlFfMacScheduler::DoSchedDlMacBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlMacBufferReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  NS_FATAL_ERROR ("method not implemented");
  return;
}

int
cv2x_RrSlFfMacScheduler::GetRbgSize (int dlbandwidth)
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
cv2x_RrSlFfMacScheduler::SortRlcBufferReq (cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters i,cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters j)
{
  return (i.m_rnti < j.m_rnti);
}


uint8_t
cv2x_RrSlFfMacScheduler::HarqProcessAvailability (uint16_t rnti)
{
  NS_LOG_FUNCTION (this << rnti);

  std::map <uint16_t, uint8_t>::iterator it = m_dlHarqCurrentProcessId.find (rnti);
  if (it == m_dlHarqCurrentProcessId.end ())
    {
      NS_FATAL_ERROR ("No Process Id found for this RNTI " << rnti);
    }
  std::map <uint16_t, cv2x_DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find (rnti);
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
cv2x_RrSlFfMacScheduler::UpdateHarqProcessId (uint16_t rnti)
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
  std::map <uint16_t, cv2x_DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find (rnti);
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
cv2x_RrSlFfMacScheduler::RefreshHarqProcesses ()
{
  NS_LOG_FUNCTION (this);

  std::map <uint16_t, cv2x_DlHarqProcessesTimer_t>::iterator itTimers;
  for (itTimers = m_dlHarqProcessesTimer.begin (); itTimers != m_dlHarqProcessesTimer.end (); itTimers ++)
    {
      for (uint16_t i = 0; i < HARQ_PROC_NUM; i++)
        {
          if ((*itTimers).second.at (i) == HARQ_DL_TIMEOUT)
            {
              // reset HARQ process

              NS_LOG_INFO (this << " Reset HARQ proc " << i << " for RNTI " << (*itTimers).first);
              std::map <uint16_t, cv2x_DlHarqProcessesStatus_t>::iterator itStat = m_dlHarqProcessesStatus.find ((*itTimers).first);
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
cv2x_RrSlFfMacScheduler::DoSchedDlTriggerReq (const struct cv2x_FfMacSchedSapProvider::SchedDlTriggerReqParameters& params)
{
  /************************* Comments ********************************************************************************/
  /* - Enhanced version of the RR scheduler : assign to each flow only the required RBGs to transmit its data        */
  /*_________________________________________________________________________________________________________________*/

  //std::cout<<" ENTER cv2x_RrSlFfMacScheduler::DoSchedDlTriggerReq"<<std::endl;
  NS_LOG_FUNCTION (this << " DL Frame no. " << (params.m_sfnSf >> 4) << " subframe no. " << (0xF & params.m_sfnSf));
  // API generated by RLC for triggering the scheduling of a DL subframe

  RefreshDlCqiMaps ();
  int rbgSize = GetRbgSize (m_cschedCellConfig.m_dlBandwidth);
  int rbgNum = m_cschedCellConfig.m_dlBandwidth / rbgSize;
  cv2x_FfMacSchedSapUser::SchedDlConfigIndParameters ret;

  std::map <uint16_t, std::map <uint8_t, uint16_t > > allocationMapPerRntiPerLCId;  // allocation map per UE per Flow


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
  //TODO: if we have sidelink allocation, we should make sure that the RACH
  //is not allocated on the same RBs. 
  m_rachAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);
  uint16_t rbStart = 0;
  std::vector <struct cv2x_RachListElement_s>::iterator itRach;
  for (itRach = m_rachList.begin (); itRach != m_rachList.end (); itRach++)
    {
      NS_ASSERT_MSG (m_amc->GetDlTbSizeFromMcs (m_ulGrantMcs, m_cschedCellConfig.m_ulBandwidth) > (*itRach).m_estimatedSize, " Default UL Grant MCS does not allow to send RACH messages");
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
          tbSizeBits = m_amc->GetDlTbSizeFromMcs (m_ulGrantMcs, rbLen);
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
              std::map <uint16_t, cv2x_DlHarqProcessesStatus_t>::iterator it = m_dlHarqProcessesStatus.find (rnti);
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
                  uint32_t rbgMask = 0;
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
          std::map <uint16_t, cv2x_DlHarqProcessesTimer_t>::iterator itHarqTimer = m_dlHarqProcessesTimer.find (rnti);
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
          std::map <uint16_t, cv2x_DlHarqProcessesStatus_t>::iterator it = m_dlHarqProcessesStatus.find (m_dlInfoListBuffered.at (i).m_rnti);
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

  // Get the actual active flows (queue!=0)
  std::list<cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;
  m_rlcBufferReq.sort (SortRlcBufferReq);
  int nflows = 0;
  int nTbs = 0;
  std::map <uint16_t,uint8_t> lcActivesPerRnti; // tracks how many active LCs per RNTI there are
  std::map <uint16_t,uint8_t>::iterator itLcRnti;
  for (it = m_rlcBufferReq.begin (); it != m_rlcBufferReq.end (); it++)
    {
      // remove old entries of this UE-LC
      std::set <uint16_t>::iterator itRnti = rntiAllocated.find ((*it).m_rnti);
      if ( (((*it).m_rlcTransmissionQueueSize > 0)
            || ((*it).m_rlcRetransmissionQueueSize > 0)
            || ((*it).m_rlcStatusPduSize > 0))
           && (itRnti == rntiAllocated.end ())  // UE must not be allocated for HARQ retx
           && (HarqProcessAvailability ((*it).m_rnti))  ) // UE needs HARQ proc free

        {
          NS_LOG_LOGIC (this << " User " << (*it).m_rnti << " LC " << (uint16_t)(*it).m_logicalChannelIdentity << " is active, status  " << (*it).m_rlcStatusPduSize << " retx " << (*it).m_rlcRetransmissionQueueSize << " tx " << (*it).m_rlcTransmissionQueueSize);
          std::map <uint16_t,uint8_t>::iterator itCqi = m_p10CqiRxed.find ((*it).m_rnti);
          uint8_t cqi = 0;
          if (itCqi != m_p10CqiRxed.end ())
            {
              cqi = (*itCqi).second;
            }
          else
            {
              cqi = 1; // lowest value fro trying a transmission
            }
          if (cqi != 0)
            {
              // CQI == 0 means "out of range" (see table 7.2.3-1 of 36.213)
              nflows++;
              itLcRnti = lcActivesPerRnti.find ((*it).m_rnti);
              if (itLcRnti != lcActivesPerRnti.end ())
                {
                  (*itLcRnti).second++;
                }
              else
                {
                  lcActivesPerRnti.insert (std::pair<uint16_t, uint8_t > ((*it).m_rnti, 1));
                  nTbs++;
                }

            }
        }
    }

  if (nflows == 0)
    {
      if ((ret.m_buildDataList.size () > 0) || (ret.m_buildRarList.size () > 0))
        {
          m_schedSapUser->SchedDlConfigInd (ret);
        }
      return;
    }
 
  int rbgAllocated = 0;

  // round robin assignment to all UEs registered starting from the subsequent of the one
  // served last scheduling trigger event
  
 //std::cout<<" next rnti dl = "<<(int)m_nextRntiDl<<" next lcid dl = "<<(int)m_nextLcidDl<<std::endl;
  if (( m_nextRntiDl != 0) && (m_nextLcidDl!=0 ) )
    {
      for (it = m_rlcBufferReq.begin (); it != m_rlcBufferReq.end (); it++)
        {
          if (((*it).m_rnti == m_nextRntiDl) && ( (*it).m_logicalChannelIdentity== m_nextLcidDl ) )
            {
              //std::cout<<"break"<<std::endl;
              break;
            }
        }

      if (it == m_rlcBufferReq.end ())
        {
          NS_LOG_ERROR (this << " no user found");
        }
    }
  else
    {
      it = m_rlcBufferReq.begin ();
      m_nextRntiDl = (*it).m_rnti;
      m_nextLcidDl = (*it).m_logicalChannelIdentity;
    }
  std::map <uint16_t,uint8_t>::iterator itTxMode;

   //std::map <uint16_t, uint16_t> satisfiedUes;       //structure to save satisfied flows 
   std::vector <cv2x_LteFlowId_t> satisfiedFlows;      // structure to save satisfied flows 
   //std::map <uint16_t, uint16_t>::iterator itSUes;
   
   double bytesTxedF = 0;                          // accumulated assigned data for each flows    

   //compute already allocated rbgs in the rbgmap
  int alreadyAllocatedRbg=0; 
  for ( unsigned int l=0 ; l < rbgMap.size(); l++)
   {
     if ( rbgMap.at(l)== true ) 
       alreadyAllocatedRbg++;
   }
  //std::cout<<"\t Already allocated rbgs = "<<(int)alreadyAllocatedRbg<<std::endl;
  rbgNum= rbgNum-alreadyAllocatedRbg;
  //std::cout<<" \t RBGs available = " <<(int)rbgNum<<std::endl;

  std::list<cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itBufferAux;  //added
  int rbgNumOriginal = m_cschedCellConfig.m_dlBandwidth / rbgSize; //added 
  
  do
    {
      //std::cout<<" rnti = "<<(int)(*it).m_rnti<<std::endl;
      itLcRnti = lcActivesPerRnti.find ((*it).m_rnti);
      //std::cout<<"\t lcActive per rnti = "<<(int)(*itLcRnti).second<<std::endl;
      std::set <uint16_t>::iterator itRnti = rntiAllocated.find ((*it).m_rnti);
      if ((itLcRnti == lcActivesPerRnti.end ())||(itRnti != rntiAllocated.end ()))
        {
          // skip this RNTI (no active queue or yet allocated for HARQ)
          uint16_t rntiDiscared = (*it).m_rnti;
          while (it != m_rlcBufferReq.end ())
            {
              if ((*it).m_rnti != rntiDiscared)
                {
                  break;
                }
              it++;
            }
          if (it == m_rlcBufferReq.end ())
            {
              // restart from the first
              it = m_rlcBufferReq.begin ();
            }
          continue;
        }
    
    //case mcs = 0 : 
     std::map <uint16_t,uint8_t>::iterator itTxMode;
     itTxMode = m_uesTxMode.find ((*it).m_rnti);
     if (itTxMode == m_uesTxMode.end ())
      {
        NS_FATAL_ERROR ("No Transmission Mode info on user " << (*it).m_rnti);
      }
     int nLayer1 = cv2x_TransmissionModesLayers::TxMode2LayerNum ((*itTxMode).second);
     std::vector< uint8_t > mcs;
     std::map <uint16_t,uint8_t>::iterator itCqi1 = m_p10CqiRxed.find ((*it).m_rnti);
     for (uint8_t i = 0; i < nLayer1; i++)
      {
        if (itCqi1 == m_p10CqiRxed.end ())
         {
           mcs.push_back (0); // no info on this user -> lowest MCS
         }
        else
         {
           mcs.push_back ( m_amc->GetMcsFromCqi ((*itCqi1).second) );
         }
      }
     //std::cout<<" rnti = " <<(int) (*it).m_rnti<< " mcs = " <<(int)mcs.at(0)<<std::endl;
     if ( mcs.at(0) == 0 ) 
      {

        // skip this RNTI mcs = 0 
          uint16_t rntiDiscared = (*it).m_rnti;
          while (it != m_rlcBufferReq.end ())
            {
              if ((*it).m_rnti != rntiDiscared)
                {
                  break;
                }
              it++;
            }
          if (it == m_rlcBufferReq.end ())
            {
              // restart from the first
              it = m_rlcBufferReq.begin ();
            }
          continue;
       }

     //added
     uint16_t rbgPerUE=0;
     uint8_t lcTreatedPerRnti = 0;
     int iRbg = 0;
     bool PassToNextUE=false;
     /********  another loop while for the lcid */
     //added 
     std::list<cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itBuffer;
      if (( m_nextRntiDl == 0) && (m_nextLcidDl==0 ) )
         itBuffer=m_rlcBufferReq.begin();
      else
       {
         for ( itBuffer = m_rlcBufferReq.begin(); itBuffer != m_rlcBufferReq.end(); itBuffer ++) 
          {
             if ( ((*itBuffer).m_rnti == (*it).m_rnti) && ((*itBuffer).m_logicalChannelIdentity == (*it).m_logicalChannelIdentity) ) 
                break;
          }
       }
     bool PassToNextLcid=false;
     do 
      {      
         uint16_t rbgPerLcid = 1;  // intialization of rbPerFlow : at least 1 rbg per TB (till available resource)
  
         while (( iRbg < rbgNum ) && (lcTreatedPerRnti < (*itLcRnti).second) && ( (*itBuffer).m_rnti ==  (*it).m_rnti) ) // ** third conditin added 
          {
            //std::cout<<" \t irbg = " <<(int)iRbg<< " rbgNum = "<<(int)rbgNum<<std::endl;
            
            while ( (itBuffer != m_rlcBufferReq.end ()) && ( iRbg < rbgNum )) 
             {
               //std::cout<<" \t it buffer rnti = " <<(int)(*itBuffer).m_rnti<<" lcid buffer "<<(int)(*itBuffer).m_logicalChannelIdentity<< " data to send = "<<(int)(*itBuffer).m_rlcTransmissionQueueSize<<std::endl;
               //if ( ((*itBuffer).m_rnti ==  (*it).m_rnti) &&  ( (*itBuffer).m_logicalChannelIdentity ==  (*it).m_logicalChannelIdentity ) && ( (*itBuffer).m_rlcTransmissionQueueSize > 0 ) ) 
               if ( ((*itBuffer).m_rnti ==  (*it).m_rnti) && ( (*itBuffer).m_rlcTransmissionQueueSize > 0 ) ) 
                {
                   int b =1;
                   //std::cout<<"  \t it buffer rnti = " <<(int)(*itBuffer).m_rnti<<"  LCID = "<< (int) (*itBuffer).m_logicalChannelIdentity << " data to send = " << (int) (*itBuffer).m_rlcTransmissionQueueSize << std::endl;
                   cv2x_LteFlowId_t flowId ;
                   flowId.m_rnti= (*itBuffer).m_rnti;
                   flowId.m_lcId= (*itBuffer).m_logicalChannelIdentity;
                   std::vector <cv2x_LteFlowId_t>::iterator itSFlows;
                   //trying to find the current flow in the satisfied flows 
                   bool findF=false;
                   for(itSFlows=satisfiedFlows.begin();itSFlows!=satisfiedFlows.end();itSFlows++)
                    {
                      if(((*itSFlows).m_rnti==flowId.m_rnti) && ((*itSFlows).m_lcId==flowId.m_lcId))
                        {
                           findF=true;
                           break;
                        }
                    }

                   if (findF==false)
                    { 
                      //std::cout<<"\t --> this flow is not satisfied ! "<<std::endl;
                      // this flows doesn't exist in the satisfied flows --> needs more RBGs to transmit           
                      // try to find this flow in the m_rlcBufferReq
                      std::map <uint16_t,uint8_t>::iterator itTxMode;
                      itTxMode = m_uesTxMode.find ((*it).m_rnti);
                      //int b=1;
                      if (itTxMode == m_uesTxMode.end ())
                       {
                         NS_FATAL_ERROR ("No Transmission Mode info on user " << (*it).m_rnti);
                       }
                      int nLayer = cv2x_TransmissionModesLayers::TxMode2LayerNum ((*itTxMode).second);
                      std::vector< uint8_t > mcs;
                      std::map <uint16_t,uint8_t>::iterator itCqi = m_p10CqiRxed.find ((*it).m_rnti);
                      for (uint8_t i = 0; i < nLayer; i++)
                       {
                         if (itCqi == m_p10CqiRxed.end ())
                          {
                            mcs.push_back (0); // no info on this user -> lowest MCS
                          }
                         else
                          {
                            mcs.push_back ( m_amc->GetMcsFromCqi ((*itCqi).second) );
                          }
                       }

                      b= b + iRbg;   // number of allocated RBGs + 1 (the current RBG) 
                      //std::cout<< "\t b = "<<(int)b <<std::endl;
                      bytesTxedF=(m_amc->GetDlTbSizeFromMcs (mcs.at (0), b * rbgSize) / 8) * nLayer;   //multiply by nLayer for MIMO 

                      // calculate the transmission queue size for all the flows belonging to this UE 
                      int dataToTransmit=0;
                      std::list<cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itTemp;
                      for (itTemp=m_rlcBufferReq.begin(); itTemp!=m_rlcBufferReq.end(); itTemp++)
                       {
                         if (((*itTemp).m_rnti== (*itBuffer).m_rnti ) && ((*itTemp).m_logicalChannelIdentity== (*itBuffer).m_logicalChannelIdentity) && ((*itBuffer).m_rnti==(*it).m_rnti) )
                          {
                            if ( ((*itBuffer).m_rlcTransmissionQueueSize > 0)
                              || ((*itBuffer).m_rlcRetransmissionQueueSize > 0)
                              || ((*itBuffer).m_rlcStatusPduSize > 0) )
                             {
                                dataToTransmit = (*itBuffer).m_rlcTransmissionQueueSize;
                             }
                          }
                       }
                      //std::cout<<"\t after the allocation of this RBG : data to send = "<<(int) dataToTransmit<< " allocated data = "<<(int)bytesTxedF<<std::endl;
                      if (dataToTransmit <= bytesTxedF )
                       {
                         //this flow is  satisfied with this new allocation --> add it to satisfied flows 
                         //std::cout<<"\t this flow is satisfied ! "<<std::endl;
                         std::vector <cv2x_LteFlowId_t>::iterator Ft;
                         bool findF2=false;
                         for (Ft=satisfiedFlows.begin();Ft!=satisfiedFlows.end();Ft++)
                         {
                           if(((*Ft).m_rnti==flowId.m_rnti) && ((*Ft).m_lcId==flowId.m_lcId))
                            {
                              findF2=true;
                              break;
                            }
                         }
                        if (findF2==false)
                         {
                           satisfiedFlows.push_back(flowId);
                         }

                        rbgNum=rbgNum-b;
                        iRbg=0;
                        rbgPerLcid=b;
                        itBufferAux= itBuffer;    
                        itBuffer++;
                        lcTreatedPerRnti ++;
                        /************************************************************/
                        std::map <uint16_t,uint8_t>::iterator itLcRntiTemp;
                        itLcRntiTemp = lcActivesPerRnti.find (flowId.m_rnti);
                        if ( lcTreatedPerRnti == (*itLcRntiTemp).second )
                           PassToNextUE=true;
                        /************************************************************/
                        rbgPerUE=rbgPerUE+rbgPerLcid;
                        PassToNextLcid = true;
                        //***********insert this flow in the allocation map 
                        std::map <uint16_t, std::map <uint8_t, uint16_t> >::iterator itMap;
                        itMap = allocationMapPerRntiPerLCId.find ((uint16_t)(flowId.m_rnti));
                        if (itMap == allocationMapPerRntiPerLCId.end ())           
                         { 
                           //the flow with the max throughput doesn't exist in the allocation map --> insert it 
                           std::map<uint8_t, uint16_t> tempMap;
                           tempMap.insert (std::pair<uint8_t, uint16_t> (flowId.m_lcId, rbgPerLcid ));
                           allocationMapPerRntiPerLCId.insert (std::pair <uint16_t, std::map<uint8_t, uint16_t> > (flowId.m_rnti, tempMap));
                         }
                        else
                         {
                           //the flow exists already in the allocation map --> check if the lcid is already existing or not 
                           std::map <uint8_t, uint16_t >::iterator itSMap;
                           itSMap=itMap->second.find ((uint8_t)flowId.m_lcId);
                           if (itSMap == itMap->second.end ())
                            {
                             itMap->second.insert (std::pair<uint8_t, uint16_t> (flowId.m_lcId, rbgPerLcid));
                            }
                           else
                            {
                             (*itSMap).second = rbgPerLcid;
                            }
                          }
               
                        }
                       else
                        {
                          //std::cout<<"\t This flow is not satisfied ! "<<std::endl;
                          iRbg++;    
                          if ( iRbg == rbgNum ) // no possibility to allocate more ressources to this flow  --> add it in satisfied Flows even it is not satisfied 
                           {
                             PassToNextLcid=true;
                             b=iRbg;
                             rbgNum=rbgNum-iRbg;
                             rbgPerLcid=b;
                             //satisfiedFlows.push_back(flowId);     //**added
                             itBufferAux = itBuffer;  
                             itBuffer++;
                             PassToNextUE=true;       //*** added        
                            //std::cout<<"\t no more resources ! after incrementing, rnti = "<<(int)(*itBuffer).m_rnti<<" lcid = "<<(int)(*itBuffer).m_logicalChannelIdentity<<std::endl;
                            //std::cout<<" \t rbg per flow (end of resources) = "<<(int)rbgPerLcid<<std::endl;

                            lcTreatedPerRnti ++;
                            //std::cout<<"\t lc treated = "<<(int)lcTreatedPerRnti<<std::endl;
                            /************************************************************/
                            std::map <uint16_t,uint8_t>::iterator itLcRntiTemp;
                            itLcRntiTemp = lcActivesPerRnti.find (flowId.m_rnti);
                            if ( lcTreatedPerRnti == (*itLcRntiTemp).second )
                              PassToNextUE=true;
                            if ( rbgPerLcid == ((m_cschedCellConfig.m_dlBandwidth / rbgSize)- alreadyAllocatedRbg) ) 
                              { 
                                PassToNextUE=true;
                                itBufferAux = itBuffer;
                                satisfiedFlows.push_back(flowId);     //**added
                              }
                            /************************************************************/

                            rbgPerUE=rbgPerUE+rbgPerLcid;
                            //***********insert this flow in the allocation map 
                            std::map <uint16_t, std::map <uint8_t, uint16_t> >::iterator itMap;
                            itMap = allocationMapPerRntiPerLCId.find ((uint16_t)(flowId.m_rnti));
                            if (itMap == allocationMapPerRntiPerLCId.end ())           
                             { 
                               //the flow with the max throughput doesn't exist in the allocation map --> insert it 
                               std::map<uint8_t, uint16_t> tempMap;
                               tempMap.insert (std::pair<uint8_t, uint16_t> (flowId.m_lcId, rbgPerLcid ));
                               allocationMapPerRntiPerLCId.insert (std::pair <uint16_t, std::map<uint8_t, uint16_t> > (flowId.m_rnti, tempMap));
                             }
                            else
                             {
                               //the flow exists already in the allocation map --> check if the lcid is already existing or not 
                               std::map <uint8_t, uint16_t >::iterator itSMap;
                               itSMap=itMap->second.find ((uint8_t)flowId.m_lcId);
                               if (itSMap == itMap->second.end ())
                                {
                                  itMap->second.insert (std::pair<uint8_t, uint16_t> (flowId.m_lcId, rbgPerLcid));
                                }
                               else
                                {
                                  (*itSMap).second = rbgPerLcid;
                                }
                              }

                            }   
                          }
                        }  
                      else 
                       { 
                         iRbg=rbgNum;
                       } // end if (findF==false)

                     } 
                    else
                     { 
                       itBuffer++;
                     }  //end if ( (*itBuffer).m_rnti ==  (*it).m_rnti) ) 
                  } //end while for itBuffer
                 if ( itBuffer == m_rlcBufferReq.end() && (lcTreatedPerRnti ==0) ) 
                  {
                     lcTreatedPerRnti = (*itLcRnti).second;
                  }
               } // end while (iRbg<rbNum)
             }
            while ( ( (*it).m_rnti == (*itBuffer).m_rnti )  && (PassToNextUE==false) );  // & 


      //std::cout<<"-----------------------------------------------------------------------------------"<<std::endl;
      //std::cout <<" \t UE = "<<(int)(*it).m_rnti<< " rbg allocated = " <<(int) rbgPerUE<<std::endl;
      //std::cout<<"___________________________________________________________________________________"<<std::endl;

      if ((rbgPerUE < 1) && (rbgNum > 0) )
       {
         //std::cout<<" first step "<<std::endl;
         //
         cv2x_LteFlowId_t flowId ;
        flowId.m_rnti= (*it).m_rnti;
        flowId.m_lcId= (*it).m_logicalChannelIdentity;
        std::vector <cv2x_LteFlowId_t>::iterator itSFlows;
        bool findFlow=false;
        for ( itSFlows=satisfiedFlows.begin(); itSFlows!=satisfiedFlows.end(); itSFlows ++)
         {
           if ( ((*itSFlows).m_rnti == flowId.m_rnti ) && ((*itSFlows).m_lcId == flowId.m_lcId) )
            findFlow = true ;
         }
        if ( findFlow == true ) 
          {
            it ++;
            if ( it == m_rlcBufferReq.end() )  
               it = m_rlcBufferReq.begin();
            m_nextLcidDl = (*it).m_logicalChannelIdentity;
            m_nextRntiDl = (*it).m_rnti;   
            continue; 
          }
         //
         it ++;
         if ( it == m_rlcBufferReq.end() ) 
            it = m_rlcBufferReq.begin();
         continue;

       }
      else if ((rbgPerUE < 1) && (rbgNum = 0) ) // no more rbgs to allocate
       {
         //std::cout<<" second step "<<std::endl;
         m_nextRntiDl = (*it).m_rnti; // store last RNTI served
         m_nextLcidDl = (*it).m_logicalChannelIdentity;
         break;                       // no more RGB to be allocated
       }

      
      itTxMode = m_uesTxMode.find ((*it).m_rnti);
      if (itTxMode == m_uesTxMode.end ())
        {
          NS_FATAL_ERROR ("No Transmission Mode info on user " << (*it).m_rnti);
        }
      //int rbgNumOriginal = m_cschedCellConfig.m_dlBandwidth / rbgSize; //added 
      int nLayer = cv2x_TransmissionModesLayers::TxMode2LayerNum ((*itTxMode).second);
      int lcNum = (*itLcRnti).second;
      // create new cv2x_BuildDataListElement_s for this RNTI
      cv2x_BuildDataListElement_s newEl;
      newEl.m_rnti = (*it).m_rnti;
      // create the cv2x_DlDciListElement_s
      cv2x_DlDciListElement_s newDci;
      newDci.m_rnti = (*it).m_rnti;
      newDci.m_harqProcess = UpdateHarqProcessId ((*it).m_rnti);
      newDci.m_resAlloc = 0;
      newDci.m_rbBitmap = 0;
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
      int tbSize = (m_amc->GetDlTbSizeFromMcs (newDci.m_mcs.at (0), rbgPerUE * rbgSize) / 8);
      //std::cout<<" before rlc pdu , it rnti = "<<(int)(*it).m_rnti<<" it lcid = "<<(int) (*it).m_logicalChannelIdentity<<" newEl rnti = "<<(int)newEl.m_rnti<<std::endl;

      //start constructing the DCI from the first LC of this UE 
      std::list<cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator itConstructDci;
      for ( itConstructDci= m_rlcBufferReq.begin(); itConstructDci != m_rlcBufferReq.end(); itConstructDci++)
       {
         if ( (*itConstructDci).m_rnti == newEl.m_rnti)  //** added it bufferaux.m_rnti
           break;
       }
      //std::cout<<" rnti itconst = "<<(int)(*itConstructDci).m_rnti<<" lcid itconst ="<<(int)(*itConstructDci).m_logicalChannelIdentity<<std::endl;
      while ((*itConstructDci).m_rnti == newEl.m_rnti)
        {
           //std::cout<<" rnti itconst = "<<(int)(*itConstructDci).m_rnti<<" lcid itconst ="<<(int)(*itConstructDci).m_logicalChannelIdentity<<std::endl;
          if ( ((*itConstructDci).m_rlcTransmissionQueueSize > 0)
               || ((*itConstructDci).m_rlcRetransmissionQueueSize > 0)
               || ((*itConstructDci).m_rlcStatusPduSize > 0) )
            {
              std::vector <struct cv2x_RlcPduListElement_s> newRlcPduLe;
              for (uint8_t j = 0; j < nLayer; j++)
                {
                  cv2x_RlcPduListElement_s newRlcEl;
                  newRlcEl.m_logicalChannelIdentity = (*itConstructDci).m_logicalChannelIdentity;
                  //NS_LOG_INFO (this << "LCID " << (uint32_t) newRlcEl.m_logicalChannelIdentity << " size " << rlcPduSize << " ID " << (*itConstructDci).m_rnti << " layer " << (uint16_t)j);
                  //newRlcEl.m_size = rlcPduSize;
                  //find the RBGs allocated for this Flow : 
                  std::map <uint16_t, std::map <uint8_t, uint16_t> >::iterator itMap;
                  itMap = allocationMapPerRntiPerLCId.find ((uint16_t)((*itConstructDci).m_rnti));
                  std::map<uint8_t, uint16_t> tempMap;
                  std::map <uint8_t, uint16_t >::iterator itSMap;
                  itSMap=itMap->second.find ((uint8_t)(*itConstructDci).m_logicalChannelIdentity);
                  uint16_t rbgRlcPdu = (*itSMap).second;
                  uint16_t rlcPduSize = (m_amc->GetDlTbSizeFromMcs (newDci.m_mcs.at (0), rbgRlcPdu * rbgSize) / 8);
                  //std::cout<<" DL DCI , RNTI : "<<(int)(*itConstructDci).m_rnti<<" LCID : "<<(int)(*itConstructDci).m_logicalChannelIdentity<<" RBGs : "<<(int)rbgRlcPdu<< " MCS = "<<(int) newDci.m_mcs.at (0)<< std::endl;
                  /***************************************************************************************************************/
                  newRlcEl.m_size = rlcPduSize;
                  UpdateDlRlcBufferInfo ((*itConstructDci).m_rnti, newRlcEl.m_logicalChannelIdentity, rlcPduSize);
                  newRlcPduLe.push_back (newRlcEl);

                  if (m_harqOn == true)
                    {
                      // store RLC PDU list for HARQ
                      std::map <uint16_t, cv2x_DlHarqRlcPduListBuffer_t>::iterator itRlcPdu =  m_dlHarqProcessesRlcPduListBuffer.find ((*itConstructDci).m_rnti);
                      if (itRlcPdu == m_dlHarqProcessesRlcPduListBuffer.end ())
                        {
                          NS_FATAL_ERROR ("Unable to find RlcPdcList in HARQ buffer for RNTI " << (*itConstructDci).m_rnti);
                        }
                      (*itRlcPdu).second.at (j).at (newDci.m_harqProcess).push_back (newRlcEl);
                    }

                }
              newEl.m_rlcPduList.push_back (newRlcPduLe);
              lcNum--;
            }
          itConstructDci++;
          if (itConstructDci == m_rlcBufferReq.end ())
            {
              // restart from the first
              itConstructDci = m_rlcBufferReq.begin ();
              break;
            }
        }

      uint32_t rbgMask = 0;
      uint16_t i = 0;
      NS_LOG_INFO (this << " DL - Allocate user " << newEl.m_rnti << " LCs " << (uint16_t)(*itLcRnti).second << " bytes " << tbSize << " mcs " << (uint16_t) newDci.m_mcs.at (0) << " harqId " << (uint16_t)newDci.m_harqProcess <<  " layers " << nLayer);
      NS_LOG_INFO ("RBG:");
      while (i < rbgPerUE)
        {
          if (rbgMap.at (rbgAllocated) == false)
            {
              rbgMask = rbgMask + (0x1 << rbgAllocated);
              NS_LOG_INFO ("\t " << rbgAllocated);
              i++;
              rbgMap.at (rbgAllocated) = true;
              rbgAllocatedNum++;
            }
          rbgAllocated++;
        }
      newDci.m_rbBitmap = rbgMask; // (32 bit bitmap see 7.1.6 of 36.213)

      for (int i = 0; i < nLayer; i++)
        {
          newDci.m_tbsSize.push_back (tbSize);
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
          std::map <uint16_t, cv2x_DlHarqProcessesTimer_t>::iterator itHarqTimer =  m_dlHarqProcessesTimer.find (newEl.m_rnti);
          if (itHarqTimer== m_dlHarqProcessesTimer.end ())
            {
              NS_FATAL_ERROR ("Unable to find HARQ timer for RNTI " << (uint16_t)newEl.m_rnti);
            }
          (*itHarqTimer).second.at (newDci.m_harqProcess) = 0;
        }
      // ...more parameters -> ignored in this version

       ret.m_buildDataList.push_back (newEl);
      //added to serve for each TTI the next UE if the current UE is not satisfied even when allocating all RBGs
      std::map <uint16_t, std::map <uint8_t, uint16_t> >::iterator itMapF = allocationMapPerRntiPerLCId.find(newEl.m_rnti) ;
      std::map <uint8_t, uint16_t> mapLCF=(*itMapF).second;
      std::map <uint8_t, uint16_t>::iterator itmapLCF=mapLCF.begin();
      uint16_t rbgF = 0 ;
      uint16_t sumRbgs = 0 ;
      for ( itmapLCF = mapLCF.begin(); itmapLCF != mapLCF.end() ; itmapLCF ++ )
       {
          sumRbgs = sumRbgs + (*itmapLCF).second;
          if ( (*itmapLCF).second == (rbgNumOriginal - alreadyAllocatedRbg) ) 
            {
              rbgF = rbgNumOriginal; 
            }
       }
       
      //std::cout<<" rnti buffer  = "<<(int) (*itBuffer).m_rnti <<" lcid buffer  = "<<(int) (*itBuffer).m_logicalChannelIdentity<<std::endl;
      //std::cout<<" rnti buffer aux = "<<(int) (*itBufferAux).m_rnti <<" lcid buffer aux = "<<(int) (*itBufferAux).m_logicalChannelIdentity<<std::endl;
      //std::cout<<" before "<<std::endl;
      it = itBufferAux;
      //case 1 : all RBG are allocated for one flow --> pass to the next flow ( same UE or different UE ) 
      if ( rbgF == rbgNumOriginal ) 
       {  
          //std::cout<<" case 1 "<<std::endl;
          //it ++;
          if ( it == m_rlcBufferReq.end() ) 
            it = m_rlcBufferReq.begin();

          m_nextLcidDl = (*it).m_logicalChannelIdentity;
          m_nextRntiDl = (*it).m_rnti;                  
       }
      // case 2 : all RBGs are used during this TTI , but the flow still not satisfied , serve it during the next TTI 
      else if ( sumRbgs == (rbgNumOriginal - alreadyAllocatedRbg) && ( PassToNextLcid == false ) )
       {
         //std::cout<<" case 2 "<<std::endl;
         m_nextLcidDl = (*it).m_logicalChannelIdentity;
         m_nextRntiDl = (*it).m_rnti;       
       }
       // case 3 pass to the next UE , just increment it 
      else if ( PassToNextUE==true ) 
      {
        //std::cout<<" case 3 "<<std::endl;
        /*************************************************************************/
        std::map <uint16_t, std::map <uint8_t, uint16_t> >::iterator itMapTemp = allocationMapPerRntiPerLCId.find((*itBufferAux).m_rnti) ;
        std::map <uint8_t, uint16_t> mapLCF=(*itMapTemp).second;
        std::map <uint8_t, uint16_t>::iterator itmapLCF=mapLCF.find((*itBufferAux).m_logicalChannelIdentity);
        cv2x_LteFlowId_t flowId ;
        flowId.m_rnti= (*itBufferAux).m_rnti;
        flowId.m_lcId= (*itBufferAux).m_logicalChannelIdentity;
        std::vector <cv2x_LteFlowId_t>::iterator itSFlows;
        bool findFlow=false;
        for ( itSFlows=satisfiedFlows.begin(); itSFlows!=satisfiedFlows.end(); itSFlows ++)
         {
           if ( ((*itSFlows).m_rnti == flowId.m_rnti ) && ((*itSFlows).m_lcId == flowId.m_lcId) )
            findFlow = true ;
         }
        
        uint16_t rbgF = (*itmapLCF).second;
        if ( (rbgF != (rbgNumOriginal - alreadyAllocatedRbg))  && (findFlow == false) ) 
           { it = itBufferAux;
             m_nextLcidDl = (*it).m_logicalChannelIdentity;
             m_nextRntiDl = (*it).m_rnti;    
             //std::cout<<" case 3: subcase 1 "<<std::endl;
           }
        /*************************************************************************/
        else 
         {
            it ++;
            if ( it == m_rlcBufferReq.end() ) 
            it = m_rlcBufferReq.begin();
            //std::cout<<" case 3: subcase 2 "<<std::endl;
         }
      }

      
 
    //std::cout<<" \t next lcid = "<<(int) m_nextLcidDl<<" next rnti = "<<(int) m_nextRntiDl<<std::endl;

    //std::cout<<" \t it rnti = "<<(int) (*it).m_rnti << " it lcid = "<<(int) (*it).m_logicalChannelIdentity<<std::endl;
   }
  while (((*it).m_rnti != m_nextRntiDl) || ( (*it).m_logicalChannelIdentity != m_nextLcidDl ) );

  ret.m_nrOfPdcchOfdmSymbols = 1;   /// \todo check correct value according the DCIs txed  

  //print allocation map
  {
    uint16_t usedRBGs = 0;
    std::map <uint16_t, std::map <uint8_t, uint16_t > >::iterator allocMapRntiIt;
    for (allocMapRntiIt = allocationMapPerRntiPerLCId.begin() ; allocMapRntiIt != allocationMapPerRntiPerLCId.end(); allocMapRntiIt++) 
      {
        //uint16_t rnti = allocMapRntiIt->first;
        std::map <uint8_t, uint16_t >::iterator allocMapLcidIt;
        for (allocMapLcidIt = allocMapRntiIt->second.begin() ; allocMapLcidIt != allocMapRntiIt->second.end() ; allocMapLcidIt++) 
          {
            //uint8_t lcid = allocMapLcidIt->first;
            usedRBGs += allocMapLcidIt->second;
            //std::cout << Simulator::Now ().GetSeconds () << " ALLOC RNTI " << rnti << " LCID " << (uint16_t) lcid << " RBGs " << allocMapLcidIt->second << std::endl;          
          }
      }
    //std::cout << Simulator::Now ().GetSeconds () << " TOTAL USED " << usedRBGs << std::endl;          
  }


  m_schedSapUser->SchedDlConfigInd (ret);
  return;

}

void
cv2x_RrSlFfMacScheduler::DoSchedDlRachInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedDlRachInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  
  m_rachList = params.m_rachList;

  return;
}

void
cv2x_RrSlFfMacScheduler::DoSchedDlCqiInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params)
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

void
cv2x_RrSlFfMacScheduler::DoSchedUlTriggerReq (const struct cv2x_FfMacSchedSapProvider::SchedUlTriggerReqParameters& params)
{
  /************************* Comments ********************************************************************************/
  /* - Enhanced version of the RR scheduler : assign to each flow only the required RBs to transmit its data         */
  /*_________________________________________________________________________________________________________________*/
  NS_LOG_FUNCTION (this << " UL - Frame no. " << (params.m_sfnSf >> 4) << " subframe no. " << (0xF & params.m_sfnSf) << " size " << params.m_ulInfoList.size ());

  RefreshUlCqiMaps ();
  //std::cout<<" ENTER cv2x_RrSlFfMacScheduler::DoSchedUlTriggerReq "<<std::endl;
  // Generate RBs map
  cv2x_FfMacSchedSapUser::SchedUlConfigIndParameters ret;
  std::vector <bool> rbMap;
  uint16_t rbAllocatedNum = 0;
  std::set <uint16_t> rntiAllocated;
  std::vector <uint16_t> rbgAllocationMap;
  rbgAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);
  std::vector <uint16_t> slAllocationMap;
  slAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);
  
  uint32_t frameNo = params.m_sfnSf >> 4;
  uint32_t subframeNo = 0xF & params.m_sfnSf;
  // Check if we have new sidelink pools to initialize
  std::map <uint32_t, PoolInfo>::iterator poolIt;
  for (poolIt = m_poolAllocations.begin() ; poolIt != m_poolAllocations.end() ; poolIt++)
    {
      if (!poolIt->second.m_init)
        {
          poolIt->second.m_npscch = poolIt->second.m_pool->GetNPscch();
          poolIt->second.m_currentScPeriod.frameNo = 0; //init to 0 to make it invalid
          poolIt->second.m_currentScPeriod.subframeNo = 0;
          poolIt->second.m_nextScPeriod = poolIt->second.m_pool->GetNextScPeriod (frameNo, subframeNo);
          //adjust because scheduler starts with frame/subframe = 1
          poolIt->second.m_nextScPeriod.frameNo++;
          poolIt->second.m_nextScPeriod.subframeNo++;
          NS_LOG_INFO (this << " Init SC period for pool of group " << poolIt->first << ". Next period at " << poolIt->second.m_nextScPeriod.frameNo << "/" << poolIt->second.m_nextScPeriod.subframeNo);
          poolIt->second.m_init = true;
        }

      std::map <uint16_t,uint32_t>::iterator it;
    
      for (it = poolIt->second.m_ceSlBsrRxed.begin (); it != poolIt->second.m_ceSlBsrRxed.end (); it++)
        {
          if (poolIt->second.m_nextAllocation.find ((*it).first) == poolIt->second.m_nextAllocation.end()) {
            //new allocation
            PoolUserAllocation alloc;
            alloc.m_rnti = it->first;
            alloc.m_n = poolIt->second.m_nextAllocation.size();
            alloc.m_slItrp = m_slItrp;
            alloc.m_rbStart = poolIt->second.m_nextAllocation.size() * m_slGrantSize;
            alloc.m_rbLen = m_slGrantSize;
            alloc.m_mcs = m_slGrantMcs; //actually the MCS is taken from the pool. Will probably remove this

              //adjust PSSCH frame to next period
              SidelinkCommResourcePool::SubframeInfo tmp;
              tmp.frameNo = poolIt->second.m_nextScPeriod.frameNo-1;
              tmp.subframeNo = poolIt->second.m_nextScPeriod.subframeNo-1;
            
              alloc.m_psschTx = poolIt->second.m_pool->GetPsschTransmissions (tmp, alloc.m_slItrp, alloc.m_rbStart, alloc.m_rbLen);
            
              for (std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator txIt = alloc.m_psschTx.begin (); txIt != alloc.m_psschTx.end (); txIt++)
                {
                  //txIt->subframe = txIt->subframe + tmp;
                  //adjust for index starting at 1
                  txIt->subframe.frameNo++;
                  txIt->subframe.subframeNo++;
                }
            
              poolIt->second.m_nextAllocation.insert (std::pair <uint16_t, PoolUserAllocation> (it->first, alloc));
              NS_LOG_INFO (this << " Adding sidelink allocation for RNTI " << it->first << " in pool " << poolIt->first << " RbStart=" << (uint32_t) alloc.m_rbStart << ", rbLen=" << (uint32_t) alloc.m_rbLen << ", itrp=" << (uint32_t) alloc.m_slItrp);


              cv2x_SlDciListElement_s dci;
              dci.m_rnti = alloc.m_rnti;
              dci.m_resPscch = alloc.m_n;
              dci.m_trp = alloc.m_slItrp;
              dci.m_rbStart = alloc.m_rbStart;
              dci.m_rbLen = alloc.m_rbLen;
            
              ret.m_sldciList.push_back (dci);
            
              for (std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator txIt = alloc.m_psschTx.begin (); txIt != alloc.m_psschTx.end (); txIt++)
                {
                  NS_LOG_INFO (this << " Subframe " << txIt->subframe.frameNo << "/" << txIt->subframe.subframeNo << ": rbStart=" << (uint32_t) txIt->rbStart << ", rbLen=" << (uint32_t) txIt->nbRb);
                }

            } // else we already allocated for this UE, nothing to do since we have fixed allocation
          // a enhanced version would adapt the size of the allocation depending on the buffer status
        }
      poolIt->second.m_ceSlBsrRxed.clear ();
    }

  
  //Check the SC period for each pool
  //Reserve the RBs for sidelink so we don't schedule uplink at the same time
  for (poolIt = m_poolAllocations.begin() ; poolIt != m_poolAllocations.end() ; poolIt++)
    {
      if (frameNo == poolIt->second.m_nextScPeriod.frameNo && subframeNo == poolIt->second.m_nextScPeriod.subframeNo)
        {
           //move allocation
          poolIt->second.m_currentAllocation = poolIt->second.m_nextAllocation;
          poolIt->second.m_nextAllocation.clear ();
          poolIt->second.m_currentScPeriod = poolIt->second.m_nextScPeriod;
          poolIt->second.m_nextScPeriod = poolIt->second.m_pool->GetNextScPeriod (frameNo, subframeNo);
          //adjust because scheduler starts with frame/subframe = 1
          poolIt->second.m_nextScPeriod.frameNo++;
          poolIt->second.m_nextScPeriod.subframeNo++;
          NS_LOG_INFO (this << " Starting new SC period for pool of group " << poolIt->first << ". Next period at " << poolIt->second.m_nextScPeriod.frameNo << "/" << poolIt->second.m_nextScPeriod.subframeNo);
          NS_LOG_INFO (this << " Number of UEs allocated = " << poolIt->second.m_currentAllocation.size ());
       }
      //check if there are PSCCH allocations in this subframe
      std::list<uint8_t> rbPscch = poolIt->second.m_pool->GetPscchOpportunities(frameNo-1, subframeNo-1);
      std::list<uint8_t>::iterator rbIt;
      
      for (rbIt = rbPscch.begin (); rbIt != rbPscch.end (); rbIt++)
        {
          //NS_LOG_INFO ("Reserving RB " << (uint32_t) (*rbIt) << " for sidelink PSCCH");
          slAllocationMap.at(*rbIt) = 999; //what RNTI to use for PSCCH?
        }
      
      //check PSSCH allocation for each user in the pool
      for (std::map <uint16_t, PoolUserAllocation>::iterator userIt = poolIt->second.m_currentAllocation.begin () ; userIt != poolIt->second.m_currentAllocation.end()  ; userIt++)
        {
          //check if this user is transmitting in this subframe
          std::list<SidelinkCommResourcePool::SidelinkTransmissionInfo>::iterator allocIt = userIt->second.m_psschTx.begin();
          if (allocIt!= userIt->second.m_psschTx.end () && (*allocIt).subframe.frameNo == frameNo && (*allocIt).subframe.subframeNo == subframeNo)
            {
              //User transmitting in this subframe, reserve RBs
              NS_LOG_INFO (this << " Reserving RBs for sidelink PSSCH from " << (uint32_t) (*allocIt).rbStart << " to " << (uint32_t) ((*allocIt).rbStart+(*allocIt).nbRb-1));
              for (int j = (*allocIt).rbStart; j < (*allocIt).rbStart + (*allocIt).nbRb ; j++)
                {
                  slAllocationMap.at(j) = userIt->first;
                }
              
            //remove entry 
              userIt->second.m_psschTx.erase (allocIt);
            }
        }
    }


  //use RACH as default allocation (highest priority)
  rbgAllocationMap = m_rachAllocationMap;
  //rbgAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);
  m_rachAllocationMap.clear ();
  m_rachAllocationMap.resize (m_cschedCellConfig.m_ulBandwidth, 0);

  rbMap.resize (m_cschedCellConfig.m_ulBandwidth, false);
  // remove RACH allocation
  for (uint16_t i = 0; i < m_cschedCellConfig.m_ulBandwidth; i++)
    {
      if (rbgAllocationMap.at (i) != 0)
        {
          rbMap.at (i) = true;
          NS_LOG_DEBUG (this << " Allocated for RACH " << i);
        }
    }

  //uplink HARQ next
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
              NS_LOG_INFO (this << " UL-HARQ retx RNTI " << rnti << " harqId " << (uint16_t)harqId);
              std::map <uint16_t, cv2x_UlHarqProcessesDciBuffer_t>::iterator itHarq = m_ulHarqProcessesDciBuffer.find (rnti);
              if (itHarq == m_ulHarqProcessesDciBuffer.end ())
                {
                  NS_LOG_ERROR ("No info find in UL-HARQ buffer for UE (might change eNB) " << rnti);
                }
              cv2x_UlDciListElement_s dci = (*itHarq).second.at (harqId);
              std::map <uint16_t, cv2x_UlHarqProcessesStatus_t>::iterator itStat = m_ulHarqProcessesStatus.find (rnti);
              if (itStat == m_ulHarqProcessesStatus.end ())
                {
                  NS_LOG_ERROR ("No info find in HARQ buffer for UE (might change eNB) " << rnti);
                }
              if ((*itStat).second.at (harqId) > 3)
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
                      rbMap.at (j) = true;
                      rbgAllocationMap.at (j) = dci.m_rnti;
                      NS_LOG_INFO ("\tRB " << j);
                      rbAllocatedNum++;
                    }
                  NS_LOG_INFO (this << " Send retx in the same RBGs " << (uint16_t)dci.m_rbStart << " to " << dci.m_rbStart + dci.m_rbLen << " RV " << (*itStat).second.at (harqId) + 1);
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
        }
    }


  // update with sidelink allocation map
  for (uint16_t i = 0; i < m_cschedCellConfig.m_ulBandwidth; i++)
    {
      //update if there is sidelink allocation and no uplink allocation
      //if uplink transmission is allocated, the eNodeB needs to keep information about the UEs
      if (slAllocationMap.at(i) != 0 && rbgAllocationMap.at(i) == 0)
        {
          rbgAllocationMap.at(i) = slAllocationMap.at (i);
          rbMap.at (i) = true;
          NS_LOG_DEBUG (this << " Allocated for sidelink " << i);
        }
  }

  
  std::multimap <uint16_t, std::map <uint8_t, uint32_t> >::iterator it;
  int nflows = 0;

  for (it = m_ceBsrRxed.begin (); it != m_ceBsrRxed.end (); it++)
    {
      //std::cout<<" rnti = "<<(int)(*it).first<<std::endl;
      std::set <uint16_t>::iterator itRnti = rntiAllocated.find ((*it).first);
      // select UEs with queues not empty and not yet allocated for HARQ
      std::map <uint8_t, uint32_t> mapLC=it->second;
      std::map <uint8_t, uint32_t>::iterator itLC;
      for (itLC=mapLC.begin(); itLC!=mapLC.end(); itLC++) 
       {  
         //std::cout<<" lcid = "<<(int)(*itLC).first <<" data = "<<(int)(*itLC).second <<std::endl;
         if (((*itLC).second > 0)&&(itRnti == rntiAllocated.end ()))
          {
            nflows++;
          }
       }
   }
  //std::cout<<" nflows = "<<(int)nflows<<std::endl;

  if (nflows == 0)
    {
      if (ret.m_dciList.size () > 0 || ret.m_sldciList.size () > 0)
        {
          m_allocationMaps.insert (std::pair <uint16_t, std::vector <uint16_t> > (params.m_sfnSf, rbgAllocationMap));
          m_schedSapUser->SchedUlConfigInd (ret);
        }
      return;  // no flows to be scheduled
    }


  uint16_t rbAllocated = 0;

  if (m_nextRntiUl != 0)
    {
      for (it = m_ceBsrRxed.begin (); it != m_ceBsrRxed.end (); it++)
        {
          if ((*it).first == m_nextRntiUl)
            {
              break;
            }
        }
      if (it == m_ceBsrRxed.end ())
        {
          NS_LOG_ERROR (this << " no user found");
        }
    }
  else
    {
      it = m_ceBsrRxed.begin ();
      m_nextRntiUl = (*it).first;
    }

  std::map <uint16_t, std::map <uint8_t, uint16_t> > satisfiedUes;       //structure to save satisfied flows 
  std::map <uint16_t, std::map <uint8_t, uint16_t> >::iterator itSUes;
  double bytesTxedF = 0;                          // accumulated assigned data for each flows             
  uint16_t rbNum= m_cschedCellConfig.m_ulBandwidth;
  uint16_t rbPerFlow = 0;  // intialization of rbPerFlow
  
  
  do
    {
      std::set <uint16_t>::iterator itRnti = rntiAllocated.find ((*it).first);
      uint32_t dataBsr = 0;
      std::map <uint8_t, uint32_t> mapDataBsr= (*it).second;
      std::map <uint8_t, uint32_t>::iterator itDataBsr;
      for ( itDataBsr = mapDataBsr.begin(); itDataBsr != mapDataBsr.end(); itDataBsr ++)
       {
          dataBsr += (*itDataBsr).second;
       } 
      //std::cout<<"\t rnti = "<<(int) (*it).first<<" data bsr = "<<(int) dataBsr <<std::endl;
      if ((itRnti != rntiAllocated.end ())||(dataBsr== 0))
        {
          // UE already allocated for UL-HARQ -> skip it
          it++;
          if (it == m_ceBsrRxed.end ())
            {
              // restart from the first
              it = m_ceBsrRxed.begin ();
            }
          continue;
        }

     std::map <uint8_t, uint32_t> mapFlow=it->second;
     std::map <uint8_t, uint32_t>::iterator itFlow;
     for (itFlow=mapFlow.begin(); itFlow!=mapFlow.end(); itFlow++) 
       {
     int i = 0;
     while (i<rbNum) 
     {
       //std::cout<<" \t i = " <<(int) i <<std::endl; 
       bool findF=false;

           itSUes = satisfiedUes.find((*it).first);
           if ( itSUes != satisfiedUes.end() ) 
            {
               std::map <uint8_t, uint16_t> satisfiedFlows= (*itSUes).second;
               std::map <uint8_t, uint16_t>:: iterator itSFlows;
               itSFlows = satisfiedFlows.find ((*itFlow).first);
               if (itSFlows != satisfiedFlows.end() ) 
                 findF = true;
            }

            if (findF==false)
             { 
               //std::cout<<" \t rnti = "<<(int)(*it).first<<" lcid = "<<(int)(*itFlow).first<<" not yet satisfied ! "<<std::endl;
               // this flows doesn't exist in the satisfied flows --> needs more RBs to transmit           
               // try to find this flow in the m_ceBsrRxed
               //std::cout<<"\t \t this UE: "<<(int)(*it).first<<" has data to transmit= "<<(int)(*it).second<<std::endl;
               std::map <uint16_t,uint8_t>::iterator itTxMode;
               itTxMode = m_uesTxMode.find ((*it).first);
               if (itTxMode == m_uesTxMode.end ())
                {
                  NS_FATAL_ERROR ("No Transmission Mode info on user " << (*it).first);
                }
               uint8_t mcsF = 0;
               std::map <uint16_t, std::vector <double> >::iterator itCqi = m_ueCqi.find ((*it).first);
               int cqi = 0;
               int b=1;
               if (itCqi == m_ueCqi.end ())
                {
                  // no cqi info about this UE
                  mcsF=0;
                }
               else
                {   
                   b= b + i;   // number of allocated RBs + 1 (the current RB)             
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
                   mcsF = m_amc->GetMcsFromCqi (cqi);

                } // end  if (itCqi == m_ueCqi.end ())

               bytesTxedF=(m_amc->GetUlTbSizeFromMcs (mcsF, b * 1) / 8); 
               //std::cout<<" \t data allocated = "<<(int) bytesTxedF<<" MCS = "<<(int)mcsF<<" data to transmit = "<<(int) (*itFlow).second<<std::endl;

               if ((*itFlow).second <= bytesTxedF )
                {
                   //this flow is  satisfied with this new allocation --> add it to satisfied flows 
                   //std::cout<<" \t\tthis flow is satisfied with thos new allocation , b = "<<(int)b <<std::endl;
                   std::map <uint16_t, std::map <uint8_t, uint16_t> >::iterator itTemp1 = satisfiedUes.find((*it).first);
                   if ( itTemp1 == satisfiedUes.end() ) // first allocation of this rnti 
                    {
                       std::map <uint8_t, uint16_t> mapflow ;
                       mapflow.insert ( std::pair<uint8_t, uint16_t > ((*itFlow).first, b));
                       satisfiedUes.insert ( std::pair <uint16_t, std::map<uint8_t,uint16_t > > ((*it).first, mapflow));
                       rbNum=rbNum-b;
                       
                       // added to not leave the loop if just only one flow is satisfied for one UE 
                       //std::cout<<" mapflow size = "<<(int)mapflow.size()<<" mapFlow size = "<<(int)mapFlow.size()<<std::endl;
     
                        i=rbNum;
                       
                    }
                   else    // this rnti already exists in the satosfied UE , check now the lcid 
                    {
                      std::map <uint8_t, uint16_t >::iterator itSMap;
                      itSMap=itTemp1->second.find ((*itFlow).first);
                      if (itSMap == itTemp1->second.end ())
                       {
                          itTemp1->second.insert (std::pair<uint8_t, uint16_t > ((*itFlow).first, b));
                          rbNum=rbNum-b;
                          // added to not leave the loop if just only one flow is satisfied for one UE 
                          //std::cout<<" mapflow size = "<<(int)itTemp1->second.size()<<" mapFlow size = "<<(int)mapFlow.size()<<std::endl;
                           i=rbNum;
                          
                       }
                    } // end if itTemp1 == satisfiedUes.end()
                } 
               else
                {
                 i++;    
                 if ( i == rbNum ) // no possibility to allocate more ressources to this UE  --> add it in satisfied UEs even it is not satisfied 
                  {
                    //std::cout<<" \t \t no more resources --> satisfied , b = "<<(int) b <<std::endl;
                    b=i;
                    rbNum=rbNum-i;
                    //rbPerFlow=b;
                    std::map <uint16_t, std::map <uint8_t, uint16_t> >::iterator itTemp1 = satisfiedUes.find((*it).first);
                    if ( itTemp1 == satisfiedUes.end() ) // first allocation of this rnti 
                     {
                       std::map <uint8_t, uint16_t> mapflow ;
                       mapflow.insert ( std::pair<uint8_t, uint16_t > ((*itFlow).first, b));
                       satisfiedUes.insert ( std::pair <uint16_t, std::map<uint8_t,uint16_t > > ((*it).first, mapflow));
                     }
                    else
                     {
                      
                       std::map <uint8_t, uint16_t >::iterator itSMap;
                       itSMap=itTemp1->second.find ((*itFlow).first);
                       if (itSMap == itTemp1->second.end ())
                        { 
                           itTemp1->second.insert (std::pair<uint8_t, uint16_t > ((*itFlow).first, b));
                        }
                     }
                  } // end if ( i == rbNum ) 
               } //end if (*itFlow).second <= bytesTxedF 
            }  
           else 
            { 
              i=rbNum;
            }   // end if (findF==false)
               

       } // end while (i<rbNum)
 } //end for (itFlow=mapFlow.begin(); itFlow!=mapFlow.end(); itFlow++)  
      
      //compute rb per flow ( per UE ) 
      rbPerFlow = 0;
      std::map <uint16_t, std::map <uint8_t, uint16_t> >::iterator itTemp2 = satisfiedUes.find((*it).first);
      if ( itTemp2 != satisfiedUes.end() ) 
       {
         std::map <uint8_t, uint16_t> mapTemp2 = itTemp2->second;
         std::map <uint8_t, uint16_t>::iterator itSTemp2;
         for ( itSTemp2 = mapTemp2.begin();  itSTemp2 != mapTemp2.end(); itSTemp2 ++ ) 
          {
            rbPerFlow = rbPerFlow + (*itSTemp2).second;
          }
      }
     // std::cout<<" \t END ALLOCATION FOR THIS UE "<<(int) (*it).first<< " rb allocated = "<<(int) rbPerFlow<< std::endl;
      if (rbPerFlow < 3)
       {
         rbPerFlow = 3;  // at least 3 rbg per flow (till available resource) to ensure TxOpportunity >= 7 bytes
       }

      if (rbAllocated + rbPerFlow - 1 > m_cschedCellConfig.m_ulBandwidth)
        {
          // limit to physical resources last resource assignment
          rbPerFlow = m_cschedCellConfig.m_ulBandwidth - rbAllocated;
          // at least 3 rbg per flow to ensure TxOpportunity >= 7 bytes
          if (rbPerFlow < 3)
            {
              // terminate allocation
              rbPerFlow = 0;      
            }
        }
      NS_LOG_INFO (this << " try to allocate " << (*it).first);
      cv2x_UlDciListElement_s uldci;
      uldci.m_rnti = (*it).first;
      uldci.m_rbLen = rbPerFlow;
      bool allocated = false;
      NS_LOG_INFO (this << " RB Allocated " << rbAllocated << " rbPerFlow " << rbPerFlow << " flows " << nflows);
      while ((!allocated)&&((rbAllocated + rbPerFlow - m_cschedCellConfig.m_ulBandwidth) < 1) && (rbPerFlow != 0))
        {
          // check availability
          bool free = true;
          for (uint16_t j = rbAllocated; j < rbAllocated + rbPerFlow; j++)
            {
              if (rbMap.at (j) == true)
                {
                  free = false;
                  break;
                }
            }
          if (free)
            {
              uldci.m_rbStart = rbAllocated;

              for (uint16_t j = rbAllocated; j < rbAllocated + rbPerFlow; j++)
                {
                  rbMap.at (j) = true;
                  // store info on allocation for managing ul-cqi interpretation
                  rbgAllocationMap.at (j) = (*it).first;
                  NS_LOG_INFO ("\t " << j);
                }
              rbAllocated += rbPerFlow;
              allocated = true;
              break;
            }
          rbAllocated++;
          if (rbAllocated + rbPerFlow - 1 > m_cschedCellConfig.m_ulBandwidth)
            {
              // limit to physical resources last resource assignment
              rbPerFlow = m_cschedCellConfig.m_ulBandwidth - rbAllocated;
              // at least 3 rbg per flow to ensure TxOpportunity >= 7 bytes
              if (rbPerFlow < 3)
                {
                  // terminate allocation
                  rbPerFlow = 0;                 
                }
            }
        }
      if (!allocated)
        {
          // unable to allocate new resource: finish scheduling
          m_nextRntiUl = (*it).first;
          if (ret.m_dciList.size () > 0 || ret.m_sldciList.size () > 0)
            {
              m_schedSapUser->SchedUlConfigInd (ret);
            }
          m_allocationMaps.insert (std::pair <uint16_t, std::vector <uint16_t> > (params.m_sfnSf, rbgAllocationMap));
          return;
        }

      ///// Compute Tb size 
      std::map <uint16_t, std::vector <double> >::iterator itCqi = m_ueCqi.find ((*it).first);
      int cqi = 0;
      if (itCqi == m_ueCqi.end ())
        {
          // no cqi info about this UE
          uldci.m_mcs = 0; // MCS 0 -> UL-AMC TBD
          NS_LOG_INFO (this << " UE does not have ULCQI " << (*it).first );
        }
      else
        {
          // take the lowest CQI value (worst RB)
          double minSinr = (*itCqi).second.at (uldci.m_rbStart);
          for (uint16_t i = uldci.m_rbStart; i < uldci.m_rbStart + uldci.m_rbLen; i++)
            {
              if ((*itCqi).second.at (i) < minSinr)
                {
                  minSinr = (*itCqi).second.at (i);
                }
            }
          // translate SINR -> cqi: WILD ACK: same as DL
          double s = log2 ( 1 + (
                                 std::pow (10, minSinr / 10 )  /
                                 ( (-std::log (5.0 * 0.00005 )) / 1.5) ));


          cqi = m_amc->GetCqiFromSpectralEfficiency (s);
          if (cqi == 0)
            {
              //added
              satisfiedUes.erase ((*it).first);
              it++;
              if (it == m_ceBsrRxed.end ())
                {
                  // restart from the first
                  it = m_ceBsrRxed.begin ();
                }
              NS_LOG_DEBUG (this << " UE discared for CQI=0, RNTI " << uldci.m_rnti);
              // remove UE from allocation map
              for (uint16_t i = uldci.m_rbStart; i < uldci.m_rbStart + uldci.m_rbLen; i++)
                {
                  rbgAllocationMap.at (i) = 0;
                  rbMap.at (i) = false;
                }
              rbNum=rbNum+rbPerFlow;
              rbAllocated=rbAllocated-rbPerFlow;
              
              
              continue; // CQI == 0 means "out of range" (see table 7.2.3-1 of 36.213)
            }
          uldci.m_mcs = m_amc->GetMcsFromCqi (cqi);
        }
      uldci.m_tbSize = (m_amc->GetUlTbSizeFromMcs (uldci.m_mcs, rbPerFlow) / 8); // MCS 0 -> UL-AMC TBD

      //UpdateUlRlcBufferInfo (uldci.m_rnti, uldci.m_tbSize);
      /************************************************************************/
      itTemp2 = satisfiedUes.find((*it).first);
      if ( itTemp2 != satisfiedUes.end() ) 
       {
         std::map <uint8_t, uint16_t> mapTemp2 = itTemp2->second;
         std::map <uint8_t, uint16_t>::iterator itSTemp2;
         for ( itSTemp2 = mapTemp2.begin();  itSTemp2 != mapTemp2.end(); itSTemp2 ++ ) 
          {
             UpdateUlRlcBufferInfo (uldci.m_rnti, (*itSTemp2).first, (m_amc->GetUlTbSizeFromMcs (uldci.m_mcs, (*itSTemp2).second) / 8)); 
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
          // Update HARQ process status (RV 0)
          std::map <uint16_t, cv2x_UlHarqProcessesStatus_t>::iterator itStat = m_ulHarqProcessesStatus.find (uldci.m_rnti);
          if (itStat == m_ulHarqProcessesStatus.end ())
            {
              NS_LOG_ERROR ("No info find in HARQ buffer for UE (might change eNB) " << uldci.m_rnti);
            }
          (*itStat).second.at (harqId) = 0;
        }
        
      NS_LOG_INFO (this << " UL Allocation - UE " << (*it).first << " startPRB " << (uint32_t)uldci.m_rbStart << " nPRB " << (uint32_t)uldci.m_rbLen << " CQI " << cqi << " MCS " << (uint32_t)uldci.m_mcs << " TBsize " << uldci.m_tbSize << " harqId " << (uint16_t)harqId);

      it++;
      if (it == m_ceBsrRxed.end ())
        {
          // restart from the first
          it = m_ceBsrRxed.begin ();
        }
      if ((rbAllocated == m_cschedCellConfig.m_ulBandwidth) || (rbPerFlow == 0))
        {
          // Stop allocation: no more PRBs
          m_nextRntiUl = (*it).first;
          break;
        }
    }
  while (((*it).first != m_nextRntiUl)&&(rbPerFlow!=0));

  m_allocationMaps.insert (std::pair <uint16_t, std::vector <uint16_t> > (params.m_sfnSf, rbgAllocationMap));
  m_schedSapUser->SchedUlConfigInd (ret);
  return;
}

void
cv2x_RrSlFfMacScheduler::DoSchedUlNoiseInterferenceReq (const struct cv2x_FfMacSchedSapProvider::SchedUlNoiseInterferenceReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  return;
}

void
cv2x_RrSlFfMacScheduler::DoSchedUlSrInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlSrInfoReqParameters& params)
{
  NS_LOG_FUNCTION (this);
  return;
}

void
cv2x_RrSlFfMacScheduler::DoSchedUlMacCtrlInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params)
{
  
  //std::cout<<" ENTER MtFfMacScheduler::DoSchedUlMacCtrlInfoReq "<<std::endl;
  NS_LOG_FUNCTION (this);
  std::multimap <uint16_t, std::map <uint8_t, uint32_t> >::iterator it;
  std::map <uint16_t,uint32_t>::iterator itSlBsr;
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
               } // end it == m_ceBsrRxed.end ()
           } //end for (itBsr=mapBsr.begin()
        } 
       else if (params.m_macCeList.at (i).m_macCeType == cv2x_MacCeListElement_s::SLBSR )
        {
          //for sidelink, the allocation is done per period
          //for now we check if we already made an allocation for this UE in the next period
          //if not, we add one 
          NS_LOG_INFO ("Received Sidelink BSR");
          //the index in the buffer identifies the group

          uint16_t rnti = params.m_macCeList.at (i).m_rnti;
          //get the pool for this destination
          std::map <uint16_t,std::vector <uint32_t> >::iterator ueIt = m_uesDestinations.find (rnti);
          NS_ASSERT_MSG (ueIt != m_uesDestinations.end (), "No destinations found for " << rnti);

          NS_ASSERT_MSG (ueIt->second.size() > 0, "Destinations empty for " << rnti);
          
          //check the report
          for (uint8_t dest = 0; dest < ueIt->second.size(); ++dest)
            {
              uint8_t bsrId = params.m_macCeList.at (i).m_macCeValue.m_bufferStatus.at (dest);
              uint32_t buffer = cv2x_BufferSizeLevelBsr::BsrId2BufferSize (bsrId);
              if (buffer > 0) {
                uint32_t destination = ueIt->second.at (dest);
                std::map <uint32_t, PoolInfo>::iterator poolIt = m_poolAllocations.find (destination);
                NS_ASSERT_MSG (poolIt != m_poolAllocations.end (), "Sidelink destination " << destination << "unknown.");
                
                itSlBsr = poolIt->second.m_ceSlBsrRxed.find (rnti);
                if (itSlBsr == poolIt->second.m_ceSlBsrRxed.end ())
                  {
                    // create the new entry
                    poolIt->second.m_ceSlBsrRxed.insert ( std::pair<uint16_t, uint32_t > (rnti, buffer));
                    NS_LOG_INFO (this << " Insert RNTI " << rnti << " sidelink queue " << buffer);
                  }
                else
                  {
                    // update the buffer size value
                    (*itSlBsr).second = buffer;
                    NS_LOG_INFO (this << " Update RNTI " << rnti << " sidelink queue " << buffer);
                  }
              }
            }    
        }  
    } //end for 
  return; 
}

void
cv2x_RrSlFfMacScheduler::DoSchedUlCqiInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params)
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
        std::map <uint16_t, std::vector <uint16_t> >::iterator itMap;
        std::map <uint16_t, std::vector <double> >::iterator itCqi;
        itMap = m_allocationMaps.find (params.m_sfnSf);
        if (itMap == m_allocationMaps.end ())
          {
            NS_LOG_INFO (this << " Does not find info on allocation, size : " << m_allocationMaps.size ());
            return;
          }
        for (uint32_t i = 0; i < (*itMap).second.size (); i++)
          {
            // convert from fixed point notation Sxxxxxxxxxxx.xxx to double
            double sinr = cv2x_LteFfConverter::fpS11dot3toDouble (params.m_ulCqi.m_sinr.at (i));
            itCqi = m_ueCqi.find ((*itMap).second.at (i));
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
                        newCqi.push_back (30.0);
                      }

                  }
                m_ueCqi.insert (std::pair <uint16_t, std::vector <double> > ((*itMap).second.at (i), newCqi));
                // generate correspondent timer
                m_ueCqiTimers.insert (std::pair <uint16_t, uint32_t > ((*itMap).second.at (i), m_cqiTimersThreshold));
              }
            else
              {
                // update the value
                (*itCqi).second.at (i) = sinr;
                // update correspondent timer
                std::map <uint16_t, uint32_t>::iterator itTimers;
                itTimers = m_ueCqiTimers.find ((*itMap).second.at (i));
                (*itTimers).second = m_cqiTimersThreshold;

              }

          }
        // remove obsolete info on allocation
        m_allocationMaps.erase (itMap);
      }
      break;
    case cv2x_UlCqi_s::SRS:
      {
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
        NS_FATAL_ERROR ("cv2x_PfFfMacScheduler supports only PUSCH and SRS UL-CQIs");
      }
      break;
    default:
      NS_FATAL_ERROR ("Unknown type of UL-CQI");
    }
  return;
}


void
cv2x_RrSlFfMacScheduler::RefreshDlCqiMaps (void)
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
cv2x_RrSlFfMacScheduler::RefreshUlCqiMaps (void)
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
cv2x_RrSlFfMacScheduler::UpdateDlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size)
{
  NS_LOG_FUNCTION (this);
  std::list<cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters>::iterator it;
  for (it = m_rlcBufferReq.begin (); it != m_rlcBufferReq.end (); it++)
    {
      if (((*it).m_rnti == rnti) && ((*it).m_logicalChannelIdentity == lcid))
        {
          NS_LOG_INFO (this << " UE " << rnti << " LC " << (uint16_t)lcid << " txqueue " << (*it).m_rlcTransmissionQueueSize << " retxqueue " << (*it).m_rlcRetransmissionQueueSize << " status " << (*it).m_rlcStatusPduSize << " decrease " << size);
          // Update queues: RLC tx order Status, ReTx, Tx
          // Update status queue
           if (((*it).m_rlcStatusPduSize > 0) && (size >= (*it).m_rlcStatusPduSize))
              {
                (*it).m_rlcStatusPduSize = 0;
              }
            else if (((*it).m_rlcRetransmissionQueueSize > 0) && (size >= (*it).m_rlcRetransmissionQueueSize))
              {
                (*it).m_rlcRetransmissionQueueSize = 0;
              }
            else if ((*it).m_rlcTransmissionQueueSize > 0)
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
                if ((*it).m_rlcTransmissionQueueSize <= size - rlcOverhead)
                  {
                    (*it).m_rlcTransmissionQueueSize = 0;
                  }
                else
                  {                    
                    (*it).m_rlcTransmissionQueueSize -= size - rlcOverhead;
                  }
              }
          return;
        }
    }
}

void
cv2x_RrSlFfMacScheduler::UpdateUlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size)
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
cv2x_RrSlFfMacScheduler::TransmissionModeConfigurationUpdate (uint16_t rnti, uint8_t txMode)
{
  NS_LOG_FUNCTION (this << " RNTI " << rnti << " txMode " << (uint16_t)txMode);
  cv2x_FfMacCschedSapUser::CschedUeConfigUpdateIndParameters params;
  params.m_rnti = rnti;
  params.m_transmissionMode = txMode;
  m_cschedSapUser->CschedUeConfigUpdateInd (params);
}



}

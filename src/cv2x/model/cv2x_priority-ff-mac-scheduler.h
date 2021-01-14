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

#ifndef CV2X_PRIORITY_FF_MAC_SCHEDULER_H
#define CV2X_PRIORITY_FF_MAC_SCHEDULER_H

#include <ns3/cv2x_lte-common.h>
#include <ns3/cv2x_ff-mac-csched-sap.h>
#include <ns3/cv2x_ff-mac-sched-sap.h>
#include <ns3/cv2x_ff-mac-scheduler.h>
#include <vector>
#include <map>
#include <ns3/nstime.h>
#include <ns3/cv2x_lte-amc.h>
#include <ns3/cv2x_lte-ffr-sap.h>

// value for SINR outside the range defined by FF-API, used to indicate that there
// is no CQI for this element
#define NO_SINR -5000


#define HARQ_PROC_NUM 8
#define HARQ_DL_TIMEOUT 11

namespace ns3 {


typedef std::vector < uint8_t > DlHarqProcessesStatus_t;
typedef std::vector < uint8_t > DlHarqProcessesTimer_t;
typedef std::vector < cv2x_DlDciListElement_s > DlHarqProcessesDciBuffer_t;
typedef std::vector < std::vector <struct cv2x_RlcPduListElement_s> > RlcPduList_t; // vector of the LCs and layers per UE
typedef std::vector < RlcPduList_t > DlHarqRlcPduListBuffer_t; // vector of the 8 HARQ processes per UE

typedef std::vector < cv2x_UlDciListElement_s > UlHarqProcessesDciBuffer_t;
typedef std::vector < uint8_t > UlHarqProcessesStatus_t;


struct cv2x_PrioritysFlowPerf_t
{
  Time flowStart;
  unsigned long totalBytesTransmitted;     /// Total bytes send by eNb for this UE
  unsigned int lastTtiBytesTransmitted;    /// Total bytes send by eNB in last tti for this UE
  double lastAveragedThroughput;           /// Past average throughput
  double secondLastAveragedThroughput;
  double targetThroughput;                 /// Target throughput

};


/**
 * \ingroup ff-api
 * \brief Implements the SCHED SAP and CSCHED SAP for a Proportional Fair scheduler
 *
 * This class implements the interface defined by the cv2x_FfMacScheduler abstract class
 */

class cv2x_PriorityFfMacScheduler : public cv2x_FfMacScheduler
{
public:
  /**
   * \brief Constructor
   *
   * Creates the MAC Scheduler interface implementation
   */
  cv2x_PriorityFfMacScheduler ();

  /**
   * Destructor
   */
  virtual ~cv2x_PriorityFfMacScheduler ();

  // inherited from Object
  virtual void DoDispose (void);
  static TypeId GetTypeId (void);

  // inherited from cv2x_FfMacScheduler
  virtual void SetFfMacCschedSapUser (cv2x_FfMacCschedSapUser* s);
  virtual void SetFfMacSchedSapUser (cv2x_FfMacSchedSapUser* s);
  virtual cv2x_FfMacCschedSapProvider* GetFfMacCschedSapProvider ();
  virtual cv2x_FfMacSchedSapProvider* GetFfMacSchedSapProvider ();

  // FFR SAPs
  virtual void SetLteFfrSapProvider (cv2x_LteFfrSapProvider* s);
  virtual cv2x_LteFfrSapUser* GetLteFfrSapUser ();

  friend class cv2x_PrioritySchedulerMemberCschedSapProvider;
  friend class cv2x_PrioritySchedulerMemberSchedSapProvider;

  void TransmissionModeConfigurationUpdate (uint16_t rnti, uint8_t txMode);

private:
  //
  // Implementation of the CSCHED API primitives
  // (See 4.1 for description of the primitives)
  //

  void DoCschedCellConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedCellConfigReqParameters& params);

  void DoCschedUeConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedUeConfigReqParameters& params);

  void DoCschedLcConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedLcConfigReqParameters& params);

  void DoCschedLcReleaseReq (const struct cv2x_FfMacCschedSapProvider::CschedLcReleaseReqParameters& params);

  void DoCschedUeReleaseReq (const struct cv2x_FfMacCschedSapProvider::CschedUeReleaseReqParameters& params);

  //
  // Implementation of the SCHED API primitives
  // (See 4.2 for description of the primitives)
  //

  void DoSchedDlRlcBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters& params);

  void DoSchedDlPagingBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlPagingBufferReqParameters& params);

  void DoSchedDlMacBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlMacBufferReqParameters& params);

  void DoSchedDlTriggerReq (const struct cv2x_FfMacSchedSapProvider::SchedDlTriggerReqParameters& params);

  void DoSchedDlRachInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedDlRachInfoReqParameters& params);

  void DoSchedDlCqiInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params);

  void DoSchedUlTriggerReq (const struct cv2x_FfMacSchedSapProvider::SchedUlTriggerReqParameters& params);

  void DoSchedUlNoiseInterferenceReq (const struct cv2x_FfMacSchedSapProvider::SchedUlNoiseInterferenceReqParameters& params);

  void DoSchedUlSrInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlSrInfoReqParameters& params);

  void DoSchedUlMacCtrlInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params);

  void DoSchedUlCqiInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params);


  int GetRbgSize (int dlbandwidth);

  int LcActivePerFlow (uint16_t rnti);

  double EstimateUlSinr (uint16_t rnti, uint16_t rb);

  void RefreshDlCqiMaps (void);
  void RefreshUlCqiMaps (void);

  void UpdateDlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size);
  void UpdateUlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size);        // modified to include the lcid 

  /**
  * \brief Update and return a new process Id for the RNTI specified
  *
  * \param rnti the RNTI of the UE to be updated
  * \return the process id  value
  */
  uint8_t UpdateHarqProcessId (uint16_t rnti);

  /**
  * \brief Return the availability of free process for the RNTI specified
  *
  * \param rnti the RNTI of the UE to be updated
  * \return the process id  value
  */
  uint8_t HarqProcessAvailability (uint16_t rnti);

  /**
  * \brief Refresh HARQ processes according to the timers
  *
  */
  void RefreshHarqProcesses ();

  Ptr<cv2x_LteAmc> m_amc;

  /*
   * Vectors of UE's LC info
  */
  std::map <cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters> m_rlcBufferReq;


  /*
  * Map of UE statistics (per RNTI basis) in downlink
  */
  std::map <uint16_t, cv2x_PrioritysFlowPerf_t> m_flowStatsDl;

  /*
  * Map of UE statistics (per RNTI basis)
  */
  std::map <uint16_t, cv2x_PrioritysFlowPerf_t> m_flowStatsUl;


  /*
  * Map of UE statistics (per RNTI basis)
  */

  std::map <cv2x_LteFlowId_t,struct cv2x_LogicalChannelConfigListElement_s> m_ueLogicalChannelsConfigList;


  /*
  * Map of UE's DL CQI P01 received
  */
  std::map <uint16_t,uint8_t> m_p10CqiRxed;
  /*
  * Map of UE's timers on DL CQI P01 received
  */
  std::map <uint16_t,uint32_t> m_p10CqiTimers;

  /*
  * Map of UE's DL CQI A30 received
  */
  std::map <uint16_t,cv2x_SbMeasResult_s> m_a30CqiRxed;
  /*
  * Map of UE's timers on DL CQI A30 received
  */
  std::map <uint16_t,uint32_t> m_a30CqiTimers;

  /*
  * Map of previous allocated UE per RBG
  * (used to retrieve info from UL-CQI)
  */
 std::map <uint16_t, std::multimap <uint16_t, std::map <uint8_t, std::vector <uint16_t> > > > m_allocationMaps;  //new structure

  /*
  * Map of UEs' UL-CQI per RBG
  */
  std::map <uint16_t, std::vector <double> > m_ueCqi;
  /*
  * Map of UEs' timers on UL-CQI per RBG
  */
  std::map <uint16_t, uint32_t> m_ueCqiTimers;

  /*
  * Map of UE's buffer status reports received
  */
  std::multimap <uint16_t, std::map <uint8_t, uint32_t> > m_ceBsrRxed; //new structure for m_ceBsrRxed in order to handle LCs

  // MAC SAPs
  cv2x_FfMacCschedSapUser* m_cschedSapUser;
  cv2x_FfMacSchedSapUser* m_schedSapUser;
  cv2x_FfMacCschedSapProvider* m_cschedSapProvider;
  cv2x_FfMacSchedSapProvider* m_schedSapProvider;

  // FFR SAPs
  cv2x_LteFfrSapUser* m_ffrSapUser;
  cv2x_LteFfrSapProvider* m_ffrSapProvider;

  // Internal parameters
  cv2x_FfMacCschedSapProvider::CschedCellConfigReqParameters m_cschedCellConfig;


  double m_timeWindow;

  uint16_t m_nextRntiUl; // RNTI of the next user to be served next scheduling in UL

  uint32_t m_cqiTimersThreshold; // # of TTIs for which a CQI canbe considered valid

  std::map <uint16_t,uint8_t> m_uesTxMode; // txMode of the UEs

  // HARQ attributes
  /**
  * m_harqOn when false inhibit te HARQ mechanisms (by default active)
  */
  bool m_harqOn;
  std::map <uint16_t, uint8_t> m_dlHarqCurrentProcessId;
  //HARQ status
  // 0: process Id available
  // x>0: process Id equal to `x` trasmission count
  std::map <uint16_t, DlHarqProcessesStatus_t> m_dlHarqProcessesStatus;
  std::map <uint16_t, DlHarqProcessesTimer_t> m_dlHarqProcessesTimer;
  std::map <uint16_t, DlHarqProcessesDciBuffer_t> m_dlHarqProcessesDciBuffer;
  std::map <uint16_t, DlHarqRlcPduListBuffer_t> m_dlHarqProcessesRlcPduListBuffer;
  std::vector <cv2x_DlInfoListElement_s> m_dlInfoListBuffered; // HARQ retx buffered

  std::map <uint16_t, uint8_t> m_ulHarqCurrentProcessId;
  //HARQ status
  // 0: process Id available
  // x>0: process Id equal to `x` trasmission count
  std::map <uint16_t, UlHarqProcessesStatus_t> m_ulHarqProcessesStatus;
  std::map <uint16_t, UlHarqProcessesDciBuffer_t> m_ulHarqProcessesDciBuffer;


  // RACH attributes
  std::vector <struct cv2x_RachListElement_s> m_rachList;
  std::vector <uint16_t> m_rachAllocationMap;
  uint8_t m_ulGrantMcs; // MCS for UL grant (default 0)

};

} // namespace ns3

#endif /* _CV2X_PRIORITY_FF_MAC_SCHEDULER_H */

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
 * Modification: Dizhi Zhou <dizhi.zhou@gmail.com>    // modify codes related to downlink scheduler
 */

#ifndef CV2X_FDMT_FF_MAC_SCHEDULER_H
#define CV2X_FDMT_FF_MAC_SCHEDULER_H

#include <ns3/cv2x_lte-common.h>
#include <ns3/cv2x_ff-mac-csched-sap.h>
#include <ns3/cv2x_ff-mac-sched-sap.h>
#include <ns3/cv2x_ff-mac-scheduler.h>
#include <vector>
#include <map>
#include <set>
#include <ns3/nstime.h>
#include <ns3/cv2x_lte-amc.h>
#include <ns3/cv2x_lte-ffr-sap.h>

/**
 * value for SINR outside the range defined by FF-API, used to indicate that there
 * is no CQI for this element
 */
#define NO_SINR -5000


/// number of HARQ processes
#define HARQ_PROC_NUM 8
/// HARQ DL timeout
#define HARQ_DL_TIMEOUT 11

namespace ns3 {


typedef std::vector < uint8_t > DlHarqProcessesStatus_t;
typedef std::vector < uint8_t > DlHarqProcessesTimer_t;
typedef std::vector < cv2x_DlDciListElement_s > DlHarqProcessesDciBuffer_t;
typedef std::vector < std::vector <struct cv2x_RlcPduListElement_s> > RlcPduList_t; ///< vector of the LCs and layers per UE
typedef std::vector < RlcPduList_t > DlHarqRlcPduListBuffer_t; ///< vector of the 8 HARQ processes per UE

typedef std::vector < cv2x_UlDciListElement_s > UlHarqProcessesDciBuffer_t;
typedef std::vector < uint8_t > UlHarqProcessesStatus_t;


/**
 * \ingroup ff-api
 * \brief Implements the SCHED SAP and CSCHED SAP for a Frequency Domain Maximize Throughput scheduler
 *
 * This class implements the interface defined by the cv2x_FfMacScheduler abstract class
 */

class cv2x_FdMtFfMacScheduler : public cv2x_FfMacScheduler
{
public:
  /**
   * \brief Constructor
   *
   * Creates the MAC Scheduler interface implementation
   */
  cv2x_FdMtFfMacScheduler ();

  /**
   * Destructor
   */
  virtual ~cv2x_FdMtFfMacScheduler ();

  // inherited from Object
  virtual void DoDispose (void);
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  // inherited from cv2x_FfMacScheduler
  virtual void SetFfMacCschedSapUser (cv2x_FfMacCschedSapUser* s);
  virtual void SetFfMacSchedSapUser (cv2x_FfMacSchedSapUser* s);
  virtual cv2x_FfMacCschedSapProvider* GetFfMacCschedSapProvider ();
  virtual cv2x_FfMacSchedSapProvider* GetFfMacSchedSapProvider ();

  // FFR SAPs
  virtual void SetLteFfrSapProvider (cv2x_LteFfrSapProvider* s);
  virtual cv2x_LteFfrSapUser* GetLteFfrSapUser ();

  /// allow cv2x_MemberCschedSapProvider<cv2x_FdMtFfMacScheduler> class friend access
  friend class cv2x_MemberCschedSapProvider<cv2x_FdMtFfMacScheduler>;
  /// allow cv2x_MemberSchedSapProvider<cv2x_FdMtFfMacScheduler> clss friend access
  friend class cv2x_MemberSchedSapProvider<cv2x_FdMtFfMacScheduler>;

  /**
   * Transmission mode configuration update
   * \param rnti the RNTI
   * \param txMode the transmit mode
   */
  void TransmissionModeConfigurationUpdate (uint16_t rnti, uint8_t txMode);

private:
  //
  // Implementation of the CSCHED API primitives
  // (See 4.1 for description of the primitives)
  //

  /**
   * Csched cell config request function
   * \param params the CSched cell config request parameters
   */
  void DoCschedCellConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedCellConfigReqParameters& params);

  /**
   * CSched UE config request function
   * \param params the CSChed UE config request parameters
   */
  void DoCschedUeConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedUeConfigReqParameters& params);

  /**
   * CSched LC config request function
   * \param params the CSChed LC config request parameters
   */
  void DoCschedLcConfigReq (const struct cv2x_FfMacCschedSapProvider::CschedLcConfigReqParameters& params);

  /**
   * CSched LC release request function
   * \param params the CSched LC release request parameters
   */
  void DoCschedLcReleaseReq (const struct cv2x_FfMacCschedSapProvider::CschedLcReleaseReqParameters& params);

  /**
   * CSched UE release request function
   * \param params th CSched UE release request parameters
   */
  void DoCschedUeReleaseReq (const struct cv2x_FfMacCschedSapProvider::CschedUeReleaseReqParameters& params);

  //
  // Implementation of the SCHED API primitives
  // (See 4.2 for description of the primitives)
  //

  /**
   * Sched DL RLC buffer request function
   * \param params the Sched DL RLC buffer request parameters
   */
  void DoSchedDlRlcBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters& params);

  /**
   * Sched DL paging buffer request function
   * \param params the Sched DL paging buffer request paarameters
   */
  void DoSchedDlPagingBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlPagingBufferReqParameters& params);

  /**
   * Sched DL MAC buffer request function
   * \param params the Sched DL MAC buffer request parameters
   */
  void DoSchedDlMacBufferReq (const struct cv2x_FfMacSchedSapProvider::SchedDlMacBufferReqParameters& params);

  /**
   * Sched DL trigger request function
   *
   * \param params struct cv2x_FfMacSchedSapProvider::SchedDlTriggerReqParameters&
   */
  void DoSchedDlTriggerReq (const struct cv2x_FfMacSchedSapProvider::SchedDlTriggerReqParameters& params);

  /**
   * Sched DL RACH info request function
   * \param params the Sched DL RACH info request parameters
   */
  void DoSchedDlRachInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedDlRachInfoReqParameters& params);

  /**
   * Sched DL CQI info request function
   * \param params the Sched DL CQI info request parameters
   */
  void DoSchedDlCqiInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params);

  /**
   * Sched UL trigger request function
   * \param params the Sched UL trigger request parameters
   */
  void DoSchedUlTriggerReq (const struct cv2x_FfMacSchedSapProvider::SchedUlTriggerReqParameters& params);

  /**
   * Sched UL noise interference request function
   * \param params the Sched UL noise interference request parameters
   */ 
  void DoSchedUlNoiseInterferenceReq (const struct cv2x_FfMacSchedSapProvider::SchedUlNoiseInterferenceReqParameters& params);

  /**
   * Sched UL SR info request function
   * \param params the Sched UL SR info request parameters
   */
  void DoSchedUlSrInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlSrInfoReqParameters& params);

  /**
   * Sched UL MAC control info request function
   * \param params the Sched UL MAC control info request parameters
   */
  void DoSchedUlMacCtrlInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params);

  /**
   * Sched UL CQI info request function
   * \param params the Sched UL CQI info request parameters
   */
  void DoSchedUlCqiInfoReq (const struct cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params);


  /**
   * Get RBG size function
   * \param dlbandwidth the DL bandwidth
   * \returns the RBG size
   */
  int GetRbgSize (int dlbandwidth);

  /**
   * LC Active per flow function
   * \param rnti the RNTI
   * \returns the LC active per flow
   */
  unsigned int LcActivePerFlow (uint16_t rnti);

  /**
   * Estimate UL SNR function
   * \param rnti the RNTI
   * \param rb the RB
   * \returns the UL SINR
   */
  double EstimateUlSinr (uint16_t rnti, uint16_t rb);

  /// Refresh DL CGI maps function
  void RefreshDlCqiMaps (void);
  /// Refresh UL CGI maps function
  void RefreshUlCqiMaps (void);

  /**
   * Update DL RLC buffer info function
   * \param rnti the RNTI
   * \param lcid the LCID
   * \param size the size
   */
  void UpdateDlRlcBufferInfo (uint16_t rnti, uint8_t lcid, uint16_t size);
  /**
   * Update UL RLC b uffer info function
   * \param rnti the RNTI
   * \param size the size
   */
  void UpdateUlRlcBufferInfo (uint16_t rnti, uint16_t size);

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

  Ptr<cv2x_LteAmc> m_amc; ///< amc

  /**
   * Vectors of UE's LC info
  */
  std::map <cv2x_LteFlowId_t, cv2x_FfMacSchedSapProvider::SchedDlRlcBufferReqParameters> m_rlcBufferReq;


  /**
  * Set of UE statistics (per RNTI basis) in downlink
  */
  std::set <uint16_t> m_flowStatsDl;

  /**
  * Set of UE statistics (per RNTI basis)
  */
  std::set <uint16_t> m_flowStatsUl;

  /**
  * Map of UE's DL CQI P01 received
  */
  std::map <uint16_t,uint8_t> m_p10CqiRxed;

  /**
  * Map of UE's timers on DL CQI P01 received
  */
  std::map <uint16_t,uint32_t> m_p10CqiTimers;

  /**
  * Map of UE's DL CQI A30 received
  */
  std::map <uint16_t,cv2x_SbMeasResult_s> m_a30CqiRxed;

  /**
  * Map of UE's timers on DL CQI A30 received
  */
  std::map <uint16_t,uint32_t> m_a30CqiTimers;

  /**
  * Map of previous allocated UE per RBG
  * (used to retrieve info from UL-CQI)
  */
  std::map <uint16_t, std::vector <uint16_t> > m_allocationMaps;

  /**
  * Map of UEs' UL-CQI per RBG
  */
  std::map <uint16_t, std::vector <double> > m_ueCqi;

  /**
  * Map of UEs' timers on UL-CQI per RBG
  */
  std::map <uint16_t, uint32_t> m_ueCqiTimers;

  /**
  * Map of UE's buffer status reports received
  */
  std::map <uint16_t,uint32_t> m_ceBsrRxed;

  // MAC SAPs
  cv2x_FfMacCschedSapUser* m_cschedSapUser; ///< csched SAP user
  cv2x_FfMacSchedSapUser* m_schedSapUser; ///< sched SAP user
  cv2x_FfMacCschedSapProvider* m_cschedSapProvider; ///< csched SAP provider
  cv2x_FfMacSchedSapProvider* m_schedSapProvider; ///< sched SAP provider

  // FFR SAPs
  cv2x_LteFfrSapUser* m_ffrSapUser; ///< FFR SAP user
  cv2x_LteFfrSapProvider* m_ffrSapProvider; ///< FFR SAP provider

  // Internal parameters
  cv2x_FfMacCschedSapProvider::CschedCellConfigReqParameters m_cschedCellConfig; ///< sched cell config


  uint16_t m_nextRntiUl; ///< RNTI of the next user to be served next scheduling in UL

  uint32_t m_cqiTimersThreshold; ///< # of TTIs for which a CQI can be considered valid

  std::map <uint16_t,uint8_t> m_uesTxMode; ///< txMode of the UEs

  // HARQ attributes
  bool m_harqOn; ///< m_harqOn when false inhibit tte HARQ mechanisms (by default active)
  std::map <uint16_t, uint8_t> m_dlHarqCurrentProcessId; ///< DL HARQ current process ID
  //HARQ status
  // 0: process Id available
  // x>0: process Id equal to `x` transmission count
  std::map <uint16_t, DlHarqProcessesStatus_t> m_dlHarqProcessesStatus; ///< DL HARQ process status
  std::map <uint16_t, DlHarqProcessesTimer_t> m_dlHarqProcessesTimer; ///< DL HARDQ process timer
  std::map <uint16_t, DlHarqProcessesDciBuffer_t> m_dlHarqProcessesDciBuffer; ///< DL HARQ process DCI buffer
  std::map <uint16_t, DlHarqRlcPduListBuffer_t> m_dlHarqProcessesRlcPduListBuffer; ///< DL HARQ process RLC PDU list buffer
  std::vector <cv2x_DlInfoListElement_s> m_dlInfoListBuffered; ///< HARQ retx buffered

  std::map <uint16_t, uint8_t> m_ulHarqCurrentProcessId; ///< UL HARQ current process ID
  //HARQ status
  // 0: process Id available
  // x>0: process Id equal to `x` transmission count
  std::map <uint16_t, UlHarqProcessesStatus_t> m_ulHarqProcessesStatus; ///< UL HARQ process status
  std::map <uint16_t, UlHarqProcessesDciBuffer_t> m_ulHarqProcessesDciBuffer; ///< UL HARQ process DCI buffer


  // RACH attributes
  std::vector <struct cv2x_RachListElement_s> m_rachList; ///< RACH list
  std::vector <uint16_t> m_rachAllocationMap; ///< RACH allocation map
  uint8_t m_ulGrantMcs; ///< MCS for UL grant (default 0)

};

} // namespace ns3

#endif /* CV2X_FDMT_FF_MAC_SCHEDULER_H */

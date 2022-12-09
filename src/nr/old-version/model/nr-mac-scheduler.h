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

#pragma once

#include <ns3/object.h>
#include "nr-mac-csched-sap.h"
#include "nr-mac-sched-sap.h"

namespace ns3 {

/**
 * \ingroup scheduler
 * \brief Interface for all the nr schedulers
 *
 * TODO: Add description of SAP user/providers
 *
 * \see NrMacSchedulerNs3
 */
class NrMacScheduler : public Object
{
public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrMacScheduler constructor
   */
  NrMacScheduler ();

  /**
   * \brief NrMacScheduler deconstructor
   */
  virtual ~NrMacScheduler ();

  /**
   * \brief Set the MacSchedSapUser pointer
   * \param sap pointer to the mac sched sap user class
   */
  void SetMacSchedSapUser (NrMacSchedSapUser* sap)
  {
    m_macSchedSapUser = sap;
  }

  /**
   * \brief Get the MacSchedSapProvider pointer
   * \return the pointer to the mac sched sap provider class
   */
  NrMacSchedSapProvider* GetMacSchedSapProvider ()
  {
    return m_macSchedSapProvider;
  }

  /**
   * \brief SetMacCschedSapUser
   * \param sap the pointer to the sap user
   */
  void SetMacCschedSapUser (NrMacCschedSapUser* sap)
  {
    m_macCschedSapUser = sap;
  }

  /**
   * \brief Get the MacCschedSapProvider pointer
   * \return the pointer to the sap provider
   */
  NrMacCschedSapProvider* GetMacCschedSapProvider ()
  {
    return m_macCschedSapProvider;
  }

  //
  // Implementation of the CSCHED API primitives
  // (See 4.1 for description of the primitives)
  //

  /**
   * \brief Configure cell.
   *
   * (Re-)configure MAC scheduler with cell configuration and scheduler
   * configuration. The cell configuration will also setup the BCH, BCCH, PCCH
   * and CCCH LC configuration (for each component carrier).
   *
   * Ns-3 does nothing.
   *
   * \param params Cell configuration
   */
  virtual void
  DoCschedCellConfigReq (const NrMacCschedSapProvider::CschedCellConfigReqParameters& params) = 0;

  /**
   * \brief Configure single UE.
   *
   * (Re-)configure MAC scheduler with single UE specific parameters.
   * A UE can only be configured when a cell configuration has been received.
   *
   * \param params UE configuration
   */
  virtual void
  DoCschedUeConfigReq (const NrMacCschedSapProvider::CschedUeConfigReqParameters& params) = 0;

  /**
   * \brief Configure UE's logical channel(s).
   *
   * (Re-)configure MAC scheduler with UE's logical channel configuration.
   * A logical channel can only be configured when a UE configuration has
   * been received.
   * \param params UE's logical channel configuration
   */
  virtual void
  DoCschedLcConfigReq (const NrMacCschedSapProvider::CschedLcConfigReqParameters& params) = 0;

  /**
   * \brief Release UE's logical channel(s).
   *
   * Release UE's logical channel(s) in the MAC scheduler. A logical channel
   * can only be released if it has been configured previously.
   *
   * \param params UE's logical channel(s) to be released
   */
  virtual void
  DoCschedLcReleaseReq (const NrMacCschedSapProvider::CschedLcReleaseReqParameters& params) = 0;

  /**
   * \brief Release UE.
   *
   * Release a UE in the MAC scheduler. The release of the UE configuration
   * implies the release of LCs, which are still active. A UE can only be
   * released if it has been configured previously.
   *
   * \param params UE to be released
   */
  virtual void
  DoCschedUeReleaseReq (const NrMacCschedSapProvider::CschedUeReleaseReqParameters& params) = 0;

  //
  // Implementation of the SCHED API primitives
  // (See 4.2 for description of the primitives)
  //

  /**
   * \brief DoSchedDlRlcBufferReq
   *
   * Update buffer status of logical channel data in RLC. The update rate with
   * which the buffer status is updated in the scheduler is outside of the scope
   * of the document.
   *
   * \param params RLC Buffer status
   */
  virtual void
  DoSchedDlRlcBufferReq (const NrMacSchedSapProvider::SchedDlRlcBufferReqParameters& params) = 0;

  /**
   * \brief Provides CQI measurement report information to the scheduler.
   * \param params DL CQI information
   */
  virtual void
  DoSchedDlCqiInfoReq (const NrMacSchedSapProvider::SchedDlCqiInfoReqParameters& params) = 0;

  /**
   * \brief Provides UL CQI measurement information to the scheduler.
   * \param params UL CQI information
   */
  virtual void
  DoSchedUlCqiInfoReq (const NrMacSchedSapProvider::SchedUlCqiInfoReqParameters& params) = 0;

  /**
   * \brief Provides mac control information (power headroom, ul buffer status) to the scheduler.
   * \param params MAC control information received
   */
  virtual void
  DoSchedUlMacCtrlInfoReq (const NrMacSchedSapProvider::SchedUlMacCtrlInfoReqParameters& params) = 0;

  /**
   * \brief Request for scheduling a slot in DL
   *
   * \param params DL HARQ information
   */
  virtual void
  DoSchedDlTriggerReq (const NrMacSchedSapProvider::SchedDlTriggerReqParameters& params) = 0;

  /**
   * \brief Request for scheduling a slot in UL
   *
   * \param params UL HARQ information
   */
  virtual void
  DoSchedUlTriggerReq (const NrMacSchedSapProvider::SchedUlTriggerReqParameters& params) = 0;

  /**
   * \brief One or more UE asked to be scheduled in UL
   * \param params SR information
   */
  virtual void
  DoSchedUlSrInfoReq (const NrMacSchedSapProvider::SchedUlSrInfoReqParameters &params) = 0;

  /**
   * \brief Forcefully set a default MCS
   * \param mcs MCS
   */
  virtual void
  DoSchedSetMcs (uint32_t mcs) = 0;

  /**
   * \brief RACH information
   *
   * \param params SchedDlRachInfoReqParameters
   */
  virtual void DoSchedDlRachInfoReq (const NrMacSchedSapProvider::SchedDlRachInfoReqParameters& params) = 0;

  /**
   * \brief Retrieve the number of DL ctrl symbols configured in the scheduler
   * \return the number of DL ctrl symbols
   */
  virtual uint8_t GetDlCtrlSyms () const = 0;

  /**
   * \brief Retrieve the number of UL ctrl symbols configured in the scheduler
   * \return the number of UL ctrl symbols
   */
  virtual uint8_t GetUlCtrlSyms () const = 0;

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  virtual int64_t AssignStreams (int64_t stream) = 0;

protected:
  NrMacSchedSapUser* m_macSchedSapUser           {nullptr};  //!< SAP user
  NrMacCschedSapUser* m_macCschedSapUser         {nullptr};  //!< SAP User
  NrMacCschedSapProvider* m_macCschedSapProvider {nullptr};  //!< SAP Provider
  NrMacSchedSapProvider* m_macSchedSapProvider   {nullptr};  //!< SAP Provider
};

} // namespace ns3

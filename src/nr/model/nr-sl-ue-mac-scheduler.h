/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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

#ifndef NR_SL_UE_MAC_SCHEDULER_H
#define NR_SL_UE_MAC_SCHEDULER_H


#include <ns3/object.h>
#include "nr-sl-ue-mac-csched-sap.h"
#include "nr-sl-ue-mac-sched-sap.h"

namespace ns3 {

/**
 * \ingroup scheduler
 * \brief Interface for all the NR Sidelink schedulers
 *
 * \see NrSlUeMacSchedulerNs3
 */
class NrSlUeMacScheduler : public Object
{
public:
  /**
   * \brief Get the type id
   * \return the type id of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrSlUeMacScheduler constructor
   */
  NrSlUeMacScheduler ();

  /**
   * \brief NrSlUeMacScheduler deconstructor
   */
  virtual ~NrSlUeMacScheduler ();

  /**
   * \brief Set the NrSlUeMacSchedSapUser pointer
   * \param sap pointer to the NR Sidelink MAC sched sap user class
   */
  void SetNrSlUeMacSchedSapUser (NrSlUeMacSchedSapUser* sap);

  /**
   * \brief Get the NrSlUeMacSchedSapProvider pointer
   * \return the pointer to the NR Sidelink MAC sched sap provider class
   */
  NrSlUeMacSchedSapProvider* GetNrSlUeMacSchedSapProvider ();

  /**
   * \brief SetNrSlUeMacCschedSapUser
   * \param sap the pointer to the NR Sidelink UE MAC sap user
   */
  void SetNrSlUeMacCschedSapUser (NrSlUeMacCschedSapUser* sap);

  /**
   * \brief Get the MacCschedSapProvider pointer
   * \return the pointer to the sap provider
   */
  NrSlUeMacCschedSapProvider* GetNrSlUeMacCschedSapProvider ();


  //
  // Implementation of the CSCHED API primitives for NR Sidelink
  //
  /**
   * \brief Send the NR Sidelink logical channel configuration from UE MAC to the UE scheduler
   *
   * \param params NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo
   */
  virtual void DoCschedUeNrSlLcConfigReq (const struct NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) = 0;


  //
  // Implementation of the SCHED API primitives for NR Sidelink
  //
  /**
   * \brief Send NR Sidelink RLC buffer status report from UE MAC to the UE scheduler
   *
   * \param params NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams
   */
  virtual void DoSchedUeNrSlRlcBufferReq (const struct NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams& params) = 0;
  /**
   * \brief Send NR Sidleink trigger request from UE MAC to the UE scheduler
   *
   * \param dstL2Id The destination layer 2 id
   * \param params NrSlUeMacSchedSapProvider::NrSlSlotInfo
   */
  virtual void DoSchedUeNrSlTriggerReq (uint32_t dstL2Id, const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& params) = 0;

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
  NrSlUeMacSchedSapUser* m_nrSlUeMacSchedSapUser           {nullptr};  //!< SAP user
  NrSlUeMacCschedSapUser* m_nrSlUeMacCschedSapUser         {nullptr};  //!< SAP User
  NrSlUeMacCschedSapProvider* m_nrSlUeMacCschedSapProvider {nullptr};  //!< SAP Provider
  NrSlUeMacSchedSapProvider* m_nrSlUeMacSchedSapProvider   {nullptr};  //!< SAP Provider
};

/**
 * \ingroup scheduler
 * \brief Class implementing the NrSlUeMacCschedSapProvider methods
 */
class NrSlUeMacGeneralCschedSapProvider : public NrSlUeMacCschedSapProvider
{
public:
  /**
   * \brief constructor
   * \param scheduler The pointer the NrSlUeMacScheduler API using this SAP
   */
  NrSlUeMacGeneralCschedSapProvider (NrSlUeMacScheduler* scheduler);

  ~NrSlUeMacGeneralCschedSapProvider () = default;

  // inherited from NrSlUeMacCschedSapProvider

  virtual void CschedUeNrSlLcConfigReq (const struct NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) override;

private:
  NrSlUeMacScheduler* m_scheduler {nullptr}; //!< pointer to the scheduler API using this SAP
};

/**
 * \ingroup scheduler
 * \brief Class implementing the NrSlUeMacSchedSapProvider methods
 */
class NrSlUeMacGeneralSchedSapProvider : public NrSlUeMacSchedSapProvider
{
public:
  /**
   * \brief constructor
   * \param sched The pointer the NrSlUeMacScheduler API using this SAP
   */
  NrSlUeMacGeneralSchedSapProvider (NrSlUeMacScheduler* sched);

  virtual void SchedUeNrSlRlcBufferReq (const struct NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams& params) override;
  virtual void SchedUeNrSlTriggerReq (uint32_t dstL2Id, const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& params) override;

private:
  NrSlUeMacScheduler* m_scheduler {nullptr}; //!< pointer to the scheduler API using this SAP
};


} // namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_H */

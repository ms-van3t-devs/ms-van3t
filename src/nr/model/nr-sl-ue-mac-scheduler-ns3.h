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
#ifndef NR_SL_UE_MAC_SCHEDULER_NS3_H
#define NR_SL_UE_MAC_SCHEDULER_NS3_H


#include "nr-sl-ue-mac-scheduler.h"
#include "nr-sl-ue-mac-scheduler-dst-info.h"
#include "nr-amc.h"
#include "nr-sl-phy-mac-common.h"
#include <ns3/random-variable-stream.h>
#include <memory>
#include <functional>
#include <list>

namespace ns3 {

/**
 * \ingroup scheduler
 *
 * \brief A general scheduler for NR SL UE in NS3
 */
class NrSlUeMacSchedulerNs3 : public NrSlUeMacScheduler
{
public:
  /**
   * \brief GetTypeId
   *
   * \return The TypeId of the class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief NrSlUeMacSchedulerNs3 default constructor
   */
  NrSlUeMacSchedulerNs3 ();

  /**
   * \brief NrSlUeMacSchedulerNs3 destructor
   */
  virtual ~NrSlUeMacSchedulerNs3 ();

  /**
   * \brief Send the NR Sidelink logical channel configuration from UE MAC to the UE scheduler
   *
   * This method is also responsible to create the destination info.
   *
   * \see CreateDstInfo
   * \see CreateLCG
   * \see CreateLc
   *
   * \param params The NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo struct
   */
  virtual void DoCschedUeNrSlLcConfigReq (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) override;

  /**
   * \brief UE RLC informs the scheduler of NR SL data
   *
   * The message contains the LC and the amount of data buffered. Therefore,
   * in this method we cycle through all the destinations LCG to find the LC, and once
   * it is found, it is updated with the new amount of data.
   *
   * \param params The buffer status report from UE RLC

   */
  virtual void DoSchedUeNrSlRlcBufferReq (const struct NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams& params) override;
  /**
   * \brief Send NR Sidleink trigger request from UE MAC to the UE scheduler
   *
   * \param dstL2Id The destination layer 2 id
   * \param params The list of NrSlUeMacSchedSapProvider::NrSlSlotInfo
   */
  virtual void DoSchedUeNrSlTriggerReq (uint32_t dstL2Id, const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& params) override;


  /**
   * \brief Install the AMC for the NR Sidelink
   *
   * Usually called by the helper
   *
   * \param nrSlAmc NR Sidelink AMC
   */
  void InstallNrSlAmc (const Ptr<NrAmc> &nrSlAmc);

  /**
   * \brief Get the AMC for NR Sidelink
   *
   * \return the NR Sidelink AMC
   */
  Ptr<const NrAmc> GetNrSlAmc () const;

  /**
   * \brief Set the flag if the MCS for NR SL is fixed (in this case,
   *        it will take the initial value)
   *
   * \param fixMcs the flag to indicate if the NR SL MCS is fixed
   *
   * \see SetInitialMcsSl
   */
  void UseFixedNrSlMcs (bool fixMcs);
  /**
   * \brief Check if the MCS in NR SL is fixed
   * \return true if the NR SL MCS is fixed, false otherwise
   */
  bool IsNrSlMcsFixed () const;

  /**
   * \brief Set the initial value for the NR SL MCS
   *
   * \param mcs the MCS value
   */
  void SetInitialNrSlMcs (uint8_t mcs);

  /**
   * \brief Get the SL MCS initial value
   *
   * \return the value
   */
  uint8_t GetInitialNrSlMcs () const;

  /**
   * \brief Get Redundancy Version number
   *
   * We assume rvid = 0, so RV would take 0, 2, 3, 1. See TS 38.21 table 6.1.2.1-2
   *
   * \param txNumTb The transmission index of the TB, e.g., 0 for initial tx,
   *        1 for a first retransmission, and so on.
   * \return The Redundancy Version number
   */
  uint8_t GetRv (uint8_t txNumTb) const;

  /**
   * \brief Assign a fixed random variable stream number to the random variables
   * used by this model. Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream The first stream index to use
   * \return The number of stream indices assigned by this model
   */
  virtual int64_t AssignStreams (int64_t stream) override;


protected:
  /**
   * \brief Do the NE Sidelink allocation
   *
   * All the child classes should implement this method
   *
   * For allocating resources to more than one LCs of a
   * destination so they can be multiplexed, one could consider
   * the following procedure.
   *
   * 1. Irrespective of the priority of LCc, sum their buffer size.
   * 2. Compute the TB size using the AMC given the available resources, the
   *    buffer size computed in step 1, and the MCS.
   * 3. Starting from the highest priority LC, distribute the bytes among LCs
   *    from the TB size computed in step 2 as per their buffer status report
   *    until we satisfy all the LCs or the TB size computed in step 2 is fully
   *    consumed. There may be more than one LCs with the same priority, which
   *    could have same or different buffer sizes. In case of equal buffer sizes,
   *    these LCs should be assigned equal number of bytes. If these LCs have
   *    unequal buffer sizes, we can use the minimum buffer size among the LCs
   *    to assign the same bytes.
   *
   * \param params The list of the txOpps from the UE MAC
   * \param dstInfo The pointer to the NrSlUeMacSchedulerDstInfo of the destination
   *        for which UE MAC asked the scheduler to allocate the recourses
   * \param slotAllocList The slot allocation list to be updated by a specific scheduler
   * \return The status of the allocation, true if the destination has been
   *         allocated some resources; false otherwise.
   */
  virtual bool
  DoNrSlAllocation (const std::list <NrSlUeMacSchedSapProvider::NrSlSlotInfo>& params,
                    const std::shared_ptr<NrSlUeMacSchedulerDstInfo> &dstInfo,
                    std::set<NrSlSlotAlloc> &slotAllocList) = 0;
  /**
   * \brief Method to get total number of sub-channels.
   *
   * \return the total number of sub-channels.
   */
  uint8_t GetTotalSubCh () const;

  /**
   * \brief Method to get the maximum transmission number
   *        (including new transmission and retransmission) for PSSCH.
   *
   * \return The max number of PSSCH transmissions
   */
  uint8_t GetSlMaxTxTransNumPssch () const;

  Ptr<UniformRandomVariable> m_uniformVariable; //!< Uniform random variable

private:
  /**
   * \brief Create destination info
   *
   * If the scheduler does not have the destination info then it creates it,
   * and then save its pointer in the m_dstMap map.
   *
   * If the scheduler already have the destination info, it does noting. This
   * could happen when we are trying add more than one logical channels
   * for a destination.
   *
   * \param params params of the UE
   * \return A std::shared_ptr to newly created NrSlUeMacSchedulerDstInfo
   */
  std::shared_ptr<NrSlUeMacSchedulerDstInfo>
  CreateDstInfo (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params);

  /**
   * \brief Create a NR Sidelink logical channel group
   *
   * A subclass can return its own representation of a logical channel by
   * implementing a proper subclass of NrSlUeMacSchedulerLCG and returning a
   * pointer to a newly created instance.
   *
   * \param lcGroup The logical channel group id
   * \return a pointer to the representation of a logical channel group
   */
  NrSlLCGPtr CreateLCG (uint8_t lcGroup) const;

  /**
   * \brief Create a NR Sidelink logical channel
   *
   * A subclass can return its own representation of a logical channel by
   * implementing a proper subclass of NrSlUeMacSchedulerLC and returning a
   * pointer to a newly created instance.
   *
   * \param params configuration of the logical channel
   * \return a pointer to the representation of a logical channel
   */

  NrSlLCPtr CreateLC (const NrSlUeMacCschedSapProvider::SidelinkLogicalChannelInfo& params) const;

  std::unordered_map<uint32_t, std::shared_ptr<NrSlUeMacSchedulerDstInfo> > m_dstMap; //!< The map of between destination layer 2 id and the destination info

  Ptr<NrAmc> m_nrSlAmc;           //!< AMC pointer for NR SL

  bool    m_fixedNrSlMcs {false}; //!< Fixed MCS for *all* the destinations

  uint8_t m_initialNrSlMcs   {0}; //!< Initial (or fixed) value for NR SL MCS

};

} //namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_NS3_H */

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

#include <memory>
#include <ns3/ff-mac-common.h>
#include <ns3/nstime.h>
#include "nr-mac-sched-sap.h"

namespace ns3 {

/**
 * \ingroup scheduler
 * \brief Represent a DL Logical Channel of an UE
 *
 * The scheduler stores here the information that comes from BSR, arriving
 * from the gNB.
 *
 * Please use the unique ptr defined by the typedef LCPtr.
 *
 * \see Update
 * \see GetTotalSize
 */
class NrMacSchedulerLC
{
public:
  /**
   * \brief NrMacSchedulerLC constructor
   * \param conf Configuration of the LC
   */
  NrMacSchedulerLC (const LogicalChannelConfigListElement_s &conf);
  /**
   * \brief NrMacSchedulerLC default constructor (deletec)
   */
  NrMacSchedulerLC () = delete;
  /**
   * \brief NrMacSchedulerLC copy constructor (deleted)
   * \param o other instance
   */
  NrMacSchedulerLC (const NrMacSchedulerLC &o) = delete;

  /**
   * \brief Overwrite all the parameters with the one contained in the message
   * \param params the message received from the RLC layer, containing the information about the queues
   *
   * \return Number of bytes added or removed from the LC
   */
  int
  Update (const NrMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
  {
    NS_ASSERT (params.m_logicalChannelIdentity == m_id);

    int ret = 0;

    if (params.m_rlcTransmissionQueueSize > m_rlcTransmissionQueueSize)
      {
        ret += params.m_rlcTransmissionQueueSize - m_rlcTransmissionQueueSize;
      }
    if (params.m_rlcRetransmissionQueueSize > m_rlcRetransmissionQueueSize)
      {
        ret += params.m_rlcRetransmissionQueueSize - m_rlcRetransmissionQueueSize;
      }
    if (params.m_rlcStatusPduSize - m_rlcStatusPduSize)
      {
        ret += params.m_rlcStatusPduSize - m_rlcStatusPduSize;
      }

    m_rlcTransmissionQueueSize = params.m_rlcTransmissionQueueSize;
    m_rlcTransmissionQueueHolDelay = params.m_rlcTransmissionQueueHolDelay;
    m_rlcRetransmissionQueueSize = params.m_rlcRetransmissionQueueSize;
    m_rlcRetransmissionHolDelay = params.m_rlcRetransmissionHolDelay;
    m_rlcStatusPduSize = params.m_rlcStatusPduSize;

    return ret;
  }

  /**
   * \brief Forcefully update m_rlcTransmissionQueueSize
   * \param size Num. of bytes
   */
  void OverwriteTxQueueSize (uint32_t size)
  {
    m_rlcTransmissionQueueSize = size;
  }

  /**
   * \brief Get the total size of the LC
   * \return the total size of the LC
   */
  uint32_t
  GetTotalSize () const
  {
    return m_rlcTransmissionQueueSize + m_rlcRetransmissionQueueSize + m_rlcStatusPduSize;
  }

  uint32_t m_id                           {0}; //!< ID of the LC
  uint32_t m_rlcTransmissionQueueSize     {0}; //!< The current size of the new transmission queue in byte.
  uint16_t m_rlcTransmissionQueueHolDelay {0}; //!< Head of line delay of new transmissions in ms.
  uint16_t m_rlcRetransmissionHolDelay    {0}; //!< Head of line delay of retransmissions in ms.
  uint32_t m_rlcRetransmissionQueueSize   {0}; //!< The current size of the retransmission queue in byte.
  uint16_t m_rlcStatusPduSize             {0}; //!< The current size of the pending STATUS message in byte.

  Time m_delayBudget    {Time::Min ()}; //!< Delay budget of the flow
  double m_PER          {0.0};         //!< PER of the flow
  bool m_isGbr          {false};       //!< Is GBR?
};

/**
 * \brief Unique pointer to an instance of NrMacSchedulerLC
 * \ingroup scheduler
 */
typedef std::unique_ptr<NrMacSchedulerLC> LCPtr;

/**
 * \ingroup scheduler
 * \brief Represent an UE LCG (can be DL or UL)
 *
 * A Logical Channel Group has an id (represented by m_id) and can contain
 * logical channels. The LC are stored inside an unordered map, indexed
 * by their ID.
 *
 * The LCs are inserted through the method Insert, and they can be updated with
 * a call to UpdateInfo. The update is different in DL and UL: in UL only the
 * sum of all components is available, while for DL there is a complete picture,
 * thanks to all the variables defined in
 * NrMacSchedSapProvider::SchedDlRlcBufferReqParameters.
 *
 * The general usage of this class is to insert each LC, and then update the
 * amount of bytes stored. The removal of an LC is still missing.
 *
 * For what regards UL, we currently support only one LC per LCG. This comes
 * from the fact that the BSR is reported for all the LCG, and the scheduler
 * has no way to identify which LCID contains bytes. So, even at the cost to
 * have a mis-representation between the ID inside the UEs and the ID inside
 * the scheduler, we should make sure that each LCG in UL has only one LC.
 *
 * \see UpdateInfo
 */
class NrMacSchedulerLCG
{
public:
  /**
   * \brief NrMacSchedulerLCG constructor
   * \param id The id of the LCG
   */
  NrMacSchedulerLCG (uint8_t id) : m_id (id)
  {
    (void) m_id;
  }
  /**
   * \brief NrMacSchedulerLCG copy constructor (deleted)
   * \param other other instance
   */
  NrMacSchedulerLCG (const NrMacSchedulerLCG &other) = delete;

  /**
   * \brief Check if the LCG contains the LC id specified
   * \param lcId LC ID to check for
   * \return true if the LCG contains the LC
   */
  bool
  Contains (uint8_t lcId) const
  {
    return m_lcMap.find (lcId) != m_lcMap.end ();
  }

  /**
   * \brief Get the number of LC currently in the LCG
   * \return the number of LC
   */
  uint32_t
  NumOfLC () const
  {
    return static_cast<uint32_t> (m_lcMap.size ());
  }

  /**
   * \brief Insert LC in the group
   * \param lc LC to insert
   * \return true if the insertion was fine (false in the case the LC already exists)
   */
  bool
  Insert (LCPtr && lc)
  {
    NS_ASSERT (!Contains (lc->m_id));
    return m_lcMap.emplace (std::make_pair (lc->m_id, std::move (lc))).second;
  }

  /**
   * \brief Update the LCG with a message coming from RLC in the gNB.
   * \param params message from gNB RLC layer.
   *
   * The method is able to update the LC using all the information such as
   * Retx queue, Tx queue, and the various delays.
   *
   * A call to NrMacSchedulerLC::Update is performed.
   */
  void
  UpdateInfo (const NrMacSchedSapProvider::SchedDlRlcBufferReqParameters& params)
  {
    NS_ASSERT (Contains (params.m_logicalChannelIdentity));
    int ret = m_lcMap.at (params.m_logicalChannelIdentity)->Update (params);
    if (ret < 0)
      {
        NS_ASSERT_MSG (m_totalSize >= static_cast<uint32_t> (std::abs (ret)),
                       "totSize: " << m_totalSize << " ret: " << ret);
      }
    // Update m_totalSize to include the sizes of the buffers
    m_totalSize += ret;
  }

  /**
   * Checks if m_totalSize is in correct state, and if not resets it.
   * If m_totalSize is higher than it should at this point, i.e.,
   * some overheads were not taken into account when it was being updated.
   */
  void
  SanityCheck ()
  {
    //sanity check of m_totalSize
    uint32_t size = 0;
    for (const auto & lc : m_lcMap)
      {
        size += lc. second->m_rlcStatusPduSize + lc.second -> m_rlcRetransmissionQueueSize + lc.second ->m_rlcTransmissionQueueSize;
      }

    if (size == 0)
      {
        m_totalSize = 0;
      }
  }


  /**
   * \brief Update the LCG with just the LCG occupancy. Used in UL case when a BSR is received.
   * \param lcgQueueSize Sum of the size of all components in B
   *
   * Used in the UL case, in which only the sum of the components are
   * available. For the LC, only the value m_rlcTransmissionQueueSize is updated.
   *
   * For UL, only 1 LC per LCG is supported.
   *
   * \see NrMacSchedulerLC::OverwriteTxQueueSize()
   */
  void
  UpdateInfo (uint32_t lcgQueueSize)
  {
    NS_ABORT_IF (m_lcMap.size () > 1);
    uint32_t lcIdPart = lcgQueueSize / m_lcMap.size ();
    for (auto & lc : m_lcMap)
      {
        lc.second->OverwriteTxQueueSize (lcIdPart);
      }
    m_totalSize = lcgQueueSize;
  }

  /**
   * \brief Get the total size of the LCG
   * \return the total size of the LCG
   */
  uint32_t
  GetTotalSize () const
  {
    return m_totalSize;
  }

  /**
   * \brief Get TotalSize Of LC
   * \param lcId LC ID
   * \return the total size of the LC
   */
  uint32_t
  GetTotalSizeOfLC (uint8_t lcId) const
  {
    NS_ABORT_IF (m_lcMap.size () == 0);
    return m_lcMap.at (lcId)->GetTotalSize ();
  }

  /**
   * \brief Get a vector of LC ID
   * \return a vector with all the LC id present in this LCG
   */
  std::vector<uint8_t>
  GetLCId () const
  {
    std::vector<uint8_t> ret;
    for (const auto & lc : m_lcMap)
      {
        ret.emplace_back (lc.first);
      }
    return ret;
  }

  /**
   * \brief Inform the LCG of the assigned data to a LC id
   * \param lcId the LC id to which the data was assigned
   * \param size amount of assigned data
   * \param type String representing the type of allocation currently in act (DL or UL)
   */
  void AssignedData (uint8_t lcId, uint32_t size, std::string type);

private:
  uint32_t m_totalSize {0};                  //!< Total size
  uint8_t m_id {0};                          //!< ID of the LCG
  std::unordered_map<uint8_t, LCPtr> m_lcMap; //!< Map between LC id and their pointer
};

/**
 * \brief LCGPtr unique pointer to a LCG
 * \ingroup scheduler
 */
typedef std::unique_ptr<NrMacSchedulerLCG> LCGPtr;

} // namespace ns3

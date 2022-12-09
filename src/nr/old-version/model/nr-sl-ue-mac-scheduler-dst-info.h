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
#ifndef NR_SL_UE_MAC_SCHEDULER_DST_INFO_H
#define NR_SL_UE_MAC_SCHEDULER_DST_INFO_H


#include "nr-sl-ue-mac-scheduler-lcg.h"
#include "nr-amc.h"

#include <unordered_map>
#include <memory>

namespace ns3 {

class NrSlUeMacSchedulerDstInfo;
/**
 * \brief Shared pointer to an instance of NrSlUeMacSchedulerDstInfo
 */
typedef std::shared_ptr<NrSlUeMacSchedulerDstInfo> DstPtr;

/**
 * \ingroup scheduler
 *
 * \brief The representation of a destination for any NR Sidelink UE MAC scheduler
 *
 * Basic representation for a destination inside any NR Sidelink UE MAC scheduler.
 * The class is responsible to store all the related field that can be used
 * by any NR Sidelink scheduler to schedule resources for a destination.
 */
class NrSlUeMacSchedulerDstInfo
{
public:
  /**
   * \brief Create a new destination info
   *
   * \param dstL2Id the destination layer 2 id
   */
  NrSlUeMacSchedulerDstInfo (uint32_t dstL2Id);

  /**
   * \brief ~NrSlUeMacSchedulerDstInfo destructor
   */
  virtual ~NrSlUeMacSchedulerDstInfo ();

  /**
   * \brief Get the NR Sidelink Logical Channel Group map
   *
   * \return The map storing the pointer to the NrSlUeMacSchedulerLCG
   * per logical channel group id
   */
  std::unordered_map<uint8_t, NrSlLCGPtr> & GetNrSlLCG ();

  /**
   * \brief Insert the pointer to the NrSlUeMacSchedulerLCG object
   *
   *\param lcg the pointer to the NrSlUeMacSchedulerLCG object
   * \return The pointer to the map location where this method has inserted the
   * pointer to the NrSlUeMacSchedulerLCG object
   */
  NrSlLCGIt Insert (NrSlLCGPtr && lcg);

  /**
   * \brief Get destiantion layer 2 id of this destination
   *
   * \return The destination layer 2 id
   */
  uint32_t GetDstL2Id () const;

  /**
   * \brief Set destination MCS
   *
   * \param mcs The MCS used for the destination
   */
  void SetDstMcs (uint8_t mcs);

  /**
   * \brief Get destination MCS
   *
   * \return The MCS used for the destination
   */
  uint8_t GetDstMcs () const;

private:
  std::unordered_map<uint8_t, NrSlLCGPtr> m_nrSlLCG;//!< NR Sidelink LCG
  uint32_t m_dstL2Id {0}; //!< The destination layer 2 id
  uint8_t m_mcs {0}; //!< The MCS
};

} // namespace ns3

#endif /* NR_SL_UE_MAC_SCHEDULER_DST_INFO_H */

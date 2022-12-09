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

#include "nr-mac-scheduler-ns3.h"

namespace ns3 {

/**
 * \ingroup scheduler
 * \brief UE representation for a round-robin scheduler
 *
 * The UE representation does not store any additional information,
 * but provides a way for a RR scheduler to order the UE based on the assigned
 * RBG.
 *
 * \see CompareUeWeightsDl
 */
class NrMacSchedulerUeInfoRR : public NrMacSchedulerUeInfo
{
public:
  /**
   * \brief NrMacSchedulerUeInfoRR constructor
   * \param rnti RNTI of the UE
   * \param beamConfId BeamConfId of the UE
   * \param fn A function that tells how many RB per RBG
   */
  NrMacSchedulerUeInfoRR (uint16_t rnti, BeamConfId beamConfId, const GetRbPerRbgFn &fn)
    : NrMacSchedulerUeInfo (rnti, beamConfId, fn)
  {
  }

  /**
   * \brief comparison function object (i.e. an object that satisfies the
   * requirements of Compare) which returns ​true if the first argument is less
   * than (i.e. is ordered before) the second.
   * \param lue Left UE
   * \param rue Right UE
   * \return true if the assigned RBG of lue is less than the assigned RBG of rue
   *
   * The ordering is made by considering the RBG. An UE with 0 RBG will always
   * be the first (i.e., has an higher priority) in a RR scheduler. The objective
   * is to distribute evenly all the resources, in order to have the same RBG
   * number for all the UEs.
   */
  static bool CompareUeWeightsDl (const NrMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const NrMacSchedulerNs3::UePtrAndBufferReq &rue)
  {
    return (lue.first->m_dlRBG < rue.first->m_dlRBG);
  }

  /**
   * \brief comparison function object (i.e. an object that satisfies the
   * requirements of Compare) which returns ​true if the first argument is less
   * than (i.e. is ordered before) the second.
   * \param lue Left UE
   * \param rue Right UE
   * \return true if the assigned RBG of lue is less than the assigned RBG of rue
   *
   * The ordering is made by considering the RBG. An UE with 0 RBG will always
   * be the first (i.e., has an higher priority) in a RR scheduler. The objective
   * is to distribute evenly all the resources, in order to have the same RBG
   * number for all the UEs.
   */
  static bool CompareUeWeightsUl (const NrMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const NrMacSchedulerNs3::UePtrAndBufferReq &rue)
  {
    return (lue.first->m_ulRBG < rue.first->m_ulRBG);
  }
};

} // namespace ns3

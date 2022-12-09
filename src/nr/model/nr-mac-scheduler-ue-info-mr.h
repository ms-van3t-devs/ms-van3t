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
#include "nr-mac-scheduler-ue-info-rr.h"

namespace ns3 {

/**
 * \ingroup scheduler
 * \brief UE representation for a maximum rate scheduler
 *
 * The class does not store anything more than the NrMacSchedulerUeInfo
 * base class. However, it provides functions to sort the UE based on their
 * maximum achievable rate.
 *
 * \see CompareUeWeightsDl
 */
class NrMacSchedulerUeInfoMR : public NrMacSchedulerUeInfo
{
public:
  /**
   * \brief NrMacSchedulerUeInfoMR constructor
   * \param rnti RNTI of the UE
   * \param beamConfId BeamConfId of the UE
   * \param fn A function that tells how many RB per RBG
   */
  NrMacSchedulerUeInfoMR (uint16_t rnti, BeamConfId beamConfId, const GetRbPerRbgFn &fn)
    : NrMacSchedulerUeInfo (rnti, beamConfId, fn)
  {
  }

  /**
   * \brief comparison function object (i.e. an object that satisfies the
   * requirements of Compare) which returns ​true if the first argument is less
   * than (i.e. is ordered before) the second.
   * \param lue Left UE
   * \param rue Right UE
   * \return true if the MCS of lue is greater than the MCS of rue
   *
   * The ordering is made by considering the MCS of the UE. The higher the MCS,
   * the higher the assigned resources until it has enough to transmit the data.
   */
  static bool CompareUeWeightsDl (const NrMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const NrMacSchedulerNs3::UePtrAndBufferReq &rue)
  {
    if (lue.first->m_dlMcs == rue.first->m_dlMcs)
      {
        return NrMacSchedulerUeInfoRR::CompareUeWeightsDl (lue, rue);
      }

    return (lue.first->m_dlMcs > rue.first->m_dlMcs);
  }

  /**
   * \brief comparison function object (i.e. an object that satisfies the
   * requirements of Compare) which returns ​true if the first argument is less
   * than (i.e. is ordered before) the second.
   * \param lue Left UE
   * \param rue Right UE
   * \return true if the MCS of lue is greater than the MCS of rue
   *
   * The ordering is made by considering the MCS of the UE. The higher the MCS,
   * the higher the assigned resources until it has enough to transmit the data.
   */
  static bool CompareUeWeightsUl (const NrMacSchedulerNs3::UePtrAndBufferReq &lue,
                                  const NrMacSchedulerNs3::UePtrAndBufferReq &rue)
  {
    if (lue.first->m_ulMcs == rue.first->m_ulMcs)
      {
        return NrMacSchedulerUeInfoRR::CompareUeWeightsUl (lue, rue);
      }

    return (lue.first->m_ulMcs > rue.first->m_ulMcs);
  }
};

} // namespace ns3

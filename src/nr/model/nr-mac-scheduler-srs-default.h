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
#ifndef NR_MAC_SCHEDULER_SRS_DEFAULT_H
#define NR_MAC_SCHEDULER_SRS_DEFAULT_H

#include <ns3/object.h>
#include <ns3/random-variable-stream.h>

#include "nr-mac-scheduler-srs.h"

namespace ns3 {

/**
 * \brief Default algorithm for assigning offset and periodicity
 *
 * The algorithm assign the same periodicity to all the UEs. When a new periodicity
 * is asked, it is returned a value between 1 and the configured periodicity (minus 1).
 *
 * The returned values will never be the same; instead, when this must happen,
 * an invalid value is returned and (hopefully) an increase of periodicity is invoked.
 */
class NrMacSchedulerSrsDefault : public NrMacSchedulerSrs, public Object
{
public:
  /**
   * \brief NrMacSchedulerSrsDefault
   */
  NrMacSchedulerSrsDefault ();
  /**
   * \brief ~NrMacSchedulerSrsDefault
   */
  virtual ~NrMacSchedulerSrsDefault ();

  /**
   * \brief GetTypeId
   * \return the object type id
   */
  static TypeId GetTypeId ();

  // inherited from NrMacSchedulerSrs
  virtual SrsPeriodicityAndOffset AddUe (void) override;
  void RemoveUe (uint32_t offset) override;
  virtual bool IncreasePeriodicity (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap) override;
  virtual bool DecreasePeriodicity (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap) override;

  /**
   * \brief Set the Periodicity for all the UEs
   * \param start the periodicity
   */
  void SetStartingPeriodicity (uint32_t start);

  /**
   * \brief Get the periodicity
   * \return the periodicity
   */
  uint32_t GetStartingPeriodicity () const;


  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.
   *
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */
  int64_t AssignStreams (int64_t stream);

private:
  /**
   * \brief Reassign offset/periodicity to all the UEs
   * \param ueMap the UE map of the scheduler
   */
  void ReassignSrsValue (std::unordered_map<uint16_t, std::shared_ptr<NrMacSchedulerUeInfo> > *ueMap);
  static std::vector<uint32_t> StandardPeriodicity; //!< Standard periodicity of SRS

private:
  uint32_t m_periodicity {0};  //!< Configured periodicity
  std::vector<uint32_t> m_availableOffsetValues; //!< Available offset values
  Ptr<UniformRandomVariable> m_random; //!< Random variable
};

} // namespace ns3

#endif // NR_MAC_SCHEDULER_SRS_DEFAULT_H

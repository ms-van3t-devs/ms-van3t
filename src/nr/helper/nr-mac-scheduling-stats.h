/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * This API is derived from MacStatsCalculator of LTE module
 */

#ifndef NR_MAC_SCHEDULING_STATS_H_
#define NR_MAC_SCHEDULING_STATS_H_

#include "ns3/nr-stats-calculator.h"
#include "ns3/nstime.h"
#include "ns3/uinteger.h"
#include <string>
#include <fstream>
#include "ns3/nr-gnb-mac.h"

namespace ns3 {

/**
 * \ingroup nr
 *
 * Takes care of storing the information generated at MAC layer. Metrics saved are:
 *   - Timestamp (in MilliSeconds)
 *   - Cell id
 *   - BWP id
 *   - IMSI
 *   - RNTI
 *   - Frame number
 *   - Subframe number
 *   - Slot number
 *   - Stream id
 *   - MCS
 *   - Size of transport block
 */
class NrMacSchedulingStats : public NrStatsCalculator
{
public:
  /**
   * Constructor
   */
  NrMacSchedulingStats ();

  /**
   * Destructor
   */
  virtual ~NrMacSchedulingStats ();

  // Inherited from ns3::Object
  /**
   *  Register this type.
   *  \return The object TypeId.
   */
  static TypeId GetTypeId (void);

  /**
   * Set the name of the file where the uplink statistics will be stored.
   *
   * \param outputFilename string with the name of the file
   */
  void SetUlOutputFilename (std::string outputFilename);

  /**
   * Get the name of the file where the uplink statistics will be stored.
   * @return the name of the file where the uplink statistics will be stored
   */
  std::string GetUlOutputFilename (void);

  /**
   * Set the name of the file where the downlink statistics will be stored.
   *
   * \param outputFilename string with the name of the file
   */
  void SetDlOutputFilename (std::string outputFilename);

  /**
   * Get the name of the file where the downlink statistics will be stored.
   * @return the name of the file where the downlink statistics will be stored
   */
  std::string GetDlOutputFilename (void);

  /**
   * Notifies the stats calculator that an downlink scheduling has occurred.
   * \param cellId Cell ID of the attached gNb
   * \param imsi IMSI of the scheduled UE
   * \param traceInfo NrSchedulingCallbackInfo structure containing all downlink
   *        information that is generated when DlScheduling trace is fired
   */
  void DlScheduling (uint16_t cellId, uint64_t imsi, const NrSchedulingCallbackInfo &traceInfo);

  /**
   * Notifies the stats calculator that an uplink scheduling has occurred.
   * \param cellId Cell ID of the attached gNB
   * \param imsi IMSI of the scheduled UE
   * \param traceInfo NrSchedulingCallbackInfo structure containing all uplink
   *        information that is generated when DlScheduling trace is fired
   */
  void UlScheduling (uint16_t cellId, uint64_t imsi, const NrSchedulingCallbackInfo &traceInfo);


  /**
   * Trace sink for the ns3::NrGnbMac::DlScheduling trace source
   *
   * \param macStats
   * \param path
   * \param traceInfo NrSchedulingCallbackInfo structure containing all downlink
   *        information that is generated when DlScheduling trace is fired
   */
  static void DlSchedulingCallback (Ptr<NrMacSchedulingStats> macStats, std::string path, NrSchedulingCallbackInfo traceInfo);

  /**
   * Trace sink for the ns3::NrGnbMac::UlScheduling trace source
   *
   * \param macStats the pointer to the MAC stats
   * \param path the trace source path
   * \param traceInfo - all the traces information in a single structure
   */
  static void UlSchedulingCallback (Ptr<NrMacSchedulingStats> macStats, std::string path, NrSchedulingCallbackInfo traceInfo);

private:
  /**
   * When writing DL MAC statistics first time to file,
   * columns description is added. Then next lines are
   * appended to file. This value is true if output
   * files have not been opened yet
   */
  bool m_dlFirstWrite;

  /**
   * When writing UL MAC statistics first time to file,
   * columns description is added. Then next lines are
   * appended to file. This value is true if output
   * files have not been opened yet
   */
  bool m_ulFirstWrite;

};

} // namespace ns3

#endif /* NR_MAC_SCHEDULING_STATS_H_ */

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
#ifndef FLOW_MONITOR_OUTPUT_STATS_H
#define FLOW_MONITOR_OUTPUT_STATS_H

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>
#include <ns3/flow-monitor-helper.h>

namespace ns3 {

/**
 * \brief Class to store the flow monitor values obtained from a simulation
 *
 * The class is meant to store in a database the e2e values for
 * a simulation.
 *
 * \see SetDb
 * \see Save
 */
class FlowMonitorOutputStats
{
public:
  /**
   * \brief FlowMonitorOutputStats constructor
   */
  FlowMonitorOutputStats ();

  /**
   * \brief Install the output dabase.
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   * The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "FlowId INTEGER NOT NULL, "
   * - "TxPackets INTEGER NOT NULL,"
   * - "TxBytes INTEGER NOT NULL,"
   * - "TxOfferedMbps DOUBLE NOT NULL,"
   * - "RxBytes INTEGER NOT NULL,"
   * - "ThroughputMbps DOUBLE NOT NULL, "
   * - "MeanDelayMs DOUBLE NOT NULL, "
   * - "MeanJitterMs DOUBLE NOT NULL, "
   * - "RxPackets INTEGER NOT NULL, "
   * - "SEED INTEGER NOT NULL,"
   * - "RUN INTEGER NOT NULL,"
   * - "PRIMARY KEY(FlowId,SEED,RUN)"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string & tableName);

  /**
   * \brief Store the flow monitor output in the database
   * \param monitor Flow Monitor
   * \param flowmonHelper Flow Monitor Helper
   * \param filename filename for a text output
   */
  void Save (const Ptr<FlowMonitor> &monitor, FlowMonitorHelper &flowmonHelper, const std::string &filename);

private:
  static void
  DeleteWhere (SQLiteOutput *p, uint32_t seed, uint32_t run, const std::string &table);

  SQLiteOutput *m_db;
  std::string m_tableName;
};

} // namespace ns3

#endif // FLOW_MONITOR_OUTPUT_STATS_H

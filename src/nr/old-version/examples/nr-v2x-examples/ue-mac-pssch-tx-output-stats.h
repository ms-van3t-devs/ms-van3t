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
#ifndef UE_MAC_PSSCH_TX_OUTPUT_STATS_H
#define UE_MAC_PSSCH_TX_OUTPUT_STATS_H

#include <vector>

#include <ns3/sqlite-output.h>
#include <ns3/nr-sl-phy-mac-common.h>

namespace ns3 {

/**
 * \brief Class to listen and store the SlPsschScheduling trace of NrUeMac
 *        into a database.
 *
 * \see SetDb
 * \see Save
 */
class UeMacPsschTxOutputStats
{
public:
  /**
   * \brief UeMacPsschTxOutputStats constructor
   */
  UeMacPsschTxOutputStats ();

  /**
   * \brief Install the output database for PSSCH trace from NrUeMac.
   *
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   * The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "timeMs DOUBLE NOT NULL,"
   * - "imsi INTEGER NOT NULL,"
   * - "rnti INTEGER NOT NULL,"
   * - "srcL2Id INTEGER NOT NULL,"
   * - "dstL2Id INTEGER NOT NULL,"
   * - "frame INTEGER NOT NULL,"
   * - "subFrame INTEGER NOT NULL,"
   * - "slot INTEGER NOT NULL,"
   * - "symStart INTEGER NOT NULL,"
   * - "symLen INTEGER NOT NULL,"
   * - "sbChSize INTEGER NOT NULL,"
   * - "rbStart INTEGER NOT NULL,"
   * - "rbLen INTEGER NOT NULL,"
   * - "harqId INTEGER NOT NULL,"
   * - "ndi INTEGER NOT NULL,"
   * - "rv INTEGER NOT NULL,"
   * - "reselCounter INTEGER NOT NULL,"
   * - "cReselCounter INTEGER NOT NULL,"
   * - "csiReq INTEGER NOT NULL,"
   * - "castType INTEGER NOT NULL,"
   * - "SEED INTEGER NOT NULL,"
   * - "RUN INTEGER NOT NULL"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string & tableName);

  /**
   * \brief Store the PSSCH stats parameters into a local vector, which
   *        acts as a cache.
   *
   * \param psschStatsParams The PSSCH stats parameters
   */
  void Save (const SlPsschUeMacStatParameters psschStatsParams);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  /**
   * \brief Delete the table if it already exists with same seed and run number
   * \param p The pointer to the DB
   * \param seed The seed index
   * \param run The run index
   * \param table The name of the table
   */
  static void DeleteWhere (SQLiteOutput *p, uint32_t seed, uint32_t run, const std::string &table);
  /**
   * \brief Write the data stored in our local cache into the DB
   */
  void WriteCache ();

  SQLiteOutput *m_db {nullptr}; //!< DB pointer
  std::string m_tableName {"InvalidTableName"}; //!< table name
  std::vector<SlPsschUeMacStatParameters> m_psschCache;   //!< Result cache
};

} // namespace ns3

#endif // UE_MAC_PSSCH_TX_OUTPUT_STATS_H

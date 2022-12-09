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
#ifndef SINR_OUTPUT_STATS_H
#define SINR_OUTPUT_STATS_H

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>

namespace ns3 {

/**
 * \brief Class to collect and store the SINR values obtained from a simulation
 *
 * The class is meant to store in a database the SINR values from UE or GNB during
 * a simulation. The class contains a cache, that after some time is written
 * to the disk.
 *
 * \see SetDb
 * \see SaveSinr
 * \see EmptyCache
 */
class SinrOutputStats
{
public:
  /**
   * \brief Constructor
   */
  SinrOutputStats ();

  /**
   * \brief Install the output dabase.
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   *  The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "CellId INTEGER NOT NULL, "
   * - "BwpId INTEGER NOT NULL,"
   * - "Rnti INTEGER NOT NULL,"
   * - "AvgSinr DOUBLE NOT NULL,"
   * - "Seed INTEGER NOT NULL,"
   * - "Run INTEGER NOT NULL);"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string& tableName = "sinr");

  /**
   * \brief Store the SINR values
   * \param cellId Cell ID
   * \param rnti RNTI
   * \param avgSinr Average SINR
   * \param bwpId BWP ID
   *
   * The method saves the result in a cache, and if it is necessary, writes the
   * cache to disk before emptying it.
   */
  void SaveSinr (uint16_t cellId, uint16_t rnti, double avgSinr, uint16_t bwpId);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  static void
  DeleteWhere (SQLiteOutput *p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct SinrResultCache
  {
    SinrResultCache (uint16_t c, uint16_t b, uint16_t r, double s)
      : cellId (c), bwpId (b), rnti (r), avgSinr (s) {}

    uint16_t cellId {0};
    uint16_t bwpId {0};
    uint16_t rnti {0};
    double avgSinr {0.0};
  };

  SQLiteOutput *m_db;                         //!< DB pointer
  std::vector<SinrResultCache> m_sinrCache;   //!< Result cache
  std::string m_tableName;                    //!< Table name
};

} // namespace ns3

#endif // SINR_OUTPUT_STATS_H

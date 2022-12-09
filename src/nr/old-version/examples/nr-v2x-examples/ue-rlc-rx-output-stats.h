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
#ifndef UE_RLC_RX_OUTPUT_STATS_H
#define UE_RLC_RX_OUTPUT_STATS_H

#include <vector>

#include <ns3/sqlite-output.h>
#include <ns3/nr-sl-phy-mac-common.h>

namespace ns3 {

/**
 * \brief Class to listen and store the RxRlcPduWithTxRnti trace of NrUeMac
 *        into a database.
 *
 * We added this trace in NR UE MAC because we were unable to figure out how to use
 * ObjectMapValue for nested SL DRB maps in NrSlUeRrc class. This map should be
 * an attribute to hook functions to the RLC and PDCP traces.
 *
 * \see SetDb
 * \see Save
 */
class UeRlcRxOutputStats
{
public:
  /**
   * \brief UeRlcRxOutputStats constructor
   */
  UeRlcRxOutputStats ();

  /**
   * \brief Install the output database for RxRlcPduWithTxRnti trace from NrUeMac.
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
   * - "txRnti INTEGER NOT NULL,"
   * - "lcid INTEGER NOT NULL,"
   * - "rxPdueSize INTEGER NOT NULL,"
   * - "delayNsec INTEGER NOT NULL,"
   * - "SEED INTEGER NOT NULL,"
   * - "RUN INTEGER NOT NULL"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string & tableName);

  /**
   * \brief Store the RxRlcPduWithTxRnti trace parameters into a local vector, which
   *        acts as a cache.
   *
   * \param imsi
   * \param rnti
   * \param txRnti
   * \param lcid
   * \param rxPduSize
   * \param delaySeconds
   */
  void Save (uint64_t imsi, uint16_t rnti, uint16_t txRnti, uint8_t lcid,
             uint32_t rxPduSize, double delaySeconds);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  /**
   * \brief The UeRlcRxData struct to cache the information communicated by
   *        RxRlcPduWithTxRnti trace in NrUeMac
   */
  struct UeRlcRxData
  {
    /**
     * \brief UeRlcRxData constructor
     * \param timeMs The time in milliseconds
     * \param imsi The IMSI of the UE
     * \param rnti The RNTI of the UE
     * \param txRnti The RNTI of the TX UE
     * \param lcid The logical channel id
     * \param rxPduSize The received PDU size in bytes
     * \param delayMicroSeconds The end-to-end, i.e., from TX RLC entity to RX
     *        RLC entity, delay in microseconds
     */
    UeRlcRxData (double timeMs, uint64_t imsi, uint16_t rnti, uint16_t txRnti,
                 uint8_t lcid, uint32_t rxPduSize, int64_t delayMicroSeconds)
      : timeMs (timeMs), imsi (imsi), rnti (rnti), txRnti (txRnti),
        lcid (lcid), rxPduSize (rxPduSize), delayMicroSeconds (delayMicroSeconds)
    {
    }

    double timeMs {0.0}; //!< timeMs The time in milliseconds
    uint64_t imsi {std::numeric_limits <uint64_t>::max ()}; //!< The IMSI of the UE
    uint16_t rnti {std::numeric_limits <uint16_t>::max ()}; //!< The RNTI of the UE
    uint16_t txRnti {std::numeric_limits <uint16_t>::max ()}; //!< The RNTI of the TX UE
    uint8_t lcid {std::numeric_limits <uint8_t>::max ()}; //!< The logical channel id
    uint32_t rxPduSize {std::numeric_limits <uint32_t>::max ()}; //!< The received PDU size in bytes
    double delayMicroSeconds {0}; //!< The end-to-end, i.e., from TX RLC entity to RX RLC entity, delay in microseconds
  };
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
  std::vector<UeRlcRxData> m_rlcRxDataCache;   //!< Result cache
};

} // namespace ns3

#endif // UE_RLC_RX_OUTPUT_STATS_H

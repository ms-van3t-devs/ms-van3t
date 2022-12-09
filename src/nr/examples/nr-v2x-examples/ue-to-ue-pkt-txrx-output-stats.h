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
#ifndef UE_TO_UE_PKT_TXRX_OUTPUT_STATS_H
#define UE_TO_UE_PKT_TXRX_OUTPUT_STATS_H

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>
#include <ns3/network-module.h>

namespace ns3 {

/**
 * \brief Class to listen and store the application level traces of type
 *        TxWithAddresses and RxWithAddresses into a database from an NR
 *        V2X simulation. This class expects both TX and RX nodes to be
 *        a UE, hence, apart from the parameters of the trace source, it methods
 *        require some additional parameters, .e.g., IMSI of the UEs.
 *
 * \see SetDb
 * \see Save
 */
class UeToUePktTxRxOutputStats
{
public:
  /**
   * \brief UeToUePktTxRxOutputStats constructor
   */
  UeToUePktTxRxOutputStats ();

  /**
   * \brief Install the output database for packet TX and RX traces from ns-3
   *        apps. In particular, the traces TxWithAddresses and RxWithAddresses.
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   * The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "timeSec DOUBLE NOT NULL, "
   * - "txRx TEXT NOT NULL,"
   * - "nodeId INTEGER NOT NULL,"
   * - "imsi INTEGER NOT NULL,"
   * - "pktSizeBytes INTEGER NOT NULL,"
   * - "srcIp TEXT NOT NULL,"
   * - "srcPort TEXT NOT NULL,"
   * - "dstIp TEXT NOT NULL,"
   * - "dstPort TEXT NOT NULL,"
   * - "SEED INTEGER NOT NULL,"
   * - "RUN INTEGER NOT NULL"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string & tableName);

  /**
   * \brief Store the packet transmissions and receptions from the application
   *        layer in the database.
   *
   * The parameter 'localAddrs' is passed to this trace in case the
   * address passed by the trace is not set (i.e., is '0.0.0.0' or '::').
   *
   * \param txRx The string indicating the type of node, i.e., TX or RX
   * \param localAddrs The local IPV4 address of the node
   * \param nodeId The node id
   * \param imsi The IMSI
   * \param pktSize The packet size
   * \param srcAddrs The source address from the trace
   * \param dstAddrs The destination address from the trace
   * \param seq The packet sequence number
   */
  void Save (const std::string txRx, const Address &localAddrs, uint32_t nodeId, uint64_t imsi, uint32_t pktSize, const Address &srcAddrs, const Address &dstAddrs, uint32_t seq);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  /**
   * \ingroup nr
   * \brief UePacketResultCache struct to cache the information communicated
   *        by TX/RX application layer traces.
   */
  struct UePacketResultCache
  {
    /**
     * \brief UePacketResultCache constructor
     * \param timeSec The time in seconds
     * \param txRx The string indicating the type of node, i.e., TX or RX
     * \param localAddrs The local IPV4 address of the node
     * \param nodeId The node id of the TX or RX node
     * \param imsi The IMSI of the UE
     * \param pktSize The packet size
     * \param srcAddrs The source address from the trace
     * \param dstAddrs The destination address from the trace
     * \param seq The sequence number of the packet
     */
    UePacketResultCache (double timeSec, std::string txRx, Address localAddrs,
                         uint32_t nodeId, uint64_t imsi, uint32_t pktSize,
                         Address srcAddrs, Address dstAddrs, uint32_t seq)
      : timeSec (timeSec), txRx (txRx), localAddrs (localAddrs),
      nodeId (nodeId), imsi (imsi), pktSize (pktSize), srcAddrs (srcAddrs),
      dstAddrs (dstAddrs),
      seq (seq)
    {
    }

    double timeSec {0.0}; //!< The time in seconds
    std::string txRx {""}; //!< The string indicating the type of node, i.e., TX or RX
    Address localAddrs; //!< The local IPV4 address of the node
    uint32_t nodeId {std::numeric_limits <uint32_t>::max ()}; //!< The node id of the TX or RX node
    uint64_t imsi {std::numeric_limits <uint64_t>::max ()}; //!< The IMSI of the UE
    uint32_t pktSize; //!< The packet size
    Address srcAddrs; //!< The source address from the trace
    Address dstAddrs; //!< The destination address from the trace
    uint32_t seq {std::numeric_limits <uint32_t>::max ()}; //!< The sequence number of the packet
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
  std::vector<UePacketResultCache> m_pktCache;   //!< Result cache
};

} // namespace ns3

#endif // UE_TO_UE_PKT_TXRX_OUTPUT_STATS_H

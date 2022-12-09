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
#ifndef V2X_KPI
#define V2X_KPI

#include <ns3/core-module.h>
#include <inttypes.h>
#include <vector>
#include <sqlite3.h>

namespace ns3 {

/**
 * \brief Class which reads the specific tables of a given DB to
 *        compute V2X KPIs. It could compute following KPIs:
 *        - Average PIR
 *        - Average PRR
 *        - Throughput
 *        - Simultaneous Pssch Tx
 *        - Pssch TB Rx
 *
 * \see SaveAvrgPir
 * \see SaveAvrgPrr
 * \see SaveThput
 * \see SaveSimultPsschTxStats
 * \see SavePsschTbCorruptionStats
 */
class V2xKpi
{
public:
  /**
   * \brief V2xKpi constructor
   */
  V2xKpi ();

  /**
   * \brief V2xKpi destructor
   */
  ~V2xKpi ();

  /**
   * \brief Set the path to the DB to read the tables from.
   * \param dbPath The path to the DB
   */
  void SetDbPath (std::string dbPath);
  /**
   * \brief Set the duration of the transmitting application.
   *
   * Note: Without setting this throughput KPI can not be computed.
   *
   * \param duration The duration of the transmitting application in seconds.
   */
  void SetTxAppDuration (double duration);
  /**
   * \brief Write the KPIs in their respective tables in the DB.
   */
  void WriteKpis ();
  /**
   * \brief Consider all TX links while writing the stats, e.g, throughput to the DB.
   *
   * If this flag is set the code will also write the stats for those TX
   * nodes from whom the RX node didn't receive anything. In such case, the
   * stats, e.g., the throughput and the rxed packets will be zero for such TX
   * node.
   *
   * \param allTx Flag to consider all the TX nodes for stats
   */
  void ConsiderAllTx (bool allTx);
  /**
   * \brief Fill the map to store the IP and the initial position of each node
   * \param ip The IP of the node
   * \param pos The position of the node
   */
  void FillPosPerIpMap (std::string ip, Vector pos);
  /**
   * \brief Set the range to be considered while writing the range based
   * KPIs, e.g., PIR, PRR.
   * \param range The inter-node-distance (2D) in meter
   */
  void SetRangeForV2xKpis (uint16_t range);

private:
  /**
   * \ingroup nr
   * \brief PktTxRxData struct to store information read from a table in the
   *        database storing the information communicated by TX/RX application
   *        layer traces.
   *
   */
  struct PktTxRxData
  {
    /**
     * \brief PktTxRxData constructor
     * \param time The time value in the table
     * \param txRx Nature of the trace, i.e., TX or RX
     * \param nodeId The node id
     * \param imsi The IMSI of the UE
     * \param pktSize The packet size
     * \param ipAddrs The IP address of the node
     * \param pktSeq The packet sequence number
     */
    PktTxRxData (double time, std::string txRx, uint32_t nodeId, uint64_t imsi, uint32_t pktSize, std::string ipAddrs, uint32_t pktSeq)
      : time (time), txRx (txRx), nodeId (nodeId), imsi (imsi), pktSize (pktSize), ipAddrs (ipAddrs), pktSeq (pktSeq)
    {}

    double time; //!< time
    std::string txRx {""}; //!< tx/rx indicator
    uint32_t nodeId {std::numeric_limits <uint32_t>::max ()}; //!< node id of tx or rx node
    uint64_t imsi {std::numeric_limits <uint64_t>::max ()}; //!< IMSI of the tx and rx node
    uint32_t pktSize; //!< packet size
    std::string ipAddrs; //!< The ip address of the node.
    uint32_t pktSeq {std::numeric_limits <uint32_t>::max ()}; //!<< packet sequance number
  };
  /**
   * \ingroup nr
   * \brief PsschTxData struct to store information read from a table in the
   *        database storing the information communicated by SlPsschScheduling
   *        trace of NrUeMac class.
   *
   */
  struct PsschTxData
  {
    /**
     * \brief PsschTxData constructor
     * \param frame The frame number
     * \param subFrame The subframe number
     * \param slot The slot number
     * \param symStart The starting symbol index
     * \param symLen The total number of symbol
     * \param rbStart The starting RB index
     * \param rbLen The total number of RBs
     */
    PsschTxData (uint32_t frame, uint32_t subFrame, uint16_t slot, uint16_t symStart, uint16_t symLen, uint16_t rbStart, uint16_t rbLen)
      : frame (frame), subFrame (subFrame), slot (slot),symStart (symStart), symLen (symLen), rbStart (rbStart), rbLen (rbLen)
    {}

    uint32_t frame {std::numeric_limits<uint32_t>::max ()}; //!< The frame number;
    uint32_t subFrame {std::numeric_limits<uint32_t>::max ()}; //!< The subframe number;
    uint16_t slot {std::numeric_limits<uint16_t>::max ()}; //!< The slot number;
    uint16_t symStart {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the starting symbol used for sidelink PSSCH in a slot;
    uint16_t symLen {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the total number of symbols allocated for sidelink PSSCH;
    uint16_t rbStart {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the starting resource block
    uint16_t rbLen {std::numeric_limits <uint16_t>::max ()}; //!< Indicates the total number of contiguous resource block

    /**
     * \brief Equal to operator for PsschTxData object
     * \param r another instance
     * \return true if this and r are equal, false otherwise
     */
    bool operator == (const PsschTxData &r)
    {
      return (this->frame == r.frame
             && this->subFrame == r.subFrame
             && this->slot == r.slot
             && ((this->symStart <= r.symStart + r.symLen - 1) && (r.symStart <= this->symStart + this->symLen - 1))
             && ((this->rbStart <= r.rbStart + r.rbLen - 1) && (r.rbStart <= this->rbStart + this->rbLen - 1)));
    }
  };

  /**
   * \brief Delete the table if it already exists with same seed and run number
   * \param seed The seed index
   * \param run The run index
   * \param table The name of the table
   */
  void DeleteWhere (uint32_t seed, uint32_t run, const std::string &table);
  /**
   * \brief Save the RX packet data from pktTxRx table.
   *
   * This method reads and save the entries of pktTxRx table by filtering
   * txRx column using rx key.
   */
  void SavePktRxData ();
  /**
   * \brief Save the TX packet data from pktTxRx table.
   *
   * This method reads and save the entries of pktTxRx table by filtering
   * txRx column using tx key.
   */
  void SavePktTxData ();
  /**
   * \brief Save average PIR
   *
   * This method compute the average PIR of each receiver node with
   * respect to each transmitter it has received packets from. This
   * average PIR is then written in to a new table "avrgPir" of
   * the DB. There are two things to keep in mind:
   *
   * 1) Range based PIR computation is only valid for the scenarios with
   *    constant velocity, i.e., inter-vehicle distance remains the same.
   * 2) To enable the range based PIR m_range variable value must be greater
   *    than zero; otherwise, range is ignored, i.e., all the transmitting nodes
   *    from whom the a receiving node has received more than 1 packet
   *    becomes the potential transmitter, hence, its PIR must be computed.
   */
  void SaveAvrgPir ();
  /**
   * \brief Compute the average PIR
   * \param ipTx of the transmitting node for which we are computing the PIR
   * \param data The data to be used to compute the average PIR
   * \return The average PIR
   */
  double ComputeAvrgPir (std::string ipTx, std::vector <PktTxRxData> data);
  /**
   * \brief Save throughput
   *
   * This method compute the throughput of each receiver node with
   * respect to each transmitter it has received packets from. This
   * throughput is then written in to a new table "thputKbps" of
   * the DB.
   */
  void SaveThput ();
  /**
   * \brief Compute the throughput
   *
   * This method would fail if TX application duration is not set.
   *
   * \param data The data to be used to compute the throughput
   * \return The throughput
   *
   * \see SetTxAppDuration
   */
  double ComputeThput (std::vector <PktTxRxData> data);
  /**
   * \brief Get the total transmitted packets by a transmitter
   * \param srcIpAddrs The IP of the transmitter
   * \return The total transmitted packets
   */
  uint64_t GetTotalTxPkts (std::string srcIpAddrs);
  /**
   * \brief Compute PSSCH TX stats
   *
   * This method for a simulation will count the total PSSCH transmission,
   * the number of non-overlapping PSSCH transmission, and the number of
   * overlapping PSSCH transmissions. An overlap means if the two transmissions
   * occurred on the same frame, subframe, slot, and there is a full or partial
   * overlap in frequency, i.e. RBs and in time, i.e., symbols.
   */
  void ComputePsschTxStats ();
  /**
   * \brief Save Simultaneous Pssch Tx Stats to sqlite table in the DB
   * \param totalPsschTx The total PSSCH transmissions in a simulation
   * \param numNonOverLapPsschTx The number of non-overlapping PSSCH TX
   * \param numOverLapPsschTx The number of overlapping PSSCH TX
   */
  void SaveSimultPsschTxStats (uint32_t totalPsschTx, uint32_t numNonOverLapPsschTx, uint32_t numOverLapPsschTx);
  /**
   * \brief Compute PSSCH TB corruption stats
   */
  void ComputePsschTbCorruptionStats ();
  /**
   * \brief Save PSSCH TB corruption stats
   * \param totalTbRx The total received TBs
   * \param psschSuccessCount The count of successfully decoded PSSCH
   * \param sci2SuccessCount The count of successfully decoded SCI 2
   */
  void SavePsschTbCorruptionStats (uint32_t totalTbRx, uint32_t psschSuccessCount, uint32_t sci2SuccessCount);
  /**
   * \brief Save average PRR (Packet Reception Ratio)
   *
   * This method compute the average PRR of each transmitting node with
   * respect to each of its potential neighbor in a range it has transmitted
   * packets to. This average PIR is then written in to a new table "avrgPrr"
   * of the DB. There are two things to keep in mind:
   *
   * 1) Range based PRR computation is only valid for the scenarios with
   *    constant velocity, i.e., inter-vehicle distance remains the same.
   * 2) To enable the range based PRR m_range variable value must be greater
   *    than zero; otherwise, range is ignored, i.e., all the receiving nodes
   *    becomes the potential receivers for a transmitter.
   */
  void SaveAvrgPrr ();
  /**
   * \brief Compute average PRR (Packet Reception Ratio)
   * \param txIt An iterator of the m_txDataMap pointing to the transmitter
   *        data for which PRR is to be computed
   * \param numNeib A variable where the total number of neighbors for a
   *        transmitter will be stored (note, it is passed as reference)
   * \return The average PRR value
   */
  double ComputeAvrgPrr (std::map <uint32_t, std::vector <PktTxRxData> >::const_iterator &txIt, uint32_t &numNeib);

  /*
   * Key 1 = Rx node id
   * Key 2 = IP address of the transmitter this RX node received pkts from
   * value of second map = data to compute KPIs or other stats, e.g., PIR, thput.
   */
  std::map <uint32_t, std::map <std::string, std::vector <PktTxRxData> > > m_rxDataMap; //!< map to store the rx data of each node w.r.t its transmitters
  /*
   * Key = Tx node id
   * value = data to compute KPI or other stats, e.g., total txed packets by a tx node
   */
  std::map <uint32_t, std::vector <PktTxRxData> > m_txDataMap; //!< map to store the tx data per transmitting node
  sqlite3* m_db {nullptr}; //!< DB pointer
  std::string m_dbPath {""}; //!< path to the DB to read
  double m_txAppDuration {0.0}; //!< The TX application duration to compute the throughput
  bool m_considerAllTx {true}; //!< Consider all TX flag for throughput computation
  std::map <std::string, Vector> m_posPerIp; //!< Map to store position and IPs of the nodes
  std::uint16_t m_range {0}; //!< Range in meter to be used to compute PIR and PRR
  double m_interTxRxDistance {0.0}; //!< The inter-TX-RX distance logged for PIR
};

} // namespace ns3

#endif // V2X_KPI

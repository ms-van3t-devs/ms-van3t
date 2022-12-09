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
#include "v2x-kpi.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("V2xKpi");

V2xKpi::V2xKpi ()
{}

V2xKpi::~V2xKpi ()
{
  sqlite3_close (m_db);
  m_db = nullptr;
}

void
V2xKpi::DeleteWhere (uint32_t seed, uint32_t run, const std::string &table)
{
  int rc;
  sqlite3_stmt *stmt;
  std::string cmd = "DELETE FROM \"" + table + "\" WHERE SEED = ? AND RUN = ?;";
  rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Could not prepare correctly the delete statement. Db error: " << sqlite3_errmsg (m_db));

  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 1, seed) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, run) == SQLITE_OK);

  rc = sqlite3_step (stmt);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the finalize statement. Db error: " << sqlite3_errmsg (m_db));
}

void
V2xKpi::SetDbPath (std::string dbPath)
{
  m_dbPath = dbPath + ".db";
}

void
V2xKpi::SetTxAppDuration (double duration)
{
  m_txAppDuration = duration;
}

void
V2xKpi::WriteKpis ()
{
  SavePktTxData ();
  SavePktRxData ();
  SaveAvrgPir ();
  SaveThput ();
  ComputePsschTxStats ();
  ComputePsschTbCorruptionStats ();
  SaveAvrgPrr ();
}

void
V2xKpi::ConsiderAllTx (bool allTx)
{
  m_considerAllTx = allTx;
}

void
V2xKpi::SetRangeForV2xKpis (uint16_t range)
{
  m_range = range;
}

void
V2xKpi::FillPosPerIpMap (std::string ip, Vector pos)
{
  bool insertStatus = m_posPerIp.insert (std::make_pair (ip, pos)).second;
  NS_ABORT_MSG_IF (insertStatus == false, "Insert Error: Pos of the ip " << ip << " already exist in the map");
}

void
V2xKpi::SavePktRxData ()
{
  int rc;
  rc = sqlite3_open (m_dbPath.c_str (), &m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error open DB. Db error: " << sqlite3_errmsg (m_db));

  sqlite3_stmt *stmt;
  std::string sql ("SELECT * FROM pktTxRx WHERE txRx = 'rx' AND txRx IS NOT NULL AND SEED = ? AND RUN = ?;");
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error SELECT. Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 1, RngSeedManager::GetSeed ()) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, RngSeedManager::GetRun ()) == SQLITE_OK);

  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      uint32_t nodeId = sqlite3_column_int (stmt, 2);
      auto nodeIt = m_rxDataMap.find (nodeId);
      if (nodeIt == m_rxDataMap.end ())
        {
          //NS_LOG_UNCOND ("nodeId = " << nodeId << " not found creating new entry");
          std::map <std::string, std::vector <PktTxRxData> > secondMap;
          std::string srcIp = std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 5)));
          std::vector <PktTxRxData> data;
          PktTxRxData result (sqlite3_column_double (stmt, 0),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 1))),
                              sqlite3_column_int (stmt, 2),
                              sqlite3_column_int (stmt, 3),
                              sqlite3_column_int (stmt, 4),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 7))),
                              sqlite3_column_int (stmt, 9));
          data.push_back (result);
          secondMap.insert (std::make_pair (srcIp, data));
          m_rxDataMap.insert (std::make_pair (nodeId, secondMap));
        }
      else
        {
          //NS_LOG_UNCOND ("nodeId = " << nodeId << " found");
          std::string srcIp = std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 5)));
          auto txIt = nodeIt->second.find (srcIp);
          if (txIt == nodeIt->second.end ())
            {
              //NS_LOG_UNCOND ("srcIp = " << srcIp << " not found creating new entry");
              std::vector <PktTxRxData> data;
              PktTxRxData result (sqlite3_column_double (stmt, 0),
                                  std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 1))),
                                  sqlite3_column_int (stmt, 2),
                                  sqlite3_column_int (stmt, 3),
                                  sqlite3_column_int (stmt, 4),
                                  std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 7))),
                                  sqlite3_column_int (stmt, 9));
              data.push_back (result);
              nodeIt->second.insert (std::make_pair (srcIp, data));
            }
          else
            {
              //NS_LOG_UNCOND ("srcIp = " << srcIp << " found");
              PktTxRxData result (sqlite3_column_double (stmt, 0),
                                  std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 1))),
                                  sqlite3_column_int (stmt, 2),
                                  sqlite3_column_int (stmt, 3),
                                  sqlite3_column_int (stmt, 4),
                                  std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 7))),
                                  sqlite3_column_int (stmt, 9));
              txIt->second.push_back (result);
            }
        }
    }

  NS_ABORT_MSG_UNLESS (rc == SQLITE_DONE, "Error not DONE. Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
}

void
V2xKpi::SaveAvrgPir ()
{
  std::string tableName = "avrgPir";
  std::string cmd =  ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "txRx TEXT NOT NULL,"
                      "nodeId INTEGER NOT NULL,"
                      "imsi INTEGER NOT NULL,"
                      "srcIp TEXT NOT NULL,"
                      "dstIp TEXT NOT NULL,"
                      "avrgPirSec DOUBLE NOT NULL,"
                      "TxRxDistance DOUBLE NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");
  int rc = sqlite3_exec (m_db, cmd.c_str (), NULL, NULL, NULL);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error creating table. Db error: " << sqlite3_errmsg (m_db));

  DeleteWhere (RngSeedManager::GetSeed (), RngSeedManager::GetRun (), tableName);

  for (const auto &it:m_rxDataMap)
    {
      for (const auto &it2:it.second)
        {
          double avrgPir = ComputeAvrgPir (it2.first.c_str (), it2.second);
          if (avrgPir == -1.0)
            {
              //It may happen that a node would rxed only one pkt from a
              //particular tx node. In that case, PIR can not be computed.
              //Therefore, we do not log the PIR.
              continue;
            }
          //NS_LOG_UNCOND ("Avrg PIR " << avrgPir);
          PktTxRxData data = it2.second.at (0);
          sqlite3_stmt *stmt;
          std::string cmd = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
          rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 1, data.txRx.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, data.nodeId) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 3, data.imsi) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 4,it2.first.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 5, data.ipAddrs.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 6, avrgPir) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 7, m_interTxRxDistance) == SQLITE_OK);
          m_interTxRxDistance = 0.0;
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 8, RngSeedManager::GetSeed ()) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 9, RngSeedManager::GetRun ()) == SQLITE_OK);

          rc = sqlite3_step (stmt);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
          rc = sqlite3_finalize (stmt);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
        }
    }
}
double
V2xKpi::ComputeAvrgPir (std::string ipTx, std::vector <PktTxRxData> data)
{
  uint64_t pirCounter = 0;
  double lastPktRxTime = 0.0;
  double pir = 0.0;

  //NS_LOG_UNCOND ("Packet Vector size " << data.size () << " to compute average PIR");
  if (m_range > 0)
    {
      PktTxRxData dataIpRx = data.at (0);
      auto itRxIpPos = m_posPerIp.find (dataIpRx.ipAddrs);
      NS_ABORT_MSG_IF (itRxIpPos == m_posPerIp.end (), "Unable to find the position of RX IP " << dataIpRx.ipAddrs);
      Vector rxIpPos = itRxIpPos->second;
      auto itTxIpPos = m_posPerIp.find (ipTx);
      NS_ABORT_MSG_IF (itTxIpPos == m_posPerIp.end (), "Unable to find the position of TX IP " << ipTx);
      Vector txIpPos = itTxIpPos->second;
      double distance = CalculateDistance (rxIpPos, txIpPos);
      m_interTxRxDistance = distance;
      if (distance > m_range)
        {
          return -1.0;
        }
    }

  for (const auto &it:data)
    {
      if (pirCounter == 0 && lastPktRxTime == 0.0)
        {
          //this is the first packet, just store the time and get out
          lastPktRxTime = it.time;
          continue;
        }

      // NS_LOG_UNCOND ("it.time - lastPktRxTime " << it.time - lastPktRxTime);
      pir = pir + (it.time - lastPktRxTime);
      lastPktRxTime = it.time;
      pirCounter++;
    }
  double avrgPir = 0.0;
  if (pirCounter == 0)
    {
      //It may happen that a node would rxed only one pkt from a
      //particular tx node. In that case, PIR can not be computed.
      //assert added just to check range based
      avrgPir = -1.0;
    }
  else
    {
      avrgPir = pir / pirCounter;
    }
  return avrgPir;
}


void
V2xKpi::SaveAvrgPrr ()
{
  std::string tableName = "avrgPrr";
  std::string cmd =  ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "txRx TEXT NOT NULL,"
                      "nodeId INTEGER NOT NULL,"
                      "imsi INTEGER NOT NULL,"
                      "Ip TEXT NOT NULL,"
                      "range DOUBLE NOT NULL,"
                      "numNieb INTEGER NOT NULL,"
                      "avrgPrr DOUBLE NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");
  int rc = sqlite3_exec (m_db, cmd.c_str (), NULL, NULL, NULL);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error creating table. Db error: " << sqlite3_errmsg (m_db));

  DeleteWhere (RngSeedManager::GetSeed (), RngSeedManager::GetRun (), tableName);

  std::map <uint32_t, std::vector <PktTxRxData> >::const_iterator it;
  for (it = m_txDataMap.cbegin (); it != m_txDataMap.cend (); it++)
    {
      uint32_t numNeib = 0;
      double avrgPrr = ComputeAvrgPrr (it, numNeib);
      if (avrgPrr == -1.0)
        {
          continue;
        }
      sqlite3_stmt *stmt;
      std::string cmd = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
      rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
      NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

      NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 1, it->second.at (0).txRx.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, it->second.at (0).nodeId) == SQLITE_OK);
      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 3, it->second.at (0).imsi) == SQLITE_OK);
      NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 4,it->second.at (0).ipAddrs.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 5, m_range) == SQLITE_OK);
      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 6, numNeib) == SQLITE_OK);
      NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 7, avrgPrr) == SQLITE_OK);
      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 8, RngSeedManager::GetSeed ()) == SQLITE_OK);
      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 9, RngSeedManager::GetRun ()) == SQLITE_OK);

      rc = sqlite3_step (stmt);
      NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
      rc = sqlite3_finalize (stmt);
      NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
    }

}

double
V2xKpi::ComputeAvrgPrr (std::map <uint32_t, std::vector <PktTxRxData> >::const_iterator &txIt, uint32_t &numNeib)
{
  std::string txIp = txIt->second.at (0).ipAddrs;
  auto txIpPosIt = m_posPerIp.find (txIp);
  Vector txIpPos = txIpPosIt->second;
  //std::cout << "Tx IP " << txIp << " pos " << txIpPos << std::endl;

  std::map<std::string, std::vector<uint32_t> > pktSeqPerRx;

  for (const auto &it:m_rxDataMap)
    {
      //we can read the RX IP from any PktTxRxData of any TX
      std::string rxIp = it.second.begin ()->second.at (0).ipAddrs;
      auto rxIpPosIt = m_posPerIp.find (rxIp);
      Vector rxIpPos = rxIpPosIt->second;
      double txRxDist = CalculateDistance (rxIpPos, txIpPos);
      //go to the next RX IP if the current RX IP is out of range AND m_range is non-zero
      //the condition m_range > 0.0 is to ignore range based PRR and consider
      //all rx nodes as potential receivers.
      if (txRxDist > m_range && m_range > 0.0)
        {
          //std::cout << "Rx IP " << rxIp << " at " <<  rxIpPos << " is out of range" << std::endl;
          continue;
        }
      else
        {
          std::vector<uint32_t> seqVec;
          //RX IP is in range. Lets find out if this RX IP rexed any pkt from
          //Tx IP
          auto txIt = it.second.find (txIp);
          if (txIt == it.second.end ())
            {
              //if with in range Rx IP did not rxed any packet from Tx IP
              //we still need to consider it as valid neighbor. Therefore,
              //I am inserting it in the neighbor vector which received
              //a bit unrealistic packet seq of 2^32
              seqVec.push_back (std::numeric_limits <uint32_t>::max ());
              bool insertStatus = pktSeqPerRx.emplace (std::make_pair (rxIp,seqVec)).second;
              NS_ASSERT_MSG (insertStatus == true, "Rx IP " << rxIp << " already present in pktSeqPerRx map");
              //now continue to next RX IP
              continue;
            }
          for (const auto &pktVectIt:txIt->second)
            {
              //push all the pkt seq number this receiver received from the TX IP
              seqVec.push_back (pktVectIt.pktSeq);
            }
          bool insertStatus = pktSeqPerRx.emplace (std::make_pair (rxIp,seqVec)).second;
          NS_ASSERT_MSG (insertStatus == true, "Rx IP " << rxIp << " already present in pktSeqPerRx map");
        }
    }

  numNeib = pktSeqPerRx.size ();

  //std::cout << "IP " << txIp << " has " << pktSeqPerRx.size () << " neighbors" << std::endl;
  //std::cout << "IP " << txIp << " has txed " << txIt->second.size ()<< " packets" << std::endl;

  //if none of the rx nodes is in range do not log such PRR
  if (pktSeqPerRx.size () == 0)
    {
      return -1.0;
    }

  double pktRxCount = 0;

  for (const auto &itPktData:txIt->second)
    {
      //iterating over each pkt tx by the transmitter
      for (const auto &itRx:pktSeqPerRx)
        {
          if (std::find (itRx.second.begin(), itRx.second.end(), itPktData.pktSeq) != itRx.second.end())
          {
              pktRxCount++;
          }
        }
    }

  //if none of the rx nodes in range received any packet return 0
  if (pktRxCount == 0)
    {
      return 0.0;
    }

  double avrgPrr = pktRxCount / (txIt->second.size () * pktSeqPerRx.size ());

  //std::cout << "Tx IP " << txIp << " has average PRR of " << avrgPrr << std::endl;

  return avrgPrr;
}


void
V2xKpi::SaveThput ()
{
  std::string tableName = "thput";
  std::string cmd =  ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "txRx TEXT NOT NULL,"
                      "nodeId INTEGER NOT NULL,"
                      "imsi INTEGER NOT NULL,"
                      "srcIp TEXT NOT NULL,"
                      "totalPktTxed int NOT NULL,"
                      "dstIp TEXT NOT NULL,"
                      "totalPktRxed int NOT NULL,"
                      "thputKbps DOUBLE NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");
  int rc = sqlite3_exec (m_db, cmd.c_str (), NULL, NULL, NULL);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error creating table. Db error: " << sqlite3_errmsg (m_db));

  DeleteWhere (RngSeedManager::GetSeed (), RngSeedManager::GetRun (), tableName);

  for (const auto &it:m_rxDataMap)
    {
      for (const auto &it2:it.second)
        {
          double thput = ComputeThput (it2.second);
          //NS_LOG_UNCOND ("thput " << thput << " kbps");
          PktTxRxData data = it2.second.at (0);
          sqlite3_stmt *stmt;
          std::string cmd = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
          rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 1, data.txRx.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, data.nodeId) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 3, data.imsi) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 4, it2.first.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 5, GetTotalTxPkts (it2.first)) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 6, data.ipAddrs.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 7, static_cast <uint64_t> (it2.second.size ())) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 8, thput) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 9, RngSeedManager::GetSeed ()) == SQLITE_OK);
          NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 10, RngSeedManager::GetRun ()) == SQLITE_OK);

          rc = sqlite3_step (stmt);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
          rc = sqlite3_finalize (stmt);
          NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
        }

      //Now put zero thput for the transmitters nodes from
      //whom this RX node didn't receive any packet.
      NS_LOG_DEBUG ("m_considerAllTx flag value" << m_considerAllTx);
      NS_LOG_DEBUG ("Num of transmitters this receiver able to rxed data " << it.second.size ());
      NS_LOG_DEBUG ("Total number of transmitters " << m_txDataMap.size ());
      //Lets read the first entry of our m_rxDataMap just to read some info of
      //the RX node.
      PktTxRxData data = it.second.begin ()->second.at (0);
      uint32_t numTx = 0;
      auto itToRxNode = m_txDataMap.find (data.nodeId);
      if (itToRxNode != m_txDataMap.end ())
        {
          //if this receiver is one of the transmitter, the number of
          //transmitters for which we need to compute thput is:
          numTx = m_txDataMap.size () - 1;
        }
      else
        {
          //consider all the transmitters
          numTx = m_txDataMap.size ();
        }
      //if (report stats for all TX nodes AND total number of tx nodes from whom
      //this receiver node rxed the packets is less the number of actual
      //transmitters)
      if (m_considerAllTx && (it.second.size () < numTx))
        {
          for (const auto &itTx:m_txDataMap)
            {
              if (it.second.find (itTx.second.at (0).ipAddrs) == it.second.end ())
                {
                  //we didnt find the TX in our m_rxDataMap.
                  //avoid my own IP
                  if (itTx.second.at (0).ipAddrs != data.ipAddrs)
                    {
                      sqlite3_stmt *stmt;
                      std::string cmd = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
                      rc = sqlite3_prepare_v2 (m_db, cmd.c_str (), static_cast<int> (cmd.size ()), &stmt, nullptr);
                      NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

                      NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 1, data.txRx.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, data.nodeId) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 3, data.imsi) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 4, itTx.second.at (0).ipAddrs.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 5, GetTotalTxPkts (itTx.second.at (0).ipAddrs)) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_text (stmt, 6, data.ipAddrs.c_str (), -1, SQLITE_STATIC) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 7, 0) == SQLITE_OK); // zero rxed pkets
                      NS_ABORT_UNLESS (sqlite3_bind_double (stmt, 8, 0) == SQLITE_OK); // zero thput
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 9, RngSeedManager::GetSeed ()) == SQLITE_OK);
                      NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 10, RngSeedManager::GetRun ()) == SQLITE_OK);

                      rc = sqlite3_step (stmt);
                      NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
                      rc = sqlite3_finalize (stmt);
                      NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
                    }
                }
            }
        }
    }
}

double
V2xKpi::ComputeThput (std::vector <PktTxRxData> data)
{
  NS_ABORT_MSG_IF (m_txAppDuration == 0.0, "Can not compute throughput with " << m_txAppDuration << " duration");
  uint64_t rxByteCounter = 0;

  //NS_LOG_UNCOND ("Packet Vector size " << data.size () << " to throughput");
  for (const auto &it:data)
    {
      rxByteCounter += it.pktSize;
    }

  //thput in kpbs
  double thput = (rxByteCounter * 8) / m_txAppDuration / 1000.0;
  return thput;
}

void
V2xKpi::SavePktTxData ()
{
  int rc;
  rc = sqlite3_open (m_dbPath.c_str (), &m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error open DB. Db error: " << sqlite3_errmsg (m_db));

  sqlite3_stmt *stmt;
  std::string sql ("SELECT * FROM pktTxRx WHERE txRx = 'tx' AND txRx IS NOT NULL AND SEED = ? AND RUN = ?;");
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error SELECT. Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 1, RngSeedManager::GetSeed ()) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, RngSeedManager::GetRun ()) == SQLITE_OK);

  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      uint32_t nodeId = sqlite3_column_int (stmt, 2);
      auto nodeIt = m_txDataMap.find (nodeId);
      if (nodeIt == m_txDataMap.end ())
        {
          //NS_LOG_UNCOND ("nodeId = " << nodeId << " not found creating new entry");
          PktTxRxData result (sqlite3_column_double (stmt, 0),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 1))),
                              sqlite3_column_int (stmt, 2),
                              sqlite3_column_int (stmt, 3),
                              sqlite3_column_int (stmt, 4),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 5))),
                              sqlite3_column_int (stmt, 9));
          std::vector <PktTxRxData> data;
          data.push_back (result);
          m_txDataMap.insert (std::make_pair (nodeId, data));
        }
      else
        {
          PktTxRxData result (sqlite3_column_double (stmt, 0),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 1))),
                              sqlite3_column_int (stmt, 2),
                              sqlite3_column_int (stmt, 3),
                              sqlite3_column_int (stmt, 4),
                              std::string (reinterpret_cast< const char* > (sqlite3_column_text (stmt, 5))),
                              sqlite3_column_int (stmt, 9));
          nodeIt->second.push_back (result);
        }
    }

  NS_ABORT_MSG_UNLESS (rc == SQLITE_DONE, "Error not DONE. Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
}

uint64_t
V2xKpi::GetTotalTxPkts (std::string srcIpAddrs)
{
  uint64_t totalTxPkts = 0;
  for (const auto &it:m_txDataMap)
    {
      if (it.second.at (0).ipAddrs == srcIpAddrs)
        {
          totalTxPkts = it.second.size ();
          break;
        }
    }

  return totalTxPkts;
}

void
V2xKpi::ComputePsschTxStats ()
{
  int rc;
  rc = sqlite3_open (m_dbPath.c_str (), &m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error open DB. Db error: " << sqlite3_errmsg (m_db));

  sqlite3_stmt *stmt;
  std::string sql ("SELECT * FROM psschTxUeMac WHERE SEED = ? AND RUN = ?;");
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error SELECT. Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 1, RngSeedManager::GetSeed ()) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, RngSeedManager::GetRun ()) == SQLITE_OK);
  uint32_t rowCount = 0;
  std::vector <PsschTxData> nonOverLapPsschTx;
  std::vector <PsschTxData> overLapPsschTx;
  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      rowCount++;
      PsschTxData result (sqlite3_column_int (stmt, 5),
                          sqlite3_column_int (stmt, 6),
                          sqlite3_column_int (stmt, 7),
                          sqlite3_column_int (stmt, 8),
                          sqlite3_column_int (stmt, 9),
                          sqlite3_column_int (stmt, 11),
                          sqlite3_column_int (stmt, 12));

      if (nonOverLapPsschTx.size () == 0 && nonOverLapPsschTx.size () == 0)
        {
          nonOverLapPsschTx.push_back (result);
        }
      else
        {
          auto it = std::find (nonOverLapPsschTx.begin (), nonOverLapPsschTx.end (), result);
          if (it != nonOverLapPsschTx.end ())
            {
              overLapPsschTx.push_back (result);
              overLapPsschTx.push_back (*it);
              nonOverLapPsschTx.erase (it);
            }
          else
            {
              auto it = std::find (overLapPsschTx.begin (), overLapPsschTx.end (), result);
              if (it != overLapPsschTx.end ())
                {
                  overLapPsschTx.push_back (result);
                }
              else
                {
                  nonOverLapPsschTx.push_back (result);
                }
            }
        }
    }

  NS_ABORT_MSG_UNLESS (rc == SQLITE_DONE, "Error not DONE. Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));


  //NS_LOG_UNCOND ("Non-overlapping Tx " << nonOverLapPsschTx.size ());
  //NS_LOG_UNCOND ("overlapping Tx " << overLapPsschTx.size ());
  //NS_LOG_UNCOND ("Total rows " << rowCount);
  SaveSimultPsschTxStats (rowCount, nonOverLapPsschTx.size (), overLapPsschTx.size ());
}

void
V2xKpi::SaveSimultPsschTxStats (uint32_t totalPsschTx, uint32_t numNonOverLapPsschTx, uint32_t numOverLapPsschTx)
{
  std::string tableName = "simulPsschTx";
  std::string cmd =  ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "totalTx INTEGER NOT NULL,"
                      "numNonOverlapping INTEGER NOT NULL,"
                      "numOverlapping INTEGER NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");
  int rc = sqlite3_exec (m_db, cmd.c_str (), NULL, NULL, NULL);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error creating table. Db error: " << sqlite3_errmsg (m_db));

  DeleteWhere (RngSeedManager::GetSeed (), RngSeedManager::GetRun (), tableName);


  sqlite3_stmt *stmt;
  std::string sql = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?);";
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 1, totalPsschTx) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 2, numNonOverLapPsschTx) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 3, numOverLapPsschTx) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 4, RngSeedManager::GetSeed ()) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 5, RngSeedManager::GetRun ()) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_step (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
}

void
V2xKpi::ComputePsschTbCorruptionStats ()
{
  int rc;
  rc = sqlite3_open (m_dbPath.c_str (), &m_db);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error open DB. Db error: " << sqlite3_errmsg (m_db));

  sqlite3_stmt *stmt;
  std::string sql ("SELECT * FROM psschRxUePhy WHERE SEED = ? AND RUN = ?;");
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error SELECT. Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 1, RngSeedManager::GetSeed ()) == SQLITE_OK);
  NS_ABORT_UNLESS (sqlite3_bind_int (stmt, 2, RngSeedManager::GetRun ()) == SQLITE_OK);
  uint32_t rowCount = 0;
  uint32_t psschSuccessCount = 0;
  uint32_t sci2SuccessCount = 0;


  while ((rc = sqlite3_step (stmt)) == SQLITE_ROW)
    {
      rowCount++;
      uint8_t psschcorrupt = sqlite3_column_int (stmt, 21);
      uint8_t sci2corrupt = sqlite3_column_int (stmt, 23);

      if (!psschcorrupt)
        {
          ++psschSuccessCount;
        }

      if (!sci2corrupt)
        {
          ++sci2SuccessCount;
        }
    }

  NS_ABORT_MSG_UNLESS (rc == SQLITE_DONE, "Error not DONE. Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));


  //NS_LOG_UNCOND ("psschSuccessCount " << psschSuccessCount);
  //NS_LOG_UNCOND ("sci2SuccessCount " << sci2SuccessCount);
  //NS_LOG_UNCOND ("Total rows " << rowCount);
  SavePsschTbCorruptionStats (rowCount, psschSuccessCount, sci2SuccessCount);
}

void
V2xKpi::SavePsschTbCorruptionStats (uint32_t totalTbRx, uint32_t psschSuccessCount, uint32_t sci2SuccessCount)
{
  std::string tableName = "PsschTbRx";
  std::string cmd =  ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "totalRx INTEGER NOT NULL,"
                      "psschSuccessCount INTEGER NOT NULL,"
                      "psschFailCount INTEGER NOT NULL,"
                      "sci2SuccessCount INTEGER NOT NULL,"
                      "sci2FailCount INTEGER NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");
  int rc = sqlite3_exec (m_db, cmd.c_str (), NULL, NULL, NULL);

  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error creating table. Db error: " << sqlite3_errmsg (m_db));

  DeleteWhere (RngSeedManager::GetSeed (), RngSeedManager::GetRun (), tableName);


  sqlite3_stmt *stmt;
  std::string sql = "INSERT INTO " + tableName + " VALUES (?, ?, ?, ?, ?, ?, ?);";
  rc = sqlite3_prepare_v2 (m_db, sql.c_str (), static_cast<int> (sql.size ()), &stmt, nullptr);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK, "Error INSERT. Db error: " << sqlite3_errmsg (m_db));

  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 1, totalTbRx) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 2, psschSuccessCount) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 3, (totalTbRx - psschSuccessCount)) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 4, sci2SuccessCount) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 5, (totalTbRx - sci2SuccessCount)) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 6, RngSeedManager::GetSeed ()) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));
  NS_ABORT_MSG_UNLESS (sqlite3_bind_int (stmt, 7, RngSeedManager::GetRun ()) == SQLITE_OK, "Db error: " << sqlite3_errmsg (m_db));

  rc = sqlite3_step (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly execute the statement. Db error: " << sqlite3_errmsg (m_db));
  rc = sqlite3_finalize (stmt);
  NS_ABORT_MSG_UNLESS (rc == SQLITE_OK || rc == SQLITE_DONE, "Could not correctly finalize the statement. Db error: " << sqlite3_errmsg (m_db));
}


} // namespace ns3

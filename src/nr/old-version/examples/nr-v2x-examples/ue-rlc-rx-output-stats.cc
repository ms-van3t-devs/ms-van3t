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

#include "ue-rlc-rx-output-stats.h"

#include <ns3/core-module.h>

namespace ns3 {

UeRlcRxOutputStats::UeRlcRxOutputStats ()
{
}

void
UeRlcRxOutputStats::SetDb (SQLiteOutput *db, const std::string &tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  ret = db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "timeMs DOUBLE NOT NULL,"
                      "imsi INTEGER NOT NULL,"
                      "rnti INTEGER NOT NULL,"
                      "txRnti INTEGER NOT NULL,"
                      "lcid INTEGER NOT NULL,"
                      "rxPdueSize INTEGER NOT NULL,"
                      "delayMicroSec DOUBLE NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");

  NS_ABORT_UNLESS (ret);

  UeRlcRxOutputStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                       RngSeedManager::GetRun(), tableName);
}

void
UeRlcRxOutputStats::Save (uint64_t imsi, uint16_t rnti,
                          uint16_t txRnti, uint8_t lcid, uint32_t rxPduSize,
                          double delaySeconds)
{
  UeRlcRxData data (Simulator::Now ().GetSeconds () * 1000.0,
                    imsi, rnti, txRnti, lcid, rxPduSize, delaySeconds * 1e6);
  m_rlcRxDataCache.emplace_back (data);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_rlcRxDataCache.size () * sizeof (SlPscchUeMacStatParameters) > 1000000)
    {
      WriteCache ();
    }
}

void
UeRlcRxOutputStats::EmptyCache()
{
  WriteCache ();
}

void
UeRlcRxOutputStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_rlcRxDataCache)
    {
      sqlite3_stmt *stmt;
      m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?);");
      ret = m_db->Bind (stmt, 1, v.timeMs);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 2, static_cast<uint32_t> (v.imsi));
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 3, v.rnti);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 4, v.txRnti);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 5, static_cast<uint16_t> (v.lcid));
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 6, v.rxPduSize);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 7, v.delayMicroSeconds);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 8, RngSeedManager::GetSeed ());
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 9, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      NS_ABORT_UNLESS (ret);

      ret = m_db->SpinExec (stmt);
      NS_ABORT_UNLESS (ret);
    }

  m_rlcRxDataCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ABORT_UNLESS (ret);
}

void
UeRlcRxOutputStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
                                           uint32_t run, const std::string &table)
{
  bool ret;
  sqlite3_stmt *stmt;
  ret = p->SpinPrepare (&stmt, "DELETE FROM \"" + table + "\" WHERE SEED = ? AND RUN = ?;");
  NS_ABORT_IF (ret == false);
  ret = p->Bind (stmt, 1, seed);
  NS_ABORT_IF (ret == false);
  ret = p->Bind (stmt, 2, run);

  ret = p->SpinExec (stmt);
  NS_ABORT_IF (ret == false);
}

} // namespace ns3

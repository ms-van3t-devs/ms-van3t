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
#include "ue-mac-pssch-tx-output-stats.h"

#include <ns3/core-module.h>

namespace ns3 {

UeMacPsschTxOutputStats::UeMacPsschTxOutputStats ()
{
}

void
UeMacPsschTxOutputStats::SetDb (SQLiteOutput *db, const std::string &tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  ret = db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "timeMs DOUBLE NOT NULL,"
                      "imsi INTEGER NOT NULL,"
                      "rnti INTEGER NOT NULL,"
                      "srcL2Id INTEGER NOT NULL,"
                      "dstL2Id INTEGER NOT NULL,"
                      "frame INTEGER NOT NULL,"
                      "subFrame INTEGER NOT NULL,"
                      "slot INTEGER NOT NULL,"
                      "symStart INTEGER NOT NULL,"
                      "symLen INTEGER NOT NULL,"
                      "sbChSize INTEGER NOT NULL,"
                      "rbStart INTEGER NOT NULL,"
                      "rbLen INTEGER NOT NULL,"
                      "harqId INTEGER NOT NULL,"
                      "ndi INTEGER NOT NULL,"
                      "rv INTEGER NOT NULL,"
                      "reselCounter INTEGER NOT NULL,"
                      "cReselCounter INTEGER NOT NULL,"
                      "csiReq INTEGER NOT NULL,"
                      "castType INTEGER NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");

  NS_ABORT_UNLESS (ret);

  UeMacPsschTxOutputStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                        RngSeedManager::GetRun (), tableName);
}

void
UeMacPsschTxOutputStats::Save (const SlPsschUeMacStatParameters psschStatsParams)
{
  m_psschCache.emplace_back (psschStatsParams);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_psschCache.size () * sizeof (SlPsschUeMacStatParameters) > 1000000)
    {
      WriteCache ();
    }
}

void
UeMacPsschTxOutputStats::EmptyCache ()
{
  WriteCache ();
}

void
UeMacPsschTxOutputStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_psschCache)
    {
      sqlite3_stmt *stmt;
      m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
      ret = m_db->Bind (stmt, 1, v.timeMs);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 2, static_cast<uint32_t> (v.imsi));
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 3, v.rnti);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 4, v.srcL2Id);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 5, v.dstL2Id);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 6, v.frameNum);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 7, v.subframeNum);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 8, v.slotNum);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 9, v.symStart);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 10, v.symLength);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 11, v.subChannelSize);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 12, v.rbStart);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 13, v.rbLength);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 14, v.harqId);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 15, v.ndi);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 16, v.rv);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 17, v.resoReselCounter);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 18, v.cReselCounter);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 19, v.csiReq);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 20, v.castType);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 21, RngSeedManager::GetSeed ());
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 22, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      NS_ABORT_UNLESS (ret);

      ret = m_db->SpinExec (stmt);
      NS_ABORT_UNLESS (ret);
    }

  m_psschCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ABORT_UNLESS (ret);
}

void
UeMacPsschTxOutputStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

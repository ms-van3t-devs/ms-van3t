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
#include "ue-phy-pscch-rx-output-stats.h"

#include <ns3/core-module.h>

namespace ns3 {

UePhyPscchRxOutputStats::UePhyPscchRxOutputStats ()
{
}

void
UePhyPscchRxOutputStats::SetDb (SQLiteOutput *db, const std::string &tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  ret = db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                      "timeMs DOUBLE NOT NULL,"
                      "cellId INTEGER NOT NULL,"
                      "rnti INTEGER NOT NULL,"
                      "bwpId INTEGER NOT NULL,"
                      "frame INTEGER NOT NULL,"
                      "subFrame INTEGER NOT NULL,"
                      "slot INTEGER NOT NULL,"
                      "txRnti INTEGER NOT NULL,"
                      "dstL2Id INTEGER NOT NULL,"
                      "pscchRbStart INTEGER NOT NULL,"
                      "pscchRbLen INTEGER NOT NULL,"
                      "pscchMcs INTEGER NOT NULL,"
                      "avrgSinr DOUBLE NOT NULL,"
                      "minSinr DOUBLE NOT NULL,"
                      "tbler INTEGER NOT NULL,"
                      "corrupt INTEGER NOT NULL,"
                      "psschStartSbCh INTEGER NOT NULL,"
                      "psschLenSbCh INTEGER NOT NULL,"
                      "maxNumPerReserve INTEGER NOT NULL,"
                      "rsvpMs INTEGER NOT NULL,"
                      "SEED INTEGER NOT NULL,"
                      "RUN INTEGER NOT NULL"
                      ");");

  NS_ABORT_UNLESS (ret);

  UePhyPscchRxOutputStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                        RngSeedManager::GetRun (), tableName);
}

void
UePhyPscchRxOutputStats::Save (const SlRxCtrlPacketTraceParams pscchStatsParams)
{
  m_pscchCache.emplace_back (pscchStatsParams);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_pscchCache.size () * sizeof (SlRxCtrlPacketTraceParams) > 1000000)
    {
      WriteCache ();
    }
}

void
UePhyPscchRxOutputStats::EmptyCache ()
{
  WriteCache ();
}

void
UePhyPscchRxOutputStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_pscchCache)
    {
      sqlite3_stmt *stmt;
      m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");
      ret = m_db->Bind (stmt, 1, v.m_timeMs);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 2, static_cast<uint32_t> (v.m_cellId));
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 3, v.m_rnti);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 4, v.m_bwpId);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 5, v.m_frameNum);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 6, v.m_subframeNum);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 7, v.m_slotNum);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 8, v.m_txRnti);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 9, v.m_dstL2Id);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 10, v.m_rbStart);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 11, v.m_rbAssignedNum);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 12, v.m_mcs);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 13, v.m_sinr);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 14, v.m_sinrMin);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 15, v.m_tbler);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 16, (v.m_corrupt) ? 1 : 0);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 17, v.m_indexStartSubChannel);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 18, v.m_lengthSubChannel);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 19, v.m_maxNumPerReserve);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 20, v.m_slResourceReservePeriod);
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 21, RngSeedManager::GetSeed ());
      NS_ABORT_UNLESS (ret);
      ret = m_db->Bind (stmt, 22, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      NS_ABORT_UNLESS (ret);

      ret = m_db->SpinExec (stmt);
      NS_ABORT_UNLESS (ret);
    }

  m_pscchCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ABORT_UNLESS (ret);
}

void
UePhyPscchRxOutputStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

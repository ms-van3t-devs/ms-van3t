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
#include "power-output-stats.h"
#include <ns3/rng-seed-manager.h>
#include <ns3/abort.h>

namespace ns3 {

PowerOutputStats::PowerOutputStats ()
{}

void
PowerOutputStats::SetDb (SQLiteOutput *db, const std::string & tableName)
{
  m_db = db;
  m_tableName = tableName;

  bool ret;

  ret = m_db->SpinExec ("CREATE TABLE IF NOT EXISTS " + tableName + " ("
                        "Frame INTEGER NOT NULL, "
                        "SubFrame INTEGER NOT NULL,"
                        "Slot INTEGER NOT NULL,"
                        "Rnti INTEGER NOT NULL,"
                        "Imsi INTEGER NOT NULL,"
                        "BwpId INTEGER NOT NULL,"
                        "CellId INTEGER NOT NULL,"
                        "txPowerRb DOUBLE NOT NULL,"
                        "txPowerTotal DOUBLE NOT NULL,"
                        "rbNumActive INTEGER NOT NULL,"
                        "rbNumTotal INTEGER NOT NULL,"
                        "Seed INTEGER NOT NULL,"
                        "Run INTEGER NOT NULL);");
  NS_ASSERT (ret);

  PowerOutputStats::DeleteWhere (m_db, RngSeedManager::GetSeed (),
                                 RngSeedManager::GetRun (), tableName);
}

void PowerOutputStats::SavePower (const SfnSf &sfnSf, Ptr<const SpectrumValue> txPsd,
                                  [[maybe_unused]] const Time &t, uint16_t rnti, uint64_t imsi,
                                  uint16_t bwpId, uint16_t cellId)
{
  PowerResultCache c;
  c.frame = sfnSf.GetFrame ();
  c.subFrame = sfnSf.GetSubframe ();
  c.slot = sfnSf.GetSlot ();
  c.rnti = rnti;
  c.imsi = imsi;
  c.bwpId = bwpId;
  c.cellId = cellId;

  uint32_t rbNumTotal = txPsd->GetValuesN ();
  uint32_t rbNumActive = 0;

  for (uint32_t rbIndex = 0; rbIndex < rbNumTotal; rbIndex++)
    {
      if ((*txPsd)[rbIndex] != 0)
        {
          rbNumActive++;
        }
    }

  if (rbNumActive == 0)
    {
      return;  //ignore this entry
    }

  c.txPowerTotal = (Integral (*txPsd));
  c.txPowerRb = c.txPowerTotal / rbNumActive;
  c.rbNumActive = rbNumActive;
  c.rbNumTotal = rbNumTotal;

  m_powerCache.emplace_back (c);

  // Let's wait until ~1MB of entries before storing it in the database
  if (m_powerCache.size () * sizeof (PowerResultCache) > 1000000)
    {
      WriteCache ();
    }
}

void
PowerOutputStats::EmptyCache ()
{
  WriteCache ();
}

void
PowerOutputStats::DeleteWhere (SQLiteOutput *p, uint32_t seed,
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

void PowerOutputStats::WriteCache ()
{
  bool ret = m_db->SpinExec ("BEGIN TRANSACTION;");
  for (const auto & v : m_powerCache)
    {
      sqlite3_stmt *stmt;
      ret = m_db->SpinPrepare (&stmt, "INSERT INTO " + m_tableName + " VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);");
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 1, v.frame);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 2, v.subFrame);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 3, v.slot);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 4, v.rnti);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 5, static_cast<uint32_t> (v.imsi));
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 6, v.bwpId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 7, v.cellId);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 8, v.txPowerRb);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 9, v.txPowerTotal);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 10, v.rbNumActive);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 11, v.rbNumTotal);
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 12, RngSeedManager::GetSeed ());
      NS_ASSERT (ret);
      ret = m_db->Bind (stmt, 13, static_cast<uint32_t> (RngSeedManager::GetRun ()));
      NS_ASSERT (ret);

      ret = m_db->SpinExec (stmt);
      NS_ASSERT (ret);
    }
  m_powerCache.clear ();
  ret = m_db->SpinExec ("END TRANSACTION;");
  NS_ASSERT (ret);
}

} // namespace ns3

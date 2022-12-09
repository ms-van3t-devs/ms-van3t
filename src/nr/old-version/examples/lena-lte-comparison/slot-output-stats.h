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
#ifndef SLOT_OUTPUT_STATS_H
#define SLOT_OUTPUT_STATS_H

#include <inttypes.h>
#include <vector>

#include <ns3/sqlite-output.h>
#include <ns3/sfnsf.h>

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
class SlotOutputStats
{
public:
  /**
   * \brief Constructor
   */
  SlotOutputStats ();

  /**
   * \brief Install the output dabase.
   * \param db database pointer
   * \param tableName name of the table where the values will be stored
   *
   *  The db pointer must be valid through all the lifespan of the class. The
   * method creates, if not exists, a table for storing the values. The table
   * will contain the following columns:
   *
   * - "(Frame INTEGER NOT NULL, "
   * - "SubFrame INTEGER NOT NULL,"
   * - "Slot INTEGER NOT NULL,"
   * - "BwpId INTEGER NOT NULL,"
   * - "CellId INTEGER NOT NULL,"
   * - "ActiveUe INTEGER NOT NULL,"
   * - "UsedReg INTEGER NOT NULL,"
   * - "AvailableRb INTEGER NOT NULL,"
   * - "AvailableSym INTEGER NOT NULL,"
   * - "Seed INTEGER NOT NULL,"
   * - "Run INTEGER NOT NULL);"
   *
   * Please note that this method, if the db already contains a table with
   * the same name, also clean existing values that has the same
   * Seed/Run pair.
   */
  void SetDb (SQLiteOutput *db, const std::string& tableName = "slotStats");

  /**
   * \brief Save the slot statistics
   * \param [in] sfnSf Slot number
   * \param [in] scheduledUe The number of scheduled UE in the slot
   * \param [in] usedReg Used Resource Element Group (1 sym x 1 RB)
   * \param [in] usedSym Used symbols
   * \param [in] availableRb Available RBs
   * \param [in] availableSym Available symbols
   * \param [in] bwpId BWP ID
   * \param [in] cellId Cell ID
   */
  void SaveSlotStats (const SfnSf &sfnSf, uint32_t scheduledUe, uint32_t usedReg,
                      uint32_t usedSym, uint32_t availableRb,
                      uint32_t availableSym, uint16_t bwpId,
                      uint16_t cellId);

  /**
   * \brief Force the cache write to disk, emptying the cache itself.
   */
  void EmptyCache ();

private:
  static void
  DeleteWhere (SQLiteOutput *p, uint32_t seed, uint32_t run, const std::string &table);

  void WriteCache ();

  struct SlotCache
  {
    SfnSf sfnSf;
    uint32_t scheduledUe;
    uint32_t usedReg;
    uint32_t usedSym;
    uint32_t availableRb;
    uint32_t availableSym;
    uint16_t bwpId;
    uint16_t cellId;
  };

  SQLiteOutput *m_db;                         //!< DB pointer
  std::vector<SlotCache> m_slotCache;         //!< Result cache
  std::string m_tableName;                    //!< Table name
};

} // namespace ns3

#endif // SLOT_OUTPUT_STATS_H

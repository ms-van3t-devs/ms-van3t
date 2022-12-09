/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
#pragma once

#include <unordered_map>
#include "nr-mac-harq-process.h"

namespace ns3 {

/**
 * \ingroup scheduler
 * \brief Data structure to save all the HARQ process of an UE
 *
 * The data is stored as an unordered_map between the process ID and the
 * real data, saved in the structure HarqProcess. The vector (yes, it is an
 * improper name) is always full (i.e., it always contains almost 20 HARQ processes)
 * but they can be inactive (i.e., no data is stored there). The duty of finding
 * an empty spot is split between Insert and FirstAvailableId.
 *
 * The class does not support going "out of space", or in other words, if all
 * the spots are filled with active processes, the next insert will fail.
 *
 * \see HarqProcess
 */
class NrMacHarqVector : private std::unordered_map<uint8_t, HarqProcess>
{
public:
  friend std::ostream &  operator<< (std::ostream & os, NrMacHarqVector const & item);
  /**
   * \brief iterator of the map
   */
  typedef typename std::unordered_map<uint8_t, HarqProcess>::iterator iterator;
  /**
   * \brief const_iterator of the map
   */
  typedef typename std::unordered_map<uint8_t, HarqProcess>::const_iterator const_iterator;

  /**
    * \brief Default constructor
    */
  NrMacHarqVector () = default;

  /**
   * \brief Set and reserve the size of the vector
   * \param size the vector size
   *
   * The method will reserve and create the necessary processes.
   */
  void SetMaxSize (uint8_t size)
  {
    m_maxSize = size;
    reserve (size);
    for (auto i = 0; i < size; ++i)
      {
        emplace (i, HarqProcess ());
      }
  }

  /**
   * \brief Erase the selected process
   * \param id ID of the process to erase
   * \return true if the process was erased successfully
   */
  bool Erase (uint8_t id);

  /**
   * \brief Insert a process
   * \param id Will be overwritten with process ID
   * \param element process to store
   * \return true if the process was stored successfully
   */
  bool Insert (uint8_t *id, const HarqProcess& element);

  /**
   * \brief Find a process
   * \param key ID of the process to find
   * \return an iterator to the process
   */
  const iterator
  Find (uint8_t key)
  {
    return this->find (key);
  }
  /**
   * \brief Begin of the vector
   * \return an iterator to the first element
   */
  const iterator
  Begin ()
  {
    return this->begin ();
  }
  /**
   * \brief End of the vector
   * \return an iterator to the end() element
   */
  const iterator
  End ()
  {
    return this->end ();
  }
  /**
   * \brief Const begin of the vector
   * \return a const iterator to the first element
   */
  const_iterator
  CBegin ()
  {
    return this->cbegin ();
  }
  /**
   * \brief Const end of the vector
   * \return a const iterator to the end() element
   */
  const_iterator
  CEnd ()
  {
    return this->cend ();
  }
  /**
   * \brief Check if the ID exists in the map
   * \param id ID to check
   * \return true if the ID exists, false if the ID is outside the maximum number
   * of stored elements
   */
  bool Exist (uint8_t id) const
  {
    // could be id > max_size?
    return find (id) != end ();
  }
  /**
   * \brief Get a reference to a process
   * \param id ID of the process
   * \return a reference to the process identified by the parameter id
   */
  HarqProcess & Get (uint8_t id)
  {
    NS_ASSERT (Exist (id));
    return find (id)->second;
  }
  /**
   * \brief Get a const reference to a process
   * \param id ID of the process
   * \return a const reference to the process identified by the parameter id
   */
  const HarqProcess & Get (uint8_t id) const
  {
    NS_ASSERT (Exist (id));
    return find (id)->second;
  }
  /**
   * \brief Find the first (INACTIVE) ID
   * \return an usable ID, or 255 in case no ID are available
   */
  uint8_t FirstAvailableId () const
  {
    for (const auto & it : *this)
      {
        if (!it.second.m_active)
          {
            return it.first;
          }
      }
    return 255;
  }
  /**
   * \brief Can an ID be inserted?
   * \return true if there is space to insert a new process, false otherwise
   */
  bool CanInsert () const
  {
    return Size () < m_maxSize;
  }
  /**
   * \brief Get the used size of the vector
   * \return the real size occupied by ACTIVE processes.
   */
  uint32_t Size () const
  {
    return m_usedSize;
  }

private:
  uint8_t m_maxSize  {0}; //!< Maximum size (or the number of processes stored)
  uint8_t m_usedSize {0}; //!< Number of ACTIVE processes
};

/**
 * \brief Print HarqProcess onto a ostream
 * \param os Ostream output
 * \param item Item to print
 * \return the Ostream for concatenation
 */
std::ostream & operator<< (std::ostream & os, HarqProcess const & item);

} // namespace ns3

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

#ifndef NR_SL_SCI_F2_HEADER_H
#define NR_SL_SCI_F2_HEADER_H

#include "ns3/header.h"
#include <iostream>

namespace ns3 {

/**
 * \ingroup nr
 * \brief The packet header for the NR Sidelink Control Information (SCI)
 * format 02 (TS 38.212 Sec 8.3 Rel 16). The following fields must be set
 * before adding this header to a packet.
 *
 * - m_harqId [4 bits]
 * - m_ndi [1 bit]
 * - m_rv [2 bits]
 * - m_srcId [8 bits]
 * - m_dstId [16 bits]
 *
 * Following is the field which is not mandatory to be set since, the
 * code is not yet ready to use it.
 *
 * - m_harqFbIndicator [1 bit]
 *
 * The total size of this header is 4 bytes, including the above one
 * non mandatory field.
 *
 * The use of this header is only possible if:
 * - All the mandatory fields are set using their respective setter methods.
 *   Otherwise, serialization will hit an assert.
 */
class NrSlSciF2Header : public Header
{
public:
  /**
   * \brief Constructor
   *
   * Creates an SCI header
   */
  NrSlSciF2Header ();
  ~NrSlSciF2Header ();

  /**
   * \brief Set the HARQ process id field
   *
   * \param harqId The HARQ process id
   */
  void SetHarqId (uint8_t harqId);
  /**
   * \brief Set the new data indicator field
   *
   * \param ndi The new data indicator
   */
  void SetNdi (uint8_t ndi);
  /**
   * \brief Set the redundancy version
   *
   * \param rv The redundancy version
   */
  void SetRv (uint8_t rv);
  /**
   * \brief Set the layer 2 source id
   *
   * \param srcId The layer 2 source id
   */
  void SetSrcId (uint32_t srcId);
  /**
   * \brief Set the layer 2 destination id
   *
   * \param dstId The layer 2 destination id
   */
  void SetDstId (uint32_t dstId);
  /**
   * \brief Set the HARQ feedback indicator
   *
   * \param harqFb The HARQ feedback enabled/disabled indicator
   */
  void SetHarqFbIndicator (uint8_t harqFb);

  /**
   * \brief Get the HARQ process id
   *
   * \return The HARQ process id
   */
  uint8_t GetHarqId () const;
  /**
   * \brief Get the new data indicator field value
   *
   * \return The new data indicator field value
   */
  uint8_t GetNdi () const;
  /**
   * \brief Get the Redundancy version
   *
   * \return The Redundancy version
   */
  uint8_t GetRv () const;
  /**
   * \brief Get the source layer 2 id
   *
   * \return The source layer 2 id
   */
  uint8_t GetSrcId () const;
  /**
   * \brief Get the destination layer 2 id
   *
   * \return The destination layer 2 id
   */
  uint16_t GetDstId () const;
  /**
   * \brief Get the HARQ feedback indicator
   *
   * \return The HARQ feedback enabled/disabled indicator
   */
  uint8_t GetHarqFbIndicator () const;


  /**
   * \brief Ensure that mandatory fields are configured
   *
   * All the mandatory fields are initialized by default with
   * an invalid value. Therefore, if a mandatory field value is
   * different than this invalid value, we consider it set.
   *
   * \return True if all the mandatory fields are set, false otherwise
   */
  bool EnsureMandConfig () const;


  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

protected:
  /**
   * \brief Serialize the common fields of second stage SCI
   * \param i the buffer iterator
   */
  void PreSerialize (Buffer::Iterator &i) const;
  /**
   * \brief Deserialize the common fields of second stage SCI
   * \param i the buffer iterator
   * \return The serialized size of the header
   */
  uint32_t PreDeserialize (Buffer::Iterator &i);

  //Mandatory SCI format 02 fields
  uint8_t m_harqId {std::numeric_limits <uint8_t>::max ()}; //!< The HARQ process id
  uint8_t m_ndi {std::numeric_limits <uint8_t>::max ()}; //!< The new data indicator
  uint8_t m_rv {std::numeric_limits <uint8_t>::max ()}; //!< The redundancy version
  uint32_t m_srcId {std::numeric_limits <uint32_t>::max ()}; //!< The source layer 2 id
  uint32_t m_dstId {std::numeric_limits <uint32_t>::max ()}; //!< The destination layer 2 id
  //SCI fields end

  //fields which are not used yet, therefore, it is not mandatory to set them
  uint32_t m_harqFbIndicator {0}; //!< HARQ feedback enabled/disabled indicator
};


} // namespace ns3

#endif // NR_SL_SCI_F2_HEADER_H

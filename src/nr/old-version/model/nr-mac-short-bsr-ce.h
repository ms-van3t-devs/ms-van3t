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

#ifndef NR_MAC_SHORT_BSR_CE_H
#define NR_MAC_SHORT_BSR_CE_H

#include "ns3/packet.h"
#include "nr-mac-header-fs-ul.h"

namespace ns3 {

/**
 * \ingroup ue-mac
 * \ingroup gnb-mac
 * \brief BSR control element
 *
 * This is the short BSR control element, meant to be written after a subHeader,
 * within a NR subPDU. It always be fixed size of 4 byte. Please use the
 * conversion function to write or read the buffer level.
 *
 * The serialization looks like the following:
 *
 * \verbatim
 +-----+-----+-----+-------------------------------------------+
 |                 |                                           |
 |    LCG ID       |         Buffer Level                      |   Oct 1
 |                 |                                           |
 +-----+-----+-----+-------------------------------------------+
 |                 |                                           |
 |    LCG ID       |         Buffer Level                      |   Oct 2
 |                 |                                           |
 +-----+-----+-----+-------------------------------------------+
 |                 |                                           |
 |    LCG ID       |         Buffer Level                      |   Oct 3
 |                 |                                           |
 +-----+-----+-----+-------------------------------------------+
 |                 |                                           |
 |    LCG ID       |         Buffer Level                      |   Oct 4
 |                 |                                           |
 +-----+-----+-----+-------------------------------------------+
\endverbatim
 *
 * It is a mixed version of what LENA uses, and what the standard says. We call
 * it short bsr, but in reality the standard defines a short bsr with only 1 byte,
 * to report only one LCG. Here, we save four bytes, and the position tells us
 * to what LCG it pertains (hence, on the serialization, the LCG ID will
 * always be 0).
 *
 * We have done this mix to avoid touching too deeply the FF API for the scheduler.
 * In the future, we may improve it.
 *
 * Please refer to TS 38.321 section 6.1.2 for more information.
 */
class NrMacShortBsrCe : public Header
{
public:
  /**
   * \brief GetTypeId
   * \return the type id of the object
   */
  static TypeId  GetTypeId (void);
  /**
   * \brief GetInstanceTypeId
   * \return the instance type id
   */
  virtual TypeId  GetInstanceTypeId (void) const;

  /**
   * \brief NrMacShortBsrCe constructor
   */
  NrMacShortBsrCe ();

  /**
   * \brief Serialize on a buffer
   * \param start start position
   */
  void Serialize (Buffer::Iterator start) const;
  /**
   * \brief Deserialize from a buffer
   * \param start start position
   * \return the number of bytes read from the buffer
   */
  uint32_t Deserialize (Buffer::Iterator start);
  /**
   * \brief Get the serialized size
   * \return 1
   */
  uint32_t GetSerializedSize () const;
  /**
   * \brief Print the struct on a ostream
   * \param os ostream
   */
  void Print (std::ostream &os) const;

  /**
   * \brief IsEqual
   * \param o another instance
   * \return true if this and o are equal, false otherwise
   */
  bool operator == (const NrMacShortBsrCe &o) const;

  /**
   * \brief Convert a bytes value into the 3GPP-standard level to write in the BSR
   * \param bufferSize The buffer size
   * \return a number between 0 and 31 that represents the buffer level as specified in the standard
   */
  static uint8_t FromBytesToLevel (uint64_t bufferSize);

  /**
   * \brief Convert a buffer level into a buffer size
   * \param bufferLevel The buffer level
   * \return the buffer size
   */
  static uint64_t FromLevelToBytes (uint8_t bufferLevel);

  uint8_t m_bufferSizeLevel_0 {0}; //!< Buffer size level for LCG 0 (maximum value: 31)
  uint8_t m_bufferSizeLevel_1 {0}; //!< Buffer size level for LCG 1 (maximum value: 31)
  uint8_t m_bufferSizeLevel_2 {0}; //!< Buffer size level for LCG 2 (maximum value: 31)
  uint8_t m_bufferSizeLevel_3 {0}; //!< Buffer size level for LCG 3 (maximum value: 31)

private:
  NrMacHeaderFsUl m_header; //!< Fixed-size header to prepend to the BSR
};

} //namespace ns3

#endif /* NR_MAC_SHORT_BSR_CE_H */

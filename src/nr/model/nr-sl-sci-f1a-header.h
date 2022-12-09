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

#ifndef NR_SL_SCI_F1A_HEADER_H
#define NR_SL_SCI_F1A_HEADER_H

#include "ns3/header.h"
#include <iostream>

namespace ns3 {

/**
 * \ingroup nr
 * \brief The packet header for the NR Sidelink Control Information (SCI)
 * format 1A. It is not a standard compliment header, therefore, its size (in bytes)
 * is also different. This header includes only one field, i.e.,
 * m_lengthSubChannel, to indicate the total subchannel assigned to the
 * first transmission and the possible retransmissions. This is because for
 * the SPS type scheduling a fixed/same MCS is used to create the allocation for
 * the original transmission and retransmission(s) of a TB; therefore, the
 * same number of subchannels are used for each transmission of the TB. Finally,
 * the following fields must be set before adding this header to a packet.
 *
 * - m_priority [1 byte, always present]
 * - m_mcs [1 byte, always present]
 * - m_slSciStage2Format [1 byte, always present]
 * - m_slResourceReservePeriod [2 byte, always present]
 * - m_totalSubChannels [2 bytes, always present]
 * - m_IndexStartSubChannel [1 byte, always present]
 * - m_lengthSubChannel [1 byte, always present]
 * - m_slMaxNumPerReserve [1 byte, always present]
 *
 * 10 bytes always present.
 *
 * Optional fields:
 * - m_gapReTx1 = [1 byte if slMaxNumPerReserve == 2]
 * - m_indexStartSbChReTx1 = [1 byte if slMaxNumPerReserve == 2]
 * - m_gapReTx2 = [1 byte if slMaxNumPerReserve == 3]
 * - m_indexStartSbChReTx2 = [1 byte if slMaxNumPerReserve == 3]
 *
 * The use of this header is only possible if:
 * - All the mandatory fields are set using their respective setter methods;
 *   otherwise, serialization will hit an assert.
 */
class NrSlSciF1aHeader : public Header
{
public:
  /**
   * \brief Constructor
   *
   * Creates an SCI header
   */
  NrSlSciF1aHeader ();
  ~NrSlSciF1aHeader ();

  /// SCI Stage 2 format enumeration
  enum SciStage2Format_t : uint8_t
  {
    SciFormat2A = 0,
    SciFormat2B = 1,
  };

  /**
   * \brief Set the priority
   *
   * \param priority The priority of the packet
   */
  void SetPriority (uint8_t priority);
  /**
   * \brief Set the total number of sub-channels
   *
   * \param totalSubChannels The total number of sub-channels
   */
  void SetTotalSubChannels (uint16_t totalSubChannels);
  /**
   * \brief Set the index of the starting sub-channel allocated
   *
   * \param IndexStartSubChannel The index of the starting sub-channel
   */
  void SetIndexStartSubChannel (uint8_t IndexStartSubChannel);
  /**
   * \brief Set the total number of sub-channels allocated
   *
   * \param lengthSubChannel The total number of sub-channels allocated
   */
  void SetLengthSubChannel (uint8_t lengthSubChannel);
  /**
   * \brief Set the Sidelink resource reservation period
   *
   * \param slResourceReservePeriod The Resource reservation period in milliseconds
   */
  void SetSlResourceReservePeriod (uint16_t slResourceReservePeriod);
  /**
   * \brief Set the MCS
   * \param mcs The MCS
   */
  void SetMcs (uint8_t mcs);
  /**
   * \brief Set Sidelink maximum number of reservations per transmission
   *
   * This value is a maximum number of reserved
   * PSCCH/PSSCH resources that can be indicated by an SCI. For one ReTx, it
   * should be set to 2, which indicates the reservation of resources for
   * the initial transmission and the first re-transmission. Similarly, for
   * two re-transmissions it should be set to 3.
   *
   * \param slMaxNumPerReserve
   */
  void SetSlMaxNumPerReserve (uint8_t slMaxNumPerReserve);
  /**
   * \brief Set the gap between a transmission and its first re-transmission
   *
   * \param gapReTx1 The gap between a transmission and its first re-transmission in slots
   */
  void SetGapReTx1 (uint8_t gapReTx1);
  /**
   * \brief Set the gap between a transmission and its second re-transmission
   *
   * \param gapReTx2 The gap between a transmission and its second re-transmission in slots
   */
  void SetGapReTx2 (uint8_t gapReTx2);
  /**
   * \brief Set the second stage SCI format value.
   *
   * We only support SCI format 2A
   *
   * TS 38.212 Table 8.3.1.1-1
   * SCI format 2A = 0
   * SCI format 2B = 1
   * Reserved = 2
   * Reserved = 3
   *
   * \param formatValue the second stage SCI format value
   */
  void SetSciStage2Format (uint8_t formatValue);
  /**
   * \brief Set the start sub-channel index of the first re-transmission
   *
   * \param sbChIndexReTx1 The start sub-channel index of the first re-transmission
   */
  void SetIndexStartSbChReTx1 (uint8_t sbChIndexReTx1);
  /**
   * \brief Set the start sub-channel index of the second re-transmission
   *
   * \param sbChIndexReTx2 The start sub-channel index of the second re-transmission
   */
  void SetIndexStartSbChReTx2 (uint8_t sbChIndexReTx2);


  /**
   * \brief Get the priority
   *
   * \return The priority
   */
  uint8_t GetPriority () const;
  /**
   * \brief Get the total number of sub-channels
   *
   * \return The total number of sub-channels
   */
  uint16_t GetTotalSubChannels () const;
  /**
   * \brief Get the index of the starting sub-channel allocated
   *
   * \return The index of the starting sub-channel
   */
  uint8_t GetIndexStartSubChannel () const;
  /**
   * \brief Get the total number of sub-channels allocated
   *
   * \return The total number of sub-channels allocated
   */
  uint8_t GetLengthSubChannel () const;
  /**
   * \brief Get the Sidelink resource reservation period
   *
   * \return The Resource reservation period in milliseconds
   */
  uint16_t GetSlResourceReservePeriod () const;
  /**
   * \brief Get the MCS
   *
   * \return The MCS
   */
  uint8_t GetMcs () const;
  /**
   * \brief Get Sidelink maximum number of reservations per transmission
   *
   * This value returned by the method is a maximum number of reserved
   * PSCCH/PSSCH resources that can be indicated by an SCI.
   *
   * \return slMaxNumPerReserve
   */
  uint8_t GetSlMaxNumPerReserve () const;
  /**
   * \brief Get the gap between a transmission and its first re-transmission
   *
   * \return The gap between a transmission and its first re-transmission in slots
   */
  uint8_t GetGapReTx1 () const;
  /**
   * \brief Get the gap between a transmission and its second re-transmission
   *
   * \return The gap between a transmission and its second re-transmission in slots
   */
  uint8_t GetGapReTx2 () const;
  /**
   * \brief Get the second stage SCI format value.
   *
   * TS 38.212 Table 8.3.1.1-1
   * SCI format 2A = 0
   * SCI format 2B = 1
   * Reserved = 2
   * Reserved = 3
   *
   * \return the second stage SCI format value
   */
  uint8_t GetSciStage2Format () const;
  /**
   * \brief Get the start sub-channel index of the first re-transmission
   *
   * \return The start sub-channel index of the first re-transmission
   */
  uint8_t GetIndexStartSbChReTx1 () const;
  /**
   * \brief Get the start sub-channel index of the second re-transmission
   *
   * \return The start sub-channel index of the second re-transmission
   */
  uint8_t GetIndexStartSbChReTx2 () const;


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
   * \brief Equality operator
   *
   * \param [in] b the NrSlSciF1aHeader to compare
   * \returns \c true if \pname{b} is equal
   */
  bool operator == (const NrSlSciF1aHeader &b) const;



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

private:
  //Mandatory fields including the SCI fields
  uint16_t m_totalSubChannels {std::numeric_limits <uint16_t>::max ()}; //!< The total number of sub-channels
  //SCI fields
  uint8_t m_priority {std::numeric_limits <uint8_t>::max ()}; //!< The priority
  uint8_t m_indexStartSubChannel {std::numeric_limits <uint8_t>::max ()}; //!< The index of the starting sub-channel allocated
  uint8_t m_lengthSubChannel {std::numeric_limits <uint8_t>::max ()}; //!< The total number of the sub-channel allocated
  uint8_t m_mcs {std::numeric_limits <uint8_t>::max ()}; //!< The Modulation and Coding Scheme (MCS)
  uint16_t m_slResourceReservePeriod {std::numeric_limits <uint16_t>::max ()}; //!< Resource reservation period
  uint8_t m_slMaxNumPerReserve {std::numeric_limits <uint8_t>::max ()}; //!< maximum number of reserved resources
  uint8_t m_slSciStage2Format {std::numeric_limits <uint8_t>::max ()}; //!< maximum number of reserved resources
  //SCI fields end

  //Optional fields
  uint8_t m_indexStartSbChReTx1 {std::numeric_limits <uint8_t>::max ()}; //!< The index of the starting sub-channel allocated to first retransmission
  uint8_t m_indexStartSbChReTx2 {std::numeric_limits <uint8_t>::max ()}; //!< The index of the starting sub-channel allocated to second retransmission
  uint8_t m_gapReTx1 {std::numeric_limits <uint8_t>::max ()}; //!< The gap between a transmission and its first retransmission in slots
  uint8_t m_gapReTx2 {std::numeric_limits <uint8_t>::max ()}; //!< The gap between a transmission and its second retransmission in slots
  static std::vector<SciStage2Format_t> m_allowedSciStage2Format; //!< Vector of allowed Stage 2 SCI formats, to speed up checking
};


} // namespace ns3

#endif // NR_SL_SCI_F1A_HEADER_H

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

#ifndef NR_MAC_HEADER_VS_H
#define NR_MAC_HEADER_VS_H

#include "ns3/packet.h"

namespace ns3 {

/**
 * \ingroup ue-mac
 * \ingroup gnb-mac
 *
 * \brief Mac variable-size Header
 *
 * This standard-compliant variable-size subheader can be 16 or 24 bits. The final
 * length depends on the size of the part that follows:
 *
 * \verbatim
 +-----+-----+-------------------------------------------------+
 |     |     |                                                 |
 |  0  |  0  |                     m_lcId                      |   Oct 1
 |     |     |                                                 |
 +-----+-----+-------------------------------------------------+
 |                                                             |
 |                  m_size (lower 8bit)                        |   Oct 2
 |                                                             |
 +-------------------------------------------------------------+
 |                                                             |
 |                  m_size (higher 8bit)                       |   Oct 3 (optional)
 |                                                             |
 +-------------------------------------------------------------+
\endverbatim
 *
 * Do not confuse m_size with the serialized size that is obtained through
 * GetSerializedSize(). The former is, technically, the size of the block
 * that follows this header, while the serialized size can be 2 or 3 bytes,
 * depending on the m_size value. The standard supports m_size that can take
 * up to the UINT16T_MAX value.
 *
 * Please refer to TS 38.321 section 6.1.2 for more information.
 *
 * <b>Users, don't use this header directly: you've been warned.</b>
 *
 * \internal
 *
 * Using this header requires to know the size of the data that will follow
 * this header, and then to set the LCID (that can be from 1 to 32). This
 * header can be used <b>only</b> to indicate that, following this header,
 * there will be a data SDU. If you need to indicate that there is a variable-sized
 * CE, you need to use one of the subclasses NrMacHeaderVsUl or NrMacHeaderVsDl,
 * respectively for UL (from UE to GNB) or DL (from GNB to UE).
 */
class NrMacHeaderVs : public Header
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
   * \brief NrMacHeaderVs constructor
   */
  NrMacHeaderVs ();

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
   * \return if the size can be represented with 1 byte, then the S.S. is 2,
   * otherwise 3
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
  bool operator == (const NrMacHeaderVs &o) const;

  static const uint8_t LC_ID_1  = 1; //!< Identity of the logical channel 1
  static const uint8_t LC_ID_2  = 2; //!< Identity of the logical channel 2
  static const uint8_t LC_ID_3  = 3; //!< Identity of the logical channel 3
  static const uint8_t LC_ID_4  = 4; //!< Identity of the logical channel 4
  static const uint8_t LC_ID_5  = 5; //!< Identity of the logical channel 5
  static const uint8_t LC_ID_6  = 6; //!< Identity of the logical channel 6
  static const uint8_t LC_ID_7  = 7; //!< Identity of the logical channel 7
  static const uint8_t LC_ID_8  = 8; //!< Identity of the logical channel 8
  static const uint8_t LC_ID_9  = 9; //!< Identity of the logical channel 9
  static const uint8_t LC_ID_10 = 10; //!< Identity of the logical channel 10
  static const uint8_t LC_ID_11 = 11; //!< Identity of the logical channel 11
  static const uint8_t LC_ID_12 = 12; //!< Identity of the logical channel 12
  static const uint8_t LC_ID_13 = 13; //!< Identity of the logical channel 13
  static const uint8_t LC_ID_14 = 14; //!< Identity of the logical channel 14
  static const uint8_t LC_ID_15 = 15; //!< Identity of the logical channel 15
  static const uint8_t LC_ID_16 = 16; //!< Identity of the logical channel 16
  static const uint8_t LC_ID_17 = 17; //!< Identity of the logical channel 17
  static const uint8_t LC_ID_18 = 18; //!< Identity of the logical channel 18
  static const uint8_t LC_ID_19 = 19; //!< Identity of the logical channel 19
  static const uint8_t LC_ID_20 = 20; //!< Identity of the logical channel 20
  static const uint8_t LC_ID_21 = 21; //!< Identity of the logical channel 21
  static const uint8_t LC_ID_22 = 22; //!< Identity of the logical channel 22
  static const uint8_t LC_ID_23 = 23; //!< Identity of the logical channel 23
  static const uint8_t LC_ID_24 = 24; //!< Identity of the logical channel 24
  static const uint8_t LC_ID_25 = 25; //!< Identity of the logical channel 25
  static const uint8_t LC_ID_26 = 26; //!< Identity of the logical channel 26
  static const uint8_t LC_ID_27 = 27; //!< Identity of the logical channel 27
  static const uint8_t LC_ID_28 = 28; //!< Identity of the logical channel 28
  static const uint8_t LC_ID_29 = 29; //!< Identity of the logical channel 29
  static const uint8_t LC_ID_30 = 30; //!< Identity of the logical channel 30
  static const uint8_t LC_ID_31 = 31; //!< Identity of the logical channel 31
  static const uint8_t LC_ID_32 = 32; //!< Identity of the logical channel 32

  /**
   * \brief Set the LC ID
   * \param lcId LC ID
   *
   * It will assert if the value is not inside the vector of allowed one.
   * To not make any error, please use one of the pre-defined const values in
   * this class.
   */
  virtual void SetLcId (uint8_t lcId);

  /**
   * \brief Retrieve the LC ID of this header
   * \return the LC ID
   */
  uint8_t GetLcId () const;

  /**
   * \brief Set the size to store (L in the standard)
   * \param size the size to store (L in the standard)
   */
  void SetSize (uint16_t size);

  /**
   * \brief GetS the stored size (L in the standard)
   * \return the stored size
   */
  uint16_t GetSize () const;

protected:
  uint8_t   m_lcid {0}; //!< LC ID
  uint16_t  m_size {0}; //!< Size (L in the standard)

private:
  static std::vector<uint8_t> m_allowedLcId; //!< Vector of allowed LCID, to speed up checking
};

} //namespace ns3

#endif /* NR_MAC_HEADER_VS_H */

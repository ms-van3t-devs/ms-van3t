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

#ifndef SFNSF_H
#define SFNSF_H

#include <ns3/simple-ref-count.h>

namespace ns3 {

/**
 * \brief The SfnSf class
 * \ingroup utils
 *
 * Keep track of the frame, subframe, slot number. The value can be normalized
 * in the number of slot. Please keep in mind that a SfnSf is valid only when
 * there is an associated numerology, and that a particular value is not
 * normalized the same in two different numerologies.
 *
 * \section sfnsf_usage Usage
 *
 * To create a SfnSf with numerology 2 (for example), please do:
 * \verbatim
 auto sfn = SfnSf (1, 0, 0, 2);
 \endverbatim
 *
 * \see Normalize
 */
class SfnSf : public SimpleRefCount<SfnSf>
{
public:

  /**
    * \brief constructor
    */
  SfnSf () = default;

  /**
   * \brief SfnSf constructor
   * \param frameNum Frame number
   * \param sfNum Subframe Number
   * \param slotNum Slot number
   * \param numerology Numerology (will be always associated with this SfnSf)
   */
  SfnSf (uint16_t frameNum, uint8_t sfNum, uint16_t slotNum, uint8_t numerology);

  /**
   * \brief Get enconding for this SfnSf
   * \return an uint64_t that is able to represent fully this SfnSf
   * \see FromEncoding
   */
  uint64_t GetEncoding () const;
  /**
   * \brief Get the enconding number, including a symbol start value
   * \param symStart the symbol start value to include
   * \return an uint64_t that can represent this SfnSf plus 2 bytes to represent the
   * symbol start
   */
  uint64_t GetEncodingWithSymStart (uint8_t symStart) const;

  /**
   * \brief Fill the private fields with the value extracted from the parameter
   * \param sfn Encoding from which extract the value
   *
   * \see GetEncoding
   */
  void FromEncoding (uint64_t sfn);

  /**
   * \brief Encode the parameter in a uint64_t
   * \param p the SfnSf to encode
   * \return an uint64_t that can represent this SfnSf
   * \see Decode
   */
  static uint64_t Encode (const SfnSf &p);
  /**
   * \brief Decode the parameter and return a SfnSf
   * \param sfn the encoded value
   * \return an object which contains the values extracted from the encoding
   */
  static SfnSf Decode (uint64_t sfn);

  /**
   * \return the number of subframes per frame, 10
   */
  static uint32_t GetSubframesPerFrame ();

  /**
   * \brief Get SlotPerSubframe
   * \return the number of slot per subframe; depends on the numerology installed
   */
  uint32_t GetSlotPerSubframe () const;


  /**
   * \brief Normalize the SfnSf in slot number
   * \return The number of total slots passed (can overlap)
   */
  uint64_t Normalize () const;

  /**
   * \brief Add to this SfnSf a number of slot indicated by the first parameter
   * \param slotN Number of slot to add
   */
  void Add (uint32_t slotN);

  /**
   * \brief Get a Future SfnSf
   * \param slotN slot to sum
   * \return the SfnSf that results from the operation (*this) + slotN
   */
  SfnSf GetFutureSfnSf (uint32_t slotN) const;

  /**
   * \brief operator < (less than)
   * \param rhs other SfnSf to compare
   * \return true if this SfnSf is less than rhs
   *
   * The comparison is done on m_frameNum, m_subframeNum, and m_slotNum: not on varTti
   */
  bool operator < (const SfnSf& rhs) const;

  /**
   * \brief operator ==, compares only frame, subframe, and slot
   * \param o other instance to compare
   * \return true if this instance and the other have the same frame, subframe, slot
   *
   * Used in the MAC operation, in which the varTti is not used (or, it is
   * its duty to fill it).
   *
   * To check the varTti, please use this operator and IsTtiEqual()
   */
  bool operator == (const SfnSf &o) const;

  /**
   * \brief GetFrame
   * \return the frame number
   */
  uint16_t GetFrame () const;
  /**
   * \brief GetSubframe
   * \return the subframe number
   */
  uint8_t GetSubframe () const;
  /**
   * \brief GetSlot
   * \return the slot number
   */
  uint16_t GetSlot () const;
  /**
   * \brief GetNumerology
   * \return the numerology installed
   *
   * Please note that if you invoke this method without passing a numerology
   * to the constructor or without constructing the object from an encoded
   * value, the program will fail.
   */
  uint16_t GetNumerology () const;


private:
  uint16_t m_frameNum   { 0 };  //!< Frame Number
  uint8_t m_subframeNum { 0 };  //!< SubFrame Number
  uint16_t m_slotNum    { 0 };  //!< Slot number (a slot is made by 14 symbols)
  int16_t m_numerology  {-1 };  //!< Slot per subframe: 2^{numerology}
};

} // namespace ns3
#endif // SFNSF_H

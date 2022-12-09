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

#ifndef NR_SL_SCI_F2A_HEADER_H
#define NR_SL_SCI_F2A_HEADER_H

#include "nr-sl-sci-f2-header.h"

#include <iostream>

namespace ns3 {

/**
 * \ingroup nr
 * \brief The packet header for the NR Sidelink Control Information (SCI)
 * format 2A (TS 38.212 Sec 8.3 Rel 16). The following includes two fields.
 *
 * - m_castTypeIndicator [2 bits]
 * - m_csiReq [1 bit]
 *
 * The total size of this header is 5 bytes. 4 bytes of \link NrSlSciF2aHeader \endlink
 * , 3 bits of above fields plus 5 bits of zero padding.
 *
 * The use of this header is only possible if:
 * - All the mandatory fields in \link NrSlSciF2aHeader \endlink are set using
 *   their respective setter methods. Otherwise, serialization will hit an assert.
 *
 * \see NrSlSciF2Header
 */
class NrSlSciF2aHeader : public NrSlSciF2Header
{
public:
  /**
   * \brief Constructor
   *
   * Creates an SCI header
   */
  NrSlSciF2aHeader ();
  ~NrSlSciF2aHeader ();

  /// Cast type indicator enumeration
  enum CastTypeIndicator_t : uint8_t
  {
    Broadcast         = 0,
    Groupcast         = 1,
    Unicast           = 2,
    GroupcastOnlyNack = 3,
  };

  /**
   * \brief Set the cast type indicator field
   *
   * \param castType The cast type indicator value
   */
  void SetCastType (uint8_t castType);
  /**
   * \brief Set the Channel State Information request flag
   *
   * \param csiReq The channel state information request flag
   */
  void SetCsiReq (uint8_t csiReq);

  /**
   * \brief Get the cast type indicator field value
   *
   * \return The cast type indicator value
   */
  uint8_t GetCastType () const;
  /**
   * \brief Get the Channel State Information request flag
   *
   * \return The channel state information request flag
   */
  uint8_t GetCsiReq () const;

  /**
   * \brief Equality operator
   *
   * \param [in] b the NrSlSciF2aHeader to compare
   * \returns \c true if \pname{b} is equal
   */
  bool operator == (const NrSlSciF2aHeader &b) const;

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

  uint8_t m_castType {Broadcast}; //!< The type of communication this NrSlSciF2aHeader is used for
  uint8_t m_csiReq {0}; //!< The channel state information request flag


  static std::vector<CastTypeIndicator_t> m_allowedCastType; //!< Vector of allowed Stage 2 SCI formats, to speed up checking
};


} // namespace ns3

#endif // NR_SL_SCI_F2A_HEADER_H

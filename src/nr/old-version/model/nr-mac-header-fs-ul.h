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

#ifndef NR_MAC_HEADER_FS_UL_H
#define NR_MAC_HEADER_FS_UL_H

#include "nr-mac-header-fs.h"

namespace ns3 {

/**
 * \ingroup ue-mac
 * \ingroup gnb-mac
 * \brief Mac fixed-size Header for UL
 *
 * This header performs some sanity check for the LCID value, but the functionality
 * is almost the same as NrMacHeaderFs. Please note that, by standard, only
 * some LCID can be used in UL transmissions.
 *
 * Please refer to TS 38.321 section 6.1.2 for more information.
 *
 * <b>Users, don't use this header directly: you've been warned.</b>
 *
 * \internal
 *
 * This header must be used to report some fixed-sized CE to the UE. An
 * example is NrMacShortBsrCe.
 */
class NrMacHeaderFsUl : public NrMacHeaderFs
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
  virtual TypeId  GetInstanceTypeId (void) const override;

  /**
   * \brief NrMacHeaderFsUl constructor
   */
  NrMacHeaderFsUl ();

  /**
   * \brief ~NrMacHeaderFsUl
   */
  virtual ~NrMacHeaderFsUl ();


  // CCCH_LARGE  = 0, //!< CCCH of size 64 bit   (is it fixed or variable?)
  // CCCH_SMALL = 52, //!< CCCH of size 48  (is it fixed or variable?)
  static const uint8_t BIT_RATE_QUERY = 53;                  //!< Recommended bit rate query
  static const uint8_t CONFIGURED_GRANT_CONFIRMATION = 55;   //!< Configured Grant Confirmation
  static const uint8_t SINGLE_ENTRY_PHR = 57;                //!< Single entry PHR
  static const uint8_t C_RNTI = 58;                          //!< C-RNTI
  static const uint8_t SHORT_TRUNCATED_BSR = 59;             //!< Short Truncated BSR
  static const uint8_t SHORT_BSR = 61;                       //!< Short BSR

  /**
   * \brief Set the LC ID
   * \param lcId LC ID
   *
   * It will assert if the value is not inside the vector of allowed one.
   * To not make any error, please use one of the pre-defined const values in
   * this class.
   */
  virtual void SetLcId (uint8_t lcId) override;

  /**
   * \brief Check if it really a fixed-size header
   * \return true if the lcId value stored internally matches with a fixed-size header
   */
  bool IsFixedSizeHeader () const;
};

} //namespace ns3

#endif /* NR_MAC_HEADER_FS_UL_H */

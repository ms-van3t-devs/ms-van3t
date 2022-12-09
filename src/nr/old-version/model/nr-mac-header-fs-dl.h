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

#ifndef NR_MAC_HEADER_FS_DL_H
#define NR_MAC_HEADER_FS_DL_H

#include "nr-mac-header-fs.h"

namespace ns3 {

/**
 * \ingroup ue-mac
 * \ingroup gnb-mac
 * \brief Mac fixed-size Header for DL
 *
 * This header performs some sanity check for the LCID value, but the functionality
 * is almost the same as NrMacHeaderFs. Please note that, by standard, only
 * some LCID values can be used in DL transmissions.
 *
 * Please refer to TS 38.321 section 6.1.2 for more information.
 *
 * <b>Users, don't use this header directly: you've been warned.</b>
 *
 * \internal
 *
 * This header must be used to report some fixed-sized CE to the UE. At
 * the moment, we don't use it.
 */
class NrMacHeaderFsDl : public NrMacHeaderFs
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
   * \brief NrMacHeaderFsDl constructor
   */
  NrMacHeaderFsDl ();

  /**
   * \brief ~NrMacHeaderFsDl
   */
  virtual ~NrMacHeaderFsDl ();

  // CCCH = 0,                      //!< CCCH  (is it fixed or variable?)
  static const uint8_t RECOMMENDED_BIT_RATE = 47;        //!< Recommended bit rate
  static const uint8_t SP_ZP_CSI_RS = 48;                //!< SP ZP CSI-RS Resource Set Activation/DeActivation
  static const uint8_t PUCCH_SPATIAL_RELATION = 49;      //!< PUCCH spatial relation Activation/Deactivation
  static const uint8_t SP_CSI_REPORT = 51;               //!< SP CSI reporting on PUCCH Activation/Deactivation
  static const uint8_t TCI_STATE_INDICATION_PDCCH = 52;  //!< TCI State Indication for UE-specific PDCCH
  static const uint8_t DUPLICATION = 56;                 //!< Duplication Activation/Deactivation
  static const uint8_t SCELL_FOUR_OCTET = 57;            //!< SCell Activation/Deactivation (four octet)
  static const uint8_t SCELL_ONE_OCTET = 58;             //!< SCell Activation/Deactivation (one octet)
  static const uint8_t LONG_DRX = 59;                    //!< Long DRX Command
  static const uint8_t DRX = 60;                         //!< DRX command
  static const uint8_t TIMING_ADVANCE = 61;              //!< Timing advance command
  static const uint8_t UE_CONTENTION_RESOLUTION = 62;    //!< UE Contention resolution identity

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

#endif /* NR_MAC_HEADER_FS_DL_H */

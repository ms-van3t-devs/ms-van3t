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

#ifndef NR_MAC_HEADER_VS_DL_H
#define NR_MAC_HEADER_VS_DL_H

#include "nr-mac-header-vs.h"

namespace ns3 {

/**
 * \ingroup ue-mac
 * \ingroup gnb-mac
 * \brief Mac variable-size Header for DL
 *
 * This header performs some sanity check for the LCID value, but the functionality
 * is almost the same as NrMacHeaderVs. Please note that, by standard, only
 * some LCID can be used in DL transmissions.
 *
 * Please refer to TS 38.321 section 6.1.2 for more information.
 *
 * <b>Users, don't use this header directly: you've been warned.</b>
 *
 * \internal
 *
 * This header must be used to report some variable-sized CE to the UE. At
 * the moment, we don't use it.
 */
class NrMacHeaderVsDl : public NrMacHeaderVs
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
   * \brief NrMacHeaderVsDl constructor
   */
  NrMacHeaderVsDl ();

  /**
   * \brief ~NrMacHeaderVsDl
   */
  virtual ~NrMacHeaderVsDl ();

  // const uint8_t CCCH = 0;          //!< CCCH  (is it fixed or variable?)
  static const uint8_t SP_SRS = 50;          //!< SP SRS Activation/Deactivation
  static const uint8_t TCI_STATES_PDSCH = 53;//!< TCI States Activation/Deactivation for UE-specific PDSCH
  static const uint8_t APERIODIC_CSI = 54;   //!< Aperiodic CSI Trigger State Subselection
  static const uint8_t SP_CSI_RS_IM = 55;    //!< SP CSI-RS / CSI-IM Resource Set Activation/Deactivation

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
   * \brief Check if it really a variable-size header
   * \return true if the lcId value stored internally matches with a variable-size header
   */
  bool IsVariableSizeHeader () const;
};

} //namespace ns3

#endif /* NR_MAC_HEADER_VS_DL_H */

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

#ifndef NR_SL_COMM_RESOURCE_POOL_FACTORY_H
#define NR_SL_COMM_RESOURCE_POOL_FACTORY_H

#include <ns3/simple-ref-count.h>
#include <ns3/lte-rrc-sap.h>
#include <bitset>

namespace ns3 {

/** Class to configure and generate resource pools */
class NrSlCommResourcePoolFactory : public SimpleRefCount<NrSlCommResourcePoolFactory>
{
public:
  NrSlCommResourcePoolFactory ();
  virtual ~NrSlCommResourcePoolFactory ();

  /**
   * \brief Create pool
   *
   * Until 3GPP TS 38.331 is not making a clear distinction between a dedicated
   * and preconfigured pool, we are making this method virtual so child class
   * can override it.
   *
   * \return The struct of type LteRrcSap::SlResourcePoolNr defining the SL pool
   */
  virtual const LteRrcSap::SlResourcePoolNr CreatePool (void);

  /**
   * \brief Setup PSCCH resources
   */
  void SetupPscchResources (void);
  /**
   * \brief Is setup PscchResources
   * \return flag indicating if it is setting up or releasing PSCCH resources
   */
  bool IsSetupPscchResources (void) const;
  /**
   * \brief Get sidelink time resource PSCCH
   * \return The number of symbols for PSCCH transmission
   */
  uint16_t GetSlTimeResourcePscch () const;
  /**
   * \brief Set sidelink time resource PSCCH
   * \param slTimeResourcePscch The number of symbols for PSCCH transmission
   */
  void SetSlTimeResourcePscch (uint16_t slTimeResourcePscch);
  /**
   * \brief Get sidelink frequency resource PSCCH
   * \return The number of resource blocks allocated to PSCCH transmission
   */
  uint16_t GetSlFreqResourcePscch () const;
  /**
   * \brief Set sidelink frequency resource PSCCH
   * \param  slFreqResourcePscch The number of resource blocks allocated to PSCCH transmission
   */
  void SetSlFreqResourcePscch (uint16_t slFreqResourcePscch);
  /**
   * \brief Get sidelink subchannel size
   * \return The subchannel size in RBs
   */
  uint16_t GetSlSubchannelSize () const;
  /**
   * \brief Set sidelink subchannel size
   * \param slSubchannelSize The subchannel size in RBs
   */
  void SetSlSubchannelSize (uint16_t slSubchannelSize);
  /**
   * \brief Get sidelink sensing window
   * \return The sensing window start time in milliseconds
   */
  uint16_t GetSlSensingWindow () const;
  /**
   * \brief Set sidelink sensing window
   * \param slSensingWindow The sensing window start time in milliseconds
   */
  void SetSlSensingWindow (uint16_t slSensingWindow);
  /**
   * \brief Get Sidelink selection window
   * \return The end of the selection window in number of slots
   */
  uint16_t GetSlSelectionWindow () const;
  /**
   * \brief Set Sidelink selection window
   * \param slSelectionWindow The end of the selection window in number of slots
   */
  void SetSlSelectionWindow (uint16_t slSelectionWindow);
  /**
   * \brief Get Sidelink resource reservation period list
   *
   * \return An array of 16 possible resource reservation period allowed in the resource pool
   */
  const std::array<uint16_t, 16>& GetSlResourceReservePeriodList () const;
  /**
   * \brief Set Sidelink resource reservation period list
   *
   * \param slResourceReservePeriodList An array of 16 possible resource reservation period allowed in the resource pool
   */
  void SetSlResourceReservePeriodList (std::array<uint16_t, 16>& slResourceReservePeriodList);
  /**
   * \brief Get Sidelink time resources
   *
   * \return A vector representing a bitmap for sidelink time resources
   */
  const std::vector<std::bitset<1> >& GetSlTimeResources () const;
  /**
   * \brief Set Sidelink time resources
   *
   * \param slBitmap A vector representing a bitmap for sidelink time resources
   */
  void SetSlTimeResources (std::vector <std::bitset<1> >& slBitmap);
  /**
   * \brief Get Sidelink maximum number resource reserve for PSCCH/PSSCH
   *
   * \return The maximum number of reserved PSCCH/PSSCH resources that can be
   *         indicated by an SCI.
   */
  uint16_t GetSlMaxNumPerReserve () const;
  /**
   * \brief Set Sidelink maximum number resource reserve for PSCCH/PSSCH
   *
   * \param maxNumPerReserve The maximum number of reserved PSCCH/PSSCH
   *        resources that can be indicated by an SCI.
   */
  void SetSlMaxNumPerReserve (uint16_t maxNumPerReserve);

private:
  LteRrcSap::SlResourcePoolNr m_pool; //!< Sidelink communication pool

protected:
  //SlPscchConfig
  std::string m_setupReleasePscch; //!< Indicates if it is allocating or releasing PSCCH resources
  uint16_t m_slTimeResourcePscch; //!< Indicates the number of symbols of PSCCH in a resource pool.
  uint16_t m_slFreqResourcePscch; //!< Indicates the number of PRBs for PSCCH in a resource pool where it is not greater than the number PRBs of the subchannel.


  uint16_t m_slSubchannelSize; //!< Sidelink subchannel size in PRBs


  uint16_t m_slSensingWindow; //!< Start of the sensing window in milliseconds.
  uint16_t m_slSelectionWindow; //!< End of the selection window in number of slots.

  std::array <uint16_t, 16> m_slResourceReservePeriodList; //!< Set of 16 possible resource reservation period allowed in the resource pool.
  std::vector <std::bitset<1> > m_slTimeResource; //!< The sidelink time resource bitmap
  uint16_t m_slMaxNumPerReserve; //!< The maximum number of reserved PSCCH/PSSCH resources that can be indicated by an SCI.


};

}

#endif /* NR_SL_COMM_RESOURCE_POOL_FACTORY_H */

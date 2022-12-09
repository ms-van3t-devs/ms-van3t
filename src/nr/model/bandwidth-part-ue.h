/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef COMPONENT_CARRIER_NR_UE_H
#define COMPONENT_CARRIER_NR_UE_H

#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include "ns3/nr-phy.h"
#include <ns3/nr-ue-phy.h>
#include <ns3/component-carrier.h>
#include <ns3/nr-sl-ue-mac-scheduler.h>

namespace ns3 {

class NrUeMac;

/**
 * \ingroup ue-bwp
 * \brief Bandwidth part representation for a UE
 *
 */
class BandwidthPartUe : public ComponentCarrier
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief BandwidthPartUe constructor
   */
  BandwidthPartUe ();

  /**
   * \brief ~BandwidthPartUe
   */
  virtual ~BandwidthPartUe (void) override;

  /**
   * \return a pointer to the physical layer.
   */
  Ptr<NrUePhy> GetPhy (void) const;

  /**
   * \return a pointer to the MAC layer.
   */
  Ptr<NrUeMac> GetMac (void) const;

  /**
   * Set NrUePhy
   * \param s a pointer to the NrUePhy
   */
  void SetPhy (Ptr<NrUePhy> s);

  /**
   * Set the NrGnbMac
   * \param s a pointer to the NrGnbMac
   */
  void SetMac (Ptr<NrUeMac> s);

  virtual void SetDlBandwidth (uint16_t bw) override { m_dlBandwidth = bw; }
  virtual void SetUlBandwidth (uint16_t bw) override { m_ulBandwidth = bw; }

protected:
  /**
   * \brief DoDispose method inherited from Object
   */
  void virtual DoDispose () override;

private:
  Ptr<NrUePhy> m_phy; ///< the Phy instance of this eNodeB component carrier
  Ptr<NrUeMac> m_mac; ///< the MAC instance of this eNodeB component carrier

//NR Sidelink
public:
  /**
   * \brief Set the NR Sidelink UE scheduler type
   * \param s a pointer to the NrSlUeMacScheduler
   */
  void SetNrSlUeMacScheduler (Ptr<NrSlUeMacScheduler> s);
  /**
   * \brief Get the NR Sidelink UE scheduler type
   * \return a pointer to the NrSlUeMacScheduler
   */
  Ptr<NrSlUeMacScheduler> GetNrSlUeMacScheduler ();

private:

  Ptr<NrSlUeMacScheduler> m_slUeScheduler {nullptr};

};

} // namespace ns3

#endif /* COMPONENT_CARRIER_UE_H */

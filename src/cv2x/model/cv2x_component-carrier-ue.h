/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Danilo Abrignani
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
 * Author: Danilo Abrignani <danilo.abrignani@unibo.it>
 */


#ifndef CV2X_COMPONENT_CARRIER_UE_H
#define CV2X_COMPONENT_CARRIER_UE_H

#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include "ns3/cv2x_lte-phy.h"
#include <ns3/cv2x_lte-ue-phy.h>
#include <ns3/cv2x_component-carrier.h>

namespace ns3 {

class cv2x_LteUeMac;
/**
 * \ingroup lte
 *
 * cv2x_ComponentCarrierUe Object, it defines a single Carrier for the Ue
 */
class cv2x_ComponentCarrierUe : public cv2x_ComponentCarrier
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  cv2x_ComponentCarrierUe ();

  virtual ~cv2x_ComponentCarrierUe (void);
  virtual void DoDispose (void);


  /**
   * \return a pointer to the physical layer.
   */
  Ptr<cv2x_LteUePhy> GetPhy (void) const;

  /**
   * \return a pointer to the MAC layer.
   */
  Ptr<cv2x_LteUeMac> GetMac (void) const;

  /**
   * Set cv2x_LteUePhy
   * \param s a pointer to the cv2x_LteUePhy
   */  
  void SetPhy (Ptr<cv2x_LteUePhy> s);

  /**
   * Set the cv2x_LteEnbMac
   * \param s a pointer to the cv2x_LteEnbMac
   */ 
  void SetMac (Ptr<cv2x_LteUeMac> s);
  
protected:
  // inherited from Object
  virtual void DoInitialize (void);

private:

  Ptr<cv2x_LteUePhy> m_phy; ///< the Phy instance of this eNodeB component carrier
  Ptr<cv2x_LteUeMac> m_mac; ///< the MAC instance of this eNodeB component carrier

};



} // namespace ns3



#endif /* CV2X_COMPONENT_CARRIER_UE_H */

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


#ifndef CV2X_COMPONENT_CARRIER_ENB_H
#define CV2X_COMPONENT_CARRIER_ENB_H

#include "cv2x_component-carrier.h"
#include <ns3/object.h>
#include <ns3/packet.h>
#include <ns3/nstime.h>
#include "ns3/cv2x_lte-phy.h"
#include <ns3/cv2x_lte-enb-phy.h>
#include <ns3/pointer.h>
//#include <ns3/cv2x_lte-enb-mac.h>


namespace ns3 {

class cv2x_LteEnbMac;
class cv2x_FfMacScheduler;
class cv2x_LteFfrAlgorithm;

/**
 * \ingroup lte
 *
 * Defines a single carrier for enb, and contains pointers to cv2x_LteEnbPhy,
 * cv2x_LteEnbMac, cv2x_LteFfrAlgorithm, and cv2x_FfMacScheduler objects.
 *
 */
class cv2x_ComponentCarrierEnb : public cv2x_ComponentCarrier
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  cv2x_ComponentCarrierEnb ();

  virtual ~cv2x_ComponentCarrierEnb (void);
  virtual void DoDispose (void);

  /**
   * Get cell identifier
   * \return cell identifier
   */
  uint16_t GetCellId ();

  /**
   * \return a pointer to the physical layer.
   */
  Ptr<cv2x_LteEnbPhy> GetPhy (void);

  /**
   * \return a pointer to the MAC layer.
   */
  Ptr<cv2x_LteEnbMac> GetMac (void);

  /**
   * \return a pointer to the Ffr Algorithm.
   */
  Ptr<cv2x_LteFfrAlgorithm> GetFfrAlgorithm ();

  /**
   * \return a pointer to the Mac Scheduler.
   */
  Ptr<cv2x_FfMacScheduler> GetFfMacScheduler ();

  /**
   * Set physical cell identifier
   * \param cellId cell identifier
   */
  void SetCellId (uint16_t cellId);

  /**
   * Set the cv2x_LteEnbPhy
   * \param s a pointer to the cv2x_LteEnbPhy
   */
  void SetPhy (Ptr<cv2x_LteEnbPhy> s);
  /**
   * Set the cv2x_LteEnbMac
   * \param s a pointer to the cv2x_LteEnbMac
   */
  void SetMac (Ptr<cv2x_LteEnbMac> s);

  /**
   * Set the cv2x_FfMacScheduler Algorithm
   * \param s a pointer to the cv2x_FfMacScheduler
   */
  void SetFfMacScheduler (Ptr<cv2x_FfMacScheduler> s);

  /**
   * Set the cv2x_LteFfrAlgorithm
   * \param s a pointer to the cv2x_LteFfrAlgorithm
   */
  void SetFfrAlgorithm (Ptr<cv2x_LteFfrAlgorithm> s);
  
protected:

  virtual void DoInitialize (void);

private:

  uint16_t m_cellId; ///< Cell identifier
  Ptr<cv2x_LteEnbPhy> m_phy; ///< the Phy instance of this eNodeB component carrier
  Ptr<cv2x_LteEnbMac> m_mac; ///< the MAC instance of this eNodeB component carrier
  Ptr<cv2x_FfMacScheduler> m_scheduler; ///< the scheduler instance of this eNodeB component carrier
  Ptr<cv2x_LteFfrAlgorithm> m_ffrAlgorithm; ///< the FFR algorithm instance of this eNodeB component carrier
 

};

} // namespace ns3



#endif /* COMPONENT_CARRIER_H */

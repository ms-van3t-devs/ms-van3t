/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2013 Budiarto Herman
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
 * Author: Budiarto Herman <budiarto.herman@magister.fi>
 *
 */

#ifndef CV2X_LTE_HANDOVER_ALGORITHM_H
#define CV2X_LTE_HANDOVER_ALGORITHM_H

#include <ns3/object.h>
#include <ns3/cv2x_lte-rrc-sap.h>

namespace ns3 {


class cv2x_LteHandoverManagementSapUser;
class cv2x_LteHandoverManagementSapProvider;


/**
 * \brief The abstract base class of a handover algorithm that operates using
 *        the Handover Management SAP interface.
 *
 * Handover algorithm receives measurement reports from an eNodeB RRC instance
 * and tells the eNodeB RRC instance when to do a handover.
 *
 * This class is an abstract class intended to be inherited by subclasses that
 * implement its virtual methods. By inheriting from this abstract class, the
 * subclasses gain the benefits of being compatible with the cv2x_LteEnbNetDevice
 * class, being accessible using namespace-based access through ns-3 Config
 * subsystem, and being installed and configured by cv2x_LteHelper class (see
 * cv2x_LteHelper::SetHandoverAlgorithmType and
 * cv2x_LteHelper::SetHandoverAlgorithmAttribute methods).
 *
 * The communication with the eNodeB RRC instance is done through the *Handover
 * Management SAP* interface. The handover algorithm instance corresponds to the
 * "provider" part of this interface, while the eNodeB RRC instance takes the
 * role of the "user" part. The following code skeleton establishes the
 * connection between both instances:
 *
 *     Ptr<cv2x_LteEnbRrc> u = ...;
 *     Ptr<cv2x_LteHandoverAlgorithm> p = ...;
 *     u->SetLteHandoverManagementSapProvider (p->GetLteHandoverManagementSapProvider ());
 *     p->SetLteHandoverManagementSapUser (u->GetLteHandoverManagementSapUser ());
 *
 * However, user rarely needs to use the above code, since it has already been
 * taken care by cv2x_LteHelper::InstallEnbDevice.
 *
 * \sa cv2x_LteHandoverManagementSapProvider, cv2x_LteHandoverManagementSapUser
 */
class cv2x_LteHandoverAlgorithm : public Object
{
public:
  cv2x_LteHandoverAlgorithm ();
  virtual ~cv2x_LteHandoverAlgorithm ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * \brief Set the "user" part of the Handover Management SAP interface that
   *        this handover algorithm instance will interact with.
   * \param s a reference to the "user" part of the interface, typically a
   *          member of an cv2x_LteEnbRrc instance
   */
  virtual void SetLteHandoverManagementSapUser (cv2x_LteHandoverManagementSapUser* s) = 0;

  /**
   * \brief Export the "provider" part of the Handover Management SAP interface.
   * \return the reference to the "provider" part of the interface, typically to
   *         be kept by an cv2x_LteEnbRrc instance
   */
  virtual cv2x_LteHandoverManagementSapProvider* GetLteHandoverManagementSapProvider () = 0;

protected:

  // inherited from Object
  virtual void DoDispose ();

  // HANDOVER MANAGEMENT SAP PROVIDER IMPLEMENTATION

  /**
   * \brief Implementation of cv2x_LteHandoverManagementSapProvider::ReportUeMeas.
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   * \param measResults a single report of one measurement identity
   */
  virtual void DoReportUeMeas (uint16_t rnti, cv2x_LteRrcSap::MeasResults measResults) = 0;

}; // end of class cv2x_LteHandoverAlgorithm


} // end of namespace ns3


#endif /* CV2X_LTE_HANDOVER_ALGORITHM_H */

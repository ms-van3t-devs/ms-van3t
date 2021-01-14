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

#ifndef CV2X_LTE_ANR_SAP_H
#define CV2X_LTE_ANR_SAP_H

#include <ns3/cv2x_lte-rrc-sap.h>

namespace ns3 {


/**
 * \brief Service Access Point (SAP) offered by the ANR instance to the eNodeB
 *        RRC instance.
 *
 * This is the *ANR SAP Provider*, i.e., the part of the SAP that contains the
 * ANR (Automatic Neighbour Relation) methods called by the eNodeB RRC instance.
 */
class cv2x_LteAnrSapProvider
{
public:
  virtual ~cv2x_LteAnrSapProvider ();

  /**
   * \brief Send a UE measurement report to the ANC instance.
   * \param measResults a single report of one measurement identity
   *
   * The received measurement report is a result of the UE measurement
   * configuration previously configured by calling
   * cv2x_LteAnrSapUser::AddUeMeasReportConfigForAnr. The report may be stored and
   * utilized for the purpose of maintaining Neighbour Relation Table (NRT).
   */
  virtual void ReportUeMeas (cv2x_LteRrcSap::MeasResults measResults) = 0;

  /**
   * \brief Add a new Neighbour Relation entry.
   * \param cellId the Physical Cell ID of the new neighbouring cell
   */
  virtual void AddNeighbourRelation (uint16_t cellId) = 0;

  /**
   * \brief Get the value of *No Remove* field of a neighbouring cell from the
   *        Neighbour Relation Table (NRT).
   * \param cellId the Physical Cell ID of the neighbouring cell of interest
   * \return if true, the Neighbour Relation shall *not* be removed from the NRT
   */
  virtual bool GetNoRemove (uint16_t cellId) const = 0;

  /**
   * \brief Get the value of *No HO* field of a neighbouring cell from the
   *        Neighbour Relation Table (NRT).
   * \param cellId the Physical Cell ID of the neighbouring cell of interest
   * \return if true, the Neighbour Relation shall *not* be used by the eNodeB
   *         for handover reasons
   */
  virtual bool GetNoHo (uint16_t cellId) const = 0;

  /**
   * \brief Get the value of *No X2* field of a neighbouring cell from the
   *        Neighbour Relation Table (NRT).
   * \param cellId the Physical Cell ID of the neighbouring cell of interest
   * \return if true, the Neighbour Relation shall *not* use an X2 interface in
   *         order to initiate procedures towards the eNodeB parenting the
   *         target cell
   */
  virtual bool GetNoX2 (uint16_t cellId) const = 0;

}; // end of class cv2x_LteAnrSapProvider



/**
 * \brief Service Access Point (SAP) offered by the eNodeB RRC instance to the
 *        ANR instance.
 *
 * This is the *ANR SAP User*, i.e., the part of the SAP that contains the
 * eNodeB RRC methods called by the ANR (Automatic Neighbour Relation) instance.
 */
class cv2x_LteAnrSapUser
{
public:
  virtual ~cv2x_LteAnrSapUser ();

  /**
   * \brief Request a certain reporting configuration to be fulfilled by the UEs
   *        attached to the eNodeB entity.
   * \param reportConfig the UE measurement reporting configuration
   * \return the measurement identity associated with this newly added
   *         reporting configuration
   *
   * The eNodeB RRC entity is expected to configure the same reporting
   * configuration in each of the attached UEs. When later in the simulation a
   * UE measurement report is received from a UE as a result of this
   * configuration, the eNodeB RRC entity shall forward this report to the ANC
   * instance through the cv2x_LteAnrSapProvider::ReportUeMeas SAP function.
   *
   * \note This function is only valid before the simulation begins.
   */
  virtual uint8_t AddUeMeasReportConfigForAnr (cv2x_LteRrcSap::ReportConfigEutra reportConfig) = 0;

}; // end of class cv2x_LteAnrSapUser



/**
 * \brief Template for the implementation of the cv2x_LteAnrSapProvider as a member
 *        of an owner class of type C to which all methods are forwarded.
 */
template <class C>
class cv2x_MemberLteAnrSapProvider : public cv2x_LteAnrSapProvider
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteAnrSapProvider (C* owner);

  // inherited from cv2x_LteAnrSapProvider
  virtual void ReportUeMeas (cv2x_LteRrcSap::MeasResults measResults);
  virtual void AddNeighbourRelation (uint16_t cellId);
  virtual bool GetNoRemove (uint16_t cellId) const;
  virtual bool GetNoHo (uint16_t cellId) const;
  virtual bool GetNoX2 (uint16_t cellId) const;

private:
  cv2x_MemberLteAnrSapProvider ();
  C* m_owner; ///< the owner class

}; // end of class cv2x_MemberLteAnrSapProvider


template <class C>
cv2x_MemberLteAnrSapProvider<C>::cv2x_MemberLteAnrSapProvider (C* owner)
  : m_owner (owner)
{
}


template <class C>
void
cv2x_MemberLteAnrSapProvider<C>::ReportUeMeas (cv2x_LteRrcSap::MeasResults measResults)
{
  m_owner->DoReportUeMeas (measResults);
}


template <class C>
void
cv2x_MemberLteAnrSapProvider<C>::AddNeighbourRelation (uint16_t cellId)
{
  m_owner->DoAddNeighbourRelation (cellId);
}


template <class C>
bool
cv2x_MemberLteAnrSapProvider<C>::GetNoRemove (uint16_t cellId) const
{
  return m_owner->DoGetNoRemove (cellId);
}


template <class C>
bool
cv2x_MemberLteAnrSapProvider<C>::GetNoHo (uint16_t cellId) const
{
  return m_owner->DoGetNoHo (cellId);
}


template <class C>
bool
cv2x_MemberLteAnrSapProvider<C>::GetNoX2 (uint16_t cellId) const
{
  return m_owner->DoGetNoX2 (cellId);
}



/**
 * \brief Template for the implementation of the cv2x_LteAnrSapUser as a member of an
 *        owner class of type C to which all methods are forwarded.
 */
template <class C>
class cv2x_MemberLteAnrSapUser : public cv2x_LteAnrSapUser
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteAnrSapUser (C* owner);

  // inherited from cv2x_LteAnrSapUser
  virtual uint8_t AddUeMeasReportConfigForAnr (cv2x_LteRrcSap::ReportConfigEutra reportConfig);

private:
  cv2x_MemberLteAnrSapUser ();
  C* m_owner; ///< the owner class

}; // end of class cv2x_MemberLteAnrSapUser


template <class C>
cv2x_MemberLteAnrSapUser<C>::cv2x_MemberLteAnrSapUser (C* owner)
  : m_owner (owner)
{
}


template <class C>
uint8_t
cv2x_MemberLteAnrSapUser<C>::AddUeMeasReportConfigForAnr (cv2x_LteRrcSap::ReportConfigEutra reportConfig)
{
  return m_owner->DoAddUeMeasReportConfigForAnr (reportConfig);
}


} // end of namespace ns3


#endif /* CV2X_LTE_ANR_SAP_H */

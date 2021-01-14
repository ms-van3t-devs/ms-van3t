/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Piotr Gawlowicz
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
 * Author: Piotr Gawlowicz <gawlowicz.p@gmail.com>
 *
 */

#ifndef CV2X_LTE_FFR_ALGORITHM_H
#define CV2X_LTE_FFR_ALGORITHM_H

#include <ns3/object.h>
#include <ns3/cv2x_lte-rrc-sap.h>
#include <ns3/cv2x_epc-x2-sap.h>
#include <ns3/cv2x_ff-mac-sched-sap.h>
#include <map>

namespace ns3 {

class cv2x_LteFfrSapUser;
class cv2x_LteFfrSapProvider;

class cv2x_LteFfrRrcSapUser;
class cv2x_LteFfrRrcSapProvider;

/**
 * \brief The abstract base class of a Frequency Reuse algorithm
 *
 * Generally Frequency reuse algorithm tells the Scheduler which RB can be allocated
 * and which can not. FR policy depend on its implementation.
 *
 * The communication with the eNodeB MAC Scheduler instance is done through
 * the *LteFfrSap* interface. The frequency reuse algorithm instance corresponds to the
 * "provider" part of this interface, while the eNodeB MAC Scheduler instance takes the
 * role of the "user" part.
 *
 * The communication with the eNodeB RRC instance is done through the *LteFfrRrcSap*
 * interface. The frequency reuse algorithm instance corresponds to the
 * "provider" part of this interface, while the eNodeB RRC instance takes the
 * role of the "user" part.
 *
 */

class cv2x_LteFfrAlgorithm : public Object
{
public:
  cv2x_LteFfrAlgorithm ();
  virtual ~cv2x_LteFfrAlgorithm ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * \brief Set the "user" part of the LteFfrSap interface that
   *        this frequency reuse algorithm instance will interact with.
   * \param s a reference to the "user" part of the interface, typically a
   *          member of an Scheduler instance
   */
  virtual void SetLteFfrSapUser (cv2x_LteFfrSapUser* s) = 0;

  /**
   * \brief Set the "user" part of the LteFfrRrcSap interface that
   *        this frequency reuse algorithm instance will interact with.
   * \param s a reference to the "user" part of the interface, typically a
   *          member of an cv2x_LteEnbRrc instance
   */
  virtual void SetLteFfrRrcSapUser (cv2x_LteFfrRrcSapUser* s) = 0;

  /**
   * \brief Export the "provider" part of the LteFfrSap interface.
   * \return the reference to the "provider" part of the interface, typically to
   *         be kept by an Scheduler instance
   */
  virtual cv2x_LteFfrSapProvider* GetLteFfrSapProvider () = 0;

  /**
   * \brief Export the "provider" part of the LteFfrRrcSap interface.
   * \return the reference to the "provider" part of the interface, typically to
   *         be kept by an cv2x_LteEnbRrc instance
   */
  virtual cv2x_LteFfrRrcSapProvider* GetLteFfrRrcSapProvider () = 0;

  /**
   * \return the uplink bandwidth in RBs
   */
  uint8_t GetUlBandwidth () const;

  /**
   * \param bw the uplink bandwidth in RBs
   */
  void SetUlBandwidth (uint8_t bw);

  /**
   * \return the downlink bandwidth in RBs
   */
  uint8_t GetDlBandwidth () const;

  /**
   * \param bw the downlink bandwidth in RBs
   */
  void SetDlBandwidth (uint8_t bw);

  /**
   * \param cellTypeId for automatic FR configuration
   */
  void SetFrCellTypeId (uint8_t cellTypeId);

  /**
   * \return cellTypeId which is used for automatic FR configuration
   */
  uint8_t GetFrCellTypeId () const;

protected:

  // inherited from Object
  virtual void DoDispose ();

  /**
   * \brief Automatic FR reconfiguration
   */
  virtual void Reconfigure () = 0;

  // FFR SAP PROVIDER IMPLEMENTATION

  /**
   * \brief Implementation of cv2x_LteFfrSapProvider::GetAvailableDlRbg
   * \return vector of size (m_dlBandwidth/RbgSize); false indicates
   *                    that RBG is free to use, true otherwise
   */
  virtual std::vector <bool> DoGetAvailableDlRbg () = 0;

  /**
   * \brief Implementation of cv2x_LteFfrSapProvider::IsDlRbgAvailableForUe
   * \param rbId
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   * \return true if UE can be served on i-th RB, false otherwise
   */
  virtual bool DoIsDlRbgAvailableForUe (int rbId, uint16_t rnti) = 0;

  /**
   * \brief Implementation of cv2x_LteFfrSapProvider::GetAvailableUlRbg.
   * \return vector of size m_ulBandwidth; false indicates
   *                    that RB is free to use, true otherwise
   */
  virtual std::vector <bool> DoGetAvailableUlRbg () = 0;

  /**
   * \brief Implementation of cv2x_LteFfrSapProvider::IsUlRbgAvailableForUe.
   * \param rbId
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   * \return true if UE can be served on i-th RB, false otherwise
   */
  virtual bool DoIsUlRbgAvailableForUe (int rbId, uint16_t rnti) = 0;

  /**
   * \brief DoReportDlCqiInfo
   * \param params
   *
   */
  virtual void DoReportDlCqiInfo (const struct cv2x_FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params) = 0;

  /**
   * \brief DoReportUlCqiInfo
   * \param params
   *
   */
  virtual void DoReportUlCqiInfo (const struct cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params) = 0;

  /**
   * \brief DoReportUlCqiInfo
   * \param ulCqiMap
   *
   */
  virtual void DoReportUlCqiInfo ( std::map <uint16_t, std::vector <double> > ulCqiMap ) = 0;

  /**
   * \brief DoGetTpc for UE
   * \param rnti
   * \return TPC value
   */
  virtual uint8_t DoGetTpc (uint16_t rnti) = 0;

  /**
   * \brief DoGetMinContinuousUlBandwidth in number of RB
   * \return number of RB in min continuous UL Bandwidth
   */
  virtual uint8_t DoGetMinContinuousUlBandwidth () = 0;

  // FFR SAP RRC PROVIDER IMPLEMENTATION

  /**
   * \brief SetCellId
   * \param cellId the Cell Identifier
   */
  virtual void DoSetCellId (uint16_t cellId);

  /**
   * \brief Implementation of cv2x_LteFfrRrcSapProvider::SetBandwidth.
   * \param ulBandwidth UL bandwidth in number of RB
   * \param dlBandwidth DL bandwidth in number of RB
   */
  virtual void DoSetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth);

  /**
   * \brief Implementation of cv2x_LteFfrRrcSapProvider::ReportUeMeas.
   * \param rnti Radio Network Temporary Identity, an integer identifying the UE
   *             where the report originates from
   * \param measResults a single report of one measurement identity
   */
  virtual void DoReportUeMeas (uint16_t rnti, cv2x_LteRrcSap::MeasResults measResults) = 0;

  /**
   * \brief DoRecvLoadInformation
   * \param params
   *
   */
  virtual void DoRecvLoadInformation (cv2x_EpcX2Sap::LoadInformationParams params) = 0;

  /**
  * \brief Get RBG size for DL Bandwidth according to table 7.1.6.1-1 of 36.213
  * \param dlbandwidth
  * \return size of RBG in number of RB
  */
  int GetRbgSize (int dlbandwidth);


  uint16_t m_cellId; /**< cell ID */

  uint8_t m_dlBandwidth; /**< downlink bandwidth in RBs */
  uint8_t m_ulBandwidth; /**< uplink bandwidth in RBs */

  uint8_t m_frCellTypeId; /**< FFR cell type ID for automatic configuration */

  bool m_enabledInUplink; /**< If true FR algorithm will also work in Uplink*/

  bool m_needReconfiguration; /**< If true FR algorithm will be reconfigured*/

}; // end of class cv2x_LteFfrAlgorithm


} // end of namespace ns3


#endif /* CV2X_LTE_FFR_ALGORITHM_H */

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

#ifndef CV2X_LTE_FR_HARD_ALGORITHM_H
#define CV2X_LTE_FR_HARD_ALGORITHM_H

#include <ns3/cv2x_lte-ffr-algorithm.h>
#include <ns3/cv2x_lte-ffr-sap.h>
#include <ns3/cv2x_lte-ffr-rrc-sap.h>
#include <ns3/cv2x_lte-rrc-sap.h>

namespace ns3 {


/**
 * \brief Hard Frequency Reuse algorithm implementation which uses only 1 sub-band.
 */
class cv2x_LteFrHardAlgorithm : public cv2x_LteFfrAlgorithm
{
public:
  /**
   * \brief Creates a trivial ffr algorithm instance.
   */
  cv2x_LteFrHardAlgorithm ();

  virtual ~cv2x_LteFrHardAlgorithm ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  // inherited from cv2x_LteFfrAlgorithm
  virtual void SetLteFfrSapUser (cv2x_LteFfrSapUser* s);
  virtual cv2x_LteFfrSapProvider* GetLteFfrSapProvider ();

  virtual void SetLteFfrRrcSapUser (cv2x_LteFfrRrcSapUser* s);
  virtual cv2x_LteFfrRrcSapProvider* GetLteFfrRrcSapProvider ();

  /// let the forwarder class access the protected and private members
  friend class cv2x_MemberLteFfrSapProvider<cv2x_LteFrHardAlgorithm>;
  /// let the forwarder class access the protected and private members
  friend class cv2x_MemberLteFfrRrcSapProvider<cv2x_LteFrHardAlgorithm>;

protected:
  // inherited from Object
  virtual void DoInitialize ();
  virtual void DoDispose ();

  virtual void Reconfigure ();

  // FFR SAP PROVIDER IMPLEMENTATION
  virtual std::vector <bool> DoGetAvailableDlRbg ();
  virtual bool DoIsDlRbgAvailableForUe (int i, uint16_t rnti);
  virtual std::vector <bool> DoGetAvailableUlRbg ();
  virtual bool DoIsUlRbgAvailableForUe (int i, uint16_t rnti);
  virtual void DoReportDlCqiInfo (const struct cv2x_FfMacSchedSapProvider::SchedDlCqiInfoReqParameters& params);
  virtual void DoReportUlCqiInfo (const struct cv2x_FfMacSchedSapProvider::SchedUlCqiInfoReqParameters& params);
  virtual void DoReportUlCqiInfo ( std::map <uint16_t, std::vector <double> > ulCqiMap );
  virtual uint8_t DoGetTpc (uint16_t rnti);
  virtual uint8_t DoGetMinContinuousUlBandwidth ();

  // FFR SAP RRC PROVIDER IMPLEMENTATION
  virtual void DoReportUeMeas (uint16_t rnti, cv2x_LteRrcSap::MeasResults measResults);
  virtual void DoRecvLoadInformation (cv2x_EpcX2Sap::LoadInformationParams params);

private:
  /**
   * Set downlink configuration
   *
   * \param cellId the cell ID
   * \param bandwidth the bandwidth
   */
  void SetDownlinkConfiguration (uint16_t cellId, uint8_t bandwidth);
  /**
   * Set uplink configuration
   *
   * \param cellId the cell ID
   * \param bandwidth the bandwidth
   */
  void SetUplinkConfiguration (uint16_t cellId, uint8_t bandwidth);
  /**
   * Initialize downlink rbg maps
   */
  void InitializeDownlinkRbgMaps ();
  /**
   * Initialize uplink rbg maps
   */
  void InitializeUplinkRbgMaps ();

  // FFR SAP
  cv2x_LteFfrSapUser* m_ffrSapUser; ///< FFR SAP user
  cv2x_LteFfrSapProvider* m_ffrSapProvider; ///< FFR SAP provider

  // FFR RRF SAP
  cv2x_LteFfrRrcSapUser* m_ffrRrcSapUser; ///< FFR RRC SAP user
  cv2x_LteFfrRrcSapProvider* m_ffrRrcSapProvider; ///< FFR RRC SAP provider

  uint8_t m_dlOffset; ///< DL offset
  uint8_t m_dlSubBand; ///< DL subband

  uint8_t m_ulOffset; ///< UL offset
  uint8_t m_ulSubBand; ///< UL subband

  std::vector <bool> m_dlRbgMap; ///< DL RBG Map
  std::vector <bool> m_ulRbgMap; ///< UL RBG Map

}; // end of class cv2x_LteFrHardAlgorithm


} // end of namespace ns3


#endif /* CV2X_LTE_FR_HARD_ALGORITHM_H */

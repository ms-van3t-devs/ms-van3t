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

#ifndef NR_SL_BWP_MANAGER_UE_H
#define NR_SL_BWP_MANAGER_UE_H

#include <ns3/bwp-manager-ue.h>
#include <ns3/nr-sl-ue-bwpm-rrc-sap.h>
#include <ns3/nr-sl-ue-cmac-sap.h>
#include <ns3/lte-rrc-sap.h>
#include <ns3/nr-sl-mac-sap.h>
#include <map>
#include <unordered_map>

namespace ns3 {


/**
 * \ingroup nr
 * \brief The NrSlBwpManagerUe class
 */
class NrSlBwpManagerUe : public BwpManagerUe
{
public:
  /// let the forwarder class access the protected and private members
  friend class MemberNrSlUeBwpmRrcSapProvider<NrSlBwpManagerUe>;

  //Allow calls from RLC and MAC to land in this class
  /// allow MemberNrSlMacSapProvider class friend access
  friend class MemberNrSlMacSapProvider <NrSlBwpManagerUe>;
  /// allow MemberNrSlMacSapUser class friend access
  friend class MemberNrSlMacSapUser <NrSlBwpManagerUe>;

  /**
   * \brief BwpManagerUe constructor
   */
  NrSlBwpManagerUe ();
  /**
   * \brief ~NrSlBwpManagerUe
   */
  virtual ~NrSlBwpManagerUe ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId ();

  /**
   * \brief Get the NR Sidelik BWP Manager SAP offered by this manager
   *
   * \return s the NR Sidelik BWP Manager SAP Provider interface offered to the
   *           RRC by this manager
   */
  NrSlUeBwpmRrcSapProvider* GetNrSlUeBwpmRrcSapProvider () const;

  /**
   * \brief Set the NR Sidelik BWP Manager SAP this manager should use to
   *        interact with UE RRC.
   *
   * \param nrSlUeBwpmRrcSapUser the NR Sidelik UE BWP Manager SAP User to be used by this manager
   */
  void SetNrSlUeBwpmRrcSapUser (NrSlUeBwpmRrcSapUser* nrSlUeBwpmRrcSapUser);

  /**
   * \brief Returns the NR SL MAC SAP provider interface own by this BWP manager
   *
   * RRC takes this pointer and set it in the UE MAC, and that is how
   * calls from MAC lands in this BWP manager.
   *
   * \return the pointer of of to NrSlMacSapProvider interface
   */
  NrSlMacSapProvider* GetNrSlMacSapProviderFromBwpm ();

  /**
   * \brief Sets a pointer to NR SL MAC SAP provider of UE per BWP id.
   *
   * \param bwpId the bandwidth part id
   * \param sap the pointer to the sap interface
   *
   * \return whether the settings of the sap provider was successful
   */
  bool SetNrSlMacSapProviders (uint8_t bwpId, NrSlMacSapProvider* sap);


protected:
  // inherited from Object
  virtual void DoDispose ();

  /// allow NrSlBwpmUeMacSapProvider class friend access
  friend class NrSlBwpmUeMacSapProvider;
  /// allow NrSlBwpmUeMacSapUser class friend access
  friend class NrSlBwpmUeMacSapUser;

  // forwarded from NrSlMacSapProvider
  /**
   * \brief Transmit NR SL PDU function
   *
   * \param params NrSlMacSapProvider::NrSlRlcPduParameters
   */
  void DoTransmitNrSlRlcPdu (const NrSlMacSapProvider::NrSlRlcPduParameters &params);
  /**
   * \brief Report buffer status function
   *
   * \param params NrSlMacSapProvider::NrSlReportBufferStatusParameters
   */
  void DoReportNrSlBufferStatus (const NrSlMacSapProvider::NrSlReportBufferStatusParameters &params);

  // forwarded from NrSlMacSapUser
  /**
   * \brief Notify NR SL TX opportunity function
   *
   * \param txOpParams the NrSlMacSapUser::NrSlTxOpportunityParameters
   */
  void DoNotifyNrSlTxOpportunity (const NrSlMacSapUser::NrSlTxOpportunityParameters &txOpParams);

  /**
   * \brief Receive NR SL PDU function
   *
   * \param rxPduParams the NrSlMacSapUser::NrSlReceiveRlcPduParameters
   */
  void DoReceiveNrSlRlcPdu (NrSlMacSapUser::NrSlReceiveRlcPduParameters rxPduParams);

  //forwarded from NrSlUeBwpmRrcSapProvider
  /**
   * \brief Add a new NR Sidelink Data Radio Bearer Logical Channel (LC)
   *
   * \param lcInfo is the Sidelink Logical Channel Information
   * \param msu is the pointer to NrSlMacSapUser, which MAC uses to call RLC methods
   * \return vector of LcsConfig contains the lc configuration for each MAC
   *         the size of the vector is equal to the number of bandwidth part manager enabled.
   */
  std::vector<NrSlUeBwpmRrcSapProvider::SlLcInfoBwpm> DoAddNrSlDrbLc (const NrSlUeCmacSapProvider::SidelinkLogicalChannelInfo &lcInfo, NrSlMacSapUser* msu);

  /**
   * \brief Remove an existing NR Sidelink Drb Logical Channel for a UE in the NrSlBwpManager
   *
   * \param slLcId is the Sidelink Logical Channel Id
   * \param srcL2Id is the Source L2 ID
   * \param dstL2Id is the Destination L2 ID
   *
   * \return A vector containing the BWP ids for which the LC was removed.
   */
  std::vector<uint8_t> DoRemoveNrSlDrbLc (uint8_t slLcId, uint32_t srcL2Id, uint32_t dstL2Id);
  /**
   * \brief Reset Nr Sidelink DRB LC map
   *
   */
  void DoResetNrSlDrbLcMap ();
  /**
   * \brief Set Bwp Id Container
   *
   * \param bwpIdVec The container of SL BWP ids
   */
  void DoSetBwpIdContainer (const std::set<uint8_t> &bwpIdVec);

private:
  /// NR sidelink UE BWP Logical Channel Identifier
  struct NrSlUeBwpLcIdentifier
  {
    uint8_t lcId; //!< Sidelink LCID
    uint32_t srcL2Id; //!< Source L2 ID
    uint32_t dstL2Id; //!< Destination L2 ID
  };

  /**
   * \brief Less than operator overloaded for NrSlUeBwpLcIdentifier
   *
   * \param l first SidelinkLcIdentifier
   * \param r second SidelinkLcIdentifier
   * \returns true if first NrSlUeBwpLcIdentifier parameter values are less than the second NrSlUeBwpLcIdentifier parameters"
   */
  friend bool operator < (const NrSlUeBwpLcIdentifier &l, const NrSlUeBwpLcIdentifier &r)
  {
    return l.lcId < r.lcId || (l.lcId == r.lcId && l.srcL2Id < r.srcL2Id) || (l.lcId == r.lcId && l.srcL2Id == r.srcL2Id && l.dstL2Id < r.dstL2Id);
  }

  /**
   * \brief Nr Sidelink logical channel configuration per BWP Id of this UE.
   * Key : BWP id
   * Mapped value: A map whose key is a struct
   * object of type NrSlUeBwpLcIdentifier
   * and mapped value is a NrSlMacSapProvider pointer
   */
  std::map<uint8_t, std::map<NrSlUeBwpLcIdentifier, NrSlMacSapProvider*> > m_nrSlBwpLcMap;
  std::map<NrSlUeBwpLcIdentifier, NrSlMacSapUser*> m_nrSlDrbLcInfoMap; //!< NR Sidelink data radio bearer logical channel info map
  std::map<NrSlUeBwpLcIdentifier, EpsBearer::Qci> m_slLcToBearerMap; //!< Map from LCID to bearer Qci/priority
  //I am using std::set here instead of std::unordered_set for 2 reason:
  //1. Python bindings does not support std::unordered_set
  //2. I do not see this container to pass max 2 elements
  std::set <uint8_t> m_slBwpIds; //!< A container to store SL BWP ids
  std::map <uint8_t, NrSlMacSapProvider*> m_nrSlMacSapProvidersMap; //!< Map of pointers to the NR SL MAC SAP Provider interface of the UE.

  NrSlUeBwpmRrcSapProvider* m_nrSlUeBwpmRrcSapProvider; //!< NR SL UE BWP manager RRC SAP provider
  NrSlUeBwpmRrcSapUser* m_nrSlUeBwpmRrcSapUser {nullptr}; //!< NR SL UE BWP manager RRC SAP user

  NrSlMacSapProvider* m_nrSlBwpmUeMacSapProvider; //!< Receive API calls from the UE RLC instance for UE MAC
  NrSlMacSapUser* m_nrSlBwpmUeMacSapUser;//!< Receive API calls from the UE MAC instance for UE RLC

}; // end of class NrSlBwpManagerUe


} // end of namespace ns3


#endif /* NR_SL_BWP_MANAGER_UE_H */

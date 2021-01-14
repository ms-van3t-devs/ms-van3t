/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Lluis Parcerisa <lparcerisa@cttc.cat>
 * Modified by:
 *          Danilo Abrignani <danilo.abrignani@unibo.it> (Carrier Aggregation - GSoC 2015)
 *          Biljana Bojovic <biljana.bojovic@cttc.es> (Carrier Aggregation)
 *          NIST (D2D)
 */

#ifndef CV2X_RRC_HEADER_H
#define CV2X_RRC_HEADER_H

#include "ns3/header.h"

#include <bitset>
#include <string>

#include "ns3/cv2x_lte-rrc-sap.h"
#include "ns3/cv2x_lte-asn1-header.h"

namespace ns3 {

/**
 * \ingroup lte
 */

/**
 * This class extends cv2x_Asn1Header functions, adding serialization/deserialization
 * of some Information elements defined in 3GPP TS 36.331
 */
class cv2x_RrcAsn1Header : public cv2x_Asn1Header
{
public:
  cv2x_RrcAsn1Header ();
  /**
   * Get message type
   *
   * \returns the message type
   */
  int GetMessageType ();

protected:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  // Inherited from cv2x_Asn1Header
  virtual TypeId GetInstanceTypeId (void) const;
  uint32_t Deserialize (Buffer::Iterator bIterator) = 0;
  virtual void PreSerialize (void) const = 0;


  // Auxiliary functions
  /**
   * Convert from bandwidth (in RBs) to ENUMERATED value
   *
   * \param bandwidth Bandwidth in RBs: 6, 15, 25, 50, 75, 100
   * \returns ENUMERATED value: 0, 1, 2, 3, 4, 5
   */
  int BandwidthToEnum (uint8_t bandwidth) const;
  /**
   * Convert from ENUMERATED value to bandwidth (in RBs)
   *
   * \param n ENUMERATED value: 0, 1, 2, 3, 4, 5
   * \returns bandwidth Bandwidth in RBs: 6, 15, 25, 50, 75, 100
   */
  uint8_t EnumToBandwidth (int n) const;


  // Serialization functions
  /**
   * Serialize SRB to add mod list function
   *
   * \param srbToAddModList std::list<cv2x_LteRrcSap::SrbToAddMod>
   */
  void SerializeSrbToAddModList (std::list<cv2x_LteRrcSap::SrbToAddMod> srbToAddModList) const;
  /**
   * Serialize DRB to add mod list function
   *
   * \param drbToAddModList std::list<cv2x_LteRrcSap::SrbToAddMod>
   */
  void SerializeDrbToAddModList (std::list<cv2x_LteRrcSap::DrbToAddMod> drbToAddModList) const;
  /**
   * Serialize logicala channel config function
   *
   * \param logicalChannelConfig cv2x_LteRrcSap::LogicalChannelConfig
   */
  void SerializeLogicalChannelConfig (cv2x_LteRrcSap::LogicalChannelConfig logicalChannelConfig) const;
  /**
   * Serialize radio resource config function
   *
   * \param radioResourceConfigDedicated cv2x_LteRrcSap::RadioResourceConfigDedicated
   */
  void SerializeRadioResourceConfigDedicated (cv2x_LteRrcSap::RadioResourceConfigDedicated radioResourceConfigDedicated) const;
  /**
   * Serialize physical config dedicated function
   *
   * \param physicalConfigDedicated cv2x_LteRrcSap::PhysicalConfigDedicated
   */
  void SerializePhysicalConfigDedicated (cv2x_LteRrcSap::PhysicalConfigDedicated physicalConfigDedicated) const;
  /**
   * Serialize physical config dedicated function
   *
   * \param pcdsc cv2x_LteRrcSap::PhysicalConfigDedicatedSCell
   */
  void SerializePhysicalConfigDedicatedSCell (cv2x_LteRrcSap::PhysicalConfigDedicatedSCell pcdsc) const;
  /**
   * Serialize system information block type 1 function
   *
   * \param systemInformationBlockType1 cv2x_LteRrcSap::SystemInformationBlockType1
   */
  void SerializeSystemInformationBlockType1 (cv2x_LteRrcSap::SystemInformationBlockType1 systemInformationBlockType1) const;
  /**
   * Serialize system information block type 2 function
   *
   * \param systemInformationBlockType2 cv2x_LteRrcSap::SystemInformationBlockType2
   */
  void SerializeSystemInformationBlockType2 (cv2x_LteRrcSap::SystemInformationBlockType2 systemInformationBlockType2) const;
  /**
   * Serialize system information block type 2 function
   *
   * \param radioResourceConfigCommon cv2x_LteRrcSap::RadioResourceConfigCommon
   */
  void SerializeRadioResourceConfigCommon (cv2x_LteRrcSap::RadioResourceConfigCommon radioResourceConfigCommon) const;
  /**
   * Serialize radio resource config common SIB function
   *
   * \param radioResourceConfigCommonSib cv2x_LteRrcSap::RadioResourceConfigCommonSib
   */
  void SerializeRadioResourceConfigCommonSib (cv2x_LteRrcSap::RadioResourceConfigCommonSib radioResourceConfigCommonSib) const;
  /**
   * Serialize measure results function
   *
   * \param measResults cv2x_LteRrcSap::MeasResults
   */
  void SerializeMeasResults (cv2x_LteRrcSap::MeasResults measResults) const;
  /**
   * Serialize PLMN identity function
   *
   * \param plmnId the PLMN ID
   */
  void SerializePlmnIdentity (uint32_t plmnId) const;
  /**
   * Serialize RACH config common function
   *
   * \param rachConfigCommon cv2x_LteRrcSap::RachConfigCommon
   */
  void SerializeRachConfigCommon (cv2x_LteRrcSap::RachConfigCommon rachConfigCommon) const;
  /**
   * Serialize measure config function
   *
   * \param measConfig cv2x_LteRrcSap::MeasConfig
   */
  void SerializeMeasConfig (cv2x_LteRrcSap::MeasConfig measConfig) const;
  /**
   * Serialize non critical extension config function
   *
   * \param nonCriticalExtensionConfiguration cv2x_LteRrcSap::NonCriticalExtensionConfiguration
   */
  void SerializeNonCriticalExtensionConfiguration (cv2x_LteRrcSap::NonCriticalExtensionConfiguration nonCriticalExtensionConfiguration) const;
  /**
   * Serialize radio resource config common SCell function
   *
   * \param rrccsc cv2x_LteRrcSap::RadioResourceConfigCommonSCell
   */
  void SerializeRadioResourceConfigCommonSCell (cv2x_LteRrcSap::RadioResourceConfigCommonSCell rrccsc) const;
  /**
   * Serialize radio resource dedicated SCell function
   *
   * \param rrcdsc cv2x_LteRrcSap::RadioResourceConfigDedicatedSCell
   */
  void SerializeRadioResourceDedicatedSCell (cv2x_LteRrcSap::RadioResourceConfigDedicatedSCell rrcdsc) const;
  /**
   * Serialize Q offset range function
   *
   * \param qOffsetRange q offset range
   */
  void SerializeQoffsetRange (int8_t qOffsetRange) const;
  /**
   * Serialize threshold eutra function
   *
   * \param thresholdEutra cv2x_LteRrcSap::ThresholdEutra
   */
  void SerializeThresholdEutra (cv2x_LteRrcSap::ThresholdEutra thresholdEutra) const;
  /**
   * Serialize sidelink ue information function
   *
   * \param slUeInfo cv2x_LteRrcSap::SidelinkUeInformation
   */
  void SerializeSidelinkUeInformation (cv2x_LteRrcSap::SidelinkUeInformation slUeInfo) const;
  
  
  // Deserialization functions
  /**
   * Deserialize DRB to add mod list function
   *
   * \param drbToAddModLis std::list<cv2x_LteRrcSap::DrbToAddMod> *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeDrbToAddModList (std::list<cv2x_LteRrcSap::DrbToAddMod> *drbToAddModLis, Buffer::Iterator bIterator);
  /**
   * Deserialize SRB to add mod list function
   *
   * \param srbToAddModList std::list<cv2x_LteRrcSap::SrbToAddMod> *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeSrbToAddModList (std::list<cv2x_LteRrcSap::SrbToAddMod> *srbToAddModList, Buffer::Iterator bIterator);
  /**
   * Deserialize logical channel config function
   *
   * \param logicalChannelConfig cv2x_LteRrcSap::LogicalChannelConfig *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeLogicalChannelConfig (cv2x_LteRrcSap::LogicalChannelConfig *logicalChannelConfig, Buffer::Iterator bIterator);
  /**
   * Deserialize radio resource config dedicated function
   *
   * \param radioResourceConfigDedicated cv2x_LteRrcSap::RadioResourceConfigDedicated *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeRadioResourceConfigDedicated (cv2x_LteRrcSap::RadioResourceConfigDedicated *radioResourceConfigDedicated, Buffer::Iterator bIterator);
  /**
   * Deserialize physical config dedicated function
   *
   * \param physicalConfigDedicated cv2x_LteRrcSap::PhysicalConfigDedicated *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializePhysicalConfigDedicated (cv2x_LteRrcSap::PhysicalConfigDedicated *physicalConfigDedicated, Buffer::Iterator bIterator);
  /**
   * Deserialize system information block type 1 function
   *
   * \param systemInformationBlockType1 cv2x_LteRrcSap::SystemInformationBlockType1 *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeSystemInformationBlockType1 (cv2x_LteRrcSap::SystemInformationBlockType1 *systemInformationBlockType1, Buffer::Iterator bIterator);
  /**
   * Deserialize system information block type 2 function
   *
   * \param systemInformationBlockType2 cv2x_LteRrcSap::SystemInformationBlockType2 *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeSystemInformationBlockType2 (cv2x_LteRrcSap::SystemInformationBlockType2 *systemInformationBlockType2, Buffer::Iterator bIterator);
  /**
   * Deserialize radio resource config common function
   *
   * \param radioResourceConfigCommon cv2x_LteRrcSap::RadioResourceConfigCommon *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeRadioResourceConfigCommon (cv2x_LteRrcSap::RadioResourceConfigCommon *radioResourceConfigCommon, Buffer::Iterator bIterator);
  /**
   * Deserialize radio resource config common SIB function
   *
   * \param radioResourceConfigCommonSib cv2x_LteRrcSap::RadioResourceConfigCommonSib *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeRadioResourceConfigCommonSib (cv2x_LteRrcSap::RadioResourceConfigCommonSib *radioResourceConfigCommonSib, Buffer::Iterator bIterator);
  /**
   * Deserialize measure results function
   *
   * \param measResults cv2x_LteRrcSap::MeasResults *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeMeasResults (cv2x_LteRrcSap::MeasResults *measResults, Buffer::Iterator bIterator);
  /**
   * Deserialize PLMN identity function
   *
   * \param plmnId the PLMN ID
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializePlmnIdentity (uint32_t *plmnId, Buffer::Iterator bIterator);
  /**
   * Deserialize RACH config common function
   *
   * \param rachConfigCommon cv2x_LteRrcSap::RachConfigCommon *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeRachConfigCommon (cv2x_LteRrcSap::RachConfigCommon * rachConfigCommon, Buffer::Iterator bIterator);
  /**
   * Deserialize measure config function
   *
   * \param measConfig cv2x_LteRrcSap::MeasConfig *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeMeasConfig (cv2x_LteRrcSap::MeasConfig * measConfig, Buffer::Iterator bIterator);
  /**
   * Deserialize Qoffset range function
   *
   * \param qOffsetRange Qoffset range
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeQoffsetRange (int8_t * qOffsetRange, Buffer::Iterator bIterator);
  /**
   * Deserialize threshold eutra function
   *
   * \param thresholdEutra cv2x_LteRrcSap::ThresholdEutra *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeThresholdEutra (cv2x_LteRrcSap::ThresholdEutra * thresholdEutra, Buffer::Iterator bIterator);
  /**
   * Deserialize non critical extension config function
   *
   * \param nonCriticalExtension cv2x_LteRrcSap::NonCriticalExtensionConfiguration *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeNonCriticalExtensionConfig (cv2x_LteRrcSap::NonCriticalExtensionConfiguration * nonCriticalExtension, Buffer::Iterator bIterator);
  /**
   * Deserialize cell identification function
   *
   * \param ci cv2x_LteRrcSap::CellIdentification *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeCellIdentification (cv2x_LteRrcSap::CellIdentification * ci, Buffer::Iterator bIterator);
  /**
   * Deserialize radio resource config common SCell function
   *
   * \param rrccsc cv2x_LteRrcSap::RadioResourceConfigCommonSCell *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeRadioResourceConfigCommonSCell (cv2x_LteRrcSap::RadioResourceConfigCommonSCell * rrccsc, Buffer::Iterator bIterator);
  /**
   * Deserialize radio resource config dedicated SCell function
   *
   * \param rrcdsc cv2x_LteRrcSap::RadioResourceConfigDedicatedSCell *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeRadioResourceConfigDedicatedSCell (cv2x_LteRrcSap::RadioResourceConfigDedicatedSCell * rrcdsc, Buffer::Iterator bIterator);
  /**
   * Deserialize physical config dedicated SCell function
   *
   * \param pcdsc cv2x_LteRrcSap::PhysicalConfigDedicatedSCell *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializePhysicalConfigDedicatedSCell (cv2x_LteRrcSap::PhysicalConfigDedicatedSCell *pcdsc, Buffer::Iterator bIterator);
  /**
   * Deserialize sidelink ue information function
   *
   * \param slInfo cv2x_LteRrcSap::SidelinkUeInformation *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeSidelinkUeInformation (cv2x_LteRrcSap::SidelinkUeInformation *slInfo, Buffer::Iterator bIterator);


  /**
   * This function prints the object, for debugging purposes.
   * @param os The output stream to use (i.e. std::cout)
   */
  void Print (std::ostream &os) const;
  /**
   * This function prints RadioResourceConfigDedicated IE, for debugging purposes.
   * @param os The output stream to use (i.e. std::cout)
   * @param radioResourceConfigDedicated The information element to be printed
   */
  void Print (std::ostream &os, cv2x_LteRrcSap::RadioResourceConfigDedicated radioResourceConfigDedicated) const;

  /// Stores RRC message type, according to 3GPP TS 36.331
  int m_messageType;
};


/**
 * This class only serves to discriminate which message type has been received
 * in uplink (ue to eNb) for channel DCCH
 */
class cv2x_RrcUlDcchMessage : public cv2x_RrcAsn1Header
{
public:
  cv2x_RrcUlDcchMessage ();
  ~cv2x_RrcUlDcchMessage ();

  // Inherited from cv2x_RrcAsn1Header
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;
  void PreSerialize () const;

protected:
  /**
   * Serialize UL DCCH message function
   *
   * \param msgType message type
   */
  void SerializeUlDcchMessage (int msgType) const;
  /**
   * Deserialize UL DCCH message function
   *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeUlDcchMessage (Buffer::Iterator bIterator);
};

/**
 * This class only serves to discriminate which message type has been received
 * in downlink (eNb to ue) for channel DCCH
 */
class cv2x_RrcDlDcchMessage : public cv2x_RrcAsn1Header
{
public:
  cv2x_RrcDlDcchMessage ();
  ~cv2x_RrcDlDcchMessage ();

  // Inherited from cv2x_RrcAsn1Header
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;
  void PreSerialize () const;

protected:
  /**
   * Serialize DL DCCH message function
   *
   * \param msgType message type
   */
  void SerializeDlDcchMessage (int msgType) const;
  /**
   * Deserialize DL DCCH message function
   *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeDlDcchMessage (Buffer::Iterator bIterator);
};

/**
 * This class only serves to discriminate which message type has been received
 * in uplink (ue to eNb) for channel CCCH
 */
class cv2x_RrcUlCcchMessage : public cv2x_RrcAsn1Header
{
public:
  cv2x_RrcUlCcchMessage ();
  ~cv2x_RrcUlCcchMessage ();

  // Inherited from cv2x_RrcAsn1Header 
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;
  void PreSerialize () const;

protected:
  /**
   * Serialize UL CCCH message function
   *
   * \param msgType message type
   */
  void SerializeUlCcchMessage (int msgType) const;
  /**
   * Deserialize DL CCCH message function
   *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeUlCcchMessage (Buffer::Iterator bIterator);
};

/**
 * This class only serves to discriminate which message type has been received
 * in downlink (eNb to ue) for channel CCCH
 */
class cv2x_RrcDlCcchMessage : public cv2x_RrcAsn1Header
{
public:
  cv2x_RrcDlCcchMessage ();
  ~cv2x_RrcDlCcchMessage ();

  // Inherited from cv2x_RrcAsn1Header 
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;
  void PreSerialize () const;

protected:
  /**
   * Serialize DL CCCH message function
   *
   * \param msgType message type
   */
  void SerializeDlCcchMessage (int msgType) const;
  /**
   * Deserialize DL CCCH message function
   *
   * \param bIterator buffer iterator
   * \returns buffer iterator
   */
  Buffer::Iterator DeserializeDlCcchMessage (Buffer::Iterator bIterator);
};

/**
* This class manages the serialization/deserialization of RrcConnectionRequest IE
*/
class cv2x_RrcConnectionRequestHeader : public cv2x_RrcUlCcchMessage
{
public:
  cv2x_RrcConnectionRequestHeader ();
  ~cv2x_RrcConnectionRequestHeader ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
   * Receives a RrcConnectionRequest IE and stores the contents into the class attributes
   * @param msg The information element to parse
   */
  void SetMessage (cv2x_LteRrcSap::RrcConnectionRequest msg);

  /**
   * Returns a RrcConnectionRequest IE from the values in the class attributes
   * @return A RrcConnectionRequest, as defined in cv2x_LteRrcSap
   */
  cv2x_LteRrcSap::RrcConnectionRequest GetMessage () const;

  /**
   * Get MMEC attribute
   * @return m_mmec attribute
   */
  std::bitset<8> GetMmec () const;

  /**
   * Get M-TMSI attribute
   * @return m_tmsi attribute
   */
  std::bitset<32> GetMtmsi () const;

private:
  std::bitset<8> m_mmec; ///< MMEC
  std::bitset<32> m_mTmsi; ///< TMSI
  /// EstablishmentCause enumeration
  enum
  {
    EMERGENCY = 0, HIGHPRIORITYACCESS, MT_ACCESS,
    MO_SIGNALLING, MO_DATA, SPARE3, SPARE2, SPARE1
  } m_establishmentCause; ///< the establishent cause
  std::bitset<1> m_spare; ///< spare bit
};

/**
* This class manages the serialization/deserialization of RrcConnectionSetup IE
*/
class cv2x_RrcConnectionSetupHeader : public cv2x_RrcDlCcchMessage
{
public:
  cv2x_RrcConnectionSetupHeader ();
  ~cv2x_RrcConnectionSetupHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionSetup IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */
  void SetMessage (cv2x_LteRrcSap::RrcConnectionSetup msg);

  /**
  * Returns a RrcConnectionSetup IE from the values in the class attributes
  * @return A RrcConnectionSetup, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::RrcConnectionSetup GetMessage () const;

  /**
  * Getter for m_rrcTransactionIdentifier
  * @return m_rrcTransactionIdentifier
  */
  uint8_t GetRrcTransactionIdentifier () const;

  /**
  * Getter for m_radioResourceConfigDedicated
  * @return m_radioResourceConfigDedicated
  */
  cv2x_LteRrcSap::RadioResourceConfigDedicated GetRadioResourceConfigDedicated () const; 

  /**
  * Gets m_radioResourceConfigDedicated.havePhysicalConfigDedicated
  * @return m_radioResourceConfigDedicated.havePhysicalConfigDedicated
  */
  bool HavePhysicalConfigDedicated () const;

  /**
  * Gets m_radioResourceConfigDedicated.physicalConfigDedicated
  * @return m_radioResourceConfigDedicated.physicalConfigDedicated
  */
  cv2x_LteRrcSap::PhysicalConfigDedicated GetPhysicalConfigDedicated () const;

  /**
  * Gets m_radioResourceConfigDedicated.srbToAddModList
  * @return m_radioResourceConfigDedicated.srbToAddModList
  */
  std::list<cv2x_LteRrcSap::SrbToAddMod> GetSrbToAddModList () const;

  /**
  * Gets m_radioResourceConfigDedicated.drbToAddModList
  * @return m_radioResourceConfigDedicated.drbToAddModList
  */
  std::list<cv2x_LteRrcSap::DrbToAddMod> GetDrbToAddModList () const;

  /**
  * Gets m_radioResourceConfigDedicated.drbToReleaseList
  * @return m_radioResourceConfigDedicated.drbToReleaseList
  */
  std::list<uint8_t> GetDrbToReleaseList () const;

private:
  uint8_t m_rrcTransactionIdentifier; ///< RRC transaction identifier
  mutable cv2x_LteRrcSap::RadioResourceConfigDedicated m_radioResourceConfigDedicated; ///< radio resource config dedicated
};

/**
* This class manages the serialization/deserialization of RrcConnectionSetupComplete IE
*/
class cv2x_RrcConnectionSetupCompleteHeader : public cv2x_RrcUlDcchMessage
{
public:
  cv2x_RrcConnectionSetupCompleteHeader ();
  ~cv2x_RrcConnectionSetupCompleteHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionSetupCompleted IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */
  void SetMessage (cv2x_LteRrcSap::RrcConnectionSetupCompleted msg);

  /**
  * Returns a RrcConnectionSetupCompleted IE from the values in the class attributes
  * @return A RrcConnectionSetupCompleted, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::RrcConnectionSetupCompleted GetMessage () const;

  /**
  * Getter for m_rrcTransactionIdentifier
  * @return m_rrcTransactionIdentifier
  */
  uint8_t GetRrcTransactionIdentifier () const;

private:
  uint8_t m_rrcTransactionIdentifier; ///< RRC transaction identifier

};

/**
* This class manages the serialization/deserialization of RrcConnectionSetupComplete IE
*/
class cv2x_RrcConnectionReconfigurationCompleteHeader : public cv2x_RrcUlDcchMessage
{
public:
  cv2x_RrcConnectionReconfigurationCompleteHeader ();
  ~cv2x_RrcConnectionReconfigurationCompleteHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReconfigurationCompleted IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */
  void SetMessage (cv2x_LteRrcSap::RrcConnectionReconfigurationCompleted msg);

  /**
  * Returns a RrcConnectionReconfigurationCompleted IE from the values in the class attributes
  * @return A RrcConnectionReconfigurationCompleted, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::RrcConnectionReconfigurationCompleted GetMessage () const;

  /**
  * Getter for m_rrcTransactionIdentifier
  * @return m_rrcTransactionIdentifier
  */ 
  uint8_t GetRrcTransactionIdentifier () const;

private:
  uint8_t m_rrcTransactionIdentifier; ///< RRC transaction identifier

};

/**
* This class manages the serialization/deserialization of RrcConnectionReconfiguration IE
*/
class cv2x_RrcConnectionReconfigurationHeader : public cv2x_RrcDlDcchMessage
{
public:
  cv2x_RrcConnectionReconfigurationHeader ();
  ~cv2x_RrcConnectionReconfigurationHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReconfiguration IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (cv2x_LteRrcSap::RrcConnectionReconfiguration msg);

  /**
  * Returns a RrcConnectionReconfiguration IE from the values in the class attributes
  * @return A RrcConnectionReconfiguration, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::RrcConnectionReconfiguration GetMessage () const; 

  /**
  * Getter for m_haveMeasConfig
  * @return m_haveMeasConfig
  */
  bool GetHaveMeasConfig ();

  /**
  * Getter for m_measConfig
  * @return m_measConfig
  */
  cv2x_LteRrcSap::MeasConfig GetMeasConfig ();

  /**
  * Getter for m_haveMobilityControlInfo
  * @return m_haveMobilityControlInfo
  */
  bool GetHaveMobilityControlInfo ();

  /**
  * Getter for m_mobilityControlInfo
  * @return m_mobilityControlInfo
  */
  cv2x_LteRrcSap::MobilityControlInfo GetMobilityControlInfo ();

  /**
  * Getter for m_haveRadioResourceConfigDedicated
  * @return m_haveRadioResourceConfigDedicated
  */
  bool GetHaveRadioResourceConfigDedicated ();

  /**
  * Getter for m_radioResourceConfigDedicated
  * @return m_radioResourceConfigDedicated
  */
  cv2x_LteRrcSap::RadioResourceConfigDedicated GetRadioResourceConfigDedicated ();

  /**
  * Getter for m_rrcTransactionIdentifier
  * @return m_rrcTransactionIdentifier
  */
  uint8_t GetRrcTransactionIdentifier () const;

  /**
  * Getter for m_radioResourceConfigDedicated
  * @return m_radioResourceConfigDedicated
  */
  cv2x_LteRrcSap::RadioResourceConfigDedicated GetRadioResourceConfigDedicated () const; 

  /**
  * Getter for m_haveNonCriticalExtension
  * @return m_haveNonCriticalExtension
  */
  bool GetHaveNonCriticalExtensionConfig ();

  /**
  * Getter for m_nonCriticalExtension
  * @return m_nonCriticalExtension
  */
  cv2x_LteRrcSap::NonCriticalExtensionConfiguration GetNonCriticalExtensionConfig ();  

  /**
  * Gets m_radioResourceConfigDedicated.havePhysicalConfigDedicated
  * @return m_radioResourceConfigDedicated.havePhysicalConfigDedicated
  */
  bool HavePhysicalConfigDedicated () const;

  /**
  * Gets m_radioResourceConfigDedicated.physicalConfigDedicated
  * @return m_radioResourceConfigDedicated.physicalConfigDedicated
  */
  cv2x_LteRrcSap::PhysicalConfigDedicated GetPhysicalConfigDedicated () const;

  /**
  * Gets m_radioResourceConfigDedicated.srbToAddModList
  * @return m_radioResourceConfigDedicated.srbToAddModList
  */
  std::list<cv2x_LteRrcSap::SrbToAddMod> GetSrbToAddModList () const;

  /**
  * Gets m_radioResourceConfigDedicated.drbToAddModList
  * @return m_radioResourceConfigDedicated.drbToAddModList
  */
  std::list<cv2x_LteRrcSap::DrbToAddMod> GetDrbToAddModList () const;

  /**
  * Gets m_radioResourceConfigDedicated.drbToReleaseList
  * @return m_radioResourceConfigDedicated.drbToReleaseList
  */
  std::list<uint8_t> GetDrbToReleaseList () const;

private:
  uint8_t m_rrcTransactionIdentifier; ///< RRC transaction identifier
  bool m_haveMeasConfig; ///< have measure config?
  cv2x_LteRrcSap::MeasConfig m_measConfig; ///< the measure config
  bool m_haveMobilityControlInfo; ///< have mobility control info?
  cv2x_LteRrcSap::MobilityControlInfo m_mobilityControlInfo; ///< the modility control info
  bool m_haveRadioResourceConfigDedicated; ///< have radio resource config dedicated?
  cv2x_LteRrcSap::RadioResourceConfigDedicated m_radioResourceConfigDedicated; ///< the radio resource config dedicated
  bool m_haveNonCriticalExtension; ///< Have critical extension
  cv2x_LteRrcSap::NonCriticalExtensionConfiguration m_nonCriticalExtension; ///< the critical extension
};

/**
* This class manages the serialization/deserialization of HandoverPreparationInfo IE
*/
class cv2x_HandoverPreparationInfoHeader : public cv2x_RrcAsn1Header
{
public:
  cv2x_HandoverPreparationInfoHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a HandoverPreparationInfo IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (cv2x_LteRrcSap::HandoverPreparationInfo msg);

  /**
  * Returns a HandoverPreparationInfo IE from the values in the class attributes
  * @return A HandoverPreparationInfo, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::HandoverPreparationInfo GetMessage () const;

  /**
  * Getter for m_asConfig
  * @return m_asConfig
  */ 
  cv2x_LteRrcSap::AsConfig GetAsConfig () const;

private:
  cv2x_LteRrcSap::AsConfig m_asConfig; ///< AS config
};

/**
* This class manages the serialization/deserialization of RRCConnectionReestablishmentRequest IE
*/
class cv2x_RrcConnectionReestablishmentRequestHeader : public cv2x_RrcUlCcchMessage
{
public:
  cv2x_RrcConnectionReestablishmentRequestHeader ();
  ~cv2x_RrcConnectionReestablishmentRequestHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReestablishmentRequest IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (cv2x_LteRrcSap::RrcConnectionReestablishmentRequest msg);

  /**
  * Returns a RrcConnectionReestablishmentRequest IE from the values in the class attributes
  * @return A RrcConnectionReestablishmentRequest, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::RrcConnectionReestablishmentRequest GetMessage () const;

  /**
  * Getter for m_ueIdentity
  * @return m_ueIdentity
  */
  cv2x_LteRrcSap::ReestabUeIdentity GetUeIdentity () const;

  /**
  * Getter for m_reestablishmentCause
  * @return m_reestablishmentCause
  */
  cv2x_LteRrcSap::ReestablishmentCause GetReestablishmentCause () const;

private:
  cv2x_LteRrcSap::ReestabUeIdentity m_ueIdentity; ///< UE identity
  cv2x_LteRrcSap::ReestablishmentCause m_reestablishmentCause; ///< reestablishment cause
};

/**
* This class manages the serialization/deserialization of RrcConnectionReestablishment IE
*/
class cv2x_RrcConnectionReestablishmentHeader : public cv2x_RrcDlCcchMessage
{
public:
  cv2x_RrcConnectionReestablishmentHeader ();
  ~cv2x_RrcConnectionReestablishmentHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReestablishment IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (cv2x_LteRrcSap::RrcConnectionReestablishment msg);

  /**
  * Returns a RrcConnectionReestablishment IE from the values in the class attributes
  * @return A RrcConnectionReestablishment, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::RrcConnectionReestablishment GetMessage () const;

  /**
  * Getter for m_rrcTransactionIdentifier attribute
  * @return m_rrcTransactionIdentifier
  */
  uint8_t GetRrcTransactionIdentifier () const;

  /**
  * Getter for m_radioResourceConfigDedicated attribute
  * @return m_radioResourceConfigDedicated
  */
  cv2x_LteRrcSap::RadioResourceConfigDedicated GetRadioResourceConfigDedicated () const;

private:
  uint8_t m_rrcTransactionIdentifier; ///< RRC transaction identifier
  cv2x_LteRrcSap::RadioResourceConfigDedicated m_radioResourceConfigDedicated; ///< radio resource config dedicated
};

/**
* This class manages the serialization/deserialization of RrcConnectionReestablishmentComplete IE
*/
class cv2x_RrcConnectionReestablishmentCompleteHeader : public cv2x_RrcUlDcchMessage
{
public:
  cv2x_RrcConnectionReestablishmentCompleteHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReestablishmentComplete IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (cv2x_LteRrcSap::RrcConnectionReestablishmentComplete msg);

  /**
  * Returns a RrcConnectionReestablishmentComplete IE from the values in the class attributes
  * @return A RrcConnectionReestablishmentComplete, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::RrcConnectionReestablishmentComplete GetMessage () const;

  /**
  * Getter for m_rrcTransactionIdentifier attribute
  * @return m_rrcTransactionIdentifier
  */
  uint8_t GetRrcTransactionIdentifier () const;

private:
  uint8_t m_rrcTransactionIdentifier; ///< RRC transaction identifier
};

/**
* This class manages the serialization/deserialization of RrcConnectionReestablishmentReject IE
*/
class cv2x_RrcConnectionReestablishmentRejectHeader : public cv2x_RrcDlCcchMessage
{
public:
  cv2x_RrcConnectionReestablishmentRejectHeader ();
  ~cv2x_RrcConnectionReestablishmentRejectHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReestablishmentReject IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (cv2x_LteRrcSap::RrcConnectionReestablishmentReject msg);

  /**
  * Returns a RrcConnectionReestablishmentReject IE from the values in the class attributes
  * @return A RrcConnectionReestablishmentReject, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::RrcConnectionReestablishmentReject GetMessage () const;

private:
  cv2x_LteRrcSap::RrcConnectionReestablishmentReject m_rrcConnectionReestablishmentReject; ///< RRC connection reestablishmnet reject
};

/**
* This class manages the serialization/deserialization of RrcConnectionRelease IE
*/
class cv2x_RrcConnectionReleaseHeader : public cv2x_RrcDlDcchMessage
{
public:
  cv2x_RrcConnectionReleaseHeader ();
  ~cv2x_RrcConnectionReleaseHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionRelease IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (cv2x_LteRrcSap::RrcConnectionRelease msg);

  /**
  * Returns a RrcConnectionRelease IE from the values in the class attributes
  * @return A RrcConnectionRelease, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::RrcConnectionRelease GetMessage () const;

private:
  cv2x_LteRrcSap::RrcConnectionRelease m_rrcConnectionRelease; ///< RRC connection release
};

/**
* This class manages the serialization/deserialization of RrcConnectionReject IE
*/
class cv2x_RrcConnectionRejectHeader : public cv2x_RrcDlCcchMessage
{
public:
  cv2x_RrcConnectionRejectHeader ();
  ~cv2x_RrcConnectionRejectHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a RrcConnectionReject IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */ 
  void SetMessage (cv2x_LteRrcSap::RrcConnectionReject msg);

  /**
  * Returns a RrcConnectionReject IE from the values in the class attributes
  * @return A RrcConnectionReject, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::RrcConnectionReject GetMessage () const;

private:
  cv2x_LteRrcSap::RrcConnectionReject m_rrcConnectionReject; ///< RRC connection reject
};

/**
* This class manages the serialization/deserialization of MeasurementReport IE
*/
class cv2x_MeasurementReportHeader : public cv2x_RrcUlDcchMessage
{
public:
  cv2x_MeasurementReportHeader ();
  ~cv2x_MeasurementReportHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a MeasurementReport IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */
  void SetMessage (cv2x_LteRrcSap::MeasurementReport msg);

  /**
  * Returns a MeasurementReport IE from the values in the class attributes
  * @return A MeasurementReport, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::MeasurementReport GetMessage () const;

private:
  cv2x_LteRrcSap::MeasurementReport m_measurementReport; ///< measurement report

};


/**
* This class manages the serialization/deserialization of MeasurementReport IE
*/
class cv2x_SidelinkUeInformationHeader : public cv2x_RrcUlDcchMessage
{
public:
  cv2x_SidelinkUeInformationHeader ();
  ~cv2x_SidelinkUeInformationHeader ();

  // Inherited from cv2x_RrcAsn1Header 
  void PreSerialize () const;
  uint32_t Deserialize (Buffer::Iterator bIterator);
  void Print (std::ostream &os) const;

  /**
  * Receives a SidelinkUeInformation IE and stores the contents into the class attributes
  * @param msg The information element to parse
  */
  void SetMessage (cv2x_LteRrcSap::SidelinkUeInformation msg);

  /**
  * Returns a SidelinkUeInformation IE from the values in the class attributes
  * @return A SidelinkUeInformation, as defined in cv2x_LteRrcSap
  */
  cv2x_LteRrcSap::SidelinkUeInformation GetMessage () const;

private:
  cv2x_LteRrcSap::SidelinkUeInformation m_sidelinkUeInformation;

};

} // namespace ns3

#endif // CV2X_RRC_HEADER_H


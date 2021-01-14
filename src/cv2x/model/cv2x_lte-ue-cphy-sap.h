/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011, 2012 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
 * Author: Nicola Baldo <nbaldo@cttc.es>,
 *         Marco Miozzo <mmiozzo@cttc.es>
 * Modified by: NIST (D2D)
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#ifndef CV2X_LTE_UE_CPHY_SAP_H
#define CV2X_LTE_UE_CPHY_SAP_H

#include <stdint.h>
#include <ns3/ptr.h>

#include <ns3/cv2x_lte-rrc-sap.h>
#include <ns3/cv2x_sl-pool.h>

namespace ns3 {


class cv2x_LteEnbNetDevice;

/**
 * Service Access Point (SAP) offered by the UE PHY to the UE RRC for control purposes
 *
 * This is the PHY SAP Provider, i.e., the part of the SAP that contains
 * the PHY methods called by the MAC
 */
class cv2x_LteUeCphySapProvider
{
public:

  /** 
   * destructor
   */
  virtual ~cv2x_LteUeCphySapProvider ();

  /** 
   * reset the PHY
   * 
   */
  virtual void Reset () = 0;

  /**
   * \brief Tell the PHY entity to listen to PSS from surrounding cells and
   *        measure the RSRP.
   * \param dlEarfcn the downlink carrier frequency (EARFCN) to listen to
   *
   * This function will instruct this PHY instance to listen to the DL channel
   * over the bandwidth of 6 RB at the frequency associated with the given
   * EARFCN.
   *
   * After this, it will start receiving Primary Synchronization Signal (PSS)
   * and periodically returning measurement reports to RRC via
   * cv2x_LteUeCphySapUser::ReportUeMeasurements function.
   */
  virtual void StartCellSearch (uint32_t dlEarfcn) = 0;

  /**
   * \brief Tell the PHY entity to synchronize with a given eNodeB over the
   *        currently active EARFCN for communication purposes.
   * \param cellId the ID of the eNodeB to synchronize with
   *
   * By synchronizing, the PHY will start receiving various information
   * transmitted by the eNodeB. For instance, when receiving system information,
   * the message will be relayed to RRC via
   * cv2x_LteUeCphySapUser::RecvMasterInformationBlock and
   * cv2x_LteUeCphySapUser::RecvSystemInformationBlockType1 functions.
   *
   * Initially, the PHY will be configured to listen to 6 RBs of BCH.
   * cv2x_LteUeCphySapProvider::SetDlBandwidth can be called afterwards to increase
   * the bandwidth.
   */
  virtual void SynchronizeWithEnb (uint16_t cellId) = 0;

  /**
   * \brief Tell the PHY entity to align to the given EARFCN and synchronize
   *        with a given eNodeB for communication purposes.
   * \param cellId the ID of the eNodeB to synchronize with
   * \param dlEarfcn the downlink carrier frequency (EARFCN)
   *
   * By synchronizing, the PHY will start receiving various information
   * transmitted by the eNodeB. For instance, when receiving system information,
   * the message will be relayed to RRC via
   * cv2x_LteUeCphySapUser::RecvMasterInformationBlock and
   * cv2x_LteUeCphySapUser::RecvSystemInformationBlockType1 functions.
   *
   * Initially, the PHY will be configured to listen to 6 RBs of BCH.
   * cv2x_LteUeCphySapProvider::SetDlBandwidth can be called afterwards to increase
   * the bandwidth.
   */
  virtual void SynchronizeWithEnb (uint16_t cellId, uint32_t dlEarfcn) = 0;

  /**
   * \param dlBandwidth the DL bandwidth in number of PRBs
   */
  virtual void SetDlBandwidth (uint8_t dlBandwidth) = 0;

  /** 
   * \brief Configure uplink (normally done after reception of SIB2)
   * 
   * \param ulEarfcn the uplink carrier frequency (EARFCN)
   * \param ulBandwidth the UL bandwidth in number of PRBs
   */
  virtual void ConfigureUplink (uint32_t ulEarfcn, uint8_t ulBandwidth) = 0;

  /**
   * \brief Configure referenceSignalPower
   *
   * \param referenceSignalPower received from eNB in SIB2
   */
  virtual void ConfigureReferenceSignalPower (int8_t referenceSignalPower) = 0;

  /** 
   * 
   * \param rnti the cell-specific UE identifier
   */
  virtual void SetRnti (uint16_t rnti) = 0;

  /**
   * \param txMode the transmissionMode of the user
   */
  virtual void SetTransmissionMode (uint8_t txMode) = 0;

  /**
   * \param srcCi the SRS configuration index
   */
  virtual void SetSrsConfigurationIndex (uint16_t srcCi) = 0;

  /**
   * \param pa the P_A value
   */
  virtual void SetPa (double pa) = 0;

//discovery
  /**
   * set the current discovery transmit pool
   * \param pool the transmission pool
   */
  virtual void SetSlTxPool (Ptr<SidelinkTxDiscResourcePool> pool) = 0;

  /**
   * remove the discovery transmission pool 
   */
  virtual void RemoveSlTxPool (bool disc) = 0;

  /**
   * set the discovery receiving pools
   * \param pools the receiving pools
   */
  virtual void SetSlRxPools (std::list<Ptr<SidelinkRxDiscResourcePool> > pools) = 0;

  /**
   * set discovery annoucement apps
   * \param apps applications we are interested in announcing
   */
  virtual void AddDiscTxApps (std::list<uint32_t> apps) = 0;

  /**
   * set discovery monitoring apps
   * \param apps applications we are interested in monitoring
   */
  virtual void AddDiscRxApps (std::list<uint32_t> apps) = 0;


  /**
   * Set grant for discovery
   */
  virtual void SetDiscGrantInfo (uint8_t resPsdch) = 0;

//communication
  /**
   * set the current sidelink transmit pool
   * \param pool the transmission pool
   */
  virtual void SetSlTxPool (Ptr<SidelinkTxCommResourcePool> pool) = 0;

  /**
   * set the current sidelink transmit pool
   * \param pool the transmission pool
   */
  virtual void SetSlV2xTxPool (Ptr<SidelinkTxCommResourcePoolV2x> pool) = 0;

  /**
   * remove the sidelink transmission pool 
   * \param dstL2Id the destination
   */
  virtual void RemoveSlTxPool () = 0;

  /**
   * remove the sidelink transmission pool 
   * \param dstL2Id the destination
   */
  virtual void RemoveSlV2xTxPool () = 0;

  /**
   * set the sidelink receiving pools
   * \param destinations The list of destinations (group) to monitor
   * \param pools the sidelink receiving pools
   */
  virtual void SetSlRxPools (std::list<Ptr<SidelinkRxCommResourcePool> > pools) = 0;

  /**
   * set the sidelink receiving pools
   * \param destinations The list of destinations (group) to monitor
   * \param pools the sidelink receiving pools
   */
  virtual void SetSlV2xRxPools (std::list<Ptr<SidelinkRxCommResourcePoolV2x> > pools) = 0;

  /**
   * add a new destination to listen for
   * \param a destination to listen for
   */
  virtual void AddSlDestination (uint32_t destination) = 0;

  /**
   * remove a destination to listen for
   * \param destination the destination that is no longer of interest
   */
  virtual void RemoveSlDestination (uint32_t destination) = 0;
  
  /**
   * Pass to the PHY entity the SLSSID to be set
   * \param slssid the SLSSID
   */
  virtual void SetSlssId (uint64_t slssid) = 0;

  /**
    * Pass to the PHY entity a SLSS to be sent
    * \param mibSl the MIB-SL to send
    */
  virtual void SendSlss (cv2x_LteRrcSap::MasterInformationBlockSL mibSl) = 0;
  
  /**
   * Notify the PHY entity that a SyncRef has been selected and that it should apply
   * the corresponding change of timing when appropriate
   * \param mibSl the MIB-SL containing the information of the selected SyncRef
   */
  virtual void SynchronizeToSyncRef (cv2x_LteRrcSap::MasterInformationBlockSL mibSl) = 0;

};


/**
 * Service Access Point (SAP) offered by the UE PHY to the UE RRC for control purposes
 *
 * This is the CPHY SAP User, i.e., the part of the SAP that contains the RRC
 * methods called by the PHY
*/
class cv2x_LteUeCphySapUser
{
public:

  /** 
   * destructor
   */
  virtual ~cv2x_LteUeCphySapUser ();


  /**
   * Parameters of the ReportUeMeasurements primitive: RSRP [dBm] and RSRQ [dB]
   * See section 5.1.1 and 5.1.3 of TS 36.214
   */
  struct UeMeasurementsElement
  {
    uint16_t m_cellId; ///< cell ID
    double m_rsrp;  ///< [dBm]
    double m_rsrq;  ///< [dB]
  };

  /// UeMeasurementsParameters structure
  struct UeMeasurementsParameters
  {
    std::vector <struct UeMeasurementsElement> m_ueMeasurementsList; ///< UE measurement list
    uint8_t m_componentCarrierId; ///< component carrier ID
  };

  /**
   * Parameters for reporting S-RSRP measurements to the RRC by the PHY
   */
  struct UeSlssMeasurementReportElement
  {
    uint16_t m_slssid; ///< SLSSID of the measured SyncRef
    double m_srsrp;  ///< Measured S-RSRP [dBm]
    uint16_t m_offset; ///< Reception offset
  };
  /**
   * List of SLSS measurements to be reported to the RRC by the PHY
   */
  struct UeSlssMeasurementsParameters
  {
    std::vector <struct UeSlssMeasurementReportElement> m_ueSlssMeasurementsList; ///< List of SLSS measurements to be reported to the RRC by the PHY
  };

  /**
   * \brief Relay an MIB message from the PHY entity to the RRC layer.
   * \param cellId the ID of the eNodeB where the message originates from
   * \param mib the Master Information Block message
   * 
   * This function is typically called after PHY receives an MIB message over
   * the BCH.
   */
  virtual void RecvMasterInformationBlock (uint16_t cellId,
                                           cv2x_LteRrcSap::MasterInformationBlock mib) = 0;

  /**
   * \brief Relay an SIB1 message from the PHY entity to the RRC layer.
   * \param cellId the ID of the eNodeB where the message originates from
   * \param sib1 the System Information Block Type 1 message
   *
   * This function is typically called after PHY receives an SIB1 message over
   * the BCH.
   */
  virtual void RecvSystemInformationBlockType1 (uint16_t cellId,
                                                cv2x_LteRrcSap::SystemInformationBlockType1 sib1) = 0;

  /**
   * \brief Send a report of RSRP and RSRQ values perceived from PSS by the PHY
   *        entity (after applying layer-1 filtering) to the RRC layer.
   * \param params the structure containing a vector of cellId, RSRP and RSRQ
   */
  virtual void ReportUeMeasurements (UeMeasurementsParameters params) = 0;

  /**
    * \brief Send a report of S-RSRP values perceived from SLSSs by the PHY
    *        entity (after applying layer-1 filtering) to the RRC layer.
    * \param params the structure containing a list of
    *        (SyncRef SLSSID, SyncRef offset and S-RSRP value)
    * \param slssid the SLSSID of the evaluated SyncRef if corresponding
    * \param offset the offset of the evaluated SyncRef if corresponding
    */
  virtual void ReportSlssMeasurements (cv2x_LteUeCphySapUser::UeSlssMeasurementsParameters params,  uint64_t slssid, uint16_t offset) = 0;
  
  /**
   * The PHY indicated to the RRC the current subframe indication
   * \param frameNo the current frameNo
   * \param subFrameNo the current subframeNo
   */
  virtual void ReportSubframeIndication (uint16_t frameNo, uint16_t subFrameNo) = 0;
  
  /**
   * The PHY pass a received MIB-SL to the RRC
   * \param mibSl the received MIB-SL
   */
  virtual void ReceiveMibSL (cv2x_LteRrcSap::MasterInformationBlockSL mibSl) = 0;
  
  /**
   * Notify the successful change of timing/SyncRef, and store the selected
   * (current) SyncRef information
   * \param mibSl the SyncRef MIB-SL containing its information
   * \param frameNo the current frameNo
   * \param subFrameNo the current subframeNo
   */
  virtual void ReportChangeOfSyncRef (cv2x_LteRrcSap::MasterInformationBlockSL mibSl, uint16_t frameNo, uint16_t subFrameNo) = 0;

};




/**
 * Template for the implementation of the cv2x_LteUeCphySapProvider as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class cv2x_MemberLteUeCphySapProvider : public cv2x_LteUeCphySapProvider
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteUeCphySapProvider (C* owner);

  // inherited from cv2x_LteUeCphySapProvider
  virtual void Reset ();
  virtual void StartCellSearch (uint32_t dlEarfcn);
  virtual void SynchronizeWithEnb (uint16_t cellId);
  virtual void SynchronizeWithEnb (uint16_t cellId, uint32_t dlEarfcn);
  virtual void SetDlBandwidth (uint8_t dlBandwidth);
  virtual void ConfigureUplink (uint32_t ulEarfcn, uint8_t ulBandwidth);
  virtual void ConfigureReferenceSignalPower (int8_t referenceSignalPower);
  virtual void SetRnti (uint16_t rnti);
  virtual void SetTransmissionMode (uint8_t txMode);
  virtual void SetSrsConfigurationIndex (uint16_t srcCi);
  virtual void SetPa (double pa);
  //discovery
  virtual void SetSlTxPool (Ptr<SidelinkTxDiscResourcePool> pool);
  virtual void RemoveSlTxPool (bool disc);
  virtual void SetSlRxPools (std::list<Ptr<SidelinkRxDiscResourcePool> > pools);
  virtual void SetDiscGrantInfo (uint8_t resPsdch);
  virtual void AddDiscTxApps (std::list<uint32_t> apps);
  virtual void AddDiscRxApps (std::list<uint32_t> apps);
  //communication
  virtual void SetSlTxPool (Ptr<SidelinkTxCommResourcePool> pool);
  virtual void RemoveSlTxPool ();
  virtual void SetSlRxPools (std::list<Ptr<SidelinkRxCommResourcePool> > pools);
  virtual void AddSlDestination (uint32_t destination);
  virtual void RemoveSlDestination (uint32_t destination);
  virtual void SetSlssId (uint64_t slssid);
  virtual void SendSlss (cv2x_LteRrcSap::MasterInformationBlockSL mibSl);
  virtual void SynchronizeToSyncRef (cv2x_LteRrcSap::MasterInformationBlockSL mibSl);
  //V2X
  virtual void SetSlV2xTxPool (Ptr<SidelinkTxCommResourcePoolV2x> pool);
  virtual void RemoveSlV2xTxPool ();
  virtual void SetSlV2xRxPools (std::list<Ptr<SidelinkRxCommResourcePoolV2x> > pools);


private:
  cv2x_MemberLteUeCphySapProvider ();
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteUeCphySapProvider<C>::cv2x_MemberLteUeCphySapProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
cv2x_MemberLteUeCphySapProvider<C>::cv2x_MemberLteUeCphySapProvider ()
{
}

template <class C>
void 
cv2x_MemberLteUeCphySapProvider<C>::Reset ()
{
  m_owner->DoReset ();
}

template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::StartCellSearch (uint32_t dlEarfcn)
{
  m_owner->DoStartCellSearch (dlEarfcn);
}

template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::SynchronizeWithEnb (uint16_t cellId)
{
  m_owner->DoSynchronizeWithEnb (cellId);
}

template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::SynchronizeWithEnb (uint16_t cellId, uint32_t dlEarfcn)
{
  m_owner->DoSynchronizeWithEnb (cellId, dlEarfcn);
}

template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::SetDlBandwidth (uint8_t dlBandwidth)
{
  m_owner->DoSetDlBandwidth (dlBandwidth);
}

template <class C>
void 
cv2x_MemberLteUeCphySapProvider<C>::ConfigureUplink (uint32_t ulEarfcn, uint8_t ulBandwidth)
{
  m_owner->DoConfigureUplink (ulEarfcn, ulBandwidth);
}

template <class C>
void 
cv2x_MemberLteUeCphySapProvider<C>::ConfigureReferenceSignalPower (int8_t referenceSignalPower)
{
  m_owner->DoConfigureReferenceSignalPower (referenceSignalPower);
}

template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::SetRnti (uint16_t rnti)
{
  m_owner->DoSetRnti (rnti);
}

template <class C>
void 
cv2x_MemberLteUeCphySapProvider<C>::SetTransmissionMode (uint8_t txMode)
{
  m_owner->DoSetTransmissionMode (txMode);
}

template <class C>
void 
cv2x_MemberLteUeCphySapProvider<C>::SetSrsConfigurationIndex (uint16_t srcCi)
{
  m_owner->DoSetSrsConfigurationIndex (srcCi);
}

template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::SetPa (double pa)
{
  m_owner->DoSetPa (pa);
}

//discovery
template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::SetSlTxPool (Ptr<SidelinkTxDiscResourcePool> pool)
{
  m_owner->DoSetSlTxPool (pool);
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::RemoveSlTxPool (bool disc)
{
  m_owner->DoRemoveSlTxPool (disc);
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::SetSlRxPools (std::list<Ptr<SidelinkRxDiscResourcePool> > pools)
{
  m_owner->DoSetSlRxPools (pools);
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::SetDiscGrantInfo (uint8_t resPsdch) 
{
  m_owner->DoSetDiscGrantInfo (resPsdch);
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::AddDiscTxApps (std::list<uint32_t> apps)
{
  m_owner->DoAddDiscTxApps (apps);
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::AddDiscRxApps (std::list<uint32_t> apps)
{
  m_owner->DoAddDiscRxApps (apps);
}


//communication
template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::SetSlTxPool (Ptr<SidelinkTxCommResourcePool> pool)
{
  m_owner->DoSetSlTxPool (pool);
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::RemoveSlTxPool ()
{
  m_owner->DoRemoveSlTxPool ();
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::SetSlRxPools (std::list<Ptr<SidelinkRxCommResourcePool> > pools)
{
  m_owner->DoSetSlRxPools (pools);
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::AddSlDestination (uint32_t destination)
{
  m_owner->DoAddSlDestination (destination);
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::RemoveSlDestination (uint32_t destination)
{
  m_owner->DoRemoveSlDestination (destination);
}

template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::SetSlssId (uint64_t slssid)
{
  m_owner->DoSetSlssId (slssid);
}

template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::SendSlss (cv2x_LteRrcSap::MasterInformationBlockSL mibSl)
{
  m_owner->DoSendSlss (mibSl);
}

template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::SynchronizeToSyncRef (cv2x_LteRrcSap::MasterInformationBlockSL mibSl)
{
  m_owner->DoSynchronizeToSyncRef (mibSl);
}

// V2X communication
template <class C>
void
cv2x_MemberLteUeCphySapProvider<C>::SetSlV2xTxPool (Ptr<SidelinkTxCommResourcePoolV2x> pool)
{
  m_owner->DoSetSlV2xTxPool (pool);
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::RemoveSlV2xTxPool ()
{
  m_owner->DoRemoveSlV2xTxPool ();
}

template <class C>
void cv2x_MemberLteUeCphySapProvider<C>::SetSlV2xRxPools (std::list<Ptr<SidelinkRxCommResourcePoolV2x> > pools)
{
  m_owner->DoSetSlV2xRxPools (pools);
}

/**
 * Template for the implementation of the cv2x_LteUeCphySapUser as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class cv2x_MemberLteUeCphySapUser : public cv2x_LteUeCphySapUser
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteUeCphySapUser (C* owner);

  // methods inherited from cv2x_LteUeCphySapUser go here
  virtual void RecvMasterInformationBlock (uint16_t cellId,
                                           cv2x_LteRrcSap::MasterInformationBlock mib);
  virtual void RecvSystemInformationBlockType1 (uint16_t cellId,
                                                cv2x_LteRrcSap::SystemInformationBlockType1 sib1);
  virtual void ReportUeMeasurements (cv2x_LteUeCphySapUser::UeMeasurementsParameters params);

  virtual void ReportSlssMeasurements (cv2x_LteUeCphySapUser::UeSlssMeasurementsParameters params,  uint64_t slssid, uint16_t offset);
  virtual void ReportSubframeIndication (uint16_t frameNo, uint16_t subFrameNo);
  virtual void ReceiveMibSL (cv2x_LteRrcSap::MasterInformationBlockSL mibSL);
  virtual void ReportChangeOfSyncRef (cv2x_LteRrcSap::MasterInformationBlockSL mibSL, uint16_t frameNo, uint16_t subFrameNo);


private:
  cv2x_MemberLteUeCphySapUser ();
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteUeCphySapUser<C>::cv2x_MemberLteUeCphySapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
cv2x_MemberLteUeCphySapUser<C>::cv2x_MemberLteUeCphySapUser ()
{
}

template <class C> 
void 
cv2x_MemberLteUeCphySapUser<C>::RecvMasterInformationBlock (uint16_t cellId,
                                                       cv2x_LteRrcSap::MasterInformationBlock mib)
{
  m_owner->DoRecvMasterInformationBlock (cellId, mib);
}

template <class C>
void
cv2x_MemberLteUeCphySapUser<C>::RecvSystemInformationBlockType1 (uint16_t cellId,
                                                            cv2x_LteRrcSap::SystemInformationBlockType1 sib1)
{
  m_owner->DoRecvSystemInformationBlockType1 (cellId, sib1);
}

template <class C>
void
cv2x_MemberLteUeCphySapUser<C>::ReportUeMeasurements (cv2x_LteUeCphySapUser::UeMeasurementsParameters params)
{
  m_owner->DoReportUeMeasurements (params);
}

template <class C>
void
cv2x_MemberLteUeCphySapUser<C>::ReportSlssMeasurements (cv2x_LteUeCphySapUser::UeSlssMeasurementsParameters params,  uint64_t slssid, uint16_t offset)
{
  m_owner->DoReportSlssMeasurements (params,  slssid, offset);
}


template <class C>
void
cv2x_MemberLteUeCphySapUser<C>::ReportSubframeIndication (uint16_t frameNo, uint16_t subFrameNo)
{
  m_owner->DoReportSubframeIndication (frameNo, subFrameNo);
}

template <class C>
void
cv2x_MemberLteUeCphySapUser<C>::ReceiveMibSL (cv2x_LteRrcSap::MasterInformationBlockSL mibSL)
{
  m_owner->DoReceiveMibSL (mibSL);
}

template <class C>
void
cv2x_MemberLteUeCphySapUser<C>::ReportChangeOfSyncRef (cv2x_LteRrcSap::MasterInformationBlockSL mibSL, uint16_t frameNo, uint16_t subFrameNo)
{
  m_owner->DoReportChangeOfSyncRef (mibSL, frameNo, subFrameNo );
}

} // namespace ns3


#endif // CV2X_LTE_UE_CPHY_SAP_H

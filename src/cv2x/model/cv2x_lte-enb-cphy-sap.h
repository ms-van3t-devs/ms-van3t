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
 */

#ifndef CV2X_LTE_ENB_CPHY_SAP_H
#define CV2X_LTE_ENB_CPHY_SAP_H

#include <stdint.h>
#include <ns3/ptr.h>

#include <ns3/cv2x_lte-rrc-sap.h>

namespace ns3 {

class cv2x_LteEnbNetDevice;

/**
 * Service Access Point (SAP) offered by the UE PHY to the UE RRC for control purposes
 *
 * This is the PHY SAP Provider, i.e., the part of the SAP that contains
 * the PHY methods called by the MAC
 */
class cv2x_LteEnbCphySapProvider
{
public:

  /** 
   * destructor
   */
  virtual ~cv2x_LteEnbCphySapProvider ();

  /** 
   * 
   * 
   * \param cellId the Cell Identifier
   */
  virtual void SetCellId (uint16_t cellId) = 0;

  /**
   * \param ulBandwidth the UL bandwidth in PRBs
   * \param dlBandwidth the DL bandwidth in PRBs
   */
  virtual void SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth) = 0;

  /**
   * \param ulEarfcn the UL EARFCN
   * \param dlEarfcn the DL EARFCN
   */
  virtual void SetEarfcn (uint32_t ulEarfcn, uint32_t dlEarfcn) = 0;
  
  /** 
   * Add a new UE to the cell
   * 
   * \param rnti the UE id relative to this cell
   */
  virtual void AddUe (uint16_t rnti) = 0;

  /** 
   * Remove an UE from the cell
   * 
   * \param rnti the UE id relative to this cell
   */
  virtual void RemoveUe (uint16_t rnti) = 0;
  
  /**
   * Set the UE transmission power offset P_A
   *
   * \param rnti the UE id relative to this cell
   * \param pa transmission power offset
   */
  virtual void SetPa (uint16_t rnti, double pa) = 0;

  /**
   * \param rnti the RNTI of the user
   * \param txMode the transmissionMode of the user
   */
  virtual void SetTransmissionMode (uint16_t rnti, uint8_t txMode) = 0;

  /**
   * \param rnti the RNTI of the user
   * \param srsCi the SRS Configuration Index of the user
   */
  virtual void SetSrsConfigurationIndex (uint16_t rnti, uint16_t srsCi) = 0;

  /** 
   * 
   * \param mib the Master Information Block to be sent on the BCH
   */
  virtual void SetMasterInformationBlock (cv2x_LteRrcSap::MasterInformationBlock mib) = 0;

  /**
   *
   * \param sib1 the System Information Block Type 1 to be sent on the BCH
   */
  virtual void SetSystemInformationBlockType1 (cv2x_LteRrcSap::SystemInformationBlockType1 sib1) = 0;

  /**
   *
   * \return Reference Signal Power for SIB2
   */
  virtual int8_t GetReferenceSignalPower () = 0;
};


/**
 * Service Access Point (SAP) offered by the UE PHY to the UE RRC for control purposes
 *
 * This is the CPHY SAP User, i.e., the part of the SAP that contains the RRC
 * methods called by the PHY
*/
class cv2x_LteEnbCphySapUser
{
public:
  
  /** 
   * destructor
   */
  virtual ~cv2x_LteEnbCphySapUser ();

};


/**
 * Template for the implementation of the cv2x_LteEnbCphySapProvider as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class cv2x_MemberLteEnbCphySapProvider : public cv2x_LteEnbCphySapProvider
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteEnbCphySapProvider (C* owner);

  // inherited from cv2x_LteEnbCphySapProvider
  virtual void SetCellId (uint16_t cellId);
  virtual void SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth);
  virtual void SetEarfcn (uint32_t ulEarfcn, uint32_t dlEarfcn);
  virtual void AddUe (uint16_t rnti);
  virtual void RemoveUe (uint16_t rnti);
  virtual void SetPa (uint16_t rnti, double pa);
  virtual void SetTransmissionMode (uint16_t  rnti, uint8_t txMode);
  virtual void SetSrsConfigurationIndex (uint16_t  rnti, uint16_t srsCi);
  virtual void SetMasterInformationBlock (cv2x_LteRrcSap::MasterInformationBlock mib);
  virtual void SetSystemInformationBlockType1 (cv2x_LteRrcSap::SystemInformationBlockType1 sib1);
  virtual int8_t GetReferenceSignalPower ();
  
private:
  cv2x_MemberLteEnbCphySapProvider ();
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteEnbCphySapProvider<C>::cv2x_MemberLteEnbCphySapProvider (C* owner)
  : m_owner (owner)
{
}

template <class C>
cv2x_MemberLteEnbCphySapProvider<C>::cv2x_MemberLteEnbCphySapProvider ()
{
}

template <class C>
void 
cv2x_MemberLteEnbCphySapProvider<C>::SetCellId (uint16_t cellId)
{
  m_owner->DoSetCellId (cellId);
}


template <class C>
void 
cv2x_MemberLteEnbCphySapProvider<C>::SetBandwidth (uint8_t ulBandwidth, uint8_t dlBandwidth)
{
  m_owner->DoSetBandwidth (ulBandwidth, dlBandwidth);
}

template <class C>
void 
cv2x_MemberLteEnbCphySapProvider<C>::SetEarfcn (uint32_t ulEarfcn, uint32_t dlEarfcn)
{
  m_owner->DoSetEarfcn (ulEarfcn, dlEarfcn);
}

template <class C>
void 
cv2x_MemberLteEnbCphySapProvider<C>::AddUe (uint16_t rnti)
{
  m_owner->DoAddUe (rnti);
}

template <class C>
void 
cv2x_MemberLteEnbCphySapProvider<C>::RemoveUe (uint16_t rnti)
{
  m_owner->DoRemoveUe (rnti);
}

template <class C>
void 
cv2x_MemberLteEnbCphySapProvider<C>::SetPa (uint16_t rnti, double pa)
{
  m_owner->DoSetPa (rnti, pa);
}

template <class C>
void
cv2x_MemberLteEnbCphySapProvider<C>::SetTransmissionMode (uint16_t  rnti, uint8_t txMode)
{
  m_owner->DoSetTransmissionMode (rnti, txMode);
}

template <class C>
void 
cv2x_MemberLteEnbCphySapProvider<C>::SetSrsConfigurationIndex (uint16_t  rnti, uint16_t srsCi)
{
  m_owner->DoSetSrsConfigurationIndex (rnti, srsCi);
}

template <class C> 
void 
cv2x_MemberLteEnbCphySapProvider<C>::SetMasterInformationBlock (cv2x_LteRrcSap::MasterInformationBlock mib)
{
  m_owner->DoSetMasterInformationBlock (mib);
}

template <class C>
void
cv2x_MemberLteEnbCphySapProvider<C>::SetSystemInformationBlockType1 (cv2x_LteRrcSap::SystemInformationBlockType1 sib1)
{
  m_owner->DoSetSystemInformationBlockType1 (sib1);
}

template <class C>
int8_t
cv2x_MemberLteEnbCphySapProvider<C>::GetReferenceSignalPower ()
{
  return m_owner->DoGetReferenceSignalPower ();
}

/**
 * Template for the implementation of the cv2x_LteEnbCphySapUser as a member
 * of an owner class of type C to which all methods are forwarded
 * 
 */
template <class C>
class cv2x_MemberLteEnbCphySapUser : public cv2x_LteEnbCphySapUser
{
public:
  /**
   * Constructor
   *
   * \param owner the owner class
   */
  cv2x_MemberLteEnbCphySapUser (C* owner);

  // methods inherited from cv2x_LteEnbCphySapUser go here

private:
  cv2x_MemberLteEnbCphySapUser ();
  C* m_owner; ///< the owner class
};

template <class C>
cv2x_MemberLteEnbCphySapUser<C>::cv2x_MemberLteEnbCphySapUser (C* owner)
  : m_owner (owner)
{
}

template <class C>
cv2x_MemberLteEnbCphySapUser<C>::cv2x_MemberLteEnbCphySapUser ()
{
}






} // namespace ns3


#endif // CV2X_LTE_ENB_CPHY_SAP_H

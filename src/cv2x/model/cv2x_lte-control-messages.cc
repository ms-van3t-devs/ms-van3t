/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 TELEMATICS LAB, DEE - Politecnico di Bari
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
 * Author: Giuseppe Piro  <g.piro@poliba.it>
 *         Marco Miozzo <marco.miozzo@cttc.es>
 * Modified by: NIST
 *              Fabian Eckermann <fabian.eckermann@udo.edu> (CNI)
 *              Moritz Kahlert <moritz.kahlert@udo.edu> (CNI)
 */

#include "cv2x_lte-control-messages.h"
#include "ns3/address-utils.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
#include "cv2x_lte-net-device.h"
#include "cv2x_lte-ue-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("cv2x_LteControlMessage");

cv2x_LteControlMessage::cv2x_LteControlMessage (void)
{
}


cv2x_LteControlMessage::~cv2x_LteControlMessage (void)
{
}


void
cv2x_LteControlMessage::SetMessageType (cv2x_LteControlMessage::MessageType type)
{
  m_type = type;
}


cv2x_LteControlMessage::MessageType
cv2x_LteControlMessage::GetMessageType (void)
{
  return m_type;
}


// ----------------------------------------------------------------------------------------------------------


cv2x_DlDciLteControlMessage::cv2x_DlDciLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::DL_DCI);
}


cv2x_DlDciLteControlMessage::~cv2x_DlDciLteControlMessage (void)
{

}

void
cv2x_DlDciLteControlMessage::SetDci (cv2x_DlDciListElement_s dci)
{
  m_dci = dci;
}


cv2x_DlDciListElement_s
cv2x_DlDciLteControlMessage::GetDci (void)
{
  return m_dci;
}


// ----------------------------------------------------------------------------------------------------------

cv2x_SlDciLteControlMessageV2x::cv2x_SlDciLteControlMessageV2x (void)
{
  SetMessageType (cv2x_LteControlMessage::SL_DCI_V2X);
}

cv2x_SlDciLteControlMessageV2x::~cv2x_SlDciLteControlMessageV2x (void)
{

}

void 
cv2x_SlDciLteControlMessageV2x::SetDci (cv2x_SlDciListElementV2x dci)
{
  m_dci = dci; 
}

cv2x_SlDciListElementV2x
cv2x_SlDciLteControlMessageV2x::GetDci (void)
{
  return m_dci;
}

// ----------------------------------------------------------------------------------------------------------



cv2x_UlDciLteControlMessage::cv2x_UlDciLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::UL_DCI);
}


cv2x_UlDciLteControlMessage::~cv2x_UlDciLteControlMessage (void)
{

}

void
cv2x_UlDciLteControlMessage::SetDci (cv2x_UlDciListElement_s dci)
{
  m_dci = dci;

}


cv2x_UlDciListElement_s
cv2x_UlDciLteControlMessage::GetDci (void)
{
  return m_dci;
}


// ----------------------------------------------------------------------------------------------------------


cv2x_SlDciLteControlMessage::cv2x_SlDciLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::SL_DCI);
}


cv2x_SlDciLteControlMessage::~cv2x_SlDciLteControlMessage (void)
{

}

void
cv2x_SlDciLteControlMessage::SetDci (cv2x_SlDciListElement_s dci)
{
  m_dci = dci;

}


cv2x_SlDciListElement_s
cv2x_SlDciLteControlMessage::GetDci (void)
{
  return m_dci;
}


// ----------------------------------------------------------------------------------------------------------


cv2x_DlCqiLteControlMessage::cv2x_DlCqiLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::DL_CQI);
}


cv2x_DlCqiLteControlMessage::~cv2x_DlCqiLteControlMessage (void)
{

}

void
cv2x_DlCqiLteControlMessage::SetDlCqi (cv2x_CqiListElement_s dlcqi)
{
  m_dlCqi = dlcqi;

}


cv2x_CqiListElement_s
cv2x_DlCqiLteControlMessage::GetDlCqi (void)
{
  return m_dlCqi;
}



// ----------------------------------------------------------------------------------------------------------


cv2x_BsrLteControlMessage::cv2x_BsrLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::BSR);
}


cv2x_BsrLteControlMessage::~cv2x_BsrLteControlMessage (void)
{

}

void
cv2x_BsrLteControlMessage::SetBsr (cv2x_MacCeListElement_s bsr)
{
  m_bsr = bsr;

}


cv2x_MacCeListElement_s
cv2x_BsrLteControlMessage::GetBsr (void)
{
  return m_bsr;
}



// ----------------------------------------------------------------------------------------------------------


cv2x_RachPreambleLteControlMessage::cv2x_RachPreambleLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::RACH_PREAMBLE);
}

void
cv2x_RachPreambleLteControlMessage::SetRapId (uint32_t rapId)
{
  m_rapId = rapId;
}

uint32_t 
cv2x_RachPreambleLteControlMessage::GetRapId () const
{
  return m_rapId;
}


// ----------------------------------------------------------------------------------------------------------


cv2x_RarLteControlMessage::cv2x_RarLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::RAR);
}


void
cv2x_RarLteControlMessage::SetRaRnti (uint16_t raRnti)
{
  m_raRnti = raRnti;
}

uint16_t 
cv2x_RarLteControlMessage::GetRaRnti () const
{
  return m_raRnti;
}


void
cv2x_RarLteControlMessage::AddRar (Rar rar)
{
  m_rarList.push_back (rar);
}

std::list<cv2x_RarLteControlMessage::Rar>::const_iterator 
cv2x_RarLteControlMessage::RarListBegin () const
{
  return m_rarList.begin ();
}

std::list<cv2x_RarLteControlMessage::Rar>::const_iterator 
cv2x_RarLteControlMessage::RarListEnd () const
{
  return m_rarList.end ();
}


// ----------------------------------------------------------------------------------------------------------



cv2x_MibLteControlMessage::cv2x_MibLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::MIB);
}


void
cv2x_MibLteControlMessage::SetMib (cv2x_LteRrcSap::MasterInformationBlock  mib)
{
  m_mib = mib;
}

cv2x_LteRrcSap::MasterInformationBlock 
cv2x_MibLteControlMessage::GetMib () const
{
  return m_mib;
}


// ----------------------------------------------------------------------------------------------------------



Sib1cv2x_LteControlMessage::Sib1cv2x_LteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::SIB1);
}


void
Sib1cv2x_LteControlMessage::SetSib1 (cv2x_LteRrcSap::SystemInformationBlockType1 sib1)
{
  m_sib1 = sib1;
}

cv2x_LteRrcSap::SystemInformationBlockType1
Sib1cv2x_LteControlMessage::GetSib1 () const
{
  return m_sib1;
}


// ----------------------------------------------------------------------------------------------------------


cv2x_SciLteControlMessage::cv2x_SciLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::SCI);
}


void
cv2x_SciLteControlMessage::SetSci (cv2x_SciListElement_s sci)
{
  m_sci = sci;
}

cv2x_SciListElement_s
cv2x_SciLteControlMessage::GetSci ()
{
  return m_sci;
}


// ---------------------------------------------------------------------------


cv2x_SciLteControlMessageV2x::cv2x_SciLteControlMessageV2x (void)
{
  SetMessageType (cv2x_LteControlMessage::SCI_V2X);
}

void
cv2x_SciLteControlMessageV2x::SetSci (cv2x_SciListElementV2x sci)
{
  m_sci = sci;
}

cv2x_SciListElementV2x
cv2x_SciLteControlMessageV2x::GetSci()
{
  return m_sci; 
}


// ---------------------------------------------------------------------------



cv2x_DlHarqFeedbackLteControlMessage::cv2x_DlHarqFeedbackLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::DL_HARQ);
}


cv2x_DlHarqFeedbackLteControlMessage::~cv2x_DlHarqFeedbackLteControlMessage (void)
{

}

void
cv2x_DlHarqFeedbackLteControlMessage::SetDlHarqFeedback (cv2x_DlInfoListElement_s m)
{
  m_dlInfoListElement = m;
}


cv2x_DlInfoListElement_s
cv2x_DlHarqFeedbackLteControlMessage::GetDlHarqFeedback (void)
{
  return m_dlInfoListElement;
}


// ----------------------------------------------------------------------------------------------------------


cv2x_MibSLLteControlMessage::cv2x_MibSLLteControlMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::MIB_SL);
}

void
cv2x_MibSLLteControlMessage::SetMibSL (cv2x_LteRrcSap::MasterInformationBlockSL mibSL)
{
  m_mibSL = mibSL;
}

cv2x_LteRrcSap::MasterInformationBlockSL
cv2x_MibSLLteControlMessage::GetMibSL ()
{
  return m_mibSL;
}


// ---------------------------------------------------------------------------


cv2x_SlDiscMessage::cv2x_SlDiscMessage (void)
{
  SetMessageType (cv2x_LteControlMessage::SL_DISC_MSG);
}


void
cv2x_SlDiscMessage::SetSlDiscMessage (cv2x_SlDiscMsg discMsg)
{
  m_discMsg = discMsg;
}

cv2x_SlDiscMsg
cv2x_SlDiscMessage::GetSlDiscMessage ()
{
  return m_discMsg;
}


} // namespace ns3


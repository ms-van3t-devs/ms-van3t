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

#include <ns3/log.h>
#include "nr-control-messages.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("nrControlMessage");

NrControlMessage::NrControlMessage (void)
{
  NS_LOG_INFO (this);
}

NrControlMessage::~NrControlMessage (void)
{
  NS_LOG_INFO (this);
}

void
NrControlMessage::SetMessageType (messageType type)
{
  m_messageType = type;
}

NrControlMessage::messageType
NrControlMessage::GetMessageType (void) const
{
  return m_messageType;
}

void
NrControlMessage::SetSourceBwp (uint16_t bwpId)
{
  m_bwpId = bwpId;
}

uint16_t
NrControlMessage::GetSourceBwp () const
{
  NS_ABORT_IF (m_bwpId < 0);
  return static_cast<uint16_t> (m_bwpId);
}

NrSRMessage::NrSRMessage ()
{
  NS_LOG_INFO (this);
  SetMessageType (NrControlMessage::SR);
}

NrSRMessage::~NrSRMessage ()
{
  NS_LOG_INFO (this);
}

void
NrSRMessage::SetRNTI (uint16_t rnti)
{
  m_rnti = rnti;
}

uint16_t
NrSRMessage::GetRNTI () const
{
  return m_rnti;
}

NrDlDciMessage::NrDlDciMessage (const std::shared_ptr<DciInfoElementTdma> &dci)
  : m_dciInfoElement (dci)
{
  NS_LOG_INFO (this);
  SetMessageType (NrControlMessage::DL_DCI);
}

NrDlDciMessage::~NrDlDciMessage (void)
{
  NS_LOG_INFO (this);
}

std::shared_ptr<DciInfoElementTdma>
NrDlDciMessage::GetDciInfoElement (void)
{
  return m_dciInfoElement;
}

void
NrDlDciMessage::SetKDelay (uint32_t delay)
{
  m_k = delay;
}

void
NrDlDciMessage::SetK1Delay (uint32_t delay)
{
  m_k1 = delay;
}

uint32_t
NrDlDciMessage::GetKDelay (void) const
{
  return m_k;
}

uint32_t
NrDlDciMessage::GetK1Delay (void) const
{
  return m_k1;
}

NrUlDciMessage::NrUlDciMessage (const std::shared_ptr<DciInfoElementTdma> &dci)
  : m_dciInfoElement (dci)
{
  NS_LOG_INFO (this);
  SetMessageType (NrControlMessage::UL_DCI);
}

NrUlDciMessage::~NrUlDciMessage (void)
{
  NS_LOG_INFO (this);
}

std::shared_ptr<DciInfoElementTdma>
NrUlDciMessage::GetDciInfoElement (void)
{
  return m_dciInfoElement;
}

void
NrUlDciMessage::SetKDelay (uint32_t delay)
{
  m_k = delay;
}

uint32_t
NrUlDciMessage::GetKDelay (void) const
{
  return m_k;
}

NrDlCqiMessage::NrDlCqiMessage (void)
{
  SetMessageType (NrControlMessage::DL_CQI);
  NS_LOG_INFO (this);
}
NrDlCqiMessage::~NrDlCqiMessage (void)
{
  NS_LOG_INFO (this);
}

void
NrDlCqiMessage::SetDlCqi (DlCqiInfo cqi)
{
  m_cqi = cqi;
}

DlCqiInfo
NrDlCqiMessage::GetDlCqi ()
{
  return m_cqi;
}

// ----------------------------------------------------------------------------------------------------------

NrBsrMessage::NrBsrMessage (void)
{
  SetMessageType (NrControlMessage::BSR);
}


NrBsrMessage::~NrBsrMessage (void)
{

}

void
NrBsrMessage::SetBsr (MacCeElement bsr)
{
  m_bsr = bsr;

}


MacCeElement
NrBsrMessage::GetBsr (void)
{
  return m_bsr;
}

// ----------------------------------------------------------------------------------------------------------



NrMibMessage::NrMibMessage (void)
{
  SetMessageType (NrControlMessage::MIB);
}


void
NrMibMessage::SetMib (LteRrcSap::MasterInformationBlock  mib)
{
  m_mib = mib;
}

LteRrcSap::MasterInformationBlock
NrMibMessage::GetMib () const
{
  return m_mib;
}


// ----------------------------------------------------------------------------------------------------------



NrSib1Message::NrSib1Message (void)
{
  SetMessageType (NrControlMessage::SIB1);
}


void
NrSib1Message::SetSib1 (LteRrcSap::SystemInformationBlockType1 sib1)
{
  m_sib1 = sib1;
}

LteRrcSap::SystemInformationBlockType1
NrSib1Message::GetSib1 () const
{
  return m_sib1;
}

// ----------------------------------------------------------------------------------------------------------

NrRachPreambleMessage::NrRachPreambleMessage (void)
{
  SetMessageType (NrControlMessage::RACH_PREAMBLE);
}

NrRachPreambleMessage::~NrRachPreambleMessage()
{

}

void
NrRachPreambleMessage::SetRapId (uint32_t rapId)
{
  m_rapId = rapId;
}

uint32_t
NrRachPreambleMessage::GetRapId () const
{
  return m_rapId;
}

// ----------------------------------------------------------------------------------------------------------


NrRarMessage::NrRarMessage (void)
{
  SetMessageType (NrControlMessage::RAR);
}

NrRarMessage::~NrRarMessage()
{

}

void
NrRarMessage::SetRaRnti (uint16_t raRnti)
{
  m_raRnti = raRnti;
}

uint16_t
NrRarMessage::GetRaRnti () const
{
  return m_raRnti;
}

void
NrRarMessage::AddRar (Rar rar)
{
  m_rarList.push_back (rar);
}

std::list<NrRarMessage::Rar>::const_iterator
NrRarMessage::RarListBegin () const
{
  return m_rarList.begin ();
}

std::list<NrRarMessage::Rar>::const_iterator
NrRarMessage::RarListEnd () const
{
  return m_rarList.end ();
}

NrDlHarqFeedbackMessage::NrDlHarqFeedbackMessage (void)
{
  SetMessageType (NrControlMessage::DL_HARQ);
}


NrDlHarqFeedbackMessage::~NrDlHarqFeedbackMessage (void)
{

}

void
NrDlHarqFeedbackMessage::SetDlHarqFeedback (DlHarqInfo m)
{
  m_dlHarqInfo = m;
}


DlHarqInfo
NrDlHarqFeedbackMessage::GetDlHarqFeedback (void)
{
  return m_dlHarqInfo;
}

NrSrsMessage::NrSrsMessage ()
{
  SetMessageType (NrControlMessage::SRS);
}

std::ostream &
operator<< (std::ostream &os, const LteNrTddSlotType &item)
{
  switch (item)
    {
    case LteNrTddSlotType::DL:
      os << "DL";
      break;
    case LteNrTddSlotType::F:
      os << "F";
      break;
    case LteNrTddSlotType::S:
      os << "S";
      break;
    case LteNrTddSlotType::UL:
      os << "UL";
      break;
    }
  return os;
}

}


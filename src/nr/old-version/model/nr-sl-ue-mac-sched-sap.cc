/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2020 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
*/
#include "nr-sl-ue-mac-sched-sap.h"

namespace ns3 {

std::ostream &operator<< (std::ostream &os,
                          const NrSlUeMacSchedSapProvider::SchedUeNrSlReportBufferStatusParams &p)
{
  os << "RNTI: " << p.rnti << " LCId: " << static_cast<uint32_t> (p.lcid)
     << " RLCTxQueueSize: " << p.txQueueSize
     << " B, RLCTXHolDel: " << p.txQueueHolDelay
     << " ms, RLCReTXQueueSize: " << p.retxQueueSize
     << " B, RLCReTXHolDel: " << p.retxQueueHolDelay
     << " ms, RLCStatusPduSize: " << p.statusPduSize
     << " B, source layer 2 id: " << p.srcL2Id
     << ", destination layer 2 id " << p.dstL2Id;
  return os;
}

bool
NrSlUeMacSchedSapProvider::NrSlSlotInfo::operator < (const NrSlSlotInfo &rhs) const
{
  return sfn < rhs.sfn;
}

} // namespace ns3

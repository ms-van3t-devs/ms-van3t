/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
*   Copyright (c) 2018 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
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
#include "nr-mac-sched-sap.h"

namespace ns3 {

std::ostream &operator<< (std::ostream &os,
                          const NrMacSchedSapProvider::SchedDlRlcBufferReqParameters &p)
{
  os << "RNTI: " << p.m_rnti << " LCId: " << static_cast<uint32_t> (p.m_logicalChannelIdentity)
     << " RLCTxQueueSize: " << p.m_rlcTransmissionQueueSize
     << " B, RLCTXHolDel: " << p.m_rlcTransmissionQueueHolDelay
     << " ms, RLCReTXQueueSize: " << p.m_rlcRetransmissionQueueSize
     << " B, RLCReTXHolDel: " << p.m_rlcRetransmissionHolDelay
     << " ms, RLCStatusPduSize: " << p.m_rlcStatusPduSize << " B.";
  return os;
}

NrMacSchedSapUser::~NrMacSchedSapUser ()
{

}

} // namespace ns3

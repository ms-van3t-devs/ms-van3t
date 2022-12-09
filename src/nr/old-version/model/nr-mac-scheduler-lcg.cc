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

#include "nr-mac-scheduler-lcg.h"

#include <ns3/eps-bearer.h>
#include <ns3/log.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacSchedulerLCG");

NrMacSchedulerLC::NrMacSchedulerLC (const LogicalChannelConfigListElement_s &conf)
  : m_id (conf.m_logicalChannelIdentity)
{
  EpsBearer bearer (static_cast<EpsBearer::Qci> (conf.m_qci));

  m_delayBudget = MilliSeconds (bearer.GetPacketDelayBudgetMs ());
  m_isGbr = bearer.IsGbr ();
  m_PER = bearer.GetPacketErrorLossRate ();
}

void
NrMacSchedulerLCG::AssignedData (uint8_t lcId, uint32_t size)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_lcMap.size () > 0);

  // Update queues: RLC tx order Status, ReTx, Tx. To understand this, you have
  // to see RlcAm::NotifyTxOpportunity
  NS_LOG_INFO ("Status of LCID " << static_cast<uint32_t> (lcId) << ": RLCSTATUS=" <<
               m_lcMap.at (lcId)->m_rlcStatusPduSize << ", RLC Retr=" <<
               m_lcMap.at (lcId)->m_rlcRetransmissionQueueSize << ", RLC TX=" <<
               m_lcMap.at (lcId)->m_rlcTransmissionQueueSize);

  if ((m_lcMap.at (lcId)->m_rlcStatusPduSize > 0) && (size >= m_lcMap.at (lcId)->m_rlcStatusPduSize))
    {
      // Update status queue
      m_lcMap.at (lcId)->m_rlcStatusPduSize = 0;
    }
  else if ((m_lcMap.at (lcId)->m_rlcRetransmissionQueueSize > 0) && (size >= m_lcMap.at (lcId)->m_rlcRetransmissionQueueSize))
    {
      // update retx queue
      m_lcMap.at (lcId)->m_rlcRetransmissionQueueSize = 0;
    }
  else if (m_lcMap.at (lcId)->m_rlcTransmissionQueueSize > 0)
    {
      // uint32_t rlcOverhead = 4; // TODO: What's that value? it was 4 for LCI 1 and 2 for others
      // update transmission queue without overhead, that I don't understand
      if (m_lcMap.at (lcId)->m_rlcTransmissionQueueSize <= size) // - overhead)
        {
          m_lcMap.at (lcId)->m_rlcTransmissionQueueSize = 0;
        }
      else
        {
          NS_ASSERT (m_lcMap.at (lcId)->m_rlcTransmissionQueueSize > size);
          m_lcMap.at (lcId)->m_rlcTransmissionQueueSize -= size; //- rlcOverhead;
        }
    }

  if (m_totalSize >= size)
    {
      m_totalSize -= size;
    }
  else
    {
      m_totalSize = 0;
    }

}

} // namespace ns3

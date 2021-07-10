/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

 * Created by:
 *  Marco Malinverno, Politecnico di Torino (marco.malinverno1@gmail.com)
 *  Francesco Raviglione, Politecnico di Torino (francescorav.es483@gmail.com)
*/

#include "ITSSOriginatingTableEntry.h"

namespace ns3 {
  ITSSOriginatingTableEntry::ITSSOriginatingTableEntry() {
    m_status = STATE_UNSET;
    m_actionid.originatingStationID = 0;
    m_actionid.sequenceNumber = -1;
    m_referenceTime = -1;
  }

  ITSSOriginatingTableEntry::ITSSOriginatingTableEntry(Packet asnDenmPacket, denm_table_state_t status, DEN_ActionID_t actionID)
  {
    m_denm_encoded = asnDenmPacket;
    m_status = status;
    m_actionid = actionID;
  }

  ITSSOriginatingTableEntry::ITSSOriginatingTableEntry(Packet asnDenmPacket, denm_table_state_t status, DEN_ActionID_t actionID, long referenceTime)
  {
    m_denm_encoded = asnDenmPacket;
    m_status = status;
    m_actionid = actionID;
    m_referenceTime = referenceTime;
  }
}

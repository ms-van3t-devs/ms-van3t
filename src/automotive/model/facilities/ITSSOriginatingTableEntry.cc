#include "ITSSOriginatingTableEntry.h"

namespace ns3 {
  ITSSOriginatingTableEntry::ITSSOriginatingTableEntry() {
    m_status = STATE_UNSET;
    m_actionid.originatingStationID = 0;
    m_actionid.sequenceNumber = -1;
    m_referenceTime = -1;
  }

  ITSSOriginatingTableEntry::ITSSOriginatingTableEntry(Packet asnDenmPacket, denm_table_state_t status, ActionID_t actionID)
  {
    m_denm_encoded = asnDenmPacket;
    m_status = status;
    m_actionid = actionID;
  }

  ITSSOriginatingTableEntry::ITSSOriginatingTableEntry(Packet asnDenmPacket, denm_table_state_t status, ActionID_t actionID, long referenceTime)
  {
    m_denm_encoded = asnDenmPacket;
    m_status = status;
    m_actionid = actionID;
    m_referenceTime = referenceTime;
  }
}

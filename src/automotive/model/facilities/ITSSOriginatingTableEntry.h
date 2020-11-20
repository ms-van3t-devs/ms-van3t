#ifndef ITSSORIGINATINGTABLEENTRY_H
#define ITSSORIGINATINGTABLEENTRY_H

#include "ns3/DENM.h"
#include "ns3/packet.h"

namespace ns3 {
  class ITSSOriginatingTableEntry
  {
  public:
    typedef enum {
      STATE_UNSET,
      STATE_ACTIVE,
      STATE_CANCELLED,
      STATE_NEGATED
    } denm_table_state_t;

    ITSSOriginatingTableEntry();
    ITSSOriginatingTableEntry(Packet asnDenmPacket, denm_table_state_t status, ActionID_t actionID);
    ITSSOriginatingTableEntry(Packet asnDenmPacket, denm_table_state_t status, ActionID_t actionID, long referenceTime);

    /* Setters */
    void setStatus(denm_table_state_t status) {m_status=status;}
    void setDENMPacket(Packet DENMPacket) {m_denm_encoded=DENMPacket;}

    /* Getters */
    denm_table_state_t getStatus(void) {return m_status;}
    Packet getDENMPacket(void) {return m_denm_encoded;}
    long getReferenceTime(void) {return m_referenceTime;}

  private:
    ActionID_t m_actionid;
    Packet m_denm_encoded;
    denm_table_state_t m_status;
    long m_referenceTime;

  };

}

#endif // ITSSORIGINATINGTABLEENTRY_H

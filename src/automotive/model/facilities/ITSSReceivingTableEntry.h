#ifndef ITSSRECEIVINGTABLEENTRY_H
#define ITSSRECEIVINGTABLEENTRY_H
#include "ns3/DENM.h"
#include "ns3/packet.h"

#define ITSS_RX_ENTRY_TERMINATION_UNSET -1

namespace ns3 {
  class ITSSReceivingTableEntry
  {
  public:
    typedef enum {
      STATE_UNSET,
      STATE_ACTIVE,
      STATE_CANCELLED,
      STATE_NEGATED
    } denm_table_state_t;

    ITSSReceivingTableEntry();
    ITSSReceivingTableEntry(Packet asnDenmPacket, denm_table_state_t status, ActionID_t actionID, long referenceTime, long detectionTime);
    ITSSReceivingTableEntry(Packet asnDenmPacket, denm_table_state_t status, ActionID_t actionID, long referenceTime, long detectionTime, Termination_t *termination_ptr);

    /* Setters */
    void setStatus(denm_table_state_t status) {m_status=status;}
    void setDENMPacket(Packet DENMPacket) {m_denm_encoded=DENMPacket;}
    void setTermination_if_available(Termination_t *termination_ptr) {m_termination = termination_ptr==NULL ? ITSS_RX_ENTRY_TERMINATION_UNSET : *(termination_ptr);}
    void setTermination(Termination_t termination) {m_termination=termination;}

    /* Getters */
    denm_table_state_t getStatus(void) {return m_status;}
    Packet getDENMPacket(void) {return m_denm_encoded;}
    long getReferenceTime(void) {return m_referenceTime;}
    long getDetectionTime(void) {return m_detectionTime;}
    long getTermination(void) {return m_termination;}
    bool isTerminationSet(void) {return m_termination!=ITSS_RX_ENTRY_TERMINATION_UNSET;}

  private:
    ActionID_t m_actionid;
    Packet m_denm_encoded;
    denm_table_state_t m_status;
    long m_referenceTime;
    long m_detectionTime;
    long m_termination;
  };

}

#endif // ITSSRECEIVINGTABLEENTRY_H

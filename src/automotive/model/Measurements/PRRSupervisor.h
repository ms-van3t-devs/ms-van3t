#ifndef PRRSUPERVISOR_H
#define PRRSUPERVISOR_H

#include <list>
#include <unordered_map>
#include <string>
#include "ns3/traci-client.h"
#include "ns3/event-id.h"

namespace ns3 {
  class PRRSupervisor : public Object {
    typedef struct baselineVehicleData {
      std::list<uint64_t> vehList;
      int x;
    } baselineVehicleData_t;

    public:
      static TypeId GetTypeId();
      PRRSupervisor() {}
      PRRSupervisor(int baseline_m) : m_baseline_m(baseline_m) {}
      virtual ~PRRSupervisor();

      static std::string bufToString(uint8_t *buf, uint32_t bufsize);

      void setTraCIClient(Ptr<TraciClient> traci_ptr) {m_traci_ptr = traci_ptr;}

      void signalSentPacket(std::string buf,double lat,double lon);
      void signalReceivedPacket(std::string buf,uint64_t vehicleID);

      double getAveragePRR(void) {return m_avg_PRR;}
      double getAverageLatency(void) {return m_avg_latency_ms;}
    private:
      void computePRR(std::string buf);

      std::unordered_map<std::string,baselineVehicleData_t> m_packetbuff_map;
      std::unordered_map<std::string,int64_t> m_latency_map;
      int m_count = 0;
      uint64_t m_count_latency = 0;
      double m_avg_PRR = 0.0;
      double m_avg_latency_ms = 0.0;
      Ptr<TraciClient> m_traci_ptr = nullptr;
      double m_baseline_m = 150.0;

      std::list<EventId> eventList;
  };
}


#endif // PRRSUPERVISOR_H

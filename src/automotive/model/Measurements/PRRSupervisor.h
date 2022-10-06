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

      void signalSentPacket(std::string buf,double lat,double lon,uint64_t vehicleID);
      void signalReceivedPacket(std::string buf,uint64_t vehicleID);

      double getAveragePRR_overall(void) {return m_avg_PRR;}
      double getAverageLatency_overall(void) {return m_avg_latency_ms;}

      double getAveragePRR_vehicle(uint64_t vehicleID) {return m_avg_PRR_per_veh[vehicleID];}
      double getAverageLatency_vehicle(uint64_t vehicleID) {return m_avg_latency_ms_per_veh[vehicleID];}
    private:
      void computePRR(std::string buf);

      std::unordered_map<std::string,baselineVehicleData_t> m_packetbuff_map;
      std::unordered_map<std::string,int64_t> m_latency_map;
      std::unordered_map<std::string,uint64_t> m_vehicleid_map;
      int m_count = 0;
      uint64_t m_count_latency = 0;
      double m_avg_PRR = 0.0;
      double m_avg_latency_ms = 0.0;
      std::unordered_map<uint64_t,double> m_avg_PRR_per_veh;
      std::unordered_map<uint64_t,double> m_avg_latency_ms_per_veh;
      std::unordered_map<uint64_t,int> m_count_per_veh;
      std::unordered_map<uint64_t,uint64_t> m_count_latency_per_veh;
      Ptr<TraciClient> m_traci_ptr = nullptr;
      double m_baseline_m = 150.0;

      std::list<EventId> eventList;
  };
}


#endif // PRRSUPERVISOR_H

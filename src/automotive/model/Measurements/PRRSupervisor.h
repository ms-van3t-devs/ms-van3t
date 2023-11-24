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

      typedef enum messageType {
              messageType_unsupported = 0,
              messageType_denm	= 1,
              messageType_cam	= 2,
              messageType_poi	= 3,
              messageType_spatem	= 4,
              messageType_mapem	= 5,
              messageType_ivim	= 6,
              messageType_ev_rsr	= 7,
              messageType_tistpgtransaction	= 8,
              messageType_srem	= 9,
              messageType_ssem	= 10,
              messageType_evcsn	= 11,
              messageType_saem	= 12,
              messageType_rtcmem	= 13,
              messageType_cpm     = 14,
              messageType_imzm    = 15,
              messageType_vam     = 16,
              messageType_dsm     = 17,
              messageType_pcim    = 18,
              messageType_pcvm    = 19,
              messageType_mcm     = 20,
              messageType_pam     = 21,
              messageType_cem    = 200,
              messageType_GNbeacon = 1000
      } messageType_e;

      static TypeId GetTypeId();
      PRRSupervisor() {}
      PRRSupervisor(int baseline_m) : m_baseline_m(baseline_m) {}
      virtual ~PRRSupervisor();

      static std::string bufToString(uint8_t *buf, uint32_t bufsize);

      void setTraCIClient(Ptr<TraciClient> traci_ptr) {m_traci_ptr = traci_ptr;}

      void signalSentPacket(std::string buf,double lat,double lon,uint64_t vehicleID, messageType_e messagetype);
      void signalReceivedPacket(std::string buf,uint64_t vehicleID);

      double getAveragePRR_overall(void) {return m_avg_PRR;}
      double getAverageLatency_overall(void) {return m_avg_latency_ms;}

      double getAveragePRR_vehicle(uint64_t vehicleID) {return m_avg_PRR_per_veh[vehicleID];}
      double getAverageLatency_vehicle(uint64_t vehicleID) {return m_avg_latency_ms_per_veh[vehicleID];}

      double getAveragePRR_messagetype(messageType_e messagetype) {return m_avg_PRR_per_messagetype[messagetype];}
      double getAverageLatency_messagetype(messageType_e messagetype) {return m_avg_latency_ms_per_messagetype[messagetype];}

      void enableVerboseOnStdout() {m_verbose_stdout=true;}
      void disableVerboseOnStdout() {m_verbose_stdout=false;}

      void addExcludedID(uint64_t m_vehid) {m_excluded_vehID_list.insert(m_vehid); m_excluded_vehID_enabled=true;}
      void clearExcludedIDs() {m_excluded_vehID_list.clear(); m_excluded_vehID_enabled=false;}
    private:
      void computePRR(std::string buf);

      std::unordered_map<std::string,baselineVehicleData_t> m_packetbuff_map;
      std::unordered_map<std::string,int64_t> m_latency_map;
      std::unordered_map<std::string,uint64_t> m_vehicleid_map;
      std::unordered_map<std::string,messageType_e> m_messagetype_map;
      int m_count = 0;
      uint64_t m_count_latency = 0;
      double m_avg_PRR = 0.0;
      double m_avg_latency_ms = 0.0;
      std::unordered_map<uint64_t,double> m_avg_PRR_per_veh;
      std::unordered_map<uint64_t,double> m_avg_latency_ms_per_veh;
      std::unordered_map<uint64_t,int> m_count_per_veh;
      std::unordered_map<uint64_t,uint64_t> m_count_latency_per_veh;

      std::unordered_map<messageType_e,double> m_avg_PRR_per_messagetype;
      std::unordered_map<messageType_e,double> m_avg_latency_ms_per_messagetype;
      std::unordered_map<messageType_e,int> m_count_per_messagetype;
      std::unordered_map<messageType_e,uint64_t> m_count_latency_per_messagetype;

      Ptr<TraciClient> m_traci_ptr = nullptr;
      double m_baseline_m = 150.0;

      bool m_verbose_stdout = false;

      std::list<EventId> eventList;

      std::set<uint64_t> m_excluded_vehID_list;
      bool m_excluded_vehID_enabled = false;
  };
}


#endif // PRRSUPERVISOR_H

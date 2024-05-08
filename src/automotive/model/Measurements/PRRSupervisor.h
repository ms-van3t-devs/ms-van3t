#ifndef PRRSUPERVISOR_H
#define PRRSUPERVISOR_H

#include <list>
#include <unordered_map>
#include <string>
#include "ns3/traci-client.h"
#include "ns3/event-id.h"

namespace ns3 {

/**
 * \ingroup automotive
 *
 * \brief This class is used to supervise the Packet Reception Ratio (PRR) and One-way latency of messages in the simulation.
 *
 * This class provides capabilities for computing PRR and Latency values for:
 * - All the messages sent and received in the simulation
 * - All the messages sent and received by a specific vehicle
 * - All the messages of a specific type (i.e CAM, DENM, CPM, IVIM or VAM) sent and received in the simulation
 */
class PRRSupervisor : public Object {

  typedef struct baselineVehicleData {
    std::list<uint64_t> nodeList;
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
  /**
   * \brief Default constructor
   *
   */
  PRRSupervisor() {}
  /**
   * \brief Constructor
   *
   * This constructor initializes the PRRSupervisor object.
   * @param baseline_m The baseline distance in meters to consider for a packet to be received.
   */
  PRRSupervisor(int baseline_m) : m_baseline_m(baseline_m) {}
  virtual ~PRRSupervisor();

  static std::string bufToString(uint8_t *buf, uint32_t bufsize);

  /**
   * @brief Set the TraCI client pointer.
   * @param traci_ptr
   */
  void setTraCIClient(Ptr<TraciClient> traci_ptr) {m_traci_ptr = traci_ptr;}

  /**
   * @brief This function is called everytime a packet is sent in the simulation by the GeoNet object.
   * @param buf  The buffer containing the packet.
   * @param lat   The latitude of the sender.
   * @param lon   The longitude of the sender.
   * @param vehicleID  The ID of the sender.
   * @param messagetype  The ETSI type of the message.
   */
  void signalSentPacket(std::string buf,double lat,double lon,uint64_t vehicleID, messageType_e messagetype);
  /**
   * @brief This function is called everytime a packet is received in the simulation by the GeoNet object.
   * @param buf  The buffer containing the packet.
   * @param vehicleID  The ID of the receiver.
   */
  void signalReceivedPacket(std::string buf,uint64_t vehicleID);

  /**
   * @brief Get the average PRR for all the messages sent and received in the simulation.
   * @return  The average PRR.
   */
  double getAveragePRR_overall(void) {return m_avg_PRR;}
  /**
   * @brief Get the average latency for all the messages sent and received in the simulation.
   * @return  The average latency [ms]
   */
  double getAverageLatency_overall(void) {return m_avg_latency_ms;}

  /**
   * @brief Get the average PRR for all the messages sent and received by a specific vehicle.
   * @param vehicleID  The ID of the vehicle.
   * @return  The average PRR.
   */
  double getAveragePRR_vehicle(uint64_t vehicleID) {return m_avg_PRR_per_veh[vehicleID];}
  /**
   * @brief Get the average latency for all the messages sent and received by a specific vehicle.
   * @param vehicleID  The ID of the vehicle.
   * @return  The average latency [ms]
   */
  double getAverageLatency_vehicle(uint64_t vehicleID) {return m_avg_latency_ms_per_veh[vehicleID];}

  double getAveragePRR_pedestrian(uint64_t pedestrianID) {return m_avg_PRR_per_ped[pedestrianID];}
  double getAverageLatency_pedestrian(uint64_t pedestrianID) {return m_avg_latency_ms_per_ped[pedestrianID];}

  /**
   * @brief Get the average PRR for all the messages of a specific type sent and received in the simulation.
   * @param messagetype  The ETSI type of the message.
   * @return  The average PRR.
   */
  double getAveragePRR_messagetype(messageType_e messagetype) {return m_avg_PRR_per_messagetype[messagetype];}
  /**
   * @brief Get the average latency for all the messages of a specific type sent and received in the simulation.
   * @param messagetype  The ETSI type of the message.
   * @return  The average latency [ms]
   */
  double getAverageLatency_messagetype(messageType_e messagetype) {return m_avg_latency_ms_per_messagetype[messagetype];}

  void enableVerboseOnStdout() {m_verbose_stdout=true;}
  void disableVerboseOnStdout() {m_verbose_stdout=false;}

  /**
   * @brief Add a vehicle ID to the list of IDs to be excluded from the PRR computation.
   * @param m_vehid The ID of the vehicle to be excluded.
   */
  void addExcludedID(uint64_t m_vehid) {m_excluded_vehID_list.insert(m_vehid); m_excluded_vehID_enabled=true;}
  void clearExcludedIDs() {m_excluded_vehID_list.clear(); m_excluded_vehID_enabled=false;}


  /**
   * @brief Modify the timeout value for the PRR computation.
   *
   * This function lets the user customize the timeout value (in seconds) for the PRR computation.
   * By default, the PRRSupervisor starts a timeout of 3 seconds when a packet is sent. It will
   * then record all the times the same packet is received by other road users within the baseline and within
   * the 3 seconds (increasing each time the "X" at the numerator of the formula PRR = X/Y). After 3 seconds
   * the PRR for that packet is computed. Therefore, all packets with a latency greater than 3 seconds are
   * considered as lost (and they likely are lost).
   * The default value of 3 seconds has been considered as worst case, considering that this is the same
   * periodicity as the one of GeoNetworking beacons, when no CAMs are being exchanged.
   * With this function, the timeout can be adjusted depending on the user needs.
   *
   * @param prr_comp_timeout_sec
   */
  void modifyPRRComputationTimeout(double prr_comp_timeout_sec) {m_pprcomp_timeout=prr_comp_timeout_sec;}
private:
  void computePRR(std::string buf);

  std::unordered_map<std::string,baselineVehicleData_t> m_packetbuff_map; //! key: packet, value: list of vehicle IDs
  std::unordered_map<std::string,int64_t> m_latency_map; //! key: packet, value: latency
  std::unordered_map<std::string,uint64_t> m_id_map; //! key: packet, value: ID
  std::unordered_map<std::string,messageType_e> m_messagetype_map; //! key: packet, value: message type
  std::unordered_map<std::string,StationType_t> m_stationtype_map; //! key: packet, value: station type

  int m_count = 0;
  uint64_t m_count_latency = 0;
  double m_avg_PRR = 0.0;
  double m_avg_latency_ms = 0.0;
  std::unordered_map<uint64_t,double> m_avg_PRR_per_veh; //! key: vehicle ID, value: PRR
  std::unordered_map<uint64_t,double> m_avg_PRR_per_ped; //! key: pedestrian ID, value: PRR
  std::unordered_map<uint64_t,double> m_avg_PRR_per_rsu; //! key: RSU ID, value: PRR
  std::unordered_map<uint64_t,double> m_avg_latency_ms_per_veh;  //! key: vehicle ID, value: latency
  std::unordered_map<uint64_t,double> m_avg_latency_ms_per_ped;  //! key: pedestrian ID, value: latency
  std::unordered_map<uint64_t,double> m_avg_latency_ms_per_rsu;  //! key: RSU ID, value: latency
  std::unordered_map<uint64_t,int> m_count_per_veh; //! key: vehicle ID, value: count
  std::unordered_map<uint64_t,int> m_count_per_ped;  //! key: pedestrian ID, value: count
  std::unordered_map<uint64_t,int> m_count_per_rsu;  //! key: RSU ID, value: count
  std::unordered_map<uint64_t,uint64_t> m_count_latency_per_veh;    //! key: vehicle ID, value: latency
  std::unordered_map<uint64_t,uint64_t> m_count_latency_per_ped;    //! key: pedestrian ID, value: latency
  std::unordered_map<uint64_t,uint64_t> m_count_latency_per_rsu;    //! key: RSU ID, value: latency

  std::unordered_map<messageType_e,double> m_avg_PRR_per_messagetype; //! key: message type, value: PRR
  std::unordered_map<messageType_e,double> m_avg_latency_ms_per_messagetype; //! key: message type, value: latency
  std::unordered_map<messageType_e,int> m_count_per_messagetype; //! key: message type, value: count
  std::unordered_map<messageType_e,uint64_t> m_count_latency_per_messagetype; //! key: message type, value: latency

  Ptr<TraciClient> m_traci_ptr = nullptr;
  double m_baseline_m = 150.0;

  bool m_verbose_stdout = false;

  std::list<EventId> eventList;

  std::set<uint64_t> m_excluded_vehID_list;
  bool m_excluded_vehID_enabled = false;

  double m_pprcomp_timeout = 3.0;

  uint64_t m_stationId_baseline = 1000000;
};
}


#endif // PRRSUPERVISOR_H
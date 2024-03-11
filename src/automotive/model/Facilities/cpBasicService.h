#ifndef CPBASICSERVICE_H
#define CPBASICSERVICE_H

#include "ns3/socket.h"
#include "ns3/core-module.h"
#include "ns3/vdp.h"
#include "ns3/asn_utils.h"
#include "ns3/btp.h"
#include "ns3/btpHeader.h"
#include "ns3/Seq.hpp"
#include "ns3/Getter.hpp"
#include "ns3/vdp.h"
#include "ns3/vdpTraci.h"
#include "ns3/LDM.h"
#include "ns3/ldm-utils.h"
#include "signalInfoUtils.h"

extern "C" {
  #include "ns3/CollectivePerceptionMessage.h"
}

namespace ns3
{

class CPBasicService: public SignalInfoUtils
{ 
public:
  CPBasicService();
  void setStationID(unsigned long fixed_stationid);
  void setStationType(long fixed_stationtype);
  void setStationProperties(unsigned long fixed_stationid,long fixed_stationtype);
  void setSocketTx(Ptr<Socket> socket_tx) {m_btp->setSocketTx (socket_tx);}
  void setSocketRx(Ptr<Socket> socket_rx);
  void setLDM(Ptr<LDM> LDM){m_LDM = LDM;}
  void setVDP(VDP* vdp) {m_vdp=vdp;}
  void setTraCIclient(Ptr<TraciClient> client){m_client=client;}
  void setBTP(Ptr<btp> btp){m_btp = btp;}

  void receiveCpm(BTPDataIndication_t dataIndication, Address from);
  void changeNGenCpmMax(int16_t N_GenCpmMax) {m_N_GenCpmMax=N_GenCpmMax;}
  void addCPRxCallback(std::function<void(asn1cpp::Seq<CollectivePerceptionMessage>, Address)> rx_callback) {m_CPReceiveCallback=rx_callback;}
  void addCPRxCallbackExtended(std::function<void(asn1cpp::Seq<CollectivePerceptionMessage>, Address, StationID_t, StationType_t, SignalInfo)> rx_callback) {m_CPReceiveCallbackExtended=rx_callback;}
  void setRealTime(bool real_time){m_real_time=real_time;}
  void startCpmDissemination();
  uint64_t terminateDissemination();
  void setRedundancyMitigation(bool choice){m_redundancy_mitigation = choice;}
  void disableRedundancyMitigation(){m_redundancy_mitigation = false;}

  const long T_GenCpmMin_ms = 100;
  const long T_GenCpm_ms = 100;
  const long T_GenCpmMax_ms = 1000;
  const long m_T_AddSensorInformation = 1000;


private:

  void initDissemination();
  void RSUDissemination();
  void checkCpmConditions();
  void generateAndEncodeCPM();
  int64_t computeTimestampUInt64();
  bool checkCPMconditions(std::vector<LDM::returnedVehicleData_t>::iterator it);
  double cartesian_dist(double lon1, double lat1, double lon2, double lat2);

  std::function<void(asn1cpp::Seq<CollectivePerceptionMessage>, Address)> m_CPReceiveCallback;
  std::function<void(asn1cpp::Seq<CollectivePerceptionMessage>, Address, Ptr<Packet>)> m_CPReceiveCallbackPkt;
  std::function<void(asn1cpp::Seq<CollectivePerceptionMessage>, Address, StationID_t, StationType_t, SignalInfo)> m_CPReceiveCallbackExtended;


  Ptr<btp> m_btp;

  long m_T_CheckCpmGen_ms;
  long m_T_LastSensorInfoContainer;

  long m_T_GenCpm_ms;
  int16_t m_N_GenCpm;
  int16_t m_N_GenCpmMax;

  int64_t lastCpmGen;
  int64_t lastCpmGenLowFrequency;
  int64_t lastCpmGenSpecialVehicle;

  bool m_real_time;
  bool m_vehicle;
  bool m_redundancy_mitigation;
  VDP* m_vdp;
  Ptr<TraciClient> m_client;

  Ptr<Socket> m_socket_tx; // Socket TX

  Ptr<LDM> m_LDM;

  StationID_t m_station_id;
  StationType_t m_stationtype;

  // Previous Cpm relevant values
  double m_prev_heading;
  double m_prev_distance;
  double m_prev_speed;
  std::vector<long> m_lastCPM_POs;

  // Statistic: number of Cpms successfully sent since the CA Basic Service has been started
  // The CA Basic Service can count up to 18446744073709551615 (UINT64_MAX) Cpms
  uint64_t m_cpm_sent;

  // ns-3 event IDs used to properly stop the simulation with terminateDissemination()
  EventId m_event_cpmDisseminationStart;
  EventId m_event_cpmSend;
};
}
#endif // CPBASICSERVICE_H

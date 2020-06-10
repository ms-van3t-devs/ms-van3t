#ifndef CABASICSERVICE_H
#define CABASICSERVICE_H

#include "ns3/socket.h"
#include "ns3/core-module.h"
#include "ns3/vdpTraci.h"
#include "ns3/asn_utils.h"

#define CURRENT_VDP_TYPE VDPTraCI

namespace ns3
{
  typedef enum {
    CAM_NO_ERROR=0,
    CAM_WRONG_INTERVAL=1,
    CAM_ALLOC_ERROR=2,
    CAM_NO_RSU_CONTAINER=3,
    CAM_ASN1_UPER_ENC_ERROR=4,
    CAM_CANNOT_SEND=5
  } CABasicService_error_t;

  class CABasicService
  {
  public:
    CABasicService();
    CABasicService(unsigned long fixed_stationid,long fixed_stationtype,CURRENT_VDP_TYPE vdp,bool real_time,bool is_vehicle);
    CABasicService(unsigned long fixed_stationid,long fixed_stationtype,CURRENT_VDP_TYPE vdp,bool real_time,bool is_vehicle,Ptr<Socket> socket_tx);

    void setStationProperties(unsigned long fixed_stationid,long fixed_stationtype) {m_station_id=fixed_stationid; m_stationtype=fixed_stationtype;}
    void setStationID(unsigned long fixed_stationid) {m_station_id=fixed_stationid;}
    void setStationType(long fixed_stationtype) {m_stationtype=fixed_stationtype;}
    void setSocketTx(Ptr<Socket> socket_tx) {m_socket_tx=socket_tx;}
    void setRSU() {m_vehicle=false;}
    void setVDP(CURRENT_VDP_TYPE vdp) {m_vdp=vdp;}

    void receiveCam(Ptr<Socket> socket);
    void changeNGenCamMax(int16_t N_GenCamMax) {m_N_GenCamMax=N_GenCamMax;}
    void changeRSUGenInterval(long RSU_GenCam_ms) {m_RSU_GenCam_ms=RSU_GenCam_ms;}
    void addCARxCallback(std::function<void(CAM_t *, Address)> rx_callback) {m_CAReceiveCallback=rx_callback;}

    void startCamDissemination();
    void startCamDissemination(double desync_s);

    uint64_t terminateDissemination();

    const long T_GenCamMin_ms = 100;
    const long T_GenCamMax_ms = 1000;

  private:
    void initDissemination();
    void RSUDissemination();
    void checkCamConditions();
    CABasicService_error_t generateAndEncodeCam();
    int64_t computeTimestampUInt64();

    std::function<void(CAM_t *, Address)> m_CAReceiveCallback;

    long m_T_CheckCamGen_ms;
    long m_T_GenCam_ms;
    int16_t m_N_GenCam;
    int16_t m_N_GenCamMax;

    long m_RSU_GenCam_ms; // CAM generation interval for RSU ITS-Ss

    int64_t lastCamGen;
    int64_t lastCamGenLowFrequency;
    int64_t lastCamGenSpecialVehicle;

    bool m_real_time;
    bool m_vehicle;
    CURRENT_VDP_TYPE m_vdp;

    Ptr<Socket> m_socket_tx; // Socket TX

    StationID_t m_station_id;
    StationType_t m_stationtype;

    // Previous CAM relevant values
    double m_prev_heading;
    double m_prev_distance;
    double m_prev_speed;

    // Statistic: number of CAMs successfully sent since the CA Basic Service has been started
    // The CA Basic Service can count up to 18446744073709551615 (UINT64_MAX) CAMs
    uint64_t m_cam_sent;

    // ns-3 event IDs used to properly stop the simulation with terminateDissemination()
    EventId m_event_camDisseminationStart;
    EventId m_event_camCheckConditions;
    EventId m_event_camRsuDissemination;


  };
}

#endif // CABASICSERVICE_H

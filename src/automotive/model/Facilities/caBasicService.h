#ifndef CABASICSERVICE_H
#define CABASICSERVICE_H

#include "ns3/socket.h"
#include "ns3/core-module.h"
#include "ns3/vdp.h"
#include "ns3/asn_utils.h"
#include "ns3/btp.h"
#include "ns3/btpHeader.h"
#include "ns3/Seq.hpp"
#include "ns3/Getter.hpp"
#include "ns3/LDM.h"
#include "signalInfoUtils.h"

extern "C" {
  #include "ns3/CAM.h"
}

//#define CURRENT_VDP_TYPE VDPTraCI

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

  class CABasicService: public Object, public SignalInfoUtils
  {
  public:
    CABasicService();
    ~CABasicService();
    CABasicService(unsigned long fixed_stationid,long fixed_stationtype,VDP* vdp,bool real_time,bool is_vehicle);
    CABasicService(unsigned long fixed_stationid,long fixed_stationtype,VDP* vdp,bool real_time,bool is_vehicle,Ptr<Socket> socket_tx);

    void setStationProperties(unsigned long fixed_stationid,long fixed_stationtype);
    void setFixedPositionRSU(double latitude_deg, double longitude_deg);
    void setStationID(unsigned long fixed_stationid);
    void setStationType(long fixed_stationtype);
    void setSocketTx(Ptr<Socket> socket_tx) {m_btp->setSocketTx (socket_tx);}
    void setSocketRx(Ptr<Socket> socket_rx);
    void setRSU() {m_vehicle=false;}
    void setVDP(VDP* vdp) {m_vdp=vdp;}
    void setLDM(Ptr<LDM> LDM){m_LDM = LDM;}
    void setBTP(Ptr<btp> btp){m_btp = btp;}

    void receiveCam(BTPDataIndication_t dataIndication, Address from);
    void changeNGenCamMax(int16_t N_GenCamMax) {m_N_GenCamMax=N_GenCamMax;}
    void changeRSUGenInterval(long RSU_GenCam_ms) {m_RSU_GenCam_ms=RSU_GenCam_ms;}
    // Warning: if both the standard and extended callbacks are set, only the standard callback will be called
    void addCARxCallback(std::function<void(asn1cpp::Seq<CAM>, Address)> rx_callback) {m_CAReceiveCallback=rx_callback;}
    void addCARxCallbackExtended(std::function<void(asn1cpp::Seq<CAM>, Address, StationId_t, StationType_t, SignalInfo)> rx_callback) {m_CAReceiveCallbackExtended=rx_callback;}
    void setRealTime(bool real_time){m_real_time=real_time;}

    void setLowFrequencyContainer(bool enable) {m_lowFreqContainerEnabled = enable;}
    void setSpecialVehicleContainer(bool enabled) {m_specialVehContainerEnabled = enabled;}

    void startCamDissemination();
    void startCamDissemination(double desync_s);

    //High frequency RSU container setters
    void setProtectedCommunicationsZonesRSU(asn1cpp::Seq<RSUContainerHighFrequency> sequence) {m_protectedCommunicationsZonesRSU = sequence;}

    uint64_t terminateDissemination();

    const long T_GenCamMin_ms = 100;
    const long T_GenCamMax_ms = 1000;

  private:
    const size_t m_MaxPHLength = 23;

    void initDissemination();
    void RSUDissemination();
    void checkCamConditions();
    CABasicService_error_t generateAndEncodeCam();
    int64_t computeTimestampUInt64();
    void vLDM_handler(asn1cpp::Seq<CAM> decodedCAM);

    // std::function<void(CAM_t *, Address)> m_CAReceiveCallback;
    std::function<void(asn1cpp::Seq<CAM>, Address)> m_CAReceiveCallback;
    std::function<void(asn1cpp::Seq<CAM>, Address, Ptr<Packet>)> m_CAReceiveCallbackPkt;
    std::function<void(asn1cpp::Seq<CAM>, Address, StationId_t, StationType_t, SignalInfo)> m_CAReceiveCallbackExtended;

    Ptr<btp> m_btp;

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
    VDP* m_vdp;

    Ptr<Socket> m_socket_tx; // Socket TX

     Ptr<LDM> m_LDM;

    StationId_t m_station_id;
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

    std::vector<std::pair<ReferencePositionWithConfidence_t,PathHistoryDeltas_t>> m_refPositions;

    //High frequency RSU container
    asn1cpp::Seq<RSUContainerHighFrequency> m_protectedCommunicationsZonesRSU;
    double m_RSUlon;
    double m_RSUlat;

    // Boolean/Enum variables to enable/disable the presence of certain optional containers in the CAM messages
    bool m_lowFreqContainerEnabled;
    bool m_specialVehContainerEnabled;
  };
}

#endif // CABASICSERVICE_H

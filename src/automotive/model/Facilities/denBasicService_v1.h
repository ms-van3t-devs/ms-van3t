#ifndef DENBasicServiceV1_H
#define DENBasicServiceV1_H

#include "asn_utils.h"
#include "denData.h"
#include "ITSSOriginatingTableEntry.h"
#include "ITSSReceivingTableEntry.h"
#include "ns3/core-module.h"
#include "ns3/socket.h"
#include "ns3/btp.h"
#include "ns3/btpHeader.h"
#include "ns3/btpdatarequest.h"
#include <functional>
#include <mutex>
#include <queue>
#include "signalInfoUtils.h"

#define V_O_VALIDITY_INDEX 0
#define T_REPETITION_INDEX 1
#define T_REPETITION_DURATION_INDEX 2

namespace ns3 {

  typedef enum {
    DENMV1_NO_ERROR=0,
    DENMV1_ATTRIBUTES_UNSET=1,
    DENMV1_ALLOC_ERROR=2,
    DENMV1_WRONG_DE_DATA=3,
    DENMV1_WRONG_TABLE_DATA=4,
    DENMV1_T_O_VALIDITY_EXPIRED=5,
    DENMV1_ASN1_UPER_ENC_ERROR=6,
    DENMV1_UNKNOWN_ACTIONID=7,
    DENMV1_UNKNOWN_ACTIONID_RECEIVING=8,
    DENMV1_UNKNOWN_ACTIONID_ORIGINATING=9,
    DENMV1_NON_ACTIVE_ACTIONID_RECEIVING=10,
    DENMV1_NON_ACTIVE_ACTIONID_ORIGINATING=11,
    DENMV1_TX_SOCKET_NOT_SET=12
  } DENBasicServiceV1_error_t;

  class DENBasicServiceV1: public Object, public SignalInfoUtils
  {
    public:
    DENBasicServiceV1();
    DENBasicServiceV1(unsigned long fixed_stationid,long fixed_stationtype,Ptr<Socket> socket_tx);

    // Warning: if both the standard and extended callbacks are set, only the standard callback will be called
    void addDENRxCallback(std::function<void(denData,Address)> rx_callback) {m_DENReceiveCallback=rx_callback;}
    void addDENRxCallbackExtended(std::function<void(denData,Address,unsigned long,long)> rx_callback) {m_DENReceiveCallbackExtended=rx_callback;}

    DENBasicServiceV1_error_t appDENM_trigger(denData data, DEN_ActionID_t &actionid);
    DENBasicServiceV1_error_t appDENM_update(denData data, const DEN_ActionID_t actionid);
    DENBasicServiceV1_error_t appDENM_termination(denData data, const DEN_ActionID_t actionid);
    void receiveDENM(BTPDataIndication_t dataIndication, Address address);

    void setStationProperties(unsigned long fixed_stationid,long fixed_stationtype);
    void setFixedPositionRSU(double latitude_deg, double longitude_deg);
    void setStationID(unsigned long fixed_stationid);
    void setStationType(long fixed_stationtype);

    void setVDP(VDP* vdp) {m_btp->setVDP(vdp);}
    void setBTP(Ptr<btp> btp){m_btp = btp;}

    void setSocketTx(Ptr<Socket> socket_tx);
    void setSocketRx(Ptr<Socket> socket_rx);
    void setGeoArea(GeoArea_t geoArea){m_geoArea = geoArea;}

    void setRealTime(bool real_time){m_real_time=real_time;}

    /* Cleanup function - always call this before terminating the simulation */
    void cleanup(void);

  private:
    bool CheckMainAttributes(void);

    DENBasicServiceV1_error_t fillDENM(asn1cpp::Seq<DENMV1> &denm, denData &data, const DEN_ActionID_t actionID, long referenceTimeLong);

    template<typename MEM_PTR> void setDENTimer(Timer &timer,Time delay,MEM_PTR callback_fcn,DEN_ActionID_t actionID);

    void T_O_ValidityStop(DEN_ActionID_t entry_actionid);
    void T_RepetitionDurationStop(DEN_ActionID_t entry_actionid);
    void T_RepetitionStop(DEN_ActionID_t entry_actionid);

    void T_R_ValidityStop(DEN_ActionID_t entry_actionid);

    template <typename T> static int asn_maybe_assign_optional_data(T *data, T **asn_structure,std::queue<void *> &ptr_queue);

    std::function<void(denData,Address)> m_DENReceiveCallback;
    std::function<void(denData,Address,unsigned long,long)> m_DENReceiveCallbackExtended;

    uint16_t m_port;
    bool m_real_time;
    std::string m_model;

    unsigned long m_station_id;
    long m_stationtype;
    uint16_t m_seq_number;

    Ptr<btp> m_btp;

    GeoArea_t m_geoArea;

    Ptr<Socket> m_socket_tx; // Socket TX

    std::map<std::pair<unsigned long,long>,ITSSOriginatingTableEntry> m_originatingITSSTable;
    std::map<std::pair<unsigned long,long>,ITSSReceivingTableEntry> m_receivingITSSTable;

    std::map<std::pair<unsigned long,long>,std::tuple<Timer,Timer,Timer>> m_originatingTimerTable;
    std::map<std::pair<unsigned long,long>,Timer> m_T_R_Validity_Table;

    /* den_data private fillers (ASN.1 types), used within "receiveDENM" */
    void fillDenDataHeader(asn1cpp::Seq<ItsPduHeaderV1> denm_header, denData &denm_data);
    void fillDenDataManagement(asn1cpp::Seq<ManagementContainerV1> denm_mgmt_container, denData &denm_data);
    void fillDenDataSituation(asn1cpp::Seq<SituationContainerV1> denm_situation_container, denData &denm_data);
    void fillDenDataLocation(asn1cpp::Seq<LocationContainerV1> denm_location_container, denData &denm_data);
    void fillDenDataAlacarte(asn1cpp::Seq<AlacarteContainerV1> denm_alacarte_container, denData &denm_data);

    /*
    * Mutex to protect m_originatingITSSTable when appDENM_update() and the callback for the expiration of the T_Repetion timer may try to
    * access the map concurrently, resulting in a thread-unsafe code.
    */
    std::mutex T_Repetition_Mutex;

    std::queue<void *> m_ptr_queue;
  };

}


#endif // DENBasicServiceV1_H

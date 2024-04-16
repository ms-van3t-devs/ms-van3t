#ifndef IVIBasicService_H
#define IVIBasicService_H

#include "asn_utils.h"
#include "denData.h"
#include "ividata.h"
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
#include "ns3/asn_application.h"
#include "ns3/Seq.hpp"
#include "ns3/Getter.hpp"
#include "ns3/Setter.hpp"
#include "ns3/Encoding.hpp"
#include "ns3/SetOf.hpp"
#include "ns3/SequenceOf.hpp"
#include "ns3/BitString.hpp"
#include "ns3/View.hpp"
#include "ns3/Utils.hpp"
#include "signalInfoUtils.h"

extern "C" {
#include "ns3/IVIM.h"
}

#define V_O_VALIDITY_INDEX 0
#define T_REPETITION_INDEX 1
#define T_REPETITION_DURATION_INDEX 2

namespace ns3 {

  typedef enum {
    IVIM_NO_ERROR=0,
    IVIM_ATTRIBUTES_UNSET=1,
    IVIM_ALLOC_ERROR=2,
    IVIM_WRONG_DE_DATA=3,
    IVIM_WRONG_TABLE_DATA=4,
    IVIM_T_O_VALIDITY_EXPIRED=5,
    IVIM_ASN1_UPER_ENC_ERROR=6,
    IVIM_UNKNOWN_ACTIONID=7,
    IVIM_UNKNOWN_ACTIONID_RECEIVING=8,
    IVIM_UNKNOWN_ACTIONID_ORIGINATING=9,
    IVIM_NON_ACTIVE_ACTIONID_RECEIVING=10,
    IVIM_NON_ACTIVE_ACTIONID_ORIGINATING=11,
    IVIM_TX_SOCKET_NOT_SET=12
  } IVIBasicService_error_t;


typedef enum {
    term_cancellation,
    term_negation
} ivimTerminationType;


/**
 * \ingroup automotive
 * \brief This class implements the basic service for the  Infrastructure to Vehicle Information (IVI) service defined in ETSI TS 103 301 V1.3.1 (2020-02).
 *
 * IVI service is one instantiation of the infrastructure services to manage the generation, transmission and reception of the IVIM messages.
 * An IVIM supports mandatory and advisory road signage such as contextual speeds and road works warnings.
 * IVIM either provides information of physical road signs such as static or variable road signs, virtual signs or road works.
 *
 */

  class IVIBasicService: public SignalInfoUtils
  {
    public:
    /**
     * @brief Constructor
     *
     * This constructor initializes the IVIBasicService object.
     */
    IVIBasicService();
    /**
     * @brief Constructor
     *
     * This constructor initializes the IVIBasicService object.
     * @param fixed_stationid The station ID of the ITS-S.
     * @param fixed_stationtype The station type of the ITS-S.
     * @param socket_tx The socket used for transmitting IVIM messages.
     */
    IVIBasicService(unsigned long fixed_stationid,long fixed_stationtype,Ptr<Socket> socket_tx);
    int n_update;
    int receivedIVIM ;
    bool b_trig;
    bool b_update;
    bool b_canc;
    bool b_neg;
    long timeStamp;

    /**
     * @brief Set Reception callback for IVI messages.
     * @param rx_callback
     */
    void addIVIRxCallback(std::function<void(iviData,Address)> rx_callback) {m_IVIReceiveCallback=rx_callback;}
    void addIVIRxCallbackExtended(std::function<void(iviData,Address,StationID_t,StationType_t,SignalInfo)> rx_callback) {m_IVIReceiveCallbackExtended=rx_callback;}

    /**
     * @brief Trigger an IVIM message.
     * @param Data  The IVI data to be transmitted.
     * @return  error code.
     */
    IVIBasicService_error_t appIVIM_trigger(iviData Data);
    /**
     * @brief Repetition of an IVIM message.
     * @param Data  The IVI data to be transmitted.
     * @return  error code.
     */
    IVIBasicService_error_t appIVIM_repetition(iviData Data);
    /**
     * @brief Update an IVIM message.
     * @param Data  The IVI data to be transmitted.
     * @param actionID  The action ID of the IVIM message.
     * @return  error code.
     */
    IVIBasicService_error_t appIVIM_update(iviData Data, ActionID_t actionID);
    /**
     * @brief Termination of an IVIM message.
     * @param Data  The IVI data to stop being transmitted.
     * @param term  The termination type.
     * @param actionID  The action ID of the IVIM message.
     * @return  error code.
     */
    IVIBasicService_error_t appIVIM_termination(iviData Data, ivimTerminationType term,ActionID_t actionID);
    IVIBasicService_error_t fillIVIM(asn1cpp::Seq<IVIM> &ivim, iviData Data, const ActionID_t actionID);


    /**
     * @brief Process a received IVIM message.
     * @param dataIndication  The received data indication from BTP/GeoNet.
     * @param address  The address of the sender of the IVIM message.
     */
    void receiveIVIM(BTPDataIndication_t dataIndication, Address address);

    /**
     * @brief Set the station ID and station type of the ITS-S.
     *
     * @param fixed_stationid
     * @param fixed_stationtype
     */
    void setStationProperties(unsigned long fixed_stationid,long fixed_stationtype);
    /**
     * @brief Set the fixed position of the ITS-S.
     *
     * @param latitude_deg
     * @param longitude_deg
     */
    void setFixedPositionRSU(double latitude_deg, double longitude_deg);
    /**
     * @brief Set the station ID of the ITS-S.
     * @param fixed_stationid
     */
    void setStationID(unsigned long fixed_stationid);
    /**
     * @brief Set the station type of the ITS-S.
     * @param fixed_stationtype
     */
    void setStationType(long fixed_stationtype);
    ActionID getActionId();

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

    iviData::IVI_glcP_PolygonalLine_t getPolyLine (asn1cpp::Seq<PolygonalLine> line);

    asn1cpp::Seq<PolygonalLine> fillPolyLine(iviData::IVI_glcP_PolygonalLine line);

    std::function<void(iviData,Address)> m_IVIReceiveCallback;
    std::function<void(iviData,Address,StationID_t,StationType_t,SignalInfo)> m_IVIReceiveCallbackExtended;

    uint16_t m_port;
    bool m_real_time;
    std::string m_model;


    StationID_t m_station_id;
    StationType_t m_stationtype;
    uint16_t m_seq_number;

    Ptr<btp> m_btp;

    GeoArea_t m_geoArea;

    Ptr<Socket> m_socket_tx; // Socket TX


    std::queue<void *> m_ptr_queue;
  };

}


#endif // IVIBasicService_H

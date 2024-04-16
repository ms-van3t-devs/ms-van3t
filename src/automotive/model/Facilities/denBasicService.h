#ifndef DENBASICSERVICE_H
#define DENBASICSERVICE_H

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
    DENM_NO_ERROR=0,
    DENM_ATTRIBUTES_UNSET=1,
    DENM_ALLOC_ERROR=2,
    DENM_WRONG_DE_DATA=3,
    DENM_WRONG_TABLE_DATA=4,
    DENM_T_O_VALIDITY_EXPIRED=5,
    DENM_ASN1_UPER_ENC_ERROR=6,
    DENM_UNKNOWN_ACTIONID=7,
    DENM_UNKNOWN_ACTIONID_RECEIVING=8,
    DENM_UNKNOWN_ACTIONID_ORIGINATING=9,
    DENM_NON_ACTIVE_ACTIONID_RECEIVING=10,
    DENM_NON_ACTIVE_ACTIONID_ORIGINATING=11,
    DENM_TX_SOCKET_NOT_SET=12
  } DENBasicService_error_t;

  /**
   * \ingroup automotive
   * \brief This class implements the Dencentralized Environmental Notification Basic Service.
   *
   * This class implements the Dencentralized Environmental Notification Basic Service as defined in ETSI EN 302 637-3 V1.3.1 (2019-04).
   * Decentralized Environmental Notification Message (DENM) is a facilities layer message that is mainly used by ITS applications
   * in order to alert road users of a detected event using ITS communication technologies. DENM is used to describe a variety of
   * events that can be detected by ITS stations (ITS-S).
   */
  class DENBasicService: public Object, public SignalInfoUtils
  {
    public:
    /**
     * @brief Constructor
     *
     * This constructor creates a DENBasicService object with the default values.
     */
    DENBasicService();
    /**
     * @brief Constructor
     *
     * This constructor creates a DENBasicService object with the specified values.
     *
     * @param fixed_stationid The station ID of the DENM sender.
     * @param fixed_stationtype The station type of the DENM sender.
     * @param socket_tx The socket used to send the DENM messages.
     */
    DENBasicService(unsigned long fixed_stationid,long fixed_stationtype,Ptr<Socket> socket_tx);

    // Warning: if both the standard and extended callbacks are set, only the standard callback will be called
    /**
     * @brief Set the callback function for the reception of a DENM message.
     *
     * This function sets the callback function that will be called when a DENM message is received.
     *
     * @param rx_callback The callback function to be called when a DENM message is received.
     */
    void addDENRxCallback(std::function<void(denData,Address)> rx_callback) {m_DENReceiveCallback=rx_callback;}
    void addDENRxCallbackExtended(std::function<void(denData,Address,unsigned long,long,SignalInfo)> rx_callback) {m_DENReceiveCallbackExtended=rx_callback;}

    /**
     * @brief trigger a DENM message
     *
     * This function triggers the transmission of a DENM message.
     *
     * @param data The data to be included in the DENM message.
     * @param actionid  The action ID of the DENM message.
     *
     */
    DENBasicService_error_t appDENM_trigger(denData data, DEN_ActionID_t &actionid);

    /**
     * @brief update a DENM message
     *
     * This function updates a DENM message.
     *
     * @param data The data to be included in the DENM message.
     * @param actionid  The action ID of the DENM message.
     *
     */
    DENBasicService_error_t appDENM_update(denData data, const DEN_ActionID_t actionid);
    /**
     * @brief termination of a DENM message
     *
     * This function terminates a DENM message.
     *
     * @param data The data to be included in the DENM message.
     * @param actionid  The action ID of the DENM message.
     *
     *
     * @param data The data to be included in the DENM message.
     * @param actionid  The action ID of the DENM message.
     *
     */
    DENBasicService_error_t appDENM_termination(denData data, const DEN_ActionID_t actionid);

    /**
     * @brief reception of a DENM message
     *
     * This function is called when a DENM message is received by  the BTP layer.
     *
     * @param dataIndication The data indication of the received DENM message.
     * @param address  The address of the sender of the DENM message.
     *
     */
    void receiveDENM(BTPDataIndication_t dataIndication, Address address);

    /**
     * @brief Set the station properties
     *
     * @param fixed_stationid  Station ID of the ITS-S
     * @param fixed_stationtype  Station type of the ITS-S
     */
    void setStationProperties(unsigned long fixed_stationid,long fixed_stationtype);

    /**
     * @brief Set the fixed position of the RSU (ITS-S)
     *
     * @param latitude_deg  Latitude of the ITS-S in degrees
     * @param longitude_deg  Longitude of the ITS-S in degrees
     */
    void setFixedPositionRSU(double latitude_deg, double longitude_deg);

    /**
     * @brief Set the station ID of the ITS-S
     * @param fixed_stationid
     */
    void setStationID(unsigned long fixed_stationid);

    /**
     * @brief Set the station type of the ITS-S
     * @param fixed_stationtype
     */
    void setStationType(long fixed_stationtype);

    /**
     * @brief Set the VDP object
     * @param vdp
     */
    void setVDP(VDP* vdp) {m_btp->setVDP(vdp);}
    /**
     * @brief Set the BTP object
     * @param btp
     */
    void setBTP(Ptr<btp> btp){m_btp = btp;}

    /**
     * @brief Set the socket used to send the DENM messages
     * @param socket_tx
     */
    void setSocketTx(Ptr<Socket> socket_tx);
    /**
     * @brief Set the socket used to receive the DENM messages
     * @param socket_rx
     */
    void setSocketRx(Ptr<Socket> socket_rx);
    /**
     * @brief Set the GeoArea for which the DENM messages are intended
     * @param geoArea
     */
    void setGeoArea(GeoArea_t geoArea){m_geoArea = geoArea;}

    /**
     * @brief Use real time for timestamps
     *
     * @param real_time  If true, the timestamps will be based on the real time. If false, the timestamps will be based on the simulation time.
     */
    void setRealTime(bool real_time){m_real_time=real_time;}

    /* Cleanup function - always call this before terminating the simulation */
    void cleanup(void);

  private:
    bool CheckMainAttributes(void);

    /**
     * @brief Fill the DENM message
     * @param denm  The ASN.1 DENM message to be filled
     * @param data  The data to be included in the DENM message
     * @param actionID  The action ID of the DENM message
     * @param referenceTimeLong  The reference time of the DENM message
     * @return  DENBasicService error code
     */
    DENBasicService_error_t fillDENM(asn1cpp::Seq<DENM> &denm, denData &data, const DEN_ActionID_t actionID, long referenceTimeLong);

    /**
     * @brief Set the timer for the T_O_Validity timer
     * @tparam MEM_PTR
     * @param timer
     * @param delay
     * @param callback_fcn
     * @param actionID
     */
    template<typename MEM_PTR> void setDENTimer(Timer &timer,Time delay,MEM_PTR callback_fcn,DEN_ActionID_t actionID);

    void T_O_ValidityStop(DEN_ActionID_t entry_actionid);
    void T_RepetitionDurationStop(DEN_ActionID_t entry_actionid);
    void T_RepetitionStop(DEN_ActionID_t entry_actionid);

    void T_R_ValidityStop(DEN_ActionID_t entry_actionid);

    template <typename T> static int asn_maybe_assign_optional_data(T *data, T **asn_structure,std::queue<void *> &ptr_queue);

    //!< Callback function for the reception of a DENM message
    std::function<void(denData,Address)> m_DENReceiveCallback;
        //!< Callback function for the reception of a DENM message (extended)
    std::function<void(denData,Address,unsigned long,long,SignalInfo)> m_DENReceiveCallbackExtended;


    uint16_t m_port;

    bool m_real_time; //! Flag to indicate if the timestamps are based on the real time
    std::string m_model;


    unsigned long m_station_id; //! Station ID of the ITS-S

    long m_stationtype; //! Station type of the ITS-S

    uint16_t m_seq_number; //! Sequence number of the DENM messages


    Ptr<btp> m_btp; //! BTP object used to send the DENM messages


    GeoArea_t m_geoArea; //! GeoArea for which the DENM messages are intended


    Ptr<Socket> m_socket_tx; //! Socket used to send the DENM messages


    std::map<std::pair<unsigned long,long>,ITSSOriginatingTableEntry> m_originatingITSSTable; //! Originating ITS-S table

    std::map<std::pair<unsigned long,long>,ITSSReceivingTableEntry> m_receivingITSSTable; //! Receiving ITS-S table


    std::map<std::pair<unsigned long,long>,std::tuple<Timer,Timer,Timer>> m_originatingTimerTable; //! Originating timer table

    std::map<std::pair<unsigned long,long>,Timer> m_T_R_Validity_Table; //! Validity timer table

    /* den_data private fillers (ASN.1 types), used within "receiveDENM" */
    /**
     * @brief Fill the DENM message header
     * @param denm_header  The ASN.1 DENM header to be filled
     * @param denm_data  The data to be included in the DENM message
     */
    void fillDenDataHeader(asn1cpp::Seq<ItsPduHeader> denm_header, denData &denm_data);
    /**
     * @brief Fill the DENM message management container
     * @param denm_mgmt_container  The ASN.1 DENM management container to be filled
     * @param denm_data  The data to be included in the DENM message
     */
    void fillDenDataManagement(asn1cpp::Seq<ManagementContainer> denm_mgmt_container, denData &denm_data);
    /**
     * @brief Fill the DENM message situation container
     * @param denm_situation_container  The ASN.1 DENM situation container to be filled
     * @param denm_data  The data to be included in the DENM message
     */
    void fillDenDataSituation(asn1cpp::Seq<SituationContainer> denm_situation_container, denData &denm_data);
    /**
     * @brief Fill the DENM message location container
     * @param denm_location_container  The ASN.1 DENM location container to be filled
     * @param denm_data  The data to be included in the DENM message
     */
    void fillDenDataLocation(asn1cpp::Seq<LocationContainer> denm_location_container, denData &denm_data);
    /**
     * @brief Fill the DENM message alacarte container
     * @param denm_alacarte_container  The ASN.1 DENM alacarte container to be filled
     * @param denm_data  The data to be included in the DENM message
     */
    void fillDenDataAlacarte(asn1cpp::Seq<AlacarteContainer> denm_alacarte_container, denData &denm_data);

    /*
    * Mutex to protect m_originatingITSSTable when appDENM_update() and the callback for the expiration of the T_Repetion timer may try to
    * access the map concurrently, resulting in a thread-unsafe code.
    */
    std::mutex T_Repetition_Mutex;

    std::queue<void *> m_ptr_queue;
  };

}


#endif // DENBASICSERVICE_H

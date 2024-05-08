#ifndef emergencyVehicleWarningClient80211P_H
#define emergencyVehicleWarningClient80211P_H


#include "ns3/application.h"
#include "ns3/asn_utils.h"

#include <unordered_map>

#include "ns3/denBasicService.h"
#include "ns3/caBasicService.h"
#include "ns3/vdpTraci.h"
#include "ns3/socket.h"
#include "ns3/iviService.h"
#include "ns3/traci-client.h"
namespace ns3 {

class emergencyVehicleWarningClient80211p : public Application
{
  public:

    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId (void);

    emergencyVehicleWarningClient80211p ();

    virtual ~emergencyVehicleWarningClient80211p ();

    void StopApplicationNow ();

    /**
     * \brief Callback to handle a CAM reception.
     *
     * This function is called everytime a packet is received by the CABasicService.
     *
     * \param the ASN.1 CAM structure containing the info of the packet that was received.
     */
    void receiveCAM (asn1cpp::Seq<CAM>, Address from);

    /**
     * \brief Callback to handle a IVIM reception.
     *
     * This function is called everytime a packet is received by the IVIBasicService.
     *
     * \param the denData structure containing the info of the packet that was received.
     */
    void receiveIVIM (iviData ivim, Address from);
    void checkRelevanceZone();

  protected:
    virtual void DoDispose (void);

  private:

    CABasicService m_caService; //!< CA Basic Service object
    IVIBasicService m_iviService; //! IVI service object
    Ptr<btp> m_btp; //! BTP object
    Ptr<GeoNet> m_geoNet; //! GeoNetworking Object
    Ipv4Address m_ipAddress; //!< C-V2X self IP address (set by 'v2v-cv2x.cc')
    Ptr<Socket> m_socket; //!< Socket TX/RX for everything
    std::string m_model; //!< Communication Model (possible values: 80211p and cv2x)
    libsumo::TraCIPosition m_startRel, m_endRel;
    double m_segmentLower,m_segmentUpper;


    /**
     * \brief Set the maximum speed of the current vehicle
     *
     * This function rolls back the speed of the vehicle, turning it to its original value.
     * It also change the color of the vehicle to yellow (i.e. the default vehicle color)
     *
     */
    void SetMaxSpeed ();

    virtual void StartApplication (void);
    virtual void StopApplication (void);

    double m_distance_threshold;
    double m_heading_threshold;

    Ptr<TraciClient> m_client; //!< TraCI client
    std::string m_id; //!< vehicle id
    std::string m_type; //!< vehicle type
    double m_max_speed; //!< To save initial veh max speed
    double m_denm_intertime; //!< Time between two consecutives DENMs
    bool m_print_summary; //!< To print a small summary when vehicle leaves the simulation
    bool m_already_print; //!< To avoid printing two summaries
    bool m_real_time; //!< To decide wheter to use realtime scheduler
    std::string m_csv_name; //!< CSV log file name
    std::ofstream m_csv_ofstream_cam; //!< CSV log stream (CAM), created using m_csv_name
    std::vector <bool> m_schedule; //!< To check if a relrevanceZone check is already scheduled

    /* Counters */
    int m_cam_received;
    int m_denm_sent;
    int m_denm_received;
    int m_ivim_received;

    bool m_send_cam;

    Ptr<PRRSupervisor> m_PRR_supervisor = nullptr;

    EventId m_speed_ev; //!< Event to change the vehicle speed
    EventId m_send_denm_ev; //!< Event to send the DENM
    EventId m_send_cam_ev; //!< Event to send the CAM
    EventId m_update_denm_ev; //!< Event to update the DENM
    EventId m_check_rel_zone_ev; //!<Event to check if still in the relevance zone

  };

} // namespace ns3

#endif /* emergencyVehicleWarningClient80211P_H */


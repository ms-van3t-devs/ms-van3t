#ifndef trafficManagerServerLTE_H
#define trafficManagerServerLTE_H



#include "ns3/application.h"
#include "ns3/asn_utils.h"
#include "ns3/vdp.h"
#include "ns3/denBasicService.h"
#include "ns3/caBasicService.h"
#include "ns3/btp.h"
#include "ns3/btpHeader.h"
#include <mutex>
#include <unordered_map>
#include "ns3/traci-client.h"
namespace ns3 {

class trafficManagerServerLTE : public Application
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId (void);

    trafficManagerServerLTE ();

    virtual ~trafficManagerServerLTE ();

    /**
     * \brief Callback to handle a CAM reception.
     *
     * This function is called everytime a packet is received by the CABasicService.
     *
     * \param the ASN.1 CAM structure containing the info of the packet that was received.
     */
    void receiveCAM (asn1cpp::Seq<CAM> cam, Address address);

    void receiveDENM(denData denm, Address from);

    void StopApplicationNow ();
 
  protected:
    virtual void DoDispose (void);

  private:

    DENBasicService m_denService; //!< DEN Basic Service object
    CABasicService m_caService; //!< CA Basic Service object

    Ptr<btp> m_btp; //! BTP object
    Ptr<GeoNet> m_geoNet; //! GeoNetworking Object

    Ptr<Socket> m_socket; //!< Server socket

    virtual void StartApplication (void);
    virtual void StopApplication (void);

     /**
     * @brief This function compute the milliseconds elapsed from 2004-01-01
    */
    long compute_timestampIts ();

    void phase1Timeout(void);
    void phase2Timeout(void);
    void phaseTimeout(std::string intersection);

    uint8_t edgeCheck(double,double,double);

    Ptr<TraciClient> m_client; //!< TraCI client
    bool m_real_time; //!< To decide wheter to use realtime scheduler

    std::unordered_map<std::string,std::vector<std::string>> m_tls_lane_map;
    unsigned long m_tls_lanes_number;

    /* Counters */
    u_int m_cam_received;

    uint16_t m_phase_time;
    double m_threshold;
    std::map <unsigned long, std::string> m_veh_position;
    std::map <std::string, unsigned long> m_veh_number;
    std::map <std::string,std::pair<double,double>> m_load;
    std::map <std::string,unsigned long> m_phase_map;
    std::map <std::string,EventId> m_timeout_map;
    std::mutex m_load_Mutex;

    Ptr<MetricSupervisor> m_metric_supervisor = nullptr;
  };

} // namespace ns3

#endif /* trafficManagerServerLTE_H */


#ifndef AREASPEEDADVISORSERVERLTE_H
#define AREASPEEDADVISORSERVERLTE_H

#include "ns3/traci-client.h"
#include "ns3/application.h"
#include "ns3/asn_utils.h"

#include "ns3/denBasicService.h"
#include "ns3/caBasicService.h"
#include "ns3/btp.h"
#include "ns3/btpHeader.h"

namespace ns3 {

  enum pos {
          INSIDE,
          OUTSIDE
  };

  typedef enum
  {
      slowSpeedkmph=25,
      highSpeedkmph=75
  } speedmode_t;

  class areaSpeedAdvisorServerLTE : public Application
  {
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId (void);

    areaSpeedAdvisorServerLTE ();

    virtual ~areaSpeedAdvisorServerLTE ();

    /**
     * \brief Callback to handle a CAM reception.
     *
     * This function is called everytime a packet is received by the CABasicService.
     *
     * \param the ASN.1 CAM structure containing the info of the packet that was received.
     */
    void receiveCAM (CAM_t *cam, Address address);

    void receiveDENM(denData denm, Address from);

    void StopApplicationNow ();

    bool m_lon_lat; //!< Use LonLat instead of XY

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

    bool isInside(double, double);
    /**
     * \brief Trigger a new DENM (i.e. call appDENM_trigger as foreseen by ETSI EN 302 637-3 V1.3.1)
     *
     * This function can be called to send a new DENM.
     *
     */
    void TriggerDenm(speedmode_t speedmode, Address from);
    /**
     * @brief This function compute the milliseconds elapsed from 2004-01-01
    */
    long compute_timestampIts ();
    /**
     * @brief Used to print a report on number of msg received each second
    */
    void aggregateOutput(void);

    Ptr<TraciClient> m_client; //!< TraCI client
    libsumo::TraCIPosition m_upperLimit; //!< To store the speed limit area boundaries
    libsumo::TraCIPosition m_lowerLimit; //!< To store the speed limit area boundaries
    bool m_aggregate_output; //!< To decide wheter to print the report each second or not
    bool m_real_time; //!< To decide wheter to use realtime scheduler
    std::string m_csv_name; //!< CSV log file name
    std::ofstream m_csv_ofstream_cam;

    /* Counters */
    u_int m_cam_received;
    u_int m_denm_sent;

    EventId m_aggegateOutputEvent; //!< Event to create aggregate output

    std::map <Address, int> m_veh_position; //!< To trigger the DENM sending. If int = 0 the vehicle with Address is in the slow-speed area, 1 in the high-speed area
  };

} // namespace ns3

#endif /* AREASPEEDADVISORSERVERLTE_H */


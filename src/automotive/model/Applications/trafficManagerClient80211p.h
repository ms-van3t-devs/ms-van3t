#ifndef trafficManagerClient80211p_H
#define trafficManagerClient80211p_H


#include "ns3/application.h"
#include "ns3/asn_utils.h"

#include "ns3/denBasicService.h"
#include "ns3/caBasicService.h"

#include "ns3/btp.h"
#include "ns3/traci-client.h"

namespace ns3 {


class trafficManagerClient80211p : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  trafficManagerClient80211p ();

  virtual ~trafficManagerClient80211p ();

  void receiveDENM(denData denm, Address from);
  void receiveCAM (asn1cpp::Seq<CAM>, Address from);
  void StopApplicationNow ();

protected:
  virtual void DoDispose (void);

private:

  DENBasicService m_denService; //!< DEN Basic Service object
  CABasicService m_caService; //!< CA Basic Service object

  Ptr<btp> m_btp; //! BTP object
  Ptr<GeoNet> m_geoNet; //! GeoNetworking Object

  Ptr<Socket> m_socket; //!< Client socket

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  Ptr<TraciClient> m_client; //!< TraCI client
  std::string m_id; //!< vehicle id
  bool m_real_time; //!< To decide wheter to use realtime scheduler
  bool m_print_summary; //!< To print a small summary when vehicle leaves the simulation
  bool m_already_print; //!< To avoid printing two summary
  Ipv4Address m_server_addr; //!< Remote addr

  /* Counters */
  int m_cam_sent;
  int m_denm_received;


  bool m_send_cam;

  Ptr<MetricSupervisor> m_metric_supervisor = nullptr;
};
}
#endif // trafficManagerClient80211p_H

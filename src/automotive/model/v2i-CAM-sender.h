#ifndef V2I_CAM_SENDER_H
#define V2I_CAM_SENDER_H

#include "ns3/traci-client.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"

#define SPEED_OF_LIGHT      299792458.0
#define OFFSET_X            0
#define OFFSET_Y            0

//CAM utils
#define FIX_PROT_VERS       1
#define FIX_CAMID           2
#define DEF_YAWRATE         32767
#define FIX_YAWRATE_CONF    2
#define DEF_YAWRATE_CONF    8
#define FIX2D               1
#define DEF_LATITUDE        90000001
#define DEF_LONGITUDE       1800000001
#define DEF_SPEED           16383
#define DEF_SPEED_CONF      127
#define FIX_SPEED_CONF      1
#define DEF_ACCELERATION    161
#define DEF_ACCEL_CONF      102
#define FIX_ACCEL_CONF      1
#define DEF_HEADING_CONF    127
#define DEF_HEADING         3601



//unit measure CAM
#define CENTI               100
#define DOT_ONE_MICRO       10000000


namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications 
 * \defgroup TrafficInfo TrafficInfo
 */

/**
 * \ingroup TrafficInfo
 * \brief A Traffic Info server
 *
 * Traffic information is broadcasted
 */

class CAMSender : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  CAMSender ();

  virtual ~CAMSender ();

  void StopApplicationNow ();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  
  /**
   * @brief This function is used to send CAMs
   */
  void SendCam(void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

  /**
   * @brief This function is used to print the DENM and call the logic after a Denm reception
   * @param The vector containing an element for each value received
   */
  void HandleDenm (std::vector<std::string> values);

  /**
   * @brief This function is to encode and send a CAM using ASN.1
  */
  void Populate_and_send_asn_cam(struct timespec tv);

  /**
   * @brief This function is to send a CAM in plain text
  */
  void Populate_and_send_normal_cam(struct timespec tv);

  /**
   * @brief This function is to eventually compute the time diff between two timestamps
  */
  double time_diff(double sec1, double usec1, double sec2, double usec2);


  bool m_asn;

  double m_sumo_update;//!< SUMO granularity
  Ptr<Socket> m_socket; //!< Socket
  uint16_t m_port; //!< Port on which client will listen for traffic information
  Ptr<TraciClient> m_client;
  bool m_real_time;

  int m_cam_sent;
  int m_denm_received;
  bool m_print_summary;
  std::string m_server_addr;
  bool m_already_print;

  /* This part is only used when we have one client per process*/
  int m_index;
  std::string m_id;
  std::string m_veh_prefix;

  u_int16_t m_cam_seq;
  std::map <u_int16_t,struct timespec> m_cam_times;
  bool m_send_cam;
  double m_cam_intertime;

  long long m_start_ms; /*To save the base time of simulation*/

  EventId m_sendCamEvent; //!< Event to print the position

};

} // namespace ns3

#endif /* TRAFFIC_INFO_SERVER_H */


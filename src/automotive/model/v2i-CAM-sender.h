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
   * @brief This function is to encode and send a CAM using ASN.1
  */
  void Populate_and_send_asn_cam();

  /**
   * @brief This function is to send a CAM in plain text
  */
  void Populate_and_send_normal_cam();

  /**
   * @brief This function is to eventually compute the time diff between two timestamps
  */
  double time_diff(double sec1, double usec1, double sec2, double usec2);

  /**
   * @brief This function compute the timestamps
  */
  struct timespec compute_timestamp();

  Ptr<Socket> m_socket; //!< Socket
  uint16_t m_port; //!< Port on which client will listen for traffic information
  Ptr<TraciClient> m_client; //!< TraCI client

  bool m_real_time; //!< To decide wheter to use realtime scheduler
  bool m_asn; //!< To decide if ASN.1 is used
  double m_sumo_update; //!< SUMO granularity
  bool m_print_summary; //!< To print a small summary when vehicle leaves the simulation
  std::string m_server_addr; //!< Remote addr
  bool m_already_print; //!< To avoid printing two summary
  bool m_send_cam; //!< To decide if CAM dissemination is active or not
  double m_cam_intertime; //!< Time between two consecutives CAMs

  /* Counters */
  int m_cam_sent;
  int m_denm_received;
  u_int16_t m_cam_seq; //!< CAM sequence

  int m_index;  //!< vehicle index
  std::string m_id; //!< vehicle id
  std::string m_veh_prefix; //!< prefix used in SUMO

  long long m_start_ms; //!< To save the base time of simulation*/

  EventId m_sendCamEvent; //!< Event to send the CAM

};

} // namespace ns3

#endif /* V2I_CAM_SENDER_H */


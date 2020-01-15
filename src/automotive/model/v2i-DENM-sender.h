#ifndef V2I_DENM_SENDER_H
#define V2I_DENM_SENDER_H

#include "ns3/traci-client.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"

#define SPEED_OF_LIGHT      299792458.0
#define MAX_CACHED_SEQ      30000

#define FIX_PROT_VERS       1
#define FIX_DENMID          1


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

class DENMSender : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  DENMSender ();
  virtual ~DENMSender ();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

/**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */

  void HandleRead (Ptr<Socket> socket);


   /**
   * @brief CollisionMapFill - used to fill the collision map when a collision vetween two vehicles is identified by the collision detector
   * @param vehicle_id1
   * @param vehicle_id2
   * @param t2c_s2c
   * @param cam_seq_num
   * @param is_responsible
   */

  void aggregateOutput(void);

  /**
   * @brief This function is to encode and send a DENM using ASN.1
  */
  void Populate_and_send_asn_denm(Address address);

  /**
   * @brief This function is to encode and send a DENM in plain text
  */
  void Populate_and_send_normal_denm(Address address);

  /**
   * @brief This function is to eventually compute the time diff between two timestamps
  */
  double time_diff(double sec1, double usec1, double sec2, double usec2);

  bool m_aggregate_output;

  Ptr<TraciClient> m_client;
  uint16_t m_port; //!< Port on which traffic information is sent
  Ptr<Socket> m_socket; //!< IPv4 Socket
  EventId m_aggegateOutputEvent; //!< Event to create aggregate output
  bool m_real_time;
  EventId m_sendEvent; //!< Event to send the next packet

  //Value used to create aggregate output
  u_int m_cam_received;
  u_int m_denm_sent;

  long long m_start_ms; /*To save the base time of simulation*/

  bool m_asn;

};

} // namespace ns3

#endif /* TRAFFIC_INFO_SERVER_H */


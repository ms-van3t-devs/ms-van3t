#ifndef V2I_DENM_SENDER_H
#define V2I_DENM_SENDER_H

#include "ns3/socket.h"
#include "utils.h"
#include "ns3/appServer.h"
#include <chrono>

#define SPEED_OF_LIGHT      299792458.0
#define MAX_CACHED_SEQ      30000

#define FIX_PROT_VERS       1
#define FIX_DENMID          1
#define FIX_CAMID           2

#define CENTI               100

//Epoch time at 2004-01-01
#define TIME_SHIFT 1072915200000

enum ret {
	DO_NOT_SEND,	/* Dont send DENM */
	SEND_0,		/* Send DENM with seq=0 */
	SEND_1		/* Send DENM with seq=1 */
};

namespace ns3 {

class Socket;
class Packet;

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

  /**
   * @brief This function is used to send CAMs
   */
  int SendDenm(den_data_t denm, Address address);

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
   * @brief This function is to encode and send a DENM using ASN.1
  */
  int Populate_and_send_asn_denm(den_data_t denm, Address address);

  /**
   * @brief This function is to encode and send a DENM in plain text
  */
  int Populate_and_send_normal_denm(den_data_t denm, Address address);
  /**
   * @brief This function is to decode a CAM using ASN.1
  */
  void Decode_asn_cam(uint8_t *buffer,uint32_t size,Address address);
  /**
   * @brief This function is to decode a CAM in plain text
  */
  void Decode_normal_cam(uint8_t *buffer,Address address);
  /**
   * @brief This function is to eventually compute the time diff between two timestamps
  */
  double time_diff(double sec1, double usec1, double sec2, double usec2);

  /**
   * @brief This function compute the timestamps
  */
  struct timespec compute_timestamp();
  /**
   * @brief This function compute the milliseconds elapsed from 2004-01-01
  */
  long compute_timestampIts ();

  Ptr<TraciClient> m_client; //!< TraCI client
  uint16_t m_port; //!< Port on which traffic is sent
  Ptr<Socket> m_socket; //!< Socket
  bool m_real_time; //!< To decide wheter to use realtime scheduler
  bool m_asn; //!< To decide if ASN.1 is used

  long m_sequence = 0; //!< Sequence number of DENMs
  long m_actionId = 0; //!< ActionID number of DENMs

  EventId m_sendEvent; //!< Event to send the next packet

  int m_this_id; //!< The ID of the ITS station. Set to 0

};

} // namespace ns3

#endif /* V2I_DENM_SENDER_H */


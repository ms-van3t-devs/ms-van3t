#ifndef V2I_CAM_SENDER_H
#define V2I_CAM_SENDER_H

#include "ns3/socket.h"
#include "utils.h"
#include "appClient.h"

#include <chrono>

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
  /**
   * @brief This function is used to send CAMs
   */
  void SendCam(ca_data_t cam);


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
   * \param the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);

  /**
   * @brief This function is to encode and send a CAM using ASN.1
  */
  void Populate_and_send_asn_cam(ca_data_t cam);

  /**
   * @brief This function is to send a CAM in plain text
  */
  void Populate_and_send_normal_cam(ca_data_t cam);
  /**
   * @brief This function is to decode a DENM using ASN.1
  */
  void Decode_asn_denm(uint8_t *buffer,uint32_t size);
  /**
   * @brief This function is to decode a DENM in plain text
  */
  void Decode_normal_denm(uint8_t *buffer);


  Ptr<Socket> m_socket; //!< Socket
  uint16_t m_port; //!< Port on which client will listen for traffic information

  bool m_asn; //!< To decide if ASN.1 is used
  Ipv4Address m_server_addr; //!< Remote addr

};

} // namespace ns3



#endif /* V2I_CAM_SENDER_H */

